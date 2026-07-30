// date_picker_handler — WinUI 3 platform recipe: a real Microsoft.UI.Xaml.Controls.CalendarDatePicker,
// the same native type DatePickerHandler.Windows.cs creates (`ViewHandler<IDatePicker,
// CalendarDatePicker>` — NOT the plain `DatePicker` control, which has a completely different property
// surface: no MinDate/MaxDate/DateFormat, no Opened/Closed/DateChanged). Ported from
// DatePickerHandler.Windows.cs + DatePickerHandler.cs (the shared mapper table) +
// DatePickerExtensions.cs + CalendarDatePickerExtensions.cs (the ToDateFormat grammar).
//
// DOCUMENTED SIMPLIFICATIONS against C#, each narrowed on purpose rather than left vague:
//
//  1. Background (MapBackground -> DatePickerExtensions.UpdateBackground) is Android/Windows-only in the
//     C# mapper table, but date_picker_handler.cpp's cross-platform mapper() ALREADY documents "The
//     Android/Windows-only Background remap ... stay platform-specific in C# and are not replicated" —
//     background is not in this handler's mapper table at all (confirmed against the headless twin, which
//     also has no map_background). The generic IView Background still reaches the CalendarDatePicker
//     through the five-override block below (a direct brush push), not the per-visual-state
//     CalendarDatePickerBackground* resource keys UpdateBackground uses — see picker_handler.cpp's
//     identical map_text_color note for why THAT distinction matters for state-restyled brushes;
//     Background here is unstated in the C# picker mapper for this handler, so the plain generic push is
//     not a narrowing of anything this handler is asked to do.
//  2. CharacterSpacing (UpdateCharacterSpacing -> ApplyCharacterSpacingToTextBlocks). C# reaches INSIDE
//     the CalendarDatePicker's control template for a "DateText" descendant TextBlock (with a
//     Loaded-deferred retry if the template isn't built yet) and sets THAT TextBlock's CharacterSpacing —
//     CalendarDatePicker has no top-level CharacterSpacing of its own that the template binds to. This
//     backend has no VisualTreeHelper descendant-by-name walk yet, the SAME documented gap as
//     entry_handler.cpp's PlaceholderTextContentPresenter reach and search_bar_handler.cpp's
//     "DeleteButton"/inner-TextBox reaches — map_character_spacing is mirror-only (records the value on
//     date_picker_platform, no native push) until that infrastructure exists.
//  3. IsTextScaleFactorEnabled. ControlExtensions.UpdateFont assigns this on every `Control`, but per
//     button_handler.cpp's map_font note, this WinRT projection only exposes the property on `TextBlock` —
//     matching picker_handler.cpp/entry_handler.cpp's identical skip for the same reason.
//  4. MinimumDate/MaximumDate "no bound" fallbacks. UpdateMinimumDate's null case is
//     `DateTime.Now.AddYears(-100)`; UpdateMaximumDate's is `DateTime.MaxValue`. The port's date_time
//     (date_time.hpp) is deliberately minimal and has no AddYears/MaxValue — `default_minimum_date()`
//     below reconstructs the same calendar day 100 years earlier via year_month_day (TODO: does not
//     special-case a Feb-29 anchor landing on a non-leap target year, unlike .NET's clamp-to-Feb-28 —
//     negligible for a "100 years back, effectively unbounded" floor); `default_maximum_date()` uses
//     9999-12-31 at midnight rather than DateTime.MaxValue's exact 23:59:59.9999999 tail (TODO: verify
//     against DatePickerExtensions.cs:53 if a future caller ever needs exact equality with MaxValue —
//     the practical effect, an uncapped upper bound, is identical).

#include "maui/core/date_picker_handler.hpp"

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>

#include "maui/core/bindable_object.hpp"
#include "maui/core/date_time.hpp"
#include "maui/core/i_date_picker.hpp"
#include "maui/core/view_chrome_ops.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "winui_interop.hpp"
#include "winui_visual_ops.hpp"

namespace
{
    // Named `winui`, NOT `xaml`: the port already has a maui::xaml namespace (the XAML loader), and
    // inside namespace maui::* that name WINS over a file-scope alias — see picker_handler.cpp's
    // identical note.
    namespace winui = winrt::Microsoft::UI::Xaml;
    using calendar_date_picker = winui::Controls::CalendarDatePicker;

    calendar_date_picker as_calendar_date_picker(void* native)
    {
        return maui::platform::windows::ref<winui::UIElement>(native).as<calendar_date_picker>();
    }

    // DatePickerExtensions.cs's TextColorResourceKeys — the per-visual-state Foreground/glyph-color
    // brushes the CalendarDatePicker control template binds to; a plain Foreground override alone is
    // dropped the moment the pointer enters/leaves/presses/selects (see picker_handler.cpp's identical
    // k_text_color_keys note for why the resource-key set is needed at all).
    constexpr std::array<std::wstring_view, 9> k_text_color_keys{
        L"CalendarDatePickerTextForeground",
        L"CalendarDatePickerTextForegroundPointerOver",
        L"CalendarDatePickerTextForegroundPressed",
        L"CalendarDatePickerTextForegroundDisabled",
        L"CalendarDatePickerTextForegroundSelected",
        L"CalendarDatePickerCalendarGlyphForeground",
        L"CalendarDatePickerCalendarGlyphForegroundPointerOver",
        L"CalendarDatePickerCalendarGlyphForegroundPressed",
        L"CalendarDatePickerCalendarGlyphForegroundDisabled",
    };

    void set_resources(const calendar_date_picker& combo, std::span<const std::wstring_view> keys,
                       const winui::Media::Brush& brush)
    {
        for (const auto& key : keys)
        {
            combo.Resources().Insert(winrt::box_value(winrt::hstring{key}), brush);
        }
    }

    void remove_resources(const calendar_date_picker& combo, std::span<const std::wstring_view> keys)
    {
        for (const auto& key : keys)
        {
            combo.Resources().Remove(winrt::box_value(winrt::hstring{key}));
        }
    }

    // FrameworkElementExtensions.RefreshThemeResources: flip RequestedTheme away and back so the control
    // template re-resolves the resources just overridden — identical to picker_handler.cpp's helper.
    void refresh_theme_resources(const winui::FrameworkElement& element)
    {
        const auto previous = element.RequestedTheme();
        element.RequestedTheme(element.ActualTheme() == winui::ElementTheme::Dark ? winui::ElementTheme::Light
                                                                                  : winui::ElementTheme::Dark);
        element.RequestedTheme(previous);
    }

    // "Was this property explicitly set?" — see picker_handler.cpp/button_handler.cpp/label_handler.cpp
    // for why this must not be a value comparison ([[cpp-unset-color-sentinel-collision]]).
    bool is_set(const maui::core::i_view& view, std::string_view property)
    {
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        return bindable != nullptr && bindable->is_property_set(property);
    }

    // ---- date_time <-> Windows.Foundation.DateTime -------------------------------------------------
    // WinRT's DateTime is 100ns ticks since 1601-01-01 UTC, exposed to C++/WinRT as winrt::clock (a
    // chrono-compatible clock with from_sys/to_sys conversions to std::chrono::system_clock). The port's
    // date_time is a (sys_days, milliseconds-of-day) pair — sub-100ns precision is never needed here.
    winrt::Windows::Foundation::DateTime to_winrt_date_time(const maui::core::date_time& value)
    {
        const auto sys_tp = value.days() + value.time_of_day(); // sys_time<milliseconds>
        return winrt::clock::from_sys(sys_tp);
    }

    maui::core::date_time from_winrt_date_time(const winrt::Windows::Foundation::DateTime& value)
    {
        const auto sys_tp = winrt::clock::to_sys(value); // std::chrono::system_clock::time_point
        const std::chrono::sys_days day = std::chrono::floor<std::chrono::days>(sys_tp);
        const auto time_of_day = std::chrono::duration_cast<std::chrono::milliseconds>(sys_tp - day);
        return maui::core::date_time{day, time_of_day};
    }

    // DatePickerExtensions.UpdateMinimumDate's "no bound" fallback (see file-top deviation note 4).
    maui::core::date_time default_minimum_date()
    {
        const auto now = maui::core::date_time::now();
        const auto ymd = now.year_month_day();
        return maui::core::date_time{static_cast<int>(ymd.year()) - 100, static_cast<unsigned>(ymd.month()),
                                     static_cast<unsigned>(ymd.day())};
    }

    // DatePickerExtensions.UpdateMaximumDate's "no bound" fallback (see file-top deviation note 4).
    maui::core::date_time default_maximum_date()
    {
        return maui::core::date_time{9999, 12, 31};
    }

    // ---- CalendarDatePickerExtensions' DateTimeFormatter-template grammar (ToDateFormat) ------------
    // WinUI's CalendarDatePicker.DateFormat speaks the Windows.Globalization.DateTimeFormatting mini-
    // template language ("{month.integer}", "{day.integer(2)}", ...), NOT a .NET format string. Ported
    // 1:1 from CalendarDatePickerExtensions.cs so a CUSTOM Format (anything but "d"/""/"D", which
    // CheckDateFormat maps to string.Empty = "leave WinUI's own default template alone") renders with the
    // same day/month/year run-length rules the C# does.
    bool is_blank(std::string_view text)
    {
        return std::ranges::all_of(text, [](unsigned char ch) { return std::isspace(ch) != 0; });
    }

    // CheckDateFormat: IsNullOrWhiteSpace(format) || format == "d".
    bool check_date_format(std::string_view format)
    {
        return format.empty() || is_blank(format) || format == "d";
    }

    std::size_t count_char_ci(std::string_view text, char c)
    {
        return static_cast<std::size_t>(std::ranges::count_if(
            text, [c](unsigned char ch) { return std::tolower(ch) == std::tolower(static_cast<unsigned char>(c)); }));
    }

    bool contains_ci(std::string_view text, char c)
    {
        return count_char_ci(text, c) > 0;
    }

    std::string get_day_format(std::string_view part)
    {
        if (check_date_format(part))
        {
            return "{day.integer}";
        }
        if (part == "D")
        {
            return "{dayofweek.full}";
        }
        const auto count = count_char_ci(part, 'd');
        if (count == 3)
        {
            return "{dayofweek.abbreviated}";
        }
        if (count == 4)
        {
            return "{dayofweek.full}";
        }
        return "{day.integer(" + std::to_string(count) + ")}";
    }

    std::string get_month_format(std::string_view part)
    {
        if (check_date_format(part))
        {
            return "{month}";
        }
        if (part == "D")
        {
            return "{month.full}";
        }
        const auto count = count_char_ci(part, 'm');
        if (count <= 2)
        {
            return "{month.integer(" + std::to_string(count) + ")}";
        }
        return count == 3 ? "{month.abbreviated}" : "{month.full}";
    }

    std::string get_year_format(std::string_view part)
    {
        if (check_date_format(part))
        {
            return "{year}";
        }
        if (part == "D")
        {
            return "{year.full}";
        }
        return count_char_ci(part, 'y') <= 2 ? "{year.abbreviated}" : "{year.full}";
    }

    std::string get_part(std::string_view part)
    {
        if (contains_ci(part, 'd'))
        {
            return get_day_format(part);
        }
        if (contains_ci(part, 'm'))
        {
            return get_month_format(part);
        }
        if (contains_ci(part, 'y'))
        {
            return get_year_format(part);
        }
        return {};
    }

    // GetSeparator: the first of '/','-',' ','.' present in the format; '\0' = none (single token).
    char get_separator(std::string_view format)
    {
        for (const char candidate : {'/', '-', ' ', '.'})
        {
            if (format.find(candidate) != std::string_view::npos)
            {
                return candidate;
            }
        }
        return '\0';
    }

    std::string to_date_format(std::string_view format)
    {
        if (check_date_format(format))
        {
            return {};
        }
        const char separator = get_separator(format);
        if (separator == '\0')
        {
            return get_part(format);
        }
        std::string result;
        std::size_t start = 0;
        for (;;)
        {
            const auto pos = format.find(separator, start);
            const bool is_last = pos == std::string_view::npos;
            result += get_part(format.substr(start, is_last ? std::string_view::npos : pos - start));
            if (is_last)
            {
                break;
            }
            result += separator;
            start = pos + 1;
        }
        return result;
    }

    // DatePickerExtensions.UpdateTextColor, shared by MapTextColor and UpdateDate's tail call (C# calls
    // UpdateTextColor from both — see update_date below).
    void push_text_color(const calendar_date_picker& combo, const maui::core::i_date_picker& view,
                         const maui::graphics::color& text_color)
    {
        // An UNSET TextColor must leave the theme brush alone rather than paint transparent black — see
        // [[cpp-unset-color-sentinel-collision]] and picker_handler.cpp's identical map_text_color.
        if (!is_set(view, "text_color"))
        {
            remove_resources(combo, k_text_color_keys);
            combo.ClearValue(winui::Controls::Control::ForegroundProperty());
            refresh_theme_resources(combo);
            return;
        }
        const winui::Media::SolidColorBrush brush{maui::platform::windows::to_ui_color(text_color)};
        set_resources(combo, k_text_color_keys, brush);
        combo.Foreground(brush);
        refresh_theme_resources(combo);
    }

    // DatePickerExtensions.UpdateDate, shared by MapFormat and MapDate exactly like the headless twin's
    // update_date — a Format change re-renders through the SAME function a Date change does.
    void update_date(maui::core::date_picker_handler& handler, maui::core::i_date_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        const calendar_date_picker combo = as_calendar_date_picker(platform->native);
        const auto date = view.date();
        platform->date = date.value_or(maui::core::date_time::today());
        if (!date.has_value())
        {
            combo.Date(nullptr);
            platform->text.clear();
        }
        else
        {
            // Pass the DateTime STRAIGHT in, do NOT box_value it. CalendarDatePicker.Date projects as
            // IReference<DateTime>, and C++/WinRT gives that a converting constructor from the value
            // itself; box_value returns an IInspectable, whose conversion resolves to IReference's
            // PRIVATE constructor -- error C2248 "cannot access private member declared in class
            // 'winrt::Windows::Foundation::IReference<...DateTime>'", which does not read as "drop the
            // box_value". The null case above already passes nullptr, which is the same projection's
            // empty state.
            combo.Date(to_winrt_date_time(*date));
            const std::string custom_format = to_date_format(view.format());
            if (!custom_format.empty())
            {
                combo.DateFormat(maui::platform::windows::to_hstring(custom_format));
            }
            const std::string_view format = view.format();
            platform->text = maui::core::format_date_time(
                *date, (format.empty() || format == "d") ? "d" : (format == "D" ? "D" : format));
        }
        push_text_color(combo, view, view.text_color());
    }
} // namespace

namespace maui::core
{
    namespace
    {
        // Unhook everything on_connect_handler registered — same rationale as picker_handler.cpp's
        // detach_native_events: the lambdas capture the handler, so an undisconnected teardown must not
        // leave them subscribed.
        void detach_native_events(date_picker_platform& platform)
        {
            if (platform.native != nullptr)
            {
                const calendar_date_picker combo = as_calendar_date_picker(platform.native);
                combo.Opened(winrt::event_token{platform.opened_token});
                combo.Closed(winrt::event_token{platform.closed_token});
                combo.DateChanged(winrt::event_token{platform.date_changed_token});
            }
            platform.opened_token = 0;
            platform.closed_token = 0;
            platform.date_changed_token = 0;
        }
    } // namespace

    date_picker_platform::~date_picker_platform()
    {
        detach_native_events(*this);
        if (native != nullptr)
        {
            maui::platform::windows::drop<winui::UIElement>(native);
        }
    }

    std::unique_ptr<date_picker_platform> date_picker_handler::create_platform_view()
    {
        auto platform = std::make_unique<date_picker_platform>();
        calendar_date_picker combo;
        platform->native = maui::platform::windows::take<winui::UIElement>(combo);
        return platform;
    }

    void date_picker_handler::on_connect_handler(date_picker_platform& platform)
    {
        if (platform.native == nullptr)
        {
            return;
        }
        auto* self = this;
        const calendar_date_picker combo = as_calendar_date_picker(platform.native);

        // Opened/Closed: `void Opened(object? sender, object e)` / `void Closed(...)` in C# — both push
        // IsOpen back to the virtual view, the DatePicker analog of picker_handler.cpp's
        // DropDownOpened/DropDownClosed (minus the MinWidth pin, which is Picker/ComboBox-specific).
        platform.opened_token = combo
                                    .Opened([self](const winrt::Windows::Foundation::IInspectable&,
                                                   const winrt::Windows::Foundation::IInspectable&) {
                                        if (auto* view = self->virtual_view())
                                        {
                                            view->set_is_open(true);
                                        }
                                    })
                                    .value;

        platform.closed_token = combo
                                    .Closed([self](const winrt::Windows::Foundation::IInspectable&,
                                                   const winrt::Windows::Foundation::IInspectable&) {
                                        if (auto* view = self->virtual_view())
                                        {
                                            view->set_is_open(false);
                                        }
                                    })
                                    .value;

        // DateChanged: the native-pick commit channel (there is no separate "Done" tap on a calendar
        // flyout — picking a day fires this directly). Mirrors DatePickerHandler.Windows.cs's DateChanged:
        // a null NewDate clears VirtualView.Date; otherwise it is pushed back ONLY if it actually differs
        // (the oracle's own guard — avoids a redundant round trip through the property mapper).
        platform.date_changed_token =
            combo
                .DateChanged([self](const winrt::Windows::Foundation::IInspectable&,
                                    const winui::Controls::CalendarDatePickerDateChangedEventArgs& args) {
                    auto* view = self->virtual_view();
                    if (view == nullptr)
                    {
                        return;
                    }
                    const auto new_date_ref = args.NewDate();
                    if (!new_date_ref)
                    {
                        view->set_date(std::nullopt);
                        return;
                    }
                    const auto new_date = from_winrt_date_time(new_date_ref.Value());
                    const auto current = view->date();
                    if (!current.has_value() || *current != new_date)
                    {
                        view->set_date(new_date);
                    }
                })
                .value;
    }

    void date_picker_handler::on_disconnect_handler(date_picker_platform& platform)
    {
        detach_native_events(platform);
    }

    void date_picker_handler::map_format(date_picker_handler& handler, i_date_picker& view)
    {
        update_date(handler, view); // UpdateFormat routes into UpdateDate (re-render through DateFormat)
    }

    void date_picker_handler::map_date(date_picker_handler& handler, i_date_picker& view)
    {
        update_date(handler, view);
    }

    void date_picker_handler::map_minimum_date(date_picker_handler& handler, i_date_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        const auto minimum = view.minimum_date();
        platform->minimum_date = minimum;
        as_calendar_date_picker(platform->native).MinDate(to_winrt_date_time(minimum.value_or(default_minimum_date())));
    }

    void date_picker_handler::map_maximum_date(date_picker_handler& handler, i_date_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        const auto maximum = view.maximum_date();
        platform->maximum_date = maximum;
        as_calendar_date_picker(platform->native).MaxDate(to_winrt_date_time(maximum.value_or(default_maximum_date())));
    }

    void date_picker_handler::map_text_color(date_picker_handler& handler, i_date_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->text_color = view.text_color();
        push_text_color(as_calendar_date_picker(platform->native), view, platform->text_color);
    }

    void date_picker_handler::map_font(date_picker_handler& handler, i_date_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->text_font = view.font();
        const calendar_date_picker combo = as_calendar_date_picker(platform->native);
        const font& f = platform->text_font;
        // ALWAYS assign, never skip — see picker_handler.cpp/label_handler.cpp's identical map_font note:
        // C#'s UpdateFont resolves fontManager.GetFontSize/GetFontFamily unconditionally, falling back to
        // the framework defaults (winui_interop's default_font_size()/default_font_family()) when unset.
        combo.FontSize(f.size() > 0 ? f.size() : maui::platform::windows::default_font_size());
        combo.FontFamily(f.family().empty()
                             ? maui::platform::windows::default_font_family()
                             : winui::Media::FontFamily{maui::platform::windows::to_hstring(f.family())});
        combo.FontStyle(f.slant() == maui::core::font_slant::italic    ? winrt::Windows::UI::Text::FontStyle::Italic
                        : f.slant() == maui::core::font_slant::oblique ? winrt::Windows::UI::Text::FontStyle::Oblique
                                                                       : winrt::Windows::UI::Text::FontStyle::Normal);
        combo.FontWeight(winrt::Windows::UI::Text::FontWeight{static_cast<std::uint16_t>(f.weight())});
        // No IsTextScaleFactorEnabled push — see the file-top deviation note 3.
    }

    void date_picker_handler::map_character_spacing(date_picker_handler& handler, i_date_picker& view)
    {
        // Mirror only — see the file-top deviation note 2 (the "DateText" descendant TextBlock reach
        // needs a VisualTreeHelper walk this backend does not have yet).
        if (auto* platform = handler.typed_platform_view())
        {
            platform->character_spacing = view.character_spacing();
        }
    }

    void date_picker_handler::map_is_open(date_picker_handler& handler, i_date_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        // UpdateIsOpen: platformDatePicker.IsCalendarOpen = datePicker.IsOpen. No IsLoaded-deferred retry
        // in this oracle (unlike PickerExtensions.UpdateIsOpen's ComboBox.IsDropDownOpen) — the property
        // is fine to set before the control is loaded.
        as_calendar_date_picker(platform->native).IsCalendarOpen(view.is_open());
    }

    maui::graphics::size date_picker_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        // GetDesiredSizeFromHandler + AdjustForExplicitSize (ViewHandlerExtensions.Windows.cs:56-105) —
        // the shared base-ViewHandler measure path every Windows control uses, mirrored identically to
        // picker_handler.cpp's get_desired_size (see its comment for the full writeup).
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        if (width_constraint < 0 || height_constraint < 0)
        {
            return {0, 0};
        }
        const calendar_date_picker combo = as_calendar_date_picker(platform->native);
        const auto* view = virtual_view();
        const double explicit_width = (view != nullptr) ? view->width() : std::numeric_limits<double>::quiet_NaN();
        const double explicit_height = (view != nullptr) ? view->height() : std::numeric_limits<double>::quiet_NaN();
        combo.Width(explicit_width);
        combo.Height(explicit_height);
        const double adjusted_width_constraint =
            std::isnan(explicit_width) ? width_constraint : std::max(width_constraint, explicit_width);
        const double adjusted_height_constraint =
            std::isnan(explicit_height) ? height_constraint : std::max(height_constraint, explicit_height);
        combo.Measure(
            winrt::Windows::Foundation::Size{maui::platform::windows::measure_constraint(adjusted_width_constraint),
                                             maui::platform::windows::measure_constraint(adjusted_height_constraint)});
        const auto desired = combo.DesiredSize();
        return {desired.Width, desired.Height};
    }

    void date_picker_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        // PlatformArrangeHandler's guard, WIDENED to non-finite — see button_handler.cpp/picker_handler.cpp's
        // platform_arrange for why (a NaN reaching XAML's arrange is an unrecoverable stowed exception).
        if (!std::isfinite(frame.x) || !std::isfinite(frame.y) || !std::isfinite(frame.width) ||
            !std::isfinite(frame.height) || frame.width < 0 || frame.height < 0)
        {
            return;
        }
        const calendar_date_picker combo = as_calendar_date_picker(platform->native);
        winui::Controls::Canvas::SetLeft(combo, frame.x);
        winui::Controls::Canvas::SetTop(combo, frame.y);
        combo.Width(frame.width);
        combo.Height(frame.height);
        // Clip is bounds-dependent (view_chrome_ops.cpp's apply_native_clip reads the just-set Width/
        // Height back) — same re-invoke-after-arrange as picker_handler.cpp's platform_arrange.
        if (const auto* view = virtual_view(); view != nullptr)
        {
            apply_native_clip(platform->native, view->clip());
        }
    }

    // ---- generic-IView property pushes (view_platform_base overrides) ---------------------------
    // Delegated to the shared winui_visual_ops free functions so every control behaves identically; see
    // that header for why they are free functions taking the void* slot.
    void date_picker_platform::update_visibility(maui::core::visibility value)
    {
        maui::platform::windows::apply_visibility(native, value);
    }

    void date_picker_platform::update_opacity(double value)
    {
        maui::platform::windows::apply_opacity(native, value);
    }

    void date_picker_platform::update_is_enabled(bool value)
    {
        maui::platform::windows::apply_is_enabled(native, value);
    }

    void date_picker_platform::update_automation_id(std::string_view value)
    {
        maui::platform::windows::apply_automation_id(native, value);
    }

    void date_picker_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::windows::apply_background(native, value);
    }
} // namespace maui::core
