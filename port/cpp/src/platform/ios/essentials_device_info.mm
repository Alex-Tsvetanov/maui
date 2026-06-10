// device_info - iOS (UIKit) platform partial. Ported 1:1 from DeviceInfo.ios.tvos.watchos.cs:
// Model prefers the sysctl "hw.machine" hardware identifier (PlatformUtils.GetSystemLibraryProperty)
// and falls back to UIDevice.model; Name/VersionString come from UIDevice.currentDevice; the idiom
// maps UIUserInterfaceIdiom; DeviceType is Physical on hardware and Virtual on the simulator
// (Runtime.Arch == DEVICE becomes the TARGET_OS_SIMULATOR compile-time fact).

#include <TargetConditionals.h>
#import <UIKit/UIKit.h>
#include <sys/sysctl.h>

#include <memory>
#include <string>

#include "maui/essentials/device_info.hpp"

namespace maui::devices
{
    namespace
    {
        // PlatformUtils.GetSystemLibraryProperty("hw.machine") - empty when unavailable.
        std::string system_library_property(const char* property)
        {
            std::size_t length = 0;
            if (sysctlbyname(property, nullptr, &length, nullptr, 0) != 0 || length == 0)
            {
                return {};
            }
            std::string value(length, '\0');
            if (sysctlbyname(property, value.data(), &length, nullptr, 0) != 0)
            {
                return {};
            }
            while (!value.empty() && value.back() == '\0')
            {
                value.pop_back();
            }
            return value;
        }

        std::string to_std_string(NSString* value)
        {
            const char* const utf8 = [value UTF8String]; // messaging nil yields nullptr
            return utf8 != nullptr ? std::string(utf8) : std::string();
        }

        class ios_device_info final : public i_device_info
        {
        public:
            [[nodiscard]] std::string model() const override
            {
                std::string machine = system_library_property("hw.machine");
                if (!machine.empty())
                {
                    return machine;
                }
                // "Unable to query hardware model, returning current device model."
                return to_std_string([UIDevice currentDevice].model);
            }

            [[nodiscard]] std::string manufacturer() const override
            {
                return "Apple";
            }

            [[nodiscard]] std::string name() const override
            {
                return to_std_string([UIDevice currentDevice].name);
            }

            [[nodiscard]] std::string version_string() const override
            {
                return to_std_string([UIDevice currentDevice].systemVersion);
            }

            [[nodiscard]] device_platform platform() const override
            {
                return device_platform::ios();
            }

            [[nodiscard]] device_idiom idiom() const override
            {
                switch ([UIDevice currentDevice].userInterfaceIdiom)
                {
                    case UIUserInterfaceIdiomPad:
                        return device_idiom::tablet();
                    case UIUserInterfaceIdiomPhone:
                        return device_idiom::phone();
                    case UIUserInterfaceIdiomTV:
                        return device_idiom::tv();
                    default:
                        return device_idiom::unknown();
                }
            }

            [[nodiscard]] enum device_type device_type() const override
            {
#if TARGET_OS_SIMULATOR
                return device_type::virtual_;
#else
                return device_type::physical;
#endif
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_device_info> make_device_info()
        {
            return std::make_shared<ios_device_info>();
        }
    } // namespace detail
} // namespace maui::devices
