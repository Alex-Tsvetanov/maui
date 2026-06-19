#pragma once
// maui::samples::empty_view_view_page — ports EmptyViewGalleries/EmptyViewViewGallery.xaml
// (+ EmptyViewViewGallery.xaml.cs).
//
// The C# gallery page is a Grid with two rows: a SearchBar ("Filter", x:Name SearchBar) on top
// (Grid.Row 0, Auto) and, below (Grid.Row 1, Star), a CollectionView whose ItemsLayout is a
// GridItemsLayout (Span="3" Orientation="Vertical"). Its distinguishing feature is the EmptyView — and
// here, unlike the string gallery (EmptyViewStringGallery) and the template gallery
// (EmptyViewTemplateGallery), the EmptyView is a *VIEW* declared inline in XAML:
//   <CollectionView.EmptyView>
//     <StackLayout>
//       <Label FontAttributes="Bold"   FontSize="18" Margin="10,25,10,10"
//              HorizontalOptions="Fill" HorizontalTextAlignment="Center"
//              Text="No results matched your filter."/>
//       <Label FontAttributes="Italic" FontSize="12"
//              HorizontalOptions="Fill" HorizontalTextAlignment="Center"
//              Text="Maybe try a broader filter?"/>
//     </StackLayout>
//   </CollectionView.EmptyView>
// The xaml.cs ctor wires:
//   - CollectionView.ItemTemplate = ExampleTemplates.PhotoTemplate()  (an Image over a caption Label);
//   - CollectionView.ItemsSource  = _demoFilteredItemSource.Items     (an ObservableCollection, 50 rows);
//   - SearchBar.SearchCommand     = new Command(() => _demoFilteredItemSource.FilterItems(SearchBar.Text));
//   - SearchBar.PropertyChanged   = clearing the Text re-shows every row (FilterItems("") on IsNullOrEmpty).
// So: type a filter term, invoke the search, the source filters down; when the term matches NOTHING the
// source goes empty and the CollectionView renders the EmptyView — the StackLayout-of-two-Labels view.
// That is the demonstrated feature: an EmptyView that is a *View* (not a plain string, not an
// EmptyViewTemplate), shown on the empty state, plus the SearchBar as the empty/fill toggle.
//
// Port mapping (mirrors empty_view_page / empty_view_template_page for the filter plumbing, swapping the
// string / templated EmptyView for a boxed VIEW):
//   - the search_bar drives a filter through its `search_command` (the C# SearchBar.SearchCommand stand-in):
//     filter_items() reconciles the LIVE source against the unfiltered backing list (remove non-matching,
//     re-add matching) — exactly DemoFilteredItemSource.FilterItems;
//   - text_changed re-shows every row when the box is cleared (the C# PropertyChanged IsNullOrEmpty branch);
//   - the collection_view's EmptyView is a BOXED vertical_stack_layout (boxed_item::of over a
//     shared_ptr<vertical_stack_layout>). A boxed item that IS a view is hosted DIRECTLY by the handler
//     (collection_view_handler::realize_supplemental — `value.as_bindable()` branch — the C# `EmptyView
//     is View` split), so the StackLayout and its two child Labels render on the empty state;
//   - the GridItemsLayout (Span 3, Vertical) is attached via set_items_layout (the oracle's ItemsLayout);
//   - clear_items()/fill_items() are the explicit empty/fill toggle the task calls for: clear empties the
//     live source (the EmptyView appears), fill restores the full unfiltered set.
//
// note: the C# item template is ExampleTemplates.PhotoTemplate() (an Image over a caption Label). The
//       port template here is the caption Label only — an Image cell would need an i_image_source per
//       row, which the headless backend has no asset pipeline to resolve; the caption carries the image
//       file name, so the demonstrated text covers the intent (same deviation as the sibling EmptyView
//       pages).
// note: the EmptyView Labels carry FontAttributes (Bold / Italic) + FontSize, mapped via set_font onto a
//       core::font (weight bold / slant italic, of_size). HorizontalTextAlignment="Center" maps to
//       set_horizontal_text_alignment(center); HorizontalOptions="Fill" maps to
//       set_horizontal_layout_alignment(fill). The XAML Margin="10,25,10,10" on the primary Label maps to
//       set_margin(thickness(10, 25, 10, 10)) — the VisualElement/View.Margin seam.
//
// The page OWNS its whole element tree (the items_page pattern); attach_handlers wires handlers bottom-up
// (including the EmptyView StackLayout + its Labels) then re-hosts the grid + page.

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
#include "maui/controls/search_bar.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/font.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/layout_alignment.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/thickness.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class empty_view_view_page
    {
    public:
        // One row of the demo source — the caption the PhotoTemplate binds + the filter matches against
        // (CollectionViewGalleryTestItem.Caption, reduced to what the page surfaces).
        struct demo_item
        {
            std::string caption;
        };

        empty_view_view_page()
            : items_(std::make_shared<maui::core::observable_collection<demo_item>>(source_items())),
              empty_view_(std::make_shared<maui::controls::vertical_stack_layout>())
        {
            page_.set_title("EmptyView (view)");

            // Grid: an Auto row (the SearchBar) over a Star row (the CollectionView) — the oracle layout.
            grid_.add_row_definition(maui::core::grid_length::automatic());
            grid_.add_row_definition(maui::core::grid_length::star());

            search_.set_placeholder("Filter");
            // SearchBar.SearchCommand = filter the source by the current text (the C# xaml.cs ctor wiring).
            search_.search_command = [this] { filter_items(std::string(search_.text())); };
            // The C# PropertyChanged branch: clearing the text re-shows every row.
            search_.text_changed.connect([this](const std::string& /*old*/, const std::string& current) {
                if (current.empty())
                {
                    filter_items("");
                }
            });

            // ---- the GridItemsLayout: Span="3" Orientation="Vertical" ----
            auto layout = std::make_shared<maui::controls::grid_items_layout>(
                3, maui::controls::items_layout_orientation::vertical);
            list_.set_items_layout(layout);

            // ---- the item template: the PhotoTemplate caption Label (Text <- item caption; see note) ----
            auto cell = maui::controls::data_template::of<maui::controls::label>();
            cell->set_binding<std::string, demo_item>(maui::controls::label::text_property(),
                                                      [](const demo_item& item) { return item.caption; });
            list_.set_item_template(cell);
            list_.set_items_source(items_);

            // ---- the EmptyView: a VIEW (the StackLayout of two Labels), boxed and hosted directly ----
            // Label 1: Bold, 18pt, centered — "No results matched your filter."
            empty_label_primary_.set_text("No results matched your filter.");
            empty_label_primary_.set_font(maui::core::font::of_size("", 18, maui::core::font_weight::bold));
            empty_label_primary_.set_horizontal_text_alignment(maui::core::text_alignment::center);
            empty_label_primary_.set_horizontal_layout_alignment(maui::core::layout_alignment::fill);
            empty_label_primary_.set_margin(maui::core::thickness(10, 25, 10, 10)); // XAML Margin="10,25,10,10"
            // Label 2: Italic, 12pt, centered — "Maybe try a broader filter?"
            empty_label_secondary_.set_text("Maybe try a broader filter?");
            empty_label_secondary_.set_font(
                maui::core::font::of_size("", 12, maui::core::font_weight::regular, maui::core::font_slant::italic));
            empty_label_secondary_.set_horizontal_text_alignment(maui::core::text_alignment::center);
            empty_label_secondary_.set_horizontal_layout_alignment(maui::core::layout_alignment::fill);
            empty_view_->add(empty_label_primary_);
            empty_view_->add(empty_label_secondary_);
            // A boxed VIEW hosts directly (boxed_item::of over a bindable_object sets the as_bindable
            // slot; realize_supplemental takes the `value.as_bindable()` branch — the C# EmptyView-is-View
            // path) — so on the empty state the StackLayout + its two Labels render.
            list_.set_empty_view(maui::controls::boxed_item::of(empty_view_));

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
        // tree built in the ctor (gallery_attach.hpp). The EmptyView StackLayout + its Labels are owned
        // views too, so they get handlers + a re-host so the empty-state view materializes natively.
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, empty_label_primary_, "empty_label_primary_");
            gallery_attach_one(app, empty_label_secondary_, "empty_label_secondary_");
            gallery_attach_one(app, *empty_view_, "empty_view_");
            gallery_attach_one(app, search_, "search_");
            gallery_attach_one(app, list_, "list_");
            gallery_attach_one(app, grid_, "grid_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(*empty_view_); // the EmptyView StackLayout hosts its two Labels
            gallery_rehost_layout(grid_);        // the grid hosts the search bar + collection_view
            gallery_rehost_content(page_);       // the page hosts the grid
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
        [[nodiscard]] maui::controls::vertical_stack_layout& empty_view()
        {
            return *empty_view_;
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

        // The explicit empty/fill toggle the task calls for (clear -> the EmptyView appears).
        void clear_items()
        {
            items_->clear();
        }
        void fill_items()
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
        std::shared_ptr<maui::controls::vertical_stack_layout> empty_view_;   // the VIEW EmptyView (StackLayout)
        maui::controls::label empty_label_primary_;                           // "No results matched your filter."
        maui::controls::label empty_label_secondary_;                         // "Maybe try a broader filter?"
        maui::controls::content_page page_;
        maui::controls::grid grid_;
        maui::controls::search_bar search_;
        maui::controls::collection_view list_;
    };
} // namespace maui::samples
