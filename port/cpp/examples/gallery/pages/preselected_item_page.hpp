#pragma once
// maui::samples::preselected_item_page — ports PreselectedItemGallery.xaml (+ .xaml.cs)
//   (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.PreselectedItemGallery).
//
// The C# page demonstrates a CollectionView that already has a SINGLE item selected at startup. The XAML is a
// Grid (rows Auto/Auto/Auto/*) with two empty diagnostic Labels (SelectedItemsEvent row 1, SelectedItemsCommand
// row 2 — wired by other galleries, left blank here) and a CollectionView (row 3, Header="This is the header").
// The code-behind does the work:
//   - ItemTemplate = ExampleTemplates.PhotoTemplate()   (Image.Source ← Image, Label.Text ← Caption)
//   - ItemsSource  = DemoFilteredItemSource.Items        (50 items, captions "{image}, {n}")
//   - SelectedItem = Items.Skip(2).First()               → preselect the item at index 2
//   - SelectionMode = SelectionMode.Single
// The point: when the page appears, the index-2 item is already the single selection.
//
// This is the single-selection sibling of preselected_items_page (which preselects three items in Multiple
// mode). This headless port owns its whole tree and reproduces the single preselection against the real
// selectable_items_view: it builds the 50-item source, sets the Caption cell + the "This is the header"
// header, then — matching the code-behind ordering — sets SelectedItem = Items[2] and switches SelectionMode to
// Single. A readout reports the preselected caption so the "already selected" state is observable in the
// headless sim (the C# page shows it only via the cell's selected visual, which the headless backend has no
// analog for — see note).
//
// note: the two diagnostic Labels (SelectedItemsEvent / SelectedItemsCommand) are left blank, exactly as this
//   gallery leaves them (they are populated by event/command variants in sibling galleries, not by this page).
//   The PhotoTemplate's Image is visual chrome the headless cell drops (the Caption label is the displayable
//   signal). The per-cell "selected" visual state has no headless analog, so the preselection is surfaced
//   through the readout instead. The code-behind sets SelectedItem BEFORE SelectionMode = Single; the port
//   preserves that order — set_selected_item lands the single selection, and the switch to single mode keeps
//   it (a single value is already single-mode-legal, so no coercion trims it).

#include <memory>
#include <string>

#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/selection_changed_event_args.hpp"
#include "maui/controls/items/selection_mode.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/view.hpp" // maui::controls::margin_property()
#include "maui/core/grid_length.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/core/thickness.hpp"

namespace maui::samples
{
    class preselected_item_page
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

        preselected_item_page() : items_(std::make_shared<maui::core::observable_collection<photo_item>>())
        {
            page_.set_title("Preselected Item");

            build_items(); // DemoFilteredItemSource: 50 captioned items

            // ItemTemplate = ExampleTemplates.PhotoTemplate(): Image.Source ← Image, Label.Text ← Caption.
            // The headless cell is the Caption label (Image is visual chrome the sim drops).
            auto cell = maui::controls::data_template::of<maui::controls::label>();
            cell->set_binding<std::string, photo_item>(maui::controls::label::text_property(),
                                                       [](const photo_item& value) { return value.caption; });
            cell->set_value(maui::controls::margin_property(),
                            maui::core::thickness{6}); // <Label Margin="6"> (shared XAML)
            list_.set_item_template(cell);
            list_.set_header(maui::controls::boxed_item::of(std::string{"This is the header"})); // Header=…
            list_.set_items_source(items_);

            // Code-behind: SelectedItem = Items.Skip(2).First() (index 2) BEFORE SelectionMode = Single.
            list_.set_selected_item(maui::controls::boxed_item::of(items_->at(2)));
            list_.set_selection_mode(maui::controls::selection_mode::single); // SelectionMode = Single

            // Surface the preselection in the headless sim (the C# "already selected" state).
            list_.selection_changed.connect(
                [this](const maui::controls::selection_changed_event_args&) { update_readout(); });
            update_readout(); // initial readout: the preselected caption

            // The two diagnostic Labels are present in the XAML grid but left blank by this gallery.
            selected_items_event_.set_text("");
            selected_items_command_.set_text("");

            // ---- assemble the Grid (Auto / Auto / Auto / *) ----
            grid_.add_row_definition(maui::core::grid_length::automatic());
            grid_.add_row_definition(maui::core::grid_length::automatic());
            grid_.add_row_definition(maui::core::grid_length::automatic());
            grid_.add_row_definition(maui::core::grid_length::star());
            grid_.add(readout_);
            grid_.set_row(readout_, 0); // the readout occupies row 0 (the XAML leaves row 0 free)
            grid_.add(selected_items_event_);
            grid_.set_row(selected_items_event_, 1);
            grid_.add(selected_items_command_);
            grid_.set_row(selected_items_command_, 2);
            grid_.add(list_);
            grid_.set_row(list_, 3);

            page_.set_content(grid_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment / any test tree.
        [[nodiscard]] maui::controls::grid& grid()
        {
            return grid_;
        }
        [[nodiscard]] maui::controls::collection_view& list()
        {
            return list_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
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

        // The single preselected item's Caption (the "already selected" readout); "(none)" when empty.
        void update_readout()
        {
            std::string caption = "(none)";
            if (const auto value = list_.selected_item().as<photo_item>())
            {
                caption = value->caption;
            }
            readout_.set_text("Preselected: " + caption);
        }

        std::shared_ptr<maui::core::observable_collection<photo_item>> items_; // publisher before the list (§8)
        maui::controls::content_page page_;
        maui::controls::grid grid_;
        maui::controls::label readout_;
        maui::controls::label selected_items_event_;   // XAML SelectedItemsEvent (left blank by this gallery)
        maui::controls::label selected_items_command_; // XAML SelectedItemsCommand (left blank by this gallery)
        maui::controls::collection_view list_;
    };
} // namespace maui::samples
