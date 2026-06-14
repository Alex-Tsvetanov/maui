// maui::controls::table_section — the cell list with collection_changed + binding-context inheritance.
// See table_section.hpp; ported from src/Controls/src/Core/TableView/TableSection.cs (the
// TableSectionBase<Cell> half).

#include "maui/controls/table_section.hpp"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>
#include <utility>
#include <vector>

#include "maui/controls/cells/cell.hpp"
#include "maui/controls/observable_collection.hpp"
#include "maui/controls/table_section_base.hpp"

namespace maui::controls
{
    std::ptrdiff_t table_section::index_of(const std::shared_ptr<cell>& item) const
    {
        const auto found = std::ranges::find(cells_, item);
        return found == cells_.end() ? -1 : std::distance(cells_.begin(), found);
    }

    void table_section::add(std::shared_ptr<cell> item)
    {
        insert(cells_.size(), std::move(item));
    }

    void table_section::insert(std::size_t index, std::shared_ptr<cell> item)
    {
        if (item == nullptr || index > cells_.size())
        {
            return;
        }
        cells_.insert(cells_.begin() + static_cast<std::ptrdiff_t>(index), std::move(item));
        notify_added(index);
    }

    bool table_section::remove(const std::shared_ptr<cell>& item)
    {
        const auto found = std::ranges::find(cells_, item);
        if (found == cells_.end())
        {
            return false;
        }
        remove_at(static_cast<std::size_t>(std::distance(cells_.begin(), found)));
        return true;
    }

    void table_section::remove_at(std::size_t index)
    {
        if (index >= cells_.size())
        {
            return;
        }
        std::shared_ptr<cell> removed = cells_[index];
        cells_.erase(cells_.begin() + static_cast<std::ptrdiff_t>(index));
        collection_changed_args<std::shared_ptr<cell>> args;
        args.action = collection_changed_action::remove;
        args.old_items.push_back(std::move(removed));
        args.old_starting_index = static_cast<std::ptrdiff_t>(index);
        collection_changed.raise(args);
    }

    void table_section::clear()
    {
        // ObservableCollection.Clear raises Reset even when already empty (observable_collection mirrors).
        cells_.clear();
        collection_changed_args<std::shared_ptr<cell>> args;
        args.action = collection_changed_action::reset;
        collection_changed.raise(args);
    }

    void table_section::on_binding_context_changed()
    {
        // TableSectionBase<T>.OnBindingContextChanged: push the new context into every cell.
        table_section_base::on_binding_context_changed();
        const auto& context = raw_binding_context();
        for (const auto& item : cells_)
        {
            if (item != nullptr)
            {
                item->set_inherited_binding_context(context);
            }
        }
    }

    void table_section::notify_added(std::size_t index)
    {
        const auto& added = cells_[index];
        // TableSectionBase<T>.OnChildrenChanged: the new cell inherits the section's binding context.
        if (added != nullptr)
        {
            added->set_inherited_binding_context(raw_binding_context());
        }
        collection_changed_args<std::shared_ptr<cell>> args;
        args.action = collection_changed_action::add;
        args.new_items.push_back(added);
        args.new_starting_index = static_cast<std::ptrdiff_t>(index);
        collection_changed.raise(args);
    }
} // namespace maui::controls
