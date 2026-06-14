#pragma once
// maui::controls::effect_collection  <=  the Element.Effects TrackableCollection<Effect> + its
//                                        CollectionChanged / Clearing wiring (Element.cs)
//
// The owning list behind element.effects(). C# uses an observable TrackableCollection the element listens
// to (EffectsOnCollectionChanged → AttachEffect on Add / ClearEffect on Remove; EffectsOnClearing →
// ClearEffect each before the list empties). The port folds that wiring into the collection: it owns the
// effects (shared_ptr — the element tree's owning edge, §8) and calls back into the element on each
// mutation via two hooks the element installs:
//   - on_attach(effect&)  — AttachEffect (register with the provider + send_attached)
//   - on_clear(effect&)   — ClearEffect  (send_detached + drop the element back-ref)
// add()/insert() attach the new effect AFTER it is in the list (so EffectIsAttached/walks see it, like
// C#'s post-insert CollectionChanged); remove()/clear() clear BEFORE erasing (C#'s Clearing pre-event and
// the Remove notification carrying the removed item). Re-adding the same effect is refused (it would
// double-attach — AttachEffect throws on an already-attached effect in C#).

#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

namespace maui::controls
{
    class effect; // forward — the owned element (effect.hpp)

    class effect_collection
    {
    public:
        using effect_hook = std::function<void(effect&)>;

        // The element installs both hooks at construction (it owns this collection).
        effect_collection(effect_hook on_attach, effect_hook on_clear)
            : on_attach_(std::move(on_attach)), on_clear_(std::move(on_clear))
        {
        }

        [[nodiscard]] std::size_t count() const
        {
            return items_.size();
        }
        [[nodiscard]] bool empty() const
        {
            return items_.empty();
        }
        [[nodiscard]] const std::shared_ptr<effect>& at(std::size_t index) const
        {
            return items_.at(index);
        }
        [[nodiscard]] const std::vector<std::shared_ptr<effect>>& items() const
        {
            return items_;
        }
        [[nodiscard]] bool contains(const effect* target) const
        {
            return std::ranges::any_of(items_, [target](const auto& held) { return held.get() == target; });
        }

        // ICollection<Effect>.Add: append, then attach (AttachEffect via the element hook). A null or
        // already-present effect is ignored (a re-add would double-attach).
        void add(std::shared_ptr<effect> item)
        {
            insert(items_.size(), std::move(item));
        }

        // IList<Effect>.Insert: place at `index` (clamped to the end), then attach.
        void insert(std::size_t index, std::shared_ptr<effect> item)
        {
            if (!item || contains(item.get()))
            {
                return;
            }
            const std::size_t at = std::min(index, items_.size());
            effect& inserted = *item;
            items_.insert(items_.begin() + static_cast<std::ptrdiff_t>(at), std::move(item));
            on_attach_(inserted);
        }

        // ICollection<Effect>.Remove(item): clear (ClearEffect) THEN erase. Returns whether it was present.
        bool remove(const effect* target)
        {
            const auto it = std::ranges::find_if(items_, [target](const auto& held) { return held.get() == target; });
            if (it == items_.end())
            {
                return false;
            }
            on_clear_(**it);
            items_.erase(it);
            return true;
        }

        // IList<Effect>.RemoveAt.
        void remove_at(std::size_t index)
        {
            if (index >= items_.size())
            {
                return;
            }
            const auto it = items_.begin() + static_cast<std::ptrdiff_t>(index);
            on_clear_(**it);
            items_.erase(it);
        }

        // ICollection<Effect>.Clear: ClearEffect each (the Clearing pre-event), then empty the list.
        void clear()
        {
            for (const auto& held : items_)
            {
                on_clear_(*held);
            }
            items_.clear();
        }

    private:
        std::vector<std::shared_ptr<effect>> items_; // owning (the element's effects)
        effect_hook on_attach_;
        effect_hook on_clear_;
    };
} // namespace maui::controls
