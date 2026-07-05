#pragma once
// maui::samples::filter_collection_page — ports FilterCollectionView.xaml (+ .xaml.cs) of the C#
// CollectionView gallery.
//
// The original page (FilterCollectionView): a Grid of three rows — a horizontal StackLayout with a
// "Use EmptyView" Label + Switch, a SearchBar, and a CollectionView laid out as a 2-span vertical
// grid. The CollectionView binds a DemoFilteredItemSource: a master List<CollectionViewGalleryTestItem>
// and a live ObservableCollection<...> Items. SearchBar.TextChanged → FilterItems(text): the source
// re-projects the master list by case-insensitive Caption substring, ADDING newly-matching items and
// REMOVING newly-excluded ones from the live Items collection (so the bound CollectionView updates in
// place). The Switch toggles CollectionView.EmptyView between a "Nothing to see here" coral Label and
// null.
//
// This port mirrors that shape code-first with the headless-safe maui:: API (the items_page pattern):
//   - photo_item: the reflection-free stand-in for CollectionViewGalleryTestItem — it carries the
//     Caption the template binds and the filter matches (we drop Date/Image/Index and the More/Less
//     ICommands, which the page never surfaces);
//   - master_: the fixed master list (DemoFilteredItemSource._source); items_: the live
//     observable_collection bound to the collection_view (DemoFilteredItemSource.Items);
//   - filter_items(text) reproduces DemoFilteredItemSource.FilterItems verbatim: keep the live order,
//     remove the now-excluded, append the newly-included;
//   - the search_bar's text_changed drives filter_items live (SearchBarOnTextChanged);
//   - the toggle_switch's `toggled` swaps the EmptyView coral label in/out (UseEmptyViewOnToggled →
//     UpdateEmptyView). The headless collection_view's virtualization sim realizes the empty view when
//     the live collection is empty, so typing a non-matching filter shows the coral label.
//
// The page OWNS its whole tree (grid + the two header controls + the live collection + the cell and
// empty-view templates). The generic mount attaches every owned view's handler and hosts the tree.

#include <algorithm>
#include <cctype>
#include <memory>
#include <string>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/grid_items_layout.hpp"
#include "maui/controls/items/items_layout_orientation.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/search_bar.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/toggle_switch.hpp"
#include "maui/controls/view.hpp" // margin_property
#include "maui/core/grid_length.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"

namespace maui::samples
{
    class filter_collection_page
    {
    public:
        // The reflection-free CollectionViewGalleryTestItem stand-in: just the Caption the template
        // binds and the filter matches.
        struct photo_item
        {
            std::string caption;

            // Needed so item_collection<photo_item> can build a boxed_item (boxed_item::of copies a
            // value type and uses operator== for selection equality).
            friend bool operator==(const photo_item&, const photo_item&) = default;
        };

        filter_collection_page() : items_(std::make_shared<maui::core::observable_collection<photo_item>>())
        {
            page_.set_title("Filter CollectionView Items");

            build_master(); // DemoFilteredItemSource ctor: AddItems(_source, count)
            seed_live();    // Items = new ObservableCollection<...>(_source) — start showing all

            // ---- row 0: the "Use EmptyView" Label + Switch (horizontal StackLayout) ----
            use_empty_view_.set_is_toggled(true); // XAML IsToggled="True"
            empty_view_caption_.set_text("Use EmptyView");
            header_stack_.set_spacing(8);
            header_stack_.add(empty_view_caption_);
            header_stack_.add(use_empty_view_);
            use_empty_view_.toggled.connect([this](bool) { update_empty_view(); });

            // ---- row 1: the SearchBar ----
            search_.set_placeholder("Filter");
            search_.text_changed.connect(
                [this](const std::string&, const std::string& new_text) { filter_items(new_text); });

            // ---- row 2: the CollectionView (2-span vertical grid of caption labels, Margin 6 — the
            //      shared twin's cell shape) ----
            auto cell = maui::controls::data_template::of<maui::controls::label>();
            cell->set_binding<std::string, photo_item>(maui::controls::label::text_property(),
                                                       [](const photo_item& item) { return item.caption; });
            cell->set_value(maui::controls::margin_property(), maui::core::thickness(6));
            list_.set_item_template(cell);
            list_.set_items_layout(std::make_shared<maui::controls::grid_items_layout>(
                2, maui::controls::items_layout_orientation::vertical)); // GridItemsLayout Span="2"
            list_.set_items_source(items_);
            update_empty_view(); // UpdateEmptyView() with the switch on

            // ---- assemble the Grid (Auto / Auto / *) — Padding 12 / RowSpacing 6: the shared twin's
            //      root-grid shape ----
            grid_.set_padding(maui::core::thickness(12));
            grid_.set_row_spacing(6);
            grid_.add_row_definition(maui::core::grid_length::automatic());
            grid_.add_row_definition(maui::core::grid_length::automatic());
            grid_.add_row_definition(maui::core::grid_length::star());
            grid_.add(header_stack_);
            grid_.set_row(header_stack_, 0);
            grid_.add(search_);
            grid_.set_row(search_, 1);
            grid_.add(list_);
            grid_.set_row(list_, 2);

            page_.set_content(grid_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
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
        [[nodiscard]] maui::controls::toggle_switch& use_empty_view()
        {
            return use_empty_view_;
        }
        [[nodiscard]] maui::controls::collection_view& list()
        {
            return list_;
        }
        [[nodiscard]] const std::shared_ptr<maui::core::observable_collection<photo_item>>& items() const
        {
            return items_;
        }

        // DemoFilteredItemSource.FilterItems(filter): keep the live order, drop the now-excluded, append
        // the newly-included — exactly the C# in-place projection that drives the bound collection_view.
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

    private:
        // DemoFilteredItemSource.AddItems: 50 captioned items cycling the demo image names (we keep the
        // "<image>, <n>" caption shape the filter and template surface; the images themselves are unused
        // here).
        void build_master()
        {
            static const char* const images[] = {"cover1.jpg", "oasis.jpg",      "photo.jpg",  "Vegetables.jpg",
                                                 "Fruits.jpg", "FlowerBuds.jpg", "Legumes.jpg"};
            constexpr int count = 12; // the shared twin's static x:Array count
            constexpr int image_count = static_cast<int>(std::size(images));
            master_.reserve(count);
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

        // DemoFilteredItemSource.ItemMatches: case-insensitive Caption substring (a null/empty filter
        // matches everything, like C#'s string.Contains("")).
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

        // UpdateEmptyView: the switch on → a centered coral "Nothing to see here" label; off → no empty
        // view (boxed_item{} is the null EmptyView).
        void update_empty_view()
        {
            if (use_empty_view_.is_toggled())
            {
                auto label = std::make_shared<maui::controls::label>();
                label->set_text("Nothing to see here");
                label->set_text_color(maui::graphics::colors::coral);
                label->set_horizontal_text_alignment(maui::core::text_alignment::center);
                label->set_vertical_text_alignment(maui::core::text_alignment::center);
                list_.set_empty_view(maui::controls::boxed_item::of(label));
            }
            else
            {
                list_.set_empty_view(maui::controls::boxed_item{});
            }
        }

        std::vector<photo_item> master_;                                       // DemoFilteredItemSource._source
        std::shared_ptr<maui::core::observable_collection<photo_item>> items_; // the live, bound collection
        maui::controls::content_page page_;
        maui::controls::grid grid_;
        maui::controls::horizontal_stack_layout header_stack_;
        maui::controls::label empty_view_caption_;
        maui::controls::toggle_switch use_empty_view_;
        maui::controls::search_bar search_;
        maui::controls::collection_view list_;
    };
} // namespace maui::samples
