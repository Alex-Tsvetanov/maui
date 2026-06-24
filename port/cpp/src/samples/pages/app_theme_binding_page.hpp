#pragma once
// maui::samples::app_theme_binding_page — ports AppThemeBindingPage.xaml.
//
// The MAUI page is a StackLayout of labels whose TextColor is set via the {AppThemeBinding …} markup
// extension, which picks a value by the app's RequestedTheme and RE-APPLIES on every theme change:
//   - a headline, then a label "green in light mode, red in dark mode"
//       TextColor="{AppThemeBinding Light=Green, Dark=Red}"
//   - a second headline, then a label resolving a Light/Dark Color from the ResourceDictionary
//       (LightPrimaryColor=Orange / DarkPrimaryColor=Teal) via
//       TextColor="{AppThemeBinding Light={StaticResource LightPrimaryColor}, Dark={StaticResource DarkPrimaryColor}}"
//
// Port mapping (headless-safe, code-first): {AppThemeBinding} + {StaticResource} are layer-6 XAML; this
// code-first port reproduces the SAME behavior directly. The page owns a maui::controls::application
// (the theme source) and applies the light/dark choice itself off application::requested_theme()
// (app_theme.hpp / application.hpp), re-applying on application::requested_theme_changed — exactly the
// AppThemeBinding.Apply/ApplyCore contract (Dark → Dark branch; Light AND Unspecified → Light branch).
// The two resource colors are plain maui::graphics::colors constants here (the ResourceDictionary's
// StaticResource targets), so no XAML resource machinery is needed. A toggle flips UserAppTheme between
// Light and Dark and the bound labels recolor live; a readout echoes the active theme + resolved names.
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same controls directly.

#include <string>

#include "maui/controls/application.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/app_theme.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"

namespace maui::samples
{
    class app_theme_binding_page
    {
    public:
        app_theme_binding_page()
        {
            page_.set_title("AppThemeBinding");
            stack_.set_spacing(8);
            stack_.set_margin(maui::core::thickness(12)); // AppThemeBindingPage.xaml StackLayout Margin="12"

            // Headline + the inline {AppThemeBinding Light=Green, Dark=Red} label.
            headline_a_.set_text("AppThemeBinding");
            inline_label_.set_text("This text is green in light mode, and red in dark mode.");

            // Headline + the ResourceDictionary {StaticResource}-backed label
            // ({AppThemeBinding Light=LightPrimaryColor(Orange), Dark=DarkPrimaryColor(Teal)}).
            headline_b_.set_text("Using AppThemeBinding in a ResourceDictionary");
            resource_label_.set_text("This text uses LightPrimaryColor (Orange) in light mode, and "
                                     "DarkPrimaryColor (Teal) in dark mode.");

            // The interactive toggle (not in the XAML, but the demonstrable bit headless): flip the app's
            // UserAppTheme between Light and Dark; the bound labels recolor + the readout updates.
            toggle_button_.set_text("Toggle theme (Light/Dark)");
            toggle_button_.clicked.connect([this] { toggle_theme(); });
            readout_.set_text("...");

            // AppThemeBinding re-applies on RequestedThemeChanged — mirror that with a live subscription so
            // any theme change (from the toggle or a platform push) recolors the bound labels.
            app_.requested_theme_changed.connect([this](maui::core::app_theme /*theme*/) { apply_theme(); });

            stack_.add(headline_a_);
            stack_.add(inline_label_);
            stack_.add(headline_b_);
            stack_.add(resource_label_);
            stack_.add(toggle_button_);
            stack_.add(readout_);
            page_.set_content(stack_);

            // Seed the start theme (Light) so the static capture shows resolved light-mode colors.
            app_.set_user_app_theme(maui::core::app_theme::light);
            apply_theme();
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment / inspection.
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::label& inline_label()
        {
            return inline_label_;
        }
        [[nodiscard]] maui::controls::label& resource_label()
        {
            return resource_label_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }
        [[nodiscard]] maui::controls::button& toggle_button()
        {
            return toggle_button_;
        }
        [[nodiscard]] maui::controls::application& app()
        {
            return app_;
        }

    private:
        // Flip UserAppTheme Light<->Dark — RequestedThemeChanged fires, re-running apply_theme() (the
        // AppThemeBinding re-apply path). Falls back to Light from Unspecified on the first flip.
        void toggle_theme()
        {
            app_.set_user_app_theme(app_.requested_theme() == maui::core::app_theme::dark
                                        ? maui::core::app_theme::light
                                        : maui::core::app_theme::dark);
        }

        // AppThemeBinding.GetValue: pick the Light or Dark branch by the effective theme (Dark → Dark;
        // Light AND Unspecified → Light), push the chosen color into each bound label, and echo the result.
        void apply_theme()
        {
            const bool dark = app_.requested_theme() == maui::core::app_theme::dark;

            const maui::graphics::color inline_color =
                dark ? maui::graphics::colors::red : maui::graphics::colors::green;
            const maui::graphics::color resource_color =
                dark ? maui::graphics::colors::teal : maui::graphics::colors::orange;

            inline_label_.set_text_color(inline_color);
            resource_label_.set_text_color(resource_color);

            readout_.set_text(std::string("Theme: ") + (dark ? "Dark" : "Light") +
                              " — inline=" + (dark ? "Red" : "Green") + ", resource=" + (dark ? "Teal" : "Orange"));
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label headline_a_;
        maui::controls::label inline_label_; // {AppThemeBinding Light=Green, Dark=Red}
        maui::controls::label headline_b_;
        maui::controls::label resource_label_; // {AppThemeBinding Light=Orange(res), Dark=Teal(res)}
        maui::controls::button toggle_button_;
        maui::controls::label readout_;

        // The theme source (a C# Application's UserAppTheme/RequestedTheme/RequestedThemeChanged).
        maui::controls::application app_;
    };
} // namespace maui::samples
