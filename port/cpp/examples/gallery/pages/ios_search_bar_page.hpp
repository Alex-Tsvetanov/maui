#pragma once
// maui::samples::ios_search_bar_page — ports iOSSearchBarPage.xaml (+ iOSSearchBarPage.xaml.cs).
//
// A self-contained, code-first demo of the iOSSpecific SearchBar.SearchBarStyle knob (W2-24). It mirrors
// the C# gallery page (Pages/PlatformSpecifics/iOS/iOSSearchBarPage): one SearchBar with a placeholder on
// which the iOS-specific attached knob is applied at build time —
//   - SearchBar.SearchBarStyle = Minimal  (ios:SearchBar.SearchBarStyle="Minimal"),
// plus two Buttons:
//   - "Toggle SearchBar Style" → OnSearchBarStyleButtonClicked cycles Default → Minimal → Prominent →
//     Default through the config accessor (ported verbatim);
//   - "Toggle Background" → OnToggleBackgroundButtonClicked flips the bar background between Teal and
//     Black (the C# `searchBar.BackgroundColor == Colors.Teal ? Black : Teal`).
//
// The demonstrated point is the platform-configuration SURFACE: the knob is exercised through the ported
// `element.on<ios>()` config accessor + the ios_specific::search_bar free-function knob set
// (configuration.hpp / ios_specific/search_bar.hpp) plus the UISearchBarStyle enum (ui_search_bar_style).
// On iOS SearchBarStyle drives the native UISearchBar.searchBarStyle (Default/Prominent/Minimal chrome);
// on the headless/AppKit backends the knob is stored-inert (the value round-trips through the element's
// platform-spec store but no native UISearchBar chrome exists). Either way the SearchBar renders, the
// config calls compile + apply, and the background toggle drives the real view background paint.
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same wiring directly.
//
// note: the C# OnToggleBackgroundButtonClicked reads `searchBar.BackgroundColor == Colors.Teal`. The
//       port models a view background as an owned solid_paint with no `color`-returning BackgroundColor
//       getter, so the toggle's current state is tracked in a small bool member (background_is_teal_) and
//       the paint applied through view::set_background — observably the same Teal⇄Black flip.

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/search_bar.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"

#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/ios_specific/search_bar.hpp"
#include "maui/controls/platform_configuration/ios_specific/ui_search_bar_style.hpp"

#include <memory>

namespace maui::samples
{
    class ios_search_bar_page
    {
    public:
        ios_search_bar_page()
        {
            namespace pc = maui::controls::platform_configuration;
            namespace ios_search = pc::ios_specific::search_bar;

            page_.set_title("SearchBar style");
            stack_.set_spacing(12);

            // SearchBar — placeholder (the cross-platform control surface).
            search_bar_.set_placeholder("Enter search term");

            // iOSSpecific SearchBar knob, applied at build time through the ported config accessor
            // (search_bar.on<ios>()) + the free-function knob set — exactly the C# XAML attached property:
            //   ios:SearchBar.SearchBarStyle="Minimal".
            ios_search::set_search_bar_style(search_bar_.on<pc::ios>(), pc::ios_specific::ui_search_bar_style::minimal);

            // "Toggle SearchBar Style" — cycles Default → Minimal → Prominent → Default (ports
            // OnSearchBarStyleButtonClicked).
            style_button_.set_text("Toggle SearchBar Style");
            style_button_.clicked.connect([this] {
                namespace pcfg = maui::controls::platform_configuration;
                namespace ios_sb = pcfg::ios_specific::search_bar;
                using style = pcfg::ios_specific::ui_search_bar_style;
                switch (ios_sb::get_search_bar_style(search_bar_.on<pcfg::ios>()))
                {
                    case style::default_style:
                        ios_sb::set_search_bar_style(search_bar_.on<pcfg::ios>(), style::minimal);
                        break;
                    case style::minimal:
                        ios_sb::set_search_bar_style(search_bar_.on<pcfg::ios>(), style::prominent);
                        break;
                    case style::prominent:
                        ios_sb::set_search_bar_style(search_bar_.on<pcfg::ios>(), style::default_style);
                        break;
                }
            });

            // "Toggle Background" — flips the bar background Teal ⇄ Black (ports
            // OnToggleBackgroundButtonClicked). Start Teal so the first tap goes to Black, matching the C#
            // `== Colors.Teal ? Black : Teal` once a Teal background has been set.
            background_button_.set_text("Toggle Background");
            background_button_.clicked.connect([this] { toggle_background(); });

            stack_.add(search_bar_);
            stack_.add(style_button_);
            stack_.add(background_button_);
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
        [[nodiscard]] maui::controls::search_bar& search()
        {
            return search_bar_;
        }
        [[nodiscard]] maui::controls::button& style_button()
        {
            return style_button_;
        }
        [[nodiscard]] maui::controls::button& background_button()
        {
            return background_button_;
        }

    private:
        // The Teal ⇄ Black background flip. background_is_teal_ tracks the C# `== Colors.Teal` test the
        // port cannot read back from a color-typed BackgroundColor getter (see header note).
        void toggle_background()
        {
            background_is_teal_ = !background_is_teal_;
            const auto next = background_is_teal_ ? maui::graphics::colors::teal : maui::graphics::colors::black;
            search_bar_.set_background(std::make_shared<maui::graphics::solid_paint>(next));
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::search_bar search_bar_;
        maui::controls::button style_button_;
        maui::controls::button background_button_;
        bool background_is_teal_ = false; // first toggle → Teal (then Teal ⇄ Black thereafter)
    };
} // namespace maui::samples
