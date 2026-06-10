// vibration - iOS (UIKit) platform partial. Ported 1:1 from Vibration.ios.cs: IsSupported is
// true, every vibrate plays the system vibrate sound (AudioToolbox kSystemSoundID_Vibrate - iOS
// always vibrates ~500 ms regardless of the requested duration), and Cancel is a no-op. The
// shared clamp/gate lives in vibration_base.

#import <AudioToolbox/AudioToolbox.h>

#include <chrono>
#include <memory>

#include "maui/essentials/vibration.hpp"

#include "src/essentials/detail/vibration_base.hpp"

namespace maui::devices
{
    namespace
    {
        class ios_vibration final : public detail::vibration_base
        {
        public:
            [[nodiscard]] bool is_supported() const override
            {
                return true;
            }

        protected:
            void platform_vibrate() override
            {
                AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
            }

            void platform_vibrate(std::chrono::milliseconds /*duration*/) override
            {
                // The duration is ignored on iOS - the system vibration is always ~500 ms.
                AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
            }

            void platform_cancel() override
            {
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_vibration> make_vibration()
        {
            return std::make_shared<ios_vibration>();
        }
    } // namespace detail
} // namespace maui::devices
