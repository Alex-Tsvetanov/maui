// time_picker_handler — Windows (WinUI 3) platform partial: a REAL
// Microsoft.UI.Xaml.Controls.TimePicker. The windows twin of
// src/platform/apple/time_picker_handler.mm (time-only NSDatePicker) / the android JNI EditText
// partial, and the real-native sibling of the headless mirror partial
// (src/platform/headless/time_picker_handler.cpp). The time/format maps keep the headless UpdateTime
// mirror byte-for-byte AND push the nullable SelectedTime + the ClockIdentifier onto the control; a
// native pick flows back through i_time_picker::set_time from the TimePicker's SelectedTimeChanged
// (plus the InvalidateMeasure C# issues — a picked time can change the rendered width).
//
// Ported DIRECTLY from TimePickerHandler.Windows.cs + Platform/Windows/{TimePickerExtensions.cs
// (UpdateTime / UpdateCharacterSpacing / UpdateFont / UpdateTextColor / UpdateBackground /
// UpdateIsOpen), ControlExtensions.cs (UpdateFont / UpdateIsEnabled), ViewExtensions.cs,
// FontExtensions.cs, CharacterSpacingExtensions.cs} + Fonts/FontManager.Windows.cs.
//
// Time bridging: the port's maui::core::time_span (a std::chrono::milliseconds duration, see
// include/maui/core/date_time.hpp) converts to Windows.Foundation.TimeSpan (an int64 100ns-tick
// std::chrono duration) by duration_cast — exact in this direction (ms → ticks), and the native
// wheel only ever hands back whole minutes.
//
// DOCUMENTED DEVIATIONS (each an infrastructure gap of this first cut, not a behavior guess):
//   - Text color / background land on the DIRECT dependency properties (Foreground / Background). C#
//     additionally writes the themed resource keys (TimePickerButtonForeground* /
//     TimePickerButtonBackground* + RefreshThemeResources) so the hover/pressed/disabled states track
//     the value — deferred: the resource-dictionary seam (the button partial's identical cut). The C#
//     null branches map to ClearValue, discriminated through BindableObject.IsSet where the port's
//     color value type has no null.
//   - CharacterSpacing: C#'s first line (platformTimePicker.CharacterSpacing = value.ToEm()) is pushed
//     verbatim; the follow-up walk into the template's Hour/Minute/Period TextBlocks
//     (GetDescendantByName, retried on Loaded) is deferred with the Loaded trampoline.
//   - MapIsOpen: C#'s UpdateIsOpen drives the flyout through an automation peer (find the inner
//     Button child, invoke its IInvokeProvider) after the Loaded gate — deferred: the automation-peer
//     seam; the control-level is_open()/Opened/Closed stay the observable result (the headless twin's
//     stance).
//   - FontManager's registrar lookups are skipped (no font registrar in the port yet): a named family
//     goes straight to FontFamily(name); the default size is the constant 14
//     (ControlContentThemeFontSize). Byte-for-byte the button/label partials' map_font.
//
// XAML-less degradation (the windows twin of the android VM-less fallback): the windows preset also
// runs the cross-platform suite on the host where NO XAML runtime exists; create_platform_view
// catches the construction failure and keeps native null, while the headless mirrors are ALWAYS
// maintained and on_done stays an invokable C++ callback (the cross-platform suite drives it) — so
// that suite observes exactly the headless partial's behavior.

#include "maui/core/time_picker_handler.hpp"

#include <chrono>
#include <cmath>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Text.h>
#include <winrt/base.h>

#include "maui/core/bindable_object.hpp"
#include "maui/core/date_time.hpp"
#include "maui/core/dimension.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_time_picker.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "windows_native.hpp"

namespace
{
    namespace muxc = winrt::Microsoft::UI::Xaml::Controls;
    namespace muxm = winrt::Microsoft::UI::Xaml::Media;
    namespace wf = winrt::Windows::Foundation;
    namespace wut = winrt::Windows::UI::Text;
    namespace wnative = maui::platform::win;

    // FontManager.Windows.DefaultFontSize — the ControlContentThemeFontSize theme resource (14.0);
    // read as a constant here (no Application.Current on the XAML-less test host).
    constexpr double k_default_font_size = 14.0;

    [[nodiscard]] muxc::TimePicker picker_of(const maui::core::time_picker_platform& platform)
    {
        return wnative::borrow<muxc::TimePicker>(platform.native);
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

    // System.TimeSpan → Windows.Foundation.TimeSpan (both std::chrono durations; ms → 100ns ticks is
    // exact).
    [[nodiscard]] wf::TimeSpan to_winrt_time_span(const maui::core::time_span& value)
    {
        return std::chrono::duration_cast<wf::TimeSpan>(value.value());
    }

    // Windows.Foundation.TimeSpan → the port's time_span (the wheel hands back whole minutes, so the
    // tick → ms truncation is exact for every native pick).
    [[nodiscard]] maui::core::time_span to_port_time_span(wf::TimeSpan value)
    {
        return maui::core::time_span{std::chrono::duration_cast<std::chrono::milliseconds>(value)};
    }
} // namespace

namespace maui::core
{
    namespace
    {
        // TimePickerExtensions.UpdateTime(nativeTimePicker, timePicker): the headless mirror half is
        // byte-for-byte src/platform/headless/time_picker_handler.cpp's update_time (the wheel value +
        // the formatted display text — the XAML-less suite's observable); the native half pushes the
        // nullable SelectedTime and the H-format → ClockIdentifier switch verbatim.
        void update_time(time_picker_handler& handler, i_time_picker& view)
        {
            auto* platform = handler.typed_platform_view();
            if (platform == nullptr)
            {
                return;
            }
            // --- headless mirror (identical to src/platform/headless/time_picker_handler.cpp) ---
            const auto time = view.time();
            platform->time = time.value_or(time_span{});
            platform->text = time.has_value() ? format_time_span(*time, view.format()) : std::string{};

            // --- native push (TimePickerExtensions.UpdateTime against the real TimePicker) ---
            auto picker = picker_of(*platform);
            if (picker == nullptr)
            {
                return;
            }
            if (time.has_value())
            {
                picker.SelectedTime(wf::IReference<wf::TimeSpan>{to_winrt_time_span(*time)});
            }
            else
            {
                picker.SelectedTime(nullptr); // nativeTimePicker.SelectedTime = timePicker.Time (null)
            }
            // timePicker.Format?.Contains('H') == true → "24HourClock", else "12HourClock" (a null C#
            // Format collapses to the empty string_view here — same 12-hour outcome).
            picker.ClockIdentifier(view.format().find('H') != std::string_view::npos ? L"24HourClock"
                                                                                     : L"12HourClock");
        }
    } // namespace

    // Releases the one strong ref pinning the TimePicker (the wnative shape of the pimpl-owned-native
    // doctrine; the apple twin CFReleases its NSDatePicker here). The event token is revoked in
    // on_disconnect_handler; the dtor only drops the ref.
    time_picker_platform::~time_picker_platform()
    {
        wnative::release(native);
    }

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each
    // calls the base body FIRST — the headless mirrors must stay live for the XAML-less cross-platform
    // suite (header note) — then pushes to the real TimePicker when one exists.

    void time_picker_platform::update_visibility(maui::core::visibility value)
    {
        view_platform_base::update_visibility(value);
        // ViewExtensions.UpdateVisibility (Windows): Hidden rides Opacity 0, Collapsed collapses.
        wnative::apply_visibility(native, value, alpha);
    }

    void time_picker_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        // ViewExtensions.UpdateOpacity: the Hidden state pins opacity 0 (apply_visibility restores it).
        if (!hidden)
        {
            wnative::apply_opacity(native, value);
        }
    }

    void time_picker_platform::update_is_enabled(bool value)
    {
        view_platform_base::update_is_enabled(value);
        // ViewExtensions.UpdateIsEnabled → ControlExtensions.UpdateIsEnabled: Control.IsEnabled.
        if (auto picker = picker_of(*this))
        {
            picker.IsEnabled(value);
        }
    }

    void time_picker_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        // ViewExtensions.UpdateAutomationId: AutomationProperties.SetAutomationId.
        wnative::apply_automation_id(native, value);
    }

    void time_picker_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        auto picker = picker_of(*this);
        if (picker == nullptr)
        {
            return;
        }
        // TimePickerHandler.Windows.MapBackground → TimePickerExtensions.UpdateBackground: null →
        // RemoveKeys(TimePickerButtonBackground*) + ClearValue(BackgroundProperty); value → the
        // resource keys + Background = brush (+ RefreshThemeResources). The port pushes the DIRECT
        // Background (deferred: the per-state resource keys — header).
        if (value == nullptr)
        {
            picker.ClearValue(muxc::Control::BackgroundProperty());
            return;
        }
        // Paint.ToPlatform: solid + linear/radial gradient (to_paint_brush); image/pattern still fall back to solid.
        picker.Background(wnative::to_paint_brush(value));
        return;
        // deferred: gradient / image-source paints (Paint.ToPlatform) — the base mirror above keeps the
        // borrow observable.
    }

    std::unique_ptr<time_picker_platform> time_picker_handler::create_platform_view()
    {
        auto platform = std::make_unique<time_picker_platform>();
        try
        {
            // TimePickerHandler.Windows.CreatePlatformView: new TimePicker().
            const muxc::TimePicker picker;
            platform->native = wnative::store(picker); // released in ~time_picker_platform
        }
        catch (const winrt::hresult_error&)
        {
            platform->native = nullptr; // XAML-less degradation (header note)
        }
        return platform;
    }

    void time_picker_handler::on_connect_handler(time_picker_platform& platform)
    {
        // on_done stays wired even XAML-less so the cross-platform suite can drive a commit (the
        // headless twin's Done-tap analog: hours + minutes only — C# builds
        // `new TimeSpan(datetime.Hour, datetime.Minute, 0)`).
        platform.on_done = [this] {
            auto* view = virtual_view();
            auto* typed = typed_platform_view();
            if (view == nullptr || typed == nullptr)
            {
                return;
            }
            view->set_time(time_span(typed->time.hours(), typed->time.minutes(), 0));
        };
        auto picker = picker_of(platform);
        if (picker == nullptr)
        {
            return;
        }
        // ConnectHandler: SelectedTimeChanged += OnSelectedTimeChanged. The C# handler is a handler
        // INSTANCE method, so the lambda captures `this`; on_disconnect_handler revokes the token
        // before the handler/platform die, so the capture never dangles.
        platform.selected_time_changed_token =
            picker
                .SelectedTimeChanged([this](const muxc::TimePicker&,
                                            const muxc::TimePickerSelectedValueChangedEventArgs& args) {
                    // OnSelectedTimeChanged: VirtualView.Time = e.NewTime (nullable);
                    // VirtualView.InvalidateMeasure() — a picked time can change the rendered width.
                    auto* view = virtual_view();
                    if (view == nullptr)
                    {
                        return;
                    }
                    const auto new_time = args.NewTime();
                    if (new_time != nullptr)
                    {
                        view->set_time(to_port_time_span(new_time.Value()));
                    }
                    else
                    {
                        view->set_time(std::nullopt);
                    }
                    view->invalidate_measure();
                })
                .value;
    }

    void time_picker_handler::on_disconnect_handler(time_picker_platform& platform)
    {
        // DisconnectHandler: SelectedTimeChanged -= OnSelectedTimeChanged. The C++ callback is cleared
        // like the headless twin.
        platform.on_done = nullptr;
        if (auto picker = picker_of(platform))
        {
            if (platform.selected_time_changed_token != 0)
            {
                picker.SelectedTimeChanged(winrt::event_token{platform.selected_time_changed_token});
            }
        }
        platform.selected_time_changed_token = 0;
    }

    void time_picker_handler::map_format(time_picker_handler& handler, i_time_picker& view)
    {
        update_time(handler, view); // MapFormat → UpdateTime (a format change re-renders + re-clocks)
    }

    void time_picker_handler::map_time(time_picker_handler& handler, i_time_picker& view)
    {
        update_time(handler, view); // MapTime → UpdateTime
    }

    void time_picker_handler::map_text_color(time_picker_handler& handler, i_time_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->text_color = view.text_color(); // headless mirror
        auto picker = picker_of(*platform);
        if (picker == nullptr)
        {
            return;
        }
        // TimePickerExtensions.UpdateTextColor: null → RemoveKeys(TimePickerButtonForeground*) +
        // ClearValue(Foreground); value → the resource keys + Foreground = brush (+
        // RefreshThemeResources). The port pushes the direct Foreground (deferred: the per-state
        // resource keys — header), with the null branch discriminated through BindableObject.IsSet
        // (the port's color has no null).
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        const bool color_is_set = bindable != nullptr && bindable->is_property_set("text_color");
        if (color_is_set)
        {
            picker.Foreground(wnative::to_brush(view.text_color()));
        }
        else
        {
            picker.ClearValue(muxc::Control::ForegroundProperty());
        }
    }

    void time_picker_handler::map_font(time_picker_handler& handler, i_time_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->text_font = view.font(); // headless mirror
        auto picker = picker_of(*platform);
        if (picker == nullptr)
        {
            return;
        }
        // TimePickerExtensions.UpdateFont → ControlExtensions.UpdateFont: FontSize + FontFamily +
        // FontStyle + FontWeight + IsTextScaleFactorEnabled (FontManager.GetFontSize/GetFontFamily —
        // registrar skipped, header).
        const font value = view.font();
        const double size = value.size();
        picker.FontSize((size > 0 && !std::isnan(size)) ? size : k_default_font_size);
        if (!value.family().empty())
        {
            picker.FontFamily(muxm::FontFamily{wnative::to_hstring_utf8(value.family())});
        }
        else
        {
            picker.ClearValue(muxc::Control::FontFamilyProperty()); // C# null Family → the default family
        }
        picker.FontStyle(to_font_style(value.slant()));
        picker.FontWeight(to_font_weight(value.weight()));
        picker.IsTextScaleFactorEnabled(value.auto_scaling_enabled());
    }

    void time_picker_handler::map_character_spacing(time_picker_handler& handler, i_time_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->character_spacing = view.character_spacing(); // headless mirror
        if (auto picker = picker_of(*platform))
        {
            // TimePickerExtensions.UpdateCharacterSpacing: CharacterSpacing = value.ToEm() (verbatim
            // first line). deferred: the follow-up UpdateCharacterSpacingInTimePicker walk into the
            // template's Hour/Minute/Period TextBlocks (GetDescendantByName after the Loaded gate —
            // header deviations).
            picker.CharacterSpacing(wnative::to_em(view.character_spacing()));
        }
    }

    void time_picker_handler::map_is_open(time_picker_handler& /*handler*/, i_time_picker& /*view*/)
    {
        // deferred: TimePickerExtensions.UpdateIsOpen drives the flyout through an automation peer
        // (Focus + find the inner Button child + IInvokeProvider.Invoke, after the Loaded gate) — the
        // automation-peer seam has not landed on this backend, so this is a no-op like the headless
        // twin; the control-level is_open()/Opened/Closed are the observable result.
    }

    maui::graphics::size time_picker_handler::get_desired_size(double width_constraint,
                                                               double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return {0, 0};
        }
        if (platform->native == nullptr)
        {
            // XAML-less degradation: the headless partial's placeholder metric (a single-line field
            // ~150pt wide, clamped to a finite width constraint, one line tall), so the backend-
            // agnostic size-request suites see consistent numbers.
            double width = 150.0;
            if (width_constraint > 0 && width_constraint < width)
            {
                width = width_constraint;
            }
            return {width, 22.0};
        }
        // ViewHandlerExtensions.Windows.cs GetDesiredSizeFromHandler: FrameworkElement.Measure +
        // DesiredSize, with the AdjustForExplicitSize clamp fed from the virtual view's explicit
        // width()/height() (see measure_native).
        const double explicit_width = virtual_view() != nullptr ? virtual_view()->width() : dimension::unset;
        const double explicit_height = virtual_view() != nullptr ? virtual_view()->height() : dimension::unset;
        return wnative::measure_native(platform->native, width_constraint, height_constraint, explicit_width,
                                       explicit_height);
    }

    void time_picker_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // XAML-less: no native layout to apply
        }
        // The shared Canvas recipe: Canvas.SetLeft/SetTop + explicit Width/Height pin the TimePicker to
        // the frame (ViewHandlerExtensions.Windows.cs PlatformArrangeHandler on the Canvas layout model).
        wnative::arrange_native(platform->native, frame);
    }
} // namespace maui::core
