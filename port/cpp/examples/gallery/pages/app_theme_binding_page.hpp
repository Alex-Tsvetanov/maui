#pragma once
// maui::samples::app_theme_binding_page — ports AppThemeBindingPage.xaml.
//
// The MAUI page is a StackLayout of four labels — a bold headline, a label whose TextColor is
// {AppThemeBinding Light=Green, Dark=Red}, a second bold headline, and a label resolving a Light/Dark
// Color from the page ResourceDictionary (LightPrimaryColor=Orange / DarkPrimaryColor=Teal) via
// {AppThemeBinding Light={StaticResource LightPrimaryColor}, Dark={StaticResource DarkPrimaryColor}}.
// Nothing else: the original has NO interactive widgets (theme changes come from the OS / the app).
//
// Port mapping (headless-safe, code-first): {AppThemeBinding} + {StaticResource} are layer-6 XAML; this
// code-first port reproduces the SAME behavior directly against the HOSTING application (the page does
// NOT own a private application — the real theme source is the app the gallery mounts the page into):
//   - the ctor seeds the Light-branch colors (AppThemeBinding's Light-or-Unspecified rule), so the tree
//     is deterministic before any host exists;
//   - on_mounted (the gallery_host gallery_post_mount hook, same pattern as shape_app_theme_page)
//     re-applies from the app's CURRENT requested_theme() and subscribes to requested_theme_changed —
//     exactly the AppThemeBinding.Apply/ApplyCore re-application contract (Dark → Dark branch; Light
//     AND Unspecified → Light branch).
// The two resource colors are plain maui::graphics::colors constants here (the ResourceDictionary's
// StaticResource targets), so no XAML resource machinery is needed. The headlines carry the twin's
// inline Headline stand-in (FontSize 24 + Bold).
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same controls directly.

#include <memory>

#include "maui/controls/application.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/app_theme.hpp"
#include "maui/core/font.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/hosting/maui_app.hpp"

namespace maui::samples
{
    class app_theme_binding_page
    {
    public:
        app_theme_binding_page()
        {
            page_.set_title("AppThemeBinding");
            stack_.set_margin(maui::core::thickness(12)); // AppThemeBindingPage.xaml StackLayout Margin="12"

            // Headline + the inline {AppThemeBinding Light=Green, Dark=Red} label. The headlines carry
            // the twin's inline Headline style stand-in: FontSize 24, Bold.
            const maui::core::font headline_font =
                maui::core::font::system_font_of_size(24, maui::core::font_weight::bold);
            headline_a_.set_text("AppThemeBinding");
            headline_a_.set_font(headline_font);
            inline_label_.set_text("This text is green in light mode, and red in dark mode.");

            // Headline + the ResourceDictionary {StaticResource}-backed label
            // ({AppThemeBinding Light=LightPrimaryColor(Orange), Dark=DarkPrimaryColor(Teal)}).
            headline_b_.set_text("Using AppThemeBinding in a ResourceDictionary");
            headline_b_.set_font(headline_font);
            resource_label_.set_text("This text uses LightPrimaryColor (Orange) in light mode, and "
                                     "DarkPrimaryColor (Teal) in dark mode.");

            stack_.add(headline_a_);
            stack_.add(inline_label_);
            stack_.add(headline_b_);
            stack_.add(resource_label_);
            page_.set_content(stack_);

            // Deterministic pre-mount state: AppThemeBinding's Light-or-Unspecified rule resolves the
            // Light branch until a hosting application supplies the real theme (on_mounted below).
            apply_theme(maui::core::app_theme::unspecified);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // POST-MOUNT hook (gallery_host.hpp gallery_post_mount): bind the HOSTING app's live theme —
        // seed from the current requested_theme() and re-apply on every requested_theme_changed (the
        // AppThemeBinding.Apply/ApplyCore re-application; same pattern as shape_app_theme_page).
        void on_mounted(maui::hosting::maui_app& app)
        {
            if (const std::shared_ptr<maui::controls::application>& application = app.application())
            {
                apply_theme(application->requested_theme());
                application->requested_theme_changed.connect(
                    [this](maui::core::app_theme theme) { apply_theme(theme); });
            }
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

        // AppThemeBinding.GetValue: pick the Light or Dark branch by the effective theme (Dark → Dark;
        // Light AND Unspecified → Light) and push the chosen color into each bound label. Public so
        // tests can drive the theme swap directly (the mounted path goes through on_mounted).
        void apply_theme(maui::core::app_theme theme)
        {
            const bool dark = theme == maui::core::app_theme::dark;
            inline_label_.set_text_color(dark ? maui::graphics::colors::red : maui::graphics::colors::green);
            resource_label_.set_text_color(dark ? maui::graphics::colors::teal : maui::graphics::colors::orange);
        }

    private:
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label headline_a_;
        maui::controls::label inline_label_; // {AppThemeBinding Light=Green, Dark=Red}
        maui::controls::label headline_b_;
        maui::controls::label resource_label_; // {AppThemeBinding Light=Orange(res), Dark=Teal(res)}
    };
} // namespace maui::samples
