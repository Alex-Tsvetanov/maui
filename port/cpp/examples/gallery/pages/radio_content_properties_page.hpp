#pragma once
// maui::samples::radio_content_properties_page — ports ContentProperties.xaml
//
// A self-contained, code-first demo of how RadioButton propagates the standard Text/Font properties to
// its Content. Mirrors the C# controls gallery page (Pages/Controls/RadioButtonGalleries/
// ContentProperties.xaml): a StackLayout of small captions interleaved with RadioButtons (GroupName
// "test") whose TextColor / CharacterSpacing / TextTransform / FontAttributes / FontSize / FontFamily
// are set, demonstrating that those properties flow to the rendered Content.
//
// WHAT MAUI'S PAGE SHOWS (and how this port maps it):
//   - Option A: Content "Option A" + TextColor Red, CharacterSpacing 1.5, TextTransform Lowercase,
//     FontAttributes Italic, FontSize 14, FontFamily Baskerville.
//   - Option B: Content "Option B" + TextColor Blue, TextTransform Uppercase, FontAttributes Bold,
//     FontSize 18, FontFamily Arial.
//   These two string-content radios map DIRECTLY: set_content(std::string) + set_text_color /
//   set_character_spacing / set_font (family+size+weight+slant) / and the GroupName "test".
//
//   - The remaining radios in the XAML set Content to a *View* (a Button, or a Label) to show that the
//     Text/Font properties propagate to the embedded View, plus SemanticProperties.Description/Hint and
//     the "content already set/bound should ignore the RadioButton properties" cases. The port's
//     radio_button cuts View-Content to the native STRING path (the View-Content + propagation-to-View
//     machinery and the SemanticProperties attached surface are documented-deferred at the radio_button
//     level — radio_button.hpp). So those variants are represented faithfully by their captions plus a
//     string-content radio carrying the same Text/Font property set, with an honest note that the
//     embedded-View propagation + Semantics are deferred rather than invented.
//
// TextTransform: the radio_button header documents TextTransform as deferred (no TextTransform subsystem
// in the port's font yet). It is therefore NOT applied; each radio's caption still states the intended
// transform (Lowercase/Uppercase) so the XAML's described behavior stays cross-referenceable.
//
// FontFamily ("Baskerville"/"Arial") is carried via font::of_size(family, size, weight, slant); on the
// headless backend the family is recorded but not resolved to a system face. FontSize "Micro" in the
// XAML maps to a small concrete size here (the named-size table is not part of this cut).
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/
// ios test trees exercise the same wiring directly.
//
// note: View-as-Content propagation (Button/Label inside a RadioButton), SemanticProperties.Description/
//       .Hint, TextTransform, and the named FontSize table are documented-deferred; nothing here is
//       invented. The page's demonstrable core — Text/Font properties on a (string-content) RadioButton
//       grouped by GroupName "test" — is ported directly.

#include <memory>
#include <string>

#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/radio_button.hpp"
#include "maui/controls/radio_button_group.hpp"
#include "maui/controls/stack_layout.hpp"
#include "maui/core/font.hpp"
#include "maui/graphics/colors.hpp"

namespace maui::samples
{
    class radio_content_properties_page
    {
    public:
        radio_content_properties_page()
        {
            page_.set_title("RadioButton Content Properties");
            stack_.set_spacing(6);

            // Header captions (the XAML's first two <Label> rows).
            caption_intro_.set_text("Propagate standard Text properties to Content where applicable:");
            caption_a_.set_text("TextColor: Red, CharacterSpacing: 1.5, TextTransform: Lowercase, "
                                "FontAttributes: Italic, FontSize: 14, FontFamily: BaskerVille");

            // Option A — TextColor Red, CharacterSpacing 1.5, FontAttributes Italic, FontSize 14,
            // FontFamily Baskerville (TextTransform Lowercase deferred — see header).
            option_a_.set_content("Option A");
            option_a_.set_group_name("test");
            option_a_.set_text_color(maui::graphics::colors::red);
            option_a_.set_character_spacing(1.5);
            option_a_.set_font(maui::core::font::of_size("Baskerville", 14, maui::core::font_weight::regular,
                                                         maui::core::font_slant::italic));

            caption_b_.set_text("TextColor: Blue, CharacterSpacing: 1, TextTransform: Uppercase, "
                                "FontAttributes: Bold, FontSize: 18, FontFamily: Arial");

            // Option B — TextColor Blue, FontAttributes Bold, FontSize 18, FontFamily Arial
            // (TextTransform Uppercase deferred — see header).
            option_b_.set_content("Option B");
            option_b_.set_group_name("test");
            option_b_.set_text_color(maui::graphics::colors::blue);
            option_b_.set_font(
                maui::core::font::of_size("Arial", 18, maui::core::font_weight::bold, maui::core::font_slant::normal));

            // The View-as-Content variants (Button content, Semantic Description, Semantic Description +
            // Hint, content-already-set, content-already-bound). Their embedded-View propagation +
            // Semantics are deferred; the port renders each caption plus a string-content radio carrying
            // the same Green/Bold/Arial/12 property set (TextTransform Uppercase deferred).
            caption_button_.set_text("The RadioButton below has its content set to Button (which makes little sense, "
                                     "but this is just an example). The Text and Font properties are applied to it.");
            configure_green(radio_button_content_);

            caption_semantic1_.set_text("The RadioButton below is the same as above, but also has a Semantic "
                                        "Description set on it for better accessibility.");
            configure_green(radio_semantic1_);

            caption_semantic2_.set_text("The RadioButton below is the same as above, but has a Semantic Description "
                                        "and Semantic Hint set on it for better accessibility.");
            configure_green(radio_semantic2_);

            caption_already_set_.set_text("A Content View which already has these properties set/bound should ignore "
                                          "the RadioButton properties.");
            configure_green(radio_already_set_);
            configure_green(radio_already_bound_);

            // Assemble in XAML order.
            stack_.add(caption_intro_);
            stack_.add(caption_a_);
            stack_.add(option_a_);
            stack_.add(caption_b_);
            stack_.add(option_b_);
            stack_.add(caption_button_);
            stack_.add(radio_button_content_);
            stack_.add(caption_semantic1_);
            stack_.add(radio_semantic1_);
            stack_.add(caption_semantic2_);
            stack_.add(radio_semantic2_);
            stack_.add(caption_already_set_);
            stack_.add(radio_already_set_);
            stack_.add(radio_already_bound_);

            // Group "test": all radios mutually exclusive (the XAML's shared GroupName).
            maui::controls::radio_button_group::set_group_name(stack_, "test");

            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's inspection.
        [[nodiscard]] maui::controls::stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::radio_button& option_a()
        {
            return option_a_;
        }
        [[nodiscard]] maui::controls::radio_button& option_b()
        {
            return option_b_;
        }

    private:
        // The shared Green / Bold / Arial / 12 property set the XAML's lower radios all carry
        // (TextTransform Uppercase deferred — see header). Content is the string repr of the deferred
        // embedded View ("It's a button inside a button." / "Properties already set.").
        static void configure_green(maui::controls::radio_button& radio)
        {
            radio.set_content("It's a button inside a button.");
            radio.set_group_name("test");
            radio.set_text_color(maui::graphics::colors::green);
            radio.set_font(
                maui::core::font::of_size("Arial", 12, maui::core::font_weight::bold, maui::core::font_slant::normal));
        }

        maui::controls::content_page page_;
        maui::controls::stack_layout stack_;

        maui::controls::label caption_intro_;
        maui::controls::label caption_a_;
        maui::controls::radio_button option_a_;
        maui::controls::label caption_b_;
        maui::controls::radio_button option_b_;
        maui::controls::label caption_button_;
        maui::controls::radio_button radio_button_content_;
        maui::controls::label caption_semantic1_;
        maui::controls::radio_button radio_semantic1_;
        maui::controls::label caption_semantic2_;
        maui::controls::radio_button radio_semantic2_;
        maui::controls::label caption_already_set_;
        maui::controls::radio_button radio_already_set_;
        maui::controls::radio_button radio_already_bound_;
    };
} // namespace maui::samples
