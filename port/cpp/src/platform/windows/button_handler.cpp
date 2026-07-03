// button_handler — Windows (WinUI 3) platform partial: a REAL Microsoft.UI.Xaml.Controls.Button. The
// windows twin of src/platform/apple/button_handler.mm (NSButton) / the android JNI partial, and the
// real-native sibling of the headless mirror partial (src/platform/headless/button_handler.cpp). Every
// map_* pushes its property onto the control, and the native Click / PointerPressed / PointerReleased
// events flow back through the platform callbacks into i_button::send_clicked/pressed/released.
//
// Ported DIRECTLY from ButtonHandler.Windows.cs + Platform/Windows/{ButtonExtensions.cs,
// ControlExtensions.cs (UpdateFont/UpdateIsEnabled), ViewExtensions.cs, FontExtensions.cs,
// CharacterSpacingExtensions.cs} + Fonts/FontManager.Windows.cs.
//
// DOCUMENTED DEVIATIONS (each an infrastructure gap of this first cut, not a behavior guess):
//   - The control is a stock Button, not MauiButton (whose Content is a MauiPanel composing an Image +
//     TextBlock for the text+icon layout): the image fan-out has not reached this backend, so the text
//     rides Button.Content = boxed hstring directly, and ButtonExtensions.UpdateText's inner-TextBlock
//     visibility collapse has no analog. The image-source primitives below are headless-mirror stubs
//     (// deferred) for the same reason.
//   - Background / text color / stroke land on the DIRECT dependency properties (Background /
//     Foreground / BorderBrush / BorderThickness / CornerRadius). C# additionally writes the themed
//     resource keys (ButtonBackground*/ButtonForeground*/ButtonBorderBrush* + RefreshThemeResources) so
//     the PointerOver/Pressed/Disabled visual states track the pushed value — deferred: the direct
//     property covers the rest state; the state-brush resources need the port's resource-dictionary
//     seam. The C# null branches (RemoveKeys + ClearValue) map to ClearValue (restoring the theme
//     default), discriminated through BindableObject.IsSet where the port's value type has no null.
//   - FontManager's registrar lookups are skipped (no font registrar in the port yet): a named family
//     goes straight to FontFamily(name); the default size is the constant 14 (ControlContentThemeFontSize).
//   - The Unloaded → Released safety net (C#'s _isPressed guard for a button unloaded mid-press) is
//     deferred with the pointer-capture edge cases.
//
// XAML-less degradation (the windows twin of the android VM-less fallback): the windows preset also
// runs the cross-platform suite on the host where NO XAML runtime exists; create_platform_view catches
// the construction failure and keeps native null, while the headless mirrors are ALWAYS maintained and
// the on_press/on_release/on_click callbacks stay invokable C++ callbacks (the cross-platform suite
// drives them) — so that suite observes exactly the headless partial's behavior.

#include "maui/core/button_handler.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <string>

#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h> // ButtonBase.Click consume methods
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Input.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Text.h>
#include <winrt/base.h>

#include "maui/core/bindable_object.hpp"
#include "maui/core/dimension.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_button.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_text_button.hpp"
#include "maui/core/image_source_loader.hpp"
#include "maui/core/image_source_result.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "windows_native.hpp"

namespace
{
    namespace mux = winrt::Microsoft::UI::Xaml;
    namespace muxc = winrt::Microsoft::UI::Xaml::Controls;
    namespace muxi = winrt::Microsoft::UI::Xaml::Input;
    namespace muxm = winrt::Microsoft::UI::Xaml::Media;
    namespace wut = winrt::Windows::UI::Text;
    namespace wnative = maui::platform::win;

    // FontManager.Windows.DefaultFontSize — the ControlContentThemeFontSize theme resource (14.0);
    // read as a constant here (no Application.Current on the XAML-less test host). Same constant as the
    // label partial.
    constexpr double k_default_font_size = 14.0;

    [[nodiscard]] muxc::Button button_of(const maui::core::button_platform& platform)
    {
        return wnative::borrow<muxc::Button>(platform.native);
    }

    // FontExtensions.ToFontStyle: Slant → FontStyle (Italic / Oblique / Normal).
    [[nodiscard]] wut::FontStyle to_font_style(maui::core::font_slant slant)
    {
        switch (slant)
        {
            case maui::core::font_slant::italic:
                return wut::FontStyle::Italic;
            case maui::core::font_slant::oblique:
                return wut::FontStyle::Oblique;
            case maui::core::font_slant::normal:
            default:
                return wut::FontStyle::Normal;
        }
    }

    // FontExtensions.ToFontWeight — the port's font_weight enum values ARE the numeric OpenType
    // weights C#'s switch resolves to, so the numeric FontWeight constructor is the whole mapping.
    [[nodiscard]] wut::FontWeight to_font_weight(maui::core::font_weight weight)
    {
        return wut::FontWeight{static_cast<std::uint16_t>(weight)};
    }
} // namespace

namespace maui::core
{
    // Releases the one strong ref pinning the Button + any event-wiring remainder (the wnative shape of
    // the pimpl-owned-native doctrine; the apple twin CFReleases its NSButton here). The handler slots
    // are normally released in on_disconnect_handler — this is the defensive sweep for a platform that
    // dies without a disconnect.
    button_platform::~button_platform()
    {
        wnative::release(pointer_pressed_handler);
        wnative::release(pointer_released_handler);
        wnative::release(native);
    }

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each
    // calls the base body FIRST — the headless mirrors must stay live for the XAML-less cross-platform
    // suite (header note) — then pushes to the real Button when one exists.

    void button_platform::update_visibility(maui::core::visibility value)
    {
        view_platform_base::update_visibility(value);
        // ViewExtensions.UpdateVisibility (Windows): Hidden rides Opacity 0, Collapsed collapses.
        wnative::apply_visibility(native, value, alpha);
    }

    void button_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        // ViewExtensions.UpdateOpacity: the Hidden state pins opacity 0 (apply_visibility restores it).
        if (!hidden)
        {
            wnative::apply_opacity(native, value);
        }
    }

    void button_platform::update_is_enabled(bool value)
    {
        view_platform_base::update_is_enabled(value);
        // ViewExtensions.UpdateIsEnabled → ControlExtensions.UpdateIsEnabled: Control.IsEnabled (the
        // Button IS a Control — the label/canvas structs skip this override for the same C# reason).
        if (auto button = button_of(*this))
        {
            button.IsEnabled(value);
        }
    }

    void button_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        // ViewExtensions.UpdateAutomationId: AutomationProperties.SetAutomationId.
        wnative::apply_automation_id(native, value);
    }

    void button_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        auto button = button_of(*this);
        if (button == nullptr)
        {
            return;
        }
        // ButtonHandler.Windows.MapBackground → ButtonExtensions.UpdateBackground: a null brush removes
        // the ButtonBackground* resource keys (→ theme default), a value sets them all. The port pushes
        // the DIRECT Background property (deferred: the per-state resource keys — see the header) and
        // ClearValue for the null branch.
        if (value == nullptr)
        {
            button.ClearValue(muxc::Control::BackgroundProperty());
            return;
        }
        if (const auto* solid = dynamic_cast<const maui::graphics::solid_paint*>(value))
        {
            button.Background(wnative::to_brush(solid->color()));
            return;
        }
        // deferred: gradient / image-source paints (Paint.ToPlatform) — the base mirror above keeps the
        // borrow observable.
    }

    std::unique_ptr<button_platform> button_handler::create_platform_view()
    {
        auto platform = std::make_unique<button_platform>();
        try
        {
            // ButtonHandler.CreatePlatformView: new MauiButton() — a stock Button here (the MauiButton
            // Image+TextBlock content composition is deferred with the image fan-out; header).
            const muxc::Button button;
            platform->native = wnative::store(button); // released in ~button_platform
        }
        catch (const winrt::hresult_error&)
        {
            platform->native = nullptr; // XAML-less degradation (header note)
        }
        return platform;
    }

    void button_handler::on_connect_handler(button_platform& platform)
    {
        // ButtonHandler.Windows's event split: OnClick → Clicked; OnPointerPressed → Pressed;
        // OnPointerReleased → Released. The callbacks stay wired even XAML-less so the cross-platform
        // suite can drive them (the android partial's shape).
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
        platform.on_click = [this] {
            if (auto* view = virtual_view())
            {
                view->send_clicked();
            }
        };
        auto button = button_of(platform);
        if (button == nullptr)
        {
            return;
        }
        // The native events route through the platform callbacks (the peer is the platform struct,
        // whose heap address is stable until disconnect revokes these handlers).
        auto* peer = &platform;
        // ConnectHandler: platformView.Click += OnClick.
        const winrt::event_token click_token =
            button.Click([peer](const winrt::Windows::Foundation::IInspectable&, const mux::RoutedEventArgs&) {
                if (peer->on_click)
                {
                    peer->on_click();
                }
            });
        platform.click_token = click_token.value;
        // ConnectHandler: AddHandler(UIElement.PointerPressedEvent/PointerReleasedEvent, handler, true)
        // — handledEventsToo, because the Button control class marks the pointer events handled
        // internally. The boxed delegates are retained so DisconnectHandler can RemoveHandler the exact
        // same instances (C# keeps _pointerPressedHandler/_pointerReleasedHandler for the same reason).
        const muxi::PointerEventHandler pressed{
            [peer](const winrt::Windows::Foundation::IInspectable&, const muxi::PointerRoutedEventArgs&) {
                if (peer->on_press)
                {
                    peer->on_press();
                }
            }};
        const auto boxed_pressed = winrt::box_value(pressed);
        button.AddHandler(mux::UIElement::PointerPressedEvent(), boxed_pressed, true);
        platform.pointer_pressed_handler = wnative::store(boxed_pressed);
        const muxi::PointerEventHandler released{
            [peer](const winrt::Windows::Foundation::IInspectable&, const muxi::PointerRoutedEventArgs&) {
                if (peer->on_release)
                {
                    peer->on_release();
                }
            }};
        const auto boxed_released = winrt::box_value(released);
        button.AddHandler(mux::UIElement::PointerReleasedEvent(), boxed_released, true);
        platform.pointer_released_handler = wnative::store(boxed_released);
        // deferred: platformView.Unloaded += OnUnloaded (the mid-press unload Released safety net —
        // header deviations).
    }

    void button_handler::on_disconnect_handler(button_platform& platform)
    {
        // DisconnectHandler: Click -= OnClick; RemoveHandler(PointerPressed/ReleasedEvent, handler);
        // then drop the kept delegates. The C++ callbacks are cleared like the headless twin.
        platform.on_press = nullptr;
        platform.on_release = nullptr;
        platform.on_click = nullptr;
        if (auto button = button_of(platform))
        {
            if (platform.click_token != 0)
            {
                button.Click(winrt::event_token{platform.click_token});
            }
            if (platform.pointer_pressed_handler != nullptr)
            {
                const auto boxed =
                    wnative::borrow<winrt::Windows::Foundation::IInspectable>(platform.pointer_pressed_handler);
                button.RemoveHandler(mux::UIElement::PointerPressedEvent(), boxed);
            }
            if (platform.pointer_released_handler != nullptr)
            {
                const auto boxed =
                    wnative::borrow<winrt::Windows::Foundation::IInspectable>(platform.pointer_released_handler);
                button.RemoveHandler(mux::UIElement::PointerReleasedEvent(), boxed);
            }
        }
        platform.click_token = 0;
        wnative::release(platform.pointer_pressed_handler);
        wnative::release(platform.pointer_released_handler);
    }

    void button_handler::map_text(button_handler& handler, i_text_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->title = std::string(view.text());
        // ButtonExtensions.UpdateText writes the MauiButton's inner TextBlock (and collapses it when
        // the text is empty); the stock-Button cut sets Content = the boxed string (header deviation).
        if (auto button = button_of(*platform))
        {
            button.Content(winrt::box_value(wnative::to_hstring_utf8(view.text())));
        }
    }

    void button_handler::map_text_color(button_handler& handler, i_text_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->text_color = view.text_color();
        auto button = button_of(*platform);
        if (button == nullptr)
        {
            return;
        }
        // ButtonExtensions.UpdateTextColor: null → RemoveKeys(ButtonForeground*) +
        // ClearValue(ForegroundProperty); value → SetValueForAllKey + Foreground = brush. The port
        // pushes the direct Foreground (deferred: the per-state resource keys — header), with the null
        // branch discriminated through BindableObject.IsSet (the port's color has no null).
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        const bool color_is_set = bindable != nullptr && bindable->is_property_set("text_color");
        if (color_is_set)
        {
            button.Foreground(wnative::to_brush(view.text_color()));
        }
        else
        {
            button.ClearValue(muxc::Control::ForegroundProperty());
        }
    }

    void button_handler::map_font(button_handler& handler, i_text_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->text_font = view.font();
        auto button = button_of(*platform);
        if (button == nullptr)
        {
            return;
        }
        // ControlExtensions.UpdateFont: FontSize + FontFamily + FontStyle + FontWeight +
        // IsTextScaleFactorEnabled (FontManager.GetFontSize/GetFontFamily — registrar skipped, header).
        const font value = view.font();
        const double size = value.size();
        button.FontSize((size > 0 && !std::isnan(size)) ? size : k_default_font_size);
        if (!value.family().empty())
        {
            button.FontFamily(muxm::FontFamily{wnative::to_hstring_utf8(value.family())});
        }
        else
        {
            button.ClearValue(muxc::Control::FontFamilyProperty()); // C# null Family → the default family
        }
        button.FontStyle(to_font_style(value.slant()));
        button.FontWeight(to_font_weight(value.weight()));
        button.IsTextScaleFactorEnabled(value.auto_scaling_enabled());
    }

    void button_handler::map_character_spacing(button_handler& handler, i_text_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->character_spacing = view.character_spacing();
        // ButtonExtensions.UpdateCharacterSpacing: CharacterSpacing = value.ToEm() (the inner-TextBlock
        // second push is MauiButton-only — header deviation).
        if (auto button = button_of(*platform))
        {
            button.CharacterSpacing(wnative::to_em(view.character_spacing()));
        }
    }

    void button_handler::map_padding(button_handler& handler, i_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->padding = view.padding();
        auto button = button_of(*platform);
        if (button == nullptr)
        {
            return;
        }
        // ButtonExtensions.UpdatePadding(button, GetResource("ButtonPadding")): a NaN (unset) padding
        // falls back to the theme's ButtonPadding — ClearValue restores exactly that style value
        // without needing the resource lookup; an explicit padding is pushed directly.
        const thickness padding = view.padding();
        if (padding.is_nan())
        {
            button.ClearValue(muxc::Control::PaddingProperty());
        }
        else
        {
            button.Padding(wnative::to_thickness(padding));
        }
    }

    void button_handler::map_stroke_color(button_handler& handler, i_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->stroke_color = view.stroke_color();
        auto button = button_of(*platform);
        if (button == nullptr)
        {
            return;
        }
        // ButtonExtensions.UpdateStrokeColor: null → RemoveKeys(ButtonBorderBrush*); value → the
        // border-brush resource keys. The port pushes the direct BorderBrush (deferred: the per-state
        // resource keys — header), null discriminated through BindableObject.IsSet.
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        const bool color_is_set = bindable != nullptr && bindable->is_property_set("stroke_color");
        if (color_is_set)
        {
            button.BorderBrush(wnative::to_brush(view.stroke_color()));
        }
        else
        {
            button.ClearValue(muxc::Control::BorderBrushProperty());
        }
    }

    void button_handler::map_stroke_thickness(button_handler& handler, i_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->stroke_thickness = view.stroke_thickness();
        auto button = button_of(*platform);
        if (button == nullptr)
        {
            return;
        }
        // ButtonExtensions.UpdateStrokeThickness: thickness >= 0 → the ButtonBorderThemeThickness key
        // (a uniform Thickness); negative → RemoveKeys (theme default). Direct BorderThickness here
        // (deferred: the resource key — header).
        const double value = view.stroke_thickness();
        if (value >= 0)
        {
            button.BorderThickness(wnative::to_thickness(thickness{value}));
        }
        else
        {
            button.ClearValue(muxc::Control::BorderThicknessProperty());
        }
    }

    void button_handler::map_corner_radius(button_handler& handler, i_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->corner_radius = view.corner_radius();
        auto button = button_of(*platform);
        if (button == nullptr)
        {
            return;
        }
        // ButtonExtensions.UpdateCornerRadius: radius >= 0 → the ControlCornerRadius key (a uniform
        // CornerRadius); negative → RemoveKeys (theme default). Direct Control.CornerRadius here
        // (deferred: the resource key — header). C#'s follow-up Shadow re-map has no shadow push on
        // this backend yet.
        const int radius = view.corner_radius();
        if (radius >= 0)
        {
            const auto uniform = static_cast<double>(radius);
            button.CornerRadius(mux::CornerRadius{
                .TopLeft = uniform, .TopRight = uniform, .BottomRight = uniform, .BottomLeft = uniform});
        }
        else
        {
            button.ClearValue(muxc::Control::CornerRadiusProperty());
        }
    }

    maui::graphics::size button_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return {0, 0};
        }
        if (platform->native == nullptr)
        {
            // XAML-less degradation: the headless partial's placeholder metric (~8pt/char, 20pt line) so
            // the backend-agnostic size-request suites see consistent numbers (the android twin's shape).
            return {static_cast<double>(platform->title.size()) * 8.0, 20.0};
        }
        // ViewHandlerExtensions.Windows.cs GetDesiredSizeFromHandler: FrameworkElement.Measure +
        // DesiredSize, with the AdjustForExplicitSize clamp fed from the virtual view's explicit
        // width()/height() (the value C#'s MapWidth/MapHeight would have pushed — see measure_native).
        const double explicit_width = virtual_view() != nullptr ? virtual_view()->width() : dimension::unset;
        const double explicit_height = virtual_view() != nullptr ? virtual_view()->height() : dimension::unset;
        return wnative::measure_native(platform->native, width_constraint, height_constraint, explicit_width,
                                       explicit_height);
    }

    // Windows keeps the cross-platform ResolveConstraints clamp (an explicit size request overrides the
    // measured size) — the intrinsic-content floor is an iOS/macOS native-button behavior; the C#
    // Windows lane has no such override. False, like the headless/android twins.
    bool button_handler::content_is_minimum_size() const
    {
        return false;
    }

    void button_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // XAML-less: no native layout to apply
        }
        // The shared Canvas recipe: Canvas.SetLeft/SetTop + explicit Width/Height pin the Button to the
        // frame (ViewHandlerExtensions.Windows.cs PlatformArrangeHandler on the Canvas layout model).
        wnative::arrange_native(platform->native, frame);
    }

    // ---- per-backend image-source primitives (the cross-platform map_image_source routes here) ----
    // deferred: the windows Button's icon rides MauiButton's Image content part
    // (ButtonExtensions.UpdateImageSource) — the stock-Button cut has no image part and no decode
    // pipeline on this backend yet (header deviations). The primitives update the shared
    // headless-style mirrors (kind/file/loaded) so the cross-platform suite still observes the load.

    // deferred: leave the loader on its defaults (no BitmapImage/CanvasImageSource seam yet).
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
} // namespace maui::core
