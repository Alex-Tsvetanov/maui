#pragma once
// maui::controls::items_source_factory  <=  Microsoft.Maui.Controls.Handlers.Items.ItemsSourceFactory
//
// Mints the handler-side i_items_view_source for a control's erased ItemsSource:
//   - null            -> the empty source (EmptySource);
//   - live (changed() non-null) -> the observable source (ObservableItemsSource);
//   - snapshot        -> the list source (ListSource).
// create_grouped mirrors CreateGrouped: null -> empty, anything else -> the observable grouped
// source (ObservableGroupedSource wraps snapshot group lists too — its group-level subscription is
// simply absent then). The carousel loop flavor is out of scope until the carousel lands.
//
// The concrete sources live in items_source_factory.cpp (internal, like the C# classes); tests reach
// them through i_items_view_source.

#include <memory>

#include "maui/controls/items/item_collection.hpp"
#include "maui/controls/items/items_view_source.hpp"

namespace maui::controls
{
    class items_source_factory
    {
    public:
        items_source_factory() = delete;

        [[nodiscard]] static std::shared_ptr<i_items_view_source> create(std::shared_ptr<i_item_collection> source);
        [[nodiscard]] static std::shared_ptr<i_items_view_source> create_grouped(
            std::shared_ptr<i_item_collection> source);
    };
} // namespace maui::controls
