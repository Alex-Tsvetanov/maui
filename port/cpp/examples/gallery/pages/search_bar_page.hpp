#pragma once
// maui::samples::search_bar_page — ports SearchBarPage.xaml (Microsoft.Maui.Controls sample gallery).
//
// The MAUI SearchBarPage is a long ScrollView listing one SearchBar per property variant (Default,
// Disabled, TextColor, Placeholder/PlaceholderColor, Fonts (FontSize + Italic), SearchCommand,
// Horizontal/VerticalTextAlignment, SpellCheck/TextPrediction, Focus/Unfocus, TextChanged,
// BackgroundColor/Background, CancelButtonColor, Keyboard). Following the value_controls_page /
// input_controls_page convention, this is a focused, self-contained demo page that exercises the SAME
// signature properties and events on a small set of live search bars — each input drives a readout.
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same controls directly.
//
// Interactions demonstrated (each mirrors a SearchBarPage section):
//   - the "default" bar's text drives a length readout (text_changed → readout, OnSearchBarTextChanged),
//   - pressing Search runs the search_command then raises search_button_pressed; the command increments
//     a counter and the readout echoes the committed query (the C# SearchCommand/SearchCommandParameter
//     binding — collapsed to the port's command+event channel, see search_bar.hpp),
//   - static bars show TextColor (Green), Placeholder/PlaceholderColor (Pink), a 24pt Italic font,
//     CancelButtonColor (Red), HorizontalTextAlignment=End and Keyboard=Numeric (the catalog rows).

#include <cstdio>
#include <string>

#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/search_bar.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/font.hpp"
#include "maui/core/keyboard.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"

namespace maui::samples
{
    class search_bar_page
    {
    public:
        search_bar_page()
        {
            page_.set_title("SearchBar");
            stack_.set_spacing(12);
            // XAML: <VerticalStackLayout Spacing="12" Padding="16"> — the builder previously left the
            // root Padding unset (default 0), so every row rendered flush against the edges, offset from
            // the XAML twin by the padding amount (a Cluster-B-style root-layout divergence, same class
            // as the already-documented search_bar finding in EQUIVALENCE_FINDINGS.md).
            stack_.set_padding(maui::core::thickness(16));

            readout_.set_text("LENGTH: 0  SEARCHES: 0");

            // "Default" bar — its text drives the readout (OnSearchBarTextChanged), and the Search action
            // (SearchCommand + OnSearchButtonPressed) bumps the counter and echoes the committed query.
            default_.set_placeholder("Search...");
            default_.text_changed.connect(
                [this](const std::string& /*old_text*/, const std::string& /*new_text*/) { update_readout(); });
            default_.search_command = [this] { ++search_count_; };
            default_.search_button_pressed.connect([this] {
                std::string done = "SEARCHED: ";
                done += default_.text();
                readout_.set_text(done);
            });

            // "TextColor" — TextColor (Green in the XAML).
            text_color_.set_text("Green text");
            text_color_.set_text_color(maui::graphics::color::from_rgb(0, 128, 0)); // Green

            // "With Placeholder" / "Using PlaceholderColor" — Placeholder + PlaceholderColor (Pink).
            placeholder_.set_placeholder("Placeholder");
            placeholder_.set_placeholder_color(maui::graphics::color::from_rgb(255, 192, 203)); // Pink

            // "Fonts" — FontSize=24 + FontAttributes=Italic (one bar folding both font knobs).
            font_.set_text("Italic 24pt");
            font_.set_font(maui::core::font::system_font_of_size(24.0, maui::core::font_weight::regular,
                                                                 maui::core::font_slant::italic));

            // "HorizontalTextAlignment" — End-aligned placeholder + text.
            aligned_.set_placeholder("end of the line");
            aligned_.set_text("end of the line");
            aligned_.set_horizontal_text_alignment(maui::core::text_alignment::end);

            // "CancelButtonColor" — Red cancel button.
            cancel_color_.set_text("Cancel is red");
            cancel_color_.set_cancel_button_color(maui::graphics::color::from_rgb(255, 0, 0)); // Red

            // "Keyboard" — Numeric keyboard.
            numeric_.set_keyboard(maui::core::keyboard::numeric());
            numeric_.set_placeholder("Numeric keyboard");

            stack_.add(readout_);
            stack_.add(default_);
            stack_.add(text_color_);
            stack_.add(placeholder_);
            stack_.add(font_);
            stack_.add(aligned_);
            stack_.add(cancel_color_);
            stack_.add(numeric_);
            page_.set_content(stack_);

            // note: SearchBarPage's BackgroundColor / Background(LinearGradientBrush) sections are out of
            // scope for this code-first headless demo — view has no cross-platform set_background_color
            // seam here, so they are omitted rather than faked. The Focus/Unfocus DisplayAlert sections
            // have no headless analog and collapse into the search readout above.
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
        [[nodiscard]] maui::controls::search_bar& default_bar()
        {
            return default_;
        }

    private:
        void update_readout()
        {
            char text[64];
            std::snprintf(text, sizeof(text), "LENGTH: %zu  SEARCHES: %d", default_.text().size(), search_count_);
            readout_.set_text(text);
        }

        int search_count_ = 0;

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label readout_;
        maui::controls::search_bar default_;
        maui::controls::search_bar text_color_;
        maui::controls::search_bar placeholder_;
        maui::controls::search_bar font_;
        maui::controls::search_bar aligned_;
        maui::controls::search_bar cancel_color_;
        maui::controls::search_bar numeric_;
    };
} // namespace maui::samples
