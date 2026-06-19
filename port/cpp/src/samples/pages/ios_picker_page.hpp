#pragma once
// maui::samples::ios_picker_page — ports iOSPickerPage.xaml (+ iOSPickerPage.xaml.cs).
//
// A self-contained, code-first demo of the iOSSpecific Picker.UpdateMode knob (W2-24). It mirrors the
// C# gallery page (Pages/PlatformSpecifics/iOS/iOSPickerPage): one Picker titled "Select a monkey" with
// a string ItemsSource of seven monkeys, with the iOS-specific UpdateMode attached property set to
// WhenFinished at build time (ios:Picker.UpdateMode="WhenFinished"), plus a Button that toggles
// UpdateMode between Immediately and WhenFinished — reproducing the OnButtonClicked code-behind switch
// (picker.On<iOS>().UpdateMode() ↔ SetUpdateMode(...)).
//
// The demonstrated point is the platform-configuration SURFACE: the knob is exercised through the ported
// `element.on<ios>()` config accessor + the ios_specific::picker free-function knob set
// (configuration.hpp / ios_specific/picker.hpp). UpdateMode is a STORED knob — on the UIKit backend it is
// the picker renderer's done-button commit policy (commit the selection immediately on each spin vs only
// when the picker is dismissed); the port stores it on the element's platform-spec store and round-trips
// it on every backend. Either way the Picker control itself renders and the config calls compile + apply.
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same wiring directly.

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/picker.hpp"
#include "maui/controls/vertical_stack_layout.hpp"

#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/ios_specific/picker.hpp"
#include "maui/controls/platform_configuration/ios_specific/update_mode.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class ios_picker_page
    {
    public:
        ios_picker_page()
        {
            namespace pc = maui::controls::platform_configuration;
            namespace ios_pk = pc::ios_specific::picker;
            using pc::ios_specific::update_mode;

            page_.set_title("Picker UpdateMode");
            stack_.set_spacing(12);

            // Picker — placeholder Title + the seven-monkey string ItemsSource (cross-platform surface).
            picker_.set_title("Select a monkey");
            picker_.items().add("Baboon");
            picker_.items().add("Capuchin Monkey");
            picker_.items().add("Blue Monkey");
            picker_.items().add("Squirrel Monkey");
            picker_.items().add("Golden Lion Tamarin");
            picker_.items().add("Howler Monkey");
            picker_.items().add("Japanese Macaque");

            // iOSSpecific Picker.UpdateMode knob, applied at build time through the ported config accessor
            // — exactly the C# XAML attached property ios:Picker.UpdateMode="WhenFinished".
            ios_pk::set_update_mode(picker_.on<pc::ios>(), update_mode::when_finished);

            // Button — toggles UpdateMode between Immediately and WhenFinished (ports OnButtonClicked).
            toggle_button_.set_text("Toggle Picker UpdateMode");
            toggle_button_.clicked.connect([this] {
                namespace pcfg = maui::controls::platform_configuration;
                namespace ios_p = pcfg::ios_specific::picker;
                using pcfg::ios_specific::update_mode;
                const auto next = ios_p::update_mode(picker_.on<pcfg::ios>()) == update_mode::immediately
                                      ? update_mode::when_finished
                                      : update_mode::immediately;
                ios_p::set_update_mode(picker_.on<pcfg::ios>(), next);
            });

            stack_.add(picker_);
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
            gallery_attach_one(app, picker_, "picker_");
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
        [[nodiscard]] maui::controls::picker& monkey_picker()
        {
            return picker_;
        }
        [[nodiscard]] maui::controls::button& toggle_button()
        {
            return toggle_button_;
        }

    private:
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::picker picker_;
        maui::controls::button toggle_button_;
    };
} // namespace maui::samples
