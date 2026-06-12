// selection_list — the SelectionList choreography (SelectionList.cs): mutate with the suppression
// flag up, notify the owner with (shadow, live), then update the shadow.

#include "maui/controls/items/selection_list.hpp"

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

#include "maui/controls/items/selectable_items_view.hpp"

namespace maui::controls
{
    selection_list::selection_list(selectable_items_view& owner)
        : owner_(&owner), internal_(std::make_shared<maui::core::observable_collection<boxed_item>>())
    {
    }

    selection_list::selection_list(selectable_items_view& owner,
                                   std::shared_ptr<maui::core::observable_collection<boxed_item>> items)
        : owner_(&owner),
          internal_(items ? std::move(items) : std::make_shared<maui::core::observable_collection<boxed_item>>()),
          shadow_(copy())
    {
        // `items is INotifyCollectionChanged` — the external flavor watches the collection itself.
        subscription_ =
            maui::core::connect_scoped(internal_->collection_changed,
                                       [this](const maui::core::collection_changed_args&) { on_collection_changed(); });
    }

    selection_list::selection_list(selectable_items_view& owner, std::vector<boxed_item> items)
        : owner_(&owner), internal_(std::make_shared<maui::core::observable_collection<boxed_item>>(std::move(items))),
          shadow_(copy())
    {
    }

    void selection_list::add(boxed_item item)
    {
        external_change_ = true;
        internal_->add(item);
        external_change_ = false;

        owner_->selected_items_property_changed(shadow_, internal_->items());
        shadow_.push_back(std::move(item));
    }

    void selection_list::insert(std::size_t index, boxed_item item)
    {
        external_change_ = true;
        internal_->insert(index, item);
        external_change_ = false;

        owner_->selected_items_property_changed(shadow_, internal_->items());
        shadow_.insert(shadow_.begin() + static_cast<std::ptrdiff_t>(index), std::move(item));
    }

    bool selection_list::remove(const boxed_item& item)
    {
        external_change_ = true;
        const bool removed = internal_->remove(item);
        external_change_ = false;

        if (removed)
        {
            owner_->selected_items_property_changed(shadow_, internal_->items());
            for (auto it = shadow_.begin(); it != shadow_.end(); ++it)
            {
                if (*it == item)
                {
                    shadow_.erase(it);
                    break;
                }
            }
        }
        return removed;
    }

    void selection_list::remove_at(std::size_t index)
    {
        external_change_ = true;
        internal_->remove_at(index);
        external_change_ = false;

        owner_->selected_items_property_changed(shadow_, internal_->items());
        shadow_.erase(shadow_.begin() + static_cast<std::ptrdiff_t>(index));
    }

    void selection_list::clear()
    {
        external_change_ = true;
        internal_->clear();
        external_change_ = false;

        owner_->selected_items_property_changed(shadow_, {}); // C# notifies with s_empty
        shadow_.clear();
    }

    bool selection_list::contains(const boxed_item& item) const
    {
        return internal_->index_of(item) >= 0;
    }

    int selection_list::index_of(const boxed_item& item) const
    {
        return internal_->index_of(item);
    }

    std::size_t selection_list::count() const
    {
        return internal_->size();
    }

    const boxed_item& selection_list::at(std::size_t index) const
    {
        return internal_->at(index);
    }

    const std::vector<boxed_item>& selection_list::items() const
    {
        return internal_->items();
    }

    // A change coming from the bound collection itself (not through this face): emit a selection
    // change, then bring the shadow up to date (SelectionList.OnCollectionChanged).
    void selection_list::on_collection_changed()
    {
        if (external_change_)
        {
            return; // initiated by this face — already notified
        }
        owner_->selected_items_property_changed(shadow_, internal_->items());
        shadow_ = copy();
    }

    std::vector<boxed_item> selection_list::copy() const
    {
        return internal_->items();
    }
} // namespace maui::controls
