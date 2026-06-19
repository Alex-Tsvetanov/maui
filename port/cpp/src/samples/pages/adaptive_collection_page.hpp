#pragma once
// maui::samples::adaptive_collection_page — ports AdaptiveCollectionView.xaml (+ .xaml.cs)
//   (Maui.Controls.Sample.Pages.CollectionViewGalleries.AdaptiveCollectionView).
//
// The C# page hosts a CollectionView over a fixed x:Array of eight strings ("Item 1".."Item 8") with a
// HeightRequest=60 / centered-Label cell, and in OnAppearing subscribes SizeChanged to flip the
// ItemsLayout: Width > 600 -> GridItemsLayout(3, Vertical), else LinearItemsLayout(Vertical). That is the
// whole "adaptive" behavior — one collection_view whose items_layout swaps between a single column and a
// three-across grid as the available width crosses 600.
//
// This headless port owns its whole tree (the items_page pattern) and reproduces the adaptive swap as a
// code-first call: there is no live SizeChanged in the headless backend, so apply_width(double) carries the
// EXACT C# decision (Width > 600 ? grid(3) : linear), and use_grid()/use_linear() expose the two endpoints
// directly. A small readout label reports which layout is currently mounted (the demo's only observable
// signal that the swap happened), and the collection_view starts in the < 600 state (linear), matching a
// freshly-appeared narrow page before the first SizeChanged fires.
//
// Interactions demonstrated:
//   - the collection_view renders the eight string items through a recyclable label cell (Text = the item,
//     the C# {Binding} self-binding);
//   - apply_width(w) / use_grid() / use_linear() swap set_items_layout between a vertical
//     linear_items_layout and a grid_items_layout(span 3) — the adaptive core;
//   - the readout label mirrors the mounted layout kind.
//
// note: the C# DataTemplate's HeightRequest=60 / Padding=12 / centered Label is cosmetic chrome; the
//   headless virtualization sim keys on the cell TREE + bindings, so the cell here is the bound label
//   (the items_page precedent). The Width>600 threshold and the GridItemsLayout span of 3 are carried
//   verbatim from AdaptiveCollectionView.xaml.cs.

#include <memory>
#include <string>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/grid_items_layout.hpp"
#include "maui/controls/items/items_layout_orientation.hpp"
#include "maui/controls/items/linear_items_layout.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class adaptive_collection_page
    {
    public:
        // The C# adaptive threshold (AdaptiveCollectionView.xaml.cs: Width > 600).
        static constexpr double adaptive_width_threshold = 600.0;
        // The C# grid span (GridItemsLayout(3, Vertical)).
        static constexpr int adaptive_grid_span = 3;

        adaptive_collection_page()
            : items_(std::make_shared<maui::core::observable_collection<std::string>>(std::vector<std::string>{
                  "Item 1", "Item 2", "Item 3", "Item 4", "Item 5", "Item 6", "Item 7", "Item 8"}))
        {
            page_.set_title("Adaptive CollectionView");
            stack_.set_spacing(12);

            // The C# DataTemplate: a centered Label bound to the string item ({Binding} self-path).
            auto cell = maui::controls::data_template::of<maui::controls::label>();
            cell->set_binding<std::string, std::string>(maui::controls::label::text_property(),
                                                        [](const std::string& value) { return value; });
            list_.set_item_template(cell);
            list_.set_items_source(items_);

            // A freshly-appeared narrow page (Width <= 600) before the first SizeChanged: linear.
            use_linear();

            stack_.add(readout_);
            stack_.add(list_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The C# OnAdaptiveCollectionViewSizeChanged decision, verbatim: Width > 600 -> grid(3), else linear.
        void apply_width(double width)
        {
            if (width > adaptive_width_threshold)
            {
                use_grid();
            }
            else
            {
                use_linear();
            }
        }

        // GridItemsLayout(3, Vertical) — the wide endpoint.
        void use_grid()
        {
            list_.set_items_layout(std::make_shared<maui::controls::grid_items_layout>(
                adaptive_grid_span, maui::controls::items_layout_orientation::vertical));
            readout_.set_text("Layout: Grid (span 3)");
        }

        // LinearItemsLayout(Vertical) — the narrow endpoint.
        void use_linear()
        {
            list_.set_items_layout(std::make_shared<maui::controls::linear_items_layout>(
                maui::controls::items_layout_orientation::vertical));
            readout_.set_text("Layout: Linear (single column)");
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (leaves first, the page last), then re-host the
        // tree built in the ctor (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, list_, "list_");
            gallery_attach_one(app, readout_, "readout_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(stack_);
            gallery_rehost_content(page_);
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::collection_view& list()
        {
            return list_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }
        [[nodiscard]] const std::shared_ptr<maui::core::observable_collection<std::string>>& items() const
        {
            return items_;
        }

    private:
        std::shared_ptr<maui::core::observable_collection<std::string>> items_; // publisher before the list (§8)
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label readout_;
        maui::controls::collection_view list_;
    };
} // namespace maui::samples
