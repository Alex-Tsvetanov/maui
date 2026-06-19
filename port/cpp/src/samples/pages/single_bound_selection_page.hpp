#pragma once
// maui::samples::single_bound_selection_page — ports SingleBoundSelection.xaml (+ .xaml.cs)
//   (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.SingleBoundSelection).
//
// The C# page is a two-way-bound single-selection test: a BoundSelectionModel exposes Items + a TwoWay
// SelectedItem; the StackLayout shows an instructions Label, a "Selected: {SelectedItem}" readout Label,
// a Reset button (SelectedItem = Items[0]) and a Clear button (SelectedItem = null), then a CollectionView
// with SelectionMode=Single, ItemsSource={Binding Items}, SelectedItem={Binding SelectedItem}. The frame
// cell carries a Selected VisualState (RoyalBlue background + Scale 0.9). The point of the test: whatever
// is selected in the CollectionView must equal the readout Label, in BOTH directions — tapping a cell
// updates the binding, and Reset/Clear push the binding back into the CollectionView.
//
// This headless port owns its whole tree and reproduces both directions of that single binding:
//   - VIEW -> readout: selection_changed feeds update_readout (the "Selected: {0}" label), so a cell
//     selection shows up in the readout — the forward leg of the TwoWay binding;
//   - readout -> VIEW: reset_selection()/clear_selection() set the collection_view's SelectedItem (the C#
//     Reset/Clear button handlers), and because they go through set_selected_item the readout is refreshed
//     from the same boxed value — the back leg.
// The model is a small `country` struct (Caption + Image path) standing in for the C# item type the
// BoundSelectionModel.Items hold; selection equality is value equality over the struct (boxed_item::of<T>
// uses operator==), which is what makes Reset's "SelectedItem == Items[0]" hold.
//
// note: the C# BoundSelectionModel item type is defined elsewhere in the gallery; it exposes Image +
//   Caption (the cell binds both), so the port models exactly those two fields. The Frame cell's Selected
//   VisualState (RoyalBlue / Scale 0.9) is visual chrome the headless sim does not render — the selection
//   STATE it reflects is exercised here through selection_mode::single + the readout instead (the visual
//   state itself lives in the ported VSM tests, not this gallery page). The C# SelectedItem binding is
//   TwoWay; the port realizes the two legs explicitly (event forward, setter back) since there is no live
//   XAML binding engine wired into this gallery page.

#include <memory>
#include <string>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/selection_changed_event_args.hpp"
#include "maui/controls/items/selection_mode.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class single_bound_selection_page
    {
    public:
        // The bound item (BoundSelectionModel.Items element): the two fields the C# Frame cell binds.
        struct country
        {
            std::string caption;
            std::string image;

            friend bool operator==(const country& left, const country& right)
            {
                return left.caption == right.caption && left.image == right.image;
            }
        };

        single_bound_selection_page()
            : items_(std::make_shared<maui::core::observable_collection<country>>(std::vector<country>{
                  {"United States", "united_states.png"},
                  {"Canada", "canada.png"},
                  {"Mexico", "mexico.png"},
                  {"Brazil", "brazil.png"},
                  {"Argentina", "argentina.png"},
              }))
        {
            page_.set_title("Single Bound Selection");
            stack_.set_spacing(5);

            instructions_.set_text("The selected item in the CollectionView should match the 'Selected' Label "
                                   "below. If it does not, this test has failed.");

            // The C# Frame cell binds Image + Caption; the port's headless cell is the Caption label
            // (Text bound to country.caption) — the displayable signal the readout is compared against.
            auto cell = maui::controls::data_template::of<maui::controls::label>();
            cell->set_binding<std::string, country>(maui::controls::label::text_property(),
                                                    [](const country& value) { return value.caption; });
            list_.set_item_template(cell);
            list_.set_items_source(items_);
            list_.set_selection_mode(maui::controls::selection_mode::single); // SelectionMode="Single"
            list_.selection_changed.connect(
                [this](const maui::controls::selection_changed_event_args& args) { on_selection_changed(args); });

            update_readout(list_.selected_item()); // initial "Selected: " (nothing selected yet)

            stack_.add(instructions_);
            stack_.add(readout_);
            stack_.add(list_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // ResetClicked: SelectedItem = Items[0] (the back leg of the TwoWay binding).
        void reset_selection()
        {
            if (!items_->empty())
            {
                set_selected(maui::controls::boxed_item::of(items_->at(0)));
            }
        }

        // ClearClicked: SelectedItem = null.
        void clear_selection()
        {
            set_selected(maui::controls::boxed_item{});
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (leaves first, the page last), then re-host the
        // tree built in the ctor (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, list_, "list_");
            gallery_attach_one(app, readout_, "readout_");
            gallery_attach_one(app, instructions_, "instructions_");
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
        [[nodiscard]] maui::controls::label& instructions()
        {
            return instructions_;
        }
        [[nodiscard]] const std::shared_ptr<maui::core::observable_collection<country>>& items() const
        {
            return items_;
        }

    private:
        // VIEW -> readout (the forward leg): a cell selection updates the bound readout.
        void on_selection_changed(const maui::controls::selection_changed_event_args& args)
        {
            update_readout(args.current_selection.empty() ? maui::controls::boxed_item{}
                                                          : args.current_selection.front());
        }

        // readout -> VIEW (the back leg): push the value into the collection_view, then refresh the readout
        // from the same boxed item (set_selected_item raises selection_changed, but refreshing here keeps
        // the readout correct even when the new value equals the old and no change is signalled).
        void set_selected(maui::controls::boxed_item value)
        {
            list_.set_selected_item(value);
            update_readout(value);
        }

        void update_readout(const maui::controls::boxed_item& selected)
        {
            std::string caption = "(none)";
            if (const auto value = selected.as<country>())
            {
                caption = value->caption;
            }
            readout_.set_text("Selected: " + caption);
        }

        std::shared_ptr<maui::core::observable_collection<country>> items_; // publisher before the list (§8)
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label instructions_;
        maui::controls::label readout_;
        maui::controls::collection_view list_;
    };
} // namespace maui::samples
