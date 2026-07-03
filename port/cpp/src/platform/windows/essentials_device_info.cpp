// device_info - Windows platform partial. Ported from DeviceInfo.windows.cs
// (src/Essentials/src/DeviceInfo/DeviceInfo.windows.cs):
//   Model         => EasClientDeviceInformation.SystemProductName
//   Manufacturer  => EasClientDeviceInformation.SystemManufacturer
//   Name          => EasClientDeviceInformation.FriendlyName
//   VersionString => AnalyticsInfo.VersionInfo.DeviceFamilyVersion (a packed ulong; unpacked into
//                    "v1.v2.v3.v4" by 16-bit shifts, else the raw string when it does not parse)
//   Platform      => DevicePlatform.WinUI (fixed)
//   Idiom         => AnalyticsInfo.VersionInfo.DeviceFamily: "Windows.Mobile" => Phone;
//                    "Windows.Universal"/"Windows.Desktop" => tablet-mode ? Tablet : Desktop;
//                    "Windows.Xbox"/"Windows.Team" => TV; "Windows.IoT"/other => Unknown
//   DeviceType    => SystemProductName contains "Virtual" (Ordinal) or == "HMV domU" => Virtual,
//                    else Physical (cached after the first successful read, like the C# field)
//
// The tablet-mode probe (GetIsInTabletMode) is the C# partial's Chromium-derived heuristic over the
// same three Win32 seams its DllImports name: GetSystemMetrics(SM_MAXIMUMTOUCHES/SM_ISDOCKED/
// SM_CONVERTIBLESLATEMODE) + GetAutoRotationState (user32) and PowerDeterminePlatformRoleEx
// (powrprof) — including C#'s flags-style AND over the POWER_PLATFORM_ROLE return.
//
// Like the android partial (and unlike the unconfigured headless fake, whose model()/
// version_string() THROW feature_not_supported and so terminated the DevicePage on boot), every
// WinRT read here degrades to a sane default on failure — a rendering page beats a crash; the page
// must never throw during construction. The one windows-specific wrinkle: WinRT activation needs a
// COM apartment, which the gallery's run_app initializes but a bare unit-test host does not —
// ensure_apartment() joins/creates the MTA once (S_OK/S_FALSE/RPC_E_CHANGED_MODE all mean "an
// apartment exists"; the init is deliberately process-lifetime, matching the facade's static
// storage), so the reads stay REAL in the XAML-less suite instead of degrading.

#define NOMINMAX // windows.h's min/max macros break std::min/std::max in every header after it
#include <windows.h>

#include <powerbase.h> // PowerDeterminePlatformRoleEx (link: powrprof)

#include <charconv>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

#include <winrt/Windows.Security.ExchangeActiveSyncProvisioning.h>
#include <winrt/Windows.System.Profile.h>
#include <winrt/base.h>

#include "maui/essentials/device_info.hpp"

namespace maui::devices
{
    namespace
    {
        namespace wsp = winrt::Windows::System::Profile;
        namespace wse = winrt::Windows::Security::ExchangeActiveSyncProvisioning;

        // The C# partial's private constants (its user32 DllImport companions).
        constexpr int k_sm_convertible_slate_mode = 0x2003; // SM_CONVERTIBLESLATEMODE
        constexpr int k_sm_maximum_touches = 95;            // SM_MAXIMUMTOUCHES
        constexpr int k_sm_is_docked = 0x2004;              // SM_ISDOCKED

        // C#'s PowerPlatformRole enum slice (the two roles the tablet check ANDs against).
        constexpr int k_platform_role_mobile = 2; // PowerPlatformRole.PlatformRoleMobile
        constexpr int k_platform_role_slate = 8;  // PowerPlatformRole.PlatformRoleSlate

        // Join (or create) the process MTA so WinRT activation works even in a host that never
        // called winrt::init_apartment (the XAML-less unit suite). The S_OK init is INTENTIONALLY
        // never balanced — it keeps the apartment alive for the process-lifetime device_info facade
        // (header note); any TU calling CoUninitialize on this thread expecting balanced counts
        // would be pulling a ref this seam owns. Any OTHER failure HRESULT (E_OUTOFMEMORY, …) is
        // deliberately ignored: the reads that follow then fail activation and degrade to defaults
        // through their own catch (the page-boot doctrine).
        void ensure_apartment()
        {
            const HRESULT result = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);
            if (result == RPC_E_CHANGED_MODE)
            {
                // The thread already runs an STA (the gallery's XAML thread) — an apartment exists.
                return;
            }
            if (result == S_FALSE)
            {
                ::CoUninitialize(); // already initialized on this thread — drop the extra count
            }
        }

        // GetIsInTabletMode — the C# partial's Chromium-methodology probe, member for member.
        [[nodiscard]] bool is_in_tablet_mode()
        {
            // Device does not have a touchscreen.
            if (::GetSystemMetrics(k_sm_maximum_touches) == 0)
            {
                return false;
            }
            // If the device is docked, the user is treating it as a PC.
            if (::GetSystemMetrics(k_sm_is_docked) != 0)
            {
                return false;
            }
            // Fetch device rotation (possible for this to fail — only a successful read filters).
            AR_STATE rotation_state = AR_ENABLED;
            const BOOL success = ::GetAutoRotationState(&rotation_state);
            if (success != FALSE && (rotation_state & (AR_NOT_SUPPORTED | AR_LAPTOP | AR_NOSENSOR)) != 0)
            {
                return false;
            }
            // Check if power management says we are mobile (laptop) or a tablet. C# ANDs the enum
            // return against (Mobile | Slate) flags-style — mirrored bit for bit.
            const POWER_PLATFORM_ROLE role = ::PowerDeterminePlatformRoleEx(POWER_PLATFORM_ROLE_V2);
            if ((static_cast<int>(role) & (k_platform_role_mobile | k_platform_role_slate)) != 0)
            {
                // Tablet mode is 0 (0 is the default value).
                return ::GetSystemMetrics(k_sm_convertible_slate_mode) == 0;
            }
            return false;
        }

        class windows_device_info final : public i_device_info
        {
        public:
            // DeviceInfoImplementation(): construct the EasClientDeviceInformation and pre-read
            // SystemProductName under a swallow-all (the C# ctor's try/Debug.WriteLine).
            windows_device_info()
            {
                ensure_apartment();
                try
                {
                    eas_ = wse::EasClientDeviceInformation{};
                    system_product_name_ = winrt::to_string(eas_.SystemProductName());
                }
                catch (const winrt::hresult_error&)
                {
                    // Unable to get system product name — reads degrade to "" (header doctrine).
                }
            }

            [[nodiscard]] std::string model() const override
            {
                return eas_read([](const wse::EasClientDeviceInformation& eas) { return eas.SystemProductName(); });
            }

            [[nodiscard]] std::string manufacturer() const override
            {
                return eas_read([](const wse::EasClientDeviceInformation& eas) { return eas.SystemManufacturer(); });
            }

            [[nodiscard]] std::string name() const override
            {
                return eas_read([](const wse::EasClientDeviceInformation& eas) { return eas.FriendlyName(); });
            }

            // DeviceFamilyVersion is a decimal ulong packing four 16-bit parts; a non-parsing value
            // is returned raw (the C# TryParse fallthrough).
            [[nodiscard]] std::string version_string() const override
            {
                std::string text;
                try
                {
                    text = winrt::to_string(wsp::AnalyticsInfo::VersionInfo().DeviceFamilyVersion());
                }
                catch (const winrt::hresult_error&)
                {
                    return {};
                }
                std::uint64_t packed = 0;
                const auto* const begin = text.data();
                const auto* const end = begin + text.size();
                // from_chars rejects a leading '+' that ulong.TryParse would accept — moot for the
                // OS-controlled decimal DeviceFamilyVersion string, noted for pattern-copiers.
                const auto [parsed_to, error] = std::from_chars(begin, end, packed);
                if (error != std::errc{} || parsed_to != end || text.empty())
                {
                    return text; // ulong.TryParse failed — return the raw string
                }
                const std::uint64_t v1 = (packed & 0xFFFF000000000000ULL) >> 48U;
                const std::uint64_t v2 = (packed & 0x0000FFFF00000000ULL) >> 32U;
                const std::uint64_t v3 = (packed & 0x00000000FFFF0000ULL) >> 16U;
                const std::uint64_t v4 = packed & 0x000000000000FFFFULL;
                return std::to_string(v1) + "." + std::to_string(v2) + "." + std::to_string(v3) + "." +
                       std::to_string(v4);
            }

            [[nodiscard]] device_platform platform() const override
            {
                return device_platform::win_ui();
            }

            [[nodiscard]] device_idiom idiom() const override
            {
                std::string family;
                try
                {
                    family = winrt::to_string(wsp::AnalyticsInfo::VersionInfo().DeviceFamily());
                }
                catch (const winrt::hresult_error&)
                {
                    return device_idiom::unknown();
                }
                if (family == "Windows.Mobile")
                {
                    return device_idiom::phone();
                }
                if (family == "Windows.Universal" || family == "Windows.Desktop")
                {
                    return is_in_tablet_mode() ? device_idiom::tablet() : device_idiom::desktop();
                }
                if (family == "Windows.Xbox" || family == "Windows.Team")
                {
                    return device_idiom::tv();
                }
                return device_idiom::unknown(); // "Windows.IoT" and anything else
            }

            [[nodiscard]] enum device_type device_type() const override
            {
                if (current_type_ != device_type::unknown)
                {
                    return current_type_;
                }
                try
                {
                    if (system_product_name_.empty() && eas_ != nullptr)
                    {
                        system_product_name_ = winrt::to_string(eas_.SystemProductName());
                    }
                    const bool is_virtual = system_product_name_.find("Virtual") != std::string::npos ||
                                            system_product_name_ == "HMV domU";
                    current_type_ = is_virtual ? device_type::virtual_ : device_type::physical;
                }
                catch (const winrt::hresult_error&)
                {
                    // Unable to get device type — stays unknown; the next call retries (the C# shape).
                }
                return current_type_;
            }

        private:
            // Read one EasClientDeviceInformation string property, degrading to "" on any failure
            // (a failed activation left eas_ null; a read itself can also throw) — header doctrine.
            template <class Read> [[nodiscard]] std::string eas_read(Read&& read) const
            {
                if (eas_ == nullptr)
                {
                    return {};
                }
                try
                {
                    return winrt::to_string(read(eas_));
                }
                catch (const winrt::hresult_error&)
                {
                    return {};
                }
            }

            wse::EasClientDeviceInformation eas_{nullptr};
            // The C# fields mutated from property getters (Model caching / DeviceType memo) — the
            // const-correct port makes them mutable rather than de-const-ing i_device_info.
            // NOT thread-safe, mirroring the C# partial's lack of a lock; the facade is UI-thread
            // usage (a concurrent first-call could race the memo writes).
            mutable std::string system_product_name_;
            mutable enum device_type current_type_ = device_type::unknown;
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_device_info> make_device_info()
        {
            return std::make_shared<windows_device_info>();
        }
    } // namespace detail
} // namespace maui::devices
