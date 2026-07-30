// radio_button_handler — WinUI 3 platform recipe: a real Microsoft.UI.Xaml.Controls.RadioButton, the
// same native type RadioButtonHandler.Windows.cs creates. Ported from RadioButtonHandler.Windows.cs +
// RadioButtonExtensions.cs + ControlExtensions.cs (the generic Control.UpdateFont/UpdateCharacterSpacing
// this handler's Mapper resolves to for Font/CharacterSpacing, since RadioButtonExtensions has no
// override for either).
//
// DOCUMENTED SIMPLIFICATIONS / NOTES against C#, each narrowed on purpose rather than left vague:
//
//  1. GroupName. C# assigns `GroupName = Guid.NewGuid().ToString()` in CreatePlatformView so WinUI's own
//     native mutual-exclusion (radio buttons sharing a GroupName toggle each other off) NEVER engages —
//     the cross-platform radio_button_group (Controls layer) is the sole source of exclusion, matching
//     every other backend (see the header's deviation note + https://github.com/dotnet/maui/issues/11418).
//     The exact STRING is never read back by anything — only uniqueness matters — so this file uses a
//     monotonic per-process counter instead of a real GUID (CoCreateGuid/Rpcrt4 would work identically but
//     pulls in a COM dependency for a value nothing ever parses).
//  2. Background / TextColor / StrokeColor go through RESOURCE-KEY overrides, not a plain property push —
//     see the header's update_background note for Background. TextColor and StrokeColor are narrower
//     still: RadioButtonExtensions.UpdateTextColor/UpdateStrokeColor call ONLY UpdateColors(...) +
//     RefreshThemeResources() — UNLIKE button_handler.cpp/picker_handler.cpp/time_picker_handler.cpp's
//     identical-looking map_text_color, there is NO direct Foreground/BorderBrush push alongside the
//     resource keys here. That is not a narrowing — it is exactly what RadioButtonExtensions.cs:35-48 and
//     :60-73 do; the plain property alone would be silently overwritten by the control template on the
//     next visual-state change (hover/press/disable) since the template binds to the resource keys, not
//     the property, so setting BOTH (as the other controls do) would not be wrong either — this just
//     mirrors the oracle exactly as written.
//  3. Background's oracle (UpdateBackground) is gated on `button.Background is SolidPaint solidPaint` with
//     NO else branch — a null/gradient/pattern Background updates nothing at all (any earlier resource
//     override is left in place). That looks like an omission but is exactly RadioButtonExtensions.cs:
//     25-33; ported faithfully rather than "fixed" to also clear on null (contrast with
//     time_picker_handler.cpp's push_background, whose oracle DOES have a null-clears branch).
//  4. IsTextScaleFactorEnabled (ControlExtensions.UpdateFont's last line) is NOT pushed. This backend has
//     no owned content TextBlock for RadioButton (Content is a plain boxed string, not a child this
//     handler tracks — unlike button_handler.cpp's DefaultMauiButtonContent TextBlock), and the majority
//     precedent in this backend (picker/time_picker/date_picker/entry/editor) treats this property as
//     TextBlock-only and skips it on a bare Control — even though search_bar_handler.cpp pushes it
//     directly onto its AutoSuggestBox (a Control) and documents that as correct. The two precedents
//     disagree; this file follows the majority (skip) as the conservative default. Revisit if a capture
//     shows a font-scaling diff traceable to this property.
//  5. platform_arrange SHRINK-WRAPS the width (does not pin Width to the full arrange frame) — a BEST-
//     EFFORT judgment call, NOT verified against a compiler or a capture (this control has never rendered
//     natively on Windows before). RadioButtonHandler.Windows.cs's CreatePlatformView sets no
//     HorizontalAlignment override (unlike MauiButton.cs, which force-sets Stretch because Button's own
//     native default is not Stretch — see date_picker_handler.cpp's platform_arrange for that citation),
//     and a toggle-plus-label control (RadioButton/CheckBox) hugging its own content rather than filling a
//     wide row is the universal native-toolkit default, matching the same shrink-wrap shape already
//     confirmed for CalendarDatePicker/TimePicker on this backend. If a capture instead shows the Windows
//     RadioButton painting a full-width Background/BorderBrush, this call was wrong — flip to the
//     pin-the-frame pattern button_handler.cpp/picker_handler.cpp use instead (see that comment below).

#include "maui/core/radio_button_handler.hpp"

// RadioButton inherits IsChecked/Checked/Unchecked from ToggleButton, which lives in
// Controls.Primitives — the C++/WinRT include rule again (see picker_handler.cpp's identical note for
// ComboBox/Selector): without the FULL header those inherited members are only forward-declared and
// every use is C3779, an error naming neither the header nor the concept.
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <span>
#include <string>
#include <string_view>

#include "maui/core/bindable_object.hpp"
#include "maui/core/i_radio_button.hpp"
#include "maui/core/view_chrome_ops.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "winui_interop.hpp"
#include "winui_visual_ops.hpp"

namespace
{
    // Named `winui`, NOT `xaml`: the port already has a maui::xaml namespace (the XAML loader), and
    // inside namespace maui::* that name WINS over a file-scope alias — see button_handler.cpp's
    // identical note.
    namespace winui = winrt::Microsoft::UI::Xaml;
    using radio_button_control = winui::Controls::RadioButton;

    radio_button_control as_radio(void* native)
    {
        return maui::platform::windows::ref<winui::UIElement>(native).as<radio_button_control>();
    }

    // File-top note 1: only uniqueness matters, not the string's format.
    winrt::hstring unique_group_name()
    {
        static std::atomic<std::uint64_t> counter{0};
        return winrt::hstring{L"maui_radio_group_" + std::to_wstring(counter.fetch_add(1, std::memory_order_relaxed))};
    }

    // RadioButtonExtensions.cs's three resource-key sets — see button_handler.cpp's identical
    // k_text_color_keys pattern for WHY a local Foreground/Background/BorderBrush alone is insufficient:
    // the control template binds per-visual-state brushes to these theme resources, so an unset override
    // is dropped again the moment the pointer enters/leaves/presses/disables the control.
    constexpr std::array<std::wstring_view, 4> k_background_keys{
        L"RadioButtonBackground", L"RadioButtonBackgroundPointerOver", L"RadioButtonBackgroundPressed",
        L"RadioButtonBackgroundDisabled"};
    constexpr std::array<std::wstring_view, 4> k_foreground_keys{
        L"RadioButtonForeground", L"RadioButtonForegroundPointerOver", L"RadioButtonForegroundPressed",
        L"RadioButtonForegroundDisabled"};
    constexpr std::array<std::wstring_view, 4> k_border_keys{
        L"RadioButtonBorderBrush", L"RadioButtonBorderBrushPointerOver", L"RadioButtonBorderBrushPressed",
        L"RadioButtonBorderBrushDisabled"};

    void set_resources(const radio_button_control& radio, std::span<const std::wstring_view> keys,
                       const winui::Media::Brush& brush)
    {
        for (const auto& key : keys)
        {
            radio.Resources().Insert(winrt::box_value(winrt::hstring{key}), brush);
        }
    }

    void remove_resources(const radio_button_control& radio, std::span<const std::wstring_view> keys)
    {
        for (const auto& key : keys)
        {
            radio.Resources().Remove(winrt::box_value(winrt::hstring{key}));
        }
    }

    // FrameworkElementExtensions.RefreshThemeResources: flip RequestedTheme away and back so the control
    // template re-resolves the resources just overridden.
    void refresh_theme_resources(const winui::FrameworkElement& element)
    {
        const auto previous = element.RequestedTheme();
        element.RequestedTheme(element.ActualTheme() == winui::ElementTheme::Dark ? winui::ElementTheme::Light
                                                                                  : winui::ElementTheme::Dark);
        element.RequestedTheme(previous);
    }

    // "Was this property explicitly set?" — see button_handler.cpp/picker_handler.cpp's identical twin
    // for why this must not be a value comparison ([[cpp-unset-color-sentinel-collision]]).
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
    namespace
    {
        // Unhook everything on_connect_handler registered. Called from on_disconnect_handler AND from
        // ~radio_button_platform, same rationale as button_handler.cpp's detach_native_events: the
        // callback captures the handler, so an undisconnected teardown must not leave it subscribed.
        void detach_native_events(radio_button_platform& platform)
        {
            if (platform.native != nullptr)
            {
                const radio_button_control radio = as_radio(platform.native);
                radio.Checked(winrt::event_token{platform.checked_token});
                radio.Unchecked(winrt::event_token{platform.unchecked_token});
            }
            platform.checked_token = 0;
            platform.unchecked_token = 0;
        }
    } // namespace

    radio_button_platform::~radio_button_platform()
    {
        detach_native_events(*this);
        if (native != nullptr)
        {
            maui::platform::windows::drop<winui::UIElement>(native);
        }
    }

    std::unique_ptr<radio_button_platform> radio_button_handler::create_platform_view()
    {
        auto platform = std::make_unique<radio_button_platform>();
        radio_button_control radio;
        // File-top note 1: the GroupName work-around.
        radio.GroupName(unique_group_name());
        platform->native = maui::platform::windows::take<winui::UIElement>(radio);
        return platform;
    }

    void radio_button_handler::on_connect_handler(radio_button_platform& platform)
    {
        if (platform.native == nullptr)
        {
            return;
        }
        auto* self = this;
        const radio_button_control radio = as_radio(platform.native);
        // OnCheckedOrUnchecked (RadioButtonHandler.Windows.cs:64-71): BOTH Checked and Unchecked call the
        // SAME handler, which reads the native's CURRENT IsChecked back rather than assuming true/false
        // from which event fired — `VirtualView.IsChecked = PlatformView.IsChecked == true`.
        auto on_toggled = [self](const winrt::Windows::Foundation::IInspectable& sender,
                                 const winui::RoutedEventArgs&) {
            auto* view = self->virtual_view();
            if (view == nullptr)
            {
                return;
            }
            const auto native = sender.try_as<radio_button_control>();
            if (!native)
            {
                return;
            }
            // ToggleButton.IsChecked is IReference<bool> (nullable) — read .Value() only when non-null;
            // C#'s `PlatformView.IsChecked == true` treats a null the same as false.
            const auto is_checked_ref = native.IsChecked();
            view->send_is_checked(is_checked_ref != nullptr && is_checked_ref.Value());
        };
        platform.checked_token = radio.Checked(on_toggled).value;
        platform.unchecked_token = radio.Unchecked(on_toggled).value;
    }

    void radio_button_handler::on_disconnect_handler(radio_button_platform& platform)
    {
        detach_native_events(platform);
    }

    void radio_button_handler::map_is_checked(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->is_checked = view.is_checked();
        // IReference<bool> property, value taken DIRECTLY — box_value(bool) here would hit
        // IReference<bool>'s private constructor (C2248); the projected setter has its own converting
        // overload for the plain bool.
        as_radio(platform->native).IsChecked(platform->is_checked);
    }

    void radio_button_handler::map_content(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->content = std::string(view.content_as_string());
        // RadioButtonExtensions.UpdateContent's `else` branch only — PresentedContent is never an IView
        // on this backend (the header's string-content-only deviation note), so this always takes the
        // `$"{radioButton.Content}"` path: a plain string boxed into Content (object).
        as_radio(platform->native).Content(winrt::box_value(maui::platform::windows::to_hstring(platform->content)));
    }

    void radio_button_handler::map_text_color(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->text_color = view.text_color();
        const radio_button_control radio = as_radio(platform->native);
        // File-top note 2: resource keys ONLY, no direct Foreground push.
        if (!is_set(view, "text_color"))
        {
            remove_resources(radio, k_foreground_keys);
        }
        else
        {
            const winui::Media::SolidColorBrush brush{maui::platform::windows::to_ui_color(platform->text_color)};
            set_resources(radio, k_foreground_keys, brush);
        }
        refresh_theme_resources(radio);
    }

    void radio_button_handler::map_character_spacing(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->character_spacing = view.character_spacing();
        // CharacterSpacingExtensions.ToEm: pt * 0.0624 * 1000, in 1/1000 em units (ControlExtensions.
        // UpdateCharacterSpacing — a plain generic Control property push, no resource-key involvement).
        const auto em = static_cast<std::int32_t>(std::lround(platform->character_spacing * 0.0624 * 1000.0));
        as_radio(platform->native).CharacterSpacing(em);
    }

    void radio_button_handler::map_font(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->text_font = view.font();
        const radio_button_control radio = as_radio(platform->native);
        const font& f = platform->text_font;
        // ALWAYS assign, never skip — ControlExtensions.UpdateFont(Control, Font, IFontManager) is
        // unconditional (matching picker/label/time_picker's map_font, NOT button's skip-if-unset):
        // fontManager.GetFontSize/GetFontFamily resolve the FRAMEWORK default when the font is unset.
        radio.FontSize(f.size() > 0 ? f.size() : maui::platform::windows::default_font_size());
        radio.FontFamily(f.family().empty()
                             ? maui::platform::windows::default_font_family()
                             : winui::Media::FontFamily{maui::platform::windows::to_hstring(f.family())});
        radio.FontStyle(to_font_style(f.slant()));
        radio.FontWeight(to_font_weight(f.weight()));
        // No IsTextScaleFactorEnabled push — see file-top note 4.
    }

    void radio_button_handler::map_stroke_color(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->stroke_color = view.stroke_color();
        const radio_button_control radio = as_radio(platform->native);
        // File-top note 2: resource keys ONLY, no direct BorderBrush push.
        if (!is_set(view, "stroke_color"))
        {
            remove_resources(radio, k_border_keys);
        }
        else
        {
            const winui::Media::SolidColorBrush brush{maui::platform::windows::to_ui_color(platform->stroke_color)};
            set_resources(radio, k_border_keys, brush);
        }
        refresh_theme_resources(radio);
    }

    void radio_button_handler::map_stroke_thickness(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->stroke_thickness = view.stroke_thickness();
        // RadioButtonExtensions.UpdateStrokeThickness: ALWAYS assigns BorderThickness directly (0 when
        // negative/unset) — UNLIKE button_handler.cpp's map_stroke_thickness, there is no ClearValue/
        // restore-the-theme-default branch here; the oracle has none.
        const double t = platform->stroke_thickness < 0 ? 0.0 : platform->stroke_thickness;
        as_radio(platform->native).BorderThickness(winui::Thickness{t, t, t, t});
    }

    void radio_button_handler::map_corner_radius(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->corner_radius = view.corner_radius();
        // RadioButtonExtensions.UpdateCornerRadius: ALWAYS assigns directly, no sign guard at all (unlike
        // button_handler.cpp's map_corner_radius, which clears on a negative value) — the oracle has none.
        const auto r = static_cast<double>(platform->corner_radius);
        as_radio(platform->native).CornerRadius(winui::CornerRadius{r, r, r, r});
    }

    maui::graphics::size radio_button_handler::get_desired_size(double width_constraint, double height_constraint) const
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
        const radio_button_control radio = as_radio(platform->native);
        // ARRANGE/EXPLICIT-SIZE FIX — the same shape button_handler.cpp/picker_handler.cpp/
        // date_picker_handler.cpp/time_picker_handler.cpp all share (see button_handler.cpp's comment for
        // the full oracle citation: ViewHandlerExtensions.Windows.cs:56-74 + :91-105). Pin Width/Height to
        // the view's own explicit request, then only WIDEN the incoming constraint.
        const auto* view = virtual_view();
        const double explicit_width = (view != nullptr) ? view->width() : std::numeric_limits<double>::quiet_NaN();
        const double explicit_height = (view != nullptr) ? view->height() : std::numeric_limits<double>::quiet_NaN();
        radio.Width(explicit_width);
        radio.Height(explicit_height);
        const double adjusted_width_constraint =
            std::isnan(explicit_width) ? width_constraint : std::max(width_constraint, explicit_width);
        const double adjusted_height_constraint =
            std::isnan(explicit_height) ? height_constraint : std::max(height_constraint, explicit_height);
        radio.Measure(
            winrt::Windows::Foundation::Size{maui::platform::windows::measure_constraint(adjusted_width_constraint),
                                             maui::platform::windows::measure_constraint(adjusted_height_constraint)});
        const auto desired = radio.DesiredSize();
        return {desired.Width, desired.Height};
    }

    void radio_button_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        // PlatformArrangeHandler's guard, WIDENED to non-finite — see button_handler.cpp's
        // platform_arrange for why (a NaN reaching XAML's arrange is an unrecoverable stowed exception).
        if (!std::isfinite(frame.x) || !std::isfinite(frame.y) || !std::isfinite(frame.width) ||
            !std::isfinite(frame.height) || frame.width < 0 || frame.height < 0)
        {
            return;
        }
        const radio_button_control radio = as_radio(platform->native);
        winui::Controls::Canvas::SetLeft(radio, frame.x);
        winui::Controls::Canvas::SetTop(radio, frame.y);
        const auto* const view = virtual_view();
        // SHRINK-WRAP THE WIDTH — see file-top note 5 for the full judgment-call rationale. PER-HANDLER ON
        // PURPOSE, matching date_picker_handler.cpp/time_picker_handler.cpp's identical shape — must NOT
        // be hoisted into a shared arrange helper (button_handler.cpp/picker_handler.cpp's PIN-the-frame
        // shape is correct for THEIR controls, and is the alternative to flip to if this call is wrong).
        const double natural = view != nullptr ? view->desired_size().width : frame.width;
        radio.Width(natural > 0 ? std::min(frame.width, natural) : frame.width);
        radio.Height(frame.height);
        // Clip is bounds-dependent (view_chrome_ops.cpp's apply_native_clip reads the just-set Width/
        // Height back); map_clip's own push (view_mapper.cpp) always runs before the first arrange, so
        // this re-invoke is what actually installs the clip once the control has a real size.
        if (view != nullptr)
        {
            apply_native_clip(platform->native, view->clip());
        }
    }

    // ---- generic-IView property pushes (view_platform_base overrides) ---------------------------
    // Delegated to the shared winui_visual_ops free functions so every control behaves identically; see
    // that header for why they are free functions taking the void* slot.
    void radio_button_platform::update_visibility(maui::core::visibility value)
    {
        maui::platform::windows::apply_visibility(native, value);
    }

    void radio_button_platform::update_opacity(double value)
    {
        maui::platform::windows::apply_opacity(native, value);
    }

    void radio_button_platform::update_is_enabled(bool value)
    {
        maui::platform::windows::apply_is_enabled(native, value);
    }

    void radio_button_platform::update_automation_id(std::string_view value)
    {
        maui::platform::windows::apply_automation_id(native, value);
    }

    void radio_button_platform::update_background(const maui::graphics::paint* value)
    {
        if (native == nullptr)
        {
            return;
        }
        // File-top note 3: RadioButtonExtensions.UpdateBackground is gated on `is SolidPaint` with NO else
        // branch — a null/gradient/pattern Background updates nothing (any earlier override survives).
        if (const auto* solid = dynamic_cast<const maui::graphics::solid_paint*>(value))
        {
            const radio_button_control radio = as_radio(native);
            const winui::Media::SolidColorBrush brush{maui::platform::windows::to_ui_color(solid->color())};
            set_resources(radio, k_background_keys, brush);
            refresh_theme_resources(radio);
        }
    }
} // namespace maui::core
