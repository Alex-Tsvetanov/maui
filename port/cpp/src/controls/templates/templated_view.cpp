// maui::controls::templated_view — descriptors, the internal-children store, and the layout pass
// (templated_view.hpp). Ported from TemplatedView.cs (+ TemplateUtilities.OnControlTemplateChanged /
// OnChildRemoved for the property callback and the remove_at root-clearing).
#include "maui/controls/templates/templated_view.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <utility>

#include "maui/controls/templates/template_utilities.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<std::shared_ptr<control_template>>& templated_view::control_template_property()
    {
        // TemplatedView.ControlTemplateProperty: default null, propertyChanged →
        // TemplateUtilities.OnControlTemplateChanged (which reads the NEW value off the control).
        // (The type is qualified inside the class scope — `control_template` alone names the accessor.)
        static const maui::core::bindable_property<std::shared_ptr<maui::controls::control_template>> descriptor{
            "control_template",
            nullptr,
            {.property_changed = [](maui::core::bindable_object& owner,
                                    const std::shared_ptr<maui::controls::control_template>& old_value,
                                    const std::shared_ptr<maui::controls::control_template>& /*new_value*/) {
                template_utilities::on_control_template_changed(dynamic_cast<element&>(owner), old_value.get());
            }}};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::thickness>& templated_view::padding_property()
    {
        static const maui::core::bindable_property<maui::core::thickness> descriptor{"padding"};
        return descriptor;
    }

    void templated_view::add_logical_child(std::shared_ptr<element> child)
    {
        if (child == nullptr || std::ranges::find(internal_children_, child) != internal_children_.end())
        {
            return;
        }
        internal_children_.push_back(std::move(child));
        attach_logical_child(*internal_children_.back());
    }

    bool templated_view::remove_at(int index)
    {
        if (index < 0 || static_cast<std::size_t>(index) >= internal_children_.size())
        {
            return false;
        }
        // Keep the child alive through the detach (the store may hold the only owner).
        const std::shared_ptr<element> removed = internal_children_[static_cast<std::size_t>(index)];
        internal_children_.erase(internal_children_.begin() + index);
        if (removed.get() == template_root_)
        {
            template_root_ = nullptr; // OnChildRemoved → TemplateUtilities.OnChildRemoved
        }
        detach_logical_child(*removed);
        return true;
    }

    // C# CrossPlatformMeasure (MeasureContent): the presented content within the padding; the padding
    // is added back. CrossPlatformArrange (ArrangeContent): record the frame, size the native host,
    // then place the content inside the padding inset — the same shape as content_page.
    maui::graphics::size templated_view::measure(double width_constraint, double height_constraint)
    {
        const maui::core::thickness inset = padding();
        maui::graphics::size content_size{0, 0};
        if (auto* presented = content())
        {
            content_size = presented->measure(width_constraint - inset.horizontal_thickness(),
                                              height_constraint - inset.vertical_thickness());
        }
        const maui::graphics::size measured{content_size.width + inset.horizontal_thickness(),
                                            content_size.height + inset.vertical_thickness()};
        desired_size_ = measured;
        return measured;
    }

    maui::graphics::size templated_view::arrange(const maui::graphics::rect& bounds)
    {
        frame_ = bounds;
        if (auto* view_handler = dynamic_cast<maui::core::i_view_handler*>(handler().get()))
        {
            view_handler->platform_arrange(bounds);
        }
        if (auto* presented = content())
        {
            // The presented content is hosted as a SUBVIEW of the content host (ContentViewHandler.
            // UpdateContent), which platform_arrange framed at `bounds`. A native subview's frame is
            // expressed in its superview's (the host's) coordinate space, whose origin is (0,0) — so the
            // content is arranged HOST-RELATIVE: the padding inset from the host's top-left, with
            // `bounds.x/bounds.y` dropped. Carrying the absolute page origin here would double-offset the
            // content (host frame origin + the same origin again). See border::arrange for the full
            // rationale — these two single-content hosts stay consistent.
            const maui::core::thickness inset = padding();
            presented->arrange({inset.left, inset.top, bounds.width - inset.horizontal_thickness(),
                                bounds.height - inset.vertical_thickness()});
        }
        return {bounds.width, bounds.height};
    }
} // namespace maui::controls
