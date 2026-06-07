#pragma once
// maui::core::setter_specificity_list<T>  <=  Microsoft.Maui.Controls.SetterSpecificityList<T> (internal)
//
// Per-property value store: one value of T per setter_specificity, kept sorted by specificity so the
// highest-precedence entry is the effective value. Ported from src/Controls/src/Core/
// SetterSpecificityList.cs (a sorted array + binary search, for cache-friendly lookup). A class
// template, so header-only (PROFILE.md §3).
//
// The C# indexer becomes explicit get()/set() (C++ can't cleanly do get-by-value + insert-on-missing
// through operator[]). cleared_value() answers "what would win if the top (or a given) specificity
// were removed" — used by bindable_object::clear_value. remove() destroys the stored T (RAII; no
// manual nulling needed, unlike the GC'd C# original).

#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>

#include "maui/core/setter_specificity.hpp"

namespace maui::core
{
    template <class T> class setter_specificity_list
    {
    public:
        [[nodiscard]] std::size_t count() const
        {
            return entries_.size();
        }
        [[nodiscard]] bool empty() const
        {
            return entries_.empty();
        }

        // Value stored at exactly this specificity, or a default-constructed T if none.
        [[nodiscard]] T get(setter_specificity key) const
        {
            auto const it = find(key);
            return (it != entries_.end() && it->key == key) ? it->value : T{};
        }

        // Insert the value at this specificity, or overwrite the existing one.
        void set(setter_specificity key, T value)
        {
            auto const it = find(key);
            if (it != entries_.end() && it->key == key)
            {
                it->value = std::move(value);
                return;
            }
            entries_.insert(it, entry{key, std::move(value)});
        }

        void remove(setter_specificity key)
        {
            auto const it = find(key);
            if (it != entries_.end() && it->key == key)
            {
                entries_.erase(it);
            }
        }

        // ---- effective (highest-specificity) accessors ----
        [[nodiscard]] setter_specificity specificity() const
        {
            return entries_.empty() ? setter_specificity{} : entries_.back().key;
        }
        [[nodiscard]] T value() const
        {
            return entries_.empty() ? T{} : entries_.back().value;
        }
        // Reference to the highest-specificity value. Precondition: !empty(). Valid until the next mutation.
        [[nodiscard]] const T &value_ref() const
        {
            return entries_.back().value;
        }
        [[nodiscard]] std::pair<setter_specificity, T> specificity_and_value() const
        {
            return entries_.empty() ? std::pair<setter_specificity, T>{}
                                    : std::pair<setter_specificity, T>{entries_.back().key, entries_.back().value};
        }

        // ---- "what wins if the top is removed" (for clear_value) ----
        [[nodiscard]] T cleared_value() const
        {
            return entries_.size() < 2 ? T{} : entries_[entries_.size() - 2].value;
        }
        [[nodiscard]] setter_specificity cleared_specificity() const
        {
            return entries_.size() < 2 ? setter_specificity{} : entries_[entries_.size() - 2].key;
        }
        // What wins if `key` is removed: the top, unless the top *is* key, then the one below it.
        [[nodiscard]] T cleared_value(setter_specificity key) const
        {
            if (entries_.empty())
            {
                return T{};
            }
            std::size_t index = entries_.size() - 1;
            if (entries_[index].key == key)
            {
                if (index == 0)
                {
                    return T{};
                }
                --index;
            }
            return entries_[index].value;
        }

    private:
        struct entry
        {
            setter_specificity key;
            T value;
        };
        using iterator = std::vector<entry>::iterator;
        using const_iterator = std::vector<entry>::const_iterator;

        [[nodiscard]] iterator find(setter_specificity key)
        {
            return std::lower_bound(entries_.begin(), entries_.end(), key,
                                    [](const entry &e, setter_specificity k) { return e.key < k; });
        }
        [[nodiscard]] const_iterator find(setter_specificity key) const
        {
            return std::lower_bound(entries_.begin(), entries_.end(), key,
                                    [](const entry &e, setter_specificity k) { return e.key < k; });
        }

        std::vector<entry> entries_; // sorted ascending by key
    };
} // namespace maui::core
