#pragma once
// input_controls_page — a self-contained demo page for the W1-05 input-control set: editor, search_bar,
// radio_button (+ the radio_button_group attached grouping) and image_button on one vertical stack,
// wired together so every input drives a visible output (the C# gallery-page convention, code-first;
// the value_controls_page pattern).
//
// The page OWNS its whole element tree (the sample_app pattern in maui_app_sample.mm). It is
// backend-agnostic — a sample main attaches handlers bottom-up via the hosting layer and hosts
// page() in a window; the headless/apple/ios test trees exercise the same controls directly.
//
// Interactions demonstrated:
//   - the editor's text drives the character-count readout (text_changed),
//   - the search bar's Search action copies the query into the editor (search_button_pressed),
//   - the radio group ("casing") picks how the readout renders the text length label — the
//     radio_button_group attached SelectedValue drives the choice (selected_value_changed),
//   - the image button clears the editor (clicked).

#include <any>
#include <cstdio>
#include <string>

#include "maui/controls/content_page.hpp"
#include "maui/controls/editor.hpp"
#include "maui/controls/image_button.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/radio_button.hpp"
#include "maui/controls/radio_button_group.hpp"
#include "maui/controls/search_bar.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/boxed_value.hpp"

namespace maui::samples
{
    class input_controls_page
    {
    public:
        input_controls_page()
        {
            page_.set_title("Input controls");
            stack_.set_spacing(12);

            readout_.set_text("LENGTH: 0");

            // editor — the multiline input; its text drives the readout.
            editor_.set_placeholder("Type here...");
            editor_.text_changed.connect(
                [this](const std::string& /*old_text*/, const std::string& /*new_text*/) { update_readout(); });

            // search_bar — the Search action copies the query into the editor.
            search_.set_placeholder("Search to insert");
            search_.search_button_pressed.connect([this] { editor_.set_text(std::string(search_.text())); });

            // radio group — picks the readout casing through the attached SelectedValue channel.
            upper_choice_.set_content("UPPER");
            upper_choice_.set_value(std::any{std::string{"upper"}});
            lower_choice_.set_content("lower");
            lower_choice_.set_value(std::any{std::string{"lower"}});
            stack_.add(readout_);
            stack_.add(editor_);
            stack_.add(search_);
            stack_.add(upper_choice_);
            stack_.add(lower_choice_);
            stack_.add(clear_button_);
            maui::controls::radio_button_group::set_group_name(stack_, "casing");
            maui::controls::radio_button_group::controller_of(stack_)->selected_value_changed.connect(
                [this](const std::any& /*value*/) { update_readout(); });
            upper_choice_.set_is_checked(true);

            // image_button — clears the editor.
            clear_button_.clicked.connect([this] { editor_.set_text(""); });

            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }
        [[nodiscard]] maui::controls::editor& text_editor()
        {
            return editor_;
        }
        [[nodiscard]] maui::controls::search_bar& search()
        {
            return search_;
        }
        [[nodiscard]] maui::controls::radio_button& upper_choice()
        {
            return upper_choice_;
        }
        [[nodiscard]] maui::controls::radio_button& lower_choice()
        {
            return lower_choice_;
        }
        [[nodiscard]] maui::controls::image_button& clear_button()
        {
            return clear_button_;
        }

    private:
        void update_readout()
        {
            const bool upper = maui::core::boxed_equals(maui::controls::radio_button_group::selected_value(stack_),
                                                        std::any{std::string{"upper"}});
            char text[48];
            std::snprintf(text, sizeof(text), upper ? "LENGTH: %zu" : "length: %zu", editor_.text().size());
            readout_.set_text(text);
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label readout_;
        maui::controls::editor editor_;
        maui::controls::search_bar search_;
        maui::controls::radio_button upper_choice_;
        maui::controls::radio_button lower_choice_;
        maui::controls::image_button clear_button_;
    };
} // namespace maui::samples
