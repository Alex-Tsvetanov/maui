#pragma once
// maui::samples::semantics_page — ports SemanticsPage.xaml (+ SemanticsPage.xaml.cs)
//
// The C# SemanticsPage is an accessibility showcase: a long VerticalStackLayout where nearly every
// control carries SemanticProperties.Description / .Hint, plus a block of labels exercising
// SemanticProperties.HeadingLevel (Level1..Level4), and two layouts carrying a layout-level
// Description. A "set semantic focus" button calls semanticFocusLabel.SetSemanticFocus().
//
// In the port, SemanticProperties.* map onto view::set_semantics(shared_ptr<core::semantics>) — the
// semantics object carries description(), hint() and heading_level(); the shared view_mapper pushes it
// to the platform base (headless mirrors it; the native NSView.accessibilityLabel/Help push is deferred
// — see core/semantics.hpp). This page ports a faithful, representative subset of the C# tree (the full
// page repeats the same Description/Hint pattern across ~30 controls): the TH/DH labels, the TH/DH
// buttons, a DTH entry + editor + a DH search bar, the four HeadingLevel labels, and two layout-level
// descriptions — every control's semantics set exactly as the XAML attached properties specify. A
// readout label echoes the semantics of the currently-highlighted control.
//
// note: SetSemanticFocus() (SemanticExtensions.SetSemanticFocus) is a native accessibility focus call
// with no headless equivalent; the focus button's handler instead echoes the target label's semantics
// into the readout (the faithful stand-in for "assistive tech is now focused on this element").
//
// The page OWNS its whole element tree (the sample_app pattern). The generic mount attaches every owned view's
// handler bottom-up and re-hosts the ctor-built tree.

#include <memory>
#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/editor.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/search_bar.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/semantics.hpp"

namespace maui::samples
{
    class semantics_page
    {
    public:
        semantics_page()
        {
            page_.set_title("Semantics");
            stack_.set_spacing(10);

            intro_.set_text("SemanticProperties showcase: Description (D), Hint (H), HeadingLevel.");
            readout_.set_text("Semantics readout: (interact to inspect)");

            // ---- text labels: T+Hint, then D+Hint (XAML "Label text TH" / "Label text DH") ----
            label_th_.set_text("Label text TH");
            set_semantics(label_th_, /*description*/ "", /*hint*/ "Hint text");
            label_dh_.set_text("Label text DH");
            set_semantics(label_dh_, "Description text", "Hint text");

            // ---- buttons: T+Hint, then D+Hint ("Button text TH" / "Button text DH") ----
            button_th_.set_text("Button text TH");
            set_semantics(button_th_, "", "Hint text");
            button_th_.clicked.connect([this] { echo_semantics("Button text TH", button_th_); });
            button_dh_.set_text("Button text DH");
            set_semantics(button_dh_, "Description text", "Hint text");
            button_dh_.clicked.connect([this] { echo_semantics("Button text DH", button_dh_); });

            // ---- DTH entry + editor, DH search bar ----
            entry_dth_.set_text("Entry text, DTH");
            set_semantics(entry_dth_, "Description text", "Hint text");
            editor_dth_.set_text("Editor text, DTH");
            set_semantics(editor_dth_, "Description text", "Hint text");
            search_dh_.set_text("Search bar text, DH");
            search_dh_.set_placeholder("Placeholder text");
            set_semantics(search_dh_, "Description text", "Hint text");

            // ---- HeadingLevel block (Level1..Level4) ----
            heading_intro_.set_text("HeadingLevel labels (assistive tech navigates by heading):");
            heading1_.set_text("Heading 1");
            set_heading(heading1_, maui::core::semantic_heading_level::level1);
            heading2_.set_text("Heading 2");
            set_heading(heading2_, maui::core::semantic_heading_level::level2);
            heading3_.set_text("Heading 3");
            set_heading(heading3_, maui::core::semantic_heading_level::level3);
            heading4_.set_text("Heading 4 with semantic description and hint");
            {
                auto sem = std::make_shared<maui::core::semantics>();
                sem->set_heading_level(maui::core::semantic_heading_level::level4);
                sem->set_description("Description");
                sem->set_hint("Hint");
                heading4_.set_semantics(std::move(sem));
            }

            // ---- layout-level descriptions (VerticalStackLayout SemanticProperties.Description) ----
            layout_desc_intro_.set_text("Layouts can carry a Description too:");
            set_semantics(described_layout_, "Layout description text", "");
            described_label_a_.set_text("Label in StackLayout with semantic description");
            described_label_b_.set_text("Label in StackLayout with semantic description");
            described_layout_.set_spacing(4);
            described_layout_.add(described_label_a_);
            described_layout_.add(described_label_b_);

            // ---- semantic-focus button (XAML SetSemanticFocusButton_Clicked) ----
            focus_label_.set_text("Label receiving semantic focus");
            focus_button_.set_text("Click to set semantic focus to label below");
            // SetSemanticFocus() has no headless equivalent; echo the target's semantics instead (note above).
            focus_button_.clicked.connect([this] { echo_semantics("focus label", focus_label_); });
            set_semantics(focus_label_, "Label receiving semantic focus", "");

            stack_.add(intro_);
            stack_.add(readout_);
            stack_.add(label_th_);
            stack_.add(label_dh_);
            stack_.add(button_th_);
            stack_.add(button_dh_);
            stack_.add(entry_dth_);
            stack_.add(editor_dth_);
            stack_.add(search_dh_);
            stack_.add(heading_intro_);
            stack_.add(heading1_);
            stack_.add(heading2_);
            stack_.add(heading3_);
            stack_.add(heading4_);
            stack_.add(layout_desc_intro_);
            stack_.add(described_layout_);
            stack_.add(focus_button_);
            stack_.add(focus_label_);

            scroller_.set_content(stack_);
            page_.set_content(scroller_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // ---- owned controls, exposed for the hosting main / tests ----
        [[nodiscard]] maui::controls::scroll_view& scroller()
        {
            return scroller_;
        }
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }
        [[nodiscard]] maui::controls::label& label_dh()
        {
            return label_dh_;
        }
        [[nodiscard]] maui::controls::button& button_dh()
        {
            return button_dh_;
        }
        [[nodiscard]] maui::controls::entry& entry_dth()
        {
            return entry_dth_;
        }
        [[nodiscard]] maui::controls::label& heading4()
        {
            return heading4_;
        }
        [[nodiscard]] maui::controls::vertical_stack_layout& described_layout()
        {
            return described_layout_;
        }
        [[nodiscard]] maui::controls::button& focus_button()
        {
            return focus_button_;
        }
        [[nodiscard]] maui::controls::label& focus_label()
        {
            return focus_label_;
        }

    private:
        // Attach a fresh semantics object (description + hint) to a view — the SemanticProperties.Description
        // / .Hint attached-property pair.
        template <class View> static void set_semantics(View& view, std::string description, std::string hint)
        {
            auto sem = std::make_shared<maui::core::semantics>();
            sem->set_description(std::move(description));
            sem->set_hint(std::move(hint));
            view.set_semantics(std::move(sem));
        }

        // Attach a heading-level-only semantics object — the SemanticProperties.HeadingLevel attached prop.
        static void set_heading(maui::controls::label& view, maui::core::semantic_heading_level level)
        {
            auto sem = std::make_shared<maui::core::semantics>();
            sem->set_heading_level(level);
            view.set_semantics(std::move(sem));
        }

        // Echo a view's semantics into the readout (the SetSemanticFocus stand-in / button-inspection).
        template <class View> void echo_semantics(const std::string& label, const View& view)
        {
            std::string text = "Semantics readout [" + label + "]: ";
            if (const auto* sem = view.semantics())
            {
                const std::string& desc = sem->description();
                const std::string& hint = sem->hint();
                text += "D=\"" + (desc.empty() ? std::string("(none)") : desc) + "\" ";
                text += "H=\"" + (hint.empty() ? std::string("(none)") : hint) + "\"";
                if (sem->is_heading())
                {
                    text += " heading-level=" + std::to_string(static_cast<int>(sem->heading_level()));
                }
            }
            else
            {
                text += "(no semantics set)";
            }
            readout_.set_text(text);
        }

        maui::controls::content_page page_;
        maui::controls::scroll_view scroller_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label intro_;
        maui::controls::label readout_;
        maui::controls::label label_th_;
        maui::controls::label label_dh_;
        maui::controls::button button_th_;
        maui::controls::button button_dh_;
        maui::controls::entry entry_dth_;
        maui::controls::editor editor_dth_;
        maui::controls::search_bar search_dh_;
        maui::controls::label heading_intro_;
        maui::controls::label heading1_;
        maui::controls::label heading2_;
        maui::controls::label heading3_;
        maui::controls::label heading4_;
        maui::controls::label layout_desc_intro_;
        maui::controls::vertical_stack_layout described_layout_;
        maui::controls::label described_label_a_;
        maui::controls::label described_label_b_;
        maui::controls::button focus_button_;
        maui::controls::label focus_label_;
    };
} // namespace maui::samples
