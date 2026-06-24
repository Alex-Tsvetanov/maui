#pragma once
// maui::samples::entry_page — ports EntryPage.xaml (Microsoft.Maui.Controls sample gallery).
//
// The MAUI EntryPage is a long ScrollView listing one Entry per property variant (Basic, Disabled,
// TextColor, ClearButtonVisibility, Placeholder/PlaceholderColor, IsPassword + a CheckBox binding,
// IsReadOnly, SpellCheck/TextPrediction, ReturnType, Completed, Keyboard, Horizontal/VerticalText
// Alignment, CursorPosition/SelectionLength driven by Sliders, and the TextChanged→ReturnType
// randomizer). Following the value_controls_page / input_controls_page convention, this is a focused,
// self-contained demo page that exercises the SAME signature properties and events on a small set of
// live entries — each input drives a visible readout — rather than rebuilding the full property catalog.
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same controls directly.
//
// Interactions demonstrated (each mirrors an EntryPage section):
//   - the "basic" entry's text drives a length readout (text_changed → readout),
//   - its TextChanged also steps the ReturnType through the enum, like OnReturnTypeEntryTextChanged
//     (a fixed forward cycle stands in for the C# random pick — deterministic for the demo),
//   - Completed copies the entry text into the readout (the C# OnEntryCompleted DisplayAlert stand-in),
//   - the "password" checkbox flips IsPassword on the password entry (chkIsPassword binding stand-in),
//   - a slider drives CursorPosition on a cursor entry (OnSlideCursorPositionValueChanged stand-in),
//   - static entries show TextColor, Placeholder/PlaceholderColor, IsReadOnly, ClearButtonVisibility,
//     Keyboard=Numeric and HorizontalTextAlignment=End (the catalog rows, set once in the ctor).

#include <cstdio>
#include <string>

#include "maui/controls/check_box.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/slider.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/clear_button_visibility.hpp"
#include "maui/core/keyboard.hpp"
#include "maui/core/return_type.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/graphics/color.hpp"

namespace maui::samples
{
    class entry_page
    {
    public:
        entry_page()
        {
            page_.set_title("Entry");
            stack_.set_spacing(12);

            readout_.set_text("LENGTH: 0  RETURN: Default");

            // "Basic" entry — its text drives the readout, and (like OnReturnTypeEntryTextChanged) each
            // edit steps the ReturnType through the enum so the readout shows it changing while typing.
            basic_.set_placeholder("Type here...");
            basic_.text_changed.connect([this](const std::string& /*old_text*/, const std::string& /*new_text*/) {
                step_return_type();
                update_readout();
            });
            // Completed (OnEntryCompleted): surface the committed text in the readout.
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

            // "Password" — IsPassword toggled by a CheckBox, Keyboard=Numeric (the chkIsPassword section).
            password_.set_text("secret");
            password_.set_keyboard(maui::core::keyboard::numeric());
            password_.set_is_password(true);
            is_password_check_.set_is_checked(true);
            is_password_check_.checked_changed.connect([this](bool checked) { password_.set_is_password(checked); });

            // "Read-only" — Text + IsReadOnly.
            read_only_.set_text("I am read only");
            read_only_.set_is_read_only(true);

            // "ClearButtonVisibility = WhileEditing" — Text + ClearButtonVisibility.
            clear_button_.set_text("Text");
            clear_button_.set_clear_button_visibility(maui::core::clear_button_visibility::while_editing);

            // "HorizontalTextAlignment" — End-aligned text.
            aligned_.set_text("This should be on the end");
            aligned_.set_horizontal_text_alignment(maui::core::text_alignment::end);

            // "CursorPosition = 4" — a slider drives the cursor position over the entry's text
            // (OnSlideCursorPositionValueChanged: entryCursor.CursorPosition = (int)e.NewValue).
            cursor_.set_text("Cursor");
            cursor_.set_cursor_position(4);
            cursor_slider_.set_minimum(0);
            cursor_slider_.set_maximum(static_cast<double>(cursor_.text().size()));
            cursor_slider_.set_value(4);
            cursor_label_.set_text("CursorPosition = 4");
            cursor_slider_.value_changed.connect([this](double /*old_value*/, double new_value) {
                const auto position = static_cast<int>(new_value);
                cursor_.set_cursor_position(position);
                char text[40];
                std::snprintf(text, sizeof(text), "CursorPosition = %d", position);
                cursor_label_.set_text(text);
            });

            stack_.add(readout_);
            stack_.add(basic_);
            stack_.add(text_color_);
            stack_.add(placeholder_);
            stack_.add(is_password_check_);
            stack_.add(password_);
            stack_.add(read_only_);
            stack_.add(clear_button_);
            stack_.add(aligned_);
            stack_.add(cursor_label_);
            stack_.add(cursor_slider_);
            stack_.add(cursor_);
            page_.set_content(stack_);

            // note: EntryPage's BackgroundColor / Background(LinearGradientBrush) / VisualStateManager
            // sections and the Android ImeOptions platform-specific are out of scope for this code-first
            // headless demo — view has no cross-platform set_background_color seam here, so they are
            // omitted rather than faked. The Focus/Unfocus DisplayAlert sections likewise have no
            // headless analog and collapse into the Completed readout above.
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
        [[nodiscard]] maui::controls::entry& basic()
        {
            return basic_;
        }
        [[nodiscard]] maui::controls::entry& password()
        {
            return password_;
        }
        [[nodiscard]] maui::controls::check_box& is_password_check()
        {
            return is_password_check_;
        }
        [[nodiscard]] maui::controls::entry& cursor_entry()
        {
            return cursor_;
        }
        [[nodiscard]] maui::controls::slider& cursor_slider()
        {
            return cursor_slider_;
        }

    private:
        // Cycle the basic entry's ReturnType through the enum on each edit — the deterministic stand-in
        // for OnReturnTypeEntryTextChanged's random pick over the ReturnType names.
        void step_return_type()
        {
            using maui::core::return_type;
            return_type next{};
            switch (basic_.return_type())
            {
                case return_type::default_:
                    next = return_type::done;
                    break;
                case return_type::done:
                    next = return_type::go;
                    break;
                case return_type::go:
                    next = return_type::next;
                    break;
                case return_type::next:
                    next = return_type::search;
                    break;
                case return_type::search:
                    next = return_type::send;
                    break;
                case return_type::send:
                default:
                    next = return_type::default_;
                    break;
            }
            basic_.set_return_type(next);
        }

        void update_readout()
        {
            const char* name = "Default";
            switch (basic_.return_type())
            {
                case maui::core::return_type::done:
                    name = "Done";
                    break;
                case maui::core::return_type::go:
                    name = "Go";
                    break;
                case maui::core::return_type::next:
                    name = "Next";
                    break;
                case maui::core::return_type::search:
                    name = "Search";
                    break;
                case maui::core::return_type::send:
                    name = "Send";
                    break;
                case maui::core::return_type::default_:
                default:
                    name = "Default";
                    break;
            }
            char text[64];
            std::snprintf(text, sizeof(text), "LENGTH: %zu  RETURN: %s", basic_.text().size(), name);
            readout_.set_text(text);
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label readout_;
        maui::controls::entry basic_;
        maui::controls::entry text_color_;
        maui::controls::entry placeholder_;
        maui::controls::check_box is_password_check_;
        maui::controls::entry password_;
        maui::controls::entry read_only_;
        maui::controls::entry clear_button_;
        maui::controls::entry aligned_;
        maui::controls::label cursor_label_;
        maui::controls::slider cursor_slider_;
        maui::controls::entry cursor_;
    };
} // namespace maui::samples
