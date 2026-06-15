#pragma once
// maui::controls::list_proxy  <=  Microsoft.Maui.Controls.ListProxy (IListProxy / IReadOnlyList<object>)
//
// The read-only list view a search_handler exposes to the shell chrome over its ItemsSource (the search
// results list). Ported from src/Controls/src/Core/ListProxy.cs + IListProxy.cs, exposing the same
// observable contract: count(), index_of(item), contains(item), the indexer, try_get_value(index), and a
// collection_changed event the chrome subscribes to so it re-reads the rows.
//
// MODELING DEVIATION (documented, not stubbed). The C# ListProxy is a lazy WINDOWING proxy over an
// arbitrary IEnumerable: it pulls items through a moving window via an enumerator, supports out-of-order
// access, and forwards INotifyCollectionChanged from the source. That virtualization machinery is a
// .NET-specific optimization for unbounded/lazy sequences. The port's ItemsSource is a MATERIALIZED
// vector of shared_ptr<bindable_object> (every C# `object` here is a binding-context node — the same
// representation the data_template loader produces), so the proxy is a thin read-only view over that
// vector. The window-size / enumerator / out-of-band MoveNext path collapses away; Count/IndexOf/Contains
// /indexer are O(n) over the vector. collection_changed is raised by the OWNER (search_handler) when the
// backing vector is replaced (the C# Reset notification on ItemsSource change) — the per-source
// INotifyCollectionChanged subscription is out of scope (no observable-collection subsystem in the port).
//
// Items are shared_ptr<bindable_object> (C#'s `object`): the search results are the developer's view-model
// rows. index_of/contains compare by pointer identity (C#'s Equals defaults to reference equality for the
// object rows the chrome round-trips — selecting a row hands the SAME instance back).

#include <cstddef>
#include <memory>
#include <vector>

#include "maui/core/bindable_object.hpp"
#include "maui/core/event.hpp"

namespace maui::controls
{
    class list_proxy
    {
    public:
        using item = std::shared_ptr<maui::core::bindable_object>;

        list_proxy() = default;
        explicit list_proxy(std::vector<item> items) : items_(std::move(items))
        {
        }

        // ListProxy.Count.
        [[nodiscard]] std::size_t count() const
        {
            return items_.size();
        }

        // ListProxy[index] — throws std::out_of_range for an out-of-range index (C#'s
        // ArgumentOutOfRangeException from the indexer's TryGetValue-miss).
        [[nodiscard]] const item& operator[](std::size_t index) const
        {
            return items_.at(index);
        }

        // IListProxy.TryGetValue(index, out value): false (and leaves `value` null) when out of range.
        [[nodiscard]] bool try_get_value(std::size_t index, item& value) const
        {
            if (index >= items_.size())
            {
                value = nullptr;
                return false;
            }
            value = items_[index];
            return true;
        }

        // ListProxy.IndexOf — pointer-identity match (C# Equals reference default for the proxied rows);
        // returns -1 when absent.
        [[nodiscard]] int index_of(const maui::core::bindable_object* target) const
        {
            for (std::size_t i = 0; i < items_.size(); ++i)
            {
                if (items_[i].get() == target)
                {
                    return static_cast<int>(i);
                }
            }
            return -1;
        }

        // ListProxy.Contains.
        [[nodiscard]] bool contains(const maui::core::bindable_object* target) const
        {
            return index_of(target) >= 0;
        }

        [[nodiscard]] const std::vector<item>& items() const
        {
            return items_;
        }

        // IListProxy.CollectionChanged — raised by the owner on a Reset (the backing vector was replaced).
        maui::core::event<> collection_changed;

    private:
        std::vector<item> items_;
    };
} // namespace maui::controls
