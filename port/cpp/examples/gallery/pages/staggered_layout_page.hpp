#pragma once
// maui::samples::staggered_layout_page — ports AlternateLayoutGalleries/StaggeredLayout.xaml
//   (+ StaggeredLayout.xaml.cs) of the C# CollectionView gallery.
//
// The C# page is a STUB-IN-PROGRESS: the whole CollectionView is COMMENTED OUT in both files. The
// commented XAML sketches a `local:StaggeredCollectionView` (a `class StaggeredCollectionView :
// CollectionView {}` — the only uncommented C# type) whose ItemsLayout is a never-shipped
// `local:StaggeredGridItemsLayout Span="3" Orientation="Vertical" HorizontalItemSpacing="5"
// VerticalItemSpacing="5"`, and the commented ctor would set CV.ItemTemplate =
// ExampleTemplates.RandomSizeTemplate() over CV.ItemsSource = _demoFilteredItemSource.Items.
//
// So there is no real staggered layout to port — StaggeredGridItemsLayout was never implemented in
// MAUI (the class is commented out, deriving only sketched ctors from GridItemsLayout). This port
// therefore reproduces the SKETCHED INTENT with the closest SHIPPED layout the C# comment itself
// falls back to: a GridItemsLayout carrying the exact commented attributes (Span=3, Vertical,
// Horizontal+Vertical ItemSpacing=5), over a RandomSizeTemplate-like source of items that REQUEST
// VARYING HEIGHTS (the "staggered" signal — RandomSizeTemplate gives each cell a random height). The
// adaptive_collection_page is the structural sibling (one collection_view + a grid_items_layout); this
// page mirrors it and adds the per-item varied height the staggered demo is about.
//
// note: a true staggered/masonry layout (variable-height cells packed shortest-column-first) is NOT
//       ported — StaggeredGridItemsLayout does not exist in MAUI either, so there is nothing to derive
//       behavior from. The GridItemsLayout(Span=3) is the documented best-effort stand-in straight out
//       of the C# comment's own base class; the varied per-item HeightRequest reproduces what
//       ExampleTemplates.RandomSizeTemplate() does to each cell (the visible "staggered" effect a real
//       masonry layout would then pack). The headless virtualization sim has no geometric pack pass, so
//       Span / spacing / per-cell height have no visible arrange effect in a static capture — the layout
//       object's Span(3)/spacing(5) are stored on the bindable layout and the cell's bound HeightRequest
//       is staged on the template (both would drive a real backend); the bound caption text still
//       surfaces per cell.

#include <memory>
#include <string>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/core/safe_area_edges.hpp"
#include "maui/core/safe_area_regions.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/grid_items_layout.hpp"
#include "maui/controls/items/items_layout_orientation.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/view.hpp"
#include "maui/core/observable_collection.hpp"

namespace maui::samples
{
    class staggered_layout_page
    {
    public:
        // The commented XAML's StaggeredGridItemsLayout attributes, carried verbatim onto the shipped
        // GridItemsLayout stand-in (header note).
        static constexpr int staggered_span = 3;              // Span="3"
        static constexpr double staggered_item_spacing = 5.0; // Horizontal/VerticalItemSpacing="5"

        // One row of the RandomSizeTemplate-like source: a caption + the per-cell height the
        // RandomSizeTemplate would assign (the "staggered" signal — deterministic here, not Random(),
        // so cells are predictable, the port's seeding convention).
        struct random_size_item
        {
            std::string caption;
            double height = 0;
            friend bool operator==(const random_size_item&, const random_size_item&) = default;
        };

        staggered_layout_page()
            : items_(std::make_shared<maui::core::observable_collection<random_size_item>>(source_items()))
        {
            page_.set_title("Staggered Layout");

            // ---- the ItemsLayout: the GridItemsLayout stand-in carrying the commented StaggeredGrid
            // attributes (Span=3, Vertical, Horizontal+Vertical ItemSpacing=5) — header note ----
            auto layout = std::make_shared<maui::controls::grid_items_layout>(
                staggered_span, maui::controls::items_layout_orientation::vertical);
            layout->set_horizontal_item_spacing(staggered_item_spacing);
            layout->set_vertical_item_spacing(staggered_item_spacing);
            list_.set_items_layout(layout);

            // ---- the item template: a plain caption Label, exactly what the canonical shared
            // staggered_layout.xaml declares (`<Label Text="{Binding .}"/>`). The C# RandomSizeTemplate's
            // per-item randomized HeightRequest is the page's real subject, but the shared twin CANNOT
            // express a per-item bound height statically (its x:Array-of-string source carries no height
            // field), so the MAUI ground-truth capture shows compact uniform label rows — and an earlier
            // builder cut that staged varied heights via a bound HeightRequest rendered ~4-6x taller rows
            // than MAUI, a standing red. The varied-height scenario belongs in the P3 gap corpus
            // (a gap_*.xaml probing bound HeightRequest in a DataTemplate); at rest both frameworks now
            // match the twin. The items_ source keeps its height field for that future scenario. ----
            auto cell = maui::controls::data_template::of<maui::controls::label>();
            cell->set_binding<std::string, random_size_item>(maui::controls::label::text_property(),
                                                             [](const random_size_item& item) { return item.caption; });
            list_.set_item_template(cell);

            // ---- CV.ItemsSource = _demoFilteredItemSource.Items (the commented ctor line) ----
            list_.set_items_source(items_);

            // page-direct CollectionView bypasses the layout safe-area inset (+ .Never) so its
            // content would render under the notch/status-bar cutout; inset the page content below the
            // container safe area (mirrors the shared XAML ContentPage SafeAreaEdges="Container").
            page_.set_safe_area_edges(maui::core::safe_area_edges{maui::core::safe_area_regions::container});
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
        [[nodiscard]] const std::shared_ptr<maui::core::observable_collection<random_size_item>>& items() const
        {
            return items_;
        }

    private:
        // A RandomSizeTemplate-like source: 24 captioned cells whose heights step through a small set of
        // sizes (the deterministic stand-in for RandomSizeTemplate's random per-cell height — header note).
        [[nodiscard]] static std::vector<random_size_item> source_items()
        {
            static const std::vector<double> heights{60.0, 120.0, 90.0, 150.0, 75.0, 110.0};
            std::vector<random_size_item> rows;
            for (int n = 0; n < 24; ++n)
            {
                rows.push_back(random_size_item{"Item " + std::to_string(n),
                                                heights[static_cast<std::size_t>(n) % heights.size()]});
            }
            return rows;
        }

        std::shared_ptr<maui::core::observable_collection<random_size_item>> items_; // publisher before the list (§8)
        maui::controls::content_page page_;
        maui::controls::collection_view list_;
    };
} // namespace maui::samples
