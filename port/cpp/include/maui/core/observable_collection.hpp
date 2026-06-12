#pragma once
// maui::core::observable_collection<T>  <=  System.Collections.ObjectModel.ObservableCollection<T>
//                                           (+ Microsoft.Maui.Controls.ObservableList<T> range ops)
//
// The change-notifying list the templated-items machinery consumes (MultiPage<T>.ItemsSource in the
// oracle tests is an ObservableCollection<string> / ObservableList<string>). The port folds the two C#
// types into one: the plain ObservableCollection mutations (add / insert / remove / remove_at / move /
// replace / clear) plus the internal ObservableList<T> RANGE operations the MultiPageTests exercise
// (add_range / insert_range / move(from, to, count) / remove_at(index, count) / replace_range).
//
// `collection_changed` carries a collection_changed_args (System.Collections.Specialized.
// NotifyCollectionChangedEventArgs, reduced): the action plus the new/old starting indices and COUNTS.
// The C# args carry the item lists themselves; the port's consumers (multi_page) re-read the live
// collection by index instead, so the args only need the shape of the change — a typed item list would
// otherwise force the event type to be templated away from the type-erased consumer side.
//
// DELIBERATE DEVIATION (documented): C# ObservableList<T>.RemoveAt(index, count) contains an
// off-by-index loop (`for (int i = index; i < count; i++) Items.RemoveAt(i)`) that removes the wrong
// tail items while reporting `count` removed at `index` ("only being used for tests" — the class is
// internal). The port's remove_at(index, count) really removes the [index, index + count) window, which
// is what the reported args describe and what the MultiPageTests assert on the PAGES side.

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <utility>
#include <vector>

#include "maui/core/event.hpp"

namespace maui::core
{
    // System.Collections.Specialized.NotifyCollectionChangedAction.
    enum class collection_changed_action : std::uint8_t
    {
        add = 0,
        remove = 1,
        replace = 2,
        move = 3,
        reset = 4,
    };

    // NotifyCollectionChangedEventArgs, reduced to the change's shape (see the header note). Indices are
    // -1 ("unset", like C#) for actions that do not carry them (reset).
    struct collection_changed_args
    {
        collection_changed_action action = collection_changed_action::reset;
        int new_starting_index = -1; // C# NewStartingIndex
        int old_starting_index = -1; // C# OldStartingIndex
        std::size_t new_count = 0;   // C# NewItems.Count (0 = no new items)
        std::size_t old_count = 0;   // C# OldItems.Count (0 = no old items)
    };

    template <class T> class observable_collection
    {
    public:
        observable_collection() = default;
        explicit observable_collection(std::vector<T> items) : items_(std::move(items))
        {
        }

        // Raised after every mutation (INotifyCollectionChanged.CollectionChanged).
        event<const collection_changed_args&> collection_changed;

        [[nodiscard]] std::size_t size() const
        {
            return items_.size();
        }
        [[nodiscard]] bool empty() const
        {
            return items_.empty();
        }
        [[nodiscard]] const T& at(std::size_t index) const
        {
            return items_.at(index);
        }
        [[nodiscard]] const std::vector<T>& items() const
        {
            return items_;
        }
        // IndexOf (object equality → T's operator==); -1 when absent.
        [[nodiscard]] int index_of(const T& value) const
        {
            for (std::size_t i = 0; i < items_.size(); ++i)
            {
                if (items_[i] == value)
                {
                    return static_cast<int>(i);
                }
            }
            return -1;
        }

        // ---- ObservableCollection<T> single-item mutations ----
        void add(T value)
        {
            items_.push_back(std::move(value));
            raise({.action = collection_changed_action::add,
                   .new_starting_index = static_cast<int>(items_.size()) - 1,
                   .new_count = 1});
        }

        void insert(std::size_t index, T value)
        {
            items_.insert(items_.begin() + static_cast<std::ptrdiff_t>(index), std::move(value));
            raise({.action = collection_changed_action::add,
                   .new_starting_index = static_cast<int>(index),
                   .new_count = 1});
        }

        // Remove the first occurrence; false when absent (Collection<T>.Remove).
        bool remove(const T& value)
        {
            const int index = index_of(value);
            if (index < 0)
            {
                return false;
            }
            remove_at(static_cast<std::size_t>(index));
            return true;
        }

        void remove_at(std::size_t index)
        {
            items_.erase(items_.begin() + static_cast<std::ptrdiff_t>(index));
            raise({.action = collection_changed_action::remove,
                   .old_starting_index = static_cast<int>(index),
                   .old_count = 1});
        }

        // ObservableCollection<T>.Move(oldIndex, newIndex): remove at old, re-insert at new.
        void move(std::size_t old_index, std::size_t new_index)
        {
            move(old_index, new_index, 1);
        }

        // Replace the item at `index` (the C# indexer set → a Replace notification).
        void set(std::size_t index, T value)
        {
            items_[index] = std::move(value);
            raise({.action = collection_changed_action::replace,
                   .new_starting_index = static_cast<int>(index),
                   .old_starting_index = static_cast<int>(index),
                   .new_count = 1,
                   .old_count = 1});
        }

        void clear()
        {
            items_.clear();
            raise({.action = collection_changed_action::reset});
        }

        // ---- ObservableList<T> range operations (Controls internal; the MultiPageTests' range cases) ----
        void add_range(std::vector<T> values)
        {
            const int index = static_cast<int>(items_.size());
            const std::size_t count = values.size();
            for (T& value : values)
            {
                items_.push_back(std::move(value));
            }
            raise({.action = collection_changed_action::add, .new_starting_index = index, .new_count = count});
        }

        void insert_range(std::size_t index, std::vector<T> values)
        {
            const std::size_t count = values.size();
            items_.insert(items_.begin() + static_cast<std::ptrdiff_t>(index), std::make_move_iterator(values.begin()),
                          std::make_move_iterator(values.end()));
            raise({.action = collection_changed_action::add,
                   .new_starting_index = static_cast<int>(index),
                   .new_count = count});
        }

        // ObservableList<T>.Move(oldIndex, newIndex, count): extract `count` items at old_index, then
        // re-insert at newIndex adjusted when moving forward (index -= count - 1), exactly the C# math.
        void move(std::size_t old_index, std::size_t new_index, std::size_t count)
        {
            std::vector<T> moved;
            moved.reserve(count);
            for (std::size_t i = 0; i < count; ++i)
            {
                moved.push_back(std::move(items_[old_index + i]));
            }
            items_.erase(items_.begin() + static_cast<std::ptrdiff_t>(old_index),
                         items_.begin() + static_cast<std::ptrdiff_t>(old_index + count));

            std::size_t insert_index = new_index;
            if (new_index > old_index)
            {
                insert_index -= count - 1;
            }
            items_.insert(items_.begin() + static_cast<std::ptrdiff_t>(insert_index),
                          std::make_move_iterator(moved.begin()), std::make_move_iterator(moved.end()));

            raise({.action = collection_changed_action::move,
                   .new_starting_index = static_cast<int>(new_index),
                   .old_starting_index = static_cast<int>(old_index),
                   .new_count = count,
                   .old_count = count});
        }

        // ObservableList<T>.RemoveAt(index, count) — really removes the window (see the header DEVIATION).
        void remove_at(std::size_t index, std::size_t count)
        {
            items_.erase(items_.begin() + static_cast<std::ptrdiff_t>(index),
                         items_.begin() + static_cast<std::ptrdiff_t>(index + count));
            raise({.action = collection_changed_action::remove,
                   .old_starting_index = static_cast<int>(index),
                   .old_count = count});
        }

        // ObservableList<T>.ReplaceRange(startIndex, items).
        void replace_range(std::size_t start_index, std::vector<T> values)
        {
            const std::size_t count = values.size();
            for (std::size_t i = 0; i < count; ++i)
            {
                items_[start_index + i] = std::move(values[i]);
            }
            raise({.action = collection_changed_action::replace,
                   .new_starting_index = static_cast<int>(start_index),
                   .old_starting_index = static_cast<int>(start_index),
                   .new_count = count,
                   .old_count = count});
        }

    private:
        void raise(const collection_changed_args& args)
        {
            collection_changed.raise(args);
        }

        std::vector<T> items_;
    };
} // namespace maui::core
