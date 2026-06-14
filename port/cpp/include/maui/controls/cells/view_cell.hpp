#pragma once
// maui::controls::view_cell  <=  Microsoft.Maui.Controls.ViewCell
//
// A cell containing a developer-defined View. Ported from src/Controls/src/Core/Cells/ViewCell.cs.
//
// The View is a logical child of the cell: setting it removes the old child (clearing its parent),
// attaches the new one (parented to the cell + given the cell's binding context), and requests a size
// refresh (ViewCell.View setter → ForceUpdateSize). Content OWNERSHIP: shared_ptr<element> — the cell
// co-owns its content, like content_view (the GC-reference analog).

#include <memory>
#include <utility>

#include "maui/controls/cells/cell.hpp"
#include "maui/controls/element.hpp"

namespace maui::controls
{
    class view_cell : public cell
    {
    public:
        view_cell()
        {
            this->set_style_target_type<view_cell>();
        }

        // ViewCell.View — the hosted content (co-owned). Null until set.
        [[nodiscard]] const std::shared_ptr<element>& view() const
        {
            return view_;
        }
        void set_view(std::shared_ptr<element> value);

    protected:
        // The hosted view is a logical child — propagate the binding context to it (ViewCell inherits
        // Cell's logical-children visitation; the cell base also visits context actions).
        void for_each_logical_child(const std::function<void(element&)>& visit) const override;

    private:
        std::shared_ptr<element> view_; // ViewCell._view (co-owned content)
    };
} // namespace maui::controls
