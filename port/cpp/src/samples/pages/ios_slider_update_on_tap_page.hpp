#pragma once
// maui::samples::ios_slider_update_on_tap_page — ports iOSSliderUpdateOnTapPage.xaml
//   (+ iOSSliderUpdateOnTapPage.xaml.cs).
//
// A self-contained, code-first demo of the iOSSpecific Slider.UpdateOnTap knob (W2-24). It mirrors the
// C# gallery page (Pages/PlatformSpecifics/iOS/iOSSliderUpdateOnTapPage): an instructional Label, one
// Slider on which the iOS-specific attached knob is applied at build time —
//   - Slider.UpdateOnTap = true  (ios:Slider.UpdateOnTap="true"),
// plus a Button that toggles UpdateOnTap, reproducing the OnButtonClicked code-behind
// (_slider.On<iOS>().SetUpdateOnTap(!_slider.On<iOS>().GetUpdateOnTap())).
//
// The demonstrated point is the platform-configuration SURFACE: the knob is exercised through the
// ported `element.on<ios>()` config accessor + the ios_specific::slider free-function knob set
// (configuration.hpp / ios_specific/slider.hpp). On iOS UpdateOnTap makes a tap on the slider bar move
// the thumb to that point; on the headless/AppKit backends the knob is stored-inert (the value
// round-trips through the element's platform-spec store but no native tap-to-update gesture exists).
// Either way the Slider control itself renders and the config calls compile + apply.
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same wiring directly.
//
// note: the C# StackLayout Margin="20" is ported via set_margin (the View.Margin seam).

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/slider.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/thickness.hpp"

#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/ios_specific/slider.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class ios_slider_update_on_tap_page
    {
    public:
        ios_slider_update_on_tap_page()
        {
            namespace pc = maui::controls::platform_configuration;
            namespace ios_slider = pc::ios_specific::slider;

            page_.set_title("Slider Update on Tap");
            stack_.set_spacing(12);
            stack_.set_margin(maui::core::thickness(20)); // C# StackLayout Margin="20"

            // The instructional label (the C# Label.Text).
            hint_.set_text("Tap on the Slider bar to move the thumb.");

            // iOSSpecific Slider knob, applied at build time through the ported config accessor
            // (slider.on<ios>()) + the free-function knob set — exactly the C# XAML attached property:
            //   ios:Slider.UpdateOnTap="true".
            ios_slider::set_update_on_tap(slider_.on<pc::ios>(), true);

            // Button — toggles UpdateOnTap (ports OnButtonClicked).
            toggle_button_.set_text("Toggle Update on Tap");
            toggle_button_.clicked.connect([this] {
                namespace pcfg = maui::controls::platform_configuration;
                namespace ios_s = pcfg::ios_specific::slider;
                const bool current = ios_s::get_update_on_tap(slider_.on<pcfg::ios>());
                ios_s::set_update_on_tap(slider_.on<pcfg::ios>(), !current);
            });

            stack_.add(hint_);
            stack_.add(slider_);
            stack_.add(toggle_button_);
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
            gallery_attach_one(app, hint_, "hint_");
            gallery_attach_one(app, slider_, "slider_");
            gallery_attach_one(app, toggle_button_, "toggle_button_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(stack_);
            gallery_rehost_content(page_);
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::label& hint()
        {
            return hint_;
        }
        [[nodiscard]] maui::controls::slider& value_slider()
        {
            return slider_;
        }
        [[nodiscard]] maui::controls::button& toggle_button()
        {
            return toggle_button_;
        }

    private:
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label hint_;
        maui::controls::slider slider_;
        maui::controls::button toggle_button_;
    };
} // namespace maui::samples
