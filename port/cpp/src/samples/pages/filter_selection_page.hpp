#pragma once
// maui::samples::filter_selection_page — ports FilterSelection.xaml (+ .xaml.cs)
//   (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.FilterSelection).
//
// The C# page is a selection-survives-source-swap test. The XAML is a Grid of four rows (Auto/Auto/Auto/*):
//   - row 0: an instructions Label ("Select an item ... Tap Reset to change the ItemsSource. The item should
//            no longer be selected. If an item is still selected, this test has failed.");
//   - row 1: a SearchBar (Placeholder "Filter");
//   - row 2: a "Reset" Button (AutomationId "Reset");
//   - row 3: a CollectionView SelectionMode="Single" with a DataTemplate of a Label bound to Caption.
// The code-behind:
//   - _demoFilteredItemSource = new DemoFilteredItemSource();  CollectionView.ItemsSource = .Items
//   - SearchBar.SearchCommand = new Command(() => _demoFilteredItemSource.FilterItems(SearchBar.Text)) — so
//     pressing the search button filters the live Items collection in place (the same in-place projection
//     DemoFilteredItemSource.FilterItems performs: drop now-excluded, append newly-included);
//   - ResetButton.Clicked → _demoFilteredItemSource = new DemoFilteredItemSource(Random(3..50));
//     CollectionView.ItemsSource = .Items  — REPLACE the whole source with a fresh, differently-sized one.
// The point of the test: after a Reset replaces the ItemsSource, the previously-selected item must NOT remain
// selected (the new source has no such selection).
//
// This headless port owns its whole tree and reproduces both behaviors against the real selectable_items_view:
//   - filter_items(text) is the verbatim DemoFilteredItemSource.FilterItems in-place projection over the live
//     observable_collection bound to the collection_view (mirrors filter_collection_page's filter_items);
//   - the search_bar's search_command drives filter_items(text) on the search button (the C# SearchCommand
//     stand-in — see search_bar.hpp), and text_changed drives it live too so the filter is observable as the
//     user types in the headless sim;
//   - reset() builds a brand-new source (a fresh master + a fresh live observable_collection of a varying
//     3..50 size), points the collection_view at it via set_items_source, and clears the selection — the
//     selection-does-not-survive-source-swap behavior the test asserts. set_selected_item({}) makes the
//     "no longer selected" outcome explicit and observable through the readout.
//   - a readout Label reports the current single selection's Caption (the C# page surfaces selection only
//     through the cell's selected visual, which the headless backend has no analog for — see note).
//
// note: the C# DemoFilteredItemSource holds CollectionViewGalleryTestItem (Date/Caption/Image/Index + More/
//   Less ICommands); the template binds and the filter matches Caption only, so the port models the
//   reflection-free `photo_item { caption }` stand-in (exactly filter_collection_page's). The C# Reset uses a
//   real Random(3,50); the port uses a small deterministic-but-varying counter so the rebuilt source size
//   changes on each Reset without depending on a seeded RNG (the size value is incidental to the test — what
//   matters is that the source is REPLACED and the old selection is gone). The per-cell "selected" visual has
//   no headless analog, so selection is surfaced through the readout instead.

#include <algorithm>
#include <cctype>
#include <memory>
#include <string>
#include <vector>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/selection_changed_event_args.hpp"
#include "maui/controls/items/selection_mode.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/search_bar.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class filter_selection_page
    {
    public:
        // The reflection-free CollectionViewGalleryTestItem stand-in: just the Caption the template binds and
        // the filter matches (boxed_item::of<T> uses operator== for selection equality).
        struct photo_item
        {
            std::string caption;

            friend bool operator==(const photo_item&, const photo_item&) = default;
        };

        filter_selection_page() : items_(std::make_shared<maui::core::observable_collection<photo_item>>())
        {
            page_.set_title("Filter + Selection");

            build_master(50); // DemoFilteredItemSource ctor: AddItems(_source, 50)
            seed_live();      // Items = new ObservableCollection<...>(_source) — start showing all

            // ---- row 0: the instructions Label ----
            instructions_.set_text("Select an item in the CollectionView below. Tap the Reset button to change "
                                   "the ItemsSource. The item should no longer be selected. If an item is "
                                   "still selected, this test has failed.");

            // ---- row 1: the SearchBar — SearchCommand filters the live source in place ----
            search_.set_placeholder("Filter");
            // SearchBar.SearchCommand = new Command(() => FilterItems(SearchBar.Text)).
            search_.search_command = [this]() { filter_items(std::string(search_.text())); };
            // Drive the same projection live as the user types, so the filter is observable in the headless sim.
            search_.text_changed.connect(
                [this](const std::string&, const std::string& new_text) { filter_items(new_text); });

            // ---- row 2: the Reset Button (AutomationId "Reset") ----
            reset_button_.set_text("Reset");
            reset_button_.set_automation_id("Reset");
            reset_button_.clicked.connect([this]() { reset(); });

            // ---- row 3: the CollectionView (SelectionMode Single, Caption-label cell) ----
            auto cell = maui::controls::data_template::of<maui::controls::label>();
            cell->set_binding<std::string, photo_item>(maui::controls::label::text_property(),
                                                       [](const photo_item& item) { return item.caption; });
            list_.set_item_template(cell);
            list_.set_selection_mode(maui::controls::selection_mode::single); // SelectionMode="Single"
            list_.set_items_source(items_);
            list_.selection_changed.connect(
                [this](const maui::controls::selection_changed_event_args& args) { on_selection_changed(args); });

            update_readout(list_.selected_item()); // initial "Selected: (none)"

            // ---- assemble the Grid (Auto / Auto / Auto / *) ----
            grid_.add_row_definition(maui::core::grid_length::automatic());
            grid_.add_row_definition(maui::core::grid_length::automatic());
            grid_.add_row_definition(maui::core::grid_length::automatic());
            grid_.add_row_definition(maui::core::grid_length::star());
            grid_.add(instructions_);
            grid_.set_row(instructions_, 0);
            grid_.add(search_);
            grid_.set_row(search_, 1);
            grid_.add(reset_button_);
            grid_.set_row(reset_button_, 2);
            grid_.add(readout_);
            grid_.set_row(readout_, 2); // the readout shares the Reset row visually; placement is incidental
            grid_.add(list_);
            grid_.set_row(list_, 3);

            page_.set_content(grid_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // ResetButtonClicked: _demoFilteredItemSource = new DemoFilteredItemSource(Random(3,50));
        // CollectionView.ItemsSource = .Items — REPLACE the source with a fresh, differently-sized one. The
        // previously-selected item must not survive the swap, so the selection is explicitly cleared.
        void reset()
        {
            const int count = 3 + (reset_count_ % 48); // a varying 3..50 size (the C# Random(3,50) stand-in)
            ++reset_count_;
            build_master(count);
            items_ = std::make_shared<maui::core::observable_collection<photo_item>>();
            seed_live();
            list_.set_items_source(items_);                        // CollectionView.ItemsSource = new source
            list_.set_selected_item(maui::controls::boxed_item{}); // selection does NOT survive the swap
            update_readout(list_.selected_item());
        }

        // SearchBar.SearchCommand body: FilterItems(text). DemoFilteredItemSource.FilterItems(filter): keep the
        // live order, drop the now-excluded, append the newly-included — the in-place projection that drives
        // the bound collection_view.
        void filter_items(const std::string& filter)
        {
            for (const photo_item& item : master_)
            {
                const bool matches = item_matches(filter, item);
                const bool present = items_->index_of(item) >= 0;
                if (matches && !present)
                {
                    items_->add(item);
                }
                else if (!matches && present)
                {
                    items_->remove(item);
                }
            }
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (leaves first, the page last), then re-host the tree.
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, instructions_, "instructions_");
            gallery_attach_one(app, search_, "search_");
            gallery_attach_one(app, reset_button_, "reset_button_");
            gallery_attach_one(app, readout_, "readout_");
            gallery_attach_one(app, list_, "list_");
            gallery_attach_one(app, grid_, "grid_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(grid_);  // the grid hosts the instructions, search, reset, readout, list
            gallery_rehost_content(page_); // the page hosts the grid
        }

        // ---- accessors (used by the hosting main + any test tree) ----
        [[nodiscard]] maui::controls::grid& grid()
        {
            return grid_;
        }
        [[nodiscard]] maui::controls::search_bar& search()
        {
            return search_;
        }
        [[nodiscard]] maui::controls::button& reset_button()
        {
            return reset_button_;
        }
        [[nodiscard]] maui::controls::collection_view& list()
        {
            return list_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }
        [[nodiscard]] const std::shared_ptr<maui::core::observable_collection<photo_item>>& items() const
        {
            return items_;
        }

    private:
        // DemoFilteredItemSource.AddItems: `count` captioned items cycling the demo image names; the caption
        // shape "{image}, {n}" is what the filter matches and the cell surfaces.
        void build_master(int count)
        {
            static const char* const images[] = {"cover1.jpg", "oasis.jpg",      "photo.jpg",  "Vegetables.jpg",
                                                 "Fruits.jpg", "FlowerBuds.jpg", "Legumes.jpg"};
            constexpr int image_count = static_cast<int>(std::size(images));
            master_.clear();
            master_.reserve(static_cast<std::size_t>(count));
            for (int n = 0; n < count; ++n)
            {
                master_.push_back(photo_item{std::string{images[n % image_count]} + ", " + std::to_string(n)});
            }
        }

        // Items = new ObservableCollection<...>(_source): start showing the full master list.
        void seed_live()
        {
            for (const photo_item& item : master_)
            {
                items_->add(item);
            }
        }

        // DemoFilteredItemSource.ItemMatches: case-insensitive Caption substring (a null/empty filter matches
        // everything, like C#'s Caption.Contains("", OrdinalIgnoreCase)).
        [[nodiscard]] static bool item_matches(const std::string& filter, const photo_item& item)
        {
            if (filter.empty())
            {
                return true;
            }
            const auto lower = [](std::string value) {
                std::transform(value.begin(), value.end(), value.begin(),
                               [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
                return value;
            };
            return lower(item.caption).find(lower(filter)) != std::string::npos;
        }

        void on_selection_changed(const maui::controls::selection_changed_event_args& args)
        {
            update_readout(args.current_selection.empty() ? maui::controls::boxed_item{}
                                                          : args.current_selection.front());
        }

        void update_readout(const maui::controls::boxed_item& selected)
        {
            std::string caption = "(none)";
            if (const auto value = selected.as<photo_item>())
            {
                caption = value->caption;
            }
            readout_.set_text("Selected: " + caption);
        }

        std::vector<photo_item> master_;                                       // DemoFilteredItemSource._source
        std::shared_ptr<maui::core::observable_collection<photo_item>> items_; // the live, bound collection (§8)
        int reset_count_ = 0;
        maui::controls::content_page page_;
        maui::controls::grid grid_;
        maui::controls::label instructions_;
        maui::controls::search_bar search_;
        maui::controls::button reset_button_;
        maui::controls::label readout_;
        maui::controls::collection_view list_;
    };
} // namespace maui::samples
