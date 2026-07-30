// activity_indicator_handler — WinUI 3 platform recipe: a real Microsoft.UI.Xaml.Controls.ProgressRing,
// the same native type ActivityIndicatorHandler.Windows.cs creates (`ViewHandler<IActivityIndicator,
// ProgressRing>` — NOT a ProgressBar/RangeBase; ProgressRing is a plain Control with its own
// IsActive/IsIndeterminate surface, no Value/Minimum/Maximum). Ported from
// ActivityIndicatorHandler.Windows.cs (cross-platform mapper in ActivityIndicatorHandler.cs) +
// Platform/Windows/ActivityIndicatorExtensions.cs.
//
// DOCUMENTED SIMPLIFICATIONS / DEVIATIONS against C#, each narrowed on purpose rather than left vague:
//
//  1. Visibility is NOT coupled to IsRunning here, unlike iOS/Android. ActivityIndicatorHandler.cs's
//     shared Mapper only remaps `[nameof(IActivityIndicator.Visibility)] = MapIsRunning` under
//     `#if __ANDROID__ || __IOS__ || MACCATALYST` ("Android/iOS do not respect both properties
//     independently") — Windows keeps Visibility on the generic ViewHandler mapping, because
//     ProgressRing.IsActive and UIElement.Visibility genuinely are independent there
//     (ActivityIndicatorExtensions.UpdateIsRunning is exactly `IsActive = IsRunning;`, no Visibility
//     reference at all). This port's SHARED cross-platform mapper table
//     (src/core/activity_indicator_handler.cpp) still redirects the "visibility" key to map_is_running
//     unconditionally for every backend (a fine simplification for headless/apple/android, which all DO
//     want that coupling). Rather than add a MAUI_PLATFORM_WINDOWS branch to that shared, non-conditional
//     table, map_is_running below reproduces BOTH independent native pushes itself: IsActive tracks
//     IsRunning alone, and Visibility is pushed through the exact same apply_visibility the generic
//     view_mapper would have used. Net native behavior matches the oracle; only the C++ entry-point
//     shape differs from the C# mapper's own `#if`.
//  2. NeedsContainer/SetupContainer (background wrapped in a WrapperView + a ContainerView re-evaluation
//     nudge) is NOT ported — same documented gap as label_handler.cpp's identical note. Unlike
//     TextBlock, ProgressRing IS a Control with its own Background DependencyProperty, so no wrapper is
//     needed for Background to reach it at all: the generic five-override update_background below (the
//     same apply_background every other Windows handler uses) already paints it directly.
//  3. ARRANGE WIDTH/HEIGHT (ActivityIndicatorExtensions.UpdateWidth/UpdateHeight, both Windows-only
//     Mapper keys in the C# oracle): "Only set a value for this if it's been explicitly set... Otherwise,
//     don't set it to anything (even NaN) because it will try to fill all the space you give it." A
//     ProgressRing is fixed-size/shrink-wrap by default, NOT stretch — the opposite default from
//     picker_handler.cpp's ComboBox / entry_handler.cpp's TextBox, which this backend's Canvas panel
//     compensates for by unconditionally pinning Width/Height to the arrange frame (see those files'
//     platform_arrange). get_desired_size/platform_arrange below therefore only ever stamp Width/Height
//     when the developer set an explicit WidthRequest/HeightRequest (a finite view->width()/height(), this
//     port's NaN-is-unspecified convention) and otherwise leave the DP untouched, matching the oracle's
//     "don't set it to anything" rule exactly — even more conservative than date_picker_handler.cpp's
//     CalendarDatePicker shrink-wrap (which shrink-wraps WIDTH only and still pins Height to the frame).

#include "maui/core/activity_indicator_handler.hpp"

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
#include "maui/core/i_activity_indicator.hpp"
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
    using progress_ring = winui::Controls::ProgressRing;

    progress_ring as_ring(void* native)
    {
        return maui::platform::windows::ref<winui::UIElement>(native).as<progress_ring>();
    }

    // ActivityIndicatorExtensions.cs's ProgressRingForegroundResourceKeys — a single per-state-agnostic
    // brush key (ComboBox/CalendarDatePicker need several per-visual-state keys; ProgressRing has just
    // this one), which the control template's ring-stroke brush binds to. A plain Foreground override
    // alone would be dropped the same way picker_handler.cpp's k_text_color_keys note explains.
    constexpr std::array<std::wstring_view, 1> k_foreground_keys{L"ProgressRingForegroundThemeBrush"};

    void set_resources(const progress_ring& ring, std::span<const std::wstring_view> keys,
                       const winui::Media::Brush& brush)
    {
        for (const auto& key : keys)
        {
            ring.Resources().Insert(winrt::box_value(winrt::hstring{key}), brush);
        }
    }

    void remove_resources(const progress_ring& ring, std::span<const std::wstring_view> keys)
    {
        for (const auto& key : keys)
        {
            ring.Resources().Remove(winrt::box_value(winrt::hstring{key}));
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

    // "Was this property explicitly set?" — see picker_handler.cpp/date_picker_handler.cpp/
    // label_handler.cpp for why this must not be a value comparison
    // ([[cpp-unset-color-sentinel-collision]]): the port's Color is a non-nullable value type whose
    // default-constructed value is opaque BLACK, so it cannot stand in for C#'s `color.IsDefault()`.
    bool is_set(const maui::core::i_view& view, std::string_view property)
    {
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        return bindable != nullptr && bindable->is_property_set(property);
    }
} // namespace

namespace maui::core
{
    activity_indicator_platform::~activity_indicator_platform()
    {
        if (native != nullptr)
        {
            maui::platform::windows::drop<winui::UIElement>(native);
        }
    }

    std::unique_ptr<activity_indicator_platform> activity_indicator_handler::create_platform_view()
    {
        auto platform = std::make_unique<activity_indicator_platform>();
        // ActivityIndicatorHandler.Windows.cs's CreatePlatformView: `new ProgressRing { IsIndeterminate =
        // true }`.
        progress_ring ring;
        ring.IsIndeterminate(true);
        platform->native = maui::platform::windows::take<winui::UIElement>(ring);
        return platform;
    }

    void activity_indicator_handler::map_is_running(activity_indicator_handler& handler, i_activity_indicator& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // Headless-mirror bookkeeping, kept in the same shape every other backend uses (the shared
        // "is_running && visible" observable), even though the NATIVE push below deliberately keeps the
        // two signals independent — see the file-top deviation note 1.
        const bool visible = view.visibility() == visibility::visible;
        platform->is_running = view.is_running() && visible;
        platform->hidden = !visible;
        if (platform->native == nullptr)
        {
            return;
        }
        // ActivityIndicatorExtensions.UpdateIsRunning: `IsActive = IsRunning` — no Visibility reference.
        as_ring(platform->native).IsActive(view.is_running());
        // The independent Visibility push this handler's shared mapper would otherwise have skipped (its
        // "visibility" key is redirected here for every backend) — see the file-top deviation note 1.
        maui::platform::windows::apply_visibility(platform->native, view.visibility());
    }

    void activity_indicator_handler::map_color(activity_indicator_handler& handler, i_activity_indicator& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->color = view.color();
        const progress_ring ring = as_ring(platform->native);
        // ActivityIndicatorExtensions.UpdateColor: an unset Color (`color.IsDefault()`, with
        // ActivityIndicatorHandler.Windows.cs always passing `foregroundDefault: null`) clears the local
        // Foreground so the theme brush (ProgressRingForegroundThemeBrush) shows through instead of
        // painting transparent black.
        if (!is_set(view, "color"))
        {
            remove_resources(ring, k_foreground_keys);
            ring.ClearValue(winui::Controls::Control::ForegroundProperty());
            refresh_theme_resources(ring);
            return;
        }
        const winui::Media::SolidColorBrush brush{maui::platform::windows::to_ui_color(platform->color)};
        set_resources(ring, k_foreground_keys, brush);
        ring.Foreground(brush);
        refresh_theme_resources(ring);
    }

    maui::graphics::size activity_indicator_handler::get_desired_size(double width_constraint,
                                                                      double height_constraint) const
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
        const progress_ring ring = as_ring(platform->native);
        // See the file-top deviation note 3: only pin Width/Height when EXPLICIT; otherwise leave the ring
        // to its own natural size rather than the ARRANGE/EXPLICIT-SIZE FIX every other Windows handler
        // uses (which pins Width/Height to the request even when NaN).
        const auto* view = virtual_view();
        const double explicit_width = (view != nullptr) ? view->width() : std::numeric_limits<double>::quiet_NaN();
        const double explicit_height = (view != nullptr) ? view->height() : std::numeric_limits<double>::quiet_NaN();
        if (std::isfinite(explicit_width))
        {
            ring.Width(explicit_width);
        }
        if (std::isfinite(explicit_height))
        {
            ring.Height(explicit_height);
        }
        const double adjusted_width_constraint =
            std::isnan(explicit_width) ? width_constraint : std::max(width_constraint, explicit_width);
        const double adjusted_height_constraint =
            std::isnan(explicit_height) ? height_constraint : std::max(height_constraint, explicit_height);
        ring.Measure(
            winrt::Windows::Foundation::Size{maui::platform::windows::measure_constraint(adjusted_width_constraint),
                                             maui::platform::windows::measure_constraint(adjusted_height_constraint)});
        const auto desired = ring.DesiredSize();
        return {desired.Width, desired.Height};
    }

    void activity_indicator_handler::platform_arrange(const maui::graphics::rect& frame)
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
        const progress_ring ring = as_ring(platform->native);
        winui::Controls::Canvas::SetLeft(ring, frame.x);
        winui::Controls::Canvas::SetTop(ring, frame.y);
        // SHRINK-WRAP, do not pin the full arrange slot — see the file-top deviation note 3 and
        // get_desired_size's identical guard. A ProgressRing is fixed-size, not stretch-by-default (the
        // opposite of picker_handler.cpp's ComboBox / entry_handler.cpp's TextBox, for which pinning the
        // full frame IS correct). Only stamp Width/Height when the view has an explicit
        // WidthRequest/HeightRequest; otherwise leave the DP untouched so the ring keeps its own natural
        // DesiredSize.
        const auto* view = virtual_view();
        const double explicit_width = (view != nullptr) ? view->width() : std::numeric_limits<double>::quiet_NaN();
        const double explicit_height = (view != nullptr) ? view->height() : std::numeric_limits<double>::quiet_NaN();
        if (std::isfinite(explicit_width))
        {
            ring.Width(explicit_width);
        }
        if (std::isfinite(explicit_height))
        {
            ring.Height(explicit_height);
        }
        // Clip is bounds-dependent (view_chrome_ops.cpp's apply_native_clip reads the just-set Width/
        // Height DPs back). In the common case above (no explicit size) neither DP was touched, so both
        // read back NaN and apply_native_clip's own guard (view_chrome_ops.cpp's "PORT-SPECIFIC guard")
        // no-ops the clip rather than handing NaN to a Composition geometry factory — the correct, and
        // today the only reachable, outcome, since no gallery page sets Clip on an ActivityIndicator.
        if (view != nullptr)
        {
            apply_native_clip(platform->native, view->clip());
        }
    }

    // ---- generic-IView property pushes (view_platform_base overrides) ---------------------------
    // Delegated to the shared winui_visual_ops free functions so every control behaves identically; see
    // that header for why they are free functions taking the void* slot. ProgressRing IS a Control (has
    // its own Background/IsEnabled), unlike label_handler's bare TextBlock, so no Border-host wrapper is
    // needed for any of these five to reach it.
    void activity_indicator_platform::update_visibility(maui::core::visibility value)
    {
        maui::platform::windows::apply_visibility(native, value);
    }

    void activity_indicator_platform::update_opacity(double value)
    {
        maui::platform::windows::apply_opacity(native, value);
    }

    void activity_indicator_platform::update_is_enabled(bool value)
    {
        maui::platform::windows::apply_is_enabled(native, value);
    }

    void activity_indicator_platform::update_automation_id(std::string_view value)
    {
        maui::platform::windows::apply_automation_id(native, value);
    }

    void activity_indicator_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::windows::apply_background(native, value);
    }
} // namespace maui::core
