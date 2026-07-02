// radio_button_handler â€” Windows (WinUI 3) platform partial: a REAL
// Microsoft.UI.Xaml.Controls.RadioButton. The windows twin of
// src/platform/apple/radio_button_handler.mm (NSButtonTypeRadio) / the android JNI partial, and the
// real-native sibling of the headless mirror partial (src/platform/headless/radio_button_handler.cpp).
// Every map_* keeps the headless mirror AND pushes onto the control; the native Checked/Unchecked
// events flow back through i_radio_button::send_is_checked (C#'s single OnCheckedOrUnchecked
// write-back).
//
// Ported DIRECTLY from RadioButtonHandler.Windows.cs + Platform/Windows/{RadioButtonExtensions.cs
// (UpdateIsChecked / UpdateBackground / UpdateTextColor / UpdateContent / UpdateStrokeColor /
// UpdateStrokeThickness / UpdateCornerRadius), ControlExtensions.cs (UpdateFont /
// UpdateCharacterSpacing / UpdateIsEnabled), ViewExtensions.cs, FontExtensions.cs} +
// Fonts/FontManager.Windows.cs.
//
// Group behavior: CreatePlatformView sets a RANDOM GUID as the native GroupName â€” C#'s work-around
// for https://github.com/dotnet/maui/issues/11418 â€” so every native RadioButton sits in its OWN
// native group and WinUI's built-in exclusion never fights the cross-platform RadioButtonGroup, which
// stays authoritative (exactly as the apple partial refuses the NSButton sibling-action exclusion).
// No further native GroupName wiring exists in C# and none is invented here.
//
// DOCUMENTED DEVIATIONS (each an infrastructure gap of this first cut, not a behavior guess):
//   - Content: C#'s UpdateContent presents an IView PresentedContent via ToPlatform; the port's
//     radio_button contract is string-content only (see i_radio_button.hpp), so Content is the boxed
//     string â€” exactly C#'s `platformRadioButton.Content = $"{radioButton.Content}"` else-branch. The
//     templated-content path is deferred with the templates layer.
//   - Background / text color / stroke color land on the DIRECT dependency properties (Background /
//     Foreground / BorderBrush). C# writes ONLY the themed resource keys (RadioButtonBackground* /
//     RadioButtonForeground* / RadioButtonBorderBrush* + RefreshThemeResources) â€” deferred: the
//     resource-dictionary seam; the direct property covers the rest state (the button partial's
//     identical cut). C#'s null branches (RemoveKeys) map to ClearValue, discriminated through
//     BindableObject.IsSet where the port's color value type has no null.
//   - StrokeThickness / CornerRadius are DIRECT dependency properties in C# too
//     (WinUIHelpers.CreateThickness / CreateCornerRadius) and are pushed verbatim, including the
//     negative-thickness â†’ 0 guard.
//   - The Unchecked event's write-back leg: C#'s OnCheckedOrUnchecked writes the native IsChecked
//     state on BOTH events. With the unique-GroupName isolation a WinUI radio never self-unchecks
//     from user input â€” the Unchecked event only echoes a programmatic map_is_checked(false) push â€”
//     so both events route into the same lambda that reads the native state, exactly like C#.
//   - FontManager's registrar lookups are skipped (no font registrar in the port yet): a named family
//     goes straight to FontFamily(name); the default size is the constant 14
//     (ControlContentThemeFontSize). Byte-for-byte the button/label partials' map_font.
//
// XAML-less degradation (the windows twin of the android VM-less fallback): the windows preset also
// runs the cross-platform suite on the host where NO XAML runtime exists; create_platform_view
// catches the construction failure and keeps native null, while the headless mirrors are ALWAYS
// maintained and on_select stays an invokable C++ callback (the cross-platform suite drives it) â€” so
// that suite observes exactly the headless partial's behavior.

#include "maui/core/radio_button_handler.hpp"

#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h> // ToggleButton: RadioButton's IsChecked/Checked/Unchecked live on the projected base
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Text.h>
#include <winrt/base.h>

#include "maui/core/bindable_object.hpp"
#include "maui/core/dimension.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_radio_button.hpp"
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
    namespace muxm = winrt::Microsoft::UI::Xaml::Media;
    namespace wut = winrt::Windows::UI::Text;
    namespace wnative = maui::platform::win;

    // FontManager.Windows.DefaultFontSize â€” the ControlContentThemeFontSize theme resource (14.0);
    // read as a constant here (no Application.Current on the XAML-less test host).
    constexpr double k_default_font_size = 14.0;

    [[nodiscard]] muxc::RadioButton radio_of(const maui::core::radio_button_platform& platform)
    {
        return wnative::borrow<muxc::RadioButton>(platform.native);
    }

    // FontExtensions.ToFontStyle: Slant â†’ FontStyle (Italic / Oblique / Normal).
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

    // FontExtensions.ToFontWeight â€” the port's font_weight enum values ARE the numeric OpenType
    // weights C#'s switch resolves to, so the numeric FontWeight constructor is the whole mapping.
    [[nodiscard]] wut::FontWeight to_font_weight(maui::core::font_weight weight)
    {
        return wut::FontWeight{static_cast<std::uint16_t>(weight)};
    }
} // namespace

namespace maui::core
{
    // Releases the one strong ref pinning the RadioButton (the wnative shape of the pimpl-owned-native
    // doctrine; the apple twin CFReleases its radio NSButton here). Event tokens are revoked in
    // on_disconnect_handler; the dtor only drops the ref.
    radio_button_platform::~radio_button_platform()
    {
        wnative::release(native);
    }

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each
    // calls the base body FIRST â€” the headless mirrors must stay live for the XAML-less cross-platform
    // suite (header note) â€” then pushes to the real RadioButton when one exists.

    void radio_button_platform::update_visibility(maui::core::visibility value)
    {
        view_platform_base::update_visibility(value);
        // ViewExtensions.UpdateVisibility (Windows): Hidden rides Opacity 0, Collapsed collapses.
        wnative::apply_visibility(native, value, alpha);
    }

    void radio_button_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        // ViewExtensions.UpdateOpacity: the Hidden state pins opacity 0 (apply_visibility restores it).
        if (!hidden)
        {
            wnative::apply_opacity(native, value);
        }
    }

    void radio_button_platform::update_is_enabled(bool value)
    {
        view_platform_base::update_is_enabled(value);
        // ViewExtensions.UpdateIsEnabled â†’ ControlExtensions.UpdateIsEnabled: Control.IsEnabled.
        if (auto radio = radio_of(*this))
        {
            radio.IsEnabled(value);
        }
    }

    void radio_button_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        // ViewExtensions.UpdateAutomationId: AutomationProperties.SetAutomationId.
        wnative::apply_automation_id(native, value);
    }

    void radio_button_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        auto radio = radio_of(*this);
        if (radio == nullptr)
        {
            return;
        }
        // RadioButtonHandler.Windows.MapBackground â†’ RadioButtonExtensions.UpdateBackground: ONLY a
        // SolidPaint is pushed (through the RadioButtonBackground* resource keys +
        // RefreshThemeResources); any other paint â€” including null â€” is a C# no-op. The port lands the
        // solid on the DIRECT Background property (deferred: the per-state resource keys â€” header).
        if (const auto* solid = dynamic_cast<const maui::graphics::solid_paint*>(value))
        {
            radio.Background(wnative::to_brush(solid->color()));
        }
    }

    std::unique_ptr<radio_button_platform> radio_button_handler::create_platform_view()
    {
        auto platform = std::make_unique<radio_button_platform>();
        try
        {
            // RadioButtonHandler.Windows.CreatePlatformView: new RadioButton { GroupName =
            // Guid.NewGuid().ToString() } â€” the dotnet/maui#11418 work-around isolating every native
            // radio in its own native group so the cross-platform RadioButtonGroup owns the exclusion
            // (header note; the braced winrt GUID rendering differs from C#'s "D" format only
            // cosmetically â€” uniqueness is the whole contract).
            const muxc::RadioButton radio;
            radio.GroupName(winrt::to_hstring(winrt::Windows::Foundation::GuidHelper::CreateNewGuid()));
            platform->native = wnative::store(radio); // released in ~radio_button_platform
        }
        catch (const winrt::hresult_error&)
        {
            platform->native = nullptr; // XAML-less degradation (header note)
        }
        return platform;
    }

    void radio_button_handler::on_connect_handler(radio_button_platform& platform)
    {
        // The portable inbound channel, wired exactly like the headless twin (the cross-platform suite
        // drives it XAML-less): RadioButton.SelectRadioButton â€” a native tap checks the button
        // (from-handler write); the group's mutual exclusion unchecks the others at the Controls layer.
        platform.on_select = [this] {
            if (auto* view = virtual_view())
            {
                view->send_is_checked(true);
            }
        };
        auto radio = radio_of(platform);
        if (radio == nullptr)
        {
            return;
        }
        // ConnectHandler: Checked += OnCheckedOrUnchecked; Unchecked += OnCheckedOrUnchecked. C#'s one
        // instance method serves both events, reading the CURRENT native state:
        // VirtualView.IsChecked = PlatformView.IsChecked == true. The lambda captures `this` (+ the
        // platform peer for the native read); on_disconnect_handler revokes both tokens before the
        // handler/platform die, so the captures never dangle. The Unchecked leg only ever echoes a
        // programmatic push here (header deviations).
        auto* peer = &platform;
        const auto checked_or_unchecked = [this, peer](const winrt::Windows::Foundation::IInspectable&,
                                                       const mux::RoutedEventArgs&) {
            auto* view = virtual_view();
            auto sender = radio_of(*peer);
            if (view == nullptr || sender == nullptr)
            {
                return;
            }
            const auto native_checked = sender.IsChecked(); // IReference<bool> (a nullable bool)
            view->send_is_checked(native_checked != nullptr && native_checked.Value());
        };
        platform.checked_token = radio.Checked(checked_or_unchecked).value;
        platform.unchecked_token = radio.Unchecked(checked_or_unchecked).value;
    }

    void radio_button_handler::on_disconnect_handler(radio_button_platform& platform)
    {
        // DisconnectHandler: Checked/Unchecked -= OnCheckedOrUnchecked. The C++ callback is cleared
        // like the headless twin.
        platform.on_select = nullptr;
        if (auto radio = radio_of(platform))
        {
            if (platform.checked_token != 0)
            {
                radio.Checked(winrt::event_token{platform.checked_token});
            }
            if (platform.unchecked_token != 0)
            {
                radio.Unchecked(winrt::event_token{platform.unchecked_token});
            }
        }
        platform.checked_token = 0;
        platform.unchecked_token = 0;
    }

    void radio_button_handler::map_is_checked(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->is_checked = view.is_checked(); // headless mirror
        if (auto radio = radio_of(*platform))
        {
            // RadioButtonExtensions.UpdateIsChecked: IsChecked = radioButton.IsChecked.
            radio.IsChecked(view.is_checked());
        }
    }

    void radio_button_handler::map_content(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->content = std::string(view.content_as_string()); // headless mirror
        if (auto radio = radio_of(*platform))
        {
            // RadioButtonExtensions.UpdateContent's string branch: Content = $"{radioButton.Content}"
            // (the IView PresentedContent branch is deferred with the templates layer â€” header).
            radio.Content(winrt::box_value(wnative::to_hstring_utf8(view.content_as_string())));
        }
    }

    void radio_button_handler::map_text_color(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->text_color = view.text_color(); // headless mirror
        auto radio = radio_of(*platform);
        if (radio == nullptr)
        {
            return;
        }
        // RadioButtonExtensions.UpdateTextColor: null â†’ RemoveKeys(RadioButtonForeground*); value â†’
        // SetValueForAllKey (+ RefreshThemeResources). The port pushes the direct Foreground
        // (deferred: the per-state resource keys â€” header), with the null branch mapped to ClearValue,
        // discriminated through BindableObject.IsSet (the port's color has no null).
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        const bool color_is_set = bindable != nullptr && bindable->is_property_set("text_color");
        if (color_is_set)
        {
            radio.Foreground(wnative::to_brush(view.text_color()));
        }
        else
        {
            radio.ClearValue(muxc::Control::ForegroundProperty());
        }
    }

    void radio_button_handler::map_character_spacing(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->character_spacing = view.character_spacing(); // headless mirror
        if (auto radio = radio_of(*platform))
        {
            // RadioButtonHandler.MapCharacterSpacing â†’ ControlExtensions.UpdateCharacterSpacing:
            // CharacterSpacing = value.ToEm().
            radio.CharacterSpacing(wnative::to_em(view.character_spacing()));
        }
    }

    void radio_button_handler::map_font(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->text_font = view.font(); // headless mirror
        auto radio = radio_of(*platform);
        if (radio == nullptr)
        {
            return;
        }
        // RadioButtonHandler.MapFont â†’ ControlExtensions.UpdateFont: FontSize + FontFamily + FontStyle
        // + FontWeight + IsTextScaleFactorEnabled (FontManager.GetFontSize/GetFontFamily â€” registrar
        // skipped, header).
        const font value = view.font();
        const double size = value.size();
        radio.FontSize((size > 0 && !std::isnan(size)) ? size : k_default_font_size);
        if (!value.family().empty())
        {
            radio.FontFamily(muxm::FontFamily{wnative::to_hstring_utf8(value.family())});
        }
        else
        {
            radio.ClearValue(muxc::Control::FontFamilyProperty()); // C# null Family â†’ the default family
        }
        radio.FontStyle(to_font_style(value.slant()));
        radio.FontWeight(to_font_weight(value.weight()));
        radio.IsTextScaleFactorEnabled(value.auto_scaling_enabled());
    }

    void radio_button_handler::map_stroke_color(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->stroke_color = view.stroke_color(); // headless mirror
        auto radio = radio_of(*platform);
        if (radio == nullptr)
        {
            return;
        }
        // RadioButtonExtensions.UpdateStrokeColor: null â†’ RemoveKeys(RadioButtonBorderBrush*); value â†’
        // SetValueForAllKey (+ RefreshThemeResources). The port pushes the direct BorderBrush
        // (deferred: the per-state resource keys â€” header), null discriminated through
        // BindableObject.IsSet.
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        const bool color_is_set = bindable != nullptr && bindable->is_property_set("stroke_color");
        if (color_is_set)
        {
            radio.BorderBrush(wnative::to_brush(view.stroke_color()));
        }
        else
        {
            radio.ClearValue(muxc::Control::BorderBrushProperty());
        }
    }

    void radio_button_handler::map_stroke_thickness(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->stroke_thickness = view.stroke_thickness(); // headless mirror
        if (auto radio = radio_of(*platform))
        {
            // RadioButtonExtensions.UpdateStrokeThickness: a negative value defaults to zero (WinUI
            // throws on a negative Thickness), else the uniform thickness â€” verbatim.
            const double value = view.stroke_thickness();
            radio.BorderThickness(wnative::to_thickness(thickness{value < 0 ? 0.0 : value}));
        }
    }

    void radio_button_handler::map_corner_radius(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->corner_radius = view.corner_radius(); // headless mirror
        if (auto radio = radio_of(*platform))
        {
            // RadioButtonExtensions.UpdateCornerRadius: CornerRadius =
            // WinUIHelpers.CreateCornerRadius(radioButton.CornerRadius) â€” a uniform radius, pushed
            // verbatim (no negative guard in C#).
            const auto uniform = static_cast<double>(view.corner_radius());
            radio.CornerRadius(mux::CornerRadius{
                .TopLeft = uniform, .TopRight = uniform, .BottomRight = uniform, .BottomLeft = uniform});
        }
    }

    maui::graphics::size radio_button_handler::get_desired_size(double width_constraint,
                                                                double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return {0, 0};
        }
        if (platform->native == nullptr)
        {
            // XAML-less degradation: the headless partial's metric â€” no native control, no intrinsic
            // content size to report.
            return {0, 0};
        }
        // ViewHandlerExtensions.Windows.cs GetDesiredSizeFromHandler: FrameworkElement.Measure +
        // DesiredSize, with the AdjustForExplicitSize clamp fed from the virtual view's explicit
        // width()/height() (see measure_native).
        const double explicit_width = virtual_view() != nullptr ? virtual_view()->width() : dimension::unset;
        const double explicit_height = virtual_view() != nullptr ? virtual_view()->height() : dimension::unset;
        return wnative::measure_native(platform->native, width_constraint, height_constraint, explicit_width,
                                       explicit_height);
    }

    void radio_button_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // XAML-less: no native layout to apply
        }
        // The shared Canvas recipe: Canvas.SetLeft/SetTop + explicit Width/Height pin the RadioButton
        // to the frame (ViewHandlerExtensions.Windows.cs PlatformArrangeHandler on the Canvas layout
        // model).
        wnative::arrange_native(platform->native, frame);
    }
} // namespace maui::core
