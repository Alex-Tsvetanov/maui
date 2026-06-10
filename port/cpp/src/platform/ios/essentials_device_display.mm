// device_display - iOS (UIKit) platform partial. Ported from DeviceDisplay.ios.cs:
//   - metrics: UIScreen.mainScreen bounds scaled by .scale; density = scale; refresh rate =
//     maximumFramesPerSecond; orientation/rotation derive from the interface orientation
//     (CalculateOrientation/CalculateRotation). C# reads the deprecated
//     UIApplication.StatusBarOrientation (with analyzer suppressions); the port reads the modern
//     equivalent - the first connected UIWindowScene's interfaceOrientation - with the same
//     mapping (no scene, e.g. a plain spawned test process, behaves like Unknown: portrait +
//     unknown rotation, matching the C# switch defaults).
//   - keep-screen-on: UIApplication.sharedApplication.idleTimerDisabled (no-op without an app
//     instance).
//   - listeners: C# observes the deprecated DidChangeStatusBarOrientationNotification; the port
//     subscribes the supported stand-in (UIDeviceOrientationDidChangeNotification with
//     begin/endGeneratingDeviceOrientationNotifications), feeding the same
//     on_main_display_info_changed dedupe.

#import <UIKit/UIKit.h>

#include <memory>

#include "maui/essentials/device_display.hpp"

#include "src/essentials/detail/device_display_base.hpp"

namespace maui::devices
{
    namespace
    {
        UIInterfaceOrientation current_interface_orientation()
        {
            UIApplication* const application = [UIApplication sharedApplication];
            NSArray<UIScene*>* const scenes = application.connectedScenes.allObjects;
            for (NSUInteger i = 0; i < scenes.count; ++i)
            {
                UIScene* const scene = scenes[i];
                if ([scene isKindOfClass:[UIWindowScene class]])
                {
                    return ((UIWindowScene*)scene).interfaceOrientation;
                }
            }
            return UIInterfaceOrientationUnknown;
        }

        class ios_device_display final : public detail::device_display_base
        {
        public:
            ~ios_device_display() override
            {
                platform_stop_screen_metrics_listeners();
            }
            ios_device_display() = default;
            ios_device_display(const ios_device_display&) = delete;
            ios_device_display(ios_device_display&&) = delete;
            ios_device_display& operator=(const ios_device_display&) = delete;
            ios_device_display& operator=(ios_device_display&&) = delete;

        protected:
            [[nodiscard]] display_info platform_get_main_display_info() const override
            {
                UIScreen* const screen = [UIScreen mainScreen];
                const CGRect bounds = screen.bounds;
                const CGFloat scale = screen.scale;
                const UIInterfaceOrientation orientation = current_interface_orientation();

                display_info info;
                info.width = bounds.size.width * scale;
                info.height = bounds.size.height * scale;
                info.density = scale;
                // CalculateOrientation: IsLandscape ? Landscape : Portrait.
                info.orientation = UIInterfaceOrientationIsLandscape(orientation) ? display_orientation::landscape
                                                                                  : display_orientation::portrait;
                // CalculateRotation.
                switch (orientation)
                {
                    case UIInterfaceOrientationPortrait:
                        info.rotation = display_rotation::rotation_0;
                        break;
                    case UIInterfaceOrientationPortraitUpsideDown:
                        info.rotation = display_rotation::rotation_180;
                        break;
                    case UIInterfaceOrientationLandscapeLeft:
                        info.rotation = display_rotation::rotation_270;
                        break;
                    case UIInterfaceOrientationLandscapeRight:
                        info.rotation = display_rotation::rotation_90;
                        break;
                    default:
                        info.rotation = display_rotation::unknown;
                        break;
                }
                info.refresh_rate = static_cast<float>(screen.maximumFramesPerSecond);
                return info;
            }

            [[nodiscard]] bool platform_get_keep_screen_on() const override
            {
                UIApplication* const application = [UIApplication sharedApplication];
                return application != nil && application.idleTimerDisabled;
            }

            void platform_set_keep_screen_on(bool value) override
            {
                UIApplication* const application = [UIApplication sharedApplication];
                if (application != nil)
                {
                    application.idleTimerDisabled = value;
                }
            }

            void platform_start_screen_metrics_listeners() override
            {
                [[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
                observer_ =
                    [[NSNotificationCenter defaultCenter] addObserverForName:UIDeviceOrientationDidChangeNotification
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
                    [[UIDevice currentDevice] endGeneratingDeviceOrientationNotifications];
                }
            }

        private:
            id observer_ = nil; // ARC-managed strong reference
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_device_display> make_device_display()
        {
            return std::make_shared<ios_device_display>();
        }
    } // namespace detail
} // namespace maui::devices
