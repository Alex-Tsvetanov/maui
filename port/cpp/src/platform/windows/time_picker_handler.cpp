// time_picker_handler — WinUI 3 platform recipe: a real Microsoft.UI.Xaml.Controls.TimePicker, the same
// native type TimePickerHandler.Windows.cs creates. Ported from TimePickerHandler.Windows.cs +
// TimePickerExtensions.cs.
//
// DOCUMENTED SIMPLIFICATIONS against C#, each narrowed on purpose rather than left vague:
//
//  1. CharacterSpacing's per-TextBlock push (UpdateCharacterSpacingInTimePicker, TimePickerExtensions.cs:
//     96-113) additionally reaches into the control template for the HourTextBlock/MinuteTextBlock/
//     PeriodTextBlock parts via a Loaded-deferred VisualTreeHelper descendant-by-name walk
//     (GetDescendantByName). This backend has no descendant-lookup helper yet — the same documented gap
//     entry_handler.cpp's header records for MauiTextBox's placeholder TextBlock/ScrollViewer reach.
//     map_character_spacing below still pushes the real, un-narrowed Control.CharacterSpacing property
//     (which the stock template's parts TemplateBind to), so a freshly-created TimePicker still gets the
//     value at first template application; only a RETROACTIVE re-push onto an already-templated live
//     control's cached text blocks is skipped.
//  2. IsTextScaleFactorEnabled. ControlExtensions.UpdateFont assigns this on every `Control`, but per
//     button_handler.cpp's map_font note, this WinRT projection only exposes the property on
//     `TextBlock` — a TimePicker has no single content TextBlock this handler owns, so this push is
//     skipped, matching picker_handler.cpp's identical skip for the same reason.
//  3. CORRECTED (was a bug, not a simplification): an earlier version of this file believed Background
//     was unstated for this handler and let it fall through to the generic five-override
//     apply_background push (a plain Control.Background set). That is wrong — TimePickerHandler.cs's
//     Mapper DOES carry `#if ANDROID || WINDOWS [nameof(ITimePicker.Background)] = MapBackground`, and
//     Windows's MapBackground calls TimePickerExtensions.UpdateBackground, which overrides the
//     TimePickerButtonBackground*/PointerOver/Pressed/Disabled/Focused resource keys the control
//     template's per-visual-state brushes bind to (a plain Control.Background alone is dropped the
//     instant the template resolves those keys — same shape as k_text_color_keys below, and the same
//     shape slider_handler.cpp's update_background already carries for Slider's analogous Windows-only
//     `MapBackgroundColor` remap). push_background below reproduces UpdateBackground exactly: the
//     resource-key set AND the plain Background property. time_picker_platform::update_background now
//     calls it instead of the shared winui_visual_ops::apply_background.
//  4. map_is_open's "open" branch (TimePickerExtensions.cs:138-159): WinUI's TimePicker exposes no direct
//     "open the flyout" boolean (unlike ComboBox.IsDropDownOpen), so MAUI itself opens it by locating the
//     control's automation peer, walking its children for one whose automation class name contains
//     "Button", and invoking it through UI Automation's Invoke pattern — reproduced as faithfully as
//     possible below. This is the single most exotic WinRT surface in this file and could not be verified
//     against a compiler (the Windows guest was mid-capture); if IsOpen fails to open the flyout, this is
//     the first place to check against TimePickerExtensions.cs:138-159.

#include "maui/core/time_picker_handler.hpp"

#include <winrt/Microsoft.UI.Xaml.Automation.Peers.h>
#include <winrt/Microsoft.UI.Xaml.Automation.Provider.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <span>
#include <string>
#include <string_view>

#include "maui/core/bindable_object.hpp"
#include "maui/core/date_time.hpp"
#include "maui/core/i_time_picker.hpp"
#include "maui/core/view_chrome_ops.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "winui_interop.hpp"
#include "winui_visual_ops.hpp"

namespace
{
    // Named `winui`, NOT `xaml`: the port already has a maui::xaml namespace (the XAML loader), and
    // inside namespace maui::* that name WINS over a file-scope alias.
    namespace winui = winrt::Microsoft::UI::Xaml;
    using time_picker_control = winui::Controls::TimePicker;

    time_picker_control as_time_picker(void* native)
    {
        return maui::platform::windows::ref<winui::UIElement>(native).as<time_picker_control>();
    }

    // TimePickerExtensions.cs:70-76's TextColorResourceKeys — see picker_handler.cpp's identical
    // k_text_color_keys pattern for WHY a local Foreground alone is insufficient: the control template
    // binds per-visual-state brushes to these theme resources, so an unset override is dropped again the
    // moment the pointer enters/leaves/presses the control.
    constexpr std::array<std::wstring_view, 4> k_text_color_keys{
        L"TimePickerButtonForeground", L"TimePickerButtonForegroundPointerOver", L"TimePickerButtonForegroundPressed",
        L"TimePickerButtonForegroundDisabled"};

    // TimePickerExtensions.cs's BackgroundColorResourceKeys — see the file-top deviation note 3.
    constexpr std::array<std::wstring_view, 5> k_background_keys{
        L"TimePickerButtonBackground", L"TimePickerButtonBackgroundPointerOver", L"TimePickerButtonBackgroundPressed",
        L"TimePickerButtonBackgroundDisabled", L"TimePickerButtonBackgroundFocused"};

    void set_resources(const time_picker_control& native, std::span<const std::wstring_view> keys,
                       const winui::Media::Brush& brush)
    {
        for (const auto& key : keys)
        {
            native.Resources().Insert(winrt::box_value(winrt::hstring{key}), brush);
        }
    }

    void remove_resources(const time_picker_control& native, std::span<const std::wstring_view> keys)
    {
        for (const auto& key : keys)
        {
            native.Resources().Remove(winrt::box_value(winrt::hstring{key}));
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

    // "Was this property explicitly set?" — see the twin in button_handler.cpp/picker_handler.cpp for why
    // this must not be a value comparison ([[cpp-unset-color-sentinel-collision]]).
    bool is_set(const maui::core::i_view& view, std::string_view property)
    {
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        return bindable != nullptr && bindable->is_property_set(property);
    }

    // TimePickerExtensions.UpdateBackground — see the file-top deviation note 3. Keyed purely off `value`
    // being null (matching C#'s `timePicker.Background is null` check, not the bindable is-set flag).
    void push_background(const time_picker_control& native, const maui::graphics::paint* value)
    {
        if (value == nullptr)
        {
            remove_resources(native, k_background_keys);
            native.ClearValue(winui::Controls::Control::BackgroundProperty());
            refresh_theme_resources(native);
            return;
        }
        const winui::Media::Brush brush = maui::platform::windows::brush_for(*value);
        set_resources(native, k_background_keys, brush);
        native.Background(brush);
        refresh_theme_resources(native);
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
        // ~time_picker_platform, same rationale as picker_handler.cpp's detach_native_events.
        void detach_native_events(time_picker_platform& platform)
        {
            if (platform.native != nullptr)
            {
                as_time_picker(platform.native)
                    .SelectedTimeChanged(winrt::event_token{platform.selected_time_changed_token});
            }
            platform.selected_time_changed_token = 0;
        }

        // TimePickerExtensions.UpdateTime, shared by MapFormat and MapTime (C# calls the same UpdateTime
        // from both). Pushes SelectedTime (a null Time clears the native selection — no zero-fallback,
        // unlike the headless mirror's zero-defaulted `time` field below) and ClockIdentifier.
        void update_time(time_picker_handler& handler, i_time_picker& view)
        {
            auto* platform = handler.typed_platform_view();
            if (platform == nullptr || platform->native == nullptr)
            {
                return;
            }
            const auto time = view.time();
            platform->time = time.value_or(time_span{});
            platform->text = time.has_value() ? format_time_span(*time, view.format()) : std::string{};

            const time_picker_control native = as_time_picker(platform->native);
            if (time.has_value())
            {
                // TimeSpan (ms) -> Windows::Foundation::TimeSpan (100ns ticks); the value implicitly
                // boxes into the native's IReference<TimeSpan> property, the same pattern
                // scroll_view_handler.cpp's ChangeView call documents for IReference<double>.
                native.SelectedTime(std::chrono::duration_cast<winrt::Windows::Foundation::TimeSpan>(time->value()));
            }
            else
            {
                native.SelectedTime(nullptr);
            }
            // Format.Contains('H') (ordinal) selects the 24-hour clock face; anything else (including an
            // empty/unset Format) is 12-hour — TimePickerExtensions.cs:18-25. WinUI's ClockIdentifier is
            // NOT a .NET format string — it only toggles 12h/24h, so the rest of `view.format()` (e.g.
            // "hh\\:mm" vs "HH\\:mm") has no further effect on the native control beyond this one check.
            native.ClockIdentifier(view.format().find('H') != std::string_view::npos ? winrt::hstring{L"24HourClock"}
                                                                                     : winrt::hstring{L"12HourClock"});
        }
    } // namespace

    time_picker_platform::~time_picker_platform()
    {
        detach_native_events(*this);
        if (native != nullptr)
        {
            maui::platform::windows::drop<winui::UIElement>(native);
        }
    }

    std::unique_ptr<time_picker_platform> time_picker_handler::create_platform_view()
    {
        auto platform = std::make_unique<time_picker_platform>();
        time_picker_control native;
        platform->native = maui::platform::windows::take<winui::UIElement>(native);
        return platform;
    }

    void time_picker_handler::on_connect_handler(time_picker_platform& platform)
    {
        if (platform.native == nullptr)
        {
            return;
        }
        auto* self = this;
        const time_picker_control native = as_time_picker(platform.native);
        // OnSelectedTimeChanged: write the native pick back, seconds included this time (unlike the
        // headless Done-tap analog, TimePickerSelectedValueChangedEventArgs.NewTime is not truncated by
        // this handler — TimePickerHandler.Windows.cs:58-65 assigns it straight through), then
        // InvalidateMeasure (a differently-formatted time can change the control's natural width).
        platform.selected_time_changed_token =
            native
                .SelectedTimeChanged([self](const time_picker_control&,
                                            const winui::Controls::TimePickerSelectedValueChangedEventArgs& args) {
                    auto* view = self->virtual_view();
                    if (view == nullptr)
                    {
                        return;
                    }
                    // args.NewTime() is IReference<TimeSpan>, NOT TimeSpan -- duration_cast has no overload
                    // for it (error C2672). Unwrap first, and propagate the EMPTY state rather than
                    // collapsing it to zero: the oracle assigns `VirtualView.Time = e.NewTime` where BOTH
                    // sides are nullable (ITimePicker.cs:34 `TimeSpan? Time`, and this port's
                    // i_time_picker::time() is std::optional<time_span>), so a cleared native selection
                    // must clear the virtual view too, not read back as midnight.
                    const auto new_time = args.NewTime();
                    view->set_time(new_time != nullptr
                                       ? std::optional<time_span>{time_span{
                                             std::chrono::duration_cast<std::chrono::milliseconds>(new_time.Value())}}
                                       : std::nullopt);
                    view->invalidate_measure();
                })
                .value;
    }

    void time_picker_handler::on_disconnect_handler(time_picker_platform& platform)
    {
        detach_native_events(platform);
    }

    void time_picker_handler::map_format(time_picker_handler& handler, i_time_picker& view)
    {
        update_time(handler, view); // UpdateFormat routes into UpdateTime (re-renders the clock face)
    }

    void time_picker_handler::map_time(time_picker_handler& handler, i_time_picker& view)
    {
        update_time(handler, view);
    }

    void time_picker_handler::map_text_color(time_picker_handler& handler, i_time_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->text_color = view.text_color();
        const time_picker_control native = as_time_picker(platform->native);
        // An UNSET TextColor must leave the theme brush alone rather than paint transparent black — see
        // [[cpp-unset-color-sentinel-collision]] and picker_handler.cpp's identical map_text_color.
        if (!is_set(view, "text_color"))
        {
            remove_resources(native, k_text_color_keys);
            native.ClearValue(winui::Controls::Control::ForegroundProperty());
            refresh_theme_resources(native);
            return;
        }
        const winui::Media::SolidColorBrush brush{maui::platform::windows::to_ui_color(platform->text_color)};
        set_resources(native, k_text_color_keys, brush);
        native.Foreground(brush);
        refresh_theme_resources(native);
    }

    void time_picker_handler::map_font(time_picker_handler& handler, i_time_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->text_font = view.font();
        const time_picker_control native = as_time_picker(platform->native);
        const font& f = platform->text_font;
        // ALWAYS assign, never skip — see picker_handler.cpp's identical map_font note: C#'s UpdateFont
        // resolves fontManager.GetFontSize/GetFontFamily unconditionally, falling back to the framework
        // defaults (winui_interop's default_font_size()/default_font_family()) when unset.
        native.FontSize(f.size() > 0 ? f.size() : maui::platform::windows::default_font_size());
        native.FontFamily(f.family().empty()
                              ? maui::platform::windows::default_font_family()
                              : winui::Media::FontFamily{maui::platform::windows::to_hstring(f.family())});
        native.FontStyle(to_font_style(f.slant()));
        native.FontWeight(to_font_weight(f.weight()));
        // No IsTextScaleFactorEnabled push — see the file-top deviation note 2.
    }

    void time_picker_handler::map_character_spacing(time_picker_handler& handler, i_time_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->character_spacing = view.character_spacing();
        // CharacterSpacingExtensions.ToEm: pt * 0.0624 * 1000, in 1/1000 em units (picker_handler.cpp's
        // identical map_character_spacing).
        const auto em = static_cast<std::int32_t>(std::lround(platform->character_spacing * 0.0624 * 1000.0));
        as_time_picker(platform->native).CharacterSpacing(em);
        // NOT REPLICATED: the extra Loaded-deferred push onto the HourTextBlock/MinuteTextBlock/
        // PeriodTextBlock template parts — see the file-top deviation note 1.
    }

    void time_picker_handler::map_is_open(time_picker_handler& handler, i_time_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        const time_picker_control native = as_time_picker(platform->native);
        if (!native.IsLoaded())
        {
            // UpdateIsOpen's self-unsubscribing Loaded handler (TimePickerExtensions.cs:126-136) — the
            // same re-run-once-loaded pattern as picker_handler.cpp's map_is_open.
            auto token = std::make_shared<winrt::event_token>();
            auto* self = &handler;
            *token = native.Loaded(
                [native, token, self](const winrt::Windows::Foundation::IInspectable&, const winui::RoutedEventArgs&) {
                    native.Loaded(*token);
                    if (auto* live_view = self->virtual_view())
                    {
                        map_is_open(*self, *live_view);
                    }
                });
            return;
        }

        if (!view.is_open())
        {
            // Lost the WinUI TimePicker focus (TimePickerExtensions.cs:162-166): move focus to the parent.
            if (const auto parent = winui::Media::VisualTreeHelper::GetParent(native).try_as<winui::UIElement>())
            {
                parent.Focus(winui::FocusState::Programmatic);
            }
            return;
        }

        native.Focus(winui::FocusState::Programmatic);

        // See the file-top deviation note 4: the automation-peer Invoke walk (TimePickerExtensions.cs:
        // 142-159), reproduced as faithfully as this projection allows.
        winui::Automation::Peers::AutomationPeer peer =
            winui::Automation::Peers::FrameworkElementAutomationPeer::CreatePeerForElement(native);
        if (!peer)
        {
            peer = winui::Automation::Peers::TimePickerAutomationPeer(native);
        }
        const auto children = peer.GetChildren();
        if (!children)
        {
            return;
        }
        for (const auto& child : children)
        {
            // ponytail: a plain case-sensitive find is enough here — automation peer class names are
            // always PascalCase ("ButtonAutomationPeer"), so C#'s OrdinalIgnoreCase never actually matters.
            if (maui::platform::windows::to_utf8(child.GetClassName()).find("Button") == std::string::npos)
            {
                continue;
            }
            if (auto invoke = child.GetPattern(winui::Automation::Peers::PatternInterface::Invoke)
                                  .try_as<winui::Automation::Provider::IInvokeProvider>())
            {
                invoke.Invoke();
                return;
            }
        }
    }

    maui::graphics::size time_picker_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        // GetDesiredSizeFromHandler's first guard: a negative constraint measures to nothing.
        if (width_constraint < 0 || height_constraint < 0)
        {
            return {0, 0};
        }
        const time_picker_control native = as_time_picker(platform->native);
        // ARRANGE/EXPLICIT-SIZE FIX (picker_handler.cpp's identical get_desired_size; see its comment for
        // the full writeup and the ViewHandlerExtensions.Windows.cs oracle citation).
        const auto* view = virtual_view();
        const double explicit_width = (view != nullptr) ? view->width() : std::numeric_limits<double>::quiet_NaN();
        const double explicit_height = (view != nullptr) ? view->height() : std::numeric_limits<double>::quiet_NaN();
        native.Width(explicit_width);
        native.Height(explicit_height);
        const double adjusted_width_constraint =
            std::isnan(explicit_width) ? width_constraint : std::max(width_constraint, explicit_width);
        const double adjusted_height_constraint =
            std::isnan(explicit_height) ? height_constraint : std::max(height_constraint, explicit_height);
        native.Measure(
            winrt::Windows::Foundation::Size{maui::platform::windows::measure_constraint(adjusted_width_constraint),
                                             maui::platform::windows::measure_constraint(adjusted_height_constraint)});
        const auto desired = native.DesiredSize();
        return {desired.Width, desired.Height};
    }

    void time_picker_handler::platform_arrange(const maui::graphics::rect& frame)
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
        const time_picker_control native = as_time_picker(platform->native);
        winui::Controls::Canvas::SetLeft(native, frame.x);
        winui::Controls::Canvas::SetTop(native, frame.y);
        const auto* const view = virtual_view();
        // SHRINK-WRAP THE WIDTH -- see date_picker_handler.cpp's platform_arrange for the full rationale and
        // the oracle citation (ViewHandlerExtensions.Windows.cs:76-89: MAUI's arrange calls Arrange(rect) and
        // never assigns Width, so WinUI honours the control's own non-Stretch default alignment).
        // This control MANIFESTS the same bug differently, which is worth recording: the port rendered the
        // TimePicker 456 DIP wide and CENTRED (x 284..739) where MAUI renders 242 DIP left-aligned
        // (x 20..261) -- WinUI's TimePicker template does not fill a pinned width the way
        // CalendarDatePicker's does, it centres inside it. Same cause, different symptom.
        // PER-HANDLER ON PURPOSE: pinning stays correct for the stretch-by-default controls
        // (picker_handler.cpp's ComboBox, entry_handler.cpp's TextBox), so do not hoist this.
        const double natural = view != nullptr ? view->desired_size().width : frame.width;
        native.Width(natural > 0 ? std::min(frame.width, natural) : frame.width);
        native.Height(frame.height);
        // Clip is bounds-dependent; map_clip's own push (view_mapper.cpp) always runs before the first
        // arrange, so this re-invoke is what actually installs the clip once the control has a real size
        // (picker_handler.cpp's identical platform_arrange).
        if (view != nullptr)
        {
            apply_native_clip(platform->native, view->clip());
        }
    }

    // ---- generic-IView property pushes (view_platform_base overrides) ---------------------------
    // Delegated to the shared winui_visual_ops free functions so every control behaves identically; see
    // that header for why they are free functions taking the void* slot.
    void time_picker_platform::update_visibility(maui::core::visibility value)
    {
        maui::platform::windows::apply_visibility(native, value);
    }

    void time_picker_platform::update_opacity(double value)
    {
        maui::platform::windows::apply_opacity(native, value);
    }

    void time_picker_platform::update_is_enabled(bool value)
    {
        maui::platform::windows::apply_is_enabled(native, value);
    }

    void time_picker_platform::update_automation_id(std::string_view value)
    {
        maui::platform::windows::apply_automation_id(native, value);
    }

    void time_picker_platform::update_background(const maui::graphics::paint* value)
    {
        // NOT the generic apply_background push — see the file-top deviation note 3 and push_background's
        // own comment (same shape as slider_handler.cpp's update_background override).
        if (native == nullptr)
        {
            return;
        }
        push_background(as_time_picker(native), value);
    }
} // namespace maui::core
