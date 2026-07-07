#pragma once
// maui::devices::detail::vibration_base  <=  the cross-platform half of
// Microsoft.Maui.Devices.VibrationImplementation (Vibration.shared.cs): every member gates on
// is_supported() (throwing feature_not_supported - the C# FeatureNotSupportedException), and the
// duration overload clamps to [0 ms, 5 s] before reaching the platform hook.

#include <algorithm>
#include <chrono>

#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/vibration.hpp"

namespace maui::devices::detail
{
    class vibration_base : public i_vibration
    {
    public:
        void vibrate() override
        {
            if (!is_supported())
            {
                throw maui::application_model::feature_not_supported();
            }
            platform_vibrate();
        }

        void vibrate(std::chrono::milliseconds duration) override
        {
            if (!is_supported())
            {
                throw maui::application_model::feature_not_supported();
            }
            duration = std::clamp(duration, std::chrono::milliseconds::zero(), std::chrono::milliseconds{5000});
            platform_vibrate(duration);
        }

        void cancel() override
        {
            if (!is_supported())
            {
                throw maui::application_model::feature_not_supported();
            }
            platform_cancel();
        }

    protected:
        vibration_base() = default;

        // PlatformVibrate() / PlatformVibrate(TimeSpan) / PlatformCancel().
        virtual void platform_vibrate() = 0;
        virtual void platform_vibrate(std::chrono::milliseconds duration) = 0;
        virtual void platform_cancel() = 0;
    };
} // namespace maui::devices::detail
