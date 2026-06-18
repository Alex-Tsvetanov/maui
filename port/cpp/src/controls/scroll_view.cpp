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

    void scroll_view::set_safe_area_insets(const maui::core::thickness& value)
    {
        (void)value;
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
        if (content_ == nullptr)
        {
            content_size_ = {0, 0};
            desired_size_ = {resolve_size_request(0, width(), minimum_width(), maximum_width()),
                             resolve_size_request(0, height(), minimum_height(), maximum_height())};
            return desired_size_;
        }

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
        const double width_value = measured.width <= width_constraint ? measured.width : width_constraint;
        const double height_value = measured.height <= height_constraint ? measured.height : height_constraint;

        desired_size_ = {resolve_size_request(width_value, width(), minimum_width(), maximum_width()),
                         resolve_size_request(height_value, height(), minimum_height(), maximum_height())};
        return desired_size_;
    }

    // LayoutExtensions.ArrangeContentUnbounded: the content arranges into the LARGER of the bounds and
    // its desired size + Padding (the overflow is the scrollable range); ContentSize then follows the
    // arranged content frame + margin (ScrollView.ContentSizeChanged). The handler frames the native
    // scroller AFTER the content settles, so its scrollable-extent push reads fresh values.
    maui::graphics::size scroll_view::arrange(const maui::graphics::rect& bounds)
    {
        frame_ = bounds;
        if (content_ != nullptr)
        {
            const maui::core::thickness inset = padding();
            const maui::graphics::size desired = content_->desired_size();
            const double expanded_width = std::max(bounds.width, desired.width + inset.horizontal_thickness());
            const double expanded_height = std::max(bounds.height, desired.height + inset.vertical_thickness());
            // ArrangeContent within the expanded bounds: the padding insets off the expanded rect.
            content_->arrange({bounds.x + inset.left, bounds.y + inset.top,
                               expanded_width - inset.horizontal_thickness(),
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
