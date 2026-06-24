#pragma once
// maui::samples::radio_button_group_binding_page — ports RadioButtonGroupBindingGallery.xaml
//
// A code-first demo of binding the RadioButtonGroup attached properties (GroupName + SelectedValue) to a
// view-model. It mirrors the C# controls gallery page
// (Pages/Controls/RadioButtonGalleries/RadioButtonGroupBindingGallery.xaml + .xaml.cs):
//
//   <Grid RadioButtonGroup.GroupName="{Binding GroupName}"
//         RadioButtonGroup.SelectedValue="{Binding Selection}">
//     <Label Text="The RadioButtons in this Grid have a GroupName and Selection bound to a ViewModel."/>
//     <Label Text="{Binding GroupName, StringFormat='The GroupName is {0}'}"/>
//     <Label Text="{Binding Selection, StringFormat='The Selection is {0}',
//                   TargetNullValue='The Selection is (null)'}"/>
//     <RadioButton Content="Option A" Value="A"/> ... Option B/C/D with Values "B"/"C"/"D"
//     <Button Text="Set selection in view model to 'B'"   Clicked="Set_Button_Clicked"/>
//     <Button Text="Clear selection in view model to 'null'" Clicked="Clear_Button_Clicked"/>
//   </Grid>
//
// and the code-behind view-model (RadioButtonGroupBindingModel : INotifyPropertyChanged) initialized with
// GroupName = "group1", whose Selection is poked to "B" / null by the two buttons.
//
// PORT MAPPING. The headless API has no XAML binding engine, so the {Binding} two-way attached-property
// channel is reproduced through the controller seam (radio_button_group.hpp):
//   - GroupName="{Binding GroupName}"   → set_group_name(grid_, vm_group_name) at construction (the VM's
//     GroupName is fixed to "group1", as in the code-behind — no later mutation, so a one-shot push is
//     faithful).
//   - SelectedValue="{Binding Selection}" two-way → BOTH directions are wired:
//       VM → group: the Set/Clear buttons call set_selected_value("B") / set_selected_value(null), which
//         sweeps the group (a non-null value checks the matching button, null unchecks the checked one).
//       group → VM: the controller's selected_value_changed updates the bound "Selection" readout label,
//         standing in for the TwoWay write-back into the view-model.
//   - The two StringFormat labels are driven the same way: the GroupName label is set once; the Selection
//     label is refreshed on every selected_value_changed, with the TargetNullValue '(null)' fallback.
//   - Button.Clicked → button::clicked.connect (the port's named-event/clicked seam).
//
// Each RadioButton's Value ("A".."D") is the boxed std::any the controller compares (boxed_equals), so
// set_selected_value("B") matches Option B and checks it — exactly the C# behavior.
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same wiring directly.

#include <any>
#include <memory>
#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/radio_button.hpp"
#include "maui/controls/radio_button_group.hpp"
#include "maui/controls/stack_layout.hpp"
#include "maui/core/grid_length.hpp"

namespace maui::samples
{
    class radio_button_group_binding_page
    {
    public:
        radio_button_group_binding_page()
        {
            page_.set_title("RadioButton Group (Attached Property, Binding)");

            // Grid: two star columns and six rows (the second is the fixed-height 50 readout row, the rest
            // are star), mirroring the XAML RowDefinitions/ColumnDefinitions.
            grid_.add_column_definition(maui::core::grid_length::star());
            grid_.add_column_definition(maui::core::grid_length::star());
            grid_.add_row_definition(maui::core::grid_length::star()); // row 0: header label
            grid_.add_row_definition(maui::core::grid_length{50.0});   // row 1: the two readouts
            grid_.add_row_definition(maui::core::grid_length::star()); // row 2: Option A / B
            grid_.add_row_definition(maui::core::grid_length::star()); // row 3: Option C / D
            grid_.add_row_definition(maui::core::grid_length::star()); // row 4: Set button
            grid_.add_row_definition(maui::core::grid_length::star()); // row 5: Clear button

            header_.set_text("The RadioButtons in this Grid have a GroupName and Selection bound to a "
                             "ViewModel.");

            // The two StringFormat readout labels. GroupName is fixed by the VM ("group1"); Selection starts
            // null → TargetNullValue text.
            group_name_label_.set_text("The GroupName is " + vm_group_name_);
            selection_label_.set_text("The Selection is (null)");

            // The four grouped radio buttons, each with its boxed Value the controller compares.
            option_a_.set_content("Option A");
            option_a_.set_value(std::any{std::string{"A"}});
            option_b_.set_content("Option B");
            option_b_.set_value(std::any{std::string{"B"}});
            option_c_.set_content("Option C");
            option_c_.set_value(std::any{std::string{"C"}});
            option_d_.set_content("Option D");
            option_d_.set_value(std::any{std::string{"D"}});

            // The two buttons that poke the bound Selection in the "view model" (here: directly through the
            // controller's set_selected_value — the VM→group leg of the two-way binding).
            set_button_.set_text("Set selection in view model to 'B'");
            set_button_.clicked.connect([this] {
                maui::controls::radio_button_group::controller_of(grid_)->set_selected_value(
                    std::any{std::string{"B"}});
            });
            clear_button_.set_text("Clear selection in view model to 'null'");
            clear_button_.clicked.connect(
                [this] { maui::controls::radio_button_group::controller_of(grid_)->set_selected_value(std::any{}); });

            // Place every child in its grid cell (Grid.Row/Column/ColumnSpan from the XAML).
            grid_.add(header_);
            grid_.set_row(header_, 0);
            grid_.set_column_span(header_, 2); // Grid.ColumnSpan="2"

            grid_.add(group_name_label_);
            grid_.set_row(group_name_label_, 1); // Grid.Row="1", Column 0

            grid_.add(selection_label_);
            grid_.set_row(selection_label_, 1);
            grid_.set_column(selection_label_, 1); // Grid.Row="1" Grid.Column="1"

            grid_.add(option_a_);
            grid_.set_row(option_a_, 2);
            grid_.add(option_b_);
            grid_.set_row(option_b_, 2);
            grid_.set_column(option_b_, 1);
            grid_.add(option_c_);
            grid_.set_row(option_c_, 3);
            grid_.add(option_d_);
            grid_.set_row(option_d_, 3);
            grid_.set_column(option_d_, 1);

            grid_.add(set_button_);
            grid_.set_row(set_button_, 4);
            grid_.set_column_span(set_button_, 2);
            grid_.add(clear_button_);
            grid_.set_row(clear_button_, 5);
            grid_.set_column_span(clear_button_, 2);

            stack_.add(grid_);
            page_.set_content(stack_);

            // The demonstrated feature: GroupName + SelectedValue bound to a "view model" on the CONTAINER.
            // GroupName is pushed once (the VM fixes it to "group1"); SelectedValue is two-way — the buttons
            // write VM→group above, and selected_value_changed writes group→VM (the Selection readout) here.
            maui::controls::radio_button_group::set_group_name(grid_, vm_group_name_);
            maui::controls::radio_button_group::controller_of(grid_)->selected_value_changed.connect(
                [this](const std::any& value) { update_selection_readout(value); });
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::grid& grid()
        {
            return grid_;
        }
        [[nodiscard]] maui::controls::radio_button& option_a()
        {
            return option_a_;
        }
        [[nodiscard]] maui::controls::radio_button& option_b()
        {
            return option_b_;
        }
        [[nodiscard]] maui::controls::radio_button& option_c()
        {
            return option_c_;
        }
        [[nodiscard]] maui::controls::radio_button& option_d()
        {
            return option_d_;
        }
        [[nodiscard]] maui::controls::button& set_button()
        {
            return set_button_;
        }
        [[nodiscard]] maui::controls::button& clear_button()
        {
            return clear_button_;
        }
        [[nodiscard]] maui::controls::label& selection_label()
        {
            return selection_label_;
        }

    private:
        // The group→VM (TwoWay write-back) readout: a non-null Selection prints its value, null prints the
        // TargetNullValue text — exactly the StringFormat / TargetNullValue pair on the XAML label.
        void update_selection_readout(const std::any& value)
        {
            if (const auto* selected = std::any_cast<std::string>(&value))
            {
                selection_label_.set_text("The Selection is " + *selected);
            }
            else
            {
                selection_label_.set_text("The Selection is (null)");
            }
        }

        // The "view model" GroupName, fixed to "group1" as in the code-behind RadioButtonGroupBindingModel.
        std::string vm_group_name_{"group1"};

        maui::controls::content_page page_;
        maui::controls::stack_layout stack_;
        maui::controls::grid grid_;
        maui::controls::label header_;
        maui::controls::label group_name_label_;
        maui::controls::label selection_label_;
        maui::controls::radio_button option_a_;
        maui::controls::radio_button option_b_;
        maui::controls::radio_button option_c_;
        maui::controls::radio_button option_d_;
        maui::controls::button set_button_;
        maui::controls::button clear_button_;
    };
} // namespace maui::samples
