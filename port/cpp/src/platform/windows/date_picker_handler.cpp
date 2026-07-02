// date_picker_handler — Windows (WinUI 3) platform partial: a REAL
// Microsoft.UI.Xaml.Controls.CalendarDatePicker. The windows twin of
// src/platform/apple/date_picker_handler.mm (NSDatePicker) / the android JNI EditText partial, and
// the real-native sibling of the headless mirror partial
// (src/platform/headless/date_picker_handler.cpp). The date/format maps keep the headless UpdateDate
// mirror byte-for-byte AND push the nullable Date onto the control; a native pick flows back through
// i_date_picker::set_date from the CalendarDatePicker's DateChanged, and Opened/Closed write is_open
// back (the C# Opened/Closed instance handlers).
//
// Ported DIRECTLY from DatePickerHandler.Windows.cs + Platform/Windows/{DatePickerExtensions.cs
// (UpdateDate / UpdateMinimumDate / UpdateMaximumDate / UpdateCharacterSpacing / UpdateFont /
// UpdateTextColor / UpdateBackground / UpdateIsOpen), CalendarDatePickerExtensions.cs (ToDateFormat),
// ControlExtensions.cs (UpdateFont / UpdateIsEnabled), ViewExtensions.cs, FontExtensions.cs} +
// Fonts/FontManager.Windows.cs.
//
// Date bridging: the port's maui::core::date_time (a std::chrono sys_days + time-of-day, see
// include/maui/core/date_time.hpp) converts to Windows.Foundation.DateTime (100ns ticks since the
// 1601 FILETIME epoch) through winrt::clock::from_sys/to_sys — an exact ticks conversion, no
// days-math approximation. C#'s CalendarDatePicker.Date is a DateTimeOffset? whose .DateTime leg is
// LOCAL time; the port's date_time is UTC-based (the documented date_time.hpp deviation), so the
// calendar DAY is what round-trips here.
//
// DOCUMENTED DEVIATIONS (each an infrastructure gap of this first cut, not a behavior guess):
//   - DateFormat: C#'s UpdateDate maps a custom Format through
//     CalendarDatePickerExtensions.ToDateFormat into the WinUI DateTimeFormatter pattern string
//     ({day.integer(n)}/{month.*}/{year.*}). deferred: that pattern build — the control shows its
//     default short-date rendering; the FORMATTED text stays observable through the headless `text`
//     mirror (the twins' format_date_time path).
//   - Text color / background land on the DIRECT dependency properties (Foreground / Background). C#
//     additionally writes the themed resource keys (CalendarDatePickerTextForeground* + the calendar
//     glyph keys / CalendarDatePickerBackground* + RefreshThemeResources) so the hover/pressed/
//     disabled states track the value — deferred: the resource-dictionary seam (the button partial's
//     identical cut). The C# null branches map to ClearValue, discriminated through
//     BindableObject.IsSet where the port's color value type has no null.
//   - CharacterSpacing: C# walks the loaded template for the "DateText" TextBlock
//     (GetDescendantByName, retried on Loaded) — there is NO control-level push in C#. deferred: the
//     descendant walk needs the Loaded trampoline; only the headless mirror is kept.
//   - FontManager's registrar lookups are skipped (no font registrar in the port yet): a named family
//     goes straight to FontFamily(name); the default size is the constant 14
//     (ControlContentThemeFontSize). Byte-for-byte the button/label partials' map_font.
//
// XAML-less degradation (the windows twin of the android VM-less fallback): the windows preset also
// runs the cross-platform suite on the host where NO XAML runtime exists; create_platform_view
// catches the construction failure and keeps native null, while the headless mirrors are ALWAYS
// maintained and on_done stays an invokable C++ callback (the cross-platform suite drives it) — so
// that suite observes exactly the headless partial's behavior.

#include "maui/core/date_picker_handler.hpp"

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
#include "maui/core/i_date_picker.hpp"
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

    [[nodiscard]] muxc::CalendarDatePicker picker_of(const maui::core::date_picker_platform& platform)
    {
        return wnative::borrow<muxc::CalendarDatePicker>(platform.native);
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

    // System.DateTime → Windows.Foundation.DateTime: the port's sys_days + time-of-day becomes a
    // system_clock time_point, which winrt::clock (the 1601 FILETIME epoch, 100ns ticks) converts
    // exactly.
    [[nodiscard]] wf::DateTime to_winrt_date_time(const maui::core::date_time& value)
    {
        return winrt::clock::from_sys(value.days() + value.time_of_day());
    }

    // Windows.Foundation.DateTime → the port's date_time: back through the system clock, split into
    // the day and the time-of-day remainder (C# reads args.NewDate.Value.DateTime).
    [[nodiscard]] maui::core::date_time to_port_date_time(wf::DateTime value)
    {
        const auto sys = winrt::clock::to_sys(value);
        const auto day = std::chrono::floor<std::chrono::days>(sys);
        const auto time_of_day = std::chrono::duration_cast<std::chrono::milliseconds>(sys - day);
        return maui::core::date_time{std::chrono::sys_days{day}, time_of_day};
    }

    // DateTime.Now.AddYears(-100) — the WinUI-default MinDate jump DatePickerExtensions.UpdateMinimumDate
    // replicates for a null MinimumDate. AddYears clamps an invalid day (Feb 29 → Feb 28), which the
    // year_month_day ok() check reproduces via the last-day snap.
    [[nodiscard]] maui::core::date_time now_minus_100_years()
    {
        auto ymd = std::chrono::year_month_day{maui::core::date_time::now().days()};
        ymd -= std::chrono::years{100};
        if (!ymd.ok())
        {
            ymd = std::chrono::year_month_day{ymd.year() / ymd.month() / std::chrono::last};
        }
        return maui::core::date_time{std::chrono::sys_days{ymd}};
    }
} // namespace

namespace maui::core
{
    namespace
    {
        // DatePickerExtensions.UpdateTextColor: null → RemoveKeys + ClearValue(Foreground); value →
        // SetValueForAllKey + Foreground = brush (+ RefreshThemeResources). The port pushes the direct
        // Foreground (deferred: the CalendarDatePickerTextForeground* / glyph resource keys — header),
        // with the null branch discriminated through BindableObject.IsSet (the port's color has no
        // null). A helper because UpdateDate's tail re-runs it (C#: platformDatePicker
        // .UpdateTextColor(datePicker)).
        void push_text_color(const date_picker_platform& platform, i_date_picker& view)
        {
            auto picker = picker_of(platform);
            if (picker == nullptr)
            {
                return;
            }
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

        // DatePickerExtensions.UpdateDate(platformDatePicker, datePicker): the headless mirror half is
        // byte-for-byte src/platform/headless/date_picker_handler.cpp's update_date (the dialog date +
        // the formatted display text — the XAML-less suite's observable); the native half pushes the
        // nullable Date, defers the DateFormat pattern build (header deviations), and re-runs the text
        // color exactly like C#'s tail.
        void update_date(date_picker_handler& handler, i_date_picker& view)
        {
            auto* platform = handler.typed_platform_view();
            if (platform == nullptr)
            {
                return;
            }
            // --- headless mirror (identical to src/platform/headless/date_picker_handler.cpp) ---
            const auto date = view.date();
            platform->date = date.value_or(date_time::today());
            if (!date.has_value())
            {
                platform->text.clear();
            }
            else
            {
                const std::string_view format = view.format();
                if (format.empty() || format == "d" || format == "D")
                {
                    platform->text = format_date_time(*date, format == "D" ? "D" : "d");
                }
                else
                {
                    platform->text = format_date_time(*date, format);
                }
            }

            // --- native push (DatePickerExtensions.UpdateDate against the real CalendarDatePicker) ---
            auto picker = picker_of(*platform);
            if (picker == nullptr)
            {
                return;
            }
            if (!date.has_value())
            {
                picker.Date(nullptr); // datePicker.Date is null → platformDatePicker.Date = null
            }
            else
            {
                picker.Date(wf::IReference<wf::DateTime>{to_winrt_date_time(*date)});
            }
            // deferred: platformDatePicker.DateFormat = format.ToDateFormat()
            // (CalendarDatePickerExtensions.ToDateFormat's {day.*}/{month.*}/{year.*} pattern build —
            // header deviations).
            push_text_color(*platform, view); // UpdateDate's tail: UpdateTextColor(datePicker)
        }
    } // namespace

    // Releases the one strong ref pinning the CalendarDatePicker (the wnative shape of the
    // pimpl-owned-native doctrine; the apple twin CFReleases its NSDatePicker here). Event tokens are
    // revoked in on_disconnect_handler; the dtor only drops the ref.
    date_picker_platform::~date_picker_platform()
    {
        wnative::release(native);
    }

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each
    // calls the base body FIRST — the headless mirrors must stay live for the XAML-less cross-platform
    // suite (header note) — then pushes to the real CalendarDatePicker when one exists.

    void date_picker_platform::update_visibility(maui::core::visibility value)
    {
        view_platform_base::update_visibility(value);
        // ViewExtensions.UpdateVisibility (Windows): Hidden rides Opacity 0, Collapsed collapses.
        wnative::apply_visibility(native, value, alpha);
    }

    void date_picker_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        // ViewExtensions.UpdateOpacity: the Hidden state pins opacity 0 (apply_visibility restores it).
        if (!hidden)
        {
            wnative::apply_opacity(native, value);
        }
    }

    void date_picker_platform::update_is_enabled(bool value)
    {
        view_platform_base::update_is_enabled(value);
        // ViewExtensions.UpdateIsEnabled → ControlExtensions.UpdateIsEnabled: Control.IsEnabled.
        if (auto picker = picker_of(*this))
        {
            picker.IsEnabled(value);
        }
    }

    void date_picker_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        // ViewExtensions.UpdateAutomationId: AutomationProperties.SetAutomationId.
        wnative::apply_automation_id(native, value);
    }

    void date_picker_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        auto picker = picker_of(*this);
        if (picker == nullptr)
        {
            return;
        }
        // DatePickerHandler.Windows.MapBackground → DatePickerExtensions.UpdateBackground: null →
        // RemoveKeys(CalendarDatePickerBackground*) + ClearValue(BackgroundProperty); value → the
        // resource keys + Background = brush (+ RefreshThemeResources). The port pushes the DIRECT
        // Background (deferred: the per-state resource keys — header).
        if (value == nullptr)
        {
            picker.ClearValue(muxc::Control::BackgroundProperty());
            return;
        }
        if (const auto* solid = dynamic_cast<const maui::graphics::solid_paint*>(value))
        {
            picker.Background(wnative::to_brush(solid->color()));
            return;
        }
        // deferred: gradient / image-source paints (Paint.ToPlatform) — the base mirror above keeps the
        // borrow observable.
    }

    std::unique_ptr<date_picker_platform> date_picker_handler::create_platform_view()
    {
        auto platform = std::make_unique<date_picker_platform>();
        try
        {
            // DatePickerHandler.Windows.CreatePlatformView: new CalendarDatePicker().
            const muxc::CalendarDatePicker picker;
            platform->native = wnative::store(picker); // released in ~date_picker_platform
        }
        catch (const winrt::hresult_error&)
        {
            platform->native = nullptr; // XAML-less degradation (header note)
        }
        return platform;
    }

    void date_picker_handler::on_connect_handler(date_picker_platform& platform)
    {
        // on_done stays wired even XAML-less so the cross-platform suite can drive a commit (the
        // headless twin's Done-tap analog: commit the dialog's current `date` mirror through set_date,
        // whose control-side coercion clamps it into [MinimumDate, MaximumDate]).
        platform.on_done = [this] {
            auto* view = virtual_view();
            auto* typed = typed_platform_view();
            if (view == nullptr || typed == nullptr)
            {
                return;
            }
            view->set_date(typed->date);
        };
        auto picker = picker_of(platform);
        if (picker == nullptr)
        {
            return;
        }
        // ConnectHandler: Opened += Opened; Closed += Closed; DateChanged += DateChanged. The C#
        // handlers are handler INSTANCE methods, so the lambdas capture `this`;
        // on_disconnect_handler revokes the tokens before the handler/platform die, so the captures
        // never dangle.
        platform.date_changed_token =
            picker
                .DateChanged([this](const muxc::CalendarDatePicker&,
                                    const muxc::CalendarDatePickerDateChangedEventArgs& args) {
                    // DatePickerHandler.Windows.DateChanged: a cleared native date nulls the virtual
                    // Date; a new value writes back only when it differs (the guard against the
                    // map-push echo).
                    auto* view = virtual_view();
                    if (view == nullptr)
                    {
                        return;
                    }
                    const auto new_date = args.NewDate();
                    if (new_date == nullptr)
                    {
                        view->set_date(std::nullopt);
                        return;
                    }
                    const date_time picked = to_port_date_time(new_date.Value());
                    if (!view->date().has_value() || *view->date() != picked)
                    {
                        view->set_date(picked);
                    }
                })
                .value;
        platform.opened_token = picker
                                    .Opened([this](const winrt::Windows::Foundation::IInspectable&,
                                                   const winrt::Windows::Foundation::IInspectable&) {
                                        // Opened: VirtualView.IsOpen = true.
                                        if (auto* view = virtual_view())
                                        {
                                            view->set_is_open(true);
                                        }
                                    })
                                    .value;
        platform.closed_token = picker
                                    .Closed([this](const winrt::Windows::Foundation::IInspectable&,
                                                   const winrt::Windows::Foundation::IInspectable&) {
                                        // Closed: VirtualView.IsOpen = false.
                                        if (auto* view = virtual_view())
                                        {
                                            view->set_is_open(false);
                                        }
                                    })
                                    .value;
    }

    void date_picker_handler::on_disconnect_handler(date_picker_platform& platform)
    {
        // DisconnectHandler: Opened/Closed/DateChanged -= …. The C++ callback is cleared like the
        // headless twin.
        platform.on_done = nullptr;
        if (auto picker = picker_of(platform))
        {
            if (platform.date_changed_token != 0)
            {
                picker.DateChanged(winrt::event_token{platform.date_changed_token});
            }
            if (platform.opened_token != 0)
            {
                picker.Opened(winrt::event_token{platform.opened_token});
            }
            if (platform.closed_token != 0)
            {
                picker.Closed(winrt::event_token{platform.closed_token});
            }
        }
        platform.date_changed_token = 0;
        platform.opened_token = 0;
        platform.closed_token = 0;
    }

    void date_picker_handler::map_format(date_picker_handler& handler, i_date_picker& view)
    {
        update_date(handler, view); // MapFormat → UpdateDate (a format change re-renders)
    }

    void date_picker_handler::map_date(date_picker_handler& handler, i_date_picker& view)
    {
        update_date(handler, view); // MapDate → UpdateDate
    }

    void date_picker_handler::map_minimum_date(date_picker_handler& handler, i_date_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->minimum_date = view.minimum_date(); // headless mirror
        if (auto picker = picker_of(*platform))
        {
            // DatePickerExtensions.UpdateMinimumDate: a set MinimumDate lands on MinDate; a null one
            // replays WinUI's own default (DateTime.Now.AddYears(-100)) so clearing the bound restores
            // the stock calendar range.
            const auto minimum = view.minimum_date();
            picker.MinDate(to_winrt_date_time(minimum.has_value() ? *minimum : now_minus_100_years()));
        }
    }

    void date_picker_handler::map_maximum_date(date_picker_handler& handler, i_date_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->maximum_date = view.maximum_date(); // headless mirror
        if (auto picker = picker_of(*platform))
        {
            // DatePickerExtensions.UpdateMaximumDate: MaxDate = MaximumDate ?? DateTime.MaxValue
            // (9999-12-31 — the sub-day .MaxValue tail is immaterial to a calendar clamp).
            const auto maximum = view.maximum_date();
            picker.MaxDate(to_winrt_date_time(maximum.has_value() ? *maximum : date_time{9999, 12, 31}));
        }
    }

    void date_picker_handler::map_text_color(date_picker_handler& handler, i_date_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->text_color = view.text_color(); // headless mirror
        push_text_color(*platform, view);         // DatePickerExtensions.UpdateTextColor
    }

    void date_picker_handler::map_font(date_picker_handler& handler, i_date_picker& view)
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
        // DatePickerExtensions.UpdateFont → ControlExtensions.UpdateFont: FontSize + FontFamily +
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

    void date_picker_handler::map_character_spacing(date_picker_handler& handler, i_date_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->character_spacing = view.character_spacing(); // headless mirror
        }
        // deferred: DatePickerExtensions.UpdateCharacterSpacing applies the em value to the template's
        // "DateText" TextBlock (GetDescendantByName, retried on Loaded) — the descendant walk needs the
        // Loaded trampoline; C# has no control-level CharacterSpacing push for this control (header
        // deviations).
    }

    void date_picker_handler::map_is_open(date_picker_handler& handler, i_date_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        if (auto picker = picker_of(*platform))
        {
            // DatePickerExtensions.UpdateIsOpen: IsCalendarOpen = datePicker.IsOpen.
            picker.IsCalendarOpen(view.is_open());
        }
    }

    maui::graphics::size date_picker_handler::get_desired_size(double width_constraint,
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

    void date_picker_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // XAML-less: no native layout to apply
        }
        // The shared Canvas recipe: Canvas.SetLeft/SetTop + explicit Width/Height pin the
        // CalendarDatePicker to the frame (ViewHandlerExtensions.Windows.cs PlatformArrangeHandler on
        // the Canvas layout model).
        wnative::arrange_native(platform->native, frame);
    }
} // namespace maui::core
