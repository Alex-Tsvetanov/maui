// battery - iOS (UIKit) platform partial. Ported 1:1 from Battery.ios.watchos.cs (the __IOS__
// branches): every read saves UIDevice.batteryMonitoringEnabled, enables monitoring, reads, and
// restores the prior flag; ChargeLevel is UIDevice.batteryLevel (-1 on the simulator); State maps
// UIDeviceBatteryState (default: Full when the level reads >= 1.0, else Unknown); PowerSource maps
// Charging/Full -> AC and Unplugged -> Battery; the battery listeners enable monitoring and
// observe the level/state notifications; the energy saver reads NSProcessInfo.lowPowerModeEnabled
// and observes NSProcessInfoPowerStateDidChangeNotification (marshalled to the main queue).

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#include <atomic>
#include <memory>

#include "maui/essentials/battery.hpp"

#include "src/essentials/detail/battery_base.hpp"

namespace maui::devices
{
    namespace
    {
        // The save/enable/read/restore pattern every C# property uses.
        template <class Read> auto with_battery_monitoring(const Read& read)
        {
            UIDevice* const device = [UIDevice currentDevice];
            const BOOL was_enabled = device.batteryMonitoringEnabled;
            device.batteryMonitoringEnabled = YES;
            const auto result = read();
            device.batteryMonitoringEnabled = was_enabled;
            return result;
        }

        class ios_battery final : public detail::battery_base
        {
        public:
            ~ios_battery() override
            {
                *alive_ = false; // gates the main-queue raise block already dispatched (see below)
                platform_stop_battery_listeners();
                platform_stop_energy_saver_listeners();
            }
            ios_battery() = default;
            ios_battery(const ios_battery&) = delete;
            ios_battery(ios_battery&&) = delete;
            ios_battery& operator=(const ios_battery&) = delete;
            ios_battery& operator=(ios_battery&&) = delete;

            [[nodiscard]] double charge_level() const override
            {
                return with_battery_monitoring([] { return double{[UIDevice currentDevice].batteryLevel}; });
            }

            [[nodiscard]] battery_state state() const override
            {
                return with_battery_monitoring([this] {
                    switch ([UIDevice currentDevice].batteryState)
                    {
                        case UIDeviceBatteryStateCharging:
                            return battery_state::charging;
                        case UIDeviceBatteryStateFull:
                            return battery_state::full;
                        case UIDeviceBatteryStateUnplugged:
                            return battery_state::discharging;
                        default:
                            return charge_level() >= 1.0 ? battery_state::full : battery_state::unknown;
                    }
                });
            }

            [[nodiscard]] battery_power_source power_source() const override
            {
                return with_battery_monitoring([] {
                    switch ([UIDevice currentDevice].batteryState)
                    {
                        case UIDeviceBatteryStateFull:
                        case UIDeviceBatteryStateCharging:
                            return battery_power_source::ac;
                        case UIDeviceBatteryStateUnplugged:
                            return battery_power_source::battery;
                        default:
                            return battery_power_source::unknown;
                    }
                });
            }

            [[nodiscard]] enum energy_saver_status energy_saver_status() const override
            {
                return [NSProcessInfo processInfo].lowPowerModeEnabled ? energy_saver_status::on
                                                                       : energy_saver_status::off;
            }

        protected:
            void platform_start_battery_listeners() override
            {
                [UIDevice currentDevice].batteryMonitoringEnabled = YES;
                NSNotificationCenter* const center = [NSNotificationCenter defaultCenter];
                level_observer_ = [center addObserverForName:UIDeviceBatteryLevelDidChangeNotification
                                                      object:nil
                                                       queue:[NSOperationQueue mainQueue]
                                                  usingBlock:^(NSNotification*) {
                                                    this->on_battery_info_changed();
                                                  }];
                state_observer_ = [center addObserverForName:UIDeviceBatteryStateDidChangeNotification
                                                      object:nil
                                                       queue:[NSOperationQueue mainQueue]
                                                  usingBlock:^(NSNotification*) {
                                                    this->on_battery_info_changed();
                                                  }];
            }

            void platform_stop_battery_listeners() override
            {
                [UIDevice currentDevice].batteryMonitoringEnabled = NO;
                NSNotificationCenter* const center = [NSNotificationCenter defaultCenter];
                if (level_observer_ != nil)
                {
                    [center removeObserver:level_observer_];
                    level_observer_ = nil;
                }
                if (state_observer_ != nil)
                {
                    [center removeObserver:state_observer_];
                    state_observer_ = nil;
                }
            }

            void platform_start_energy_saver_listeners() override
            {
                // PowerChangedNotification => BeginInvokeOnMainThread(OnEnergySaverChanged). The
                // notification arrives on the POSTING thread (queue:nil); the main-queue hop holds
                // the shared alive flag so a block already dispatched when the implementation is
                // destroyed (set_default swap) does not touch the dead object.
                const std::shared_ptr<std::atomic<bool>> alive = alive_;
                saver_observer_ = [[NSNotificationCenter defaultCenter]
                    addObserverForName:NSProcessInfoPowerStateDidChangeNotification
                                object:nil
                                 queue:nil
                            usingBlock:^(NSNotification*) {
                              dispatch_async(dispatch_get_main_queue(), ^{
                                if (*alive)
                                {
                                    this->on_energy_saver_changed();
                                }
                              });
                            }];
            }

            void platform_stop_energy_saver_listeners() override
            {
                if (saver_observer_ != nil)
                {
                    [[NSNotificationCenter defaultCenter] removeObserver:saver_observer_];
                    saver_observer_ = nil;
                }
            }

        private:
            std::shared_ptr<std::atomic<bool>> alive_ = std::make_shared<std::atomic<bool>>(true);
            id level_observer_ = nil; // ARC-managed strong references
            id state_observer_ = nil;
            id saver_observer_ = nil;
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_battery> make_battery()
        {
            return std::make_shared<ios_battery>();
        }
    } // namespace detail
} // namespace maui::devices
