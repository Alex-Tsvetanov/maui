#pragma once
// maui::controls::collection_view  <=  Microsoft.Maui.Controls.CollectionView
// The concrete templated-collection control — an empty shell over reorderable_items_view, exactly
// like the C# class (the whole surface lives up the hierarchy).

#include "maui/controls/items/reorderable_items_view.hpp"

namespace maui::controls
{
    class collection_view : public reorderable_items_view
    {
    public:
        collection_view() = default;
    };
} // namespace maui::controls
