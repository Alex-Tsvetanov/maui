// device_info - Windows (WinUI 3 / C++/WinRT) platform partial. Ported from
// src/Essentials/src/DeviceInfo/DeviceInfo.windows.cs:
//   Model         => EasClientDeviceInformation.SystemProductName
//   Manufacturer  => EasClientDeviceInformation.SystemManufacturer
//   Name          => EasClientDeviceInformation.FriendlyName
//   VersionString => AnalyticsInfo.VersionInfo.DeviceFamilyVersion: a ulong packed as four 16-bit
//                    fields (high to low), unpacked into "major.minor.build.revision"; falls back to
//                    the raw string verbatim when it isn't a plain non-negative integer (mirrors the
//                    C# ulong.TryParse failure path).
//   Platform      => DevicePlatform.WinUI (fixed)
//   Idiom         => AnalyticsInfo.VersionInfo.DeviceFamily switch; Windows.Universal/.Desktop is
//                    further split by GetIsInTabletMode() (Chromium's win_util.cc tablet-mode
//                    heuristic, ported line-for-line below).
//   DeviceType    => SystemProductName contains "Virtual" or equals "HMV domU" => Virtual, else
//                    Physical.
//
// DEVIATION from the oracle: DeviceInfoImplementation's C# getters let WinRT/Win32 failures escape
// (only the ctor's cached systemProductName read is try/caught). Here every accessor instead catches
// winrt::hresult_error and degrades to an empty/Unknown default. Reason: an unpackaged dev build (no
// MSIX identity, which is how this port's CMake-built gallery runs today) can make
// EasClientDeviceInformation/AnalyticsInfo throw, and letting that escape through the handler seam is
// exactly the stowed-exception fail-fast this partial was written to fix (see the diagnosis in
// device_page's launch crash: the headless fake's version_string() throwing not_implemented took down
// the whole app). "The page renders instead of crashing" beats byte-for-byte exception fidelity here.
//
// NO PRECEDENT ELSEWHERE IN THIS TREE - checked: the android and apple partials do not catch. This is
// a Windows-specific judgement, and it rests entirely on WHERE the throw lands: on every other backend
// an escaping exception unwinds normally, but here it crosses XAML's Application::Start callback, which
// WinRT converts to a stowed exception and FAIL-FASTS - no catch upstream can see it. If a future reader
// finds these accessors succeeding unconditionally, the catches are inert and can go.
//
// Also: DeviceType is recomputed from a fresh model() read on every call instead of caching a
// systemProductName field + a currentType memo (the C# perf detail) - the underlying WinRT calls are
// idempotent for the process lifetime, so this is a size-of-code simplification, not a behavior change.
//
// GetAutoRotationState / PowerDeterminePlatformRoleEx are classic Win32 exports the C++/WinRT
// projection headers don't declare, and DeviceInfo.windows.cs itself reaches them by hand via
// [DllImport] (user32.dll / Powrprof.dll) rather than a projected API. This file does the same thing
// C++'s way: extern "C" prototypes matching the DllImport signatures exactly, rather than depending on
// whichever SDK header (if any) a given MSVC install exposes them through under what _WIN32_WINNT
// guard - see the AR_*/SM_*/PowerPlatformRole constants below, copied verbatim from the C# oracle's own
// literals rather than re-derived from a system header that might name or value them differently.

#include "maui/essentials/device_info.hpp"

#include <winrt/Windows.Security.ExchangeActiveSyncProvisioning.h>
#include <winrt/Windows.System.Profile.h>

#include <charconv>
#include <memory>
#include <string>

#include "winui_interop.hpp"

extern "C"
{
    __declspec(dllimport) int __stdcall GetSystemMetrics(int index);
    __declspec(dllimport) int __stdcall GetAutoRotationState(int* state);
    __declspec(dllimport) int __stdcall PowerDeterminePlatformRoleEx(unsigned long long version);
}
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Powrprof.lib")

namespace maui::devices
{
    namespace
    {
        namespace eas = winrt::Windows::Security::ExchangeActiveSyncProvisioning;
        namespace profile = winrt::Windows::System::Profile;
        using maui::platform::windows::to_utf8;

        // DeviceInfo.windows.cs's SM_*/AR_*/PowerPlatformRole literals, copied verbatim (see file
        // header for why these aren't pulled from a system header).
        constexpr int sm_convertible_slate_mode = 0x2003;
        constexpr int sm_maximum_touches = 95;
        constexpr int sm_is_docked = 0x2004;
        constexpr int ar_enabled = 0x0;
        constexpr int ar_not_supported = 0x20;
        constexpr int ar_laptop = 0x80;
        constexpr int ar_nosensor = 0x10;
        constexpr int power_role_mobile = 2;
        constexpr int power_role_slate = 8;

        // GetIsInTabletMode(): Chromium's win_util.cc heuristic (see the oracle's own citing comment),
        // ported line-for-line.
        bool get_is_in_tablet_mode()
        {
            // Device does not have a touchscreen.
            if (GetSystemMetrics(sm_maximum_touches) == 0)
            {
                return false;
            }
            // If the device is docked, the user is treating it as a PC.
            if (GetSystemMetrics(sm_is_docked) != 0)
            {
                return false;
            }
            // Fetch device rotation; possible for this to fail.
            int rotation_state = ar_enabled;
            const bool success = GetAutoRotationState(&rotation_state) != 0;
            // Fetch succeeded and device does not support rotation.
            if (success && (rotation_state & (ar_not_supported | ar_laptop | ar_nosensor)) != 0)
            {
                return false;
            }
            // Power management says we are mobile (laptop) or a tablet.
            if ((PowerDeterminePlatformRoleEx(2) & (power_role_mobile | power_role_slate)) != 0)
            {
                // Tablet mode is 0 (the default value).
                return GetSystemMetrics(sm_convertible_slate_mode) == 0;
            }
            return false;
        }

        // Runs `read` and UTF-8-converts its hstring result; "" on any WinRT failure (see file header
        // DEVIATION note).
        template <class Read> std::string try_read(Read&& read)
        {
            try
            {
                return to_utf8(read());
            }
            catch (const winrt::hresult_error&)
            {
                return {};
            }
        }

        class windows_device_info final : public i_device_info
        {
        public:
            [[nodiscard]] std::string model() const override
            {
                return try_read([] { return eas::EasClientDeviceInformation().SystemProductName(); });
            }

            [[nodiscard]] std::string manufacturer() const override
            {
                return try_read([] { return eas::EasClientDeviceInformation().SystemManufacturer(); });
            }

            [[nodiscard]] std::string name() const override
            {
                return try_read([] { return eas::EasClientDeviceInformation().FriendlyName(); });
            }

            // ulong.TryParse: whole-string, base-10, non-negative only - std::from_chars over the full
            // span mirrors that (a partial parse, e.g. trailing junk, is a TryParse failure too).
            [[nodiscard]] std::string version_string() const override
            {
                const std::string raw =
                    try_read([] { return profile::AnalyticsInfo::VersionInfo().DeviceFamilyVersion(); });
                unsigned long long packed = 0;
                const char* const begin = raw.data();
                const char* const end = raw.data() + raw.size();
                const auto [ptr, ec] = std::from_chars(begin, end, packed);
                if (ec != std::errc{} || ptr != end)
                {
                    return raw;
                }
                const unsigned long long v1 = (packed & 0xFFFF000000000000ULL) >> 48;
                const unsigned long long v2 = (packed & 0x0000FFFF00000000ULL) >> 32;
                const unsigned long long v3 = (packed & 0x00000000FFFF0000ULL) >> 16;
                const unsigned long long v4 = packed & 0x000000000000FFFFULL;
                return std::to_string(v1) + "." + std::to_string(v2) + "." + std::to_string(v3) + "." +
                       std::to_string(v4);
            }

            [[nodiscard]] device_platform platform() const override
            {
                return device_platform::win_ui();
            }

            [[nodiscard]] device_idiom idiom() const override
            {
                const std::string family =
                    try_read([] { return profile::AnalyticsInfo::VersionInfo().DeviceFamily(); });
                if (family == "Windows.Mobile")
                {
                    return device_idiom::phone();
                }
                if (family == "Windows.Universal" || family == "Windows.Desktop")
                {
                    return get_is_in_tablet_mode() ? device_idiom::tablet() : device_idiom::desktop();
                }
                if (family == "Windows.Xbox" || family == "Windows.Team")
                {
                    return device_idiom::tv();
                }
                // "Windows.IoT" and anything unrecognized (including a read failure -> "").
                return device_idiom::unknown();
            }

            [[nodiscard]] enum device_type device_type() const override
            {
                const std::string product = model();
                const bool is_virtual = product.find("Virtual") != std::string::npos || product == "HMV domU";
                return is_virtual ? device_type::virtual_ : device_type::physical;
            }
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
