#pragma once
// examples::ViewModels::selection_sync_model — the C++ twin of the original sample's SelectionSyncModel
// (src/Controls/samples/.../SelectionGalleries/SelectionSynchronization.xaml.cs:31-52) and of the
// MauiReference code-behind's copy. The shared XAML binds Items / SelectedItem / SelectedItems /
// SelectedItemNotInSource / SelectedItemsNotInSource, so BOTH frameworks must publish those names.
//
// THE DESCRIPTOR NAMES ARE THE BINDING PATHS AND THEY ARE CASE-SENSITIVE. `{Binding SelectedItems}`
// resolves against a property registered as "SelectedItems"; a lowercase descriptor binds NOTHING and
// does it silently — the load succeeds and the page renders unselected, which is indistinguishable from
// the degraded markup this page was just rescued from. Measured while writing the loader test.
//
// SelectedItem is a boxed_item, not a std::string: C#'s SelectedItem is `object` and the port's
// object-equivalent across the item seam is boxed_item, so the bound value already matches
// selected_item_property()'s type. The C# model can say `string` because MAUI boxes on assignment.

#include <memory>
#include <string>
#include <vector>

#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/item_collection.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/property.hpp"

namespace examples::ViewModels
{
    namespace detail
    {
        inline const maui::core::bindable_property<std::shared_ptr<maui::controls::i_item_collection>>&
        items_property()
        {
            static const maui::core::bindable_property<std::shared_ptr<maui::controls::i_item_collection>> d{"Items"};
            return d;
        }
        inline const maui::core::bindable_property<maui::controls::boxed_item>& selected_item_property()
        {
            static const maui::core::bindable_property<maui::controls::boxed_item> d{"SelectedItem"};
            return d;
        }
        inline const maui::core::bindable_property<std::shared_ptr<maui::controls::i_item_collection>>&
        selected_items_property()
        {
            static const maui::core::bindable_property<std::shared_ptr<maui::controls::i_item_collection>> d{
                "SelectedItems"};
            return d;
        }
        inline const maui::core::bindable_property<maui::controls::boxed_item>& selected_item_not_in_source_property()
        {
            static const maui::core::bindable_property<maui::controls::boxed_item> d{"SelectedItemNotInSource"};
            return d;
        }
        inline const maui::core::bindable_property<std::shared_ptr<maui::controls::i_item_collection>>&
        selected_items_not_in_source_property()
        {
            static const maui::core::bindable_property<std::shared_ptr<maui::controls::i_item_collection>> d{
                "SelectedItemsNotInSource"};
            return d;
        }
    } // namespace detail

    struct selection_sync_model : maui::core::bindable_object
    {
        selection_sync_model()
        {
            Items.set(maui::controls::make_item_collection(
                std::vector<std::string>{"Item 1", "Item 2", "Item 3", "Item 4"}));
            SelectedItem.set(maui::controls::boxed_item::of(std::string{"Item 2"}));
            SelectedItems.set(
                maui::controls::make_item_collection(std::vector<std::string>{"Item 3", "Item 2"}));
            // Deliberately ABSENT from Items — these drive the four "(not in source)" CollectionViews,
            // which must resolve to nothing selected. That is what the page exists to demonstrate.
            SelectedItemNotInSource.set(maui::controls::boxed_item::of(std::string{"Foo"}));
            SelectedItemsNotInSource.set(
                maui::controls::make_item_collection(std::vector<std::string>{"Foo", "Bar", "Baz"}));
        }

        maui::core::property<std::shared_ptr<maui::controls::i_item_collection>> Items{*this, detail::items_property()};
        maui::core::property<maui::controls::boxed_item> SelectedItem{*this, detail::selected_item_property()};
        maui::core::property<std::shared_ptr<maui::controls::i_item_collection>> SelectedItems{
            *this, detail::selected_items_property()};
        maui::core::property<maui::controls::boxed_item> SelectedItemNotInSource{
            *this, detail::selected_item_not_in_source_property()};
        maui::core::property<std::shared_ptr<maui::controls::i_item_collection>> SelectedItemsNotInSource{
            *this, detail::selected_items_not_in_source_property()};
    };
} // namespace examples::ViewModels
