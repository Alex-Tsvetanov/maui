#pragma once
// maui::samples::device_page — ports DevicePage.xaml.
//
// The MAUI page is a centered StackLayout of two labels driven by XAML's {OnPlatform} and {OnIdiom}
// markup extensions: the first shows the running PLATFORM (Default/Android/iOS/MacCatalyst/Tizen/WinUI),
// the second the running IDIOM (Default/Phone/Tablet/Desktop/TV/Watch). Those XAML extensions resolve at
// load time off DeviceInfo.Platform / DeviceInfo.Idiom.
//
// Port mapping (headless-safe, code-first): {OnPlatform}/{OnIdiom} are layer-6 XAML markup extensions, so
// this code-first port resolves the same source values DIRECTLY through the essentials facade —
// maui::devices::device_info::platform() / ::idiom() (device_info.hpp) — and writes them into the labels,
// plus a readout that also surfaces device_info::version() (the IDeviceInfo.Version slice) so the static
// capture shows concrete device fields. device_platform/device_idiom keep C#'s string-backed names, so
// to_string() round-trips the original identifiers ("iOS", "Desktop", …) the XAML cases keyed on.
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same controls directly. Under the headless backend device_info::current()
// resolves to the headless default implementation, so this renders deterministically with no device.

#include <string>

#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/stack_layout.hpp"
#include "maui/essentials/device_info.hpp"

namespace maui::samples
{
    class device_page
    {
    public:
        device_page()
        {
            page_.set_title("Device");
            stack_.set_spacing(8);
            // DevicePage.xaml sets StackLayout HorizontalOptions="Center" — center the stack horizontally
            // to match the maui-compare ref. (The vertical offset vs the ref is the harness inset, exempt
            // per parity ruling #2.)
            stack_.set_horizontal_layout_alignment(maui::core::layout_alignment::center);

            // {OnPlatform …}: resolve the running platform off DeviceInfo.Platform (the source the XAML
            // case keyed on); to_string() yields the C# identifier ("iOS", "MacCatalyst", …).
            platform_label_.set_text(std::string("Platform: ") +
                                     std::string(maui::devices::device_info::platform().to_string()));

            // {OnIdiom …}: resolve the running idiom off DeviceInfo.Idiom ("Phone", "Desktop", …).
            idiom_label_.set_text(std::string("Idiom: ") +
                                  std::string(maui::devices::device_info::idiom().to_string()));

            // Bonus readout: the IDeviceInfo.Version slice (not in the XAML, but a concrete device field
            // that makes the headless capture informative). version().to_string() is "major.minor[.…]".
            version_label_.set_text(std::string("Version: ") + maui::devices::device_info::version().to_string());

            stack_.add(platform_label_);
            stack_.add(idiom_label_);
            stack_.add(version_label_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment / inspection.
        [[nodiscard]] maui::controls::stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::label& platform_label()
        {
            return platform_label_;
        }
        [[nodiscard]] maui::controls::label& idiom_label()
        {
            return idiom_label_;
        }
        [[nodiscard]] maui::controls::label& version_label()
        {
            return version_label_;
        }

    private:
        maui::controls::content_page page_;
        maui::controls::stack_layout stack_;
        maui::controls::label platform_label_;
        maui::controls::label idiom_label_;
        maui::controls::label version_label_;
    };
} // namespace maui::samples
