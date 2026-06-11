// maui::controls::behavior_collection — the AttachedCollection<Behavior> mutation/attachment dance
// (behavior.hpp). Ported from AttachedCollection.cs (OnAttachedTo / OnDetachingFrom / InsertItem /
// RemoveItem / ClearItems).
#include "maui/controls/behavior.hpp"

#include <algorithm>
#include <memory>
#include <utility>

#include "maui/core/bindable_object.hpp"

namespace maui::controls
{
    void behavior_collection::attach_to(maui::core::bindable_object& bindable)
    {
        if (std::ranges::find(associated_, &bindable) == associated_.end())
        {
            associated_.push_back(&bindable);
        }
        for (const std::shared_ptr<behavior>& item : items_)
        {
            item->attach_to(bindable);
        }
    }

    void behavior_collection::detach_from(maui::core::bindable_object& bindable)
    {
        // C# order: detach every item first, then drop the association.
        for (const std::shared_ptr<behavior>& item : items_)
        {
            item->detach_from(bindable);
        }
        std::erase(associated_, &bindable);
    }

    void behavior_collection::add(std::shared_ptr<behavior> item)
    {
        if (item == nullptr)
        {
            return;
        }
        // InsertItem: append first, then attach to every associated bindable.
        items_.push_back(std::move(item));
        for (maui::core::bindable_object* bindable : associated_)
        {
            items_.back()->attach_to(*bindable);
        }
    }

    bool behavior_collection::remove(const std::shared_ptr<behavior>& item)
    {
        const auto it = std::ranges::find(items_, item);
        if (it == items_.end())
        {
            return false;
        }
        // RemoveItem: detach from every associated bindable first, then drop it.
        for (maui::core::bindable_object* bindable : associated_)
        {
            (*it)->detach_from(*bindable);
        }
        items_.erase(it);
        return true;
    }

    void behavior_collection::clear()
    {
        // ClearItems: for each associated bindable, detach every item; then clear.
        for (maui::core::bindable_object* bindable : associated_)
        {
            for (const std::shared_ptr<behavior>& item : items_)
            {
                item->detach_from(*bindable);
            }
        }
        items_.clear();
    }
} // namespace maui::controls
