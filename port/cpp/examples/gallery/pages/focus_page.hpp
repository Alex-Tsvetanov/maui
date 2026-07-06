#pragma once
// maui::samples::focus_page — ports FocusPage.xaml (+ FocusPage.xaml.cs)
//
// The C# FocusPage is a focus-subsystem demo: an Entry whose Focused/Unfocused events
// (OnFocusEntryFocusChanged) append "Focused"/"Unfocused" lines to a scrolling InfoLabel, plus two
// buttons — "Focus Entry" calls FocusEntry.Focus(), "Unfocus Entry" calls FocusEntry.Unfocus().
//
// In the port the Y1 focus subsystem lives on view: focus() / unfocus() drive the handler's
// Focus/Unfocus commands (view_command_mapper::map_focus/map_unfocus → view_focus_ops, which the
// headless backend realizes — focus always succeeds, returning true), set_is_focused() flips the state
// and raises the focused/unfocused events, and is_focused() reports it. This page wires exactly that:
// the two buttons call entry.focus()/entry.unfocus(), the entry's focused/unfocused events append to the
// info log and refresh an IsFocused readout — the same observable behavior as the C# page.
//
// note: the XAML uses a Grid (RowDefinitions) purely for placement; the demonstrated behavior is
// layout-independent, so this port uses a vertical_stack_layout (headless-safe, same control set). The
// inner ScrollView around InfoLabel is preserved.
//
// The page OWNS its whole element tree (the sample_app pattern). The generic mount (app_host.hpp) attaches
// every owned view's handler and hosts the ctor-built tree.

#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/thickness.hpp"

namespace maui::samples
{
    class focus_page
    {
    public:
        focus_page()
        {
            page_.set_title("Focus");
            stack_.set_spacing(10);
            // XAML: <VerticalStackLayout Spacing="10" Padding="12"> — the builder previously left the
            // root Padding unset (default 0), so the whole page rendered flush against the top-left
            // edge, offset ~12pt/18px from the XAML twin (a Cluster-B-style root-layout divergence).
            stack_.set_padding(maui::core::thickness(12));

            // ---- the focus target: an entry whose focus events append to the info log ----
            focus_entry_.set_placeholder("Focus target");
            // C# OnFocusEntryFocusChanged: InfoLabel.Text += IsFocused ? "Focused\n" : "Unfocused\n".
            focus_entry_.focused.connect([this](bool /*is_focused*/) { append_log("Focused"); });
            focus_entry_.unfocused.connect([this](bool /*is_focused*/) { append_log("Unfocused"); });

            status_.set_text("IsFocused: false");

            // ---- the two control buttons, side by side (XAML horizontal StackLayout) ----
            focus_button_.set_text("Focus Entry");
            // C# OnFocusClicked: FocusEntry.Focus().
            focus_button_.clicked.connect([this] {
                focus_entry_.focus();
                refresh_status();
            });
            unfocus_button_.set_text("Unfocus Entry");
            // C# OnUnfocusClicked: FocusEntry.Unfocus().
            unfocus_button_.clicked.connect([this] {
                focus_entry_.unfocus();
                refresh_status();
            });
            buttons_.set_spacing(8);
            buttons_.add(focus_button_);
            buttons_.add(unfocus_button_);

            // ---- the scrolling info log (XAML InfoLabel inside a ScrollView) ----
            info_label_.set_text("");

            stack_.add(focus_entry_);
            stack_.add(buttons_);
            stack_.add(status_);
            info_scroller_.set_content(info_label_);
            stack_.add(info_scroller_);
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
        [[nodiscard]] maui::controls::entry& focus_entry()
        {
            return focus_entry_;
        }
        [[nodiscard]] maui::controls::button& focus_button()
        {
            return focus_button_;
        }
        [[nodiscard]] maui::controls::button& unfocus_button()
        {
            return unfocus_button_;
        }
        [[nodiscard]] maui::controls::label& status()
        {
            return status_;
        }
        [[nodiscard]] maui::controls::label& info_label()
        {
            return info_label_;
        }
        [[nodiscard]] maui::controls::scroll_view& info_scroller()
        {
            return info_scroller_;
        }

    private:
        // C# OnFocusEntryFocusChanged: append the focus-state line to the running info log.
        void append_log(const std::string& line)
        {
            log_ += line;
            log_ += "\n";
            info_label_.set_text(log_);
            refresh_status();
        }

        void refresh_status()
        {
            status_.set_text(std::string("IsFocused: ") + (focus_entry_.is_focused() ? "true" : "false"));
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::entry focus_entry_;
        maui::controls::horizontal_stack_layout buttons_;
        maui::controls::button focus_button_;
        maui::controls::button unfocus_button_;
        maui::controls::label status_;
        maui::controls::scroll_view info_scroller_;
        maui::controls::label info_label_;
        std::string log_; // the accumulated InfoLabel.Text
    };
} // namespace maui::samples
