#pragma once
// maui::devices::detail::battery_base  <=  the cross-platform half of
// Microsoft.Maui.Devices.BatteryImplementation (Battery.shared.cs): the listener-starting event
// accessors plus the change-dedupe cache. The first BatteryInfoChanged subscriber runs
// platform_start_battery_listeners() BEFORE the handler is stored (so a throwing start - the
// netstandard mirror - leaves no subscription behind); the last removal stops the listeners. Same
// for the EnergySaverStatusChanged pair. on_battery_info_changed() re-reads the three values and
// raises only when any differs from the cached copy (C#'s currentLevel/Source/State statics);
// energy-saver raises are not deduped, exactly like C#. The value getters (charge_level/state/
// power_source/energy_saver_status) stay pure virtual - they ARE the platform partial in C#.

#include "maui/core/event.hpp"
#include "maui/essentials/battery.hpp"

namespace maui::devices::detail
{
    class battery_base : public i_battery
    {
    public:
        maui::core::connection_token add_battery_info_changed(
            maui::core::move_only_function<void(const battery_info_changed_event_args&)> handler) override
        {
            if (battery_subscribers_ == 0)
            {
                platform_start_battery_listeners();
            }
            ++battery_subscribers_;
            return battery_info_changed_.connect(std::move(handler));
        }

        bool remove_battery_info_changed(maui::core::connection_token token) override
        {
            if (!battery_info_changed_.disconnect(token))
            {
                return false;
            }
            if (--battery_subscribers_ == 0)
            {
                platform_stop_battery_listeners();
            }
            return true;
        }

        maui::core::connection_token add_energy_saver_status_changed(
            maui::core::move_only_function<void(const enum energy_saver_status&)> handler) override
        {
            if (energy_saver_subscribers_ == 0)
            {
                platform_start_energy_saver_listeners();
            }
            ++energy_saver_subscribers_;
            return energy_saver_status_changed_.connect(std::move(handler));
        }

        bool remove_energy_saver_status_changed(maui::core::connection_token token) override
        {
            if (!energy_saver_status_changed_.disconnect(token))
            {
                return false;
            }
            if (--energy_saver_subscribers_ == 0)
            {
                platform_stop_energy_saver_listeners();
            }
            return true;
        }

    protected:
        battery_base() = default;

        // Start/StopBatteryListeners + Start/StopEnergySaverListeners (the platform partial).
        virtual void platform_start_battery_listeners() = 0;
        virtual void platform_stop_battery_listeners() = 0;
        virtual void platform_start_energy_saver_listeners() = 0;
        virtual void platform_stop_energy_saver_listeners() = 0;

        // OnBatteryInfoChanged(): re-read, dedupe against the cache, raise.
        void on_battery_info_changed()
        {
            const battery_info_changed_event_args args{charge_level(), state(), power_source()};
            if (args.charge_level != current_level_ || args.power_source != current_source_ ||
                args.state != current_state_)
            {
                current_level_ = args.charge_level;
                current_source_ = args.power_source;
                current_state_ = args.state;
                battery_info_changed_.raise(args);
            }
        }

        // OnEnergySaverChanged(): raise with the current status (never deduped).
        void on_energy_saver_changed()
        {
            energy_saver_status_changed_.raise(energy_saver_status());
        }

    private:
        maui::core::event<battery_info_changed_event_args> battery_info_changed_;
        maui::core::event<enum energy_saver_status> energy_saver_status_changed_;
        int battery_subscribers_ = 0;
        int energy_saver_subscribers_ = 0;
        double current_level_ = 0;
        battery_power_source current_source_ = battery_power_source::unknown;
        battery_state current_state_ = battery_state::unknown;
    };
} // namespace maui::devices::detail
