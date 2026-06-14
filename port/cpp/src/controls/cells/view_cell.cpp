// maui::controls::view_cell — the View setter (logical-child swap + size refresh) + the logical-children
// visitation. See view_cell.hpp; ported from src/Controls/src/Core/Cells/ViewCell.cs.

#include "maui/controls/cells/view_cell.hpp"

#include <functional>
#include <memory>
#include <utility>

#include "maui/controls/cells/cell.hpp"
#include "maui/controls/element.hpp"

namespace maui::controls
{
    void view_cell::set_view(std::shared_ptr<element> value)
    {
        // ViewCell.View setter: equal short-circuit; otherwise detach the old child, attach the new one
        // (parented + given the cell's binding context), then request a size refresh.
        if (view_ == value)
        {
            return;
        }
        on_property_changing("view");
        if (view_ != nullptr)
        {
            detach_logical_child(*view_);
        }
        view_ = std::move(value);
        if (view_ != nullptr)
        {
            attach_logical_child(*view_);
        }
        force_update_size();
        on_property_changed("view");
    }

    void view_cell::for_each_logical_child(const std::function<void(element&)>& visit) const
    {
        // Visit the context actions (the cell base's children) then the hosted view.
        cell::for_each_logical_child(visit);
        if (view_ != nullptr)
        {
            visit(*view_);
        }
    }
} // namespace maui::controls
