#pragma once
// maui::samples::scattered_radio_button_page — ports ScatteredRadioButtonGallery.xaml
//
// A code-first demo that radio buttons DON'T have to share a container to be grouped: grouping is by
// GroupName, so buttons scattered across separate containers (and one bare button outside any grouped
// container) all mutually exclude as long as they carry the same GroupName "foo". It mirrors the C#
// controls gallery page (Pages/Controls/RadioButtonGalleries/ScatteredRadioButtonGallery.xaml):
//
//   <StackLayout>
//     <Label Text="RadioButtons don't have to be in the same container to be grouped." />
//     <Label Text="Here, we have a few in a nested StackLayout:"/>
//     <StackLayout Orientation="Horizontal" BackgroundColor="AliceBlue"
//                  RadioButtonGroup.GroupName="foo">
//       <RadioButton Content="A"/> <RadioButton Content="B"/> <RadioButton Content="C"/>
//     </StackLayout>
//     <Label Text="And another outside that StackLayout with the same GroupName:"/>
//     <RadioButton Content="D (None of the above)" GroupName="foo"/>
//   </StackLayout>
//
// TWO grouping channels are exercised, both demonstrated faithfully:
//   - CONTAINER-attached GroupName "foo" on the nested horizontal StackLayout: its controller pushes "foo"
//     down to A/B/C (radio_button_group::set_group_name on the inner stack).
//   - PER-BUTTON GroupName "foo" on Option D, which lives OUTSIDE that stack: D adopts "foo" directly via
//     radio_button::set_group_name, joining the same logical group even though it has no grouped ancestor.
// Because A/B/C and D all share GroupName "foo", checking any one unchecks the others — the cross-container
// mutual exclusion the page exists to show.
//
// READOUT (additive demo aid): the inner stack's controller is the one carrying GroupName "foo", so its
// selected_value_changed echoes the current selection. Each button is given a Value of its content letter
// so the readout reports which of A/B/C/D is selected (the XAML leaves Value unset). The nested stack also
// keeps its AliceBlue background, set via a solid_paint.
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same wiring directly.

#include <any>
#include <memory>
#include <string>

#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/radio_button.hpp"
#include "maui/controls/radio_button_group.hpp"
#include "maui/controls/stack_layout.hpp"
#include "maui/controls/stack_orientation.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class scattered_radio_button_page
    {
    public:
        scattered_radio_button_page()
        {
            page_.set_title("RadioButton Group (Across Multiple Containers)");

            // The shared scattered_radio_button.xaml root StackLayout carries Padding="16" Spacing="6";
            // the 16pt padding insets every child (crucially the nested AliceBlue horizontal stack, whose
            // light background otherwise bleeds edge-to-edge and stands out in dark mode) so the bar hugs
            // the padded content box exactly as MAUI + the xaml loader render it.
            root_stack_.set_padding(maui::core::thickness{16});
            root_stack_.set_spacing(6);

            intro_.set_text("RadioButtons don't have to be in the same container to be grouped.");
            nested_caption_.set_text("Here, we have a few in a nested StackLayout:");
            outside_caption_.set_text("And another outside that StackLayout with the same GroupName:");
            readout_.set_text("Selected: (none)");

            // The nested horizontal stack carrying GroupName "foo" + an AliceBlue background. A/B/C are its
            // children and adopt "foo" through its controller.
            nested_stack_.set_orientation(maui::controls::stack_orientation::horizontal);
            nested_stack_.set_background(
                std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::alice_blue));

            option_a_.set_content("A");
            option_a_.set_value(std::any{std::string{"A"}});
            option_b_.set_content("B");
            option_b_.set_value(std::any{std::string{"B"}});
            option_c_.set_content("C");
            option_c_.set_value(std::any{std::string{"C"}});

            nested_stack_.add(option_a_);
            nested_stack_.add(option_b_);
            nested_stack_.add(option_c_);

            // Option D lives OUTSIDE the nested stack and joins the group via its OWN GroupName "foo" (the
            // per-button channel — radio_button::set_group_name), not via a grouped ancestor.
            option_d_.set_content("D (None of the above)");
            option_d_.set_value(std::any{std::string{"D"}});
            option_d_.set_group_name("foo");

            root_stack_.add(intro_);
            root_stack_.add(readout_);
            root_stack_.add(nested_caption_);
            root_stack_.add(nested_stack_);
            root_stack_.add(outside_caption_);
            root_stack_.add(option_d_);

            page_.set_content(root_stack_);

            // The demonstrated feature: CONTAINER-attached GroupName "foo" on the nested stack groups A/B/C;
            // Option D's per-button GroupName "foo" joins the same group from outside. Attach the container
            // controller AFTER the tree is assembled (so the resource-chain walk adopts A/B/C), and wire its
            // selected_value_changed to the readout. (set_group_name on the inner stack creates/returns the
            // "foo" controller; D's per-button name slots it into the same logical group.)
            maui::controls::radio_button_group::set_group_name(nested_stack_, "foo");
            maui::controls::radio_button_group::controller_of(nested_stack_)
                ->selected_value_changed.connect([this](const std::any& value) { update_readout(value); });
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::stack_layout& root_stack()
        {
            return root_stack_;
        }
        [[nodiscard]] maui::controls::stack_layout& nested_stack()
        {
            return nested_stack_;
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
        maui::controls::stack_layout root_stack_;
        maui::controls::label intro_;
        maui::controls::label readout_;
        maui::controls::label nested_caption_;
        maui::controls::stack_layout nested_stack_;
        maui::controls::label outside_caption_;
        maui::controls::radio_button option_a_;
        maui::controls::radio_button option_b_;
        maui::controls::radio_button option_c_;
        maui::controls::radio_button option_d_;
    };
} // namespace maui::samples
