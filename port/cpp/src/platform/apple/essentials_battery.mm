// battery - Apple (AppKit / macOS) platform partial. Ported 1:1 from Battery.macos.cs and the
// IOKit helpers it calls (Platform/PlatformUtils.macos.cs): charge level aggregates Current/Max
// Capacity over the present internal batteries (1.0 when none), state walks the power-source
// descriptions (Charging > Full > Discharging-vs-NotCharging via the providing source), the power
// source maps "Battery Power" -> battery and "AC Power"/"UPS Power" -> ac, the battery listeners
// are an IOPSNotificationCreateRunLoopSource on the main run loop (raises marshalled to the main
// queue), and the energy saver is fixed Off with no listeners (macOS has no low-power notion in
// the C# partial).

#import <Foundation/Foundation.h>
#include <IOKit/ps/IOPSKeys.h>
#include <IOKit/ps/IOPowerSources.h>

#include <atomic>
#include <memory>

#include "maui/essentials/battery.hpp"

#include "src/essentials/detail/battery_base.hpp"

namespace maui::devices
{
    namespace
    {
        bool dictionary_bool(NSDictionary* dictionary, NSString* key, bool* present = nullptr)
        {
            NSNumber* const value = dictionary[key];
            if (present != nullptr)
            {
                *present = value != nil;
            }
            return value != nil && [value boolValue];
        }

        // IOKit.GetInternalBatteryState.
        battery_state internal_battery_state()
        {
            const CFTypeRef info = IOPSCopyPowerSourcesInfo();
            const CFArrayRef sources = IOPSCopyPowerSourcesList(info);

            bool has_battery = false;
            bool fully_charged = true;
            battery_state result = battery_state::unknown;
            bool resolved = false;

            const CFIndex count = sources != nullptr ? CFArrayGetCount(sources) : 0;
            for (CFIndex i = 0; i < count && !resolved; ++i)
            {
                auto* const dic =
                    (__bridge NSDictionary*)IOPSGetPowerSourceDescription(info, CFArrayGetValueAtIndex(sources, i));
                if (dic == nil)
                {
                    continue;
                }
                NSString* const type = dic[@kIOPSTypeKey];
                // We only care about present internal batteries.
                if (![type isEqualToString:@kIOPSInternalBatteryType] || !dictionary_bool(dic, @kIOPSIsPresentKey))
                {
                    continue;
                }
                has_battery = true;

                // If any battery is charging, we are charging.
                if (dictionary_bool(dic, @kIOPSIsChargingKey))
                {
                    result = battery_state::charging;
                    resolved = true;
                    break;
                }

                // If any is not [almost] fully charged, we are not full. (The second clause ports
                // C#'s quirky `!TryGet(finishing) && finishing != true`, which reduces to
                // "the Is Finishing Charge key is absent".)
                bool has_charged = false;
                bool has_finishing = false;
                const bool charged = dictionary_bool(dic, @kIOPSIsChargedKey, &has_charged);
                dictionary_bool(dic, @kIOPSIsFinishingChargeKey, &has_finishing);
                if (!(has_charged && charged) || !has_finishing)
                {
                    fully_charged = false;
                }
            }

            if (!resolved)
            {
                if (!has_battery)
                {
                    result = battery_state::not_present;
                }
                else if (fully_charged)
                {
                    result = battery_state::full;
                }
                else
                {
                    // We weren't able to work out what was happening, so try and guess.
                    auto* const providing = (__bridge NSString*)IOPSGetProvidingPowerSourceType(info);
                    result = [providing isEqualToString:@kIOPMBatteryPowerKey] ? battery_state::discharging
                                                                               : battery_state::not_charging;
                }
            }

            if (sources != nullptr)
            {
                CFRelease(sources);
            }
            if (info != nullptr)
            {
                CFRelease(info);
            }
            return result;
        }

        // IOKit.GetInternalBatteryChargeLevel.
        double internal_battery_charge_level()
        {
            const CFTypeRef info = IOPSCopyPowerSourcesInfo();
            const CFArrayRef sources = IOPSCopyPowerSourcesList(info);

            double total_current = 0;
            double total_max = 0;

            const CFIndex count = sources != nullptr ? CFArrayGetCount(sources) : 0;
            for (CFIndex i = 0; i < count; ++i)
            {
                auto* const dic =
                    (__bridge NSDictionary*)IOPSGetPowerSourceDescription(info, CFArrayGetValueAtIndex(sources, i));
                if (dic == nil)
                {
                    continue;
                }
                NSString* const type = dic[@kIOPSTypeKey];
                NSNumber* const current = dic[@kIOPSCurrentCapacityKey];
                NSNumber* const max = dic[@kIOPSMaxCapacityKey];
                // Only present internal batteries that report capacity.
                if ([type isEqualToString:@kIOPSInternalBatteryType] && dictionary_bool(dic, @kIOPSIsPresentKey) &&
                    current != nil && [current intValue] > 0 && max != nil && [max intValue] > 0)
                {
                    total_current += [current intValue];
                    total_max += [max intValue];
                }
            }

            if (sources != nullptr)
            {
                CFRelease(sources);
            }
            if (info != nullptr)
            {
                CFRelease(info);
            }

            // Something went wrong, or there is no battery.
            if (total_max <= 0)
            {
                return 1.0;
            }
            return total_current / total_max;
        }

        // IOKit.GetProvidingPowerSource.
        battery_power_source providing_power_source()
        {
            const CFTypeRef info = IOPSCopyPowerSourcesInfo();
            auto* const providing = (__bridge NSString*)IOPSGetProvidingPowerSourceType(info);
            battery_power_source result = battery_power_source::unknown;
            if ([providing isEqualToString:@kIOPMBatteryPowerKey])
            {
                result = battery_power_source::battery;
            }
            else if ([providing isEqualToString:@kIOPMACPowerKey] || [providing isEqualToString:@kIOPMUPSPowerKey])
            {
                result = battery_power_source::ac;
            }
            if (info != nullptr)
            {
                CFRelease(info);
            }
            return result;
        }

        class apple_battery final : public detail::battery_base
        {
        public:
            ~apple_battery() override
            {
                *alive_ = false; // gates the main-queue raise blocks already dispatched (see below)
                platform_stop_battery_listeners();
            }
            apple_battery() = default;
            apple_battery(const apple_battery&) = delete;
            apple_battery(apple_battery&&) = delete;
            apple_battery& operator=(const apple_battery&) = delete;
            apple_battery& operator=(apple_battery&&) = delete;

            [[nodiscard]] double charge_level() const override
            {
                return internal_battery_charge_level();
            }

            [[nodiscard]] battery_state state() const override
            {
                return internal_battery_state();
            }

            [[nodiscard]] battery_power_source power_source() const override
            {
                return providing_power_source();
            }

            [[nodiscard]] enum energy_saver_status energy_saver_status() const override
            {
                return energy_saver_status::off; // Battery.macos.cs: always Off
            }

        protected:
            void platform_start_battery_listeners() override
            {
                run_loop_source_ = IOPSNotificationCreateRunLoopSource(&apple_battery::power_source_notification, this);
                if (run_loop_source_ != nullptr)
                {
                    CFRunLoopAddSource(CFRunLoopGetMain(), run_loop_source_, kCFRunLoopDefaultMode);
                }
            }

            void platform_stop_battery_listeners() override
            {
                if (run_loop_source_ != nullptr)
                {
                    CFRunLoopRemoveSource(CFRunLoopGetMain(), run_loop_source_, kCFRunLoopDefaultMode);
                    CFRelease(run_loop_source_);
                    run_loop_source_ = nullptr;
                }
            }

            void platform_start_energy_saver_listeners() override
            {
                // Battery.macos.cs: no energy-saver listeners on macOS.
            }

            void platform_stop_energy_saver_listeners() override
            {
            }

        private:
            // PowerSourceNotification => MainThread.BeginInvokeOnMainThread(OnBatteryInfoChanged).
            // The dispatched block holds the shared alive flag, not the implementation: a block
            // already queued when the implementation is destroyed (set_default swap) must not
            // touch the dead object - it re-checks the flag on the main queue instead.
            static void power_source_notification(void* context)
            {
                auto* const self = static_cast<apple_battery*>(context);
                const std::shared_ptr<std::atomic<bool>> alive = self->alive_;
                dispatch_async(dispatch_get_main_queue(), ^{
                  if (*alive)
                  {
                      self->on_battery_info_changed();
                  }
                });
            }

            std::shared_ptr<std::atomic<bool>> alive_ = std::make_shared<std::atomic<bool>>(true);
            CFRunLoopSourceRef run_loop_source_ = nullptr;
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_battery> make_battery()
        {
            return std::make_shared<apple_battery>();
        }
    } // namespace detail
} // namespace maui::devices
