// switch_handler — WinUI 3 platform recipe: a real Microsoft.UI.Xaml.Controls.ToggleSwitch, the same
// native type SwitchHandler.Windows.cs creates (`ViewHandler<ISwitch, ToggleSwitch>` — a bare
// ToggleSwitch, not a Maui* subclass, unlike MauiButton/MauiTextBox). Ported from SwitchHandler.Windows.cs
// + SwitchHandler.cs (the shared mapper table) + Platform/Windows/SwitchExtensions.cs.
//
// BOX MEASURED, NOT ASSUMED: port/maui-reference/pages/switch.xaml's `<Switch BackgroundColor="Blue" />`
// instance (the background-reveal trick — the reliable way to read a control's real BOX rather than its
// glyph ink) spans y 163-202 (40 rows, crisp/pixel-aligned edges both sides) AND x 8-47 (40 columns, softer
// sub-pixel edges consistent with the rounded-corner curve) in
// docs/comparison/captures/windows/maui/switch_light.png — i.e. the target is a SQUARE 40x40 box, not the
// wider-than-tall shape a stock ToggleSwitch with default OnContent/OffContent would produce. That squareness
// is exactly what suppressing the "On"/"Off" labels (simplification 2 below) is expected to yield: with no
// label reserving horizontal space, the control shrink-wraps down toward its knob/track, which is roughly as
// wide as it is tall once the reserved label area collapses.
//
// DOCUMENTED SIMPLIFICATIONS / DEVIATIONS against C#, each narrowed on purpose rather than left vague:
//
//  1. OnLoaded's SwitchAreaGrid column-1 fix (SwitchHandler.Windows.cs:45-63) IS ported, via a small LOCAL
//     (file-scoped, not shared) recursive VisualTreeHelper name+type search — see find_switch_area_grid
//     below. This is narrower than the "no descendant-by-name infrastructure" gap check_box_handler.cpp /
//     date_picker_handler.cpp / entry_handler.cpp / search_bar_handler.cpp defer for OTHER controls: this
//     is CONFIRMED, not speculative, to matter for width. The Fluent ToggleSwitch template (verified against
//     Kinnara/ModernWpf's faithful WinUI-template port, since the real Microsoft.UI.Xaml generic.xaml is not
//     in this tree) nests the switch visuals in a Grid with `ColumnDefinitions="Auto,12,Auto"` — the SECOND
//     column (index 1) is a FIXED 12px spacer, not an Auto column, so it does NOT collapse on its own just
//     because OnContent/OffContent are null (simplification 2). Skipping this fix would measure the control
//     ~12 DIP wider than the 40x40 ground truth this file's header measures — on the very
//     BackgroundColor="Blue" instance used as that ground truth. The search matches the oracle's OWN typed
//     `GetDescendantByName<Grid>("SwitchAreaGrid")` (name AND type, not name alone) rather than a looser
//     type-agnostic search: this file's own template excerpt shows `OuterBorder` (the visible track) at a
//     bare 40 wide, matching the measured 40-wide ground truth with NO extra 12px — i.e. the spacer IS
//     collapsed in MAUI's real render, so MAUI's typed search must be finding a real Grid there, and this
//     port's search needs to require the same type to fire in exactly the same case (never a case MAUI's
//     own code would have skipped).
//     UNVERIFIED, TWO WAYS (cannot compile/run here): (a) the exact visual-tree shape is inferred from a
//     third-party template reproduction, not the shipped Microsoft.UI.Xaml source; (b) ORDERING — this
//     backend's boot sequence (host_run.cpp) forces a synchronous `root.UpdateLayout()` (which should flush
//     the pending Loaded dispatch for every already-parented control, firing this fix) BEFORE its OWN
//     get_desired_size()/Measure() pass, so the FIRST/boot layout of a page should see the column already
//     zeroed by the time it measures. Whether a LATER page navigation (reusing an already-activated window,
//     with no equivalent forced UpdateLayout() in between) preserves that ordering was not traceable from
//     here — if Measure() ever runs before Loaded fires, the column is still 12 wide for that measure, and
//     ts.InvalidateMeasure() below queues a native re-measure but does not, by itself, force the port's own
//     cached desired_size() to be re-read.
//  2. OnContent/OffContent = null (CreatePlatformView) IS ported — this is what lets the control shrink-wrap
//     toward the knob instead of reserving room for "On"/"Off" text.
//  3. MinimumWidth (the Windows-only `[nameof(IView.MinimumWidth)] = MapSwitchMinimumWidth` Mapper entry;
//     UpdateMinWidth: NaN -> MinWidth=0, else MinWidth=view.MinimumWidth). This port's property_mapper has
//     NO "minimum_width" key wired for ANY control — view_mapper.cpp defines no map_width / map_height /
//     map_minimum_width at all; every existing Windows handler instead reads explicit width/height directly
//     inside get_desired_size (e.g. button_handler.cpp's explicit_width/explicit_height). Adding a genuinely
//     new mapper key here would be inventing shared infrastructure no sibling handler has, for one control.
//     Folded into get_desired_size instead, in the same place/style the explicit-size pin already lives:
//     MinWidth is read straight off `view->minimum_width()` every measure (this port's unset sentinel is
//     NaN, matching C#'s), 0 when unset — the only case any current gallery page exercises.
//  4. ThumbColor's push mechanism DEVIATES from the literal C# TryUpdateResource (FrameworkElementExtensions
//     .cs): the oracle only OVERWRITES a resource key already present in the control's own LOCAL Resources
//     dictionary (`if (rd?.ContainsKey(key) ?? false) rd[key] = newValue;` — no insert, no else branch). A
//     freshly-constructed ToggleSwitch's local Resources starts EMPTY (nothing else in this file or in
//     SwitchExtensions.cs seeds the 8 "ToggleSwitchKnobFill*" keys under any other name), so a literal port
//     would make ThumbColor a silent no-op — yet switch_light.png's ThumbColor="Orange" row (switch OFF)
//     shows a visibly ORANGE knob, so real MAUI's TryUpdateResource plainly is NOT a no-op there; some
//     WinUI mechanism this port cannot inspect (no Windows runtime here) evidently pre-populates those keys.
//     Per port/CLAUDE.md's rule 4 (RENDER-BREAKS-TIES) (render wins over source when they disagree), map_thumb_color below
//     uses the same unconditional set_resources/remove_resources this file already uses for TrackColor,
//     which is a strict superset of TryUpdateResource (identical when a key is present; also inserts when
//     absent) — it can only fix the observed gap, never regress the cases where the literal port would have
//     worked anyway.
//
// Color collapse (as toggle_switch.hpp documents): C#'s nullable On/Off/ThumbColor collapse to non-nullable
// maui::graphics::color value types here, so "is it set" goes through the bindable is-property-set check
// (is_set below) rather than a null comparison ([[cpp-unset-color-sentinel-collision]]).

#include "maui/core/switch_handler.hpp"

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <memory>
#include <span>
#include <string_view>

#include "maui/core/bindable_object.hpp"
#include "maui/core/i_switch.hpp"
#include "maui/core/view_chrome_ops.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "winui_interop.hpp"
#include "winui_visual_ops.hpp"

namespace
{
    // Named `winui`, NOT `xaml`: the port already has a maui::xaml namespace (the XAML loader), and inside
    // namespace maui::* that name WINS over a file-scope alias — see picker_handler.cpp's identical note.
    namespace winui = winrt::Microsoft::UI::Xaml;
    using toggle_switch_control = winui::Controls::ToggleSwitch;

    toggle_switch_control as_toggle_switch(void* native)
    {
        return maui::platform::windows::ref<winui::UIElement>(native).as<toggle_switch_control>();
    }

    // SwitchExtensions.cs's toggleSwitchOnKeys / toggleSwitchOffKeys — the TrackColor resource-key pair,
    // keyed by the CURRENTLY active toggle state; only that state's keys are ever touched by a given push
    // (see map_track_color below — this matches UpdateTrackColor exactly, including the asymmetry that the
    // INACTIVE state's keys are left however they were last time that state was active).
    constexpr std::array<std::wstring_view, 4> k_on_keys{
        L"ToggleSwitchFillOn",
        L"ToggleSwitchFillOnPointerOver",
        L"ToggleSwitchFillOnPressed",
        L"ToggleSwitchFillOnDisabled",
    };
    constexpr std::array<std::wstring_view, 4> k_off_keys{
        L"ToggleSwitchFillOff",
        L"ToggleSwitchFillOffPointerOver",
        L"ToggleSwitchFillOffPressed",
        L"ToggleSwitchFillOffDisabled",
    };

    // SwitchExtensions.cs's UpdateThumbColor knob-key list — both on/off knob states, unconditionally (no
    // toggle-state branching, unlike the track keys above).
    constexpr std::array<std::wstring_view, 8> k_knob_keys{
        L"ToggleSwitchKnobFillOnPointerOver",  L"ToggleSwitchKnobFillOn",
        L"ToggleSwitchKnobFillOnPressed",      L"ToggleSwitchKnobFillOnDisabled",
        L"ToggleSwitchKnobFillOffPointerOver", L"ToggleSwitchKnobFillOff",
        L"ToggleSwitchKnobFillOffPressed",     L"ToggleSwitchKnobFillOffDisabled",
    };

    void set_resources(const toggle_switch_control& ts, std::span<const std::wstring_view> keys,
                       const winui::Media::Brush& brush)
    {
        for (const auto& key : keys)
        {
            ts.Resources().Insert(winrt::box_value(winrt::hstring{key}), brush);
        }
    }

    void remove_resources(const toggle_switch_control& ts, std::span<const std::wstring_view> keys)
    {
        for (const auto& key : keys)
        {
            ts.Resources().Remove(winrt::box_value(winrt::hstring{key}));
        }
    }

    // FrameworkElementExtensions.RefreshThemeResources: flip RequestedTheme away and back so the control
    // template re-resolves the resources just overridden — identical to every sibling handler's helper.
    void refresh_theme_resources(const winui::FrameworkElement& element)
    {
        const auto previous = element.RequestedTheme();
        element.RequestedTheme(element.ActualTheme() == winui::ElementTheme::Dark ? winui::ElementTheme::Light
                                                                                  : winui::ElementTheme::Dark);
        element.RequestedTheme(previous);
    }

    // "Was this property explicitly set?" — see button_handler.cpp/check_box_handler.cpp for why this must
    // not be a value comparison ([[cpp-unset-color-sentinel-collision]]).
    bool is_set(const maui::core::i_view& view, std::string_view property)
    {
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        return bindable != nullptr && bindable->is_property_set(property);
    }

    // FrameworkElementExtensions.GetDescendantByName<Grid>, narrowed to exactly this file's one use (see
    // file-top note 1): a recursive VisualTreeHelper walk for the FIRST descendant that is BOTH a Grid AND
    // named "SwitchAreaGrid" — matching the oracle's own typed filter exactly (name alone would fire in a
    // case the C# `GetDescendantByName<Grid>` does not, if the real element ever turned out to be some
    // other type). Kept LOCAL rather than promoted to winui_interop.hpp — this is the only reach in the
    // Windows backend so far that needs a NAME (not index) search; the sibling deferrals (check_box/
    // date_picker/entry/search_bar) each need several DIFFERENT named parts across DIFFERENT controls,
    // which is the shared-infrastructure investment those files are (correctly) still waiting on.
    winui::Controls::Grid find_switch_area_grid(const winui::DependencyObject& parent, const winrt::hstring& name)
    {
        const std::int32_t count = winui::Media::VisualTreeHelper::GetChildrenCount(parent);
        for (std::int32_t i = 0; i < count; ++i)
        {
            const winui::DependencyObject child = winui::Media::VisualTreeHelper::GetChild(parent, i);
            if (const auto grid = child.try_as<winui::Controls::Grid>(); grid && grid.Name() == name)
            {
                return grid;
            }
            if (const auto found = find_switch_area_grid(child, name))
            {
                return found;
            }
        }
        return nullptr;
    }

    // SwitchHandler.Windows.cs's OnLoaded (see file-top note 1): the default ToggleSwitch template reserves
    // a SECOND (index 1) FIXED-width spacer ColumnDefinition between the knob and the (here, unused) On/Off
    // content area. Zeroing it removes that reserved width now that OnContent/OffContent are null.
    void adjust_switch_area_spacer_column(const toggle_switch_control& ts)
    {
        auto token = std::make_shared<winrt::event_token>();
        *token = ts.Loaded([ts, token](const winrt::Windows::Foundation::IInspectable&, const winui::RoutedEventArgs&) {
            ts.Loaded(*token);
            const auto switch_area_grid = find_switch_area_grid(ts, winrt::hstring{L"SwitchAreaGrid"});
            if (!switch_area_grid)
            {
                return;
            }
            const auto root_grid =
                winui::Media::VisualTreeHelper::GetParent(switch_area_grid).try_as<winui::Controls::Grid>();
            if (!root_grid)
            {
                return;
            }
            // Guarded on > 1 (not the oracle's `Count > 0`, which would let a 1-column template through
            // to an out-of-range GetAt(1) — this port has no live template to confirm always has 3).
            if (const auto columns = root_grid.ColumnDefinitions(); columns.Size() > 1)
            {
                columns.GetAt(1).Width(winui::GridLength{0, winui::GridUnitType::Pixel});
                // Queue a native re-measure now that the column changed (button_handler.cpp's
                // sync_content_composition does the same after reshaping its content panel). Does not, by
                // itself, force the port's OWN cached desired_size() to be re-read — see file-top note 1's
                // ordering caveat.
                ts.InvalidateMeasure();
            }
        });
    }
} // namespace

namespace maui::core
{
    namespace
    {
        // Unhook everything on_connect_handler registered — same rationale as every sibling handler's
        // detach_native_events: the lambda captures the handler, so an undisconnected teardown must not
        // leave it subscribed.
        void detach_native_events(switch_platform& platform)
        {
            if (platform.native != nullptr)
            {
                as_toggle_switch(platform.native).Toggled(winrt::event_token{platform.toggled_token});
            }
            platform.toggled_token = 0;
        }
    } // namespace

    switch_platform::~switch_platform()
    {
        detach_native_events(*this);
        if (native != nullptr)
        {
            maui::platform::windows::drop<winui::UIElement>(native);
        }
    }

    std::unique_ptr<switch_platform> switch_handler::create_platform_view()
    {
        auto platform = std::make_unique<switch_platform>();
        // SwitchHandler.Windows.cs: `new ToggleSwitch() { OffContent = null, OnContent = null }` — no
        // "On"/"Off" labels (file-top simplification 2; this is what lets the box shrink toward the knob
        // instead of reserving a label's worth of horizontal space).
        toggle_switch_control ts;
        ts.OffContent(nullptr);
        ts.OnContent(nullptr);
        // OnLoaded's SwitchAreaGrid column-1 fix (file-top note 1) — deferred until the template is live.
        adjust_switch_area_spacer_column(ts);
        platform->native = maui::platform::windows::take<winui::UIElement>(ts);
        return platform;
    }

    void switch_handler::on_connect_handler(switch_platform& platform)
    {
        // Cross-platform half — same guard shape as headless/check_box_handler.cpp's on_checked_changed: a
        // native Toggled fired by OUR OWN map_is_on push (which only runs because the virtual view's IsOn
        // ALREADY changed to this exact value, before native.IsOn is ever assigned) reads back EQUAL to the
        // virtual view's current value and is silently absorbed instead of bouncing into a second round
        // trip through the bindable property.
        platform.on_value_changed = [this] {
            auto* view = virtual_view();
            auto* platform_view = typed_platform_view();
            if (view != nullptr && platform_view != nullptr && view->is_on() != platform_view->is_on)
            {
                view->set_is_on(platform_view->is_on);
            }
        };
        if (platform.native == nullptr)
        {
            return;
        }
        auto* self = &platform;
        const toggle_switch_control ts = as_toggle_switch(platform.native);
        // SwitchHandler.Windows's OnToggled: `if (VirtualView.IsOn == PlatformView.IsOn) return; VirtualView
        // .IsOn = PlatformView.IsOn;` — the re-entrancy guard lives in on_value_changed above (self->is_on
        // is read from the native FIRST, then compared against the virtual view there). ToggleSwitch.IsOn
        // is a plain bool (no IReference<bool> unwrap needed, unlike CheckBox.IsChecked).
        platform.toggled_token =
            ts.Toggled([self](const winrt::Windows::Foundation::IInspectable&, const winui::RoutedEventArgs&) {
                  self->is_on = as_toggle_switch(self->native).IsOn();
                  if (self->on_value_changed)
                  {
                      self->on_value_changed();
                  }
              }).value;
        // The SwitchAreaGrid column-1 Loaded fix is subscribed from create_platform_view (it needs to catch
        // the FIRST Loaded after construction, which may fire before a handler ever connects), not here.
    }

    void switch_handler::on_disconnect_handler(switch_platform& platform)
    {
        detach_native_events(platform);
        platform.on_value_changed = nullptr;
    }

    void switch_handler::map_is_on(switch_handler& handler, i_switch& view)
    {
        // C# MapIsOn's Apple/iOS twin explicitly calls UpdateIsOn(handler) -> handler.UpdateValue
        // (TrackColor) before pushing the native value (SwitchHandler.iOS.cs) — this port's shared
        // toggle_switch.cpp ALSO re-runs track_color from its own is_toggled_property changed callback, so
        // this is a harmless second pass, kept for the same cross-backend shape headless/switch_handler.hpp
        // documents for every backend's map_is_on.
        handler.update_value("track_color");
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->is_on = view.is_on();
        if (platform->native == nullptr)
        {
            return;
        }
        // SwitchExtensions.UpdateIsToggled: `toggleSwitch.IsOn = view?.IsOn ?? false;` — a plain bool push.
        as_toggle_switch(platform->native).IsOn(platform->is_on);
    }

    void switch_handler::map_track_color(switch_handler& handler, i_switch& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->track_color = view.track_color();
        if (platform->native == nullptr)
        {
            return;
        }
        const toggle_switch_control ts = as_toggle_switch(platform->native);
        // SwitchExtensions.UpdateTrackColor: TrackColor is the EFFECTIVE color here (collapsed — see
        // i_switch.hpp / toggle_switch.hpp), so "is TrackColor set" means the CURRENT state's own property
        // ("on_color" when toggled, else "off_color") was explicitly set, matching Switch.cs's
        // `ISwitch.TrackColor` getter (returns whichever of OnColor/OffColor is active). Only the active
        // state's key set is ever touched — the other is left exactly as UpdateTrackColor leaves it.
        const bool is_on = view.is_on();
        const std::span<const std::wstring_view> keys =
            is_on ? std::span<const std::wstring_view>(k_on_keys) : std::span<const std::wstring_view>(k_off_keys);
        if (is_set(view, is_on ? "on_color" : "off_color"))
        {
            const winui::Media::SolidColorBrush brush{maui::platform::windows::to_ui_color(platform->track_color)};
            set_resources(ts, keys, brush);
        }
        else
        {
            remove_resources(ts, keys);
        }
        refresh_theme_resources(ts);
    }

    void switch_handler::map_thumb_color(switch_handler& handler, i_switch& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->thumb_color = view.thumb_color();
        if (platform->native == nullptr)
        {
            return;
        }
        // UpdateThumbColor: `if (view.ThumbColor is null) return;` — an unset ThumbColor pushes NOTHING at
        // all, no clearing branch either (unlike map_track_color's remove_resources else-branch).
        if (!is_set(view, "thumb_color"))
        {
            return;
        }
        const toggle_switch_control ts = as_toggle_switch(platform->native);
        const winui::Media::SolidColorBrush brush{maui::platform::windows::to_ui_color(platform->thumb_color)};
        // set_resources, NOT a literal TryUpdateResource port — see file-top note 4 (render-wins deviation:
        // the ground-truth capture shows ThumbColor visibly applying, which a HasKey-gated update cannot
        // produce on a freshly-constructed control with an empty local Resources dictionary).
        set_resources(ts, k_knob_keys, brush);
        refresh_theme_resources(ts);
    }

    maui::graphics::size switch_handler::get_desired_size(double width_constraint, double height_constraint) const
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
        const toggle_switch_control ts = as_toggle_switch(platform->native);
        // AdjustForExplicitSize (ViewHandlerExtensions.Windows.cs:56-105) — pin Width/Height to the view's
        // own explicit request instead of clearing to NaN unconditionally, then only WIDEN the incoming
        // constraint at measure time. Identical to button_handler.cpp/check_box_handler.cpp/
        // date_picker_handler.cpp's get_desired_size. platform_arrange's OWN stamp (below) is UNTOUCHED.
        const auto* view = virtual_view();
        const double explicit_width = (view != nullptr) ? view->width() : std::numeric_limits<double>::quiet_NaN();
        const double explicit_height = (view != nullptr) ? view->height() : std::numeric_limits<double>::quiet_NaN();
        ts.Width(explicit_width);
        ts.Height(explicit_height);
        // MapSwitchMinimumWidth's UpdateMinWidth, folded in here (file-top simplification 3): pin MinWidth
        // off view->minimum_width() every measure (this port's unset sentinel is NaN, matching C#'s NaN
        // default) -> 0 when unset, else the explicit request — instead of a live mapper push this port has
        // no "minimum_width" hook for.
        const double minimum_width =
            (view != nullptr) ? view->minimum_width() : std::numeric_limits<double>::quiet_NaN();
        ts.MinWidth(std::isnan(minimum_width) ? 0.0 : minimum_width);
        const double adjusted_width_constraint =
            std::isnan(explicit_width) ? width_constraint : std::max(width_constraint, explicit_width);
        const double adjusted_height_constraint =
            std::isnan(explicit_height) ? height_constraint : std::max(height_constraint, explicit_height);
        ts.Measure(
            winrt::Windows::Foundation::Size{maui::platform::windows::measure_constraint(adjusted_width_constraint),
                                             maui::platform::windows::measure_constraint(adjusted_height_constraint)});
        const auto desired = ts.DesiredSize();
        return {desired.Width, desired.Height};
    }

    void switch_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        // PlatformArrangeHandler's guard, WIDENED to non-finite — see button_handler.cpp/check_box_handler
        // .cpp's platform_arrange for why (a NaN reaching XAML's arrange is an unrecoverable stowed
        // exception).
        if (!std::isfinite(frame.x) || !std::isfinite(frame.y) || !std::isfinite(frame.width) ||
            !std::isfinite(frame.height) || frame.width < 0 || frame.height < 0)
        {
            return;
        }
        const toggle_switch_control ts = as_toggle_switch(platform->native);
        winui::Controls::Canvas::SetLeft(ts, frame.x);
        winui::Controls::Canvas::SetTop(ts, frame.y);
        const auto* const view = virtual_view();
        // SHRINK-WRAP THE WIDTH, do not pin the whole arrange slot to `frame`. MAUI's PlatformArrangeHandler
        // (ViewHandlerExtensions.Windows.cs:76-89) only calls Arrange(rect) and NEVER assigns Width, so
        // WinUI honours the ToggleSwitch's own default HorizontalAlignment — which is NOT Stretch (the
        // 40x40 ground-truth box this file's header measures is a compact control pinned to the row's left,
        // not a full-width bar), the same shape check_box_handler.cpp / date_picker_handler.cpp document for
        // their own controls. PER-HANDLER ON PURPOSE — must NOT be hoisted into a shared arrange helper.
        //
        // NOT ADDRESSED HERE (flagged, not fixed): this Canvas-hosted backend calls SetTop with no
        // centering offset, because the Canvas never re-measures its children. Real MAUI centers via
        // Arrange-time layout from its native LayoutPanel, so once a row's height grows to fit a 40-tall
        // switch, shorter siblings in the same row (e.g. a 32-tall CheckBox/ActivityIndicator) may render
        // top-anchored here where MAUI centers them — a small residual cosmetic offset, not this control's
        // to fix.
        const double natural = view != nullptr ? view->desired_size().width : frame.width;
        ts.Width(natural > 0 ? std::min(frame.width, natural) : frame.width);
        ts.Height(frame.height);
        // Clip is bounds-dependent (view_chrome_ops.cpp's apply_native_clip reads the just-set Width/Height
        // back); map_clip's own push (view_mapper.cpp) always runs before the first arrange, so this
        // re-invoke is what actually installs the clip once the switch has a real size.
        if (view != nullptr)
        {
            apply_native_clip(platform->native, view->clip());
        }
    }

    // ---- generic-IView property pushes (view_platform_base overrides) ---------------------------
    // Delegated to the shared winui_visual_ops free functions so every control behaves identically; see
    // that header for why they are free functions taking the void* slot.
    void switch_platform::update_visibility(maui::core::visibility value)
    {
        maui::platform::windows::apply_visibility(native, value);
    }

    void switch_platform::update_opacity(double value)
    {
        maui::platform::windows::apply_opacity(native, value);
    }

    void switch_platform::update_is_enabled(bool value)
    {
        maui::platform::windows::apply_is_enabled(native, value);
    }

    void switch_platform::update_automation_id(std::string_view value)
    {
        maui::platform::windows::apply_automation_id(native, value);
    }

    void switch_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::windows::apply_background(native, value);
    }
} // namespace maui::core
