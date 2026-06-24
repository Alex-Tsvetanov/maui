#pragma once
// maui::samples::empty_view_null_page — ports EmptyViewGalleries/EmptyViewNullGallery.xaml
// (+ EmptyViewNullGallery.xaml.cs).
//
// The C# gallery page is a bare ContentPage whose Content is a single CollectionView with a
// GridItemsLayout (Span="3" Orientation="Vertical") and NO EmptyView declared in XAML. The code-behind
// ctor (EmptyViewNullGallery(bool useOnlyText = true)) is what SETS the EmptyView after
// InitializeComponent: useOnlyText ? "Nothing to display." (a plain string) : a Grid wrapping a bold,
// centered "Nothing to display." Label. The sample's gallery descriptor constructs it with the default
// useOnlyText = true, so the surfaced EmptyView is the plain string "Nothing to display." The DISTINCT
// feature of this page (vs the sibling EmptyView galleries) is that the EmptyView is assigned ENTIRELY
// from code-behind — the XAML leaves it null/unset — and the page starts with NO ItemsSource at all
// (the CollectionView is empty from the first frame, so the EmptyView shows immediately).
//
// The task framing ("EmptyView left null/unset — nothing shows when empty, vs a populated state, with a
// clear/fill toggle") maps to the C# shape thus: the XAML EmptyView IS null/unset; the assigned
// EmptyView is the plain "Nothing to display." string; the page begins empty (EmptyView visible) and a
// Fill button populates the source (a populated state), a Clear button empties it again (back to the
// EmptyView). That toggle is the explicit, observable demo affordance the task calls for over the
// oracle's "always empty" starting state.
//
// Port mapping (mirrors empty_view_page's plain-string-EmptyView plumbing, dropping the SearchBar since
// this oracle has none):
//   - the collection_view has a GridItemsLayout (Span 3, Vertical) via set_items_layout, an
//     ExampleTemplates.PhotoTemplate() caption-Label cell (the empty_view_page convention — see note),
//     and a live observable_collection bound source that STARTS EMPTY (matching the oracle: no
//     ItemsSource in XAML, so the CollectionView renders the EmptyView from the first frame);
//   - the EmptyView is the boxed STRING "Nothing to display." (boxed_item::of(std::string), the C#
//     useOnlyText branch — the boxed string-vs-view split);
//   - Fill restores the full demo set (a populated state — the EmptyView disappears); Clear empties the
//     live source again (the EmptyView reappears). The headless collection_view virtualization sim
//     realizes the cells when the source fills and surfaces the EmptyView when it empties.
//
// The page OWNS its whole element tree (the items_page pattern). The generic mount (app_host.hpp) attaches
// every owned view's handler and hosts the page content.
//
// note: the C# item template is ExampleTemplates.PhotoTemplate() (an Image over a caption Label). The
//       port cell is the caption Label only — an Image cell would need an i_image_source per row, which
//       the headless backend has no asset pipeline to resolve; the caption carries the image file name,
//       so the demonstrated text covers the intent (same deviation as empty_view_page / the EmptyView
//       sibling pages). Not invented — a documented gap.
// note: the C# useOnlyText=false branch (a Grid-wrapped bold centered Label EmptyView) is NOT
//       reproduced — the sample descriptor uses the default useOnlyText=true (the plain string), so the
//       string EmptyView is the demonstrated state, exactly like the oracle as instantiated.

#include <memory>
#include <string>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/grid_items_layout.hpp"
#include "maui/controls/items/items_layout_orientation.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/core/observable_collection.hpp"

namespace maui::samples
{
    class empty_view_null_page
    {
    public:
        // One row of the demo source — the caption the PhotoTemplate binds (the
        // CollectionViewGalleryTestItem.Caption, reduced to what the page surfaces).
        struct demo_item
        {
            std::string caption;
        };

        empty_view_null_page() : items_(std::make_shared<maui::core::observable_collection<demo_item>>())
        {
            page_.set_title("EmptyView (null in XAML)");

            // ---- the GridItemsLayout: Span="3" Orientation="Vertical" (the oracle's only layout knob) ----
            list_.set_items_layout(std::make_shared<maui::controls::grid_items_layout>(
                3, maui::controls::items_layout_orientation::vertical));

            // ---- the item template: the PhotoTemplate caption Label (Text <- item caption; see note) ----
            auto cell = maui::controls::data_template::of<maui::controls::label>();
            cell->set_binding<std::string, demo_item>(maui::controls::label::text_property(),
                                                      [](const demo_item& item) { return item.caption; });
            list_.set_item_template(cell);

            // The CollectionView starts EMPTY (the oracle declares no ItemsSource in XAML), so the
            // EmptyView is what renders from the first frame.
            list_.set_items_source(items_);

            // The EmptyView assigned from code-behind (the C# useOnlyText=true branch): a plain STRING.
            // The XAML left it null/unset — this is the entire point of the page.
            list_.set_empty_view(maui::controls::boxed_item::of(std::string{"Nothing to display."}));

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

        // The explicit demo affordance the task calls for: Fill -> a populated state (the EmptyView
        // disappears); Clear -> back to empty (the null-in-XAML, then assigned-string EmptyView shows).
        void fill_items()
        {
            if (!items_->empty())
            {
                return; // already populated — keep it idempotent
            }
            for (const demo_item& item : source_items())
            {
                items_->add(item);
            }
        }
        void clear_items()
        {
            items_->clear();
        }

    private:
        // DemoFilteredItemSource().AddItems(50): the default 50-row set, captioned "<image>, <n>" (the
        // populated state Fill restores; the oracle never populates, but the task's fill/clear toggle
        // makes the empty<->populated transition observable).
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

        std::shared_ptr<maui::core::observable_collection<demo_item>> items_; // the live, bound source
        maui::controls::content_page page_;
        maui::controls::collection_view list_;
    };
} // namespace maui::samples
