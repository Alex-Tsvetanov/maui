#pragma once
// maui::controls::table_section  <=  Microsoft.Maui.Controls.TableSection
//   (over the generic Microsoft.Maui.Controls.TableSectionBase<Cell>)
//
// A logical grouping of cells in a table_view. Ported from src/Controls/src/Core/TableView/TableSection.cs
// (the TableSection : TableSectionBase<Cell> half). A list of co-owned cells with a collection_changed
// notification + binding-context inheritance: every cell added to the section receives the section's
// binding context (TableSectionBase<T>.OnChildrenChanged / OnBindingContextChanged).
//
// collection_changed mirrors INotifyCollectionChanged for the table_root/table_view to observe (the C#
// section's CollectionChanged). Indices ride along so the root can re-parent only the new cells.

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "maui/controls/cells/cell.hpp"
#include "maui/controls/observable_collection.hpp"
#include "maui/controls/table_section_base.hpp"
#include "maui/core/event.hpp"

namespace maui::controls
{
    class table_section : public table_section_base
    {
    public:
        table_section() = default;
        explicit table_section(std::string title) : table_section_base(std::move(title))
        {
        }

        // INotifyCollectionChanged (TableSection.CollectionChanged) — fired on add/insert/remove/clear.
        maui::core::event<collection_changed_args<std::shared_ptr<cell>>> collection_changed;

        [[nodiscard]] std::size_t count() const
        {
            return cells_.size();
        }
        [[nodiscard]] const std::shared_ptr<cell>& at(std::size_t index) const
        {
            return cells_.at(index);
        }
        [[nodiscard]] const std::vector<std::shared_ptr<cell>>& cells() const
        {
            return cells_;
        }
        [[nodiscard]] std::ptrdiff_t index_of(const std::shared_ptr<cell>& item) const;

        void add(std::shared_ptr<cell> item);
        void insert(std::size_t index, std::shared_ptr<cell> item);
        bool remove(const std::shared_ptr<cell>& item);
        void remove_at(std::size_t index);
        void clear();

    protected:
        // TableSectionBase<T>.OnBindingContextChanged: push the new context into every cell.
        void on_binding_context_changed() override;

    private:
        // Give the new cell the section's binding context (TableSectionBase<T>.OnChildrenChanged), then
        // raise collection_changed(add) at `index`.
        void notify_added(std::size_t index);

        std::vector<std::shared_ptr<cell>> cells_; // TableSectionBase<T>._children (co-owned)
    };
} // namespace maui::controls
