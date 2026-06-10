// device_info - Apple (AppKit / macOS) platform partial. Ported 1:1 from DeviceInfo.macos.cs:
// Model reads the IOPlatformExpertDevice registry entry's "model" property (IOKit), Manufacturer
// is "Apple", Name is the user-assigned computer name (SCDynamicStoreCopyComputerName), the OS
// version comes from NSProcessInfo.operatingSystemVersion, and platform/idiom/device-type are
// fixed (macOS / Desktop / Physical). Compiled as Objective-C++ with ARC for the apple backend.

#import <Foundation/Foundation.h>
#include <IOKit/IOKitLib.h>
#include <SystemConfiguration/SystemConfiguration.h>

#include <cstddef>
#include <memory>
#include <span>
#include <string>

#include "maui/essentials/device_info.hpp"

namespace maui::devices
{
    namespace
    {
        // IOKit.GetPlatformExpertPropertyValue<NSData>("model") - the hardware model identifier
        // bytes (e.g. "Mac15,6\0"), trimmed of trailing NULs; empty when unavailable.
        std::string platform_expert_model()
        {
            const io_service_t service =
                IOServiceGetMatchingService(kIOMainPortDefault, IOServiceMatching("IOPlatformExpertDevice"));
            if (service == IO_OBJECT_NULL)
            {
                return {};
            }
            std::string result;
            const CFTypeRef property = IORegistryEntryCreateCFProperty(service, CFSTR("model"), kCFAllocatorDefault, 0);
            if (property != nullptr)
            {
                if (CFGetTypeID(property) == CFDataGetTypeID())
                {
                    const auto* const data = static_cast<CFDataRef>(property);
                    const std::span<const UInt8> bytes{CFDataGetBytePtr(data),
                                                       static_cast<std::size_t>(CFDataGetLength(data))};
                    result.assign(bytes.begin(), bytes.end());
                    while (!result.empty() && result.back() == '\0')
                    {
                        result.pop_back();
                    }
                }
                CFRelease(property);
            }
            IOObjectRelease(service);
            return result;
        }

        class apple_device_info final : public i_device_info
        {
        public:
            [[nodiscard]] std::string model() const override
            {
                return platform_expert_model();
            }

            [[nodiscard]] std::string manufacturer() const override
            {
                return "Apple";
            }

            // SCDynamicStoreCopyComputerName - C# returns null when the handle is zero (-> "").
            [[nodiscard]] std::string name() const override
            {
                const CFStringRef computer_name = SCDynamicStoreCopyComputerName(nullptr, nullptr);
                if (computer_name == nullptr)
                {
                    return {};
                }
                NSString* const bridged = CFBridgingRelease(computer_name);
                const char* const utf8 = [bridged UTF8String];
                return utf8 != nullptr ? std::string(utf8) : std::string();
            }

            // NSProcessInfo.OperatingSystemVersion.ToString() => "Major.Minor.PatchVersion".
            [[nodiscard]] std::string version_string() const override
            {
                const NSOperatingSystemVersion version = [[NSProcessInfo processInfo] operatingSystemVersion];
                return std::to_string(version.majorVersion) + "." + std::to_string(version.minorVersion) + "." +
                       std::to_string(version.patchVersion);
            }

            [[nodiscard]] device_platform platform() const override
            {
                return device_platform::mac_os();
            }

            [[nodiscard]] device_idiom idiom() const override
            {
                return device_idiom::desktop();
            }

            [[nodiscard]] enum device_type device_type() const override
            {
                return device_type::physical;
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_device_info> make_device_info()
        {
            return std::make_shared<apple_device_info>();
        }
    } // namespace detail
} // namespace maui::devices
