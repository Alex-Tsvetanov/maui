#pragma once
// maui::samples::empty_view_template_page — ports EmptyViewGalleries/EmptyViewTemplateGallery.xaml
// (+ EmptyViewTemplateGallery.xaml.cs).
//
// The C# gallery page is a Grid with two rows: a SearchBar ("Filter") on top (Grid.Row 0, Auto) and,
// below (Grid.Row 1, Star), a CollectionView with a GridItemsLayout (Span="3" Orientation="Vertical").
// Its distinguishing feature is an EmptyViewTemplate (a DataTemplate, NOT a plain string EmptyView):
//   <CollectionView.EmptyViewTemplate>
//     <DataTemplate>
//       <StackLayout>
//         <Label ... Text="{Binding Filter, StringFormat='Your filter term of {0} did not match any records'}"/>
//       </StackLayout>
//     </DataTemplate>
//   </CollectionView.EmptyViewTemplate>
// The xaml.cs ctor wires:
//   - CollectionView.ItemTemplate  = ExampleTemplates.PhotoTemplate()  (an Image over a caption Label);
//   - CollectionView.ItemsSource   = _demoFilteredItemSource.Items     (an ObservableCollection, 50 rows);
//   - CollectionView.EmptyView     = _emptyViewGalleryFilterInfo       (the BindingContext the
//                                    EmptyViewTemplate binds {Binding Filter} against — a tiny
//                                    INotifyPropertyChanged object with one Filter string);
//   - SearchBar.SearchCommand      = () => { _demoFilteredItemSource.FilterItems(SearchBar.Text);
//                                            _emptyViewGalleryFilterInfo.Filter = SearchBar.Text; }.
// So: type a filter term, invoke the search, the source is filtered down; when the term matches NOTHING
// the source goes empty and the CollectionView renders the EmptyViewTemplate — a Label reading
// "Your filter term of <term> did not match any records". That is the demonstrated feature: an EmptyView
// *template* (a templated view, data-bound to the EmptyView context object) shown on the empty state,
// plus the SearchBar acting as the toggle that empties / re-fills the source.
//
// Port mapping (mirrors empty_view_page for the filter plumbing, swapping the plain-string EmptyView
// for the templated EmptyViewTemplate):
//   - the search_bar drives a filter through its `search_command` (the C# SearchBar.SearchCommand
//     stand-in): filter_items() reconciles the LIVE source against the unfiltered backing list (remove
//     non-matching, re-add matching) — exactly DemoFilteredItemSource.FilterItems — and ALSO updates the
//     EmptyView context's filter so the template re-renders with the current term;
//   - the collection_view's EmptyView is a BOXED filter_info object (boxed_item::of over a SHARED
//     filter_info — reference semantics, so updating .filter mutates the very object the template's
//     binding context wraps), and EmptyViewTemplate is a data_template<label> whose Text binds
//     {Binding Filter} formatted "Your filter term of {0} did not match any records";
//   - the GridItemsLayout (Span 3, Vertical) is attached via set_items_layout;
//   - fill_items()/clear_items() are the explicit empty/fill toggle the task calls for: clear empties the
//     live source (the EmptyViewTemplate appears), fill restores the full unfiltered set.
//
// How the template re-binds on the empty state: the handler's update_empty_view realizes the
// EmptyViewTemplate's content and sets its binding context to the EmptyView's context_box (the boxed
// filter_info) — collection_view_handler::realize_supplemental does exactly this. Because filter_items
// updates filter_info_->filter BEFORE the source-change notification re-runs update_empty_view, the
// freshly realized Label reads the CURRENT term. (note: the realize-on-empty path pushes the context at
// realize time; mutating filter_info_->filter while the EmptyView is already shown takes effect the next
// time the empty view is realized — which the filter/clear flow drives, so the term shown always matches
// the last filter that emptied the source.)
//
// note: the C# item template is ExampleTemplates.PhotoTemplate() (an Image over a caption Label). The
//       port template here is the caption Label only — an Image cell would need an i_image_source per
//       row, which the headless backend has no asset pipeline to resolve; the caption carries the image
//       file name, so the demonstrated text covers the intent (same deviation as empty_view_page).
//
// The page OWNS its whole element tree (the items_page pattern); the generic mount (app_host.hpp) attaches
// every owned view's handler and hosts the grid + page.

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
#include "maui/controls/view.hpp" // margin_property
#include "maui/core/grid_length.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/core/thickness.hpp"

namespace maui::samples
{
    class empty_view_template_page
    {
    public:
        // One row of the demo source — the caption the PhotoTemplate binds + the filter matches against
        // (CollectionViewGalleryTestItem.Caption, reduced to what the page surfaces).
        struct demo_item
        {
            std::string caption;
        };

        // The EmptyViewGalleryFilterInfo stand-in: the one Filter string the EmptyViewTemplate binds
        // {Binding Filter} against (the EmptyView's binding context object).
        struct filter_info
        {
            std::string filter;
            friend bool operator==(const filter_info&, const filter_info&) = default;
        };

        empty_view_template_page()
            : items_(std::make_shared<maui::core::observable_collection<demo_item>>(source_items())),
              filter_info_(std::make_shared<filter_info>())
        {
            page_.set_title("EmptyView (template)");

            // Grid: an Auto row (the SearchBar) over a Star row (the CollectionView) — the oracle layout.
            // Padding 12 / RowSpacing 6: the shared twin's root-grid shape.
            grid_.set_padding(maui::core::thickness(12));
            grid_.set_row_spacing(6);
            grid_.add_row_definition(maui::core::grid_length::automatic());
            grid_.add_row_definition(maui::core::grid_length::star());

            search_.set_placeholder("Filter");
            // SearchBar.SearchCommand = filter the source by the current text AND publish the term into the
            // EmptyView context (the C# xaml.cs ctor wiring: FilterItems(...) + FilterInfo.Filter = Text).
            search_.search_command = [this] { run_filter(std::string(search_.text())); };
            // The C# SearchBar PropertyChanged branch: clearing the text re-shows every row.
            search_.text_changed.connect([this](const std::string& /*old*/, const std::string& current) {
                if (current.empty())
                {
                    run_filter("");
                }
            });

            // ---- the GridItemsLayout: Span="3" Orientation="Vertical" ----
            auto layout = std::make_shared<maui::controls::grid_items_layout>(
                3, maui::controls::items_layout_orientation::vertical);
            list_.set_items_layout(layout);

            // ---- the item template: the PhotoTemplate caption Label (Text ← item caption; see note),
            //      Margin 6 (the shared twin's cell shape) ----
            auto cell = maui::controls::data_template::of<maui::controls::label>();
            cell->set_binding<std::string, demo_item>(maui::controls::label::text_property(),
                                                      [](const demo_item& item) { return item.caption; });
            cell->set_value(maui::controls::margin_property(), maui::core::thickness(6));
            list_.set_item_template(cell);
            list_.set_items_source(items_);

            // ---- the EmptyView (the binding-context object) + the EmptyViewTemplate (the templated view
            // shown on the empty state). The template's Label binds {Binding Filter} formatted exactly like
            // the XAML StringFormat. The EmptyView is the SHARED filter_info_ so updating .filter mutates the
            // object the template binds against. ----
            list_.set_empty_view(maui::controls::boxed_item::of(filter_info_));
            auto empty_cell = maui::controls::data_template::of<maui::controls::label>();
            empty_cell->set_binding<std::string, filter_info>(
                maui::controls::label::text_property(), [](const filter_info& info) {
                    return "Your filter term of " + info.filter + " did not match any records";
                });
            list_.set_empty_view_template(empty_cell);

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
        [[nodiscard]] const std::shared_ptr<filter_info>& empty_view_filter_info() const
        {
            return filter_info_;
        }

        // The C# SearchCommand body: DemoFilteredItemSource.FilterItems(text) + FilterInfo.Filter = text.
        // Updating filter_info_->filter FIRST means the term is current by the time the source-change
        // notification re-realizes the EmptyViewTemplate (header note).
        void run_filter(const std::string& filter)
        {
            filter_info_->filter = filter;
            filter_items(filter);
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

        // The explicit empty/fill toggle the task calls for (clear → the EmptyViewTemplate appears).
        void clear_items()
        {
            items_->clear();
        }
        void fill_items()
        {
            run_filter(""); // restore the full unfiltered set (and clear the published filter term)
        }

    private:
        // The 12-row demo set the shared twin's x:Array carries (DemoFilteredItemSource's caption
        // pattern "<image>, <n>", truncated to the twin's static count).
        [[nodiscard]] static std::vector<demo_item> source_items()
        {
            static const std::vector<std::string> images{"cover1.jpg", "oasis.jpg",      "photo.jpg",  "Vegetables.jpg",
                                                         "Fruits.jpg", "FlowerBuds.jpg", "Legumes.jpg"};
            std::vector<demo_item> rows;
            for (int n = 0; n < 12; ++n)
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
        std::shared_ptr<filter_info> filter_info_;                            // the EmptyView binding context
        maui::controls::content_page page_;
        maui::controls::grid grid_;
        maui::controls::search_bar search_;
        maui::controls::collection_view list_;
    };
} // namespace maui::samples
