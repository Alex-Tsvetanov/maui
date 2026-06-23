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
        // The whole surface lives up the hierarchy; the ctor only declares the concrete handler-registry key
        // so the generic mount driver (app_host.hpp) resolves collection_view_handler from a bare element&
        // (collection_view has no implicit style to populate style_target_type_, unlike most controls).
        collection_view()
        {
            this->set_handler_type_tag<collection_view>();
        }
    };
} // namespace maui::controls
