#pragma once
// maui::samples::header_footer_page — ports HeaderFooterString.xaml (+ HeaderFooterString.xaml.cs).
//
// The C# gallery page is a single CollectionView with a plain STRING Header and a plain STRING Footer
// ("Just a string as a header" / "This footer is also a string"), an ExampleTemplates.PhotoTemplate()
// item template, and a DemoFilteredItemSource(3) source (three CollectionViewGalleryTestItem rows).
// This port mirrors that shape code-first:
//   - the collection_view's Header/Footer are boxed strings (structured_items_view::set_header/footer,
//     the boxed_item string-vs-view split the C# StructuredItemsView reproduces) — the view-level
//     chrome that sits outside the scroll extent;
//   - the item template is a label whose Text binds to each item's caption (the PhotoTemplate's caption
//     Label; the image half has no headless-safe analog without an image source, see note);
//   - the source is a live observable_collection<demo_item> of three rows, seeded like
//     DemoFilteredItemSource's AddItems loop (image-name + index caption).
//
// The page OWNS its whole element tree (the items_page pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless
// collection_view runs its fake-viewport virtualization simulator, which realizes the item cells (and
// the simulator binds each realized cell's content to its item, so the caption binding is exercised).
//
// note: ExampleTemplates.PhotoTemplate() pairs an Image (bound to "Image") above a Label (bound to
//       "Caption"). The port template here is the caption Label only — an Image cell would need an
//       i_image_source per row, which the headless backend has no file/asset pipeline to resolve; the
//       caption already carries the image file name, so the demonstrated text covers both halves'
//       intent. Header/Footer are strings exactly as in the oracle XAML.

#include <memory>
#include <string>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class header_footer_page
    {
    public:
        // One row of the demo source — the fields HeaderFooterString actually surfaces through the
        // PhotoTemplate caption (CollectionViewGalleryTestItem.Caption, reduced to what the template
        // binds). `caption` is the "<image>, <index>" string DemoFilteredItemSource.AddItems mints.
        struct demo_item
        {
            std::string caption;
        };

        header_footer_page() : items_(std::make_shared<maui::core::observable_collection<demo_item>>(seed_items()))
        {
            page_.set_title("Header/Footer (string)");

            // The PhotoTemplate caption Label: Text binds to the item's caption (C# Binding("Caption")).
            auto cell = maui::controls::data_template::of<maui::controls::label>();
            cell->set_binding<std::string, demo_item>(maui::controls::label::text_property(),
                                                      [](const demo_item& item) { return item.caption; });
            list_.set_item_template(cell);
            list_.set_items_source(items_);

            // The two plain-string chrome payloads — the whole point of the oracle page.
            list_.set_header(maui::controls::boxed_item::of(std::string{"Just a string as a header"}));
            list_.set_footer(maui::controls::boxed_item::of(std::string{"This footer is also a string"}));

            page_.set_content(list_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (the list first, the page last), then re-host
        // the tree built in the ctor (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, list_, "list_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_content(page_); // the page hosts the collection_view
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
        // DemoFilteredItemSource(3).AddItems: three rows, captioned "<image>, <n>" off the image ring.
        [[nodiscard]] static std::vector<demo_item> seed_items()
        {
            static const std::vector<std::string> images{"cover1.jpg", "oasis.jpg", "photo.jpg"};
            std::vector<demo_item> rows;
            for (int n = 0; n < 3; ++n)
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
