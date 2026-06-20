#pragma once
// maui::samples::multiple_bound_selection_page — ports MultipleBoundSelection.xaml (+ .xaml.cs)
//   (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.MultipleBoundSelection).
//
// The C# page is a two-way-bound MULTIPLE-selection test. A BoundSelectionModel exposes Items (four
// CollectionViewGalleryTestItem, captions "Item 0".."Item 3") and a bound SelectedItems
// ObservableCollection<object>; the CollectionView carries SelectionMode="Multiple",
// ItemsSource={Binding Items}, SelectedItems={Binding SelectedItems}, Header="This is the header".
// A StackLayout shows an instructions Label, a readout Label bound to SelectedItemsText
// (StringFormat "Selected: {0}", where SelectedItemsText is the SelectedItems captions joined by ", "),
// and three buttons:
//   - ClearAndAdd:    VM SelectedItems.Clear(); add Items[1], Items[2]   (mutate the bound collection)
//   - ResetClicked:   VM SelectedItems = new ObservableCollection<object>{ Items[1], Items[2] } (REPLACE
//                     the bound collection with a fresh one)
//   - DirectUpdate:   CollectionView.SelectedItems.Clear(); add Items[0], Items[3] (mutate the VIEW's
//                     selection_list directly, not the VM's collection)
// The point of the test: the CollectionView's selected items must ALWAYS match the "Selected:" readout,
// whichever path mutated the selection — through the bound VM collection or through the view directly.
//
// This headless port owns its whole tree and reproduces all three mutation paths against the real
// selectable_items_view surface:
//   - the bound VM collection is a core::observable_collection<boxed_item> wired in via
//     set_selected_items(...) (the C# SelectedItems={Binding SelectedItems}); mutating THAT collection
//     (clear_and_add / direct VM edits) raises a selection change through the selection_list's external
//     subscription, exactly like the C# SelectedItems.CollectionChanged feed;
//   - reset_selection() calls set_selected_items(new collection) — the C# "= new ObservableCollection"
//     setter (wrap + re-subscribe + notify);
//   - direct_update() mutates list_.selected_items() (the view's own selection_list) — the C#
//     CollectionView.SelectedItems.Clear()/Add() path.
// In every path the readout is refreshed from list_.selected_items() (the same source the C#
// SelectedItemsText reads), so the readout and the view selection stay in lockstep — the test's invariant.
//
// note: the C# Frame cell binds Image + Caption; this headless cell is the Caption label (the displayable
//   signal the readout joins). The C# readout reads the VM's SelectedItemsText (the VM collection's
//   captions); here every path keeps the view's selection_list and the VM collection equal, so the
//   readout is sourced from list_.selected_items() — the view-side truth the test compares against.

#include <memory>
#include <string>
#include <vector>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/selection_changed_event_args.hpp"
#include "maui/controls/items/selection_list.hpp"
#include "maui/controls/items/selection_mode.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class multiple_bound_selection_page
    {
    public:
        // The bound item (BoundSelectionModel.Items element ~ CollectionViewGalleryTestItem). The cell
        // binds Caption; ToCommaSeparatedList joins the Captions, so the port models exactly Caption +
        // Image. Value equality drives selection membership (boxed_item::of<T> uses operator==).
        struct test_item
        {
            std::string caption;
            std::string image;

            friend bool operator==(const test_item& left, const test_item& right)
            {
                return left.caption == right.caption && left.image == right.image;
            }
        };

        multiple_bound_selection_page()
            : items_(std::make_shared<maui::core::observable_collection<test_item>>(std::vector<test_item>{
                  // BoundSelectionModel: 4 items, captions "Item {n}", image "coffee.png".
                  {"Item 0", "coffee.png"},
                  {"Item 1", "coffee.png"},
                  {"Item 2", "coffee.png"},
                  {"Item 3", "coffee.png"},
              })),
              // The VM-bound SelectedItems collection: the C# BoundSelectionModel seeds it with
              // { Items[1], Items[2] } in its ctor.
              vm_selected_items_(std::make_shared<maui::core::observable_collection<maui::controls::boxed_item>>(
                  std::vector<maui::controls::boxed_item>{
                      maui::controls::boxed_item::of(items_->at(1)),
                      maui::controls::boxed_item::of(items_->at(2)),
                  }))
        {
            page_.set_title("Multiple Bound Selection");
            stack_.set_spacing(2);

            instructions_.set_text("The selected items in the CollectionView should always match the "
                                   "'Selected' Label below. If it does not, this test has failed.");

            // The cell: Caption label (Image+Caption in C#; Caption is the readout-comparable signal).
            auto cell = maui::controls::data_template::of<maui::controls::label>();
            cell->set_binding<std::string, test_item>(maui::controls::label::text_property(),
                                                      [](const test_item& value) { return value.caption; });
            list_.set_item_template(cell);
            list_.set_header(maui::controls::boxed_item::of(std::string{"This is the header"})); // Header=…
            list_.set_selection_mode(maui::controls::selection_mode::multiple);                  // Multiple
            list_.set_items_source(items_);
            // SelectedItems="{Binding SelectedItems}" — wire the VM-bound collection into the view.
            list_.set_selected_items(vm_selected_items_);

            // Any selection change (VM-collection edits OR direct view edits) refreshes the readout from
            // the view's selection_list — the test's "view == readout" invariant.
            list_.selection_changed.connect(
                [this](const maui::controls::selection_changed_event_args&) { update_readout(); });

            update_readout(); // initial "Selected: Item 1, Item 2" (the seeded VM selection)

            // The three action buttons (C# ClearAndAdd / ResetClicked / DirectUpdateClicked), each driving
            // the matching mutation path so the view selection and the "Selected:" readout stay in lockstep.
            clear_and_add_btn_.set_text("Clear VM selection and add Items 1 and 2");
            clear_and_add_btn_.clicked.connect([this] { clear_and_add(); });
            reset_btn_.set_text("Set VM selection to new list");
            reset_btn_.clicked.connect([this] { reset_selection(); });
            direct_update_btn_.set_text("Clear CV selection and add Items 0 and 3");
            direct_update_btn_.clicked.connect([this] { direct_update(); });

            stack_.add(instructions_);
            stack_.add(readout_);
            stack_.add(list_);
            stack_.add(clear_and_add_btn_); // C# order: instructions, readout, list, then the three buttons
            stack_.add(reset_btn_);
            stack_.add(direct_update_btn_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // ClearAndAdd: VM SelectedItems.Clear(); add Items[1], Items[2] — mutate the BOUND VM collection.
        // The view's selection_list is subscribed to this collection, so the change feeds back into the
        // view (and the readout) exactly like the C# CollectionChanged path.
        void clear_and_add()
        {
            vm_selected_items_->clear();
            vm_selected_items_->add(maui::controls::boxed_item::of(items_->at(1)));
            vm_selected_items_->add(maui::controls::boxed_item::of(items_->at(2)));
            update_readout();
        }

        // ResetClicked: VM SelectedItems = new ObservableCollection<object>{ Items[1], Items[2] } —
        // REPLACE the bound collection with a fresh one (set_selected_items re-wraps + re-subscribes).
        void reset_selection()
        {
            vm_selected_items_ = std::make_shared<maui::core::observable_collection<maui::controls::boxed_item>>(
                std::vector<maui::controls::boxed_item>{
                    maui::controls::boxed_item::of(items_->at(1)),
                    maui::controls::boxed_item::of(items_->at(2)),
                });
            list_.set_selected_items(vm_selected_items_);
            update_readout();
        }

        // DirectUpdateClicked: CollectionView.SelectedItems.Clear(); add Items[0], Items[3] — mutate the
        // VIEW's own selection_list directly (not the VM collection).
        void direct_update()
        {
            list_.selected_items().clear();
            list_.selected_items().add(maui::controls::boxed_item::of(items_->at(0)));
            list_.selected_items().add(maui::controls::boxed_item::of(items_->at(3)));
            update_readout();
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (leaves first, the page last), then re-host the
        // tree built in the ctor (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, list_, "list_");
            gallery_attach_one(app, readout_, "readout_");
            gallery_attach_one(app, instructions_, "instructions_");
            gallery_attach_one(app, clear_and_add_btn_, "clear_and_add_btn_");
            gallery_attach_one(app, reset_btn_, "reset_btn_");
            gallery_attach_one(app, direct_update_btn_, "direct_update_btn_");
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
        [[nodiscard]] const std::shared_ptr<maui::core::observable_collection<test_item>>& items() const
        {
            return items_;
        }

    private:
        // SelectedItemsText: join each selected item's Caption with ", " (SelectionHelpers
        // .ToCommaSeparatedList over CollectionViewGalleryTestItem.Caption), sourced from the view's
        // selection_list — the truth the test compares against.
        void update_readout()
        {
            std::string joined;
            for (const maui::controls::boxed_item& item : list_.selected_items().items())
            {
                if (const auto value = item.as<test_item>())
                {
                    if (!joined.empty())
                    {
                        joined += ", ";
                    }
                    joined += value->caption;
                }
            }
            readout_.set_text("Selected: " + joined); // StringFormat="Selected: {0}"
        }

        std::shared_ptr<maui::core::observable_collection<test_item>> items_; // publisher before the list (§8)
        std::shared_ptr<maui::core::observable_collection<maui::controls::boxed_item>> vm_selected_items_;
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label instructions_;
        maui::controls::label readout_;
        maui::controls::collection_view list_;
        maui::controls::button clear_and_add_btn_; // C# ClearAndAdd
        maui::controls::button reset_btn_;         // C# ResetClicked
        maui::controls::button direct_update_btn_; // C# DirectUpdateClicked
    };
} // namespace maui::samples
