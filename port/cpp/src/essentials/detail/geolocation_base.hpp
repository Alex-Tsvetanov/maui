#pragma once
// maui::devices::sensors::detail::geolocation_base  <=  the cross-platform half of
// Microsoft.Maui.Devices.Sensors.GeolocationImplementation (Geolocation.shared.cs): owns the
// LocationChanged / ListeningFailed events and the OnLocationChanged / OnLocationError raise
// helpers the platform partials call.

#include "maui/core/event.hpp"
#include "maui/essentials/geolocation.hpp"

namespace maui::devices::sensors::detail
{
    class geolocation_base : public i_geolocation
    {
    public:
        maui::core::event<location>& location_changed() override
        {
            return location_changed_;
        }

        maui::core::event<geolocation_error>& listening_failed() override
        {
            return listening_failed_;
        }

    protected:
        geolocation_base() = default;

        // OnLocationChanged(location).
        void on_location_changed(const location& new_location)
        {
            location_changed_.raise(new_location);
        }

        // OnLocationError(error) - callers stop listening BEFORE raising (the C# contract).
        void on_location_error(geolocation_error error)
        {
            listening_failed_.raise(error);
        }

    private:
        maui::core::event<location> location_changed_;
        maui::core::event<geolocation_error> listening_failed_;
    };
} // namespace maui::devices::sensors::detail
