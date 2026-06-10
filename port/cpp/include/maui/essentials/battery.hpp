#pragma once
// maui::devices::battery               <=  Microsoft.Maui.Devices.Battery (static facade)
// maui::devices::i_battery             <=  Microsoft.Maui.Devices.IBattery
// maui::devices::battery_state         <=  Microsoft.Maui.Devices.BatteryState
// maui::devices::battery_power_source  <=  Microsoft.Maui.Devices.BatteryPowerSource
// maui::devices::energy_saver_status   <=  Microsoft.Maui.Devices.EnergySaverStatus
// maui::devices::battery_info_changed_event_args  <=  Microsoft.Maui.Devices.BatteryInfoChangedEventArgs
//
// One header for the battery cluster (Battery.shared.cs defines all of these together). Like
// device_display, C#'s listener-starting event accessors map to explicit add_/remove_ pairs: the
// first BatteryInfoChanged subscriber starts the platform battery listeners (the netstandard
// partial THROWS there - mirrored by the unconfigured headless fake), the last remove stops them;
// same for EnergySaverStatusChanged. EnergySaverStatusChangedEventArgs collapses to its
// energy_saver_status payload; BatteryInfoChangedEventArgs keeps its three fields.
//
// Backends (suffix oracle): apple/macOS REAL (Battery.macos.cs - IOKit IOPSCopyPowerSourcesInfo;
// energy saver is always Off with no listeners), ios REAL (Battery.ios.watchos.cs - UIDevice
// battery monitoring + NSProcessInfo low-power mode; the simulator reports level -1 / state
// unknown), headless = netstandard mirror until faked.

#include <memory>

#include "maui/core/event.hpp"
#include "maui/core/move_only_function.hpp"

namespace maui::devices
{
    enum class battery_state
    {
        unknown = 0,
        charging = 1,
        discharging = 2,
        full = 3,
        not_charging = 4,
        not_present = 5,
    };

    enum class battery_power_source
    {
        unknown = 0,
        battery = 1,
        ac = 2,
        usb = 3,
        wireless = 4,
    };

    enum class energy_saver_status
    {
        unknown = 0,
        on = 1,
        off = 2,
    };

    struct battery_info_changed_event_args
    {
        double charge_level = 0; // 0.0-1.0 (-1 if no battery)
        battery_state state = battery_state::unknown;
        battery_power_source power_source = battery_power_source::unknown;
    };

    class i_battery
    {
    public:
        virtual ~i_battery() = default;

        // The current charge from 0.0 to 1.0 (-1 when no battery exists).
        [[nodiscard]] virtual double charge_level() const = 0;
        [[nodiscard]] virtual battery_state state() const = 0;
        [[nodiscard]] virtual battery_power_source power_source() const = 0;
        [[nodiscard]] virtual enum energy_saver_status energy_saver_status() const = 0;

        // BatteryInfoChanged event accessors (first add starts the platform listeners, last remove
        // stops them - BatteryImplementation's shared partial semantics).
        virtual maui::core::connection_token add_battery_info_changed(
            maui::core::move_only_function<void(const battery_info_changed_event_args&)> handler) = 0;
        virtual bool remove_battery_info_changed(maui::core::connection_token token) = 0;

        // EnergySaverStatusChanged event accessors (same listener lifecycle).
        virtual maui::core::connection_token add_energy_saver_status_changed(
            maui::core::move_only_function<void(const enum energy_saver_status&)> handler) = 0;
        virtual bool remove_energy_saver_status_changed(maui::core::connection_token token) = 0;

    protected:
        i_battery() = default;
        i_battery(const i_battery&) = default;
        i_battery(i_battery&&) = default;
        i_battery& operator=(const i_battery&) = default;
        i_battery& operator=(i_battery&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (BatteryImplementation), one per backend under
        // src/platform/<backend>/essentials_battery.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_battery> make_battery();
    } // namespace detail

    // The static facade over battery::default_() (C# Battery.Default; "default" is a C++ keyword).
    class battery final
    {
    public:
        battery() = delete;

        [[nodiscard]] static double charge_level()
        {
            return default_().charge_level();
        }
        [[nodiscard]] static battery_state state()
        {
            return default_().state();
        }
        [[nodiscard]] static battery_power_source power_source()
        {
            return default_().power_source();
        }
        [[nodiscard]] static enum energy_saver_status energy_saver_status()
        {
            return default_().energy_saver_status();
        }
        static maui::core::connection_token add_battery_info_changed(
            maui::core::move_only_function<void(const battery_info_changed_event_args&)> handler)
        {
            return default_().add_battery_info_changed(std::move(handler));
        }
        static bool remove_battery_info_changed(maui::core::connection_token token)
        {
            return default_().remove_battery_info_changed(token);
        }
        static maui::core::connection_token add_energy_saver_status_changed(
            maui::core::move_only_function<void(const enum energy_saver_status&)> handler)
        {
            return default_().add_energy_saver_status_changed(std::move(handler));
        }
        static bool remove_energy_saver_status_changed(maui::core::connection_token token)
        {
            return default_().remove_energy_saver_status_changed(token);
        }

        // Battery.Default (lazy platform default) + SetDefault (the test seam; nullptr resets).
        [[nodiscard]] static i_battery& default_();
        static void set_default(std::shared_ptr<i_battery> implementation);
    };
} // namespace maui::devices
