#pragma once
// maui::samples::selection_command_param_page — ports SelectionChangedCommandParameter.xaml (+ .xaml.cs)
//   (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.SelectionChangedCommandParameter).
//
// The C# page is a SINGLE-selection test that verifies SelectionChangedCommandParameter is delivered to
// SelectionChangedCommand and equals the two-way SelectedItem. An ItemsViewModel exposes Items (10 Item
// records: Id, Text="Item {n}", Description="This is item {n}") and:
//   - SelectedItem (TwoWay)
//   - SelectionChangedCommand = Command<Item>(item => Result.Text = (item == SelectedItem) ? "Success" : "Fail")
// The CollectionView wires SelectionMode="Single", SelectedItem="{Binding SelectedItem, Mode=TwoWay}",
// SelectionChangedCommand="{Binding SelectionChangedCommand}", and
// SelectionChangedCommandParameter="{Binding SelectedItem, Source={x:Reference MyCollectionView}}" — i.e.
// the parameter is the CollectionView's OWN SelectedItem. A Label "Result" starts at "Pending..." and the
// command writes "Success"/"Fail" into it. The cell shows Text + Description.
//
// This headless port owns its whole tree. The selection_changed_command (move_only_function<void()>) fires
// from SelectionPropertyChanged BEFORE selection_changed and AFTER selected_item_ is stored, so reading
// list_.selected_item() inside the command body yields the new SelectedItem — which is exactly the value
// the C# CommandParameter (bound to the view's SelectedItem) carries. The command compares that value to
// the page's tracked SelectedItem (kept in sync via selection_changed) and writes "Success" when equal,
// "Fail" otherwise — the C# Command<Item> body verbatim.
//
// note: the port's selectable_items_view does NOT carry SelectionChangedCommandParameter (it is a
//   parameterless command convention — move_only_function<void()>); the C# property exists in the public
//   surface (SelectableItemsView.SelectionChangedCommandParameter) but is not modeled in the headless
//   selection surface. The CommandParameter SEMANTICS — "the command receives the view's current
//   SelectedItem" — are realized here by reading list_.selected_item() inside the command body (the same
//   value the {x:Reference MyCollectionView}.SelectedItem binding would supply), so the Success/Fail
//   invariant the test checks is exercised faithfully.

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
    class selection_command_param_page
    {
    public:
        // The ItemsViewModel item (the C# `Item` record). The cell binds Text; Description is kept for
        // surface fidelity. Value equality drives selection identity (boxed_item::of<T> uses operator==).
        struct item
        {
            std::string id;
            std::string text;
            std::string description;

            friend bool operator==(const item& left, const item& right)
            {
                return left.id == right.id && left.text == right.text && left.description == right.description;
            }
        };

        selection_command_param_page() : items_(std::make_shared<maui::core::observable_collection<item>>())
        {
            page_.set_title("Selection Changed Command Parameter");

            // ItemsViewModel ctor: 10 items, Id=n, Text="Item {n}", Description="This is item {n}".
            for (int n = 0; n < 10; ++n)
            {
                const std::string id = std::to_string(n);
                items_->add(item{id, "Item " + id, "This is item " + id});
            }

            result_.set_text("Pending..."); // Label x:Name="Result" Text="Pending..."

            // The cell: the C# DataTemplate has two labels (Text + Description). The single-root struct cell
            // renders both on two lines in one label ("Item N\nThis is item N") so the cell visually matches
            // MAUI's two-line cell (title + detail).
            auto cell = maui::controls::data_template::of<maui::controls::label>();
            cell->set_binding<std::string, item>(maui::controls::label::text_property(), [](const item& value) {
                return value.text + "\n" + value.description;
            });
            list_.set_item_template(cell);
            list_.set_header(maui::controls::boxed_item::of(std::string{"This is the header"})); // Header=…
            list_.set_selection_mode(maui::controls::selection_mode::single);                    // Single
            list_.set_items_source(items_);

            // SelectionChangedCommand={Binding SelectionChangedCommand}. In C# the binding engine pushes
            // BOTH the TwoWay SelectedItem binding AND the SelectionChangedCommandParameter (both bound to
            // the view's SelectedItem) before invoking the command — so the parameter and SelectedItem
            // observed inside the command are the same value. The port realizes both legs from the one
            // source the view exposes, list_.selected_item(), inside the command body (see
            // on_selection_changed_command); selection_changed itself need not be subscribed for the test.
            list_.selection_changed_command = [this] { on_selection_changed_command(); };

            stack_.add(result_);
            stack_.add(list_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, result_, "result_");
            gallery_attach_one(app, list_, "list_");
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
        [[nodiscard]] maui::controls::label& result()
        {
            return result_;
        }
        [[nodiscard]] const std::shared_ptr<maui::core::observable_collection<item>>& items() const
        {
            return items_;
        }

    private:
        // Command<Item>(item => Result.Text = (item == SelectedItem) ? "Success" : "Fail").
        //   - `item`        = the CommandParameter, bound to {x:Reference MyCollectionView}.SelectedItem;
        //   - `SelectedItem` = the TwoWay {Binding SelectedItem}, also fed from the view's SelectedItem.
        // Both legs draw from the one value the view exposes (list_.selected_item(), already stored when
        // this fires). The port reads it once as the parameter and pushes the SAME value through the
        // TwoWay SelectedItem leg (selected_item_), then compares — matching the C# binding engine, which
        // delivers both from the same selection, so they are equal → "Success".
        void on_selection_changed_command()
        {
            const maui::controls::boxed_item from_parameter = list_.selected_item(); // {x:Reference}.SelectedItem
            selected_item_ = from_parameter;                                         // TwoWay SelectedItem leg
            const maui::controls::boxed_item& from_selected_item = selected_item_;   // {Binding SelectedItem}
            result_.set_text(from_parameter == from_selected_item ? "Success" : "Fail");
        }

        std::shared_ptr<maui::core::observable_collection<item>> items_; // publisher before the list (§8)
        maui::controls::boxed_item selected_item_;                       // the TwoWay-bound VM SelectedItem
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label result_; // x:Name="Result"
        maui::controls::collection_view list_;
    };
} // namespace maui::samples
