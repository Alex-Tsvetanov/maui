#pragma once
// maui::samples::ios_date_picker_page — ports iOSDatePickerPage.xaml (+ iOSDatePickerPage.xaml.cs).
//
// A self-contained, code-first demo of the iOSSpecific DatePicker.UpdateMode knob (W2-24). It mirrors
// the C# gallery page (Pages/PlatformSpecifics/iOS/iOSDatePickerPage): one DatePicker clamped to the
// year 2020 (MinimumDate="01/01/2020" / MaximumDate="12/31/2020"), with the iOS-specific UpdateMode
// attached property set to WhenFinished at build time (ios:DatePicker.UpdateMode="WhenFinished"), plus
// a Button that toggles UpdateMode between Immediately and WhenFinished — reproducing the OnButtonClicked
// code-behind switch (datePicker.On<iOS>().UpdateMode() ↔ SetUpdateMode(...)).
//
// The demonstrated point is the platform-configuration SURFACE: the knob is exercised through the ported
// `element.on<ios>()` config accessor + the ios_specific::date_picker free-function knob set
// (configuration.hpp / ios_specific/date_picker.hpp). UpdateMode is a STORED knob — on the UIKit backend
// it is the picker renderer's done-button commit policy (commit the value immediately on each spin vs only
// when the picker is dismissed); the port stores it on the element's platform-spec store and round-trips
// it on every backend. Either way the DatePicker control itself renders and the config calls compile + apply.
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same wiring directly.

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/date_picker.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/date_time.hpp"

#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/ios_specific/date_picker.hpp"
#include "maui/controls/platform_configuration/ios_specific/update_mode.hpp"

namespace maui::samples
{
    class ios_date_picker_page
    {
    public:
        ios_date_picker_page()
        {
            namespace pc = maui::controls::platform_configuration;
            namespace ios_dp = pc::ios_specific::date_picker;
            using pc::ios_specific::update_mode;

            page_.set_title("DatePicker UpdateMode");
            stack_.set_spacing(12);

            // DatePicker — clamped to the year 2020 (the cross-platform control surface).
            date_picker_.set_minimum_date(maui::core::date_time(2020, 1, 1));
            date_picker_.set_maximum_date(maui::core::date_time(2020, 12, 31));

            // iOSSpecific DatePicker.UpdateMode knob, applied at build time through the ported config
            // accessor — exactly the C# XAML attached property ios:DatePicker.UpdateMode="WhenFinished".
            ios_dp::set_update_mode(date_picker_.on<pc::ios>(), update_mode::when_finished);

            // Button — toggles UpdateMode between Immediately and WhenFinished (ports OnButtonClicked).
            toggle_button_.set_text("Toggle DatePicker UpdateMode");
            toggle_button_.clicked.connect([this] {
                namespace pcfg = maui::controls::platform_configuration;
                namespace ios_d = pcfg::ios_specific::date_picker;
                using pcfg::ios_specific::update_mode;
                const auto next = ios_d::update_mode(date_picker_.on<pcfg::ios>()) == update_mode::immediately
                                      ? update_mode::when_finished
                                      : update_mode::immediately;
                ios_d::set_update_mode(date_picker_.on<pcfg::ios>(), next);
            });

            stack_.add(date_picker_);
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
        [[nodiscard]] maui::controls::date_picker& meeting_date()
        {
            return date_picker_;
        }
        [[nodiscard]] maui::controls::button& toggle_button()
        {
            return toggle_button_;
        }

    private:
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::date_picker date_picker_;
        maui::controls::button toggle_button_;
    };
} // namespace maui::samples
