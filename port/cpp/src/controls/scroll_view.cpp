// maui::controls::scroll_view — out-of-line definitions: the shared bindable-property descriptors, the
// pending-request flush on handler attach, the measure/arrange layout (the handler-side
// CrossPlatformMeasure + LayoutExtensions.ArrangeContentUnbounded — see scroll_view.hpp for the oracle
// map), and the default-handler self-registration.

#include "maui/controls/scroll_view.hpp"

#include <algorithm>
#include <limits>
#include <memory>
#include <utility>

#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_scroll_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/safe_area_edges.hpp"
#include "maui/core/safe_area_regions.hpp"
#include "maui/core/scroll_bar_visibility.hpp"
#include "maui/core/scroll_orientation.hpp"
#include "maui/core/scroll_to_request.hpp"
#include "maui/core/scroll_view_handler.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::controls
{
    namespace
    {
        constexpr double infinity = std::numeric_limits<double>::infinity();
    } // namespace

    const maui::core::bindable_property<maui::core::thickness>& scroll_view::padding_property()
    {
        // C# ScrollView's IPaddingElement.PaddingDefaultValueCreator returns default(Thickness).
        static const maui::core::bindable_property<maui::core::thickness> descriptor{"padding"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::scroll_orientation>& scroll_view::orientation_property()
    {
        // C# ScrollView.OrientationProperty default is ScrollOrientation.Vertical.
        static const maui::core::bindable_property<maui::core::scroll_orientation> descriptor{
            "orientation", maui::core::scroll_orientation::vertical};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::scroll_bar_visibility>& scroll_view::
        horizontal_scroll_bar_visibility_property()
    {
        // C# default is ScrollBarVisibility.Default.
        static const maui::core::bindable_property<maui::core::scroll_bar_visibility> descriptor{
            "horizontal_scroll_bar_visibility", maui::core::scroll_bar_visibility::default_};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::scroll_bar_visibility>& scroll_view::
        vertical_scroll_bar_visibility_property()
    {
        static const maui::core::bindable_property<maui::core::scroll_bar_visibility> descriptor{
            "vertical_scroll_bar_visibility", maui::core::scroll_bar_visibility::default_};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::safe_area_edges>& scroll_view::safe_area_edges_property()
    {
        // C# ScrollView.SafeAreaEdgesProperty default is SafeAreaEdges.Default (all edges Default).
        static const maui::core::bindable_property<maui::core::safe_area_edges> descriptor{
            "safe_area_edges", maui::core::safe_area_edges::default_edges()};
        return descriptor;
    }

    // C# `Thickness ISafeAreaView2.SafeAreaInsets { set { } }` on ScrollView is a no-op with the comment
    // "For ScrollView, we don't need to store the SafeAreaInsets / The platform-specific MauiScrollView
    // handles this" (ScrollView.cs:559). DOCUMENTED DEVIATION, identical in kind to layout's: MAUI's
    // MauiScrollView holds the insets natively AND arranges the content natively, so the control never
    // needs them; the port arranges CROSS-PLATFORM (scroll_view::arrange plays MauiScrollView.
    // CrossPlatformArrange's role), so the realized insets must reach this object. Headless pushes
    // nothing ⇒ zero ⇒ every safe-area path below is a no-op.
    void scroll_view::set_safe_area_insets(const maui::core::thickness& value)
    {
        safe_area_insets_ = value;
    }

    // C# MauiScrollView.ValidateSafeArea's _safeArea assignment (MauiScrollView.cs:383-386) + the
    // line-389 gate, folded into one:
    //
    //   _safeArea = (SystemAdjustedContentInset == Zero || behavior == Never) ? GetInset()
    //                                                                         : SystemAdjustedContentInset
    //   _appliesSafeAreaAdjustments = !IsParentHandlingSafeArea() && RespondsToSafeArea() && !_safeArea.IsEmpty
    //
    // iOS sets AdjustedContentInset ONLY when the content overflows the scroll view; when it fits, the
    // inset stays zero and the scroll view is responsible for its own safe area (GetInset — per edge,
    // GetManualInsetForEdge zeroes an edge whose region is None). An all-zero result IS `_safeArea.IsEmpty`,
    // so callers need no separate emptiness check.
    //
    // RespondsToSafeArea() (MauiScrollView.cs:124-128) is `Superview.GetParentOfType<UIScrollView>()` — a
    // scroll view nested INSIDE another scroll view defers to the outer one. Note it starts at the
    // SUPERVIEW, so this scroll view never disqualifies itself (unlike layout's, which walks from its own
    // parent for the same reason).
    maui::core::thickness scroll_view::effective_safe_area() const
    {
        for (const element* ancestor = logical_parent(); ancestor != nullptr; ancestor = ancestor->logical_parent())
        {
            if (dynamic_cast<const maui::core::i_scroll_view*>(ancestor) != nullptr)
            {
                return {}; // nested in another scroll view — the outer one owns the insets
            }
        }
        if (system_applied_the_inset())
        {
            return system_adjusted_content_inset_;
        }
        const auto manual = [this](int edge, double value) -> double {
            // C# GetManualInsetForEdge (MauiScrollView.cs:266-273): None ⇒ edge-to-edge ⇒ 0, else the inset.
            return get_safe_area_regions_for_edge(edge) != maui::core::safe_area_regions::none ? value : 0.0;
        };
        return maui::core::thickness{manual(0, safe_area_insets_.left), manual(1, safe_area_insets_.top),
                                     manual(2, safe_area_insets_.right), manual(3, safe_area_insets_.bottom)};
    }

    maui::core::safe_area_regions scroll_view::get_safe_area_regions_for_edge(int edge) const
    {
        return safe_area_edges_.get().edge(edge);
    }

    // ScrollView.OnHandlerChangedCore: flush a request pended while no handler was attached.
    void scroll_view::set_handler(std::shared_ptr<maui::core::i_element_handler> value)
    {
        view<maui::core::i_scroll_view>::set_handler(std::move(value));
        if (handler() && pending_scroll_to_.has_value())
        {
            const maui::core::scroll_to_request pending = *pending_scroll_to_;
            pending_scroll_to_.reset();
            on_scroll_to_requested(pending);
        }
    }

    // The handler-side CrossPlatformMeasure (ScrollViewHandler.iOS.cs — the normalize-all-platforms
    // variant): unconstrain the scrolling dimension(s), measure the content within the padding, clamp
    // the result to the incoming constraints, then resolve against this view's own size requests. With
    // no content, ContentSize resets to Zero (the control-side CrossPlatformMeasure).
    maui::graphics::size scroll_view::measure(double width_constraint, double height_constraint)
    {
        // Margin, per C# LayoutExtensions.ComputeDesiredSize (LayoutExtensions.cs:11-32): the constraint
        // loses it, the reported size regains it, so the PARENT reserves the gap. A measure() OVERRIDE does
        // not inherit view<>::measure's fold and has to repeat it — omitting it is not an under-reserve but
        // an IMBALANCE, because compute_frame subtracts the margin from desired_size regardless (view.hpp
        // :1069/1076). See layout.hpp:175-180 and border.cpp for the same defect, measured. No-op at zero.
        const maui::core::thickness view_margin = margin();
        const double margin_h = view_margin.horizontal_thickness();
        const double margin_v = view_margin.vertical_thickness();

        if (content_ == nullptr)
        {
            content_size_ = {0, 0};
            desired_size_ = {resolve_size_request(0, width(), minimum_width(), maximum_width()) + margin_h,
                             resolve_size_request(0, height(), minimum_height(), maximum_height()) + margin_v};
            return desired_size_;
        }

        // C# MauiScrollView.CrossPlatformMeasure (MauiScrollView.cs:548-562): shrink the constraints by the
        // safe area, measure, then add it back "so the container can allocate the correct space". Zero safe
        // area makes both terms no-ops — exactly C#'s `if (_appliesSafeAreaAdjustments)` guard.
        const maui::core::thickness safe_area = effective_safe_area();
        width_constraint -= safe_area.horizontal_thickness();
        height_constraint -= safe_area.vertical_thickness();
        // The margin comes off the SAME constraints, and before the "smaller of it and the constraints"
        // comparison below — that comparison is against the space actually available to content, which the
        // margin has already claimed.
        width_constraint -= margin_h;
        height_constraint -= margin_v;

        const maui::core::scroll_orientation direction = orientation();
        const bool scrolls_horizontally = direction == maui::core::scroll_orientation::horizontal ||
                                          direction == maui::core::scroll_orientation::both;
        const bool scrolls_vertically =
            direction == maui::core::scroll_orientation::vertical || direction == maui::core::scroll_orientation::both;
        const double content_width_constraint = scrolls_horizontally ? infinity : width_constraint;
        const double content_height_constraint = scrolls_vertically ? infinity : height_constraint;

        // MeasureContent(scrollView, Padding, ...): the content measures within the padding inset.
        const maui::core::thickness inset = padding();
        const maui::graphics::size content_size =
            content_->measure(content_width_constraint - inset.horizontal_thickness(),
                              content_height_constraint - inset.vertical_thickness());
        const maui::graphics::size measured{content_size.width + inset.horizontal_thickness(),
                                            content_size.height + inset.vertical_thickness()};

        // "Our target size is the smaller of it and the constraints."
        const double width_value =
            (measured.width <= width_constraint ? measured.width : width_constraint) + safe_area.horizontal_thickness();
        const double height_value = (measured.height <= height_constraint ? measured.height : height_constraint) +
                                    safe_area.vertical_thickness();

        // AFTER the size-request clamp, not before: in C# that clamp lives inside the handler's
        // GetDesiredSize and ComputeDesiredSize adds the margin to whatever it returned, so an explicit
        // WidthRequest cannot swallow the margin.
        desired_size_ = {resolve_size_request(width_value, width(), minimum_width(), maximum_width()) + margin_h,
                         resolve_size_request(height_value, height(), minimum_height(), maximum_height()) + margin_v};
        return desired_size_;
    }

    // LayoutExtensions.ArrangeContentUnbounded: the content arranges into the LARGER of the bounds and
    // its desired size + Padding (the overflow is the scrollable range); ContentSize then follows the
    // arranged content frame + margin (ScrollView.ContentSizeChanged). The handler frames the native
    // scroller AFTER the content settles, so its scrollable-extent push reads fresh values.
    //
    // The content is hosted as a SUBVIEW of the native UIScrollView (ScrollViewHandler UpdateContentView),
    // and that subview's frame is expressed in the scroller's own CONTENT coordinate space — which starts
    // at the scroller's origin (0,0), not the page origin. So the content is arranged HOST-RELATIVE: the
    // padding inset measured from the scroller's top-left, with `bounds.x/bounds.y` dropped. Carrying the
    // absolute page origin here would double-offset the content (the scroller is already framed at `bounds`
    // by platform_arrange, then the content would be pushed by the same origin AGAIN inside it — a phantom
    // gap the size of the page offset, e.g. the border_playground ScrollView shoved its whole controls
    // panel ~200pt down). In C# MauiScrollView.LayoutSubviews calls CrossPlatformArrange(Bounds) with the
    // UIScrollView's own Bounds (origin 0), so the absolute origin never enters the child rect; the port
    // drives arrange top-down with absolute coordinates, so the container subtracts its origin instead.
    // Mirrors border::arrange / templated_view::arrange (the sibling single-content hosts).
    maui::graphics::size scroll_view::arrange(const maui::graphics::rect& bounds)
    {
        frame_ = bounds;
        if (content_ != nullptr)
        {
            // C# MauiScrollView.CrossPlatformArrange (MauiScrollView.cs:432-455): inset the bounds by the
            // safe area, then arrange the content in one of TWO ways depending on who applied it —
            //
            //   if (SystemAdjustedContentInset == Zero || behavior == Never)
            //       CrossPlatformArrange(bounds.ToRectangle());              // the inset rect, ORIGIN AND ALL
            //   else
            //       CrossPlatformArrange(new Rect(new Point(), bounds.Size)); // 0-origin, inset SIZE only
            //
            // The split is the whole point: when UIKit adjusted the contentInset itself (content overflows)
            // it ALREADY offsets the content visually, so honoring the origin here too would double it.
            // When UIKit declined (content fits), nothing else will inset the content and the origin is the
            // only thing that keeps it out from under the bars/notch. Both branches shrink the SIZE.
            const maui::core::thickness safe_area = effective_safe_area();
            const double safe_x = system_applied_the_inset() ? 0.0 : safe_area.left;
            const double safe_y = system_applied_the_inset() ? 0.0 : safe_area.top;
            const double safe_width = bounds.width - safe_area.horizontal_thickness();
            const double safe_height = bounds.height - safe_area.vertical_thickness();

            const maui::core::thickness inset = padding();
            const maui::graphics::size desired = content_->desired_size();
            const double expanded_width = std::max(safe_width, desired.width + inset.horizontal_thickness());
            const double expanded_height = std::max(safe_height, desired.height + inset.vertical_thickness());
            // ArrangeContent within the expanded bounds: the padding insets off the expanded rect, at the
            // scroller-relative origin (bounds.x/bounds.y dropped — see the header note above) plus the
            // safe-area origin from the branch above.
            content_->arrange({safe_x + inset.left, safe_y + inset.top, expanded_width - inset.horizontal_thickness(),
                               expanded_height - inset.vertical_thickness()});

            const maui::graphics::rect content_frame = content_->frame();
            const maui::core::thickness margin = content_->margin();
            content_size_ = {content_frame.width + margin.horizontal_thickness(),
                             content_frame.height + margin.vertical_thickness()};
        }
        if (auto* view_handler = dynamic_cast<maui::core::i_view_handler*>(handler().get()))
        {
            view_handler->platform_arrange(bounds);
        }
        return {bounds.width, bounds.height};
    }
} // namespace maui::controls

// Self-register the default handler for scroll_view (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::scroll_view, maui::core::scroll_view_handler)
