#pragma once
// maui::samples::ios_time_picker_page — ports iOSTimePickerPage.xaml
//
// The .NET MAUI PlatformSpecifics sample (Pages/PlatformSpecifics/iOS/iOSTimePickerPage.xaml +
// .xaml.cs): a TimePicker carrying the iOSSpecific TimePicker.UpdateMode knob (XAML seeds it to
// WhenFinished) over a button that toggles the knob between Immediately and WhenFinished. On UIKit the
// knob is the picker renderer's done-button commit policy — Immediately fires Time changes live as the
// wheel spins, WhenFinished defers the commit until the Done button is tapped. The knob is a STORED
// platform-spec on the headless backend (no UIKit picker to honor it), so this page renders the live
// time_picker AND exercises the config call: the readout echoes the current UpdateMode and the button
// flips it, mirroring OnButtonClicked's switch.
//
// Code-first, owns its whole element tree (the value_controls_page / pickers_page pattern). Public
// page() hands back the content_page; attach_handlers(maui_app) attaches every owned view bottom-up via
// gallery_attach_one then replays the host commands via gallery_rehost_* (gallery_attach.hpp).

#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/ios_specific/time_picker.hpp"
#include "maui/controls/platform_configuration/ios_specific/update_mode.hpp"
#include "maui/controls/time_picker.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/date_time.hpp" // time_span
#include "maui/core/thickness.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class ios_time_picker_page
    {
    public:
        ios_time_picker_page()
        {
            namespace ios_tp = maui::controls::platform_configuration::ios_specific::time_picker;

            page_.set_title("TimePicker UpdateMode");
            stack_.set_margin(maui::core::thickness(10)); // C# StackLayout Margin="10"
            stack_.set_spacing(12);

            // TimePicker Time="14:00:00".
            time_picker_.set_time(maui::core::time_span(14, 0, 0));

            // ios:TimePicker.UpdateMode="WhenFinished" — exercise the iOSSpecific knob (chaining overload
            // over the minted config<ios, time_picker>). STORED on the headless backend; the UIKit picker
            // renderer's done-button commit policy is the real consumer.
            ios_tp::set_update_mode(time_picker_.on<maui::controls::platform_configuration::ios>(),
                                    maui::controls::platform_configuration::ios_specific::update_mode::when_finished);

            // Button Text="Toggle TimePicker UpdateMode" / Clicked="OnButtonClicked" — the switch over the
            // current UpdateMode, flipping Immediately <-> WhenFinished.
            toggle_button_.set_text("Toggle TimePicker UpdateMode");
            toggle_button_.clicked.connect([this] { toggle_update_mode(); });

            update_readout();

            stack_.add(time_picker_);
            stack_.add(toggle_button_);
            stack_.add(readout_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view BOTTOM-UP (leaves first, the page last), then replay the
        // host commands built in the ctor (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, time_picker_, "time_picker_");
            gallery_attach_one(app, toggle_button_, "toggle_button_");
            gallery_attach_one(app, readout_, "readout_");
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
        [[nodiscard]] maui::controls::time_picker& picker()
        {
            return time_picker_;
        }
        [[nodiscard]] maui::controls::button& toggle_button()
        {
            return toggle_button_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }

    private:
        void toggle_update_mode()
        {
            namespace ios_tp = maui::controls::platform_configuration::ios_specific::time_picker;
            using update_mode = maui::controls::platform_configuration::ios_specific::update_mode;
            const auto cfg = time_picker_.on<maui::controls::platform_configuration::ios>();
            // Mirror OnButtonClicked's switch: Immediately -> WhenFinished, WhenFinished -> Immediately.
            const update_mode current = ios_tp::update_mode(cfg);
            ios_tp::set_update_mode(cfg, current == update_mode::immediately ? update_mode::when_finished
                                                                             : update_mode::immediately);
            update_readout();
        }

        void update_readout()
        {
            namespace ios_tp = maui::controls::platform_configuration::ios_specific::time_picker;
            using update_mode = maui::controls::platform_configuration::ios_specific::update_mode;
            const update_mode mode =
                ios_tp::update_mode(time_picker_.on<maui::controls::platform_configuration::ios>());
            readout_.set_text(std::string("UpdateMode: ") +
                              (mode == update_mode::when_finished ? "WhenFinished" : "Immediately"));
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::time_picker time_picker_;
        maui::controls::button toggle_button_;
        maui::controls::label readout_;
    };
} // namespace maui::samples
