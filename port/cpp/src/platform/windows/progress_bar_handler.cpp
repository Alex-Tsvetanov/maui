// progress_bar_handler â€” Windows (WinUI 3) platform partial: a REAL
// Microsoft.UI.Xaml.Controls.ProgressBar. The windows twin of
// src/platform/apple/progress_bar_handler.mm (NSProgressIndicator) / the android JNI partial, and the
// real-native sibling of the headless mirror partial (src/platform/headless/progress_bar_handler.cpp).
// Display-only: Progress maps onto Value over the fixed [0, 1] range, ProgressColor onto Foreground â€”
// no inbound events.
//
// Ported DIRECTLY from ProgressBarHandler.Windows.cs (CreatePlatformView's { Minimum = 0, Maximum = 1 })
// + Platform/Windows/ProgressBarExtensions.cs (UpdateProgress/UpdateProgressColor) + ViewExtensions.cs
// (the generic-IView pushes).
//
// DOCUMENTED DEVIATIONS (each an infrastructure gap of this first cut, not a behavior guess):
//   - UpdateProgressColor's null discrimination: C# is `if (progressColor != null) Foreground = ...`
//     (NO else branch â€” an unset color leaves the theme default). The port's color is a non-nullable
//     value type, so the `!= null` gate rides BindableObject.IsSet("progress_color") â€” the same
//     unset-color stand-in the button/label/android-activity-indicator partials use.
//   - IsEnabled IS pushed (unlike the apple/android twins): the WinUI ProgressBar is a Control, so
//     C#'s generic ViewExtensions.UpdateIsEnabled (`(platformView as Control)?.IsEnabled`) lands on it.
//   - FlowDirection: the shared table's "flow_direction" key carries the C# iOS-specific
//     MapFlowDirection override; on Windows the same resolved direction is pushed as the generic
//     Windows recipe (ViewExtensions.UpdateFlowDirection â€” FrameworkElement.FlowDirection / ClearValue).
//
// XAML-less degradation (the windows twin of the android VM-less fallback): the windows preset also
// runs the cross-platform suite on the host where NO XAML runtime exists; create_platform_view catches
// the construction failure and keeps native null, while the headless mirrors are ALWAYS maintained.

#include "maui/core/progress_bar_handler.hpp"

#include <memory>
#include <string_view>

#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h> // ToggleButton/RangeBase: the projected base carries IsChecked/Minimum/Maximum/Value
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/base.h>

#include "maui/core/bindable_object.hpp"
#include "maui/core/dimension.hpp"
#include "maui/core/flow_direction.hpp"
#include "maui/core/i_progress.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "windows_native.hpp"

namespace
{
    namespace mux = winrt::Microsoft::UI::Xaml;
    namespace muxc = winrt::Microsoft::UI::Xaml::Controls;
    namespace wnative = maui::platform::win;

    [[nodiscard]] muxc::ProgressBar bar_of(const maui::core::progress_bar_platform& platform)
    {
        return wnative::borrow<muxc::ProgressBar>(platform.native);
    }
} // namespace

namespace maui::core
{
    // Releases the one strong ref pinning the ProgressBar (the wnative shape of the pimpl-owned-native
    // doctrine; the apple twin CFReleases its NSProgressIndicator here).
    progress_bar_platform::~progress_bar_platform()
    {
        wnative::release(native);
    }

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each
    // calls the base body FIRST â€” the headless mirrors must stay live for the XAML-less cross-platform
    // suite (header note) â€” then pushes to the real ProgressBar when one exists.

    void progress_bar_platform::update_visibility(maui::core::visibility value)
    {
        view_platform_base::update_visibility(value);
        // ViewExtensions.UpdateVisibility (Windows): Hidden rides Opacity 0, Collapsed collapses.
        wnative::apply_visibility(native, value, alpha);
    }

    void progress_bar_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        // ViewExtensions.UpdateOpacity: the Hidden state pins opacity 0 (apply_visibility restores it).
        if (!hidden)
        {
            wnative::apply_opacity(native, value);
        }
    }

    void progress_bar_platform::update_is_enabled(bool value)
    {
        view_platform_base::update_is_enabled(value);
        // ViewExtensions.UpdateIsEnabled â†’ Control.IsEnabled: the WinUI ProgressBar IS a Control (the
        // apple/android twins keep the base mirror because their native bars have no enabled state).
        if (auto bar = bar_of(*this))
        {
            bar.IsEnabled(value);
        }
    }

    void progress_bar_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        // ViewExtensions.UpdateAutomationId: AutomationProperties.SetAutomationId.
        wnative::apply_automation_id(native, value);
    }

    std::unique_ptr<progress_bar_platform> progress_bar_handler::create_platform_view()
    {
        auto platform = std::make_unique<progress_bar_platform>();
        try
        {
            // ProgressBarHandler.Windows.CreatePlatformView: new() { Minimum = 0, Maximum = 1 } â€” the
            // IProgress [0, 1] fraction maps 1:1 onto Value.
            const muxc::ProgressBar bar;
            bar.Minimum(0.0);
            bar.Maximum(1.0);
            platform->native = wnative::store(bar); // released in ~progress_bar_platform
        }
        catch (const winrt::hresult_error&)
        {
            platform->native = nullptr; // XAML-less degradation (header note)
        }
        return platform;
    }

    void progress_bar_handler::map_progress(progress_bar_handler& handler, i_progress& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->progress = view.progress(); // headless mirror first (XAML-less suite)
        // ProgressBarExtensions.UpdateProgress: platformProgressBar.Value = progress.Progress.
        if (auto bar = bar_of(*platform))
        {
            bar.Value(view.progress());
        }
    }

    void progress_bar_handler::map_progress_color(progress_bar_handler& handler, i_progress& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->progress_color = view.progress_color(); // headless mirror first (XAML-less suite)
        auto bar = bar_of(*platform);
        if (bar == nullptr)
        {
            return;
        }
        // ProgressBarExtensions.UpdateProgressColor: `if (progressColor != null) Foreground =
        // progressColor.ToPlatform()` â€” NO else branch (an unset color keeps the theme default). The
        // `!= null` gate rides BindableObject.IsSet (header deviations).
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        const bool color_is_set = bindable != nullptr && bindable->is_property_set("progress_color");
        if (color_is_set)
        {
            bar.Foreground(wnative::to_brush(view.progress_color()));
        }
    }

    void progress_bar_handler::map_flow_direction(progress_bar_handler& handler, i_progress& view)
    {
        // ProgressBarHandler.MapFlowDirection's base part: record the RESOLVED direction (the
        // MatchParent â†’ parent-IView fallback) and push it as the generic Windows recipe
        // (ViewExtensions.UpdateFlowDirection): LeftToRight/RightToLeft set
        // FrameworkElement.FlowDirection, an unresolved MatchParent ClearValues back to the inherited
        // default. (The UISemanticContentAttribute subview walk is a UIKit concern.)
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const maui::core::flow_direction resolved = resolved_flow_direction(view);
        platform->resolved_flow_direction = resolved; // headless mirror first (XAML-less suite)
        auto bar = wnative::borrow_as<mux::FrameworkElement>(platform->native);
        if (bar == nullptr)
        {
            return;
        }
        switch (resolved)
        {
            case maui::core::flow_direction::left_to_right:
                bar.FlowDirection(mux::FlowDirection::LeftToRight);
                break;
            case maui::core::flow_direction::right_to_left:
                bar.FlowDirection(mux::FlowDirection::RightToLeft);
                break;
            case maui::core::flow_direction::match_parent:
                bar.ClearValue(mux::FrameworkElement::FlowDirectionProperty());
                break;
        }
    }

    maui::graphics::size progress_bar_handler::get_desired_size(double width_constraint,
                                                                double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return {0, 0};
        }
        if (platform->native == nullptr)
        {
            // XAML-less degradation: the headless partial's nominal track placeholder (100x4), so the
            // backend-agnostic size-request suites see consistent numbers (the android twin's shape).
            return {100.0, 4.0};
        }
        // ViewHandlerExtensions.Windows.cs GetDesiredSizeFromHandler: FrameworkElement.Measure +
        // DesiredSize, with the AdjustForExplicitSize clamp fed from the virtual view's explicit
        // width()/height() (see measure_native).
        const double explicit_width = virtual_view() != nullptr ? virtual_view()->width() : dimension::unset;
        const double explicit_height = virtual_view() != nullptr ? virtual_view()->height() : dimension::unset;
        return wnative::measure_native(platform->native, width_constraint, height_constraint, explicit_width,
                                       explicit_height);
    }

    void progress_bar_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // XAML-less: no native layout to apply
        }
        // The shared Canvas recipe: Canvas.SetLeft/SetTop + explicit Width/Height pin the ProgressBar
        // to the frame (ViewHandlerExtensions.Windows.cs PlatformArrangeHandler on the Canvas model).
        wnative::arrange_native(platform->native, frame);
    }
} // namespace maui::core
