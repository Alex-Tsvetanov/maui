#pragma once
// maui::samples::editor_page — ports EditorPage.xaml (Microsoft.Maui.Controls sample gallery).
//
// The MAUI EditorPage is a long ScrollView listing one Editor per property variant (Basic, Disabled,
// TextColor, Placeholder/PlaceholderColor, FontSize, CharacterSpacing, IsReadOnly, SpellCheck/Text
// Prediction, Completed, Keyboard, CursorPosition, Horizontal/VerticalTextAlignment, and the
// AutoSize=TextChanges growing editor). Following the value_controls_page / input_controls_page
// convention, this is a focused, self-contained demo page that exercises the SAME signature properties
// and events on a small set of live editors — each input drives a visible readout.
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same controls directly.
//
// Interactions demonstrated (each mirrors an EditorPage section):
//   - the "basic" multiline editor's text drives a character-count readout (text_changed → readout),
//   - Completed copies the editor text into the readout (the C# OnEditorCompleted DisplayAlert stand-in;
//     Editor.SendCompleted has no IsEnabled gate, unlike Entry),
//   - the AutoSize=TextChanges editor's text_changed also reports its length, demonstrating the
//     measure-invalidating auto-size knob is wired,
//   - static editors show TextColor, Placeholder/PlaceholderColor, FontSize (Large via CharacterSpacing
//     and font size), IsReadOnly, Keyboard=Numeric and VerticalTextAlignment=End (the catalog rows).

#include <cstdio>
#include <string>

#include "maui/controls/content_page.hpp"
#include "maui/controls/editor.hpp"
#include "maui/controls/editor_auto_size_option.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/font.hpp"
#include "maui/core/keyboard.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/graphics/color.hpp"

namespace maui::samples
{
    class editor_page
    {
    public:
        editor_page()
        {
            page_.set_title("Editor");
            stack_.set_spacing(12);

            readout_.set_text("LENGTH: 0");

            // "Basic" editor — the multiline input; its text drives the readout (text_changed).
            basic_.set_placeholder("Type here...");
            basic_.text_changed.connect(
                [this](const std::string& /*old_text*/, const std::string& /*new_text*/) { update_readout(); });
            // Completed (OnEditorCompleted): surface the committed text in the readout.
            basic_.completed.connect([this] {
                std::string done = "COMPLETED: ";
                done += basic_.text();
                readout_.set_text(done);
            });

            // "Using TextColor" — Text + TextColor (Purple in the XAML).
            text_color_.set_text("Text");
            text_color_.set_text_color(maui::graphics::color::from_rgb(128, 0, 128)); // Purple

            // "With Placeholder" / "Using PlaceholderColor" — Placeholder + PlaceholderColor (Purple).
            placeholder_.set_placeholder("Placeholder");
            placeholder_.set_placeholder_color(maui::graphics::color::from_rgb(128, 0, 128));

            // "FontSize" + "CharacterSpacing" — a large font with extra letter spacing (the two font
            // sections folded into one editor: FontSize=Large, CharacterSpacing=4).
            font_.set_text("FontSize (Large)");
            font_.set_placeholder("FontSize (Large)");
            font_.set_font(maui::core::font::system_font_of_size(24.0));
            font_.set_character_spacing(4.0);

            // "Read-only" — Text + IsReadOnly.
            read_only_.set_text("I am read only");
            read_only_.set_is_read_only(true);

            // "Keyboard Numeric" — Text + Keyboard.
            numeric_.set_text("123");
            numeric_.set_keyboard(maui::core::keyboard::numeric());

            // "VerticalTextAlignment" — End-aligned text.
            aligned_.set_text("This should be on the bottom");
            aligned_.set_vertical_text_alignment(maui::core::text_alignment::end);

            // "Auto Size with Text Changes" — AutoSize=TextChanges; its edits report the length so the
            // measure-invalidating knob is visibly exercised.
            auto_size_.set_placeholder("Grows as you type...");
            auto_size_.set_auto_size(maui::controls::editor_auto_size_option::text_changes);
            auto_size_.text_changed.connect([this](const std::string& /*old_text*/, const std::string& new_text) {
                char text[48];
                std::snprintf(text, sizeof(text), "AUTOSIZE LENGTH: %zu", new_text.size());
                auto_size_readout_.set_text(text);
            });
            auto_size_readout_.set_text("AUTOSIZE LENGTH: 0");

            stack_.add(readout_);
            stack_.add(basic_);
            stack_.add(text_color_);
            stack_.add(placeholder_);
            stack_.add(font_);
            stack_.add(read_only_);
            stack_.add(numeric_);
            stack_.add(aligned_);
            stack_.add(auto_size_readout_);
            stack_.add(auto_size_);
            page_.set_content(stack_);

            // note: EditorPage's BackgroundColor / Background(LinearGradientBrush) sections are out of
            // scope for this code-first headless demo — view has no cross-platform set_background_color
            // seam here, so they are omitted rather than faked. The Focus/Unfocus DisplayAlert sections
            // have no headless analog and collapse into the Completed readout above.
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
        [[nodiscard]] maui::controls::editor& basic()
        {
            return basic_;
        }
        [[nodiscard]] maui::controls::editor& auto_size_editor()
        {
            return auto_size_;
        }

    private:
        void update_readout()
        {
            char text[48];
            std::snprintf(text, sizeof(text), "LENGTH: %zu", basic_.text().size());
            readout_.set_text(text);
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label readout_;
        maui::controls::editor basic_;
        maui::controls::editor text_color_;
        maui::controls::editor placeholder_;
        maui::controls::editor font_;
        maui::controls::editor read_only_;
        maui::controls::editor numeric_;
        maui::controls::editor aligned_;
        maui::controls::label auto_size_readout_;
        maui::controls::editor auto_size_;
    };
} // namespace maui::samples
