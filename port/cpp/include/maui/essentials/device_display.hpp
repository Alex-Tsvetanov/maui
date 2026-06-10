#pragma once
// maui::devices::device_display       <=  Microsoft.Maui.Devices.DeviceDisplay (static facade)
// maui::devices::i_device_display     <=  Microsoft.Maui.Devices.IDeviceDisplay
// maui::devices::display_info         <=  Microsoft.Maui.Devices.DisplayInfo (Types/)
// maui::devices::display_orientation  <=  Microsoft.Maui.Devices.DisplayOrientation (Types/)
// maui::devices::display_rotation     <=  Microsoft.Maui.Devices.DisplayRotation (Types/)
//
// The display cluster shares this header (the enums + info struct exist only for this contract).
// C#'s MainDisplayInfoChanged event-with-add/remove-side-effects (the first subscriber starts the
// platform metrics listeners, the last unsubscribe stops them) maps to explicit
// add_main_display_info_changed / remove_main_display_info_changed methods - the port's event<>
// cannot observe subscription changes, and the add/remove pair IS the C# surface (event accessors).
// DisplayInfoChangedEventArgs collapses to its display_info payload.
//
// Backend reality (suffix oracle + unit decision): REAL on ios (DeviceDisplay.ios.cs - UIScreen +
// idleTimerDisabled + the status-bar-orientation listener). MAUI has NO DeviceDisplay.macos.cs (the
// netstandard partial covers it), but this unit was directed to ship a real AppKit implementation;
// it follows the retired Xamarin.Essentials macOS recipe (NSScreen metrics, IOPMAssertion
// keep-screen-on, NSApplicationDidChangeScreenParametersNotification) - a documented deviation.
// The headless default mirrors the netstandard partial (defaults, no throw); the headless fake
// adds setters.

#include <memory>

#include "maui/core/event.hpp"
#include "maui/core/move_only_function.hpp"

namespace maui::devices
{
    enum class display_orientation
    {
        unknown = 0,
        portrait = 1,
        landscape = 2,
    };

    enum class display_rotation
    {
        unknown = 0,
        rotation_0 = 1,
        rotation_90 = 2,
        rotation_180 = 3,
        rotation_270 = 4,
    };

    // The main screen's metrics. Equality mirrors C# DisplayInfo.Equals EXACTLY: width, height,
    // density, orientation and rotation - refresh_rate is deliberately excluded.
    struct display_info
    {
        double width = 0;  // pixels, for the current orientation
        double height = 0; // pixels, for the current orientation
        double density = 0;
        display_orientation orientation = display_orientation::unknown;
        display_rotation rotation = display_rotation::unknown;
        float refresh_rate = 0; // Hz; NOT part of equality (mirrors C#)

        friend bool operator==(const display_info& a, const display_info& b)
        {
            return a.width == b.width && a.height == b.height && a.density == b.density &&
                   a.orientation == b.orientation && a.rotation == b.rotation;
        }
    };

    class i_device_display
    {
    public:
        virtual ~i_device_display() = default;

        // IDeviceDisplay.KeepScreenOn get/set.
        [[nodiscard]] virtual bool keep_screen_on() const = 0;
        virtual void set_keep_screen_on(bool value) = 0;

        [[nodiscard]] virtual display_info main_display_info() const = 0;

        // The MainDisplayInfoChanged event accessors: the first add starts the platform metrics
        // listeners, the last remove stops them (DeviceDisplayImplementationBase semantics).
        virtual maui::core::connection_token add_main_display_info_changed(
            maui::core::move_only_function<void(const display_info&)> handler) = 0;
        virtual bool remove_main_display_info_changed(maui::core::connection_token token) = 0;

    protected:
        i_device_display() = default;
        i_device_display(const i_device_display&) = default;
        i_device_display(i_device_display&&) = default;
        i_device_display& operator=(const i_device_display&) = default;
        i_device_display& operator=(i_device_display&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (DeviceDisplayImplementation), one per backend under
        // src/platform/<backend>/essentials_device_display.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_device_display> make_device_display();
    } // namespace detail

    // The static facade over device_display::current().
    class device_display final
    {
    public:
        device_display() = delete;

        [[nodiscard]] static bool keep_screen_on()
        {
            return current().keep_screen_on();
        }
        static void set_keep_screen_on(bool value)
        {
            current().set_keep_screen_on(value);
        }
        [[nodiscard]] static display_info main_display_info()
        {
            return current().main_display_info();
        }
        static maui::core::connection_token add_main_display_info_changed(
            maui::core::move_only_function<void(const display_info&)> handler)
        {
            return current().add_main_display_info_changed(std::move(handler));
        }
        static bool remove_main_display_info_changed(maui::core::connection_token token)
        {
            return current().remove_main_display_info_changed(token);
        }

        // DeviceDisplay.Current (lazy platform default) + SetCurrent (the test seam; nullptr resets).
        [[nodiscard]] static i_device_display& current();
        static void set_current(std::shared_ptr<i_device_display> implementation);
    };
} // namespace maui::devices
