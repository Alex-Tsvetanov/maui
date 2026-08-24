#pragma once
// maui::samples::selection_synchronization_page — ports SelectionSynchronization.xaml (+ .xaml.cs)
//   (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.SelectionSynchronization).
//
// The C# page stress-tests how a CollectionView synchronizes ItemsSource and Selected* when they are set in
// either order, and when the selection contains items that are / aren't in the source. The BindingContext is a
// SelectionSyncModel:
//   - Items                    = List<string> { "Item 1", "Item 2", "Item 3", "Item 4" }
//   - SelectedItem             = "Item 2"
//   - SelectedItems            = ObservableCollection<object> { "Item 3", "Item 2" }
//   - SelectedItemNotInSource  = "Foo"
//   - SelectedItemsNotInSource = ObservableCollection<object> { "Foo", "Bar", "Baz" }
// A ScrollView/StackLayout holds an instructions Label and NINE CollectionViews (each styled via the "CV" style
// — HeightRequest 250, ItemTemplate a Label bound to "{Binding .}", Margin 5,2,5,5). Each CV pairs a "should
// be selected" Label with one ordering permutation:
//   1. ItemsSource then SelectedItems (Multiple)              → items 2 & 3 selected
//   2. SelectedItems then ItemsSource (Multiple)              → items 2 & 3 selected
//   3. ItemsSource then SelectedItem  (Single)                → item 2 selected
//   4. SelectedItem  then ItemsSource (Single)                → item 2 selected
//   5. SelectedItems (not in source) then ItemsSource (Mult.) → nothing selected
//   6. ItemsSource then SelectedItems (not in source) (Mult.) → nothing selected
//   7. SelectedItem (not in source) then ItemsSource (Single) → nothing selected
//   8. ItemsSource then SelectedItem (not in source) (Single) → nothing selected
//   9. (CVSwitchSource) ItemsSource + SelectedItems (Multiple), with a "Switch Source" Button that swaps the
//      ItemsSource to { "Item -1","Item 0","Item 1","Item 3","Item 4","Item 5" } → after the swap only "Item 3"
//      stays selected (it's the only originally-selected item that survives in the new source).
// The point: a selected item is "selected" only if it is actually present in the source — and that holds in
// either set order, and across an ItemsSource swap.
//
// This headless port owns its whole tree and reproduces every permutation against the real
// selectable_items_view. Each CV is a member; build_* helpers set ItemsSource / Selected* in the documented
// order. Because the headless selection surface only retains a selection that is present in the source (the
// selectable_items_view coerces away phantom selections — the same behavior the C# test asserts), the not-in-
// source cases come out empty and the in-source cases retain only the present items, regardless of set order.
// switch_source() reproduces SwitchSourceClicked: point CV #9 at the new string source; the selection then
// reflects only the surviving "Item 3". A per-CV readout Label reports each CV's current selection so the
// "should be selected" expectation is observable in the headless sim (the C# page shows it via the cells'
// selected visual, which the headless backend has no analog for — see note).
//
// note: the items are plain std::string ("Item N"), matching the C# string source; selection equality is
//   string value equality (boxed_item::of<std::string> uses operator==). The C# Style sets HeightRequest 250
//   (ported via set_height_request) plus Margin="5,2,5,5" (ported via set_margin), and the cell Label's
//   Margin="0,3,0,3" is set on the template. The per-cell "selected" visual state has no headless analog, so
//   each CV's selection is surfaced through its readout instead. The C# model carries the not-in-source
//   selections as separate properties so each can be wired into the right CV; the port mirrors that by seeding
//   each CV from the corresponding fixed values directly.

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
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/core/thickness.hpp"

namespace maui::samples
{
    class selection_synchronization_page
    {
    public:
        selection_synchronization_page()
            // SelectionSyncModel.Items = { "Item 1".."Item 4" }.
            : items_{"Item 1", "Item 2", "Item 3", "Item 4"}
        {
            page_.set_title("Selection Synchronization");
            stack_.set_spacing(2);

            instructions_.set_text("Each CollectionView below sets ItemsSource and Selected* in a particular "
                                   "order. Only selected items that exist in the source should stay selected.");
            stack_.add(instructions_);

            // Build the nine CVs in the same order as the XAML, each preceded by its label pair + a readout.
            // The two-selected, in-source permutations (1 & 2): "Should have items 2 and 3 selected".
            build_multiple(cv1_, &label1_, readout1_, "Set ItemsSource then SelectedItems",
                           /*source_first=*/true, in_source_multi());
            build_multiple(cv2_, &label2_, readout2_, "Set SelectedItems then ItemsSource",
                           /*source_first=*/false, in_source_multi());

            // The one-selected, in-source single permutations (3 & 4): "Should have item 2 selected".
            build_single(cv3_, label3_, readout3_, "Set ItemsSource then SelectedItem",
                         /*source_first=*/true, in_source_single());
            build_single(cv4_, label4_, readout4_, "Set SelectedItem then ItemsSource",
                         /*source_first=*/false, in_source_single());

            // The not-in-source multiple permutations (5 & 6): "Should have nothing selected".
            build_multiple(cv5_, &label5_, readout5_, "Set SelectedItems (not in source) then ItemsSource",
                           /*source_first=*/false, not_in_source_multi());
            build_multiple(cv6_, &label6_, readout6_, "Set ItemsSource then SelectedItems (not in source)",
                           /*source_first=*/true, not_in_source_multi());

            // The not-in-source single permutations (7 & 8): "Should have nothing selected".
            build_single(cv7_, label7_, readout7_, "Set SelectedItem (not in source) then ItemsSource",
                         /*source_first=*/false, not_in_source_single());
            build_single(cv8_, label8_, readout8_, "Set ItemsSource then SelectedItem (not in source)",
                         /*source_first=*/true, not_in_source_single());

            // CV #9 (CVSwitchSource): ItemsSource then SelectedItems (Multiple), then a Switch Source button.
            label9_.set_text("Switch out ItemSource for one with only some of the SelectedItems");
            stack_.add(label9_);
            switch_button_.set_text("Switch Source");
            switch_button_.clicked.connect([this]() { switch_source(); });
            stack_.add(switch_button_);
            label9b_.set_text("After hitting the button, should only have Item 3 selected");
            stack_.add(label9b_);
            build_multiple(cv9_, /*before=*/nullptr, readout9_, /*label_text=*/nullptr,
                           /*source_first=*/true, in_source_multi());

            scroll_.set_content(stack_);
            page_.set_content(scroll_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // SwitchSourceClicked: CVSwitchSource.ItemsSource = { "Item -1","Item 0","Item 1","Item 3","Item 4",
        // "Item 5" }. Only "Item 3" was both originally selected and is present in the new source, so only it
        // stays selected.
        void switch_source()
        {
            cv9_.set_items_source(
                std::vector<std::string>{"Item -1", "Item 0", "Item 1", "Item 3", "Item 4", "Item 5"});
            update_readout(cv9_, readout9_);
        }

        // ---- accessors (used by the hosting main + any test tree) ----
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::scroll_view& scroll()
        {
            return scroll_;
        }
        [[nodiscard]] maui::controls::button& switch_button()
        {
            return switch_button_;
        }
        [[nodiscard]] maui::controls::collection_view& cv_switch_source()
        {
            return cv9_;
        }

    private:
        // The fixed selection seeds, matching the SelectionSyncModel properties.
        [[nodiscard]] static std::vector<std::string> in_source_multi()
        {
            return {"Item 3", "Item 2"}; // SelectedItems = { "Item 3", "Item 2" }
        }
        [[nodiscard]] static std::string in_source_single()
        {
            return "Item 2"; // SelectedItem = "Item 2"
        }
        [[nodiscard]] static std::vector<std::string> not_in_source_multi()
        {
            return {"Foo", "Bar", "Baz"}; // SelectedItemsNotInSource
        }
        [[nodiscard]] static std::string not_in_source_single()
        {
            return "Foo"; // SelectedItemNotInSource
        }

        [[nodiscard]] static std::shared_ptr<maui::controls::data_template> caption_cell()
        {
            // The "CV" style's ItemTemplate: a Label bound to "{Binding .}" (the string item itself).
            auto cell = maui::controls::data_template::of<maui::controls::label>();
            cell->set_binding<std::string, std::string>(maui::controls::label::text_property(),
                                                        [](const std::string& value) { return value; });
            cell->set_value(maui::controls::margin_property(),
                            maui::core::thickness(0, 3, 0, 3)); // the cell Label's Margin="0,3,0,3"
            return cell;
        }

        // Apply the "CV" style essentials a headless CV honors: HeightRequest 250 + the Caption cell template.
        void style_cv(maui::controls::collection_view& cv)
        {
            cv.set_height_request(250);                       // Style Setter HeightRequest="250"
            cv.set_margin(maui::core::thickness(5, 2, 5, 5)); // Style Setter Margin="5,2,5,5"
            cv.set_item_template(caption_cell());
        }

        // Build a Single-selection CV in the documented set order. `before`/`label_text` add the "should be
        // selected" label pair when present; the readout reports the resulting single selection.
        void build_single(maui::controls::collection_view& cv, maui::controls::label& before,
                          maui::controls::label& readout, const char* label_text, bool source_first,
                          const std::string& selected)
        {
            before.set_text(std::string(label_text) + " — should have item 2 selected"
                                                      " (or nothing, if not in source)");
            stack_.add(before);

            cv.set_selection_mode(maui::controls::selection_mode::single);
            style_cv(cv);
            if (source_first)
            {
                cv.set_items_source(items_); // ItemsSource then SelectedItem
                cv.set_selected_item(maui::controls::boxed_item::of(selected));
            }
            else
            {
                cv.set_selected_item(maui::controls::boxed_item::of(selected)); // SelectedItem then ItemsSource
                cv.set_items_source(items_);
            }
            wire_readout(cv, readout);
            stack_.add(readout);
            stack_.add(cv);
        }

        // Build a Multiple-selection CV in the documented set order. A null `before` (CV #9) skips the label
        // pair (CV #9's labels are added by the caller around the Switch Source button).
        void build_multiple(maui::controls::collection_view& cv, maui::controls::label* before,
                            maui::controls::label& readout, const char* label_text, bool source_first,
                            const std::vector<std::string>& selected)
        {
            if (before != nullptr && label_text != nullptr)
            {
                before->set_text(std::string(label_text) + " — should have items 2 and 3 selected"
                                                           " (or nothing, if not in source)");
                stack_.add(*before);
            }

            cv.set_selection_mode(maui::controls::selection_mode::multiple);
            style_cv(cv);

            const auto seed = [&]() {
                std::vector<maui::controls::boxed_item> boxed;
                boxed.reserve(selected.size());
                for (const std::string& value : selected)
                {
                    boxed.push_back(maui::controls::boxed_item::of(value));
                }
                cv.set_selected_items(std::move(boxed));
            };

            if (source_first)
            {
                cv.set_items_source(items_); // ItemsSource then SelectedItems
                seed();
            }
            else
            {
                seed(); // SelectedItems then ItemsSource
                cv.set_items_source(items_);
            }
            wire_readout(cv, readout);
            stack_.add(readout);
            stack_.add(cv);
        }

        void wire_readout(maui::controls::collection_view& cv, maui::controls::label& readout)
        {
            cv.selection_changed.connect([this, &cv, &readout](const maui::controls::selection_changed_event_args&) {
                update_readout(cv, readout);
            });
            update_readout(cv, readout); // initial readout reflecting the seeded selection
        }

        // Join the CV's current selection captions (the "selected" readout); "(none)" when empty — the
        // observable signal for each CV's "should be selected" expectation.
        //
        // cv.selected_item()/selected_items() are the RAW bindable values — set_selected_item(s) never
        // prunes them against the source (matching C#: SelectionList is a plain pass-through, see
        // CoerceSelectedItems/SelectionList.cs, no membership check). What DOES filter by source
        // membership is update_platform_selection (collection_view_handler.cpp) — but it only builds the
        // native paint list (platform->selected_paths); it never writes back to the bindable collection.
        // So a "selected item not in source" scenario leaves the raw value sitting there unchanged while
        // no cell is actually highlighted — which is exactly the state this readout needs to describe (it
        // stands in for that highlight, per the file header), so it has to do its own membership check
        // rather than trust the raw value. Missed this the first time and shipped a readout that echoed
        // the raw "Foo, Bar, Baz" seed straight back, which is what the real device board caught.
        void update_readout(maui::controls::collection_view& cv, maui::controls::label& readout)
        {
            const auto& source = cv.items_source();
            const auto in_source = [&source](const maui::controls::boxed_item& item) {
                return source != nullptr && source->index_of(item) != -1;
            };
            std::string joined;
            // Single-mode CVs surface selection through selected_item; multiple-mode through selected_items.
            if (cv.selection_mode() == maui::controls::selection_mode::single)
            {
                if (const maui::controls::boxed_item& selected = cv.selected_item(); in_source(selected))
                {
                    if (const auto value = selected.as<std::string>())
                    {
                        joined = *value;
                    }
                }
            }
            else
            {
                for (const maui::controls::boxed_item& item : cv.selected_items().items())
                {
                    if (!in_source(item))
                    {
                        continue;
                    }
                    if (const auto value = item.as<std::string>())
                    {
                        if (!joined.empty())
                        {
                            joined += ", ";
                        }
                        joined += *value;
                    }
                }
            }
            readout.set_text("Selected: " + (joined.empty() ? std::string{"(none)"} : joined));
        }

        std::vector<std::string> items_; // SelectionSyncModel.Items
        maui::controls::content_page page_;
        maui::controls::scroll_view scroll_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label instructions_;

        // The eight order-permutation blocks (label + readout + CV each).
        maui::controls::label label1_, readout1_;
        maui::controls::collection_view cv1_;
        maui::controls::label label2_, readout2_;
        maui::controls::collection_view cv2_;
        maui::controls::label label3_, readout3_;
        maui::controls::collection_view cv3_;
        maui::controls::label label4_, readout4_;
        maui::controls::collection_view cv4_;
        maui::controls::label label5_, readout5_;
        maui::controls::collection_view cv5_;
        maui::controls::label label6_, readout6_;
        maui::controls::collection_view cv6_;
        maui::controls::label label7_, readout7_;
        maui::controls::collection_view cv7_;
        maui::controls::label label8_, readout8_;
        maui::controls::collection_view cv8_;

        // CV #9: the Switch Source block.
        maui::controls::label label9_, label9b_, readout9_;
        maui::controls::button switch_button_;
        maui::controls::collection_view cv9_;
    };
} // namespace maui::samples
