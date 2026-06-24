#pragma once
// maui::samples::collectionview_page — a faithful reproduction of the maui-compare "collectionview" demo
// (ComparePages.CollectionViewPage()), the shipped-.NET-MAUI reference for the visual-parity comparison: a
// CollectionView over 24 captioned items, a string Header ("This is the header"), SelectionMode.Single, a
// GridItemsLayout(3, Vertical) — a three-across grid — and a DataTemplate of a Label (Margin 6) bound to
// each item's Caption. The captions cycle the seven image names ("cover1.jpg, 0", "oasis.jpg, 1", …).
// Kept 1:1 with the C# reference.
//
// The C# items are an Item { Caption } list; the headless-safe port models each item as its caption string
// directly (the {Binding Caption} collapses to a self-binding on the string), exactly as the adaptive /
// items gallery pages do. The page OWNS its whole element tree (the items_page pattern).

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/grid_items_layout.hpp"
#include "maui/controls/items/items_layout_orientation.hpp"
#include "maui/controls/items/selection_mode.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/view.hpp" // margin_property
#include "maui/core/observable_collection.hpp"
#include "maui/core/thickness.hpp"

namespace maui::samples
{
    class collectionview_page
    {
        // The seven image names the C# captions cycle (CollectionViewPage()).
        static std::vector<std::string> build_captions()
        {
            static const std::array<const char*, 7> names{"cover1.jpg", "oasis.jpg",      "Vegetables.jpg",
                                                          "Fruits.jpg", "FlowerBuds.jpg", "Legumes.jpg",
                                                          "photo.jpg"};
            std::vector<std::string> captions;
            captions.reserve(24);
            for (int n = 0; n < 24; ++n)
            {
                captions.emplace_back(std::string(names.at(static_cast<std::size_t>(n) % names.size())) + ", " +
                                      std::to_string(n));
            }
            return captions;
        }

    public:
        collectionview_page()
            : items_(std::make_shared<maui::core::observable_collection<std::string>>(build_captions()))
        {
            page_.set_title("CollectionView");

            // DataTemplate: a Label (Margin 6) bound to the item's Caption (a self-binding on the string).
            auto cell = maui::controls::data_template::of<maui::controls::label>();
            cell->set_binding<std::string, std::string>(maui::controls::label::text_property(),
                                                        [](const std::string& value) { return value; });
            cell->set_value(maui::controls::margin_property(), maui::core::thickness(6));

            list_.set_item_template(cell);
            list_.set_items_source(items_);
            list_.set_items_layout(std::make_shared<maui::controls::grid_items_layout>(
                3, maui::controls::items_layout_orientation::vertical));
            list_.set_header(maui::controls::boxed_item::of(std::string{"This is the header"}));
            list_.set_selection_mode(maui::controls::selection_mode::single);

            page_.set_content(list_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for tests / the hosting main's bottom-up attachment.
        [[nodiscard]] maui::controls::collection_view& list()
        {
            return list_;
        }
        [[nodiscard]] const std::shared_ptr<maui::core::observable_collection<std::string>>& items() const
        {
            return items_;
        }

    private:
        std::shared_ptr<maui::core::observable_collection<std::string>> items_; // publisher before the list (§8)
        maui::controls::content_page page_;
        maui::controls::collection_view list_;
    };
} // namespace maui::samples
