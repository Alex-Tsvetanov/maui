// check_box_handler — WinUI 3 platform recipe: a real Microsoft.UI.Xaml.Controls.CheckBox, the same
// native type CheckBoxHandler.Windows.cs creates (a BARE `CheckBox`, not a Maui* subclass — unlike
// MauiButton/MauiTextBox/MauiPasswordTextBox, CheckBox has no Windows-only wrapper type). Ported from
// CheckBoxHandler.Windows.cs + CheckBoxExtensions.cs (the shared CheckBoxHandler.cs mapper table has no
// Windows-only entries beyond IsChecked/Foreground).
//
// DOCUMENTED SIMPLIFICATIONS against C#, each narrowed on purpose rather than left vague:
//
//  1. AdjustCheckBoxForNoText's OnCheckBoxLoaded reaches into the control template for its root Grid via
//     `VisualTreeHelper.GetChild(checkBox, 0)` — an INDEXED child access, not the NAMED
//     descendant-by-name walk this backend documents as missing elsewhere (date_picker_handler.cpp /
//     entry_handler.cpp / search_bar_handler.cpp / editor_handler.cpp's "DateText"/"PlaceholderText.../
//     "DeleteButton" reaches — those need a recursive name search this backend does not have).
//     `VisualTreeHelper::GetChild`/`GetChildrenCount` are plain indexed calls (the same class
//     time_picker_handler.cpp already calls `GetParent` on), so this slice PORTS it: shrink the
//     just-loaded CheckBox's template root Grid margin from its default to
//     `(CheckBoxHeight - CheckBoxSize) / 2` on every side, matching C# exactly. Skipping this would leave
//     a text-less checkbox's measured/arranged box noticeably larger than MAUI's actual render (the SIZE
//     concern this control was flagged for), so it is ported rather than deferred.
//  2. IsTextScaleFactorEnabled. ControlExtensions.UpdateFont assigns this on every `Control`, but per
//     button_handler.cpp's map_font note, this WinRT projection only exposes the property on
//     `TextBlock` — CheckBox has no font/text mapper entry at all in CheckBoxHandler.cs's shared Mapper
//     (IsChecked + Foreground only), so this deviation note is inherited but never actually reached.

#include "maui/core/check_box_handler.hpp"

// ToggleButton (Checked/Unchecked/IsChecked) lives in Controls.Primitives, inherited by CheckBox
// (Controls) — the C++/WinRT include rule: without the FULL header those members are only
// forward-declared and every call is C3779, the exact trap picker_handler.cpp's ComboBox/Selector note
// documents (SelectedIndex/SelectionChanged there, Checked/Unchecked/IsChecked here).
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <span>
#include <string_view>

#include "maui/core/i_check_box.hpp"
#include "maui/core/view_chrome_ops.hpp"
#include "maui/graphics/paint.hpp"
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
    using check_box_control = winui::Controls::CheckBox;

    check_box_control as_check_box(void* native)
    {
        return maui::platform::windows::ref<winui::UIElement>(native).as<check_box_control>();
    }

    // CheckBoxExtensions.cs's `_tintColorResourceKeys` — the per-visual-state stroke/checked-fill brushes
    // the CheckBox control template binds to; a plain Foreground override alone is dropped the moment the
    // pointer enters/leaves/presses or the checked state flips (see button_handler.cpp/picker_handler.cpp's
    // identical k_text_color_keys note for why the resource-key set is needed at all).
    constexpr std::array<std::wstring_view, 16> k_tint_color_keys{
        L"CheckBoxCheckBackgroundFillChecked",
        L"CheckBoxCheckBackgroundFillCheckedPointerOver",
        L"CheckBoxCheckBackgroundFillCheckedPressed",
        L"CheckBoxCheckBackgroundFillCheckedDisabled",
        L"CheckBoxCheckBackgroundStrokeUnchecked",
        L"CheckBoxCheckBackgroundStrokeUncheckedPointerOver",
        L"CheckBoxCheckBackgroundStrokeUncheckedPressed",
        L"CheckBoxCheckBackgroundStrokeUncheckedDisabled",
        L"CheckBoxCheckBackgroundStrokeChecked",
        L"CheckBoxCheckBackgroundStrokeCheckedPointerOver",
        L"CheckBoxCheckBackgroundStrokeCheckedPressed",
        L"CheckBoxCheckBackgroundStrokeCheckedDisabled",
        L"CheckBoxCheckBackgroundStrokeIndeterminate",
        L"CheckBoxCheckBackgroundStrokeIndeterminatePointerOver",
        L"CheckBoxCheckBackgroundStrokeIndeterminatePressed",
        L"CheckBoxCheckBackgroundStrokeIndeterminateDisabled",
    };

    void set_resources(const check_box_control& checkbox, std::span<const std::wstring_view> keys,
                       const winui::Media::Brush& brush)
    {
        for (const auto& key : keys)
        {
            checkbox.Resources().Insert(winrt::box_value(winrt::hstring{key}), brush);
        }
    }

    void remove_resources(const check_box_control& checkbox, std::span<const std::wstring_view> keys)
    {
        for (const auto& key : keys)
        {
            checkbox.Resources().Remove(winrt::box_value(winrt::hstring{key}));
        }
    }

    // FrameworkElementExtensions.RefreshThemeResources: flip RequestedTheme away and back so the control
    // template re-resolves the resources just overridden — identical to button_handler.cpp/
    // picker_handler.cpp/date_picker_handler.cpp's helper of the same name.
    void refresh_theme_resources(const winui::FrameworkElement& element)
    {
        const auto previous = element.RequestedTheme();
        element.RequestedTheme(element.ActualTheme() == winui::ElementTheme::Dark ? winui::ElementTheme::Light
                                                                                  : winui::ElementTheme::Dark);
        element.RequestedTheme(previous);
    }

    // ResourceDictionaryExtensions.TryGet<double>(key): a live theme-resource lookup with a fallback for a
    // missing/mistyped key — same resolve-or-fallback shape as winui_interop.cpp's default_font_size(),
    // kept LOCAL rather than hoisted there: these two keys (CheckBoxHeight/CheckBoxSize) are checkbox-only,
    // and a sibling agent is concurrently porting radio_button_handler.cpp, which may want the identical
    // helper for its own analogous no-text margin fix — a shared "theme_resource_double" helper is
    // described in this port's report rather than added here, to avoid two agents colliding on one file.
    double theme_double(std::wstring_view key, double fallback)
    {
        const auto resources = winui::Application::Current().Resources();
        const auto boxed_key = winrt::box_value(winrt::hstring{key});
        if (resources.HasKey(boxed_key))
        {
            return winrt::unbox_value_or<double>(resources.Lookup(boxed_key), fallback);
        }
        return fallback;
    }

    // WinUI 3's Fluent CheckBox style tokens (generic.xaml) — the fallback for theme_double() above when
    // the live lookup misses. CheckBoxSize=20 is corroborated by the ground-truth capture
    // (docs/comparison/captures/windows/maui/check_box_light.png: the "Default" glyph measures ~20x20px,
    // x≈30..51 / y≈84..103); CheckBoxHeight=32 is the paired token AdjustCheckBoxForNoText divides against
    // (not independently re-measurable from a screenshot — it is the INVISIBLE reserved touch height, not
    // a drawn extent — but ships as 32 in every current WinUI 3 Fluent theme resource dictionary).
    constexpr double k_default_check_box_height = 32.0;
    constexpr double k_default_check_box_size = 20.0;

    // CheckBoxHandler.Windows.cs's AdjustCheckBoxForNoText + its OnCheckBoxLoaded local function: shrink
    // the just-loaded CheckBox's template root Grid margin from its (content+label-oriented) default down
    // to exactly frame the CheckBoxSize glyph, since this port never gives the control a text label.
    // C#'s OnCheckBoxLoaded reads `sender` instead of capturing `checkBox` (a static local function); this
    // captures the control by value instead, matching this backend's established Loaded-deferred-retry
    // idiom (picker_handler.cpp's map_is_open / time_picker_handler.cpp's map_is_open).
    void adjust_check_box_for_no_text(const check_box_control& checkbox)
    {
        checkbox.MinWidth(0);
        checkbox.MinHeight(0);
        checkbox.Padding(winui::Thickness{0, 0, 0, 0});

        auto token = std::make_shared<winrt::event_token>();
        *token = checkbox.Loaded(
            [checkbox, token](const winrt::Windows::Foundation::IInspectable&, const winui::RoutedEventArgs&) {
                checkbox.Loaded(*token);
                if (winui::Media::VisualTreeHelper::GetChildrenCount(checkbox) <= 0)
                {
                    return;
                }
                const auto root = winui::Media::VisualTreeHelper::GetChild(checkbox, 0);
                const auto root_grid = root.try_as<winui::Controls::Grid>();
                if (!root_grid)
                {
                    return;
                }
                const double height = theme_double(L"CheckBoxHeight", k_default_check_box_height);
                const double size = theme_double(L"CheckBoxSize", k_default_check_box_size);
                const double margin = (height - size) / 2.0;
                root_grid.Margin(winui::Thickness{margin, margin, margin, margin});
            });
    }
} // namespace

namespace maui::core
{
    namespace
    {
        // Unhook everything on_connect_handler registered — same rationale as button_handler.cpp's
        // detach_native_events / date_picker_handler.cpp's identical helper: the lambdas capture the
        // handler, so an undisconnected teardown must not leave them subscribed.
        void detach_native_events(check_box_platform& platform)
        {
            if (platform.native != nullptr)
            {
                const check_box_control checkbox = as_check_box(platform.native);
                checkbox.Checked(winrt::event_token{platform.checked_token});
                checkbox.Unchecked(winrt::event_token{platform.unchecked_token});
            }
            platform.checked_token = 0;
            platform.unchecked_token = 0;
        }
    } // namespace

    check_box_platform::~check_box_platform()
    {
        detach_native_events(*this);
        if (native != nullptr)
        {
            maui::platform::windows::drop<winui::UIElement>(native);
        }
    }

    std::unique_ptr<check_box_platform> check_box_handler::create_platform_view()
    {
        auto platform = std::make_unique<check_box_platform>();
        check_box_control checkbox;
        adjust_check_box_for_no_text(checkbox);
        platform->native = maui::platform::windows::take<winui::UIElement>(checkbox);
        return platform;
    }

    void check_box_handler::on_connect_handler(check_box_platform& platform)
    {
        // Cross-platform hook — the SAME guard shape as the headless/android partials' on_checked_changed:
        // compare the just-read native state against the CURRENT virtual-view value, so a Checked/Unchecked
        // fired by OUR OWN map_is_checked push (which only runs because the virtual view's IsChecked
        // ALREADY changed to this exact value, before native.IsChecked is ever assigned) is silently
        // absorbed instead of bouncing back into another round trip through the bindable property.
        platform.on_checked_changed = [this] {
            auto* view = virtual_view();
            auto* platform_view = typed_platform_view();
            if (view != nullptr && platform_view != nullptr && view->is_checked() != platform_view->is_checked)
            {
                view->send_is_checked(platform_view->is_checked);
            }
        };
        if (platform.native == nullptr)
        {
            return;
        }
        auto* self = &platform;
        const check_box_control checkbox = as_check_box(platform.native);

        // CheckBoxHandler.Windows's ConnectHandler: Checked AND Unchecked both route to the SAME OnChecked
        // body. There is no Indeterminate subscription — WinUI's ToggleButton.IsThreeState defaults false
        // and this handler never sets it, so a user/programmatic toggle only ever fires Checked/Unchecked,
        // never leaves IsChecked null; the defensive null-check below covers only the theoretical case.
        auto on_native_checked = [self](const winrt::Windows::Foundation::IInspectable& sender,
                                        const winui::RoutedEventArgs&) {
            const auto native = sender.try_as<check_box_control>();
            if (!native)
            {
                return;
            }
            // CheckBox.IsChecked is IReference<bool> (ToggleButton's three-state support) — unwrap via
            // .Value() on the read side (the box_value private-ctor trap date_picker_handler.cpp's
            // update_date documents is a WRITE-side concern only).
            const auto boxed = native.IsChecked();
            self->is_checked = boxed ? boxed.Value() : false;
            if (self->on_checked_changed)
            {
                self->on_checked_changed();
            }
        };
        platform.checked_token = checkbox.Checked(on_native_checked).value;
        platform.unchecked_token = checkbox.Unchecked(on_native_checked).value;
    }

    void check_box_handler::on_disconnect_handler(check_box_platform& platform)
    {
        detach_native_events(platform);
        platform.on_checked_changed = nullptr;
    }

    void check_box_handler::map_is_checked(check_box_handler& handler, i_check_box& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // Native-state mirror first (also doubles as the on_checked_changed re-entrancy guard above —
        // setting it to the value about to be pushed means the synchronous Checked/Unchecked the native
        // assignment below fires reads back EQUAL to the virtual view's current value and is absorbed).
        platform->is_checked = view.is_checked();
        if (platform->native == nullptr)
        {
            return;
        }
        // CheckBoxExtensions.UpdateIsChecked: platformCheckBox.IsChecked = check.IsChecked. Pass the bool
        // DIRECTLY, do not box_value it — IReference<bool> has a converting ctor from bool, but box_value
        // returns an IInspectable whose conversion resolves to IReference's PRIVATE ctor (C2248), the exact
        // trap date_picker_handler.cpp's update_date documents for CalendarDatePicker.Date.
        as_check_box(platform->native).IsChecked(platform->is_checked);
    }

    void check_box_handler::map_foreground(check_box_handler& handler, i_check_box& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->foreground = view.foreground();
        if (platform->native == nullptr)
        {
            return;
        }
        const check_box_control checkbox = as_check_box(platform->native);
        // CheckBoxExtensions.UpdateForeground: `check.Foreground?.ToPlatform()` — i_check_box::foreground()
        // is ALREADY the nullable-paint borrow (null while Color was never set — see i_check_box.hpp), so
        // this keys off the pointer directly rather than a bindable is-set check (there is no separate
        // "foreground" bindable property to ask — see check_box.hpp's header note).
        if (platform->foreground == nullptr)
        {
            remove_resources(checkbox, k_tint_color_keys);
            checkbox.ClearValue(winui::Controls::Control::ForegroundProperty());
            refresh_theme_resources(checkbox);
            return;
        }
        const winui::Media::Brush brush = maui::platform::windows::brush_for(*platform->foreground);
        set_resources(checkbox, k_tint_color_keys, brush);
        checkbox.Foreground(brush);
        refresh_theme_resources(checkbox);
    }

    maui::graphics::size check_box_handler::get_desired_size(double width_constraint, double height_constraint) const
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
        const check_box_control checkbox = as_check_box(platform->native);
        // AdjustForExplicitSize (ViewHandlerExtensions.Windows.cs:56-105) — pin Width/Height to the view's
        // own explicit request instead of clearing to NaN unconditionally, then only WIDEN the incoming
        // constraint at measure time. Identical to button_handler.cpp/picker_handler.cpp/
        // date_picker_handler.cpp's get_desired_size — see button_handler.cpp's comment for the full
        // writeup. platform_arrange's OWN stamp (below) is UNTOUCHED.
        const auto* view = virtual_view();
        const double explicit_width = (view != nullptr) ? view->width() : std::numeric_limits<double>::quiet_NaN();
        const double explicit_height = (view != nullptr) ? view->height() : std::numeric_limits<double>::quiet_NaN();
        checkbox.Width(explicit_width);
        checkbox.Height(explicit_height);
        const double adjusted_width_constraint =
            std::isnan(explicit_width) ? width_constraint : std::max(width_constraint, explicit_width);
        const double adjusted_height_constraint =
            std::isnan(explicit_height) ? height_constraint : std::max(height_constraint, explicit_height);
        checkbox.Measure(
            winrt::Windows::Foundation::Size{maui::platform::windows::measure_constraint(adjusted_width_constraint),
                                             maui::platform::windows::measure_constraint(adjusted_height_constraint)});
        const auto desired = checkbox.DesiredSize();
        return {desired.Width, desired.Height};
    }

    void check_box_handler::platform_arrange(const maui::graphics::rect& frame)
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
        const check_box_control checkbox = as_check_box(platform->native);
        winui::Controls::Canvas::SetLeft(checkbox, frame.x);
        winui::Controls::Canvas::SetTop(checkbox, frame.y);
        const auto* const view = virtual_view();
        // SHRINK-WRAP THE WIDTH, do not pin the whole arrange slot to `frame`. MAUI's
        // PlatformArrangeHandler (ViewHandlerExtensions.Windows.cs:76-89) only calls
        // platformView.Arrange(rect) and NEVER assigns Width, so WinUI honours the CheckBox's own default
        // HorizontalAlignment — and unlike Button/ComboBox/TextBox (whose default styles ARE Stretch, so
        // pinning the frame there is correct — see button_handler.cpp/picker_handler.cpp/entry_handler.cpp),
        // a bare WinUI CheckBox's default style does NOT stretch: the ground-truth capture
        // (docs/comparison/captures/windows/maui/check_box_light.png) shows each checkbox as a small
        // ~20x20 glyph pinned to the LEFT of its row, not a full-width stretched control — the same
        // shrink-wrap shape date_picker_handler.cpp documents for CalendarDatePicker (its comment has the
        // full "876 extra DIP of smeared BackgroundColor" writeup this control would repeat if pinned).
        // PER-HANDLER ON PURPOSE — must NOT be hoisted into a shared arrange helper (see that file's note).
        const double natural = view != nullptr ? view->desired_size().width : frame.width;
        checkbox.Width(natural > 0 ? std::min(frame.width, natural) : frame.width);
        checkbox.Height(frame.height);
        // Clip is bounds-dependent (view_chrome_ops.cpp's apply_native_clip reads the just-set Width/
        // Height back); map_clip's own push (view_mapper.cpp) always runs before the first arrange, so
        // this re-invoke is what actually installs the clip once the checkbox has a real size.
        if (view != nullptr)
        {
            apply_native_clip(platform->native, view->clip());
        }
    }

    // ---- generic-IView property pushes (view_platform_base overrides) ---------------------------
    // Delegated to the shared winui_visual_ops free functions so every control behaves identically; see
    // that header for why they are free functions taking the void* slot.
    void check_box_platform::update_visibility(maui::core::visibility value)
    {
        maui::platform::windows::apply_visibility(native, value);
    }

    void check_box_platform::update_opacity(double value)
    {
        maui::platform::windows::apply_opacity(native, value);
    }

    void check_box_platform::update_is_enabled(bool value)
    {
        maui::platform::windows::apply_is_enabled(native, value);
    }

    void check_box_platform::update_automation_id(std::string_view value)
    {
        maui::platform::windows::apply_automation_id(native, value);
    }

    void check_box_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::windows::apply_background(native, value);
    }
} // namespace maui::core
