#pragma once
// maui::samples::radio_button_group_page — ports RadioButtonGroupGallery.xaml
//
// A self-contained, code-first demo of the RadioButtonGroup ATTACHED-PROPERTY grouping: a vertical
// StackLayout carries RadioButtonGroup.GroupName="foo", so every descendant RadioButton — including one
// nested inside a child Grid — automatically adopts the group name "foo" and participates in a single
// one-checked-at-a-time group. This mirrors the C# controls gallery page
// (Pages/Controls/RadioButtonGalleries/RadioButtonGroupGallery.xaml):
//
//   <StackLayout RadioButtonGroup.GroupName="foo">
//     <Label .../>               header text
//     <RadioButton Content="Option A" />
//     <RadioButton Content="Option B" />
//     <RadioButton Content="Option C" />
//     <Grid> <Label/> <RadioButton Content="Option D" Grid.Column="1"/> </Grid>
//   </StackLayout>
//
// The XAML carries no per-button GroupName and no Value — the whole point is that attaching GroupName to
// the CONTAINER pushes the name down the descendant chain (UpdateGroupNames → on_resource_chain_changed
// in the port; see radio_button_group.hpp). The port faithfully reproduces this by attaching a
// radio_button_group controller to the stack via set_group_name; the four buttons (three direct children
// + Option D nested under the Grid) are all adopted into "foo" and mutually exclude. A readout label
// (added by the port for live demonstrability) echoes the current selection through the controller's
// selected_value_changed channel — each button is given a Value of its content string so the readout has
// something to report (the XAML leaves Value unset; this is an additive demo aid, marked below).
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same wiring directly.

#include <any>
#include <memory>
#include <string>

#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/radio_button.hpp"
#include "maui/controls/radio_button_group.hpp"
#include "maui/controls/stack_layout.hpp"
#include "maui/core/grid_length.hpp"

namespace maui::samples
{
    class radio_button_group_page
    {
    public:
        radio_button_group_page()
        {
            page_.set_title("RadioButton Group (Attached Property)");

            header_.set_text("All of the RadioButtons in this StackLayout will automatically be given the "
                             "GroupName of 'foo'");
            readout_.set_text("Selected: (none)");

            // The three direct-child radio buttons (Option A/B/C). The XAML sets only Content; the port also
            // gives each a Value (its content string) so the controller's selected_value readout is live —
            // an additive demo aid (the C# page leaves Value unset).
            option_a_.set_content("Option A");
            option_a_.set_value(std::any{std::string{"Option A"}});
            option_b_.set_content("Option B");
            option_b_.set_value(std::any{std::string{"Option B"}});
            option_c_.set_content("Option C");
            option_c_.set_value(std::any{std::string{"Option C"}});

            // The nested Grid with two columns; Option D sits in column 1, the explanatory label in column 0.
            grid_.add_column_definition(maui::core::grid_length::star());
            grid_.add_column_definition(maui::core::grid_length::star());

            grid_label_.set_text("This RadioButton is inside a Grid");

            option_d_.set_content("Option D");
            option_d_.set_value(std::any{std::string{"Option D"}});

            grid_.add(grid_label_); // Grid.Column 0 (default)
            grid_.add(option_d_);
            grid_.set_column(option_d_, 1); // Grid.Column="1"

            stack_.add(header_);
            stack_.add(readout_);
            stack_.add(option_a_);
            stack_.add(option_b_);
            stack_.add(option_c_);
            stack_.add(grid_);

            // The demonstrated feature: GroupName attached to the CONTAINER (not the buttons). Attaching it
            // pushes "foo" down to every descendant radio button — the three direct children AND Option D
            // nested under the Grid — so all four mutually exclude as one group.
            maui::controls::radio_button_group::set_group_name(stack_, "foo");
            maui::controls::radio_button_group::controller_of(stack_)->selected_value_changed.connect(
                [this](const std::any& value) { update_readout(value); });

            page_.set_content(stack_);
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
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }

    private:
        void update_readout(const std::any& value)
        {
            if (const auto* selected = std::any_cast<std::string>(&value))
            {
                readout_.set_text("Selected: " + *selected);
            }
            else
            {
                readout_.set_text("Selected: (none)");
            }
        }

        maui::controls::content_page page_;
        maui::controls::stack_layout stack_;
        maui::controls::label header_;
        maui::controls::label readout_;
        maui::controls::radio_button option_a_;
        maui::controls::radio_button option_b_;
        maui::controls::radio_button option_c_;
        maui::controls::grid grid_;
        maui::controls::label grid_label_;
        maui::controls::radio_button option_d_;
    };
} // namespace maui::samples
