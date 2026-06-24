#pragma once
// maui::samples::preselected_items_page — ports PreselectedItemsGallery.xaml (+ .xaml.cs)
//   (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.PreselectedItemsGallery).
//
// The C# page demonstrates a CollectionView that already has several items selected at startup. The XAML
// is minimal — a StackLayout (Margin 10) with an instructions Label and a CollectionView (Header="This is
// the header", GridItemsLayout Span="4" Vertical). The code-behind does the work:
//   - ItemTemplate = ExampleTemplates.PhotoTemplate()  (Image.Source ← Image, Label.Text ← Caption)
//   - ItemsSource  = DemoFilteredItemSource.Items       (50 items, captions "{image}, {n}")
//   - SelectedItems.Add(Items.Skip(2).First()), Skip(4).First(), Skip(5).First()  → preselect items
//     at indices 2, 4, 5 — added BEFORE SelectionMode is set
//   - SelectionMode = SelectionMode.Multiple
// The point: when the page appears, those three items are already in the selection.
//
// This headless port owns its whole tree and reproduces the preselection against the real
// selectable_items_view surface: it builds the 50-item source, sets the grid layout (Span 4) and the
// Caption cell, preloads list_.selected_items() with items 2/4/5, then switches to selection_mode::
// multiple — the exact code-behind ordering. A readout reports the preselected captions so the
// "already selected" state is observable in the headless sim (the C# page shows it only via the cells'
// selected visual, which the headless backend has no analog for — see note).
//
// note: the C# Add-before-SelectionMode ordering is preserved verbatim; the port's selection_list accepts
//   the adds while the mode is still the default (none) and they survive the switch to multiple (no
//   single-mode coercion trims them, since the items were added as a batch and the mode goes straight to
//   multiple — mirroring C#). The PhotoTemplate's Image is visual chrome the headless cell drops (the
//   Caption label is the displayable signal); the per-cell "selected" visual state has no headless
//   analog, so the preselection is surfaced through the readout instead.

#include <memory>
#include <string>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/grid_items_layout.hpp"
#include "maui/controls/items/items_layout_orientation.hpp"
#include "maui/controls/items/selection_changed_event_args.hpp"
#include "maui/controls/items/selection_list.hpp"
#include "maui/controls/items/selection_mode.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/observable_collection.hpp"

namespace maui::samples
{
    class preselected_items_page
    {
    public:
        // The DemoFilteredItemSource element (CollectionViewGalleryTestItem). PhotoTemplate binds Image +
        // Caption; the headless cell surfaces Caption. Value equality drives selection membership.
        struct photo_item
        {
            std::string caption;
            std::string image;

            friend bool operator==(const photo_item& left, const photo_item& right)
            {
                return left.caption == right.caption && left.image == right.image;
            }
        };

        preselected_items_page() : items_(std::make_shared<maui::core::observable_collection<photo_item>>())
        {
            page_.set_title("Preselected Items");
            stack_.set_spacing(2);

            instructions_.set_text("The CollectionView below should have several items already selected.");

            build_items(); // DemoFilteredItemSource: 50 captioned items

            // ItemTemplate = ExampleTemplates.PhotoTemplate(): Image.Source ← Image, Label.Text ← Caption.
            // The headless cell is the Caption label (Image is visual chrome the sim drops).
            auto cell = maui::controls::data_template::of<maui::controls::label>();
            cell->set_binding<std::string, photo_item>(maui::controls::label::text_property(),
                                                       [](const photo_item& value) { return value.caption; });
            list_.set_item_template(cell);
            list_.set_items_layout(std::make_shared<maui::controls::grid_items_layout>(
                4, maui::controls::items_layout_orientation::vertical)); // GridItemsLayout Span="4"
            list_.set_header(maui::controls::boxed_item::of(std::string{"This is the header"})); // Header=…
            list_.set_items_source(items_);

            // Code-behind: preselect items 2, 4, 5 (Skip(2/4/5).First()) BEFORE setting the mode, then
            // switch to Multiple — verbatim ordering.
            list_.selected_items().add(maui::controls::boxed_item::of(items_->at(2)));
            list_.selected_items().add(maui::controls::boxed_item::of(items_->at(4)));
            list_.selected_items().add(maui::controls::boxed_item::of(items_->at(5)));
            list_.set_selection_mode(maui::controls::selection_mode::multiple); // SelectionMode = Multiple

            // Surface the preselection in the headless sim (the C# "already selected" state).
            list_.selection_changed.connect(
                [this](const maui::controls::selection_changed_event_args&) { update_readout(); });
            update_readout(); // initial readout: the three preselected captions

            stack_.add(instructions_);
            stack_.add(readout_);
            stack_.add(list_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
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
        [[nodiscard]] maui::controls::label& instructions()
        {
            return instructions_;
        }
        [[nodiscard]] const std::shared_ptr<maui::core::observable_collection<photo_item>>& items() const
        {
            return items_;
        }

    private:
        void build_items()
        {
            static const char* const images[] = {"cover1.jpg", "oasis.jpg",      "photo.jpg",  "Vegetables.jpg",
                                                 "Fruits.jpg", "FlowerBuds.jpg", "Legumes.jpg"};
            constexpr int count = 50;
            constexpr int image_count = static_cast<int>(std::size(images));
            for (int n = 0; n < count; ++n)
            {
                const std::string image = images[n % image_count];
                items_->add(photo_item{image + ", " + std::to_string(n), image}); // Caption = "{image}, {n}"
            }
        }

        // Join the selected items' Captions (the "already selected" readout); "(none)" when empty.
        void update_readout()
        {
            std::string joined;
            for (const maui::controls::boxed_item& item : list_.selected_items().items())
            {
                if (const auto value = item.as<photo_item>())
                {
                    if (!joined.empty())
                    {
                        joined += ", ";
                    }
                    joined += value->caption;
                }
            }
            readout_.set_text("Preselected: " + (joined.empty() ? std::string{"(none)"} : joined));
        }

        std::shared_ptr<maui::core::observable_collection<photo_item>> items_; // publisher before the list (§8)
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label instructions_;
        maui::controls::label readout_;
        maui::controls::collection_view list_;
    };
} // namespace maui::samples
