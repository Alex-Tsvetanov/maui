#pragma once
// The headless backend's essentials implementations - one controllable FAKE per feature, the
// in-memory twins of the C# *.netstandard.*.cs partials (the *.Standard.cs role: deterministic,
// device-free, fully unit-testable). One header for the family (the headless test seam; internal
// to src/, mirroring how the apple op headers live beside their backend sources).
//
// Contract every fake follows:
//   * UNCONFIGURED, it mirrors the netstandard partial byte-for-byte: members that throw
//     NotImplementedInReferenceAssemblyException there throw feature_not_supported here; members
//     that return defaults there (device_display metrics, flashlight is-supported, geolocation
//     is_enabled) return the same defaults.
//   * The set_* test setters configure it into a working in-memory device: values become readable,
//     listeners/lifecycles record themselves, and simulate_* methods drive the shared raise paths
//     (the dedupe/lifecycle logic in src/essentials/detail/*_base.hpp runs for real).
//   * Everything is inline and synchronous - async callbacks complete inline on the caller's
//     thread (the headless analog of "delivered on the main queue").
//
// The per-feature factories (detail::make_*) in src/platform/headless/essentials_<feature>.cpp
// return these fakes as the backend's lazy defaults; tests can also instantiate them directly and
// install them through the facades' set_current/set_default seams.

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/essentials/accelerometer.hpp"
#include "maui/essentials/barometer.hpp"
#include "maui/essentials/battery.hpp"
#include "maui/essentials/compass.hpp"
#include "maui/essentials/connectivity.hpp"
#include "maui/essentials/device_display.hpp"
#include "maui/essentials/device_info.hpp"
#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/flashlight.hpp"
#include "maui/essentials/geocoding.hpp"
#include "maui/essentials/geolocation.hpp"
#include "maui/essentials/gyroscope.hpp"
#include "maui/essentials/haptic_feedback.hpp"
#include "maui/essentials/location.hpp"
#include "maui/essentials/magnetometer.hpp"
#include "maui/essentials/orientation_sensor.hpp"
#include "maui/essentials/placemark.hpp"
#include "maui/essentials/screenshot.hpp"
#include "maui/essentials/sensor_types.hpp"
#include "maui/essentials/text_to_speech.hpp"
#include "maui/essentials/vibration.hpp"

#include "src/essentials/detail/accelerometer_base.hpp"
#include "src/essentials/detail/battery_base.hpp"
#include "src/essentials/detail/compass_base.hpp"
#include "src/essentials/detail/connectivity_base.hpp"
#include "src/essentials/detail/device_display_base.hpp"
#include "src/essentials/detail/geolocation_base.hpp"
#include "src/essentials/detail/sensor_base.hpp"
#include "src/essentials/detail/text_to_speech_base.hpp"
#include "src/essentials/detail/vibration_base.hpp"

namespace maui::devices
{
    namespace headless_detail
    {
        // The netstandard partials' throw, shared by every unconfigured fake member.
        [[noreturn]] inline void throw_not_implemented()
        {
            throw maui::application_model::feature_not_supported(
                "This feature is not implemented on the headless backend until the fake is configured "
                "(the netstandard-partial mirror).");
        }

        template <class T> const T& require(const std::optional<T>& value)
        {
            if (!value.has_value())
            {
                throw_not_implemented();
            }
            return *value;
        }
    } // namespace headless_detail

    // DeviceInfoImplementation (netstandard): model/manufacturer/name/version throw; platform and
    // idiom are Unknown, device type unknown - until configured.
    class headless_device_info final : public i_device_info
    {
    public:
        [[nodiscard]] std::string model() const override
        {
            return headless_detail::require(model_);
        }
        [[nodiscard]] std::string manufacturer() const override
        {
            return headless_detail::require(manufacturer_);
        }
        [[nodiscard]] std::string name() const override
        {
            return headless_detail::require(name_);
        }
        [[nodiscard]] std::string version_string() const override
        {
            return headless_detail::require(version_string_);
        }
        [[nodiscard]] device_platform platform() const override
        {
            return platform_;
        }
        [[nodiscard]] device_idiom idiom() const override
        {
            return idiom_;
        }
        [[nodiscard]] enum device_type device_type() const override
        {
            return device_type_;
        }

        void set_model(std::string value)
        {
            model_ = std::move(value);
        }
        void set_manufacturer(std::string value)
        {
            manufacturer_ = std::move(value);
        }
        void set_name(std::string value)
        {
            name_ = std::move(value);
        }
        void set_version_string(std::string value)
        {
            version_string_ = std::move(value);
        }
        void set_platform(device_platform value)
        {
            platform_ = std::move(value);
        }
        void set_idiom(device_idiom value)
        {
            idiom_ = std::move(value);
        }
        void set_device_type(enum device_type value)
        {
            device_type_ = value;
        }

    private:
        std::optional<std::string> model_;
        std::optional<std::string> manufacturer_;
        std::optional<std::string> name_;
        std::optional<std::string> version_string_;
        device_platform platform_ = device_platform::unknown();
        device_idiom idiom_ = device_idiom::unknown();
        enum device_type device_type_ = device_type::unknown;
    };

    // DeviceDisplayImplementation (netstandard): default metrics, no-op listeners. The fake makes
    // keep_screen_on a real stored bool (the netstandard no-op would hide the facade path) and
    // set_main_display_info drives the base's dedupe+raise while the metrics listeners run.
    class headless_device_display final : public detail::device_display_base
    {
    public:
        void set_main_display_info(const display_info& value)
        {
            info_ = value;
            if (listening_)
            {
                on_main_display_info_changed();
            }
        }

        [[nodiscard]] bool is_screen_metrics_listening() const
        {
            return listening_;
        }

    protected:
        [[nodiscard]] display_info platform_get_main_display_info() const override
        {
            return info_;
        }
        [[nodiscard]] bool platform_get_keep_screen_on() const override
        {
            return keep_screen_on_;
        }
        void platform_set_keep_screen_on(bool value) override
        {
            keep_screen_on_ = value;
        }
        void platform_start_screen_metrics_listeners() override
        {
            listening_ = true;
        }
        void platform_stop_screen_metrics_listeners() override
        {
            listening_ = false;
        }

    private:
        display_info info_;
        bool keep_screen_on_ = false;
        bool listening_ = false;
    };

    // BatteryImplementation (netstandard): every member (including the listener starts behind the
    // event accessors) throws - until the respective values are configured.
    class headless_battery final : public detail::battery_base
    {
    public:
        [[nodiscard]] double charge_level() const override
        {
            return headless_detail::require(charge_level_);
        }
        [[nodiscard]] battery_state state() const override
        {
            return headless_detail::require(state_);
        }
        [[nodiscard]] battery_power_source power_source() const override
        {
            return headless_detail::require(power_source_);
        }
        [[nodiscard]] enum energy_saver_status energy_saver_status() const override
        {
            return headless_detail::require(energy_saver_status_);
        }

        void set_charge_level(double value)
        {
            charge_level_ = value;
        }
        void set_state(battery_state value)
        {
            state_ = value;
        }
        void set_power_source(battery_power_source value)
        {
            power_source_ = value;
        }
        void set_energy_saver_status(enum energy_saver_status value)
        {
            energy_saver_status_ = value;
        }

        // Drive the shared dedupe+raise paths (the platform listener callbacks' role).
        void simulate_battery_info_changed()
        {
            on_battery_info_changed();
        }
        void simulate_energy_saver_changed()
        {
            on_energy_saver_changed();
        }

        [[nodiscard]] bool is_battery_listening() const
        {
            return battery_listening_;
        }
        [[nodiscard]] bool is_energy_saver_listening() const
        {
            return energy_saver_listening_;
        }

    protected:
        void platform_start_battery_listeners() override
        {
            if (!charge_level_ && !state_ && !power_source_)
            {
                headless_detail::throw_not_implemented(); // netstandard StartBatteryListeners
            }
            battery_listening_ = true;
        }
        void platform_stop_battery_listeners() override
        {
            battery_listening_ = false;
        }
        void platform_start_energy_saver_listeners() override
        {
            if (!energy_saver_status_)
            {
                headless_detail::throw_not_implemented(); // netstandard StartEnergySaverListeners
            }
            energy_saver_listening_ = true;
        }
        void platform_stop_energy_saver_listeners() override
        {
            energy_saver_listening_ = false;
        }

    private:
        std::optional<double> charge_level_;
        std::optional<battery_state> state_;
        std::optional<battery_power_source> power_source_;
        std::optional<enum energy_saver_status> energy_saver_status_;
        bool battery_listening_ = false;
        bool energy_saver_listening_ = false;
    };

    // FlashlightImplementation (netstandard): is_supported is FALSE (not a throw - the C# partial
    // returns Task.FromResult(false)); turn_on/turn_off throw until supported.
    class headless_flashlight final : public i_flashlight
    {
    public:
        [[nodiscard]] bool is_supported() override
        {
            return supported_;
        }
        void turn_on() override
        {
            if (!supported_)
            {
                headless_detail::throw_not_implemented();
            }
            on_ = true;
        }
        void turn_off() override
        {
            if (!supported_)
            {
                headless_detail::throw_not_implemented();
            }
            on_ = false;
        }

        void set_is_supported(bool value)
        {
            supported_ = value;
        }
        [[nodiscard]] bool is_on() const
        {
            return on_;
        }

    private:
        bool supported_ = false;
        bool on_ = false;
    };

    // VibrationImplementation (netstandard): IsSupported throws (so the shared gate makes every
    // member throw) - until configured; then the platform hooks record what was requested.
    class headless_vibration final : public detail::vibration_base
    {
    public:
        [[nodiscard]] bool is_supported() const override
        {
            return headless_detail::require(supported_);
        }

        void set_is_supported(bool value)
        {
            supported_ = value;
        }
        [[nodiscard]] bool is_vibrating() const
        {
            return vibrating_;
        }
        // The clamped duration the platform hook received (vibrate() records the C# default 500 ms).
        [[nodiscard]] std::optional<std::chrono::milliseconds> last_duration() const
        {
            return last_duration_;
        }

    protected:
        void platform_vibrate() override
        {
            vibrating_ = true;
            last_duration_ = std::chrono::milliseconds{500};
        }
        void platform_vibrate(std::chrono::milliseconds duration) override
        {
            vibrating_ = true;
            last_duration_ = duration;
        }
        void platform_cancel() override
        {
            vibrating_ = false;
        }

    private:
        std::optional<bool> supported_;
        std::optional<std::chrono::milliseconds> last_duration_;
        bool vibrating_ = false;
    };

    // HapticFeedbackImplementation (netstandard): IsSupported and Perform throw - until configured.
    class headless_haptic_feedback final : public i_haptic_feedback
    {
    public:
        [[nodiscard]] bool is_supported() const override
        {
            return headless_detail::require(supported_);
        }
        void perform(haptic_feedback_type type) override
        {
            if (!supported_.has_value())
            {
                headless_detail::throw_not_implemented();
            }
            last_performed_ = type;
        }

        void set_is_supported(bool value)
        {
            supported_ = value;
        }
        [[nodiscard]] std::optional<haptic_feedback_type> last_performed() const
        {
            return last_performed_;
        }

    private:
        std::optional<bool> supported_;
        std::optional<haptic_feedback_type> last_performed_;
    };
} // namespace maui::devices

namespace maui::devices::sensors
{
    namespace headless_detail = maui::devices::headless_detail;

    // The shared shape of the five basic_sensor fakes: IsSupported throws until configured (the
    // netstandard mirror); start/stop record; simulate_reading drives the shared raise path.
    template <class Interface, class Data> class headless_basic_sensor : public detail::basic_sensor<Interface, Data>
    {
    public:
        void set_is_supported(bool value)
        {
            supported_ = value;
        }
        void simulate_reading(const Data& reading)
        {
            this->raise_reading_changed(reading);
        }
        [[nodiscard]] std::optional<sensor_speed> started_speed() const
        {
            return started_speed_;
        }

    protected:
        using detail::basic_sensor<Interface, Data>::basic_sensor;

        [[nodiscard]] bool platform_is_supported() const override
        {
            return headless_detail::require(supported_);
        }
        void platform_start(sensor_speed speed) override
        {
            started_speed_ = speed;
        }
        void platform_stop() override
        {
            started_speed_.reset();
        }

    private:
        std::optional<bool> supported_;
        std::optional<sensor_speed> started_speed_;
    };

    // The accelerometer fake derives the shake-capable base directly: simulate_reading runs
    // OnChanged (reading raise + shake detection), and the shake clock is settable so the queue's
    // half-second window is deterministic.
    class headless_accelerometer final : public detail::accelerometer_base
    {
    public:
        void set_is_supported(bool value)
        {
            supported_ = value;
        }
        void simulate_reading(const accelerometer_data& reading)
        {
            on_changed(reading);
        }
        void set_now_nanoseconds(std::int64_t value)
        {
            now_override_ = value;
        }
        [[nodiscard]] std::optional<sensor_speed> started_speed() const
        {
            return started_speed_;
        }

    protected:
        [[nodiscard]] bool platform_is_supported() const override
        {
            return headless_detail::require(supported_);
        }
        void platform_start(sensor_speed speed) override
        {
            started_speed_ = speed;
        }
        void platform_stop() override
        {
            started_speed_.reset();
        }
        [[nodiscard]] std::int64_t now_nanoseconds() const override
        {
            return now_override_.has_value() ? *now_override_ : accelerometer_base::now_nanoseconds();
        }

    private:
        std::optional<bool> supported_;
        std::optional<sensor_speed> started_speed_;
        std::optional<std::int64_t> now_override_;
    };

    // Concrete fakes pin each sensor's C# double-start message ("<Sensor> has already been started.").
    class headless_gyroscope final : public headless_basic_sensor<i_gyroscope, gyroscope_data>
    {
    public:
        headless_gyroscope() : headless_basic_sensor("Gyroscope")
        {
        }
    };

    class headless_magnetometer final : public headless_basic_sensor<i_magnetometer, magnetometer_data>
    {
    public:
        headless_magnetometer() : headless_basic_sensor("Magnetometer")
        {
        }
    };

    class headless_barometer final : public headless_basic_sensor<i_barometer, barometer_data>
    {
    public:
        headless_barometer() : headless_basic_sensor("Barometer")
        {
        }
    };

    class headless_orientation_sensor final
        : public headless_basic_sensor<i_orientation_sensor, orientation_sensor_data>
    {
    public:
        headless_orientation_sensor() : headless_basic_sensor("Orientation sensor")
        {
        }
    };

    // The compass fake (its base carries the applyLowPassFilter start flag).
    class headless_compass final : public detail::compass_base
    {
    public:
        void set_is_supported(bool value)
        {
            supported_ = value;
        }
        void simulate_reading(const compass_data& reading)
        {
            raise_reading_changed(reading);
        }
        [[nodiscard]] std::optional<sensor_speed> started_speed() const
        {
            return started_speed_;
        }
        [[nodiscard]] bool started_with_low_pass_filter() const
        {
            return apply_low_pass_filter_;
        }

    protected:
        [[nodiscard]] bool platform_is_supported() const override
        {
            return headless_detail::require(supported_);
        }
        void platform_start(sensor_speed speed, bool apply_low_pass_filter) override
        {
            started_speed_ = speed;
            apply_low_pass_filter_ = apply_low_pass_filter;
        }
        void platform_stop() override
        {
            started_speed_.reset();
        }

    private:
        std::optional<bool> supported_;
        std::optional<sensor_speed> started_speed_;
        bool apply_low_pass_filter_ = false;
    };

    // GeolocationImplementation (netstandard): is_enabled / is_listening_foreground are false and
    // everything else throws - until configured. Configured, it mirrors the CoreLocation partial's
    // contract (not-enabled throw, double-listen logic_error, failure stops listening first).
    class headless_geolocation final : public detail::geolocation_base
    {
    public:
        void get_last_known_location_async(location_callback on_complete) override
        {
            if (!configured_)
            {
                headless_detail::throw_not_implemented();
            }
            require_enabled();
            on_complete(last_known_location_);
        }

        void get_location_async(const geolocation_request& request, maui::core::cancellation_token token,
                                location_callback on_complete) override
        {
            if (!configured_)
            {
                headless_detail::throw_not_implemented();
            }
            require_enabled();
            last_request_ = request;
            // A cancelled query completes with no location (the CLLocationManager Cancel path).
            on_complete(token.is_cancelled() ? std::nullopt : current_location_);
        }

        [[nodiscard]] bool is_listening_foreground() const override
        {
            return listening_;
        }
        [[nodiscard]] bool is_enabled() const override
        {
            return enabled_;
        }

        bool start_listening_foreground(const geolocation_listening_request& request) override
        {
            if (!configured_)
            {
                headless_detail::throw_not_implemented();
            }
            if (listening_)
            {
                throw std::logic_error("Already listening to location changes.");
            }
            require_enabled();
            last_listening_request_ = request;
            listening_ = true;
            return true;
        }

        void stop_listening_foreground() override
        {
            listening_ = false;
        }

        void set_is_enabled(bool value)
        {
            configured_ = true;
            enabled_ = value;
        }
        void set_last_known_location(std::optional<location> value)
        {
            configured_ = true;
            last_known_location_ = std::move(value);
        }
        void set_current_location(std::optional<location> value)
        {
            configured_ = true;
            current_location_ = std::move(value);
        }

        // A listener-delivered update / failure (failure stops listening BEFORE raising, like the
        // ContinuousLocationListener error path).
        void simulate_location_update(const location& value)
        {
            if (listening_)
            {
                on_location_changed(value);
            }
        }
        void simulate_listening_failed(geolocation_error error)
        {
            if (!listening_)
            {
                return;
            }
            stop_listening_foreground();
            on_location_error(error);
        }

        [[nodiscard]] std::optional<geolocation_request> last_request() const
        {
            return last_request_;
        }
        [[nodiscard]] std::optional<geolocation_listening_request> last_listening_request() const
        {
            return last_listening_request_;
        }

    private:
        void require_enabled() const
        {
            if (!enabled_)
            {
                // FeatureNotEnabledException, folded per the lib error-model rule.
                throw maui::application_model::feature_not_supported("Location services are not enabled on device.");
            }
        }

        bool configured_ = false;
        bool enabled_ = false;
        bool listening_ = false;
        std::optional<location> last_known_location_;
        std::optional<location> current_location_;
        std::optional<geolocation_request> last_request_;
        std::optional<geolocation_listening_request> last_listening_request_;
    };

    // GeocodingImplementation (netstandard): both queries throw - until results are staged.
    class headless_geocoding final : public i_geocoding
    {
    public:
        void get_placemarks_async(double latitude, double longitude, placemarks_callback on_complete) override
        {
            if (!placemarks_.has_value())
            {
                headless_detail::throw_not_implemented();
            }
            last_coordinates_ = location{latitude, longitude};
            on_complete(*placemarks_);
        }

        void get_locations_async(std::string_view address, locations_callback on_complete) override
        {
            if (!locations_.has_value())
            {
                headless_detail::throw_not_implemented();
            }
            last_address_ = std::string(address);
            on_complete(*locations_);
        }

        void set_placemarks(std::vector<placemark> value)
        {
            placemarks_ = std::move(value);
        }
        void set_locations(std::vector<location> value)
        {
            locations_ = std::move(value);
        }
        [[nodiscard]] std::optional<location> last_coordinates() const
        {
            return last_coordinates_;
        }
        [[nodiscard]] std::optional<std::string> last_address() const
        {
            return last_address_;
        }

    private:
        std::optional<std::vector<placemark>> placemarks_;
        std::optional<std::vector<location>> locations_;
        std::optional<location> last_coordinates_;
        std::optional<std::string> last_address_;
    };
} // namespace maui::devices::sensors

namespace maui::networking
{
    namespace headless_detail = maui::devices::headless_detail;

    // ConnectivityImplementation (netstandard): NetworkAccess, ConnectionProfiles, and the
    // listener start hidden behind the ConnectivityChanged add all throw - until configured. Once
    // the access/profiles are staged, the shared dedupe + listener lifecycle (connectivity_base)
    // runs for real, and simulate_connectivity_changed() drives the platform-callback raise path.
    class headless_connectivity final : public detail::connectivity_base
    {
    public:
        [[nodiscard]] enum network_access network_access() const override
        {
            return headless_detail::require(network_access_);
        }
        [[nodiscard]] std::vector<connection_profile> connection_profiles() const override
        {
            return headless_detail::require(connection_profiles_);
        }

        void set_network_access(enum network_access value)
        {
            network_access_ = value;
        }
        void set_connection_profiles(std::vector<connection_profile> value)
        {
            connection_profiles_ = std::move(value);
        }

        // A connectivity change arriving from the platform (the path monitor callback's role).
        void simulate_connectivity_changed()
        {
            on_connectivity_changed();
        }

        [[nodiscard]] bool is_listening() const
        {
            return listening_;
        }

    protected:
        void platform_start_listeners() override
        {
            if (!network_access_.has_value() && !connection_profiles_.has_value())
            {
                headless_detail::throw_not_implemented(); // netstandard StartListeners
            }
            listening_ = true;
        }
        void platform_stop_listeners() override
        {
            listening_ = false;
        }

    private:
        std::optional<enum network_access> network_access_;
        std::optional<std::vector<connection_profile>> connection_profiles_;
        bool listening_ = false;
    };
} // namespace maui::networking

namespace maui::media
{
    namespace headless_detail = maui::devices::headless_detail;

    // TextToSpeechImplementation (netstandard): PlatformGetLocalesAsync and PlatformSpeakAsync both
    // throw - until configured. Configured, get_locales hands back the staged locale list and
    // speak records each spoken (text, options) pair then completes inline (the headless analog of
    // the engine finishing the utterance); a cancelled token still records + completes (mirroring
    // the platforms' TryCancel, which also resolves the task). The shared validation in
    // text_to_speech_base runs for real either way.
    class headless_text_to_speech final : public detail::text_to_speech_base
    {
    public:
        struct spoken
        {
            std::string text;
            std::optional<speech_options> options;
            bool cancelled = false;
        };

        void set_locales(std::vector<locale> value)
        {
            locales_ = std::move(value);
        }

        [[nodiscard]] const std::vector<spoken>& spoken_utterances() const
        {
            return spoken_;
        }

    protected:
        void platform_get_locales_async(locales_callback on_complete) override
        {
            on_complete(headless_detail::require(locales_));
        }

        void platform_speak_async(std::string_view text, const std::optional<speech_options>& options,
                                  maui::core::cancellation_token token, speak_callback on_complete) override
        {
            if (!locales_.has_value())
            {
                headless_detail::throw_not_implemented(); // netstandard PlatformSpeakAsync
            }
            spoken_.push_back(spoken{std::string(text), options, token.is_cancelled()});
            if (on_complete)
            {
                on_complete();
            }
        }

    private:
        std::optional<std::vector<locale>> locales_;
        std::vector<spoken> spoken_;
    };

    // The canned IScreenshotResult the headless screenshot fake hands back: stored width/height +
    // encoded bytes; open_read_async replays the bytes inline (the headless stream stand-in).
    class headless_screenshot_result final : public i_screenshot_result
    {
    public:
        headless_screenshot_result(int width, int height, std::vector<std::byte> bytes)
            : width_(width), height_(height), bytes_(std::move(bytes))
        {
        }

        [[nodiscard]] int width() const override
        {
            return width_;
        }
        [[nodiscard]] int height() const override
        {
            return height_;
        }

        void open_read_async(screenshot_format format, int quality, read_callback on_complete) override
        {
            last_format_ = format;
            last_quality_ = quality;
            on_complete(bytes_);
        }

        [[nodiscard]] std::optional<screenshot_format> last_format() const
        {
            return last_format_;
        }
        [[nodiscard]] std::optional<int> last_quality() const
        {
            return last_quality_;
        }

    private:
        int width_;
        int height_;
        std::vector<std::byte> bytes_;
        std::optional<screenshot_format> last_format_;
        std::optional<int> last_quality_;
    };

    // ScreenshotImplementation (netstandard, the macOS mirror): IsCaptureSupported and CaptureAsync
    // both throw - until configured. Configured supported=true, capture_async hands back a canned
    // headless_screenshot_result with the staged width/height/bytes.
    class headless_screenshot final : public i_screenshot
    {
    public:
        [[nodiscard]] bool is_capture_supported() const override
        {
            if (!supported_.has_value())
            {
                headless_detail::throw_not_implemented();
            }
            return *supported_;
        }

        void capture_async(screenshot_callback on_complete) override
        {
            if (!supported_.has_value() || !*supported_)
            {
                headless_detail::throw_not_implemented();
            }
            on_complete(std::make_shared<headless_screenshot_result>(width_, height_, bytes_));
        }

        void set_is_capture_supported(bool value)
        {
            supported_ = value;
        }
        void set_result(int width, int height, std::vector<std::byte> bytes)
        {
            width_ = width;
            height_ = height;
            bytes_ = std::move(bytes);
        }

    private:
        std::optional<bool> supported_;
        int width_ = 0;
        int height_ = 0;
        std::vector<std::byte> bytes_;
    };
} // namespace maui::media
