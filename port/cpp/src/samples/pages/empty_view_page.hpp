#pragma once
// maui::samples::empty_view_page — ports EmptyViewStringGallery.xaml (+ EmptyViewStringGallery.xaml.cs).
//
// The C# gallery page is a Grid with two rows: a SearchBar ("Filter") on top and, below, a
// CollectionView whose EmptyView is the plain STRING "No items match your filter." The CollectionView
// has a LinearItemsLayout (Vertical), an ExampleTemplates.PhotoTemplate() item template, and a
// DemoFilteredItemSource() source. Typing in the SearchBar + invoking its SearchCommand filters the
// items down to those whose caption contains the term (DemoFilteredItemSource.FilterItems); when the
// filter matches nothing the source goes empty and the string EmptyView is what the CollectionView
// renders. This port mirrors that shape code-first:
//   - the search_bar drives a filter through its `search_command` (the direct C# SearchBar.SearchCommand
//     stand-in) — filter_items() removes non-matching rows and re-adds matching ones, exactly the
//     DemoFilteredItemSource.FilterItems add/remove reconciliation against the unfiltered backing list;
//   - the collection_view's EmptyView is the boxed string "No items match your filter."
//     (items_view::set_empty_view, the boxed_item string-vs-view split) — shown when the live source has
//     no items, which the headless simulator's update_empty_view drives;
//   - add_items()/clear_items() are the explicit demo buttons the gallery shape calls for: clear empties
//     the live source so the EmptyView appears, add restores the full unfiltered set.
//
// The page OWNS its whole element tree (the items_page pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless
// collection_view runs its fake-viewport virtualization simulator, which realizes the item cells and
// surfaces the EmptyView when the source empties.
//
// note: the C# item template is ExampleTemplates.PhotoTemplate() (an Image over a caption Label). The
//       port template here is the caption Label only — an Image cell would need an i_image_source per
//       row, which the headless backend has no asset pipeline to resolve; the caption carries the image
//       file name, so the demonstrated text covers the intent. The EmptyView is a string exactly as in
//       the oracle XAML. The C# xaml.cs also re-shows all items when the SearchBar text is cleared (the
//       PropertyChanged IsNullOrEmpty branch); filter_items("") here reproduces that (empty filter
//       matches every row).

#include <algorithm>
#include <cctype>
#include <memory>
#include <string>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/search_bar.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class empty_view_page
    {
    public:
        // One row of the demo source — the caption the PhotoTemplate binds + the filter matches against
        // (CollectionViewGalleryTestItem.Caption, reduced to what the page surfaces).
        struct demo_item
        {
            std::string caption;
        };

        empty_view_page() : items_(std::make_shared<maui::core::observable_collection<demo_item>>(source_items()))
        {
            page_.set_title("EmptyView (string)");

            // Grid: an Auto row (the SearchBar) over a Star row (the CollectionView) — the oracle layout.
            grid_.add_row_definition(maui::core::grid_length::automatic());
            grid_.add_row_definition(maui::core::grid_length::star());

            search_.set_placeholder("Filter");
            // SearchBar.SearchCommand = filter the source by the current text (the C# xaml.cs ctor wiring).
            search_.search_command = [this] { filter_items(std::string(search_.text())); };
            // The C# PropertyChanged branch: clearing the text re-shows every row. text_changed gives the
            // (old, new) pair; an emptied box re-runs the all-pass filter.
            search_.text_changed.connect([this](const std::string& /*old*/, const std::string& current) {
                if (current.empty())
                {
                    filter_items("");
                }
            });

            // The PhotoTemplate caption Label: Text binds to the item's caption (C# Binding("Caption")).
            auto cell = maui::controls::data_template::of<maui::controls::label>();
            cell->set_binding<std::string, demo_item>(maui::controls::label::text_property(),
                                                      [](const demo_item& item) { return item.caption; });
            list_.set_item_template(cell);
            list_.set_items_source(items_);

            // The whole point of the oracle page: a plain STRING EmptyView, shown when the source empties.
            list_.set_empty_view(maui::controls::boxed_item::of(std::string{"No items match your filter."}));

            grid_.set_row(search_, 0);
            grid_.add(search_);
            grid_.set_row(list_, 1);
            grid_.add(list_);
            page_.set_content(grid_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (leaves first, the page last), then re-host the
        // tree built in the ctor (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, search_, "search_");
            gallery_attach_one(app, list_, "list_");
            gallery_attach_one(app, grid_, "grid_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(grid_);  // the grid hosts the search bar + collection_view
            gallery_rehost_content(page_); // the page hosts the grid
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment / tests.
        [[nodiscard]] maui::controls::search_bar& search()
        {
            return search_;
        }
        [[nodiscard]] maui::controls::collection_view& list()
        {
            return list_;
        }
        [[nodiscard]] const std::shared_ptr<maui::core::observable_collection<demo_item>>& items() const
        {
            return items_;
        }

        // DemoFilteredItemSource.FilterItems: reconcile the LIVE source against the unfiltered backing
        // list — remove rows that no longer match, re-add rows that match and are missing. An empty
        // filter matches every row (the C# ItemMatches `filter ?? ""` Contains semantics).
        void filter_items(const std::string& filter)
        {
            for (const demo_item& item : source_)
            {
                const bool matches = caption_contains(item.caption, filter);
                const int live_index = index_in_live(item.caption);
                if (matches && live_index < 0)
                {
                    items_->add(item);
                }
                else if (!matches && live_index >= 0)
                {
                    items_->remove_at(static_cast<std::size_t>(live_index));
                }
            }
        }

        // The explicit demo controls the gallery shape calls for (clear → EmptyView appears).
        void clear_items()
        {
            items_->clear();
        }
        void add_items()
        {
            filter_items(""); // restore the full unfiltered set
        }

    private:
        // DemoFilteredItemSource().AddItems(50): the default 50-row set, captioned "<image>, <n>".
        [[nodiscard]] static std::vector<demo_item> source_items()
        {
            static const std::vector<std::string> images{"cover1.jpg", "oasis.jpg",      "photo.jpg",  "Vegetables.jpg",
                                                         "Fruits.jpg", "FlowerBuds.jpg", "Legumes.jpg"};
            std::vector<demo_item> rows;
            for (int n = 0; n < 50; ++n)
            {
                rows.push_back(
                    demo_item{images[static_cast<std::size_t>(n) % images.size()] + ", " + std::to_string(n)});
            }
            return rows;
        }

        // Case-insensitive substring (C# string.Contains(filter, OrdinalIgnoreCase)); "" matches all.
        [[nodiscard]] static bool caption_contains(const std::string& caption, const std::string& filter)
        {
            if (filter.empty())
            {
                return true;
            }
            const auto lower = [](std::string text) {
                std::transform(text.begin(), text.end(), text.begin(),
                               [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
                return text;
            };
            return lower(caption).find(lower(filter)) != std::string::npos;
        }

        // Index of the row with this caption in the LIVE source (captions are unique here), or -1.
        [[nodiscard]] int index_in_live(const std::string& caption) const
        {
            const auto& live = items_->items();
            for (std::size_t i = 0; i < live.size(); ++i)
            {
                if (live[i].caption == caption)
                {
                    return static_cast<int>(i);
                }
            }
            return -1;
        }

        std::vector<demo_item> source_ = source_items();                      // the unfiltered backing list
        std::shared_ptr<maui::core::observable_collection<demo_item>> items_; // publisher before the list (§8)
        maui::controls::content_page page_;
        maui::controls::grid grid_;
        maui::controls::search_bar search_;
        maui::controls::collection_view list_;
    };
} // namespace maui::samples
