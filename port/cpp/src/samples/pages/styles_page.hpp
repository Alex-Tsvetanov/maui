#pragma once
// maui::samples::styles_page — ports StylesPage.xaml.
//
// The C# gallery page declares a page-level ResourceDictionary with:
//   - CustomStyle (TargetType=Label, BaseResourceKey=SubtitleStyle) adding TextColor=Pink,
//   - ButtonStyle (TargetType=Button) setting BackgroundColor/CornerRadius/HeightRequest,
//   - an implicit Style (TargetType=BoxView) setting Color=Aqua,
// then a stack of labels using the App-level Title/Subtitle/Body/Caption styles (DynamicResource), a
// button using ButtonStyle, a label using the custom (SubtitleStyle-derived) style, and a BoxView picking
// up the implicit style.
//
// This port reproduces the STYLE MACHINERY code-first on the headless-safe `maui::` surface: a
// maui::controls::style is a setter bundle applied at the style specificity via style::apply(target). The
// App-level resource styles (Title/Subtitle/Body/Caption) aren't in scope here (no App ResourceDictionary
// in the gallery), so this page defines the demonstrated styles itself:
//   - a base label "subtitle" style (a stand-in for the App SubtitleStyle: TextColor + character spacing),
//   - a derived "custom" label style with based_on -> the base, adding its own TextColor (Pink) — proving
//     BasedOn: the derived setter WINS over the base's at apply (Style.ApplyCore's lowered base
//     specificity), exactly like the XAML CustomStyle inheriting SubtitleStyle,
//   - a button style with several setters (text color + corner radius + stroke), the ButtonStyle analog.
// Each style is applied to its target in the ctor (the code-first equivalent of Style="{StaticResource
// ...}"); the resulting property values are then read back through the controls.
//
// note: C#'s ButtonStyle sets BackgroundColor / HeightRequest and the BoxView style sets Color — those
// bindable descriptors aren't exposed as bindable_property<color> on this port's view/box_view surface
// (BackgroundColor flows through set_background(brush), not a color descriptor), so the button style here
// uses the button's OWN exposed descriptors (TextColor / CornerRadius / StrokeColor / StrokeThickness) to
// demonstrate the same multi-setter Style. The mechanism is identical; only the chosen properties differ.
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window.

#include <cstdio>
#include <memory>
#include <string>

#include "maui/controls/brushes/solid_color_brush.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/setter.hpp"
#include "maui/controls/style.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class styles_page
    {
    public:
        styles_page()
        {
            page_.set_title("Styles");
            stack_.set_spacing(12);

            // --- the base label style (a stand-in for the App-level SubtitleStyle that CustomStyle derives
            // from): a muted text color + character spacing. ---
            base_label_style_ =
                std::make_shared<maui::controls::style>(maui::controls::style::of<maui::controls::label>());
            base_label_style_->add(maui::controls::setter::of(maui::controls::label::text_color_property(),
                                                              maui::graphics::color::from_rgb(0x55, 0x55, 0x55)));
            base_label_style_->add(
                maui::controls::setter::of(maui::controls::label::character_spacing_property(), 0.5));

            // --- the derived "custom" label style (CustomStyle: BaseResourceKey=SubtitleStyle + TextColor=
            // Pink). based_on applies the base FIRST at the lowered base-style specificity, so this style's
            // own TextColor (Pink) WINS — the BasedOn override the XAML demonstrates. ---
            custom_label_style_ =
                std::make_shared<maui::controls::style>(maui::controls::style::of<maui::controls::label>());
            custom_label_style_->set_based_on(base_label_style_);
            custom_label_style_->add(maui::controls::setter::of(
                maui::controls::label::text_color_property(), maui::graphics::color::from_rgb(255, 192, 203))); // Pink

            // --- the button style (ButtonStyle: several setters in one bundle). Uses the button's exposed
            // descriptors (see header note) to show a multi-setter Style. ---
            button_style_ =
                std::make_shared<maui::controls::style>(maui::controls::style::of<maui::controls::button>());
            button_style_->add(maui::controls::setter::of(maui::controls::button::text_color_property(),
                                                          maui::graphics::color::from_rgb(0x10, 0x10, 0x10)));
            button_style_->add(maui::controls::setter::of(maui::controls::button::corner_radius_property(), 0));
            button_style_->add(maui::controls::setter::of(maui::controls::button::stroke_color_property(),
                                                          maui::graphics::color::from_rgb(0xC0, 0xC0, 0x00)));
            button_style_->add(maui::controls::setter::of(maui::controls::button::stroke_thickness_property(), 2.0));

            // --- the controls + apply each style (code-first Style="{StaticResource ...}") ---
            base_styled_label_.set_text("This uses the (base) subtitle style");
            base_label_style_->apply(base_styled_label_);

            custom_styled_label_.set_text("This uses a custom style derived from the subtitle style (Pink wins)");
            custom_label_style_->apply(custom_styled_label_);

            unstyled_label_.set_text("This uses no explicit style (descriptor defaults)");

            styled_button_.set_text("Style Me");
            button_style_->apply(styled_button_);
            // C#'s ButtonStyle sets BackgroundColor (a light fill that keeps the dark TextColor readable in
            // BOTH themes). BackgroundColor is not a setter-targetable descriptor here (it flows through
            // set_background_brush, see the header note), so the demo applies the matching light fill
            // directly — the multi-setter Style above still drives TextColor / CornerRadius / Stroke.
            styled_button_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::light_gray));

            stack_.add(base_styled_label_);
            stack_.add(custom_styled_label_);
            stack_.add(unstyled_label_);
            stack_.add(styled_button_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (leaves first, the page last), then re-host the
        // tree built in the ctor (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            auto one = [&app](auto& v, const char* n) {
                try
                {
                    app.attach_handler(v);
                }
                catch (const std::exception& e)
                {
                    std::fprintf(stderr, "[gallery] skip %s: %s\n", n, e.what());
                }
            };
            one(base_styled_label_, "base_styled_label_");
            one(custom_styled_label_, "custom_styled_label_");
            one(unstyled_label_, "unstyled_label_");
            one(styled_button_, "styled_button_");
            one(stack_, "stack_");
            one(page_, "page_");

            gallery_rehost_layout(stack_);
            gallery_rehost_content(page_);
        }

        // Owned controls / styles, exposed for tests / the hosting main.
        [[nodiscard]] maui::controls::label& base_styled_label()
        {
            return base_styled_label_;
        }
        [[nodiscard]] maui::controls::label& custom_styled_label()
        {
            return custom_styled_label_;
        }
        [[nodiscard]] maui::controls::label& unstyled_label()
        {
            return unstyled_label_;
        }
        [[nodiscard]] maui::controls::button& styled_button()
        {
            return styled_button_;
        }

    private:
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label base_styled_label_;
        maui::controls::label custom_styled_label_;
        maui::controls::label unstyled_label_;
        maui::controls::button styled_button_;

        // The styles outlive the apply() calls (apply pushes the values into each control's property store;
        // keeping the styles owned lets a test re-apply / unapply them).
        std::shared_ptr<maui::controls::style> base_label_style_;
        std::shared_ptr<maui::controls::style> custom_label_style_;
        std::shared_ptr<maui::controls::style> button_style_;
    };
} // namespace maui::samples
