// button_handler — WinUI 3 platform recipe: a real Microsoft.UI.Xaml.Controls.Button, the same native
// type ButtonHandler.Windows.cs creates. Ported from ButtonHandler.Windows.cs + ButtonExtensions.cs.
//
// TWO DOCUMENTED SIMPLIFICATIONS against C#, both narrowed on purpose rather than left vague:
//
//  1. C# creates a MauiButton whose Content is a DefaultMauiButtonContent — a custom MauiPanel laying an
//     Image beside a TextBlock per ContentLayout. This slice sets a plain centred TextBlock as Content,
//     which is what that panel renders whenever there is no image (i.e. every text-only button, which is
//     what the parity board's button pages are). Compound text+image buttons need that panel; until it
//     exists map_image_source stays on the mirror-only primitives at the bottom of this file.
//  2. C# pushes StrokeColor / StrokeThickness / CornerRadius as THEME-RESOURCE overrides
//     (ButtonBorderBrush, ButtonBorderBrushPointerOver, ...Pressed, ...Disabled + RefreshThemeResources),
//     so the override survives into the hover/pressed/disabled visual states. This slice sets the direct
//     BorderBrush / BorderThickness / CornerRadius properties, which is identical AT REST — what a
//     screenshot captures — and diverges only while the pointer is over or holding the button.

#include "maui/core/button_handler.hpp"

#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Input.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>

#include <algorithm>
#include <array>
#include <span>
#include <cmath>
#include <limits>
#include <memory>
#include <string>
#include <string_view>

#include "maui/core/bindable_object.hpp"
#include "maui/core/i_button.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_text_button.hpp"
#include "maui/core/image_source_loader.hpp"
#include "maui/core/image_source_result.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "winui_interop.hpp"
#include "winui_visual_ops.hpp"

namespace
{
    // Named `winui`, NOT `xaml`: the port already has a maui::xaml namespace (the XAML loader), and
    // inside namespace maui::* that name WINS over a file-scope alias - an `xaml::Application` here
    // would resolve to maui::xaml and fail with "'Start': is not a member of 'maui::xaml'".
    namespace winui = winrt::Microsoft::UI::Xaml;
    using button_control = winui::Controls::Button;
    using text_block = winui::Controls::TextBlock;

    button_control as_button(void* native)
    {
        return maui::platform::windows::ref<winui::UIElement>(native).as<button_control>();
    }

    // The Button's Content TextBlock (created in create_platform_view). Returns a null projected object
    // if the content was replaced by something else, so every caller must test it.
    text_block content_text(const button_control& button)
    {
        return button.Content().try_as<text_block>();
    }

    // ButtonExtensions' resource-key sets. A WinUI control template binds its per-visual-state brushes
    // to THEME RESOURCES, not to the control's own properties, so a local Foreground/Background is
    // dropped again the moment the control changes visual state. MAUI overrides the resources instead
    // (and then the direct property too, for API contract >= 5).
    constexpr std::array<std::wstring_view, 4> k_text_color_keys{
        L"ButtonForeground", L"ButtonForegroundPointerOver", L"ButtonForegroundPressed",
        L"ButtonForegroundDisabled"};

    void set_resources(const button_control& button, std::span<const std::wstring_view> keys,
                       const winui::Media::Brush& brush)
    {
        for (const auto& key : keys)
        {
            button.Resources().Insert(winrt::box_value(winrt::hstring{key}), brush);
        }
    }

    void remove_resources(const button_control& button, std::span<const std::wstring_view> keys)
    {
        for (const auto& key : keys)
        {
            button.Resources().Remove(winrt::box_value(winrt::hstring{key}));
        }
    }

    // FrameworkElementExtensions.RefreshThemeResources: flip RequestedTheme away and back so the control
    // template re-resolves the resources just overridden. Without it the override does not take effect
    // until something else invalidates the template.
    void refresh_theme_resources(const winui::FrameworkElement& element)
    {
        const auto previous = element.RequestedTheme();
        element.RequestedTheme(element.ActualTheme() == winui::ElementTheme::Dark ? winui::ElementTheme::Light
                                                                                 : winui::ElementTheme::Dark);
        element.RequestedTheme(previous);
    }

    // "Was this property explicitly set?" — the port's equivalent of C#'s `Color?` being non-null. It has
    // to go through bindable_object: the i_* view CONTRACTS carry values, not set-ness. Comparing the
    // value against a default instead is the bug in [[cpp-unset-color-sentinel-collision]] — it misreads
    // an explicit Black as unset, and the control loses the developer's color in dark mode.
    bool is_set(const maui::core::i_view& view, std::string_view property)
    {
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        return bindable != nullptr && bindable->is_property_set(property);
    }

    // The two pointer delegates, kept alive for as long as they are subscribed. RemoveHandler matches on
    // the delegate OBJECT (unlike the token-based Click revoke), so they have to be stored, and they are
    // WinRT types that the cross-platform button_handler.hpp must not see -- hence a heap box behind the
    // struct's void* pointer_events slot.
    struct pointer_sink
    {
        winui::Input::PointerEventHandler pressed{nullptr};
        winui::Input::PointerEventHandler released{nullptr};
    };

    winrt::Windows::UI::Text::FontWeight to_font_weight(maui::core::font_weight weight)
    {
        return winrt::Windows::UI::Text::FontWeight{static_cast<std::uint16_t>(weight)};
    }

    winrt::Windows::UI::Text::FontStyle to_font_style(maui::core::font_slant slant)
    {
        switch (slant)
        {
            case maui::core::font_slant::italic:
                return winrt::Windows::UI::Text::FontStyle::Italic;
            case maui::core::font_slant::oblique:
                return winrt::Windows::UI::Text::FontStyle::Oblique;
            case maui::core::font_slant::normal:
            default:
                return winrt::Windows::UI::Text::FontStyle::Normal;
        }
    }
} // namespace

namespace maui::core
{
    namespace
    {
        // Unhook everything on_connect_handler registered. Called from on_disconnect_handler AND from
        // ~button_platform: the handlers capture a button_platform*, so if the struct is destroyed while
        // still subscribed (a handler torn down without a disconnect, which the element tree does on
        // shutdown) the next pointer event fires into freed memory.
        void detach_native_events(maui::core::button_platform& platform)
        {
            if (platform.native != nullptr)
            {
                const button_control button = as_button(platform.native);
                button.Click(winrt::event_token{platform.click_token});
                if (platform.pointer_events != nullptr)
                {
                    auto* sink = static_cast<pointer_sink*>(platform.pointer_events);
                    button.RemoveHandler(winui::UIElement::PointerPressedEvent(), winrt::box_value(sink->pressed));
                    button.RemoveHandler(winui::UIElement::PointerReleasedEvent(), winrt::box_value(sink->released));
                }
            }
            platform.click_token = 0;
            delete static_cast<pointer_sink*>(platform.pointer_events);
            platform.pointer_events = nullptr;
        }
    } // namespace

    button_platform::~button_platform()
    {
        detach_native_events(*this);
        if (native != nullptr)
        {
            maui::platform::windows::drop<winui::UIElement>(native);
        }
    }

    std::unique_ptr<button_platform> button_handler::create_platform_view()
    {
        auto platform = std::make_unique<button_platform>();
        button_control button;
        // MauiButton's ctor: a centred content panel, stretched inside whatever hosts the button.
        text_block content;
        content.HorizontalAlignment(winui::HorizontalAlignment::Center);
        content.VerticalAlignment(winui::VerticalAlignment::Center);
        button.Content(content);
        button.HorizontalAlignment(winui::HorizontalAlignment::Stretch);
        button.VerticalAlignment(winui::VerticalAlignment::Stretch);
        platform->native = maui::platform::windows::take<winui::UIElement>(button);
        return platform;
    }

    void button_handler::on_connect_handler(button_platform& platform)
    {
        // The cross-platform half first: the same three callbacks every backend exposes, so a test (and
        // the DevFlow tap driver on the backends that have it) can raise them without a real pointer.
        platform.on_press = [this] {
            if (auto* view = virtual_view())
            {
                view->send_pressed();
            }
        };
        platform.on_release = [this] {
            if (auto* view = virtual_view())
            {
                view->send_released();
            }
        };
        // CLICKED ONLY - NOT released+clicked. The iOS/headless contract folds Released into the click
        // (UIControlEventTouchUpInside implies a release), but ButtonHandler.Windows.cs's OnClick raises
        // ONLY Clicked(); Released() comes solely from OnPointerReleased below. Raising both here would
        // report two Released events per click on this backend.
        platform.on_click = [this] {
            if (auto* view = virtual_view())
            {
                view->send_clicked();
            }
        };
        if (platform.native == nullptr)
        {
            return;
        }
        // Then the native half. Click is a normal subscription; the POINTER events are not, and that
        // difference is load-bearing: ButtonBase's control template marks PointerPressed and
        // PointerReleased handled, so a plain `.PointerPressed(...)` subscription is never invoked and
        // send_pressed/send_released would silently never fire. C#'s ConnectHandler uses
        // AddHandler(UIElement.PointerPressedEvent, handler, handledEventsToo: true) for exactly this
        // reason, and so does this.
        auto* self = &platform;
        const button_control button = as_button(platform.native);
        platform.click_token =
            button
                .Click([self](const winrt::Windows::Foundation::IInspectable&, const winui::RoutedEventArgs&) {
                    if (self->on_click)
                    {
                        self->on_click();
                    }
                })
                .value;

        auto sink = std::make_unique<pointer_sink>();
        sink->pressed = winui::Input::PointerEventHandler(
            [self](const winrt::Windows::Foundation::IInspectable&, const winui::Input::PointerRoutedEventArgs&) {
                if (self->on_press)
                {
                    self->on_press();
                }
            });
        // The ONLY source of Released on this backend (see the on_click note above): C#'s
        // OnPointerReleased -> VirtualView.Released(). It therefore fires for a release outside the
        // button too, which is what MAUI does.
        sink->released = winui::Input::PointerEventHandler(
            [self](const winrt::Windows::Foundation::IInspectable&, const winui::Input::PointerRoutedEventArgs&) {
                if (self->on_release)
                {
                    self->on_release();
                }
            });
        button.AddHandler(winui::UIElement::PointerPressedEvent(), winrt::box_value(sink->pressed), true);
        button.AddHandler(winui::UIElement::PointerReleasedEvent(), winrt::box_value(sink->released), true);
        platform.pointer_events = sink.release();
    }

    void button_handler::on_disconnect_handler(button_platform& platform)
    {
        detach_native_events(platform);
        platform.on_press = nullptr;
        platform.on_release = nullptr;
        platform.on_click = nullptr;
    }

    void button_handler::map_text(button_handler& handler, i_text_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->title = std::string(view.text());
        if (const text_block content = content_text(as_button(platform->native)))
        {
            content.Text(maui::platform::windows::to_hstring(platform->title));
            // COLLAPSE on empty, do not merely blank it: ButtonExtensions.UpdateText does this so an
            // image-only button reserves no text slot. A Visible-but-empty TextBlock still contributes a
            // line's height to the button's measure.
            content.Visibility(platform->title.empty() ? winui::Visibility::Collapsed
                                                       : winui::Visibility::Visible);
        }
    }

    void button_handler::map_text_color(button_handler& handler, i_text_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->text_color = view.text_color();
        const button_control button = as_button(platform->native);
        // As in label_handler: an UNSET TextColor must leave the theme brush alone rather than paint
        // transparent black — see [[cpp-unset-color-sentinel-collision]].
        if (!is_set(view, "text_color"))
        {
            remove_resources(button, k_text_color_keys);
            button.ClearValue(winui::Controls::Control::ForegroundProperty());
            refresh_theme_resources(button);
            return;
        }
        const winui::Media::SolidColorBrush brush{maui::platform::windows::to_ui_color(platform->text_color)};
        // BOTH the theme-resource overrides AND the direct Foreground, exactly as
        // ButtonExtensions.UpdateTextColor does. Foreground alone is reverted by the control template the
        // moment the pointer enters the button (the template rebinds Foreground to
        // ButtonForegroundPointerOver), so hover/pressed/disabled would silently drop the color.
        set_resources(button, k_text_color_keys, brush);
        button.Foreground(brush);
        refresh_theme_resources(button);
    }

    void button_handler::map_font(button_handler& handler, i_text_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->text_font = view.font();
        const button_control button = as_button(platform->native);
        const font& f = platform->text_font;
        if (f.size() > 0)
        {
            button.FontSize(f.size());
        }
        if (!f.family().empty())
        {
            button.FontFamily(winui::Media::FontFamily{maui::platform::windows::to_hstring(f.family())});
        }
        button.FontStyle(to_font_style(f.slant()));
        button.FontWeight(to_font_weight(f.weight()));
        // FontAutoScalingEnabled -> IsTextScaleFactorEnabled, which exists on TextBlock, not on Control -
        // so it goes on the content, matching what UpdateFont(TextBlock, ...) does for a label.
        if (const text_block content = content_text(button))
        {
            content.IsTextScaleFactorEnabled(f.auto_scaling_enabled());
        }
    }

    void button_handler::map_character_spacing(button_handler& handler, i_text_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->character_spacing = view.character_spacing();
        // CharacterSpacingExtensions.ToEm: pt * 0.0624 * 1000, in 1/1000 em units.
        const auto em = static_cast<std::int32_t>(std::lround(platform->character_spacing * 0.0624 * 1000.0));
        const button_control button = as_button(platform->native);
        button.CharacterSpacing(em);
        // BOTH, exactly as ButtonExtensions.UpdateCharacterSpacing does. Control.CharacterSpacing is not
        // inherited by an explicitly-constructed content TextBlock, so setting only the Button leaves the
        // label text at default spacing - the property has no visible effect at all.
        if (const text_block content = content_text(button))
        {
            content.CharacterSpacing(em);
        }
    }

    void button_handler::map_padding(button_handler& handler, i_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->padding = view.padding();
        // An UNSET Padding must leave the WinUI Button's own default content padding in place: zeroing it
        // is what produced the crammed-digit-row class of iOS diffs (see PARITY ruling 4 / the clipping
        // page). Only an explicitly-set Padding overrides the native default.
        if (!is_set(view, "padding"))
        {
            as_button(platform->native).ClearValue(winui::Controls::Control::PaddingProperty());
            return;
        }
        const maui::core::thickness& p = platform->padding;
        as_button(platform->native).Padding(winui::Thickness{p.left, p.top, p.right, p.bottom});
    }

    void button_handler::map_stroke_color(button_handler& handler, i_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->stroke_color = view.stroke_color();
        const button_control button = as_button(platform->native);
        // C#: a null StrokeColor REMOVES the resource override so the theme brush returns.
        if (!is_set(view, "stroke_color"))
        {
            button.ClearValue(winui::Controls::Control::BorderBrushProperty());
            return;
        }
        button.BorderBrush(winui::Media::SolidColorBrush{maui::platform::windows::to_ui_color(platform->stroke_color)});
    }

    void button_handler::map_stroke_thickness(button_handler& handler, i_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->stroke_thickness = view.stroke_thickness();
        const button_control button = as_button(platform->native);
        // C#: only a NON-NEGATIVE thickness overrides; a negative one restores the theme value.
        if (platform->stroke_thickness >= 0)
        {
            const double t = platform->stroke_thickness;
            button.BorderThickness(winui::Thickness{t, t, t, t});
        }
        else
        {
            button.ClearValue(winui::Controls::Control::BorderThicknessProperty());
        }
    }

    void button_handler::map_corner_radius(button_handler& handler, i_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->corner_radius = view.corner_radius();
        const button_control button = as_button(platform->native);
        if (platform->corner_radius >= 0)
        {
            const auto r = static_cast<double>(platform->corner_radius);
            button.CornerRadius(winui::CornerRadius{r, r, r, r});
        }
        else
        {
            button.ClearValue(winui::Controls::Control::CornerRadiusProperty());
        }
    }

    maui::graphics::size button_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        // GetDesiredSizeFromHandler's first guard: a negative constraint measures to nothing. XAML's
        // Measure THROWS on a negative Size, so this is a crash guard, not a formality.
        if (width_constraint < 0 || height_constraint < 0)
        {
            return {0, 0};
        }
        const button_control button = as_button(platform->native);
        // Clear the pinned size FIRST. platform_arrange stamps Width/Height on this element (a Canvas
        // child has no other way to be sized), and a FrameworkElement with an explicit Width/Height
        // measures to exactly that -- so the second and later layout passes would just read back the
        // PREVIOUS frame instead of re-measuring. NaN is XAML's "Auto". C# does the same thing for the
        // same reason: LabelHandler.Windows.SetupContainer sets `PlatformView.Height = double.NaN`.
        const auto auto_size = std::numeric_limits<double>::quiet_NaN();
        button.Width(auto_size);
        button.Height(auto_size);
        button.Measure(winrt::Windows::Foundation::Size{
            maui::platform::windows::measure_constraint(width_constraint),
            maui::platform::windows::measure_constraint(height_constraint)});
        const auto desired = button.DesiredSize();
        return {desired.Width, desired.Height};
    }

    bool button_handler::content_is_minimum_size() const
    {
        // FALSE, like headless and unlike iOS. The iOS opt-in exists because UIButton's WrapperView
        // imposes an intrinsic floor MAUI's iOS render demonstrably shows
        // ([[cpp-clipping-button-natural-floor]]). Windows has NO equivalent: ViewHandlerExtensions.
        // Windows.GetDesiredSizeFromHandler just measures and returns DesiredSize, and the explicit
        // WidthRequest flows through the cross-platform ResolveConstraints clamp like every other
        // backend. Returning true here would be asserting a floor no oracle describes; if the board
        // shows MAUI refusing to shrink a Windows button, revisit with the capture as evidence.
        return false;
    }

    void button_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        // PlatformArrangeHandler's guard, WIDENED to non-finite. C# only tests `< 0` because its
        // cross-platform arrange never yields NaN; if one ever reaches XAML here it is an unrecoverable
        // stowed exception with no message and no stack (0xC000027B), so a skipped arrange is strictly
        // better than a dead process. A NaN arriving here is an upstream layout bug worth chasing, not
        // a value with a meaning.
        if (!std::isfinite(frame.x) || !std::isfinite(frame.y) || !std::isfinite(frame.width) ||
            !std::isfinite(frame.height) || frame.width < 0 || frame.height < 0)
        {
            return;
        }
        const button_control button = as_button(platform->native);
        winui::Controls::Canvas::SetLeft(button, frame.x);
        winui::Controls::Canvas::SetTop(button, frame.y);
        button.Width(frame.width);
        button.Height(frame.height);
    }

    // ---- per-backend image-source primitives ------------------------------------------------------
    // Mirror-only for now: rendering an image INSIDE the button needs the DefaultMauiButtonContent panel
    // (see the header note), so these keep the same observable mirrors the headless partial records
    // rather than pretending to display something. map_image_source still runs end to end.

    void button_handler::configure_loader(maui::core::image_source_loader& /*loader*/)
    {
    }

    void button_handler::load_file_source_sync(button_platform& platform, const i_file_image_source& file_src)
    {
        platform.source_kind = "file";
        platform.source_file = std::string(file_src.file());
        platform.source_loaded = true;
    }

    void button_handler::apply_loaded_result(button_platform& platform, const image_source_result& result)
    {
        if (!result.loaded())
        {
            clear_source_native(platform);
            return;
        }
        platform.source_kind = result.kind();
        platform.source_file = result.detail();
        platform.source_loaded = true;
    }

    void button_handler::clear_source_native(button_platform& platform)
    {
        platform.source_kind.clear();
        platform.source_file.clear();
        platform.source_loaded = false;
    }

    // ---- generic-IView property pushes (view_platform_base overrides) ---------------------------
    // Delegated to the shared winui_visual_ops free functions so all five controls behave identically;
    // see that header for why they are free functions taking the void* slot.
    void button_platform::update_visibility(maui::core::visibility value)
    {
        maui::platform::windows::apply_visibility(native, value);
    }

    void button_platform::update_opacity(double value)
    {
        maui::platform::windows::apply_opacity(native, value);
    }

    void button_platform::update_is_enabled(bool value)
    {
        maui::platform::windows::apply_is_enabled(native, value);
    }

    void button_platform::update_automation_id(std::string_view value)
    {
        maui::platform::windows::apply_automation_id(native, value);
    }

    void button_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::windows::apply_background(native, value);
    }
} // namespace maui::core
