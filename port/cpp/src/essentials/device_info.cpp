// The cross-platform half of the device_info facade (DeviceInfo.Current / DeviceInfo.SetCurrent)
// plus the value-type logic the header declares: DevicePlatform.Create / DeviceIdiom.Create and
// Utils.ParseVersion (the System.Version slice).

#include "maui/essentials/device_info.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

// TODO(essentials/xaml integration): once include/maui/xaml/xaml_runtime_environment.hpp lands
// (the OnPlatform/OnIdiom runtime environment), device_info::current() should seed it via
// maui::xaml::xaml_runtime_environment::set_current(...) so XAML platform conditionals resolve
// from the real device info. The header does not exist on this tree yet - wire it when it does.

namespace maui::devices
{
    namespace
    {
        // Parse one dot-separated non-negative component; false on any non-digit/overflow.
        bool parse_component(std::string_view text, int& value)
        {
            constexpr std::size_t max_int32_digits = 10; // int.MaxValue is 10 digits
            if (text.empty() || text.size() > max_int32_digits)
            {
                return false;
            }
            std::int64_t parsed = 0;
            for (const char digit : text)
            {
                if (digit < '0' || digit > '9')
                {
                    return false;
                }
                parsed = (parsed * 10) + (digit - '0');
            }
            if (parsed > std::numeric_limits<int>::max())
            {
                return false;
            }
            value = static_cast<int>(parsed);
            return true;
        }

        std::shared_ptr<i_device_info>& device_info_storage()
        {
            static std::shared_ptr<i_device_info> storage;
            return storage;
        }
    } // namespace

    device_platform device_platform::create(std::string_view name)
    {
        if (name.empty())
        {
            // C# throws ArgumentNullException/ArgumentException; string_view folds both to empty.
            throw std::invalid_argument("device_platform: the platform identifier must be non-empty");
        }
        return device_platform(std::string(name));
    }

    device_idiom device_idiom::create(std::string_view name)
    {
        if (name.empty())
        {
            throw std::invalid_argument("device_idiom: the idiom name must be non-empty");
        }
        return device_idiom(std::string(name));
    }

    version_info version_info::parse(std::string_view text)
    {
        // Utils.ParseVersion: Version.TryParse demands 2-4 dot-separated non-negative ints;
        // a bare int becomes {major, 0}; anything else {0, 0}.
        std::array<int, 4> parts{0, 0, -1, -1};
        std::size_t count = 0;
        std::string_view rest = text;
        bool ok = !text.empty();
        while (ok && !rest.empty())
        {
            if (count == parts.size())
            {
                ok = false;
                break;
            }
            const auto dot = rest.find('.');
            const std::string_view component = rest.substr(0, dot);
            ok = parse_component(component, parts.at(count));
            ++count;
            if (dot == std::string_view::npos)
            {
                break;
            }
            rest = rest.substr(dot + 1);
            if (rest.empty())
            {
                ok = false; // trailing dot
            }
        }
        if (ok && count >= 2)
        {
            return {.major = parts[0], .minor = parts[1], .build = parts[2], .revision = parts[3]};
        }
        if (ok && count == 1)
        {
            return {.major = parts[0], .minor = 0, .build = -1, .revision = -1}; // new Version(major, 0)
        }
        return {.major = 0, .minor = 0, .build = -1, .revision = -1}; // new Version(0, 0)
    }

    std::string version_info::to_string() const
    {
        std::string result = std::to_string(major) + "." + std::to_string(minor);
        if (build >= 0)
        {
            result += "." + std::to_string(build);
            if (revision >= 0)
            {
                result += "." + std::to_string(revision);
            }
        }
        return result;
    }

    i_device_info& device_info::current()
    {
        auto& storage = device_info_storage();
        if (storage == nullptr)
        {
            storage = detail::make_device_info();
        }
        return *storage;
    }

    void device_info::set_current(std::shared_ptr<i_device_info> implementation)
    {
        device_info_storage() = std::move(implementation);
    }
} // namespace maui::devices
