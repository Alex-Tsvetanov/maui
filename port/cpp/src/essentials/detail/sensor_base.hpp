#pragma once
// maui::devices::sensors::detail::basic_sensor  <=  the shared partial-class logic that
// Microsoft.Maui.Devices.Sensors duplicates across {Accelerometer, Gyroscope, Magnetometer,
// Barometer, OrientationSensor}Implementation (Start/Stop gating, IsMonitoring bookkeeping, the
// reading event raise). C# repeats the block in each sensor's *.shared.cs partial; the port
// expresses it once - behavior is byte-for-byte the same per sensor:
//   start(speed): !is_supported -> feature_not_supported; already monitoring ->
//                 std::logic_error("<Sensor> has already been started."); set monitoring BEFORE
//                 platform_start and roll it back when platform_start throws.
//   stop():       !is_supported -> feature_not_supported; not monitoring -> no-op; clear
//                 monitoring BEFORE platform_stop and roll it back when platform_stop throws.
// Internal-only (PROFILE.md §3): platform partials and the headless fakes derive from this; the
// public surface is the i_* interface in include/maui/essentials/.
//
// Threading note: C# marshals the raise through MainThread.BeginInvokeOnMainThread when the speed
// is Default/UI. The port's Apple partials deliver readings on the main queue for those speeds at
// the source (the CoreMotion callback queue), so raise_reading_changed always raises inline.

#include <string>
#include <utility>

#include "maui/core/event.hpp"
#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/sensor_types.hpp"

namespace maui::devices::sensors::detail
{
    // Interface = the i_* contract being implemented; Data = the reading payload type.
    template <class Interface, class Data> class basic_sensor : public Interface
    {
    public:
        maui::core::event<Data>& reading_changed() override
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
            if (!platform_is_supported())
            {
                throw maui::application_model::feature_not_supported();
            }
            if (is_monitoring_)
            {
                throw std::logic_error(sensor_name_ + " has already been started.");
            }
            is_monitoring_ = true;
            speed_ = speed;
            try
            {
                platform_start(speed);
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
        // sensor_name feeds the double-start message ("Accelerometer has already been started.",
        // "Orientation sensor has already been started.", ...) - each C# partial hard-codes its own.
        explicit basic_sensor(std::string sensor_name) : sensor_name_(std::move(sensor_name))
        {
        }

        // The per-backend partial surface (C# PlatformIsSupported / PlatformStart / PlatformStop).
        [[nodiscard]] virtual bool platform_is_supported() const = 0;
        virtual void platform_start(sensor_speed speed) = 0;
        virtual void platform_stop() = 0;

        // RaiseReadingChanged / OnChanged: deliver a new reading to subscribers.
        void raise_reading_changed(const Data& data)
        {
            reading_changed_.raise(data);
        }

        // UseSyncContext (C#): true for Default/UI speeds - the Apple partials use it to pick the
        // main queue as the sensor callback queue.
        [[nodiscard]] bool use_sync_context() const noexcept
        {
            return speed_ == sensor_speed::default_ || speed_ == sensor_speed::ui;
        }

    private:
        std::string sensor_name_;
        maui::core::event<Data> reading_changed_;
        bool is_monitoring_ = false;
        sensor_speed speed_ = sensor_speed::default_;
    };
} // namespace maui::devices::sensors::detail
