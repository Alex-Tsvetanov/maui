#pragma once
// maui::samples::empty_view_swap_page — ports EmptyViewGalleries/EmptyViewSwapGallery.xaml (+ .xaml.cs).
//
// The original page (EmptyViewSwapGallery): a Grid of three rows —
//   - a SearchBar ("Filter") that filters the bound source (SearchBar.TextChanged → FilterItems);
//   - a horizontal StackLayout with a "Toggle Between EmptyViews" Label + a Switch (EmptyViewSwitch);
//   - a CollectionView with a GridItemsLayout (Span="3", Vertical), an ExampleTemplates.PhotoTemplate()
//     item template, and a DemoFilteredItemSource().Items source.
// Two ContentViews live in Resources as alternative EmptyViews:
//   - EmptyView1: a StackLayout with a bold centered "No results matched your filter." over an italic
//     "Maybe try a broader filter?";
//   - EmptyView2: a StackLayout with a bold centered "Nothing to see here." over an Image (coffee.png).
// SwitchEmptyView() sets CollectionView.EmptyView = IsToggled ? EmptyView1 : EmptyView2 — so the switch
// chooses WHICH empty view shows once the source goes empty. The ctor calls SwitchEmptyView() once
// (switch off → EmptyView2 selected initially).
//
// This port mirrors that shape code-first, reusing the filter_collection_page blessed patterns (the
// DemoFilteredItemSource projection + a view EmptyView boxed via boxed_item::of(shared_ptr<view>)):
//   - photo_item: the reflection-free CollectionViewGalleryTestItem stand-in (just the Caption the
//     template binds + the filter matches);
//   - master_ / items_: the fixed master list + the live observable_collection bound to the list, seeded
//     with the full set (DemoFilteredItemSource._source / .Items);
//   - filter_items(text) reproduces DemoFilteredItemSource.FilterItems in place (keep live order, drop the
//     excluded, append the newly-included); the search_bar's text_changed drives it live;
//   - the CollectionView is a 3-span vertical grid (GridItemsLayout Span="3") of caption labels;
//   - the two alternative EmptyViews are OWNED views (each a vertical_stack_layout of labels), and the
//     toggle_switch's `toggled` swaps which one is the collection_view's EmptyView (SwitchEmptyView →
//     CollectionView.EmptyView = ...). The headless collection_view sim realizes the active EmptyView
//     when the live source is empty.
//
// clear/fill toggle (the demo affordance the task calls for, atop the oracle's SearchBar filter): Clear
// empties the live source so the active EmptyView appears; Fill restores the full unfiltered master set.
// (The oracle reaches the empty state only by typing a non-matching filter; the buttons make the swap
// directly observable.)
//
// The page OWNS its whole element tree (the items_page / filter_collection_page pattern). It is
// backend-agnostic — a sample main attaches handlers bottom-up via the hosting layer and hosts page() in
// a window; the headless collection_view virtualization sim realizes the cells and surfaces the selected
// EmptyView when the source empties.
//
// note: the C# EmptyView2 ends with an Image (coffee.png). The headless backend has no asset pipeline to
//       resolve an image source for an EmptyView (the empty_view_page precedent), so EmptyView2 here is the
//       "Nothing to see here." label only — the demonstrated swap (two distinct text EmptyViews) is intact.
//       Not invented — left as a documented gap.
// note: the C# item template is ExampleTemplates.PhotoTemplate() (Image over caption Label); the port cell
//       is the caption Label only (the caption carries the image file name), exactly the empty_view_page /
//       filter_collection_page convention (no per-row image source pipeline headless).

#include <algorithm>
#include <cctype>
#include <memory>
#include <string>
#include <vector>

#include "maui/controls/button.hpp"
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
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/controls/view.hpp" // margin_property
#include "maui/core/grid_length.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/thickness.hpp"

namespace maui::samples
{
    class empty_view_swap_page
    {
    public:
        // The reflection-free CollectionViewGalleryTestItem stand-in: just the Caption the template binds
        // and the filter matches (filter_collection_page::photo_item).
        struct photo_item
        {
            std::string caption;
            friend bool operator==(const photo_item&, const photo_item&) = default;
        };

        empty_view_swap_page() : items_(std::make_shared<maui::core::observable_collection<photo_item>>())
        {
            page_.set_title("EmptyView Swap");

            build_master(); // DemoFilteredItemSource ctor: AddItems(_source, 50)
            seed_live();    // Items = new ObservableCollection<...>(_source) — start showing all

            build_empty_views(); // the two alternative EmptyView ContentViews (Resources)

            // ---- row 0: the SearchBar ("Filter") ----
            search_.set_placeholder("Filter");
            search_.text_changed.connect(
                [this](const std::string&, const std::string& new_text) { filter_items(new_text); });

            // ---- row 1: the "Toggle Between EmptyViews" Label + Switch (horizontal StackLayout) ----
            toggle_caption_.set_text("Toggle Between EmptyViews");
            // VerticalTextAlignment="Center" (shared XAML): center the caption within the HSL, whose
            // height is driven by the taller Switch — otherwise the label top-aligns ~40px above MAUI.
            toggle_caption_.set_vertical_text_alignment(maui::core::text_alignment::center);
            toggle_row_.set_spacing(8);
            toggle_row_.add(toggle_caption_);
            toggle_row_.add(empty_view_switch_);
            empty_view_switch_.toggled.connect([this](bool) { switch_empty_view(); });

            // ---- row 2: the CollectionView (3-span vertical grid of caption labels, Margin 6 — the
            //      shared twin's cell shape) ----
            auto cell = maui::controls::data_template::of<maui::controls::label>();
            cell->set_binding<std::string, photo_item>(maui::controls::label::text_property(),
                                                       [](const photo_item& item) { return item.caption; });
            cell->set_value(maui::controls::margin_property(), maui::core::thickness(6));
            list_.set_item_template(cell);
            list_.set_items_layout(std::make_shared<maui::controls::grid_items_layout>(
                3, maui::controls::items_layout_orientation::vertical)); // GridItemsLayout Span="3"
            list_.set_items_source(items_);

            switch_empty_view(); // SwitchEmptyView() in the ctor: switch off → EmptyView2 selected

            // ---- the clear/fill toggle buttons (the demo affordance — see header note) ----
            clear_button_.set_text("Clear");
            clear_button_.command = [this] { clear_items(); };
            fill_button_.set_text("Fill");
            fill_button_.command = [this] { fill_items(); };
            buttons_.set_spacing(12);
            buttons_.add(clear_button_);
            buttons_.add(fill_button_);

            // ---- assemble the Grid (Auto / Auto / Auto / *) — the oracle's three rows plus a button row.
            //      Padding 12 / RowSpacing 6: the shared twin's root-grid shape. ----
            grid_.set_padding(maui::core::thickness(12));
            grid_.set_row_spacing(6);
            grid_.add_row_definition(maui::core::grid_length::automatic());
            grid_.add_row_definition(maui::core::grid_length::automatic());
            grid_.add_row_definition(maui::core::grid_length::automatic());
            grid_.add_row_definition(maui::core::grid_length::star());

            grid_.set_row(search_, 0);
            grid_.add(search_);
            grid_.set_row(toggle_row_, 1);
            grid_.add(toggle_row_);
            grid_.set_row(buttons_, 2);
            grid_.add(buttons_);
            grid_.set_row(list_, 3);
            grid_.add(list_);

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
        [[nodiscard]] maui::controls::toggle_switch& empty_view_switch()
        {
            return empty_view_switch_;
        }
        [[nodiscard]] maui::controls::collection_view& list()
        {
            return list_;
        }
        [[nodiscard]] maui::controls::button& clear_button()
        {
            return clear_button_;
        }
        [[nodiscard]] maui::controls::button& fill_button()
        {
            return fill_button_;
        }
        [[nodiscard]] const std::shared_ptr<maui::core::observable_collection<photo_item>>& items() const
        {
            return items_;
        }
        // The currently-selected EmptyView container (EmptyView1 when the switch is on, else EmptyView2).
        [[nodiscard]] const std::shared_ptr<maui::controls::vertical_stack_layout>& active_empty_view() const
        {
            return empty_view_switch_.is_toggled() ? empty_view1_ : empty_view2_;
        }

        // SwitchEmptyView(): EmptyView = IsToggled ? EmptyView1 : EmptyView2.
        void switch_empty_view()
        {
            list_.set_empty_view(maui::controls::boxed_item::of(active_empty_view()));
        }

        // DemoFilteredItemSource.FilterItems(filter): keep the live order, drop the now-excluded, append
        // the newly-included — the in-place projection that drives the bound collection_view.
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

        // The clear/fill toggle: empty the live source (the active EmptyView appears) / restore the full set.
        void clear_items()
        {
            items_->clear();
        }
        void fill_items()
        {
            filter_items(""); // an empty filter matches every master row → restores the full set
        }

    private:
        // The two alternative EmptyView ContentViews (the XAML Resources EmptyView1 / EmptyView2), each a
        // vertical_stack_layout of labels owned for the page's lifetime so the boxed view stays valid.
        void build_empty_views()
        {
            empty_view1_ = std::make_shared<maui::controls::vertical_stack_layout>();
            empty1_primary_.set_text("No results matched your filter."); // bold, centered (cosmetic)
            empty1_primary_.set_horizontal_text_alignment(maui::core::text_alignment::center);
            empty1_secondary_.set_text("Maybe try a broader filter?"); // italic (cosmetic)
            empty1_secondary_.set_horizontal_text_alignment(maui::core::text_alignment::center);
            empty_view1_->add(empty1_primary_);
            empty_view1_->add(empty1_secondary_);

            empty_view2_ = std::make_shared<maui::controls::vertical_stack_layout>();
            empty2_primary_.set_text("Nothing to see here."); // bold, centered (cosmetic)
            empty2_primary_.set_horizontal_text_alignment(maui::core::text_alignment::center);
            empty_view2_->add(empty2_primary_); // (the coffee.png Image is omitted — see header note)
        }

        // The 12-row demo set the shared twin's x:Array carries (DemoFilteredItemSource's caption
        // pattern, truncated to the twin's static count so both frameworks render the same items).
        void build_master()
        {
            static const char* const images[] = {"cover1.jpg", "oasis.jpg",      "photo.jpg",  "Vegetables.jpg",
                                                 "Fruits.jpg", "FlowerBuds.jpg", "Legumes.jpg"};
            constexpr int count = 12;
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

        // DemoFilteredItemSource.ItemMatches: case-insensitive Caption substring (empty filter matches all).
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

        std::vector<photo_item> master_;                                       // DemoFilteredItemSource._source
        std::shared_ptr<maui::core::observable_collection<photo_item>> items_; // the live, bound collection

        // The two alternative EmptyViews (owned so the boxed view outlives the set).
        std::shared_ptr<maui::controls::vertical_stack_layout> empty_view1_;
        std::shared_ptr<maui::controls::vertical_stack_layout> empty_view2_;
        maui::controls::label empty1_primary_;
        maui::controls::label empty1_secondary_;
        maui::controls::label empty2_primary_;

        maui::controls::content_page page_;
        maui::controls::grid grid_;
        maui::controls::search_bar search_;
        maui::controls::horizontal_stack_layout toggle_row_;
        maui::controls::label toggle_caption_;
        maui::controls::toggle_switch empty_view_switch_;
        maui::controls::horizontal_stack_layout buttons_;
        maui::controls::button clear_button_;
        maui::controls::button fill_button_;
        maui::controls::collection_view list_;
    };
} // namespace maui::samples
