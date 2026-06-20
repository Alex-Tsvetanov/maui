#pragma once
// maui::samples::controls_stack_page — a faithful reproduction of the maui-compare "controls_stack" demo
// (ComparePages.ControlsStack()), the shipped-.NET-MAUI reference for the visual-parity comparison: a
// VerticalStackLayout (Spacing 12, Padding 16) showcasing the basic widgets in order — a bold FontSize-22
// "Controls" header, a Button ("A Button"), an Entry ("An Entry"), an Editor ("An Editor", height 60), a
// SearchBar ("A SearchBar"), a horizontal row of [CheckBox checked, Switch on, ActivityIndicator running],
// a Slider (0..100 at 40), a Stepper (0..10 at 3), and a ProgressBar (0.6). Kept 1:1 with the C# reference.
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same controls directly.

#include "maui/controls/activity_indicator.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/check_box.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/editor.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/progress_bar.hpp"
#include "maui/controls/search_bar.hpp"
#include "maui/controls/slider.hpp"
#include "maui/controls/stepper.hpp"
#include "maui/controls/toggle_switch.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/font.hpp"
#include "maui/core/thickness.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class controls_stack_page
    {
    public:
        controls_stack_page()
        {
            page_.set_title("Controls");
            stack_.set_spacing(12);
            stack_.set_padding(maui::core::thickness(16));

            header_.set_text("Controls");
            header_.set_font(maui::core::font::system_font_of_size(22.0, maui::core::font_weight::bold));

            button_.set_text("A Button");
            entry_.set_placeholder("An Entry");
            editor_.set_placeholder("An Editor");
            editor_.set_height_request(60);
            search_.set_placeholder("A SearchBar");

            // The horizontal row: CheckBox (checked), Switch (on), ActivityIndicator (running).
            row_.set_spacing(12);
            check_.set_is_checked(true);
            switch_.set_is_toggled(true);
            spinner_.set_is_running(true);
            row_.add(check_);
            row_.add(switch_);
            row_.add(spinner_);

            slider_.set_minimum(0);
            slider_.set_maximum(100);
            slider_.set_value(40);

            stepper_.set_minimum(0);
            stepper_.set_maximum(10);
            stepper_.set_value(3);

            progress_.set_progress(0.6);

            stack_.add(header_);
            stack_.add(button_);
            stack_.add(entry_);
            stack_.add(editor_);
            stack_.add(search_);
            stack_.add(row_);
            stack_.add(slider_);
            stack_.add(stepper_);
            stack_.add(progress_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (the row's children + the stack's children first,
        // then the row, the stack, then the page), then re-host the tree built in the ctor.
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, check_, "check_");
            gallery_attach_one(app, switch_, "switch_");
            gallery_attach_one(app, spinner_, "spinner_");
            gallery_attach_one(app, header_, "header_");
            gallery_attach_one(app, button_, "button_");
            gallery_attach_one(app, entry_, "entry_");
            gallery_attach_one(app, editor_, "editor_");
            gallery_attach_one(app, search_, "search_");
            gallery_attach_one(app, slider_, "slider_");
            gallery_attach_one(app, stepper_, "stepper_");
            gallery_attach_one(app, progress_, "progress_");
            gallery_attach_one(app, row_, "row_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(row_);   // the row hosts the checkbox/switch/spinner
            gallery_rehost_layout(stack_); // the stack hosts the header + widgets + the row
            gallery_rehost_content(page_); // the page hosts the stack
        }

        // The owned controls, exposed for tests / the hosting main's bottom-up attachment.
        [[nodiscard]] maui::controls::vertical_stack_layout& root()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::slider& slider()
        {
            return slider_;
        }
        [[nodiscard]] maui::controls::progress_bar& progress()
        {
            return progress_;
        }

    private:
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label header_;
        maui::controls::button button_;
        maui::controls::entry entry_;
        maui::controls::editor editor_;
        maui::controls::search_bar search_;
        maui::controls::horizontal_stack_layout row_;
        maui::controls::check_box check_;
        maui::controls::toggle_switch switch_;
        maui::controls::activity_indicator spinner_;
        maui::controls::slider slider_;
        maui::controls::stepper stepper_;
        maui::controls::progress_bar progress_;
    };
} // namespace maui::samples
