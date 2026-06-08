// maui::controls::element — tree-lifecycle propagation (element.hpp).
#include "maui/controls/element.hpp"

#include "maui/core/bindable_object.hpp"

namespace maui::controls
{
    void element::on_binding_context_changed()
    {
        maui::core::bindable_object::on_binding_context_changed(); // raise binding_context_changed
        const auto& context = raw_binding_context();
        for_each_logical_child([&context](element& child) { child.set_inherited_binding_context(context); });
    }

    void element::set_containing_window(window* value)
    {
        if (window_ == value)
        {
            return;
        }
        const bool was_attached = window_ != nullptr;
        window_ = value;
        if (value != nullptr && !was_attached)
        {
            loaded.raise(); // attach: this element is now in a window — fire before children attach
        }
        for_each_logical_child([value](element& child) { child.set_containing_window(value); });
        if (value == nullptr && was_attached)
        {
            unloaded.raise(); // detach: children have detached first; this element leaves the window last
        }
    }

    void element::attach_logical_child(element& child)
    {
        child.set_inherited_binding_context(raw_binding_context());
        child.set_containing_window(window_);
    }

    void element::detach_logical_child(element& child)
    {
        child.set_containing_window(nullptr);
    }
} // namespace maui::controls
