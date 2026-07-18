#pragma once
// maui::samples::alerts_page — ports AlertsPage.xaml (+ AlertsPage.xaml.cs)
//
// The C# AlertsPage drives the three Page dialog services — DisplayAlertAsync (simple OK + Yes/No),
// DisplayActionSheetAsync (simple + Cancel/Delete), and DisplayPromptAsync (two questions) — from a
// vertical stack of headline labels + buttons, writing prompt answers into two result labels.
//
// note: the Page dialog services (DisplayAlert / DisplayActionSheet / DisplayPrompt) are a native
// modal-dialog surface and are NOT implemented in the port — there is no headless dialog UI to drive.
// To demonstrate the SAME wiring faithfully, each button's clicked handler synthesizes the result the
// corresponding dialog would yield (the OnAppearing welcome alert; "OK"/Yes; the action-sheet choice;
// the prompt answers "Maui" and "10") and writes it into a shared readout label plus, for the two
// prompts, the page's own question-result labels — exactly the values the C# handlers compute. When the
// real Page dialog services land, these lambdas swap their canned result for the awaited call.
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic; the generic
// mount (maui::hosting::mount_window) attaches handlers bottom-up and hosts the tree.

#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/thickness.hpp"

namespace maui::samples
{
    class alerts_page
    {
    public:
        alerts_page()
        {
            page_.set_title("Alerts");
            // The twin is <StackLayout Margin="12"> (no Spacing → MAUI StackLayout default 0, confirmed by the
            // green xaml column). The code-first previously used spacing 8, spreading the rows wider than MAUI.
            stack_.set_spacing(0);
            stack_.set_margin(maui::core::thickness(12)); // the XAML Margin="12" (uniform) on the content stack

            // The shared alerts.xaml has NO general readout label (results surface as native DisplayAlert
            // dialogs); the readout_ is a port-only interactive stand-in, kept off the visual tree so the
            // resting capture matches MAUI. Its set_readout() updates (below) remain for interactive testing.

            // ---- Display Alert section ----
            alert_header_.set_text("Display Alert");
            alert_header_.set_font(maui::core::font::system_font_of_size(24, maui::core::font_weight::bold));
            alert_simple_.set_text("Alert Simple");
            alert_simple_.clicked.connect([this] { set_readout("Alert: You have been alerted [OK]"); });
            alert_yes_no_.set_text("Alert Yes/No");
            // C# OnAlertYesNoClicked awaits Yes/No; the synthesized answer is Yes (true).
            alert_yes_no_.clicked.connect([this] { set_readout("Question?: Would you like to play a game -> Yes"); });

            // ---- Display ActionSheet section ----
            action_header_.set_text("Display ActionSheet");
            action_header_.set_font(maui::core::font::system_font_of_size(24, maui::core::font_weight::bold));
            action_simple_.set_text("ActionSheet Simple");
            // C# DisplayActionSheet("Send to?", "Cancel", null, "Email","Twitter","Facebook").
            action_simple_.clicked.connect([this] { set_readout("ActionSheet: Send to? -> Email"); });
            action_cancel_delete_.set_text("ActionSheet Cancel/Delete");
            // C# DisplayActionSheet("SavePhoto?", "Cancel", "Delete", "Photo Roll","Email").
            action_cancel_delete_.clicked.connect([this] { set_readout("ActionSheet: SavePhoto? -> Delete"); });

            // ---- Display Prompt section ----
            prompt_header_.set_text("Display Prompt");
            prompt_header_.set_font(maui::core::font::system_font_of_size(24, maui::core::font_weight::bold));
            question1_button_.set_text("Question 1");
            // C# OnQuestion1ButtonClicked: result -> "Hello {result}." The canned answer is "Maui".
            question1_button_.clicked.connect([this] {
                const std::string result = "Maui"; // DisplayPrompt("Question 1","What's your name?")
                question1_result_.set_text("Hello " + result + ".");
                set_readout("Prompt Question 1: \"What's your name?\" -> " + result);
            });
            question2_button_.set_text("Question 2");
            // C# OnQuestion2ButtonClicked: numeric prompt, "10" -> Correct. The canned answer is "10".
            question2_button_.clicked.connect([this] {
                const std::string result = "10"; // DisplayPrompt("Question 2","What's 5 + 5?", initial "10")
                question2_result_.set_text(result == "10" ? "Correct." : "Incorrect.");
                set_readout("Prompt Question 2: \"What's 5 + 5?\" -> " + result);
            });

            stack_.add(alert_header_);
            stack_.add(alert_simple_);
            stack_.add(alert_yes_no_);
            stack_.add(action_header_);
            stack_.add(action_simple_);
            stack_.add(action_cancel_delete_);
            stack_.add(prompt_header_);
            stack_.add(question1_button_);
            stack_.add(question1_result_);
            stack_.add(question2_button_);
            stack_.add(question2_result_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // ---- owned controls, exposed for the hosting main / tests ----
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }
        [[nodiscard]] maui::controls::button& alert_simple()
        {
            return alert_simple_;
        }
        [[nodiscard]] maui::controls::button& alert_yes_no()
        {
            return alert_yes_no_;
        }
        [[nodiscard]] maui::controls::button& action_simple()
        {
            return action_simple_;
        }
        [[nodiscard]] maui::controls::button& action_cancel_delete()
        {
            return action_cancel_delete_;
        }
        [[nodiscard]] maui::controls::button& question1_button()
        {
            return question1_button_;
        }
        [[nodiscard]] maui::controls::button& question2_button()
        {
            return question2_button_;
        }
        [[nodiscard]] maui::controls::label& question1_result()
        {
            return question1_result_;
        }
        [[nodiscard]] maui::controls::label& question2_result()
        {
            return question2_result_;
        }

    private:
        void set_readout(std::string text)
        {
            readout_.set_text(std::move(text));
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label readout_;
        maui::controls::label alert_header_;
        maui::controls::button alert_simple_;
        maui::controls::button alert_yes_no_;
        maui::controls::label action_header_;
        maui::controls::button action_simple_;
        maui::controls::button action_cancel_delete_;
        maui::controls::label prompt_header_;
        maui::controls::button question1_button_;
        maui::controls::label question1_result_;
        maui::controls::button question2_button_;
        maui::controls::label question2_result_;
    };
} // namespace maui::samples
