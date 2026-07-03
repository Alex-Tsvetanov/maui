// indicator_view_handler — Windows (WinUI 3) platform partial: the position-indicator dot row drawn by
// hand as a horizontal Microsoft.UI.Xaml.Controls.StackPanel of Microsoft.UI.Xaml.Shapes dots (Ellipse
// for a circle, Rectangle for a square) — the current page tinted with SelectedIndicatorColor, the rest
// with IndicatorColor. The windows twin of the android hand-drawn dot row
// (src/platform/android/indicator_view_handler.cpp) and the apple NSStackView-of-dots
// (src/platform/apple/indicator_view_handler.mm), and the real-native sibling of the headless mirror
// (src/platform/headless/indicator_view_handler.cpp).
//
// Ported from IndicatorViewHandler.cs (the cross-platform mapper + GetMaximumVisible/IsCircleShape, in
// src/core/indicator_view_handler.cpp) + the IndicatorViewExtensions ResetIndicators dot-assembly recipe
// + MauiPageControl.cs (the GetCurrentPage clamp). The whole row is rebuilt on any count / size / shape /
// color / position change (the C# UpdateIndicatorCount + ResetIndicators collapsed — a handful of tiny
// Shapes, so a full rebuild is cheap, exactly as the android/apple twins do). The cross-platform mirror
// (dot_count / current_page / size / shape / colors) is written FIRST so the XAML-less cross-platform
// suite observes the headless partial's behavior unchanged.
//
// DOCUMENTED DEVIATIONS (infrastructure gaps of this first cut, not behavior guesses):
//   - The host is a stock StackPanel, not C#'s Windows MauiPageControl (an ItemsRepeater-backed template
//     host): the C# IndicatorTemplate (a custom per-dot view) is OMITTED — the port renders default dots
//     only, the same template collapse the android/apple twins document.
//   - The dot-tap inbound channel (writing Position back) is DEFERRED with the gesture fan-out — the
//     Shapes are inert, exactly like the apple twin's NSViews and the android plain-View dots.
//
// XAML-less degradation (the windows twin of the android VM-less fallback): the windows preset also runs
// the cross-platform suite on the host where NO XAML runtime exists; create_platform_view catches the
// construction failure and keeps native null, while the headless mirrors are ALWAYS maintained.

#include "maui/core/indicator_view_handler.hpp"

#include <algorithm>
#include <memory>
#include <string_view>

#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h> // Panel/FrameworkElement base-class consume methods
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.Shapes.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h> // the Children UIElementCollection consume methods
#include <winrt/Windows.Foundation.h>
#include <winrt/base.h>

#include "maui/controls/indicator_shape.hpp"
#include "maui/core/i_indicator_view.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "windows_native.hpp"

namespace
{
    namespace mux = winrt::Microsoft::UI::Xaml;
    namespace muxc = winrt::Microsoft::UI::Xaml::Controls;
    namespace muxs = winrt::Microsoft::UI::Xaml::Shapes;
    namespace wnative = maui::platform::win;

    // IndicatorView's default dot diameter (dp) — the C# DefaultIndicatorSize the android twin renders
    // (6); an unset/non-positive IndicatorSize falls back to it, like FontManager's "unset → default".
    constexpr double k_default_indicator_size = 6.0;
    // The inter-dot gap (dp) — the apple NSStackView spacing == 4 (the C# DefaultPadding); mirrored so
    // the windows row reads identically (StackPanel.Spacing).
    constexpr double k_dot_spacing = 4.0;

    [[nodiscard]] muxc::StackPanel panel_of(const maui::core::indicator_view_platform& platform)
    {
        return wnative::borrow<muxc::StackPanel>(platform.native);
    }
} // namespace

namespace maui::core
{
    indicator_view_platform::indicator_view_platform() = default;

    // Releases the one strong ref pinning the StackPanel host (the wnative shape of the pimpl-owned-
    // native doctrine; the android twin deletes its JNI global ref here). The dot Shapes die with it.
    indicator_view_platform::~indicator_view_platform()
    {
        wnative::release(native);
    }

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each
    // calls the base body FIRST — the headless mirrors must stay live for the XAML-less cross-platform
    // suite (header note) — then pushes to the real panel when one exists.

    void indicator_view_platform::update_visibility(maui::core::visibility value)
    {
        view_platform_base::update_visibility(value);
        // ViewExtensions.UpdateVisibility (Windows): Hidden rides Opacity 0, Collapsed collapses.
        wnative::apply_visibility(native, value, alpha);
    }

    void indicator_view_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        // ViewExtensions.UpdateOpacity: the Hidden state pins opacity 0 (apply_visibility restores it).
        if (!hidden)
        {
            wnative::apply_opacity(native, value);
        }
    }

    void indicator_view_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        // ViewExtensions.UpdateAutomationId: AutomationProperties.SetAutomationId.
        wnative::apply_automation_id(native, value);
    }

    void indicator_view_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        auto panel = panel_of(*this);
        if (panel == nullptr)
        {
            return;
        }
        // The band behind the dots (the Colors row's Yellow): Panel.Background — the dots are CHILDREN
        // of the panel (not its background), so the fill sits behind them, exactly the iOS UIPageControl
        // band. A null paint clears back to the default (no fill).
        if (value == nullptr)
        {
            panel.ClearValue(muxc::Panel::BackgroundProperty());
            return;
        }
        // Paint.ToPlatform: solid + linear/radial gradient (to_paint_brush); image/pattern still fall back to solid.
        panel.Background(wnative::to_paint_brush(value));
        return;
        // deferred: gradient / image-source paints (Paint.ToPlatform) — the base mirror above keeps the
        // borrow observable.
    }

    std::unique_ptr<indicator_view_platform> indicator_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<indicator_view_platform>();
        try
        {
            // The dot-row host: a horizontal StackPanel with the 4dp inter-dot gap (the apple
            // NSStackView spacing / the android absolute-gap twin).
            const muxc::StackPanel panel;
            panel.Orientation(muxc::Orientation::Horizontal);
            panel.Spacing(k_dot_spacing);
            platform->native = wnative::store(panel); // released in ~indicator_view_platform
        }
        catch (const winrt::hresult_error&)
        {
            platform->native = nullptr; // XAML-less degradation (header note)
        }
        return platform;
    }

    void indicator_view_handler::on_connect_handler(indicator_view_platform& /*platform*/)
    {
        // C# ConnectHandler: SetIndicatorView + UpdateIndicator. The dot row is built by the mapper pass
        // (which runs right after connect), so no extra work here (the android/apple twins are identical).
    }

    void indicator_view_handler::on_disconnect_handler(indicator_view_platform& /*platform*/)
    {
    }

    namespace
    {
        // Rebuild the dot row: GetMaximumVisible dots sized IndicatorSize, the current page tinted with
        // the selected color, the rest the indicator color (the C# UpdateIndicatorCount + ResetIndicators
        // collapsed — a full rebuild on any change, exactly as the android/apple twins rebuild). The
        // cross-platform mirror is written FIRST so the XAML-less suite observes it even when no native
        // panel exists.
        void rebuild_dots(indicator_view_handler& handler, i_indicator_view& view)
        {
            auto* platform = handler.typed_platform_view();
            if (platform == nullptr)
            {
                return;
            }
            const int dots = max_visible_indicators(view);
            const int position = view.position();
            // MauiPageControl.GetCurrentPage: clamp the position into [0, dot_count - 1].
            const int current = dots > 0 ? std::min(position, dots - 1) : -1;
            double size = view.indicator_size();
            if (!(size > 0))
            {
                size = k_default_indicator_size; // unset → the C# DefaultIndicatorSize
            }

            // Mirror first (the oracle record) — the same fields the headless partial records.
            platform->dot_count = dots;
            platform->current_page = current;
            platform->indicator_size = view.indicator_size();
            platform->shape = view.indicators_shape();
            platform->indicator_color = view.indicator_color();
            platform->selected_indicator_color = view.selected_indicator_color();

            auto panel = panel_of(*platform);
            if (panel == nullptr)
            {
                return; // XAML-less: the headless mirror is the asserted surface
            }
            const bool square = view.indicators_shape() == maui::controls::indicator_shape::square;

            // Drop the previous row (ResetIndicators rebuilds from scratch).
            panel.Children().Clear();
            for (int index = 0; index < dots; ++index)
            {
                const maui::graphics::color tint =
                    index == current ? view.selected_indicator_color() : view.indicator_color();
                // One dot: an Ellipse (circle) or Rectangle (the square-shape swap — UIPageControl's
                // "squareshape.fill" collapsed to the plain Shape, like the android GradientDrawable).
                muxs::Shape dot{nullptr};
                if (square)
                {
                    dot = muxs::Rectangle{};
                }
                else
                {
                    dot = muxs::Ellipse{};
                }
                dot.Width(size);
                dot.Height(size);
                dot.Fill(wnative::to_brush(tint));
                panel.Children().Append(dot);
            }
        }
    } // namespace

    void indicator_view_handler::map_count(indicator_view_handler& handler, i_indicator_view& view)
    {
        rebuild_dots(handler, view); // UpdateIndicatorCount
    }

    void indicator_view_handler::map_maximum_visible(indicator_view_handler& handler, i_indicator_view& view)
    {
        rebuild_dots(handler, view); // UpdateIndicatorCount
    }

    void indicator_view_handler::map_hide_single(indicator_view_handler& handler, i_indicator_view& view)
    {
        rebuild_dots(handler, view); // UpdateHideSingle → recount
    }

    void indicator_view_handler::map_position(indicator_view_handler& handler, i_indicator_view& view)
    {
        rebuild_dots(handler, view); // re-tint the selected dot
    }

    void indicator_view_handler::map_indicator_size(indicator_view_handler& handler, i_indicator_view& view)
    {
        rebuild_dots(handler, view);
    }

    void indicator_view_handler::map_indicator_color(indicator_view_handler& handler, i_indicator_view& view)
    {
        rebuild_dots(handler, view);
    }

    void indicator_view_handler::map_selected_indicator_color(indicator_view_handler& handler, i_indicator_view& view)
    {
        rebuild_dots(handler, view);
    }

    void indicator_view_handler::map_indicator_shape(indicator_view_handler& handler, i_indicator_view& view)
    {
        rebuild_dots(handler, view);
    }

    maui::graphics::size indicator_view_handler::get_desired_size(double /*width_constraint*/,
                                                                  double /*height_constraint*/) const
    {
        // A row of `dot_count` dots of indicator_size each, with the 4dp gap between them — the natural
        // dot-row extent (the dp the cross-platform measure speaks; XAML DIPs are 1:1). Mirrors the
        // android metric, defaulting an unset size to the C# DefaultIndicatorSize. Height is one dot.
        const auto* platform = typed_platform_view();
        double size = platform != nullptr ? platform->indicator_size : k_default_indicator_size;
        if (!(size > 0))
        {
            size = k_default_indicator_size;
        }
        const int dots = platform != nullptr ? platform->dot_count : 0;
        if (dots <= 0)
        {
            return {0, 0}; // a hidden / empty indicator (HideSingle collapse) reserves nothing
        }
        const double width = (size * dots) + (k_dot_spacing * (dots - 1));
        return {width, size};
    }

    void indicator_view_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // XAML-less: no native layout to apply
        }
        // The shared Canvas recipe: Canvas.SetLeft/SetTop + explicit Width/Height pin the panel to the
        // frame (ViewHandlerExtensions.Windows.cs PlatformArrangeHandler on the Canvas layout model).
        wnative::arrange_native(platform->native, frame);
    }
} // namespace maui::core
