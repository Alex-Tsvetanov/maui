#pragma once
// maui::samples::ios_first_responder_page — ports iOSFirstResponderPage.xaml (+ .xaml.cs)
//
// The C# iOSFirstResponderPage is a VisualElement-first-responder demo: a StackLayout with an
// explanatory Label, a "First Entry" + plain "OK" Button (tapping OK dismisses the keyboard because the
// button is NOT eligible to become first responder), then a second explanatory Label, a "Second Entry"
// + an "OK" Button carrying ios:VisualElement.CanBecomeFirstResponder="True" (tapping it does NOT dismiss
// the keyboard, because the button itself can take first-responder status). The whole point is the
// iOSSpecific VisualElement.CanBecomeFirstResponder attached property, applied to the second button.
//
// In the port two things are exercised, both HEADLESS-SAFE:
//   1. The focus subsystem on view (Y1): entry.focus() / entry.unfocus() drive the handler's
//      Focus/Unfocus commands (with a handler attached, focus() always succeeds headless — see view.hpp),
//      set_is_focused() flips state and raises focused/unfocused, is_focused() reports it. This page adds
//      explicit "Focus" / "Unfocus" buttons per entry plus an IsFocused readout so the focus subsystem is
//      observable on a static headless capture — the keyboard appear/dismiss behavior the C# page narrates
//      has no headless analogue, so focus()/unfocus() stand in for it (the same VisualElement contract the
//      C# keyboard show/hide rides on).
//   2. The iOSSpecific knob itself: ios_specific::visual_element::set_can_become_first_responder applied to
//      the second OK button through button.on<ios>() — exactly the C# attached property. The knob is a
//      STORED platform-spec at this layer (the actual UIView.canBecomeFirstResponder override lives on the
//      iOS backend); we set it AND read it back into a readout so the surface is demonstrably wired.
//
// note: the C# "OK" buttons have no Clicked code-behind (their effect is purely the native keyboard
//       dismissal contract). The port keeps the two OK buttons faithful to the XAML (text + the iOSSpecific
//       knob on the second), and adds the Focus/Unfocus controls + readouts as the headless-observable
//       stand-in for the keyboard behavior — never inventing C# logic that isn't there.
//
// The page OWNS its whole element tree (the sample_app pattern). attach_handlers wires every owned view
// bottom-up via the shared gallery helpers and re-hosts the ctor-built tree.

#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/thickness.hpp"
#include "maui/hosting/maui_app.hpp"

#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/ios_specific/visual_element.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class ios_first_responder_page
    {
    public:
        ios_first_responder_page()
        {
            namespace pc = maui::controls::platform_configuration;
            namespace ios_ve = pc::ios_specific::visual_element;

            page_.set_title("VisualElement first responder");
            stack_.set_margin(maui::core::thickness(10)); // XAML StackLayout Margin="10"
            stack_.set_spacing(10);

            // ---- First block: explanatory label + First Entry + plain OK button ----
            // C#: "Click in the first Entry ... click OK and the keyboard should disappear."
            first_caption_.set_text(
                "Click in the first Entry to make the keyboard appear. Then click OK and the keyboard should "
                "disappear.");
            first_entry_.set_placeholder("First Entry");
            // C# Focused/Unfocused observable wiring (the keyboard show/hide rides on VisualElement focus).
            first_entry_.focused.connect([this](bool) { refresh_status(); });
            first_entry_.unfocused.connect([this](bool) { refresh_status(); });
            first_ok_.set_text("OK");
            // The plain OK button: NOT a first responder — tapping it resigns the entry's focus (the C#
            // "keyboard should disappear" path). Headless stand-in: unfocus the first entry.
            first_ok_.clicked.connect([this] {
                first_entry_.unfocus();
                refresh_status();
            });

            // ---- Second block: explanatory label + Second Entry + OK button with the iOSSpecific knob ----
            // C#: "... click OK and the keyboard shouldn't disappear."
            second_caption_.set_text(
                "Click in the second Entry to make the keyboard appear. Then click OK and the keyboard "
                "shouldn't disappear.");
            second_entry_.set_placeholder("Second Entry");
            second_entry_.focused.connect([this](bool) { refresh_status(); });
            second_entry_.unfocused.connect([this](bool) { refresh_status(); });
            second_ok_.set_text("OK");
            // THE demonstrated attached property: ios:VisualElement.CanBecomeFirstResponder="True" on the
            // second OK button (button.on<ios>() mints the config the knob set chains on). Because this
            // button can itself become first responder, tapping it does NOT resign the entry's focus — so
            // the headless stand-in deliberately leaves second_entry_'s focus untouched (no unfocus()),
            // mirroring "the keyboard shouldn't disappear".
            ios_ve::set_can_become_first_responder(second_ok_.on<pc::ios>(), true);

            // ---- Per-entry Focus / Unfocus controls (the headless-observable focus-subsystem stand-in) ----
            focus_first_.set_text("Focus First");
            focus_first_.clicked.connect([this] {
                first_entry_.focus();
                refresh_status();
            });
            focus_second_.set_text("Focus Second");
            focus_second_.clicked.connect([this] {
                second_entry_.focus();
                refresh_status();
            });
            focus_buttons_.set_spacing(8);
            focus_buttons_.add(focus_first_);
            focus_buttons_.add(focus_second_);

            refresh_status();

            stack_.add(first_caption_);
            stack_.add(first_entry_);
            stack_.add(first_ok_);
            stack_.add(second_caption_);
            stack_.add(second_entry_);
            stack_.add(second_ok_);
            stack_.add(focus_buttons_);
            stack_.add(status_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view BOTTOM-UP (leaves before their hosts), then re-host the
        // ctor-built tree. The entries' handlers MUST be attached for focus()/unfocus() to realize (with no
        // handler, focus() returns false — see view.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, first_caption_, "first_caption_");
            gallery_attach_one(app, first_entry_, "first_entry_");
            gallery_attach_one(app, first_ok_, "first_ok_");
            gallery_attach_one(app, second_caption_, "second_caption_");
            gallery_attach_one(app, second_entry_, "second_entry_");
            gallery_attach_one(app, second_ok_, "second_ok_");
            gallery_attach_one(app, focus_first_, "focus_first_");
            gallery_attach_one(app, focus_second_, "focus_second_");
            gallery_attach_one(app, focus_buttons_, "focus_buttons_");
            gallery_attach_one(app, status_, "status_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(focus_buttons_); // the focus button row hosts its two buttons
            gallery_rehost_layout(stack_);         // outer stack hosts captions/entries/buttons/status
            gallery_rehost_content(page_);         // page hosts the stack
        }

        // ---- owned controls, exposed for the hosting main / tests ----
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::entry& first_entry()
        {
            return first_entry_;
        }
        [[nodiscard]] maui::controls::entry& second_entry()
        {
            return second_entry_;
        }
        [[nodiscard]] maui::controls::button& first_ok()
        {
            return first_ok_;
        }
        [[nodiscard]] maui::controls::button& second_ok()
        {
            return second_ok_;
        }
        [[nodiscard]] maui::controls::button& focus_first()
        {
            return focus_first_;
        }
        [[nodiscard]] maui::controls::button& focus_second()
        {
            return focus_second_;
        }
        [[nodiscard]] maui::controls::label& status()
        {
            return status_;
        }

        // The iOSSpecific knob readback — true on the second OK button, false (default) on the first.
        [[nodiscard]] bool second_ok_can_become_first_responder() const
        {
            namespace ios_ve = maui::controls::platform_configuration::ios_specific::visual_element;
            return ios_ve::get_can_become_first_responder(second_ok_);
        }

    private:
        void refresh_status()
        {
            namespace ios_ve = maui::controls::platform_configuration::ios_specific::visual_element;
            std::string text;
            text += "First IsFocused: ";
            text += first_entry_.is_focused() ? "true" : "false";
            text += "\nSecond IsFocused: ";
            text += second_entry_.is_focused() ? "true" : "false";
            text += "\nSecond OK CanBecomeFirstResponder: ";
            text += ios_ve::get_can_become_first_responder(second_ok_) ? "true" : "false";
            status_.set_text(text);
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;

        maui::controls::label first_caption_;
        maui::controls::entry first_entry_;
        maui::controls::button first_ok_;

        maui::controls::label second_caption_;
        maui::controls::entry second_entry_;
        maui::controls::button second_ok_;

        maui::controls::horizontal_stack_layout focus_buttons_;
        maui::controls::button focus_first_;
        maui::controls::button focus_second_;

        maui::controls::label status_;
    };
} // namespace maui::samples
