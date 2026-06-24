#pragma once
// maui::samples::radio_button_group_gallery_page — ports RadioButtonGroupGalleryPage.xaml
//
// A self-contained, code-first demo of RadioButton grouping SCOPE, mirroring the C# controls gallery page
// (Pages/Controls/RadioButtonGalleries/RadioButtonGroupGalleryPage.xaml). The C# page is a TabbedPage with
// three tabs (Parent level / Page level / Test) that each show how a RadioButton's GroupName determines its
// mutual-exclusion scope. The port reproduces the three scenarios as three labeled sections in one scrolled
// page, each section a StackLayout carrying its own radio_button_group controller — the same per-container
// grouping the XAML relies on (radio buttons with no GroupName auto-group by their parent container; equal
// GroupNames group together; different GroupNames stay independent). A readout label per section echoes the
// live selection so the mutual-exclusion behavior is demonstrable.
//
// The three sections (faithful to the three tabs):
//   1. "Parent level" — three radios with NO group name in one StackLayout: they auto-group by the shared
//      parent, so exactly one is checked at a time within that container.
//   2. "Page level"   — three radios all carrying group name "A" in one StackLayout: equal names group them.
//   3. "Test (mixed groups)" — radios spanning groups "A", "B", "C", and ungrouped in one StackLayout: each
//      named group is independent (checking an "A" leaves a "B"/"C" untouched), and the ungrouped buttons
//      auto-group among themselves by the container.
//
// Grouping mechanics (per radio_button_group.hpp, mirroring radio_button_border_page): each section's
// StackLayout gets a named radio_button_group controller; checking one button in a group unchecks the
// others in the SAME group; selected_value_changed feeds the section readout. Buttons get a Value (their
// content text) so the readout can name the selection. // note: the C# TabbedPage's tab UI and the
// ListView/Frame/ContentView/ControlTemplate host variations are gallery scaffolding around the SAME
// grouping rule, so the port collapses them into the three core scope scenarios (the demonstrated feature).
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same wiring directly.

#include <any>
#include <array>
#include <memory>
#include <string>

#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/radio_button.hpp"
#include "maui/controls/radio_button_group.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/stack_layout.hpp"
#include "maui/controls/vertical_stack_layout.hpp"

namespace maui::samples
{
    class radio_button_group_gallery_page
    {
    public:
        radio_button_group_gallery_page()
        {
            page_.set_title("RadioButton Group Gallery");
            root_.set_spacing(10);

            // ---- Section 1: Parent level (no group name — auto-group by container) ----
            parent_header_.set_text(
                "Parent level: radios with no group name are mutually exclusive in their container");
            parent_readout_.set_text("Selected: (none)");
            init_radio(parent_a_, "RadioButton, Group=null");
            init_radio(parent_b_, "RadioButton, Group=null");
            init_radio(parent_c_, "RadioButton, Group=null");
            parent_stack_.add(parent_a_);
            parent_stack_.add(parent_b_);
            parent_stack_.add(parent_c_);
            group_section(parent_stack_, "parent-level", parent_readout_);

            // ---- Section 2: Page level (same group name "A") ----
            page_header_.set_text("Page level: radios with the same group name 'A' are mutually exclusive");
            page_readout_.set_text("Selected: (none)");
            init_radio(page_a_, "RadioButton, Group='A'");
            init_radio(page_b_, "RadioButton, Group='A'");
            init_radio(page_c_, "RadioButton, Group='A'");
            page_a_.set_group_name("A");
            page_b_.set_group_name("A");
            page_c_.set_group_name("A");
            page_stack_.add(page_a_);
            page_stack_.add(page_b_);
            page_stack_.add(page_c_);
            group_section(page_stack_, "A", page_readout_);

            // ---- Section 3: Test (mixed groups A / B / C / null) ----
            test_header_.set_text("Test: mixed group names — each named group is independent");
            test_readout_.set_text("Selected: (none)");
            init_radio(test_a_, "RadioButton, GroupName='A'");
            init_radio(test_b1_, "RadioButton, GroupName='B'");
            init_radio(test_b2_, "RadioButton, GroupName='B'");
            init_radio(test_c_, "RadioButton, GroupName='C'");
            init_radio(test_null_, "RadioButton, GroupName=null");
            test_a_.set_group_name("A");
            test_b1_.set_group_name("B");
            test_b2_.set_group_name("B");
            test_c_.set_group_name("C");
            // test_null_ keeps the empty group name (auto-grouped with other null buttons by the container).
            test_stack_.add(test_a_);
            test_stack_.add(test_b1_);
            test_stack_.add(test_b2_);
            test_stack_.add(test_c_);
            test_stack_.add(test_null_);
            // The container carries a controller so the named groups + the null bucket are tracked; the
            // readout echoes whichever button most recently became the section's selection.
            group_section(test_stack_, "test-mixed", test_readout_);

            // Compose the scrolled page.
            root_.add(parent_header_);
            root_.add(parent_readout_);
            root_.add(parent_stack_);
            root_.add(page_header_);
            root_.add(page_readout_);
            root_.add(page_stack_);
            root_.add(test_header_);
            root_.add(test_readout_);
            root_.add(test_stack_);
            scroller_.set_content(root_);
            page_.set_content(scroller_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment / tests.
        [[nodiscard]] maui::controls::scroll_view& scroller()
        {
            return scroller_;
        }
        [[nodiscard]] maui::controls::vertical_stack_layout& root()
        {
            return root_;
        }
        [[nodiscard]] maui::controls::stack_layout& parent_stack()
        {
            return parent_stack_;
        }
        [[nodiscard]] maui::controls::stack_layout& page_stack()
        {
            return page_stack_;
        }
        [[nodiscard]] maui::controls::stack_layout& test_stack()
        {
            return test_stack_;
        }
        [[nodiscard]] maui::controls::label& parent_readout()
        {
            return parent_readout_;
        }
        [[nodiscard]] maui::controls::label& page_readout()
        {
            return page_readout_;
        }
        [[nodiscard]] maui::controls::label& test_readout()
        {
            return test_readout_;
        }
        [[nodiscard]] maui::controls::radio_button& parent_a()
        {
            return parent_a_;
        }
        [[nodiscard]] maui::controls::radio_button& page_a()
        {
            return page_a_;
        }
        [[nodiscard]] maui::controls::radio_button& test_a()
        {
            return test_a_;
        }

    private:
        // Give a radio button its display content + a boxed Value (the content text) so the group's
        // selected_value_changed can name the selection in the readout.
        static void init_radio(maui::controls::radio_button& button, const std::string& content)
        {
            button.set_content(content);
            button.set_value(std::any{content});
        }

        // Attach a named group controller to one section's container and wire its selection into the
        // section readout (the mutual-exclusion + selection-feedback the page demonstrates).
        static void group_section(maui::controls::stack_layout& container, const std::string& group_name,
                                  maui::controls::label& readout)
        {
            maui::controls::radio_button_group::set_group_name(container, group_name);
            maui::controls::radio_button_group::controller_of(container)->selected_value_changed.connect(
                [&readout](const std::any& value) {
                    if (const auto* selected = std::any_cast<std::string>(&value))
                    {
                        readout.set_text("Selected: " + *selected);
                    }
                    else
                    {
                        readout.set_text("Selected: (none)");
                    }
                });
        }

        maui::controls::content_page page_;
        maui::controls::scroll_view scroller_;
        maui::controls::vertical_stack_layout root_;

        // Section 1 — Parent level (no group name).
        maui::controls::label parent_header_;
        maui::controls::label parent_readout_;
        maui::controls::stack_layout parent_stack_;
        maui::controls::radio_button parent_a_;
        maui::controls::radio_button parent_b_;
        maui::controls::radio_button parent_c_;

        // Section 2 — Page level (group "A").
        maui::controls::label page_header_;
        maui::controls::label page_readout_;
        maui::controls::stack_layout page_stack_;
        maui::controls::radio_button page_a_;
        maui::controls::radio_button page_b_;
        maui::controls::radio_button page_c_;

        // Section 3 — Test (mixed groups A / B / C / null).
        maui::controls::label test_header_;
        maui::controls::label test_readout_;
        maui::controls::stack_layout test_stack_;
        maui::controls::radio_button test_a_;
        maui::controls::radio_button test_b1_;
        maui::controls::radio_button test_b2_;
        maui::controls::radio_button test_c_;
        maui::controls::radio_button test_null_;
    };
} // namespace maui::samples
