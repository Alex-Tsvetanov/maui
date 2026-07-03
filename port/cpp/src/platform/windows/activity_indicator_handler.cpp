// activity_indicator_handler — Windows (WinUI 3) platform partial: a REAL
// Microsoft.UI.Xaml.Controls.ProgressRing. The windows twin of
// src/platform/apple/activity_indicator_handler.mm (spinning NSProgressIndicator) / the android JNI
// partial, and the real-native sibling of the headless mirror partial
// (src/platform/headless/activity_indicator_handler.cpp). Display-only: IsRunning maps onto IsActive,
// Color onto Foreground — no inbound events.
//
// Ported DIRECTLY from ActivityIndicatorHandler.Windows.cs (CreatePlatformView's
// { IsIndeterminate = true } + MapIsRunning/MapColor/MapBackground/MapWidth/MapHeight) +
// Platform/Windows/ActivityIndicatorExtensions.cs (UpdateIsRunning/UpdateColor/UpdateWidth/
// UpdateHeight) + ViewExtensions.cs (the generic-IView pushes).
//
// DOCUMENTED DEVIATIONS (each an infrastructure gap of this first cut, not a behavior guess):
//   - map_is_running carries BOTH halves: the port's SHARED mapper routes the "visibility" key here on
//     every backend (the C# iOS/Android override — see the header/cross-platform table), while C#'s
//     WINDOWS mapper keeps the generic Visibility mapping and MapIsRunning only sets IsActive. So this
//     function performs the generic Windows UpdateVisibility push (apply_visibility) AND
//     UpdateIsRunning's IsActive = IsRunning, keeping the headless mirrors' coupled semantics
//     (is_running mirrors IsRunning && Visible) for the XAML-less suite.
//   - UpdateColor's resource-key half (the ProgressRingForegroundThemeBrush key +
//     RefreshThemeResources) is deferred with the port's resource-dictionary seam; the DIRECT
//     Foreground property carries the color (C#'s own body also sets it directly), with the null
//     branch's ClearValue. The `color.IsDefault()` (null) gate rides BindableObject.IsSet("color") —
//     the port's non-nullable color stand-in, exactly like the android twin.
//   - MapBackground (Windows-specific): C# flips NeedsContainer (a wrapper grid takes the brush) and
//     paints handler.ToPlatform(). The windows backend has no container seam yet, so update_background
//     lands the brush on the ring's own Control.Background.
//   - MapWidth/MapHeight (UpdateWidth/UpdateHeight — only push an EXPLICITLY-set dimension so the ring
//     does not fill all offered space): subsumed by the port's Canvas layout model — platform_arrange
//     pins the arranged frame's Width/Height, and measure_native resets them to Auto before measuring,
//     so no separate width/height map is needed.
//
// XAML-less degradation (the windows twin of the android VM-less fallback): the windows preset also
// runs the cross-platform suite on the host where NO XAML runtime exists; create_platform_view catches
// the construction failure and keeps native null, while the headless mirrors are ALWAYS maintained.

#include "maui/core/activity_indicator_handler.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <string_view>

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/base.h>

#include "maui/core/bindable_object.hpp"
#include "maui/core/dimension.hpp"
#include "maui/core/i_activity_indicator.hpp"
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
    namespace wnative = maui::platform::win;

    [[nodiscard]] muxc::ProgressRing ring_of(const maui::core::activity_indicator_platform& platform)
    {
        return wnative::borrow<muxc::ProgressRing>(platform.native);
    }
} // namespace

namespace maui::core
{
    // Releases the one strong ref pinning the ProgressRing (the wnative shape of the pimpl-owned-native
    // doctrine; the apple twin CFReleases its NSProgressIndicator here).
    activity_indicator_platform::~activity_indicator_platform()
    {
        wnative::release(native);
    }

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each
    // calls the base body FIRST — the headless mirrors must stay live for the XAML-less cross-platform
    // suite (header note) — then pushes to the real ProgressRing when one exists. No update_visibility
    // override: the shared mapper routes the "visibility" key to map_is_running (header deviations).

    void activity_indicator_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        // ViewExtensions.UpdateOpacity: the Hidden state pins opacity 0 (map_is_running's
        // apply_visibility restores it — `hidden` is that map's mirror).
        if (!hidden)
        {
            wnative::apply_opacity(native, value);
        }
    }

    void activity_indicator_platform::update_is_enabled(bool value)
    {
        view_platform_base::update_is_enabled(value);
        // ViewExtensions.UpdateIsEnabled → Control.IsEnabled: the WinUI ProgressRing IS a Control (the
        // apple/android twins keep the base mirror because their native spinners have no enabled state).
        if (auto ring = ring_of(*this))
        {
            ring.IsEnabled(value);
        }
    }

    void activity_indicator_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        // ViewExtensions.UpdateAutomationId: AutomationProperties.SetAutomationId.
        wnative::apply_automation_id(native, value);
    }

    void activity_indicator_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        auto ring = ring_of(*this);
        if (ring == nullptr)
        {
            return;
        }
        // ActivityIndicatorHandler.Windows.MapBackground: C# wraps the ring (NeedsContainer) and paints
        // handler.ToPlatform() — the windows backend has no container seam yet, so the brush lands on
        // the ring's own Control.Background (header deviations). A null paint restores the default.
        if (value == nullptr)
        {
            ring.ClearValue(muxc::Control::BackgroundProperty());
            return;
        }
        // Paint.ToPlatform: solid + linear/radial gradient (to_paint_brush); image/pattern still fall back to solid.
        ring.Background(wnative::to_paint_brush(value));
        return;
        // deferred: gradient / image-source paints (Paint.ToPlatform) — the base mirror above keeps
        // the borrow observable.
    }

    std::unique_ptr<activity_indicator_platform> activity_indicator_handler::create_platform_view()
    {
        auto platform = std::make_unique<activity_indicator_platform>();
        try
        {
            // ActivityIndicatorHandler.Windows.CreatePlatformView: new ProgressRing
            // { IsIndeterminate = true }.
            const muxc::ProgressRing ring;
            ring.IsIndeterminate(true);
            platform->native = wnative::store(ring); // released in ~activity_indicator_platform
        }
        catch (const winrt::hresult_error&)
        {
            platform->native = nullptr; // XAML-less degradation (header note)
        }
        return platform;
    }

    void activity_indicator_handler::map_is_running(activity_indicator_handler& handler, i_activity_indicator& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // Headless mirrors FIRST — the XAML-less cross-platform suite observes these (kept live exactly
        // as the headless partial writes them: is_running tracks IsRunning && Visible, hidden tracks
        // !Visible). The native pushes below are additive.
        const bool visible = view.visibility() == visibility::visible;
        platform->is_running = view.is_running() && visible;
        platform->hidden = !visible;
        if (platform->native == nullptr)
        {
            return;
        }
        // The generic Windows UpdateVisibility push (the shared table routes the "visibility" key here;
        // C# Windows keeps the generic mapping — header deviations): Hidden rides Opacity 0, Collapsed
        // collapses, Visible restores the mirrored opacity.
        wnative::apply_visibility(platform->native, view.visibility(), platform->alpha);
        // ActivityIndicatorExtensions.UpdateIsRunning: IsActive = virtualView.IsRunning (no visibility
        // coupling on Windows — a collapsed/hidden ring simply does not render).
        if (auto ring = ring_of(*platform))
        {
            ring.IsActive(view.is_running());
        }
    }

    void activity_indicator_handler::map_color(activity_indicator_handler& handler, i_activity_indicator& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->color = view.color(); // headless mirror first (XAML-less suite)
        auto ring = ring_of(*platform);
        if (ring == nullptr)
        {
            return;
        }
        // ActivityIndicatorExtensions.UpdateColor: color.IsDefault() (null) → RemoveKeys
        // (ProgressRingForegroundThemeBrush) + ClearValue(ForegroundProperty); a value → SetValueForAllKey
        // + Foreground = brush; then RefreshThemeResources. The port pushes the DIRECT Foreground
        // (deferred: the theme-brush resource key — header), with the null gate riding
        // BindableObject.IsSet("color") (the non-nullable-color stand-in, like the android twin).
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        const bool color_is_set = bindable != nullptr && bindable->is_property_set("color");
        if (color_is_set)
        {
            ring.Foreground(wnative::to_brush(view.color()));
        }
        else
        {
            ring.ClearValue(muxc::Control::ForegroundProperty());
        }
    }

    maui::graphics::size activity_indicator_handler::get_desired_size(double width_constraint,
                                                                      double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return {0, 0};
        }
        if (platform->native == nullptr)
        {
            // XAML-less degradation: the headless partial's medium-spinner placeholder (20x20), so the
            // backend-agnostic size-request suites see consistent numbers (the android twin's shape).
            return {20.0, 20.0};
        }
        // ViewHandlerExtensions.Windows.cs GetDesiredSizeFromHandler: FrameworkElement.Measure +
        // DesiredSize, with the AdjustForExplicitSize clamp fed from the virtual view's explicit
        // width()/height() (see measure_native — the reset-to-Auto also covers UpdateWidth/UpdateHeight's
        // explicit-only concern, header deviations).
        const double explicit_width = virtual_view() != nullptr ? virtual_view()->width() : dimension::unset;
        const double explicit_height = virtual_view() != nullptr ? virtual_view()->height() : dimension::unset;
        // A WinUI ProgressRing "fills all the space you give it" (ActivityIndicatorExtensions.UpdateWidth
        // comment) and FAULTS its Measure under an INFINITE constraint — WinUI cannot fill infinity. A
        // horizontal stack offers its children an unbounded main axis, which is exactly how the
        // controls_stack row reaches here. Substitute the default ring box for a non-finite constraint so
        // it measures to a modest square instead of throwing. (The wide-ellipse fix lives in
        // platform_arrange, which pins a CENTERED SQUARE rather than the fill-inviting frame.)
        constexpr double k_default_ring_box = 20.0;
        const double wc = std::isfinite(width_constraint) ? width_constraint : k_default_ring_box;
        const double hc = std::isfinite(height_constraint) ? height_constraint : k_default_ring_box;
        return wnative::measure_native(platform->native, wc, hc, explicit_width, explicit_height);
    }

    void activity_indicator_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // XAML-less: no native layout to apply
        }
        // A ProgressRing STRETCHES to whatever explicit Width/Height it is pinned to, so pinning the full
        // arranged frame (which the layout inflates to the row's full width for a Fill-aligned indicator)
        // produced a wide horizontal ellipse. MAUI keeps the spinner circular by never setting
        // Width/Height and letting the ring self-centre. On the port's Canvas model we replicate that:
        // pin a SQUARE the size of the frame's shorter side, CENTERED within the frame — a circular ring
        // matching MAUI's small centered spinner (and the explicit Larger/Smaller squares stay square).
        const double box = std::min(frame.width, frame.height);
        const double x = frame.x + (frame.width - box) / 2.0;
        const double y = frame.y + (frame.height - box) / 2.0;
        wnative::arrange_native(platform->native, maui::graphics::rect{x, y, box, box});
    }
} // namespace maui::core
