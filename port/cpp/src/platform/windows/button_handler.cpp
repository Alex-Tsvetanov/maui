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
#include <winrt/Windows.Foundation.h>

#include <algorithm>
#include <cmath>
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

    // "Was this property explicitly set?" — the port's equivalent of C#'s `Color?` being non-null. It has
    // to go through bindable_object: the i_* view CONTRACTS carry values, not set-ness. Comparing the
    // value against a default instead is the bug in [[cpp-unset-color-sentinel-collision]] — it misreads
    // an explicit Black as unset, and the control loses the developer's color in dark mode.
    bool is_set(const maui::core::i_view& view, std::string_view property)
    {
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        return bindable != nullptr && bindable->is_property_set(property);
    }

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
    button_platform::~button_platform()
    {
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
        platform.on_click = [this] {
            if (auto* view = virtual_view())
            {
                view->send_released();
                view->send_clicked();
            }
        };
        if (platform.native == nullptr)
        {
            return;
        }
        // Then the native half: Click + the two pointer events, exactly ButtonHandler.Windows.ConnectHandler.
        // The tokens are kept so disconnect can revoke them — these lambdas capture `this`.
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
        platform.pointer_pressed_token = button
                                             .PointerPressed([self](const winrt::Windows::Foundation::IInspectable&,
                                                                    const winui::Input::PointerRoutedEventArgs&) {
                                                 if (self->on_press)
                                                 {
                                                     self->on_press();
                                                 }
                                             })
                                             .value;
        // NOTE: on_click already sends Released before Clicked (the shared contract above), so the
        // pointer-released hook must NOT also call on_release or a plain click would report two releases.
        // It exists for the release-outside-the-button case, which raises no Click.
        platform.pointer_released_token = button
                                              .PointerReleased([self](const winrt::Windows::Foundation::IInspectable&,
                                                                      const winui::Input::PointerRoutedEventArgs&) {
                                                  if (self->on_release)
                                                  {
                                                      self->on_release();
                                                  }
                                              })
                                              .value;
    }

    void button_handler::on_disconnect_handler(button_platform& platform)
    {
        if (platform.native != nullptr)
        {
            const button_control button = as_button(platform.native);
            button.Click(winrt::event_token{platform.click_token});
            button.PointerPressed(winrt::event_token{platform.pointer_pressed_token});
            button.PointerReleased(winrt::event_token{platform.pointer_released_token});
        }
        platform.click_token = 0;
        platform.pointer_pressed_token = 0;
        platform.pointer_released_token = 0;
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
            button.ClearValue(winui::Controls::Control::ForegroundProperty());
            return;
        }
        button.Foreground(winui::Media::SolidColorBrush{maui::platform::windows::to_ui_color(platform->text_color)});
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
        as_button(platform->native).CharacterSpacing(em);
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
        const button_control button = as_button(platform->native);
        button.Measure(winrt::Windows::Foundation::Size{static_cast<float>(width_constraint),
                                                        static_cast<float>(height_constraint)});
        const auto desired = button.DesiredSize();
        return {desired.Width, desired.Height};
    }

    bool button_handler::content_is_minimum_size() const
    {
        // TRUE, unlike headless: a WinUI Button has a real intrinsic minimum (its content plus the theme's
        // ButtonPadding), and MAUI's Windows render shows the same floor the iOS one does — a WidthRequest
        // smaller than the natural width does not shrink the chrome. See [[cpp-clipping-button-natural-floor]].
        return true;
    }

    void button_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
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
} // namespace maui::core
