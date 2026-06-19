#pragma once
// maui::samples::selection_mode_page — ports SelectionGalleries/SelectionModeGallery.xaml (+ .xaml.cs)
// of the C# CollectionView gallery.
//
// The original page (SelectionModeGallery): a Grid with three readout Labels and a CollectionView
// (GridItemsLayout Span="3", Header="This is the header") whose ItemsSource is a DemoFilteredItemSource.
// The code-behind adds an EnumSelector<SelectionMode> control (a None/Single/Multiple toggle wired to
// CollectionView.SelectionMode) and then surfaces selection two ways:
//   - SelectionChanged event → UpdateSelectionInfo(current, previous): writes "Selection (event): …"
//     and "Previous (event): …" (each a comma-separated list of the items' Captions, or "[none]");
//   - SelectionChangedCommand → UpdateSelectionInfoCommand(): writes "Selection (command): …" reading
//     SelectedItems (Multiple) or SelectedItem (Single), "[none]" for None.
// The page Resources also define a Grid Selected/Normal visual-state style (LightSkyBlue when
// selected) — a per-cell visual the headless backend has no analog for (note:).
//
// The port mirrors this with the selectable_items_view surface (selectable_items_view.hpp): the
// collection_view already carries selection_mode + selected_item + selected_items + the selection_changed
// EVENT and the selection_changed_command move_only_function (the port's ICommand stand-in), so both
// readout paths port directly:
//   - photo_item: the reflection-free CollectionViewGalleryTestItem (just the Caption surfaced here);
//   - a picker (None/Single/Multiple) stands in for EnumSelector<SelectionMode>: selecting an entry
//     sets list_.set_selection_mode(...) (and re-runs the command readout, like the C# selector);
//   - selection_changed drives the two event labels (UpdateSelectionInfo);
//   - selection_changed_command drives the command label (UpdateSelectionInfoCommand).
//
// The headless collection_view virtualization sim realizes the cells; selection is exercised through
// the selectable_items_view surface (set_selected_item / selected_items()), so the readouts update
// exactly as the C# event/command pair does.

#include <memory>
#include <string>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/grid_items_layout.hpp"
#include "maui/controls/items/items_layout_orientation.hpp"
#include "maui/controls/items/selection_changed_event_args.hpp"
#include "maui/controls/items/selection_list.hpp"
#include "maui/controls/items/selection_mode.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/picker.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class selection_mode_page
    {
    public:
        // The reflection-free CollectionViewGalleryTestItem: just the Caption the readouts join.
        struct photo_item
        {
            std::string caption;
            friend bool operator==(const photo_item&, const photo_item&) = default;
        };

        selection_mode_page() : items_(std::make_shared<maui::core::observable_collection<photo_item>>())
        {
            page_.set_title("Selection Mode");

            build_items(); // DemoFilteredItemSource: 50 captioned items

            // ---- the cell template (caption label) ----
            auto cell = maui::controls::data_template::of<maui::controls::label>();
            cell->set_binding<std::string, photo_item>(maui::controls::label::text_property(),
                                                       [](const photo_item& item) { return item.caption; });
            list_.set_item_template(cell);
            list_.set_items_layout(std::make_shared<maui::controls::grid_items_layout>(
                3, maui::controls::items_layout_orientation::vertical)); // GridItemsLayout Span="3"
            list_.set_header(maui::controls::boxed_item::of(std::string{"This is the header"}));
            list_.set_items_source(items_);

            // ---- the EnumSelector<SelectionMode> stand-in: a None/Single/Multiple picker ----
            mode_picker_.set_title("SelectionMode");
            mode_picker_.items().add("None");
            mode_picker_.items().add("Single");
            mode_picker_.items().add("Multiple");
            mode_picker_.set_selected_index(0); // SelectionMode defaults to None
            mode_picker_.selected_index_changed.connect([this] { on_mode_picked(); });

            // ---- the selection readouts: two from the event, one from the command ----
            list_.selection_changed.connect([this](const maui::controls::selection_changed_event_args& args) {
                update_selection_info(args.current_selection, args.previous_selection);
            });
            list_.selection_changed_command = [this] { update_selection_info_command(); };

            // C# seeds both readouts empty in the ctor.
            update_selection_info({}, {});
            update_selection_info_command();

            // ---- assemble the Grid (Auto×4 / *) ----
            for (int row = 0; row < 4; ++row)
            {
                grid_.add_row_definition(maui::core::grid_length::automatic());
            }
            grid_.add_row_definition(maui::core::grid_length::star());

            grid_.add(mode_picker_); // the selector occupies row 0 (C# adds it to the Grid)
            grid_.set_row(mode_picker_, 0);
            grid_.add(selection_event_);
            grid_.set_row(selection_event_, 1);
            grid_.add(previous_event_);
            grid_.set_row(previous_event_, 2);
            grid_.add(selection_command_);
            grid_.set_row(selection_command_, 3);
            grid_.add(list_);
            grid_.set_row(list_, 4);

            page_.set_content(grid_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, mode_picker_, "mode_picker_");
            gallery_attach_one(app, selection_event_, "selection_event_");
            gallery_attach_one(app, previous_event_, "previous_event_");
            gallery_attach_one(app, selection_command_, "selection_command_");
            gallery_attach_one(app, list_, "list_");
            gallery_attach_one(app, grid_, "grid_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(grid_);  // the grid hosts the picker, the three labels, the list
            gallery_rehost_content(page_); // the page hosts the grid
        }

        // ---- accessors (used by the hosting main + any test tree) ----
        [[nodiscard]] maui::controls::grid& grid()
        {
            return grid_;
        }
        [[nodiscard]] maui::controls::picker& mode_picker()
        {
            return mode_picker_;
        }
        [[nodiscard]] maui::controls::collection_view& list()
        {
            return list_;
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
                items_->add(photo_item{std::string{images[n % image_count]} + ", " + std::to_string(n)});
            }
        }

        // The picker's index maps onto SelectionMode (None / Single / Multiple), then re-runs the command
        // readout — the EnumSelector's set-callback shape.
        void on_mode_picked()
        {
            switch (mode_picker_.selected_index())
            {
                case 1:
                    list_.set_selection_mode(maui::controls::selection_mode::single);
                    break;
                case 2:
                    list_.set_selection_mode(maui::controls::selection_mode::multiple);
                    break;
                default:
                    list_.set_selection_mode(maui::controls::selection_mode::none);
                    break;
            }
            update_selection_info_command();
        }

        // SelectionHelpers.ToCommaSeparatedList: join each item's Caption (cast to the test item),
        // "[none]" when empty — exactly the C# readout text.
        [[nodiscard]] static std::string to_comma_separated_list(const std::vector<maui::controls::boxed_item>& items)
        {
            std::string joined;
            for (const maui::controls::boxed_item& item : items)
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
            return joined.empty() ? std::string{"[none]"} : joined;
        }

        // UpdateSelectionInfo(current, previous): the two event labels.
        void update_selection_info(const std::vector<maui::controls::boxed_item>& current,
                                   const std::vector<maui::controls::boxed_item>& previous)
        {
            selection_event_.set_text("Selection (event): " + to_comma_separated_list(current));
            previous_event_.set_text("Previous (event): " + to_comma_separated_list(previous));
        }

        // UpdateSelectionInfoCommand(): SelectedItems (Multiple) or SelectedItem (Single), "[none]" for
        // None — the command label.
        void update_selection_info_command()
        {
            std::string current = "[none]";
            if (list_.selection_mode() == maui::controls::selection_mode::multiple)
            {
                current = to_comma_separated_list(list_.selected_items().items());
            }
            else if (list_.selection_mode() == maui::controls::selection_mode::single)
            {
                if (const auto value = list_.selected_item().as<photo_item>())
                {
                    current = value->caption;
                }
            }
            selection_command_.set_text("Selection (command): " + current);
        }

        std::shared_ptr<maui::core::observable_collection<photo_item>> items_; // publisher before the list (§8)
        maui::controls::content_page page_;
        maui::controls::grid grid_;
        maui::controls::picker mode_picker_;      // the EnumSelector<SelectionMode> stand-in
        maui::controls::label selection_event_;   // "Selection (event): …"
        maui::controls::label previous_event_;    // "Previous (event): …"
        maui::controls::label selection_command_; // "Selection (command): …"
        maui::controls::collection_view list_;
    };
} // namespace maui::samples
