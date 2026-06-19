#pragma once
// maui::samples::empty_view_load_simulate_page — ports EmptyViewGalleries/EmptyViewLoadSimulateGallery.xaml
// (+ EmptyViewLoadSimulateGallery.xaml.cs).
//
// The C# gallery page is a Grid of two rows (Auto over Star) with a single CollectionView in row 1. The
// CollectionView has a GridItemsLayout (Span="3" Orientation="Vertical") and a plain-STRING EmptyView
// declared INLINE in XAML: "Items loading simulation..." The code-behind ctor sets the
// ExampleTemplates.PhotoTemplate() item template, then kicks off a simulated async LOAD on a background
// task:
//   Task.Run(async () => {
//     await Task.Delay(1000);
//     Dispatcher.Dispatch(() => CollectionView.ItemsSource = new List<object>());          // empty list
//     await Task.Delay(1000);
//     Dispatcher.Dispatch(() => CollectionView.ItemsSource = _demoFilteredItemSource.Items); // populated
//   });
// So the page starts with NO ItemsSource (the inline string EmptyView "Items loading simulation..." is
// what renders), then 1s later swaps to an EMPTY list (the EmptyView still shows — still nothing to
// display), then 1s after that swaps to the populated DemoFilteredItemSource.Items (the EmptyView
// disappears, the 50 cells render). The demonstrated feature is the EmptyView covering a SIMULATED ASYNC
// LOAD — the placeholder shown while data "loads", replaced once the source arrives populated.
//
// Port mapping (mirrors empty_view_page's plain-string-EmptyView plumbing, in the oracle's two-row Grid):
//   - the Grid has an Auto row (0) over a Star row (1); the collection_view sits in row 1 exactly like
//     the oracle (set_row(list_, 1));
//   - the collection_view has a GridItemsLayout (Span 3, Vertical), an ExampleTemplates.PhotoTemplate()
//     caption-Label cell (the empty_view_page convention — see note), and a live observable_collection
//     bound source that STARTS EMPTY (matching the oracle's no-ItemsSource-then-empty-list start, so the
//     EmptyView renders);
//   - the EmptyView is the boxed STRING "Items loading simulation..." (boxed_item::of(std::string), the
//     XAML inline string EmptyView — the boxed string-vs-view split);
//   - load() drives the simulated async load to completion: it fills the live source with the full demo
//     set (the EmptyView disappears, the cells render) — the end-state of the C# Task. reset() empties
//     the source again (back to the "loading" EmptyView), so the load can be replayed/observed.
//
// The page OWNS its whole element tree (the items_page pattern). attach_handlers wires every owned view
// bottom-up then re-hosts the grid + page.
//
// note: the C# load is a real background Task with two 1-second Delays dispatched onto the UI thread.
//       The headless backend has no dispatcher/run-loop to pump a delayed continuation deterministically
//       in a unit-testable way, so the port collapses the simulation to a single synchronous load()
//       (start empty -> EmptyView; load() -> populated) — the OBSERVABLE end-state transition the oracle
//       drives, minus the wall-clock delays. The intermediate "empty List<object>" step (which leaves
//       the EmptyView showing) is the same empty state load() starts from, so no information is lost.
//       Not invented — a documented deviation forced by the headless backend's lack of a timed
//       dispatcher.
// note: the C# item template is ExampleTemplates.PhotoTemplate() (an Image over a caption Label). The
//       port cell is the caption Label only — an Image cell would need an i_image_source per row, which
//       the headless backend has no asset pipeline to resolve; the caption carries the image file name,
//       so the demonstrated text covers the intent (same deviation as empty_view_page). Not invented.

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
#include "maui/controls/templates/data_template.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class empty_view_load_simulate_page
    {
    public:
        // One row of the demo source — the caption the PhotoTemplate binds (the
        // CollectionViewGalleryTestItem.Caption, reduced to what the page surfaces).
        struct demo_item
        {
            std::string caption;
        };

        empty_view_load_simulate_page() : items_(std::make_shared<maui::core::observable_collection<demo_item>>())
        {
            page_.set_title("EmptyView (load simulation)");

            // ---- the Grid: an Auto row over a Star row (the oracle's two-row shape) ----
            grid_.add_row_definition(maui::core::grid_length::automatic());
            grid_.add_row_definition(maui::core::grid_length::star());

            // ---- the GridItemsLayout: Span="3" Orientation="Vertical" ----
            list_.set_items_layout(std::make_shared<maui::controls::grid_items_layout>(
                3, maui::controls::items_layout_orientation::vertical));

            // ---- the item template: the PhotoTemplate caption Label (Text <- item caption; see note) ----
            auto cell = maui::controls::data_template::of<maui::controls::label>();
            cell->set_binding<std::string, demo_item>(maui::controls::label::text_property(),
                                                      [](const demo_item& item) { return item.caption; });
            list_.set_item_template(cell);

            // The CollectionView starts EMPTY (the oracle starts with no ItemsSource, then an empty
            // List<object>), so the inline string EmptyView is what renders while data "loads".
            list_.set_items_source(items_);

            // The inline-XAML string EmptyView shown during the simulated load.
            list_.set_empty_view(maui::controls::boxed_item::of(std::string{"Items loading simulation..."}));

            // The CollectionView is in Grid.Row 1 (the oracle), under the empty Auto row 0.
            grid_.set_row(list_, 1);
            grid_.add(list_);
            page_.set_content(grid_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (the list, the grid, then the page), then
        // re-host the tree built in the ctor (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, list_, "list_");
            gallery_attach_one(app, grid_, "grid_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(grid_);  // the grid hosts the collection_view
            gallery_rehost_content(page_); // the page hosts the grid
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

        // The simulated async load's end-state (the C# Task's final Dispatch): populate the live source
        // with the full DemoFilteredItemSource set — the EmptyView disappears, the cells render. (See the
        // header note: the wall-clock delays + the intermediate empty-list step are collapsed; this is the
        // observable populated end-state.)
        void load()
        {
            if (!items_->empty())
            {
                return; // already loaded — keep it idempotent
            }
            for (const demo_item& item : source_items())
            {
                items_->add(item);
            }
        }
        // Back to the "loading" empty state (replay the load): empty the source so the EmptyView returns.
        void reset()
        {
            items_->clear();
        }

    private:
        // DemoFilteredItemSource().AddItems(50): the default 50-row set, captioned "<image>, <n>" (the
        // load's populated end-state).
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
        maui::controls::grid grid_;
        maui::controls::collection_view list_;
    };
} // namespace maui::samples
