#pragma once
// maui::samples::ios_blur_effect_page — ports iOSBlurEffectPage.xaml
//
// The .NET MAUI PlatformSpecifics sample (Pages/PlatformSpecifics/iOS/iOSBlurEffectPage.xaml +
// .xaml.cs): an Image (Source="oasis.jpg") carrying the iOSSpecific VisualElement.BlurEffect knob (XAML
// seeds it to ExtraLight) over four buttons — No / Extra Light / Light / Dark — each calling
// image.On<iOS>().UseBlurEffect(style). On UIKit the knob overlays a UIVisualEffectView blur of the
// chosen style on the element. BlurEffect is an iOS-native effect with no headless analog (the knob is a
// STORED platform-spec, inert on the headless backend), so this page renders the live image + buttons
// AND exercises the config call: each button sets the BlurEffect style on the image, mirroring the four
// click handlers. A readout echoes the currently selected style so the inert knob's effect is visible.
//
// Code-first, owns its whole element tree (the value_controls_page / pickers_page pattern). Public
// page() hands back the content_page; attach_handlers(maui_app) attaches every owned view bottom-up via
// gallery_attach_one then replays the host commands via gallery_rehost_* (gallery_attach.hpp).

#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/file_image_source.hpp" // image_source::from_file
#include "maui/controls/image.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/ios_specific/blur_effect_style.hpp"
#include "maui/controls/platform_configuration/ios_specific/visual_element.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/thickness.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class ios_blur_effect_page
    {
    public:
        ios_blur_effect_page()
        {
            using blur_effect_style = maui::controls::platform_configuration::ios_specific::blur_effect_style;

            page_.set_title("Blur Effect");
            stack_.set_margin(maui::core::thickness(20)); // C# StackLayout Margin="20"
            stack_.set_spacing(12);

            // Image Source="oasis.jpg" — mint a file image source (the gallery resource the C# sample
            // bundles). The image control owns the returned shared_ptr.
            image_.set_source(maui::controls::image_source::from_file("oasis.jpg"));

            // ios:VisualElement.BlurEffect="ExtraLight" — seed the iOSSpecific knob (chaining overload over
            // the minted config<ios, image>). STORED on the headless backend; a UIVisualEffectView is the
            // real UIKit consumer.
            apply_blur(blur_effect_style::extra_light);

            // The four buttons, each calling image.On<iOS>().UseBlurEffect(style).
            no_blur_button_.set_text("No Blur");
            no_blur_button_.clicked.connect([this] { apply_blur_and_readout(no_blur_style()); });
            extra_light_button_.set_text("Extra Light Blur");
            extra_light_button_.clicked.connect([this] { apply_blur_and_readout(extra_light_style()); });
            light_button_.set_text("Light Blur");
            light_button_.clicked.connect([this] { apply_blur_and_readout(light_style()); });
            dark_button_.set_text("Dark Blur");
            dark_button_.clicked.connect([this] { apply_blur_and_readout(dark_style()); });

            update_readout();

            stack_.add(image_);
            stack_.add(no_blur_button_);
            stack_.add(extra_light_button_);
            stack_.add(light_button_);
            stack_.add(dark_button_);
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
            gallery_attach_one(app, image_, "image_");
            gallery_attach_one(app, no_blur_button_, "no_blur_button_");
            gallery_attach_one(app, extra_light_button_, "extra_light_button_");
            gallery_attach_one(app, light_button_, "light_button_");
            gallery_attach_one(app, dark_button_, "dark_button_");
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
        [[nodiscard]] maui::controls::image& blur_image()
        {
            return image_;
        }
        [[nodiscard]] maui::controls::button& no_blur_button()
        {
            return no_blur_button_;
        }
        [[nodiscard]] maui::controls::button& extra_light_button()
        {
            return extra_light_button_;
        }
        [[nodiscard]] maui::controls::button& light_button()
        {
            return light_button_;
        }
        [[nodiscard]] maui::controls::button& dark_button()
        {
            return dark_button_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }

    private:
        using blur_effect_style = maui::controls::platform_configuration::ios_specific::blur_effect_style;

        static constexpr blur_effect_style no_blur_style()
        {
            return blur_effect_style::none;
        }
        static constexpr blur_effect_style extra_light_style()
        {
            return blur_effect_style::extra_light;
        }
        static constexpr blur_effect_style light_style()
        {
            return blur_effect_style::light;
        }
        static constexpr blur_effect_style dark_style()
        {
            return blur_effect_style::dark;
        }

        void apply_blur(blur_effect_style style)
        {
            namespace ios_ve = maui::controls::platform_configuration::ios_specific::visual_element;
            // image.On<iOS>().UseBlurEffect(style) — the chaining setter over the minted config<ios, image>.
            ios_ve::use_blur_effect(image_.on<maui::controls::platform_configuration::ios>(), style);
        }

        void apply_blur_and_readout(blur_effect_style style)
        {
            apply_blur(style);
            update_readout();
        }

        void update_readout()
        {
            namespace ios_ve = maui::controls::platform_configuration::ios_specific::visual_element;
            const blur_effect_style style =
                ios_ve::get_blur_effect(image_.on<maui::controls::platform_configuration::ios>());
            readout_.set_text(std::string("BlurEffect: ") + name_of(style));
        }

        static const char* name_of(blur_effect_style style)
        {
            switch (style)
            {
                case blur_effect_style::extra_light:
                    return "ExtraLight";
                case blur_effect_style::light:
                    return "Light";
                case blur_effect_style::dark:
                    return "Dark";
                case blur_effect_style::none:
                default:
                    return "None";
            }
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::image image_;
        maui::controls::button no_blur_button_;
        maui::controls::button extra_light_button_;
        maui::controls::button light_button_;
        maui::controls::button dark_button_;
        maui::controls::label readout_;
    };
} // namespace maui::samples
