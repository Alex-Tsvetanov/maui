#pragma once
// maui::devices::flashlight    <=  Microsoft.Maui.Devices.Flashlight (static facade)
// maui::devices::i_flashlight  <=  Microsoft.Maui.Devices.IFlashlight
//
// Turns the camera flash into a flashlight. The C# surface is Task-based (IsSupportedAsync /
// TurnOnAsync / TurnOffAsync) but every Apple implementation completes synchronously
// (Task.FromResult / Task.CompletedTask around AVCaptureDevice calls); the port - which has no
// task type yet - exposes the synchronous equivalents and documents the mapping per member.
//
// Backends (suffix oracle): ios REAL (Flashlight.ios.cs - AVCaptureDevice torch/flash; the
// simulator has no capture device, so is_supported() is false there and turn_on/off throw).
// apple/macOS NOT SUPPORTED (Flashlight.netstandard.tvos.watchos.macos.cs) - is_supported()
// returns false and turn_on/turn_off throw feature_not_supported, exactly like the netstandard
// partial. Headless mirrors netstandard until faked.

#include <memory>

namespace maui::devices
{
    class i_flashlight
    {
    public:
        virtual ~i_flashlight() = default;

        // IFlashlight.IsSupportedAsync (synchronous in every concrete implementation).
        [[nodiscard]] virtual bool is_supported() = 0;
        // IFlashlight.TurnOnAsync - throws feature_not_supported when no torch/flash exists.
        virtual void turn_on() = 0;
        // IFlashlight.TurnOffAsync - throws feature_not_supported when no torch/flash exists.
        virtual void turn_off() = 0;

    protected:
        i_flashlight() = default;
        i_flashlight(const i_flashlight&) = default;
        i_flashlight(i_flashlight&&) = default;
        i_flashlight& operator=(const i_flashlight&) = default;
        i_flashlight& operator=(i_flashlight&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (FlashlightImplementation), one per backend under
        // src/platform/<backend>/essentials_flashlight.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_flashlight> make_flashlight();
    } // namespace detail

    // The static facade over flashlight::default_() (C# Flashlight.Default).
    class flashlight final
    {
    public:
        flashlight() = delete;

        [[nodiscard]] static bool is_supported()
        {
            return default_().is_supported();
        }
        static void turn_on()
        {
            default_().turn_on();
        }
        static void turn_off()
        {
            default_().turn_off();
        }

        // Flashlight.Default (lazy platform default) + SetDefault (the test seam; nullptr resets).
        [[nodiscard]] static i_flashlight& default_();
        static void set_default(std::shared_ptr<i_flashlight> implementation);
    };
} // namespace maui::devices
