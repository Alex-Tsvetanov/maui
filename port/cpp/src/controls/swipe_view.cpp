// maui::controls::swipe_view — out-of-line definitions: the shared bindable-property descriptors, the
// ctor (mint + parent the four collections), Content + collection replacement, IsOpen/Open/Close + the
// RequestOpen/RequestClose command routing, the MeasureContent/ArrangeContent layout, and the default
// handler self-registration. Ported from SwipeView.cs.

#include "maui/controls/swipe_view.hpp"

#include <algorithm>
#include <functional>
#include <memory>
#include <utility>

#include "maui/controls/element.hpp"
#include "maui/controls/swipe_items.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/open_swipe_item.hpp"
#include "maui/core/swipe_direction.hpp"
#include "maui/core/swipe_view_handler.hpp"
#include "maui/core/swipe_view_requests.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<double>& swipe_view::threshold_property()
    {
        // C# SwipeView.ThresholdProperty default is default(double) = 0.
        static const maui::core::bindable_property<double> descriptor{"threshold", 0.0};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::thickness>& swipe_view::padding_property()
    {
        // SwipeView is a ContentView; its Padding default is default(Thickness) = 0.
        static const maui::core::bindable_property<maui::core::thickness> descriptor{"padding"};
        return descriptor;
    }

    swipe_view::swipe_view()
    {
        this->set_style_target_type<swipe_view>();

        // C# SwipeView ctor: the four collections are minted by the defaultValueCreator and added as
        // logical children (AddLogicalChild(RightItems/LeftItems/TopItems/BottomItems)). Mint them here,
        // parent each, and subscribe to each collection's `changed` to re-push to the handler.
        left_items_ = std::make_unique<swipe_items>();
        right_items_ = std::make_unique<swipe_items>();
        top_items_ = std::make_unique<swipe_items>();
        bottom_items_ = std::make_unique<swipe_items>();
        attach_logical_child(*left_items_);
        attach_logical_child(*right_items_);
        attach_logical_child(*top_items_);
        attach_logical_child(*bottom_items_);
        left_items_sub_ = subscribe_items(*left_items_, "left_items");
        right_items_sub_ = subscribe_items(*right_items_, "right_items");
        top_items_sub_ = subscribe_items(*top_items_, "top_items");
        bottom_items_sub_ = subscribe_items(*bottom_items_, "bottom_items");
    }

    swipe_view::~swipe_view() = default;

    maui::core::scoped_connection swipe_view::subscribe_items(swipe_items& items, const char* update_name)
    {
        // C# OnSwipeItemsChanged: a collection change (or Mode/Behavior property change, both folded into
        // swipe_items::changed) calls Handler.UpdateValue(nameof(LeftItems/...)). Capture the stable
        // update-name pointer (a string literal) and re-push through the handler.
        return maui::core::connect_scoped(items.changed, [this, update_name] {
            if (const auto& element_handler = handler())
            {
                element_handler->update_value(update_name);
            }
        });
    }

    void swipe_view::replace_items(std::unique_ptr<swipe_items>& slot, maui::core::scoped_connection& sub,
                                   std::unique_ptr<swipe_items> value, const char* update_name)
    {
        if (value == nullptr || value.get() == slot.get())
        {
            return;
        }
        // §8: disconnect the old subscription BEFORE the old collection dies (the closure references its
        // `changed` event). reset() runs while the old collection is still alive (slot still owns it).
        sub.reset();
        if (slot != nullptr)
        {
            detach_logical_child(*slot);
        }
        slot = std::move(value);
        attach_logical_child(*slot);
        sub = subscribe_items(*slot, update_name);
        // C# SwipeView.LeftItems setter re-enters the property path; the port pushes the whole-collection
        // change to the handler so it re-reads the items.
        if (const auto& element_handler = handler())
        {
            element_handler->update_value(update_name);
        }
    }

    void swipe_view::set_left_items(std::unique_ptr<swipe_items> value)
    {
        replace_items(left_items_, left_items_sub_, std::move(value), "left_items");
    }

    void swipe_view::set_right_items(std::unique_ptr<swipe_items> value)
    {
        replace_items(right_items_, right_items_sub_, std::move(value), "right_items");
    }

    void swipe_view::set_top_items(std::unique_ptr<swipe_items> value)
    {
        replace_items(top_items_, top_items_sub_, std::move(value), "top_items");
    }

    void swipe_view::set_bottom_items(std::unique_ptr<swipe_items> value)
    {
        replace_items(bottom_items_, bottom_items_sub_, std::move(value), "bottom_items");
    }

    void swipe_view::set_content(maui::core::i_view* value)
    {
        if (content_ == value)
        {
            return;
        }
        if (auto* old_child = dynamic_cast<element*>(content_))
        {
            detach_logical_child(*old_child);
        }
        content_ = value;
        if (auto* new_child = dynamic_cast<element*>(content_))
        {
            attach_logical_child(*new_child);
        }
        if (const auto& element_handler = handler())
        {
            element_handler->invoke("set_content");
        }
    }

    void swipe_view::set_is_open(bool value)
    {
        // C# ISwipeView.IsOpen setter: only notify the handler when the value actually changes.
        if (is_open_ == value)
        {
            return;
        }
        is_open_ = value;
        if (const auto& element_handler = handler())
        {
            element_handler->update_value("is_open");
        }
    }

    void swipe_view::open(maui::core::open_swipe_item item, bool animated)
    {
        // C# SwipeView.Open: raise OpenRequested, then RequestOpen.
        const maui::core::swipe_view_open_request request{.item = item, .animated = animated};
        open_requested.raise(request);
        request_open(request);
    }

    void swipe_view::close(bool animated)
    {
        // C# SwipeView.Close: raise CloseRequested, then RequestClose.
        const maui::core::swipe_view_close_request request{.animated = animated};
        close_requested.raise(request);
        request_close(request);
    }

    void swipe_view::request_open(const maui::core::swipe_view_open_request& request)
    {
        // C# ISwipeView.RequestOpen: derive the swipe direction from the side, then route to the handler.
        switch (request.item)
        {
            case maui::core::open_swipe_item::left_items:
                last_swipe_direction_ = maui::core::swipe_direction::right;
                break;
            case maui::core::open_swipe_item::top_items:
                last_swipe_direction_ = maui::core::swipe_direction::down;
                break;
            case maui::core::open_swipe_item::right_items:
                last_swipe_direction_ = maui::core::swipe_direction::left;
                break;
            case maui::core::open_swipe_item::bottom_items:
                last_swipe_direction_ = maui::core::swipe_direction::up;
                break;
        }
        if (const auto& element_handler = handler())
        {
            element_handler->invoke("request_open", request);
        }
    }

    void swipe_view::request_close(const maui::core::swipe_view_close_request& request)
    {
        if (const auto& element_handler = handler())
        {
            element_handler->invoke("request_close", request);
        }
    }

    void swipe_view::for_each_logical_child(const std::function<void(element&)>& visit) const
    {
        // The four collections always exist; Content is optional.
        if (auto* child = dynamic_cast<element*>(content_))
        {
            visit(*child);
        }
        visit(*left_items_);
        visit(*right_items_);
        visit(*top_items_);
        visit(*bottom_items_);
    }

    // MeasureContent: measure the single content within the Padding, then resolve against this view's own
    // size requests (the content_page recipe). No content → just the padding within the requests.
    maui::graphics::size swipe_view::measure(double width_constraint, double height_constraint)
    {
        const maui::core::thickness inset = padding();
        const maui::core::thickness view_margin = margin();
        maui::graphics::size content_size{0, 0};
        if (content_ != nullptr)
        {
            // Margin is EXCLUDED from the content's available space (the constraints shrink by it), like the
            // leaf view<>::measure (view.hpp ComputeDesiredSize) and border::measure's inset handling.
            content_size =
                content_->measure(width_constraint - inset.horizontal_thickness() - view_margin.horizontal_thickness(),
                                  height_constraint - inset.vertical_thickness() - view_margin.vertical_thickness());
        }
        const maui::graphics::size measured{content_size.width + inset.horizontal_thickness(),
                                            content_size.height + inset.vertical_thickness()};
        // Resolve against this view's own size requests, then ADD the Margin back into the reported desired
        // size (C# LayoutExtensions.ComputeDesiredSize) so the parent layout RESERVES it; arrange's
        // compute_frame subtracts it back out (measure adds, arrange subtracts — balanced at zero margin, so
        // the swipe-in-a-CollectionView common case is unchanged). Without this, N SwipeViews each with a
        // Margin (basic_swipe: 5 × Margin=12) render as one gapless block because the stack never reserved
        // the gaps.
        desired_size_ = {resolve_size_request(measured.width, width(), minimum_width(), maximum_width()) +
                             view_margin.horizontal_thickness(),
                         resolve_size_request(measured.height, height(), minimum_height(), maximum_height()) +
                             view_margin.vertical_thickness()};
        return desired_size_;
    }

    // ArrangeContent: frame this view (size the native host to `bounds`, as C# ContentView's ArrangeOverride
    // does), then arrange the single content within the Padding.
    //
    // The content is hosted as a SUBVIEW of the swipe host (SwipeViewHandler.UpdateContent), and that host is
    // framed at `bounds` by platform_arrange below. A native subview's frame is expressed in its superview's
    // coordinate space — which, for the host, starts at the host's own origin (0,0), not the page origin. So
    // the content is arranged HOST-RELATIVE: the Padding inset measured from the host's top-left, with
    // `bounds.x`/`bounds.y` dropped. Carrying the absolute page origin here would DOUBLE-OFFSET the content
    // (host frame origin + the same origin re-added by the content rect) — the swipe-in-a-stack bug where
    // each row's content was pushed down by the row's own Y (a SwipeView at y=72 in a stack rendered its
    // content at y=144). In C# the native LayoutSubviews hands each container its own 0-origin Bounds, so the
    // absolute origin never enters the child rect; the port drives arrange top-down with absolute coordinates,
    // so the host subtracts its origin instead. Mirrors border::arrange / templated_view::arrange, the sibling
    // single-content hosts. The host is framed FIRST so its bounds are set before the content lands.
    maui::graphics::size swipe_view::arrange(const maui::graphics::rect& bounds)
    {
        // View : resolve the aligned, size-requested, margin-inset FRAME within the allotted `bounds`, the
        // same LayoutExtensions.ComputeFrame the leaf view<>::arrange + border::arrange run. With Margin=0 and
        // a Fill alignment this returns `bounds` (the swipe-in-a-CollectionView common case is unchanged);
        // with an explicit Width/Height or a Margin (basic_swipe: 5 rows each Margin=12, WidthRequest=300) it
        // subtracts the margin measure reserved and positions the host inside its slot, producing the gaps.
        const maui::graphics::rect frame = compute_frame(bounds);
        frame_ = frame;
        if (auto* view_handler = dynamic_cast<maui::core::i_view_handler*>(handler().get()))
        {
            view_handler->platform_arrange(frame);
        }
        if (content_ != nullptr)
        {
            const maui::core::thickness inset = padding();
            content_->arrange({inset.left, inset.top, std::max(0.0, frame.width - inset.horizontal_thickness()),
                               std::max(0.0, frame.height - inset.vertical_thickness())});
        }
        return {frame.width, frame.height};
    }
} // namespace maui::controls

// Self-register the default handler for swipe_view (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::swipe_view, maui::core::swipe_view_handler)
