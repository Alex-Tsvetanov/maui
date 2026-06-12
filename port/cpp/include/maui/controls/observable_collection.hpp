#pragma once
// maui::controls::observable_collection<T>  <=  System.Collections.ObjectModel.ObservableCollection<T>
//                                              (+ NotifyCollectionChangedEventArgs / ...Action)
//
// The minimal observable list the picker's ItemsSource contract consumes: a vector with a typed
// collection_changed event mirroring INotifyCollectionChanged. Only the notification shapes MAUI's
// Picker reacts to are modeled (Picker.CollectionChanged switches on Add / Remove / default→Reset):
//   - add / insert / insert_range  -> add     (new_items + new_starting_index)
//   - remove_at / remove_range     -> remove  (old_items + old_starting_index)
//   - clear                        -> reset   (raised even when already empty, like ObservableCollection)
// Replace/Move are not emitted (no mutator produces them here); a consumer treating "anything else"
// as Reset (as Picker does) is unaffected. Header-only template (like the BCL's generic collection).

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include "maui/core/event.hpp"

namespace maui::controls
{
    // <= System.Collections.Specialized.NotifyCollectionChangedAction (the subset emitted here).
    enum class collection_changed_action : std::uint8_t
    {
        add,
        remove,
        reset,
    };

    // <= System.Collections.Specialized.NotifyCollectionChangedEventArgs (typed, no boxing).
    template <class T> struct collection_changed_args
    {
        collection_changed_action action = collection_changed_action::reset;
        std::vector<T> new_items;               // the items added (action == add)
        std::ptrdiff_t new_starting_index = -1; // where they were inserted (-1: unknown/none)
        std::vector<T> old_items;               // the items removed (action == remove)
        std::ptrdiff_t old_starting_index = -1; // where they were removed from (-1: unknown/none)
    };

    template <class T> class observable_collection
    {
    public:
        observable_collection() = default;
        explicit observable_collection(std::vector<T> items) : items_(std::move(items))
        {
        }

        [[nodiscard]] std::size_t count() const
        {
            return items_.size();
        }
        [[nodiscard]] const T& at(std::size_t index) const
        {
            return items_.at(index);
        }
        [[nodiscard]] const std::vector<T>& items() const
        {
            return items_;
        }
        // IndexOf: the first matching element, or -1 (the C# miss value the picker logic leans on).
        [[nodiscard]] std::ptrdiff_t index_of(const T& item) const
        {
            for (std::size_t at = 0; at < items_.size(); ++at)
            {
                if (items_[at] == item)
                {
                    return static_cast<std::ptrdiff_t>(at);
                }
            }
            return -1;
        }

        void add(T item)
        {
            insert(items_.size(), std::move(item));
        }
        void insert(std::size_t index, T item)
        {
            insert_range(index, std::vector<T>{std::move(item)});
        }
        // The ObservableRangeCollection.InsertRange shape the C# Picker tests exercise: ONE add
        // notification carrying the whole range.
        void insert_range(std::size_t index, std::vector<T> items)
        {
            items_.insert(items_.begin() + static_cast<std::ptrdiff_t>(index), items.begin(), items.end());
            collection_changed_args<T> args;
            args.action = collection_changed_action::add;
            args.new_items = std::move(items);
            args.new_starting_index = static_cast<std::ptrdiff_t>(index);
            collection_changed.raise(args);
        }

        void remove_at(std::size_t index)
        {
            remove_range(index, 1);
        }
        // The ObservableRangeCollection.RemoveRange shape: ONE remove notification for the range.
        void remove_range(std::size_t index, std::size_t count)
        {
            const auto first = items_.begin() + static_cast<std::ptrdiff_t>(index);
            const auto last = first + static_cast<std::ptrdiff_t>(count);
            collection_changed_args<T> args;
            args.action = collection_changed_action::remove;
            args.old_items.assign(first, last);
            args.old_starting_index = static_cast<std::ptrdiff_t>(index);
            items_.erase(first, last);
            collection_changed.raise(args);
        }

        // ObservableCollection.Clear raises Reset unconditionally (even on an already-empty list) —
        // the picker's reentrancy tests depend on the re-raise terminating via the no-change path.
        void clear()
        {
            items_.clear();
            collection_changed_args<T> args;
            args.action = collection_changed_action::reset;
            collection_changed.raise(args);
        }

        // INotifyCollectionChanged.CollectionChanged.
        maui::core::event<collection_changed_args<T>> collection_changed;

    private:
        std::vector<T> items_;
    };
} // namespace maui::controls
