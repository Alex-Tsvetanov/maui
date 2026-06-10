#pragma once
// maui::devices::sensors::detail::compass_base  <=  the cross-platform half of
// Microsoft.Maui.Devices.Sensors.CompassImplementation (Compass.shared.cs). Same lifecycle gating
// as basic_sensor, spelled separately because the compass start carries the applyLowPassFilter
// flag: Start(speed) => Start(speed, true), and the platform hook is
// platform_start(speed, apply_low_pass_filter). The filter flag is consumed only by the Android
// partial; Apple partials receive and ignore it (CLLocationManager's HeadingFilter handles
// smoothing there).

#include "maui/core/event.hpp"
#include "maui/essentials/compass.hpp"
#include "maui/essentials/feature_not_supported.hpp"

namespace maui::devices::sensors::detail
{
    class compass_base : public i_compass
    {
    public:
        maui::core::event<compass_data>& reading_changed() override
        {
            return reading_changed_;
        }

        [[nodiscard]] bool is_supported() const override
        {
            return platform_is_supported();
        }

        [[nodiscard]] bool is_monitoring() const override
        {
            return is_monitoring_;
        }

        void start(sensor_speed speed) override
        {
            start(speed, true);
        }

        void start(sensor_speed speed, bool apply_low_pass_filter) override
        {
            if (!platform_is_supported())
            {
                throw maui::application_model::feature_not_supported();
            }
            if (is_monitoring_)
            {
                throw std::logic_error("Compass has already been started.");
            }
            is_monitoring_ = true;
            speed_ = speed;
            try
            {
                platform_start(speed, apply_low_pass_filter);
            }
            catch (...)
            {
                is_monitoring_ = false;
                throw;
            }
        }

        void stop() override
        {
            if (!platform_is_supported())
            {
                throw maui::application_model::feature_not_supported();
            }
            if (!is_monitoring_)
            {
                return;
            }
            is_monitoring_ = false;
            try
            {
                platform_stop();
            }
            catch (...)
            {
                is_monitoring_ = true;
                throw;
            }
        }

    protected:
        compass_base() = default;

        [[nodiscard]] virtual bool platform_is_supported() const = 0;
        virtual void platform_start(sensor_speed speed, bool apply_low_pass_filter) = 0;
        virtual void platform_stop() = 0;

        void raise_reading_changed(const compass_data& data)
        {
            reading_changed_.raise(data);
        }

        [[nodiscard]] bool use_sync_context() const noexcept
        {
            return speed_ == sensor_speed::default_ || speed_ == sensor_speed::ui;
        }

    private:
        maui::core::event<compass_data> reading_changed_;
        bool is_monitoring_ = false;
        sensor_speed speed_ = sensor_speed::default_;
    };
} // namespace maui::devices::sensors::detail
