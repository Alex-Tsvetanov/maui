#pragma once
// nested_items.hpp — bindable item data for the gallery_xaml code-behind of nested_collection.xaml.
//
// The shared nested_collection.xaml sets NO outer ItemsSource (the original C# assigns
// BindingContext = a NestedCollectionViewModel of 20 sources in code-behind). The compile-time-XAML
// loader has no code-behind of its own, so the hand-written code-behind (nested_collection.xaml.cpp)
// assigns it — the same reflection-free pattern as photo_items.hpp: the item objects are
// bindable_objects that REGISTER the bound property names via observable<T>{*this, "Name"}, so the
// ItemTemplate's binding context resolves the {Binding …} paths against them.
//
// The nesting: the OUTER CollectionView binds nothing itself; each outer item (nested_source_item) is
// the binding context of an INNER CollectionView whose ItemsSource="{Binding Items}" and
// Header="{Binding Title}", and whose own ItemTemplate binds {Binding Caption}. So Items must be a
// shared_ptr<i_item_collection> (the type items_source_property binds), holding bindable
// nested_caption_items. Mirrors the deterministic data of gallery/pages/nested_collection_page.hpp
// (20 "Source N", each 6..14 image/caption items) so the C++&XAML column matches the C++ builder
// column and original MAUI.

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "maui/controls/items/boxed_item.hpp"      // Title is a boxed_item (Header's object type)
#include "maui/controls/items/item_collection.hpp" // i_item_collection + make_item_collection
#include "maui/core/bindable_object.hpp"
#include "maui/core/observable.hpp"

namespace examples::ViewModels
{
    // Inner cell binding context: {Binding Caption} — the blue 10pt Label of the horizontal inner list.
    class nested_caption_item : public maui::core::bindable_object
    {
    public:
        explicit nested_caption_item(std::string caption)
        {
            Caption.set(std::move(caption));
        }
        maui::core::observable<std::string> Caption{*this, "Caption"};
    };

    // Outer cell binding context: {Binding Title} (the inner CV's red-italic header) and {Binding Items}
    // (the inner CV's ItemsSource — an i_item_collection of nested_caption_items).
    class nested_source_item : public maui::core::bindable_object
    {
    public:
        nested_source_item(std::string title, std::shared_ptr<maui::controls::i_item_collection> items)
        {
            Title.set(maui::controls::boxed_item::of(std::move(title)));
            Items.set(std::move(items));
        }
        // boxed_item (not std::string): CollectionView.Header is a boxed_item property, and the M7 binding
        // applier sets the resolved value straight onto it with no coercion — so {Binding Title} must
        // already yield a boxed_item. The header TEMPLATE's {Binding .} then unwraps it back to the title
        // string for its Label (the same boxed_item→string context unwrap the C++ builder page relies on).
        maui::core::observable<maui::controls::boxed_item> Title{*this, "Title"};
        maui::core::observable<std::shared_ptr<maui::controls::i_item_collection>> Items{*this, "Items"};
    };

    // NestedCollectionViewModel(): 20 "Source 0".."Source 19", each with 6..14 image/caption items drawn
    // deterministically (the same counts + captions as the C++ builder page's build_sources()).
    [[nodiscard]] inline std::vector<std::shared_ptr<nested_source_item>> nested_items()
    {
        static constexpr const char* images[] = {"cover1.jpg", "oasis.jpg",      "photo.jpg",  "Vegetables.jpg",
                                                 "Fruits.jpg", "FlowerBuds.jpg", "Legumes.jpg"};
        std::vector<std::shared_ptr<nested_source_item>> sources;
        sources.reserve(20);
        for (int n = 0; n < 20; ++n)
        {
            const int count = 6 + (n % 9); // deterministic 6..14 by source index
            std::vector<std::shared_ptr<nested_caption_item>> inner;
            inner.reserve(static_cast<std::size_t>(count));
            for (int k = 0; k < count; ++k)
            {
                inner.push_back(
                    std::make_shared<nested_caption_item>(std::string(images[k % 7]) + ", " + std::to_string(k)));
            }
            sources.push_back(std::make_shared<nested_source_item>(
                "Source " + std::to_string(n), maui::controls::make_item_collection(std::move(inner))));
        }
        return sources;
    }
} // namespace examples::ViewModels
