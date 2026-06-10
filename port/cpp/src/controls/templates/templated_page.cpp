// maui::controls::templated_page — descriptors, the internal-children store, and the layout pass
// (templated_page.hpp). Ported from TemplatedPage.cs (the C# duplicates TemplatedView's
// IControlTemplated implementation on the page root; the port mirrors that).
#include "maui/controls/templates/templated_page.hpp"

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
    const maui::core::bindable_property<std::shared_ptr<control_template>>& templated_page::control_template_property()
    {
        // TemplatedPage.ControlTemplateProperty (its own descriptor, same callback as TemplatedView's).
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

    const maui::core::bindable_property<maui::core::thickness>& templated_page::padding_property()
    {
        static const maui::core::bindable_property<maui::core::thickness> descriptor{"padding"};
        return descriptor;
    }

    void templated_page::add_logical_child(std::shared_ptr<element> child)
    {
        // TemplatedPage.AddLogicalChild: only add when absent (InternalChildren.Contains guard).
        if (child == nullptr || std::ranges::find(internal_children_, child) != internal_children_.end())
        {
            return;
        }
        internal_children_.push_back(std::move(child));
        attach_logical_child(*internal_children_.back());
    }

    bool templated_page::remove_at(int index)
    {
        if (index < 0 || static_cast<std::size_t>(index) >= internal_children_.size())
        {
            return false;
        }
        std::shared_ptr<element> removed = internal_children_[static_cast<std::size_t>(index)];
        internal_children_.erase(internal_children_.begin() + index);
        if (removed.get() == template_root_)
        {
            template_root_ = nullptr; // OnChildRemoved → TemplateUtilities.OnChildRemoved
        }
        detach_logical_child(*removed);
        return true;
    }

    maui::graphics::size templated_page::measure(double width_constraint, double height_constraint)
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

    maui::graphics::size templated_page::arrange(const maui::graphics::rect& bounds)
    {
        frame_ = bounds;
        if (auto* view_handler = dynamic_cast<maui::core::i_view_handler*>(handler().get()))
        {
            view_handler->platform_arrange(bounds);
        }
        if (auto* presented = content())
        {
            const maui::core::thickness inset = padding();
            presented->arrange({bounds.x + inset.left, bounds.y + inset.top,
                                bounds.width - inset.horizontal_thickness(),
                                bounds.height - inset.vertical_thickness()});
        }
        return {bounds.width, bounds.height};
    }
} // namespace maui::controls
