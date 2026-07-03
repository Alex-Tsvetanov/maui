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
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h> // BitmapImage (the icon decode)
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Text.h>
#include <winrt/base.h>

#include <filesystem>

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
    namespace muxmi = winrt::Microsoft::UI::Xaml::Media::Imaging;
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

    // The MauiButton content parts (Content = StackPanel { Image, TextBlock }). Null when the button ran
    // XAML-less (native == nullptr) — every caller guards on the returned control being non-null.
    [[nodiscard]] muxc::StackPanel panel_of(const maui::core::button_platform& platform)
    {
        return wnative::borrow<muxc::StackPanel>(platform.content_panel);
    }

    [[nodiscard]] muxc::TextBlock text_block_of(const maui::core::button_platform& platform)
    {
        return wnative::borrow<muxc::TextBlock>(platform.text_block);
    }

    [[nodiscard]] muxc::Image image_of(const maui::core::button_platform& platform)
    {
        return wnative::borrow<muxc::Image>(platform.image_element);
    }

    // A BitmapImage over a file:/// URI for a local path (the FileImageSourceService recipe; the same
    // decode the image_handler partial uses for the button's icon). std::filesystem::absolute resolves a
    // relative path against the process cwd (where the gallery exe's assets sit). Null on any failure.
    [[nodiscard]] muxmi::BitmapImage bitmap_from_file(std::string_view file)
    {
        try
        {
            const winrt::hstring wide = wnative::to_hstring_utf8(file);
            std::error_code ec;
            std::filesystem::path absolute =
                std::filesystem::absolute(std::filesystem::path{std::wstring_view{wide}}, ec);
            if (ec)
            {
                absolute = std::filesystem::path{std::wstring_view{wide}};
            }
            const std::wstring uri = L"file:///" + absolute.generic_wstring();
            return muxmi::BitmapImage{winrt::Windows::Foundation::Uri{winrt::hstring{uri}}};
        }
        catch (const winrt::hresult_error&)
        {
            return muxmi::BitmapImage{nullptr};
        }
        catch (const std::exception&)
        {
            return muxmi::BitmapImage{nullptr};
        }
    }

    // A BitmapImage over an already-resolved uri string (the loader's uri lane); a scheme-less detail
    // reuses the file lane (the image_handler convention).
    [[nodiscard]] muxmi::BitmapImage bitmap_from_uri(std::string_view uri)
    {
        if (!uri.contains("://"))
        {
            return bitmap_from_file(uri);
        }
        try
        {
            return muxmi::BitmapImage{winrt::Windows::Foundation::Uri{wnative::to_hstring_utf8(uri)}};
        }
        catch (const winrt::hresult_error&)
        {
            return muxmi::BitmapImage{nullptr};
        }
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
        // The MauiButton content parts each hold their own strong ref (the Button also keeps them alive
        // via Content → the panel's Children; releasing here mirrors the label partial's Border+TextBlock).
        wnative::release(image_element);
        wnative::release(text_block);
        wnative::release(content_panel);
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
            // ButtonHandler.CreatePlatformView: new MauiButton(), whose Content is a DefaultMauiButtonContent
            // (a MauiPanel composing an Image + TextBlock). The WinUI-native twin: a centered StackPanel
            // holding a Uniform-stretch Image (icon) and a centered TextBlock (title), both collapsed until
            // their mapper supplies a value — so a text-only button shows just the text and an icon+text
            // button lays them out per ContentLayout (map_content_layout drives orientation/order/spacing).
            const muxc::Button button;

            const muxc::Image image;
            image.Stretch(muxm::Stretch::Uniform);
            image.HorizontalAlignment(mux::HorizontalAlignment::Center);
            image.VerticalAlignment(mux::VerticalAlignment::Center);
            image.Visibility(mux::Visibility::Collapsed);

            const muxc::TextBlock block;
            block.HorizontalAlignment(mux::HorizontalAlignment::Center);
            block.VerticalAlignment(mux::VerticalAlignment::Center);
            block.TextAlignment(mux::TextAlignment::Center);
            block.Visibility(mux::Visibility::Collapsed);

            const muxc::StackPanel panel;
            panel.HorizontalAlignment(mux::HorizontalAlignment::Center);
            panel.VerticalAlignment(mux::VerticalAlignment::Center);
            panel.Orientation(muxc::Orientation::Horizontal); // ContentLayout default is Left
            panel.Children().Append(image);                   // image-first (Left/Top); reordered by ContentLayout
            panel.Children().Append(block);
            button.Content(panel);

            platform->native = wnative::store(button);       // released in ~button_platform
            platform->content_panel = wnative::store(panel); // the three parts each keep their own ref
            platform->text_block = wnative::store(block);
            platform->image_element = wnative::store(image);
        }
        catch (const winrt::hresult_error&)
        {
            platform->native = nullptr; // XAML-less degradation (header note)
            platform->content_panel = nullptr;
            platform->text_block = nullptr;
            platform->image_element = nullptr;
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
        // ButtonExtensions.UpdateText writes the MauiButton's inner TextBlock and collapses it when the
        // text is empty (so a naked-icon button reclaims the text's space + spacing — MauiButton's
        // AdjustSpacing). The re-apply keeps the StackPanel's spacing in sync with that visibility flip.
        if (auto block = text_block_of(*platform))
        {
            const bool has_text = !view.text().empty();
            block.Text(wnative::to_hstring_utf8(view.text()));
            block.Visibility(has_text ? mux::Visibility::Visible : mux::Visibility::Collapsed);
            apply_content_layout_native(*platform, {static_cast<button_content_spec::image_position>(
                                                        platform->content_layout_position),
                                                    platform->content_layout_spacing});
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
        auto block = text_block_of(*platform);
        if (color_is_set)
        {
            button.Foreground(wnative::to_brush(view.text_color()));
            if (block)
            {
                block.Foreground(wnative::to_brush(view.text_color()));
            }
        }
        else
        {
            button.ClearValue(muxc::Control::ForegroundProperty());
            if (block)
            {
                block.ClearValue(muxc::TextBlock::ForegroundProperty());
            }
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
        const double resolved_size = (size > 0 && !std::isnan(size)) ? size : k_default_font_size;
        button.FontSize(resolved_size);
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
        // Mirror onto the composed TextBlock — ControlExtensions.UpdateFont writes the MauiButton content's
        // text part so the glyphs render at the mapped size/family/style/weight.
        if (auto block = text_block_of(*platform))
        {
            block.FontSize(resolved_size);
            if (!value.family().empty())
            {
                block.FontFamily(muxm::FontFamily{wnative::to_hstring_utf8(value.family())});
            }
            else
            {
                block.ClearValue(muxc::TextBlock::FontFamilyProperty());
            }
            block.FontStyle(to_font_style(value.slant()));
            block.FontWeight(to_font_weight(value.weight()));
            block.IsTextScaleFactorEnabled(value.auto_scaling_enabled());
        }
    }

    void button_handler::map_character_spacing(button_handler& handler, i_text_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->character_spacing = view.character_spacing();
        // ButtonExtensions.UpdateCharacterSpacing: CharacterSpacing = value.ToEm(), pushed to BOTH the
        // Button and its inner TextBlock — the TextBlock is where the visible glyph spacing lands (the
        // Control-level value alone does not reach the composed content).
        const auto em = wnative::to_em(view.character_spacing());
        if (auto button = button_of(*platform))
        {
            button.CharacterSpacing(em);
        }
        if (auto block = text_block_of(*platform))
        {
            block.CharacterSpacing(em);
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
    // The windows Button's icon rides the MauiButton content's Image part (ButtonExtensions.UpdateImageSource):
    // a decoded BitmapImage lands on the StackPanel's Image and makes it Visible; a cleared source collapses
    // it. After each visibility flip the ContentLayout is re-applied so AdjustSpacing tracks the change. The
    // shared mirrors (kind/file/loaded) are still maintained for the XAML-less cross-platform suite.

    // The loader keeps its defaults: the file fast-path decodes synchronously below, and uri results arrive
    // through apply_loaded_result — both over the BitmapImage-over-Uri lane (no CanvasImageSource needed).
    void button_handler::configure_loader(maui::core::image_source_loader& /*loader*/)
    {
    }

    void button_handler::load_file_source_sync(button_platform& platform, const i_file_image_source& file_src)
    {
        platform.source_kind = "file";
        platform.source_file = std::string(file_src.file());
        platform.source_loaded = true;
        if (auto image = image_of(platform))
        {
            if (const auto bitmap = bitmap_from_file(file_src.file()))
            {
                image.Source(bitmap);
                image.Visibility(mux::Visibility::Visible);
            }
            else
            {
                image.ClearValue(muxc::Image::SourceProperty());
                image.Visibility(mux::Visibility::Collapsed);
            }
            apply_content_layout_native(
                platform, {static_cast<button_content_spec::image_position>(platform.content_layout_position),
                           platform.content_layout_spacing});
        }
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
        if (auto image = image_of(platform))
        {
            // uri/file loader results decode over the same BitmapImage-over-Uri lane (detail = path/uri).
            if (const auto bitmap = bitmap_from_uri(result.detail()))
            {
                image.Source(bitmap);
                image.Visibility(mux::Visibility::Visible);
            }
            else
            {
                image.ClearValue(muxc::Image::SourceProperty());
                image.Visibility(mux::Visibility::Collapsed);
            }
            apply_content_layout_native(
                platform, {static_cast<button_content_spec::image_position>(platform.content_layout_position),
                           platform.content_layout_spacing});
        }
    }

    void button_handler::clear_source_native(button_platform& platform)
    {
        platform.source_kind.clear();
        platform.source_file.clear();
        platform.source_loaded = false;
        if (auto image = image_of(platform))
        {
            image.ClearValue(muxc::Image::SourceProperty());
            image.Visibility(mux::Visibility::Collapsed);
            apply_content_layout_native(
                platform, {static_cast<button_content_spec::image_position>(platform.content_layout_position),
                           platform.content_layout_spacing});
        }
    }

    // ButtonExtensions.UpdateContentLayout: orient the StackPanel along the image axis (Left/Right →
    // Horizontal; Top/Bottom → Vertical), order the parts (image-first for Left/Top, text-first for
    // Right/Bottom), and set the inter-part spacing — collapsed to 0 when either part is hidden (MauiButton's
    // AdjustSpacing). A no-op when the button ran XAML-less (parts null).
    void button_handler::apply_content_layout_native(button_platform& platform, maui::core::button_content_spec spec)
    {
        platform.content_layout_position = static_cast<int>(spec.position);
        platform.content_layout_spacing = spec.spacing;
        auto panel = panel_of(platform);
        auto image = image_of(platform);
        auto block = text_block_of(platform);
        if (!panel || !image || !block)
        {
            return;
        }
        using pos = button_content_spec::image_position;
        const bool horizontal = spec.position == pos::left || spec.position == pos::right;
        const bool image_first = spec.position == pos::left || spec.position == pos::top;
        panel.Orientation(horizontal ? muxc::Orientation::Horizontal : muxc::Orientation::Vertical);
        // Reorder the two children in place (they stay pinned by the platform's own strong refs).
        panel.Children().Clear();
        if (image_first)
        {
            panel.Children().Append(image);
            panel.Children().Append(block);
        }
        else
        {
            panel.Children().Append(block);
            panel.Children().Append(image);
        }
        const bool both_visible = image.Visibility() == mux::Visibility::Visible &&
                                  block.Visibility() == mux::Visibility::Visible;
        panel.Spacing(both_visible ? spec.spacing : 0.0);
    }
} // namespace maui::core
