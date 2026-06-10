#pragma once
// maui::devices::device_info      <=  Microsoft.Maui.Devices.DeviceInfo (static facade)
// maui::devices::i_device_info    <=  Microsoft.Maui.Devices.IDeviceInfo
// maui::devices::device_type      <=  Microsoft.Maui.Devices.DeviceType
// maui::devices::device_platform  <=  Microsoft.Maui.Devices.DevicePlatform (Types/)
// maui::devices::device_idiom     <=  Microsoft.Maui.Devices.DeviceIdiom (Types/)
// maui::devices::version_info     <=  System.Version (the ParseVersion slice IDeviceInfo.Version uses)
//
// The static C# class becomes a deleted-ctor facade over an injectable implementation seam:
// device_info::current() lazily creates the backend implementation (DeviceInfoImplementation, one
// per platform partial - the C# partial-class split), and set_current() is the C# internal
// SetCurrent test seam made public (the port has no InternalsVisibleTo). device_platform /
// device_idiom keep C#'s string-backed extensible value-type design (Create() admits custom
// values); the underlying strings are the C# names so to_string round-trips identically.

#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace maui::devices
{
    // Types of devices (DeviceInfo.shared.cs).
    enum class device_type
    {
        unknown = 0,
        physical = 1,
        virtual_ = 2, // C# DeviceType.Virtual ("virtual" is a C++ keyword; port-wide trailing-underscore rule)
    };

    // The device platform, a string-backed extensible value type. Factory names are the snake_case
    // transforms of the C# statics; the stored strings stay the C# originals.
    class device_platform
    {
    public:
        // A default instance mirrors C#'s default struct (null string): empty name, equal only to
        // another default instance.
        device_platform() = default;

        [[nodiscard]] static device_platform android()
        {
            return device_platform("Android");
        }
        [[nodiscard]] static device_platform ios()
        {
            return device_platform("iOS");
        }
        [[nodiscard]] static device_platform mac_os() // C# DevicePlatform.macOS (distinct from MacCatalyst)
        {
            return device_platform("macOS");
        }
        [[nodiscard]] static device_platform mac_catalyst()
        {
            return device_platform("MacCatalyst");
        }
        [[nodiscard]] static device_platform tv_os()
        {
            return device_platform("tvOS");
        }
        [[nodiscard]] static device_platform tizen()
        {
            return device_platform("Tizen");
        }
        [[nodiscard]] static device_platform win_ui()
        {
            return device_platform("WinUI");
        }
        [[nodiscard]] static device_platform watch_os()
        {
            return device_platform("watchOS");
        }
        [[nodiscard]] static device_platform unknown()
        {
            return device_platform("Unknown");
        }

        // DevicePlatform.Create: define a custom platform. Throws std::invalid_argument when empty
        // (C# throws ArgumentException; ArgumentNullException cannot arise - string_view is a value).
        [[nodiscard]] static device_platform create(std::string_view name);

        // ToString(): the platform identifier ("" for a default instance).
        [[nodiscard]] std::string_view to_string() const
        {
            return value_;
        }

        friend bool operator==(const device_platform& a, const device_platform& b) = default;

    private:
        explicit device_platform(std::string value) : value_(std::move(value))
        {
        }
        std::string value_;
    };

    // The device idiom (form factor), same string-backed design as device_platform.
    class device_idiom
    {
    public:
        device_idiom() = default;

        [[nodiscard]] static device_idiom phone()
        {
            return device_idiom("Phone");
        }
        [[nodiscard]] static device_idiom tablet()
        {
            return device_idiom("Tablet");
        }
        [[nodiscard]] static device_idiom desktop()
        {
            return device_idiom("Desktop");
        }
        [[nodiscard]] static device_idiom tv()
        {
            return device_idiom("TV");
        }
        [[nodiscard]] static device_idiom watch()
        {
            return device_idiom("Watch");
        }
        [[nodiscard]] static device_idiom unknown()
        {
            return device_idiom("Unknown");
        }

        // DeviceIdiom.Create (throws std::invalid_argument when empty, like DevicePlatform.Create).
        [[nodiscard]] static device_idiom create(std::string_view name);

        [[nodiscard]] std::string_view to_string() const
        {
            return value_;
        }

        friend bool operator==(const device_idiom& a, const device_idiom& b) = default;

    private:
        explicit device_idiom(std::string value) : value_(std::move(value))
        {
        }
        std::string value_;
    };

    // The System.Version slice IDeviceInfo.Version exposes (major.minor[.build[.revision]]);
    // parse() ports Utils.ParseVersion (Types/Shared/Utils.shared.cs): full Version.TryParse first
    // (two to four dot-separated non-negative ints), then a bare major int -> {major, 0}, else {0, 0}.
    struct version_info
    {
        int major = 0;
        int minor = 0;
        int build = -1;    // -1 = undefined, like System.Version
        int revision = -1; // -1 = undefined

        friend bool operator==(const version_info& a, const version_info& b) = default;

        [[nodiscard]] static version_info parse(std::string_view text);

        // System.Version.ToString(): "major.minor[.build[.revision]]" (undefined parts omitted).
        [[nodiscard]] std::string to_string() const;
    };

    // Information about the device the app runs on.
    class i_device_info
    {
    public:
        virtual ~i_device_info() = default;

        [[nodiscard]] virtual std::string model() const = 0;
        [[nodiscard]] virtual std::string manufacturer() const = 0;
        // Often user-assigned (the computer/device name).
        [[nodiscard]] virtual std::string name() const = 0;
        [[nodiscard]] virtual std::string version_string() const = 0;
        // IDeviceInfo.Version - every platform partial implements it as ParseVersion(VersionString).
        [[nodiscard]] virtual version_info version() const
        {
            return version_info::parse(version_string());
        }
        [[nodiscard]] virtual device_platform platform() const = 0;
        [[nodiscard]] virtual device_idiom idiom() const = 0;
        [[nodiscard]] virtual enum device_type device_type() const = 0;

    protected:
        i_device_info() = default;
        i_device_info(const i_device_info&) = default;
        i_device_info(i_device_info&&) = default;
        i_device_info& operator=(const i_device_info&) = default;
        i_device_info& operator=(i_device_info&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (DeviceInfoImplementation): defined once per backend under
        // src/platform/<backend>/essentials_device_info.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_device_info> make_device_info();
    } // namespace detail

    // The static facade. Statics forward to current(), exactly like the C# static class.
    class device_info final
    {
    public:
        device_info() = delete;

        [[nodiscard]] static std::string model()
        {
            return current().model();
        }
        [[nodiscard]] static std::string manufacturer()
        {
            return current().manufacturer();
        }
        [[nodiscard]] static std::string name()
        {
            return current().name();
        }
        [[nodiscard]] static std::string version_string()
        {
            return current().version_string();
        }
        [[nodiscard]] static version_info version()
        {
            return current().version();
        }
        [[nodiscard]] static device_platform platform()
        {
            return current().platform();
        }
        [[nodiscard]] static device_idiom idiom()
        {
            return current().idiom();
        }
        [[nodiscard]] static enum device_type device_type()
        {
            return current().device_type();
        }

        // DeviceInfo.Current: lazily creates the platform implementation on first use.
        [[nodiscard]] static i_device_info& current();

        // DeviceInfo.SetCurrent (internal in C#; the public test seam here). nullptr resets to the
        // lazy platform default.
        static void set_current(std::shared_ptr<i_device_info> implementation);
    };
} // namespace maui::devices
