// flashlight - iOS (UIKit) platform partial. Ported from Flashlight.ios.cs: the default video
// AVCaptureDevice's torch is the flashlight. IsSupported = a device exists with torch or flash;
// Toggle locks the device configuration and drives the torch level
// (setTorchModeOnWithLevel:AVCaptureMaxAvailableTorchLevel). The C# partial also pokes the
// deprecated still-image FlashMode (with analyzer suppressions); the port drives the torch only -
// every AVCaptureDevice with a flash has had a torch since the API's introduction, and FlashMode
// has been deprecated since iOS 10. On the SIMULATOR there is no capture device: is_supported()
// is false and turn_on/turn_off throw feature_not_supported (the C# Toggle's null-device throw).

#import <AVFoundation/AVFoundation.h>

#include <memory>

#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/flashlight.hpp"

namespace maui::devices
{
    namespace
    {
        class ios_flashlight final : public i_flashlight
        {
        public:
            [[nodiscard]] bool is_supported() override
            {
                AVCaptureDevice* const device = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
                return device != nil && (device.hasFlash || device.hasTorch);
            }

            void turn_on() override
            {
                toggle(true);
            }

            void turn_off() override
            {
                toggle(false);
            }

        private:
            static void toggle(bool on)
            {
                AVCaptureDevice* const device = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
                if (device == nil || !(device.hasFlash || device.hasTorch))
                {
                    throw maui::application_model::feature_not_supported();
                }

                NSError* error = nil;
                [device lockForConfiguration:&error];
                if (error == nil && device.hasTorch)
                {
                    if (on)
                    {
                        NSError* torch_error = nil;
                        [device setTorchModeOnWithLevel:AVCaptureMaxAvailableTorchLevel error:&torch_error];
                    }
                    else
                    {
                        device.torchMode = AVCaptureTorchModeOff;
                    }
                }
                [device unlockForConfiguration];
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_flashlight> make_flashlight()
        {
            return std::make_shared<ios_flashlight>();
        }
    } // namespace detail
} // namespace maui::devices
