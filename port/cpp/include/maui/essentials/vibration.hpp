#pragma once
// maui::devices::vibration    <=  Microsoft.Maui.Devices.Vibration (static facade)
// maui::devices::i_vibration  <=  Microsoft.Maui.Devices.IVibration
//
// Makes the device vibrate. The shared partial's gate + clamp behavior (every member throws
// feature_not_supported when is_supported() is false; durations clamp to [0, 5000] ms; iOS always
// vibrates ~500 ms regardless) lives in detail::vibration_base (src/essentials/detail/) - the
// platform partials supply platform_is_supported / platform_vibrate / platform_cancel.
// VibrationExtensions.Vibrate(double ms) collapses into the std::chrono::milliseconds overload.
//
// Backends (suffix oracle): ios REAL (Vibration.ios.cs - AudioToolbox's system vibrate sound;
// is_supported is true, cancel is a no-op). apple/macOS NOT SUPPORTED
// (Vibration.netstandard.tvos.watchos.macos.cs - everything throws). Headless mirrors netstandard
// until faked.

#include <chrono>
#include <memory>

namespace maui::devices
{
    class i_vibration
    {
    public:
        virtual ~i_vibration() = default;

        [[nodiscard]] virtual bool is_supported() const = 0;
        // Vibrate the device for 500 ms.
        virtual void vibrate() = 0;
        // Vibrate for `duration`, clamped to [0, 5000] ms (ignored on iOS - always ~500 ms).
        virtual void vibrate(std::chrono::milliseconds duration) = 0;
        // Cancel any current vibration.
        virtual void cancel() = 0;

    protected:
        i_vibration() = default;
        i_vibration(const i_vibration&) = default;
        i_vibration(i_vibration&&) = default;
        i_vibration& operator=(const i_vibration&) = default;
        i_vibration& operator=(i_vibration&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (VibrationImplementation), one per backend under
        // src/platform/<backend>/essentials_vibration.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_vibration> make_vibration();
    } // namespace detail

    // The static facade over vibration::default_() (C# Vibration.Default).
    class vibration final
    {
    public:
        vibration() = delete;

        [[nodiscard]] static bool is_supported()
        {
            return default_().is_supported();
        }
        static void vibrate()
        {
            default_().vibrate();
        }
        static void vibrate(std::chrono::milliseconds duration)
        {
            default_().vibrate(duration);
        }
        static void cancel()
        {
            default_().cancel();
        }

        // Vibration.Default (lazy platform default) + SetDefault (the test seam; nullptr resets).
        [[nodiscard]] static i_vibration& default_();
        static void set_default(std::shared_ptr<i_vibration> implementation);
    };
} // namespace maui::devices
