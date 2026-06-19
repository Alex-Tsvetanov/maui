#pragma once
// maui::samples::radio_button_border_page — ports RadioButtonBorder.xaml
//
// A self-contained, code-first demo of RadioButton border styling. It mirrors the C# controls gallery
// page (Pages/Controls/RadioButtonGalleries/RadioButtonBorder.xaml): a vertical StackLayout with a
// header Label and four RadioButtons that exercise the BorderColor / BorderWidth / CornerRadius +
// BackgroundColor surface:
//   - Option 1: Content "Option 1", BackgroundColor Yellow, BorderColor Red,   BorderWidth 4, CornerRadius 12
//   - Option 2: Content "Option 2", BackgroundColor Yellow                       (default border)
//   - Option 3: Content "Option 3"                                              (all defaults)
//   - Option 4: Content "Option 4", BorderColor Green, BorderWidth 4, CornerRadius 12, IsChecked = true
//
// In MAUI the BorderColor / BorderWidth / CornerRadius are the RadioButton's IButtonStroke surface; the
// port spells them stroke_color / stroke_thickness / corner_radius (the button control's convention) and
// they map to the same native border (radio_button.hpp). BackgroundColor is the inherited VisualElement
// background, set here as a solid_paint.
//
// Grouping: the XAML gives no GroupName, so MAUI auto-groups the four buttons by their shared StackLayout
// parent (one-checked-per-group within the container). The port reproduces this by attaching a
// radio_button_group to the stack, so checking one unchecks the others — and a readout label echoes the
// current selection so the mutual-exclusion behavior is demonstrable live. Option 4 starts checked, as in
// the XAML (IsChecked="true").
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
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class radio_button_border_page
    {
    public:
        radio_button_border_page()
        {
            page_.set_title("RadioButton Border");

            header_.set_text("RadioButton with Border");
            readout_.set_text("Selected: Option 4");

            // Option 1 — yellow fill, red 4-unit border, 12 corner radius.
            option1_.set_content("Option 1");
            option1_.set_value(std::any{std::string{"Option 1"}});
            option1_.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::yellow));
            option1_.set_stroke_color(maui::graphics::colors::red);
            option1_.set_stroke_thickness(4);
            option1_.set_corner_radius(12);

            // Option 2 — yellow fill, default border.
            option2_.set_content("Option 2");
            option2_.set_value(std::any{std::string{"Option 2"}});
            option2_.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::yellow));

            // Option 3 — all defaults.
            option3_.set_content("Option 3");
            option3_.set_value(std::any{std::string{"Option 3"}});

            // Option 4 — green 4-unit border, 12 corner radius, initially checked.
            option4_.set_content("Option 4");
            option4_.set_value(std::any{std::string{"Option 4"}});
            option4_.set_stroke_color(maui::graphics::colors::green);
            option4_.set_stroke_thickness(4);
            option4_.set_corner_radius(12);

            stack_.add(header_);
            stack_.add(readout_);
            stack_.add(option1_);
            stack_.add(option2_);
            stack_.add(option3_);
            stack_.add(option4_);

            // Auto-group by the shared parent (the XAML has no GroupName): one checked at a time, with the
            // selection echoed into the readout (the attached SelectedValue channel).
            maui::controls::radio_button_group::set_group_name(stack_, "border-options");
            maui::controls::radio_button_group::controller_of(stack_)->selected_value_changed.connect(
                [this](const std::any& value) { update_readout(value); });

            // IsChecked="true" on Option 4 (set after grouping so the group records it as the selection).
            option4_.set_is_checked(true);

            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (the header + readout + four radio buttons, then
        // the stack, then the page), then re-host the tree built in the ctor (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, header_, "header_");
            gallery_attach_one(app, readout_, "readout_");
            gallery_attach_one(app, option1_, "option1_");
            gallery_attach_one(app, option2_, "option2_");
            gallery_attach_one(app, option3_, "option3_");
            gallery_attach_one(app, option4_, "option4_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(stack_); // stack hosts the header + readout + four radio buttons
            gallery_rehost_content(page_); // page hosts the stack
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::radio_button& option1()
        {
            return option1_;
        }
        [[nodiscard]] maui::controls::radio_button& option2()
        {
            return option2_;
        }
        [[nodiscard]] maui::controls::radio_button& option3()
        {
            return option3_;
        }
        [[nodiscard]] maui::controls::radio_button& option4()
        {
            return option4_;
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
        maui::controls::radio_button option1_;
        maui::controls::radio_button option2_;
        maui::controls::radio_button option3_;
        maui::controls::radio_button option4_;
    };
} // namespace maui::samples
