// device_display - Apple (AppKit / macOS) platform partial. MAUI itself has NO
// DeviceDisplay.macos.cs (macOS falls in the netstandard partial - MAUI's mac story is Catalyst);
// this unit was directed to ship a real AppKit implementation, so it follows the retired
// Xamarin.Essentials DeviceDisplay.macos.cs recipe, kept consistent with the
// DeviceDisplayImplementationBase contract:
//   - metrics: NSScreen.mainScreen frame scaled by backingScaleFactor; density =
//     backingScaleFactor; orientation portrait iff height > width (desktops report landscape);
//     rotation fixed at Rotation0; refresh rate via CGDisplayModeGetRefreshRate of the main display.
//   - keep-screen-on: an IOPM "PreventUserIdleDisplaySleep" power assertion named "KeepScreenOn"
//     (IOKit.PreventUserIdleDisplaySleep / AllowUserIdleDisplaySleep); the getter reports whether
//     the assertion is held.
//   - listeners: NSApplicationDidChangeScreenParametersNotification on the main queue ->
//     on_main_display_info_changed() (the base dedupes by DisplayInfo equality).

#import <AppKit/AppKit.h>
#include <CoreGraphics/CoreGraphics.h>
#include <IOKit/pwr_mgt/IOPMLib.h>

#include <memory>

#include "maui/essentials/device_display.hpp"

#include "src/essentials/detail/device_display_base.hpp"

namespace maui::devices
{
    namespace
    {
        // CoreGraphicsInterop.GetRefreshRate(CGMainDisplayID()).
        float main_display_refresh_rate()
        {
            const CGDirectDisplayID display = CGMainDisplayID();
            CGDisplayModeRef mode = CGDisplayCopyDisplayMode(display);
            if (mode == nullptr)
            {
                return 0;
            }
            const double rate = CGDisplayModeGetRefreshRate(mode);
            CGDisplayModeRelease(mode);
            return static_cast<float>(rate);
        }

        class apple_device_display final : public detail::device_display_base
        {
        public:
            ~apple_device_display() override
            {
                platform_stop_screen_metrics_listeners();
                platform_set_keep_screen_on(false); // release a held power assertion
            }
            apple_device_display() = default;
            apple_device_display(const apple_device_display&) = delete;
            apple_device_display(apple_device_display&&) = delete;
            apple_device_display& operator=(const apple_device_display&) = delete;
            apple_device_display& operator=(apple_device_display&&) = delete;

        protected:
            [[nodiscard]] display_info platform_get_main_display_info() const override
            {
                NSScreen* const screen = [NSScreen mainScreen];
                if (screen == nil)
                {
                    return {};
                }
                const NSRect frame = [screen frame];
                const CGFloat scale = [screen backingScaleFactor];
                display_info info;
                info.width = frame.size.width * scale;
                info.height = frame.size.height * scale;
                info.density = scale;
                info.orientation = frame.size.height > frame.size.width ? display_orientation::portrait
                                                                        : display_orientation::landscape;
                info.rotation = display_rotation::rotation_0;
                info.refresh_rate = main_display_refresh_rate();
                return info;
            }

            [[nodiscard]] bool platform_get_keep_screen_on() const override
            {
                return assertion_id_ != kIOPMNullAssertionID;
            }

            void platform_set_keep_screen_on(bool value) override
            {
                if (value == platform_get_keep_screen_on())
                {
                    return;
                }
                if (value)
                {
                    IOPMAssertionID assertion_id = kIOPMNullAssertionID;
                    const IOReturn result =
                        IOPMAssertionCreateWithName(CFSTR("PreventUserIdleDisplaySleep"), kIOPMAssertionLevelOn,
                                                    CFSTR("KeepScreenOn"), &assertion_id);
                    assertion_id_ = result == kIOReturnSuccess ? assertion_id : kIOPMNullAssertionID;
                }
                else
                {
                    IOPMAssertionRelease(assertion_id_);
                    assertion_id_ = kIOPMNullAssertionID;
                }
            }

            void platform_start_screen_metrics_listeners() override
            {
                observer_ = [[NSNotificationCenter defaultCenter]
                    addObserverForName:NSApplicationDidChangeScreenParametersNotification
                                object:nil
                                 queue:[NSOperationQueue mainQueue]
                            usingBlock:^(NSNotification*) {
                              this->on_main_display_info_changed();
                            }];
            }

            void platform_stop_screen_metrics_listeners() override
            {
                if (observer_ != nil)
                {
                    [[NSNotificationCenter defaultCenter] removeObserver:observer_];
                    observer_ = nil;
                }
            }

        private:
            IOPMAssertionID assertion_id_ = kIOPMNullAssertionID;
            id observer_ = nil; // ARC-managed strong reference inside the C++ object
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_device_display> make_device_display()
        {
            return std::make_shared<apple_device_display>();
        }
    } // namespace detail
} // namespace maui::devices
