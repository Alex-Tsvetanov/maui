// maui::controls::cell — the shared base: IsEnabled descriptor, Height/RenderHeight, the activation +
// lifecycle events, ContextActions parenting, and the rate-limited ForceUpdateSize. See cell.hpp;
// ported from src/Controls/src/Core/Cells/Cell.cs.

#include "maui/controls/cells/cell.hpp"

#include <functional>
#include <memory>
#include <utility>

#include "maui/controls/element.hpp"
#include "maui/controls/menu_item.hpp"
#include "maui/core/bindable_property.hpp"

namespace maui::controls
{
    // Lets the IsEnabledProperty descriptor lambda reach the protected change-notification helper (the
    // C# analog is the static OnIsEnabledPropertyChanged delegate living inside the Cell class itself).
    struct cell_descriptor_access
    {
        static void notify_has_context_actions_changed(cell& self)
        {
            self.notify_has_context_actions_changed();
        }
    };

    cell::cell() = default;
    cell::~cell() = default;

    const maui::core::bindable_property<bool>& cell::is_enabled_property()
    {
        // Cell.IsEnabledProperty: default true; a change re-raises HasContextActions.
        static const maui::core::bindable_property<bool> descriptor{
            "is_enabled",
            true,
            {.property_changed = [](maui::core::bindable_object& bindable, const bool& /*old_value*/,
                                    const bool& /*new_value*/) {
                // Cell.OnIsEnabledPropertyChanged: OnPropertyChanged(nameof(HasContextActions)).
                cell_descriptor_access::notify_has_context_actions_changed(dynamic_cast<cell&>(bindable));
            }}};
        return descriptor;
    }

    void cell::set_height(double value)
    {
        // Cell.Height: equal short-circuit; otherwise fire changing/changed for both Height + RenderHeight.
        if (height_ == value)
        {
            return;
        }
        on_property_changing("height");
        on_property_changing("render_height");
        height_ = value;
        on_property_changed("height");
        on_property_changed("render_height");
    }

    double cell::render_height() const
    {
        // Cell.RenderHeight: the table/list parent decides via HasUnevenRows + RowHeight; default otherwise.
        if (const auto* container = dynamic_cast<const i_cell_container*>(logical_parent()))
        {
            return (container->has_uneven_rows() && height_ > 0) ? height_
                                                                 : static_cast<double>(container->row_height());
        }
        return static_cast<double>(default_cell_height);
    }

    void cell::add_context_action(std::shared_ptr<menu_item> item)
    {
        if (item == nullptr)
        {
            return;
        }
        // Cell.OnContextActionsChanged: parent each new item to the cell + flow the binding context down.
        attach_logical_child(*item);
        context_actions_.push_back(std::move(item));
        // HasContextActions just flipped from false (or stayed true) — notify like C# does.
        on_property_changed("has_context_actions");
    }

    void cell::on_tapped()
    {
        tapped.raise();
    }

    void cell::on_appearing()
    {
        appearing.raise();
    }

    void cell::on_disappearing()
    {
        disappearing.raise();
    }

    void cell::send_appearing()
    {
        on_appearing();
    }

    void cell::send_disappearing()
    {
        on_disappearing();
    }

    void cell::force_update_size()
    {
        // Cell.ForceUpdateSize: collapse repeated calls to one outstanding request, and only when the
        // parent table/list has uneven rows (otherwise the call is a no-op).
        if (force_update_queued_)
        {
            return;
        }
        const auto* container = dynamic_cast<const i_cell_container*>(logical_parent());
        if (container == nullptr || !container->has_uneven_rows())
        {
            return;
        }
        force_update_queued_ = true;
        // C# also invokes the Cell.Handler's "ForceUpdateSizeRequested" command; the port's cell is a
        // plain element (no own handler — the table/list realizes it), so the event below IS the seam the
        // table_view_handler listens on. No UI thread to await, so the queued flag resets immediately.
        force_update_size_requested.raise();
        force_update_queued_ = false;
    }

    void cell::for_each_logical_child(const std::function<void(element&)>& visit) const
    {
        for (const auto& item : context_actions_)
        {
            if (item != nullptr)
            {
                visit(*item);
            }
        }
    }
} // namespace maui::controls
