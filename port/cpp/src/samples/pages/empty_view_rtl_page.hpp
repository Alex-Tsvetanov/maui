#pragma once
// maui::samples::empty_view_rtl_page — ports EmptyViewGalleries/EmptyViewRTLGallery.xaml
// (+ EmptyViewRTLGallery.xaml.cs).
//
// The C# gallery page (x:Name="EmptyViewRTLPage", Title="EmptyView RTL Gallery") is a Grid of two rows
// (Auto over Star):
//   - row 0: a vertical StackLayout holding a Picker (Title="FlowDirection", ItemsSource = the two
//     strings "Left to Right" / "Right to Left") over a SearchBar (Placeholder="Filter");
//   - row 1: a CollectionView (GridItemsLayout Span="3" Vertical) whose EmptyView is an INLINE
//     StackLayout of two Labels — a bold 18pt Start-aligned "No results matched your filter." over an
//     italic 12pt End-aligned "Maybe try a broader filter?".
// The code-behind ctor sets Picker.SelectedIndex = 0, the ExampleTemplates.PhotoTemplate() item
// template, the DemoFilteredItemSource.Items source (starts populated), and the SearchBar.SearchCommand
// (= FilterItems(SearchBar.Text)). OnPickerSelectedIndexChanged sets the PAGE'S FlowDirection:
// index 0 -> LeftToRight, index 1 -> RightToLeft. So the picker flips the whole page (and thus the
// EmptyView, which is the point — it shows the EmptyView laid out under RTL), and the SearchBar filters
// the source down until it empties and the two-label EmptyView appears.
//
// The demonstrated feature: an EmptyView (a real templated/inline view, not a string) rendered under a
// FlowDirection the user toggles RightToLeft via the Picker — plus the SearchBar as the toggle that
// empties the source so the EmptyView shows.
//
// Port mapping (mirrors empty_view_swap_page's owned-view EmptyView + filter_collection_page's
// DemoFilteredItemSource projection, adding the flow_direction toggle):
//   - row 0 is a vertical_stack_layout holding the picker_ (FlowDirection title, the two LTR/RTL strings
//     as its items_source) over the search_ ("Filter"); the picker's selected_index_changed sets the
//     PAGE'S flow_direction (view::set_flow_direction — content_page is a view, so it carries the knob),
//     exactly OnPickerSelectedIndexChanged (index 0 -> left_to_right, 1 -> right_to_left). The ctor sets
//     selected_index = 0 (the C# Picker.SelectedIndex = 0), so the page starts left_to_right;
//   - the search_'s search_command filters the live source by the current text (the C#
//     SearchBar.SearchCommand = FilterItems(SearchBar.Text)); filter_items reconciles the live source
//     against the master list (DemoFilteredItemSource.FilterItems);
//   - row 1 is the collection_view (GridItemsLayout Span 3, Vertical), the PhotoTemplate caption-Label
//     cell (see note), the live source SEEDED with the full set (DemoFilteredItemSource.Items starts
//     populated); the EmptyView is an OWNED vertical_stack_layout of the two labels (Start-aligned
//     primary over End-aligned secondary), boxed via boxed_item::of(shared_ptr<view>) — shown once the
//     filter empties the source.
//
// The page OWNS its whole element tree (the items_page pattern); the EmptyView tree is owned + rehosted
// too, so it materializes when the source empties. attach_handlers wires every owned view bottom-up then
// re-hosts the stacks + grid + page.
//
// note: the C# item template is ExampleTemplates.PhotoTemplate() (an Image over a caption Label). The
//       port cell is the caption Label only — an Image cell would need an i_image_source per row, which
//       the headless backend has no asset pipeline to resolve; the caption carries the image file name,
//       so the demonstrated text covers the intent (same deviation as the sibling EmptyView pages).
// note: the C# EmptyView labels carry cosmetic FontAttributes (Bold / Italic), FontSize (18 / 12), and
//       Margin; the port sets the load-bearing HorizontalTextAlignment (Start / End) and text. The
//       cosmetic font knobs are omitted as non-load-bearing for the demonstrated layout-under-RTL intent
//       (the empty_view_swap_page precedent treats the bold/italic the same way). Not invented.

#include <algorithm>
#include <cctype>
#include <memory>
#include <string>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/grid_items_layout.hpp"
#include "maui/controls/items/items_layout_orientation.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/picker.hpp"
#include "maui/controls/search_bar.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/flow_direction.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class empty_view_rtl_page
    {
    public:
        // The reflection-free CollectionViewGalleryTestItem stand-in: just the Caption the template binds
        // and the filter matches (filter_collection_page::photo_item).
        struct photo_item
        {
            std::string caption;
            friend bool operator==(const photo_item&, const photo_item&) = default;
        };

        empty_view_rtl_page()
            : items_(std::make_shared<maui::core::observable_collection<photo_item>>()),
              flow_options_(std::make_shared<maui::controls::picker::items_source_type>())
        {
            page_.set_title("EmptyView RTL Gallery");

            build_master(); // DemoFilteredItemSource ctor: AddItems(_source, 50)
            seed_live();    // Items = new ObservableCollection<...>(_source) — start populated

            build_empty_view(); // the inline two-label EmptyView StackLayout

            // ---- the Grid: an Auto row (the header stack) over a Star row (the CollectionView) ----
            grid_.add_row_definition(maui::core::grid_length::automatic());
            grid_.add_row_definition(maui::core::grid_length::star());

            // ---- row 0: the vertical StackLayout — Picker over SearchBar ----
            // The Picker (Title="FlowDirection") with the two flow-direction strings as its items_source.
            picker_.set_title("FlowDirection");
            flow_options_->add(std::string{"Left to Right"});
            flow_options_->add(std::string{"Right to Left"});
            picker_.set_items_source(flow_options_);
            // OnPickerSelectedIndexChanged: index 0 -> LeftToRight, index 1 -> RightToLeft (on the PAGE).
            picker_.selected_index_changed.connect([this] { apply_flow_direction(); });

            // The SearchBar ("Filter") whose SearchCommand filters the live source by the current text.
            search_.set_placeholder("Filter");
            search_.search_command = [this] { filter_items(std::string(search_.text())); };

            header_stack_.set_spacing(8);
            header_stack_.add(picker_);
            header_stack_.add(search_);

            // ---- row 1: the CollectionView (3-span vertical grid of caption labels) ----
            auto cell = maui::controls::data_template::of<maui::controls::label>();
            cell->set_binding<std::string, photo_item>(maui::controls::label::text_property(),
                                                       [](const photo_item& item) { return item.caption; });
            list_.set_item_template(cell);
            list_.set_items_layout(std::make_shared<maui::controls::grid_items_layout>(
                3, maui::controls::items_layout_orientation::vertical)); // GridItemsLayout Span="3"
            list_.set_items_source(items_);
            // The inline two-label EmptyView (the owned StackLayout), shown once the filter empties source.
            list_.set_empty_view(maui::controls::boxed_item::of(empty_view_));

            grid_.set_row(header_stack_, 0);
            grid_.add(header_stack_);
            grid_.set_row(list_, 1);
            grid_.add(list_);
            page_.set_content(grid_);

            // C# ctor: Picker.SelectedIndex = 0 (the page starts LeftToRight). Set after the wiring so the
            // selected_index_changed handler applies left_to_right to the page.
            picker_.set_selected_index(0);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (leaves first, the page last), then re-host the
        // tree built in the ctor (gallery_attach.hpp). The EmptyView tree is attached + rehosted too so it
        // materializes when the source empties.
        void attach_handlers(maui::hosting::maui_app& app)
        {
            // EmptyView leaves + container
            gallery_attach_one(app, empty_primary_, "empty_primary_");
            gallery_attach_one(app, empty_secondary_, "empty_secondary_");
            gallery_attach_one(app, *empty_view_, "empty_view_");

            // header row
            gallery_attach_one(app, picker_, "picker_");
            gallery_attach_one(app, search_, "search_");
            gallery_attach_one(app, header_stack_, "header_stack_");

            // list + grid + page
            gallery_attach_one(app, list_, "list_");
            gallery_attach_one(app, grid_, "grid_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(*empty_view_);  // the EmptyView stack hosts its two labels
            gallery_rehost_layout(header_stack_); // the header stack hosts the picker + search bar
            gallery_rehost_layout(grid_);         // the grid hosts the header stack + collection_view
            gallery_rehost_content(page_);        // the page hosts the grid
        }

        // ---- accessors (used by the hosting main + any test tree) ----
        [[nodiscard]] maui::controls::grid& grid()
        {
            return grid_;
        }
        [[nodiscard]] maui::controls::picker& picker()
        {
            return picker_;
        }
        [[nodiscard]] maui::controls::search_bar& search()
        {
            return search_;
        }
        [[nodiscard]] maui::controls::collection_view& list()
        {
            return list_;
        }
        [[nodiscard]] const std::shared_ptr<maui::core::observable_collection<photo_item>>& items() const
        {
            return items_;
        }

        // OnPickerSelectedIndexChanged: set the PAGE's FlowDirection from the picker index (0 -> LTR,
        // 1 -> RTL; the C# default branch is LTR). content_page is a view, so it carries flow_direction.
        void apply_flow_direction()
        {
            page_.set_flow_direction(picker_.selected_index() == 1 ? maui::core::flow_direction::right_to_left
                                                                   : maui::core::flow_direction::left_to_right);
        }

        // DemoFilteredItemSource.FilterItems(filter): keep the live order, drop the now-excluded, append
        // the newly-included — the in-place projection that drives the bound collection_view (the C#
        // SearchBar.SearchCommand body).
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
        // The inline two-label EmptyView StackLayout (the XAML CollectionView.EmptyView), owned for the
        // page's lifetime so the boxed view stays valid.
        void build_empty_view()
        {
            empty_view_ = std::make_shared<maui::controls::vertical_stack_layout>();
            // Bold 18pt Start-aligned "No results matched your filter." (font knobs cosmetic — see note).
            empty_primary_.set_text("No results matched your filter.");
            empty_primary_.set_horizontal_text_alignment(maui::core::text_alignment::start);
            // Italic 12pt End-aligned "Maybe try a broader filter?" (font knobs cosmetic — see note).
            empty_secondary_.set_text("Maybe try a broader filter?");
            empty_secondary_.set_horizontal_text_alignment(maui::core::text_alignment::end);
            empty_view_->add(empty_primary_);
            empty_view_->add(empty_secondary_);
        }

        // DemoFilteredItemSource.AddItems: 50 captioned items cycling the demo image names.
        void build_master()
        {
            static const char* const images[] = {"cover1.jpg", "oasis.jpg",      "photo.jpg",  "Vegetables.jpg",
                                                 "Fruits.jpg", "FlowerBuds.jpg", "Legumes.jpg"};
            constexpr int count = 50;
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

        std::vector<photo_item> master_;                                          // DemoFilteredItemSource._source
        std::shared_ptr<maui::core::observable_collection<photo_item>> items_;    // the live, bound collection
        std::shared_ptr<maui::controls::picker::items_source_type> flow_options_; // the two LTR/RTL strings

        // The inline EmptyView (owned so the boxed view outlives the set).
        std::shared_ptr<maui::controls::vertical_stack_layout> empty_view_;
        maui::controls::label empty_primary_;
        maui::controls::label empty_secondary_;

        maui::controls::content_page page_;
        maui::controls::grid grid_;
        maui::controls::vertical_stack_layout header_stack_;
        maui::controls::picker picker_;
        maui::controls::search_bar search_;
        maui::controls::collection_view list_;
    };
} // namespace maui::samples
