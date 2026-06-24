#pragma once
// maui::samples::footer_only_string_page — ports FooterOnlyString.xaml (+ FooterOnlyString.xaml.cs) of
// the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries).
//
// The original page is the minimal arm of the Header/Footer trio's "string" family: a single
// CollectionView with a plain STRING Footer ("This is a footer") and NO Header at all. The xaml.cs
// seeds the source with DemoFilteredItemSource(20) (twenty CollectionViewGalleryTestItem rows), sets
// ItemTemplate = ExampleTemplates.PhotoTemplate(), and ItemsSource = the demo items. There are no
// handlers — the page exists solely to prove the footer-only path (a structured items view that boxes a
// footer string but leaves Header null).
//
// This port mirrors that shape code-first:
//   - the collection_view's Footer is a boxed string (structured_items_view::set_footer, the
//     boxed_item string-vs-view split the C# StructuredItemsView reproduces) — the view-level chrome
//     that sits below the scroll extent; the Header is deliberately left unset (null), so the headless
//     collection_view handler's update_header_footer realizes a footer supplemental cell but no header;
//   - the item template is the PhotoTemplate caption Label (Text bound to each row's caption — the image
//     half has no headless-safe analog without an asset pipeline, see note);
//   - the source is a live observable_collection<demo_item> of twenty rows, seeded like
//     DemoFilteredItemSource's AddItems loop (image-name + index caption).
//
// The page OWNS its whole element tree (the items_page pattern). The generic mount (app_host.hpp) attaches
// every owned view's handler and hosts the tree; the headless collection_view runs its
// fake-viewport virtualization simulator, which realizes the item cells (binding each realized cell to
// its item, so the caption binding is exercised) plus the footer supplemental.
//
// note: ExampleTemplates.PhotoTemplate() pairs an Image (bound to "Image") above a caption Label (bound
//       to "Caption"). The port item cell is the caption Label only — an Image row would need an
//       i_image_source the headless backend can't resolve; the caption carries the image file name, so
//       the demonstrated text covers both halves' intent. Footer is a plain string exactly as in the
//       oracle XAML; Header stays null (the whole point of "footer only").

#include <memory>
#include <string>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/core/observable_collection.hpp"

namespace maui::samples
{
    class footer_only_string_page
    {
    public:
        // One row of the demo source — the caption the PhotoTemplate binds
        // (CollectionViewGalleryTestItem.Caption, reduced to what this page surfaces). `caption` is the
        // "<image>, <index>" string DemoFilteredItemSource.AddItems mints.
        struct demo_item
        {
            std::string caption;
        };

        footer_only_string_page() : items_(std::make_shared<maui::core::observable_collection<demo_item>>(seed_items()))
        {
            page_.set_title("Footer only (string)");

            // The PhotoTemplate caption Label: Text binds to the item's caption (C# Binding("Caption")).
            auto cell = maui::controls::data_template::of<maui::controls::label>();
            cell->set_binding<std::string, demo_item>(maui::controls::label::text_property(),
                                                      [](const demo_item& item) { return item.caption; });
            list_.set_item_template(cell);
            list_.set_items_source(items_);

            // The one plain-string footer — the whole point of the oracle page. NO Header is set (it
            // stays the null boxed_item), so this is the footer-only path.
            list_.set_footer(maui::controls::boxed_item::of(std::string{"This is a footer"}));

            page_.set_content(list_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment / tests.
        [[nodiscard]] maui::controls::collection_view& list()
        {
            return list_;
        }
        [[nodiscard]] const std::shared_ptr<maui::core::observable_collection<demo_item>>& items() const
        {
            return items_;
        }

    private:
        // DemoFilteredItemSource(20).AddItems: twenty rows, captioned "<image>, <n>" off the image ring.
        [[nodiscard]] static std::vector<demo_item> seed_items()
        {
            static const std::vector<std::string> images{"cover1.jpg", "oasis.jpg",      "photo.jpg",  "Vegetables.jpg",
                                                         "Fruits.jpg", "FlowerBuds.jpg", "Legumes.jpg"};
            std::vector<demo_item> rows;
            for (int n = 0; n < 20; ++n)
            {
                rows.push_back(
                    demo_item{images[static_cast<std::size_t>(n) % images.size()] + ", " + std::to_string(n)});
            }
            return rows;
        }

        std::shared_ptr<maui::core::observable_collection<demo_item>> items_; // publisher before the list (§8)
        maui::controls::content_page page_;
        maui::controls::collection_view list_;
    };
} // namespace maui::samples
