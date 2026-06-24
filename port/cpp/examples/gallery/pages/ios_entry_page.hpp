#pragma once
// maui::samples::ios_entry_page — ports iOSEntryPage.xaml (+ iOSEntryPage.xaml.cs).
//
// A self-contained, code-first demo of the iOSSpecific Entry knobs (W2-24). It mirrors the C# gallery
// page (Pages/PlatformSpecifics/iOS/iOSEntryPage): one Entry with a placeholder and FontSize=22, on
// which two iOS-specific attached knobs are applied at build time —
//   - Entry.AdjustsFontSizeToFitWidth = true  (ios:Entry.AdjustsFontSizeToFitWidth="true"),
//   - Entry.CursorColor = LimeGreen           (ios:Entry.CursorColor="LimeGreen"),
// plus a Button that toggles AdjustsFontSizeToFitWidth, reproducing the OnButtonClicked code-behind
// (entry.On<iOS>().SetAdjustsFontSizeToFitWidth(!entry.On<iOS>().AdjustsFontSizeToFitWidth())).
//
// The demonstrated point is the platform-configuration SURFACE: the knobs are exercised through the
// ported `element.on<ios>()` config accessor + the ios_specific::entry free-function knob set
// (configuration.hpp / ios_specific/entry.hpp). CursorColor drives the real UITextField.tintColor on
// the iOS backend (WIRED-REAL); on the headless/AppKit backends the knob is stored-inert (the values
// round-trip through the element's platform-spec store but no native cursor exists to tint). Either
// way the Entry control itself renders and the config calls compile + apply.
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same wiring directly.

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/font.hpp"
#include "maui/graphics/colors.hpp"

#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/ios_specific/entry.hpp"

namespace maui::samples
{
    class ios_entry_page
    {
    public:
        ios_entry_page()
        {
            namespace pc = maui::controls::platform_configuration;
            namespace ios_entry = pc::ios_specific::entry;

            page_.set_title("Entry FontSize and CursorColor");
            stack_.set_spacing(12);

            // Entry — placeholder + FontSize=22 (the cross-platform control surface).
            entry_.set_placeholder("Enter text here to see the font size change");
            entry_.set_font(maui::core::font::system_font_of_size(22));

            // iOSSpecific Entry knobs, applied at build time through the ported config accessor
            // (entry.on<ios>()) + the free-function knob set — exactly the C# XAML attached properties:
            //   ios:Entry.AdjustsFontSizeToFitWidth="true"  /  ios:Entry.CursorColor="LimeGreen".
            ios_entry::enable_adjusts_font_size_to_fit_width(entry_.on<pc::ios>());
            ios_entry::set_cursor_color(entry_.on<pc::ios>(), maui::graphics::colors::lime_green);

            // Button — toggles AdjustsFontSizeToFitWidth (ports OnButtonClicked).
            toggle_button_.set_text("Toggle AdjustsFontSizeToFitWidth");
            toggle_button_.clicked.connect([this] {
                namespace pcfg = maui::controls::platform_configuration;
                namespace ios_e = pcfg::ios_specific::entry;
                const bool current = ios_e::adjusts_font_size_to_fit_width(entry_.on<pcfg::ios>());
                ios_e::set_adjusts_font_size_to_fit_width(entry_.on<pcfg::ios>(), !current);
            });

            stack_.add(entry_);
            stack_.add(toggle_button_);
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
        [[nodiscard]] maui::controls::entry& text_entry()
        {
            return entry_;
        }
        [[nodiscard]] maui::controls::button& toggle_button()
        {
            return toggle_button_;
        }

    private:
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::entry entry_;
        maui::controls::button toggle_button_;
    };
} // namespace maui::samples
