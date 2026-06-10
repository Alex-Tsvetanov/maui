#pragma once
// maui::xaml::device_platform      <=  Microsoft.Maui.Devices.DevicePlatform
// maui::xaml::device_idiom         <=  Microsoft.Maui.Devices.DeviceIdiom
// maui::xaml::xaml_runtime_environment  <=  Microsoft.Maui.Devices.DeviceInfo (the Platform/Idiom
//   subset the XAML layer consumes: OnPlatformExtension/OnIdiomExtension read DeviceInfo.Platform /
//   DeviceInfo.Idiom; tests swap it via DeviceInfo.SetCurrent(new MockDeviceInfo())).
//
// C#'s DevicePlatform/DeviceIdiom are open string-valued structs (DevicePlatform.Create("GTK")); the
// markup extensions, however, only ever distinguish the FIXED set of named values their CLR
// properties spell (OnPlatformExtension.cs / OnIdiomExtension.cs), so the reflection-free port models
// exactly that closed set as enums. Notable C# quirk kept in mind by the consumers: DevicePlatform
// .UWP is literally the SAME platform value as WinUI (`UWP { get; } = new DevicePlatform(nameof
// (WinUI))`), so the port has one win_ui enumerator and OnPlatform's "UWP" attribute matches it.
// Documented deviation: the legacy DevicePlatform.Create("UWP") string (a distinct value C# also
// probes) has no closed-enum representation; tvOS/watchOS (DeviceInfo-only, never markup-selectable)
// are omitted until Essentials lands.
//
// Until the Essentials layer (M7+) ships a real DeviceInfo, the process-wide current() defaults from
// the BUILD: the platform the binary was compiled for (__APPLE__/TARGET_OS_* → mac_os or ios, _WIN32
// → win_ui, __ANDROID__ → android), with the idiom only where the build implies it (desktop on
// macOS/Windows; an iOS binary cannot know phone-vs-tablet statically → unknown). set_current is the
// DeviceInfo.SetCurrent test seam — the OnPlatform/OnIdiom tests pin it exactly like C#'s
// MockDeviceInfo.

#include <cstdint>

namespace maui::xaml
{
    enum class device_platform : std::uint8_t
    {
        unknown,      // DevicePlatform.Unknown
        android,      // DevicePlatform.Android
        gtk,          // DevicePlatform.Create("GTK") — the community platform OnPlatformExtension probes
        ios,          // DevicePlatform.iOS
        mac_catalyst, // DevicePlatform.MacCatalyst
        mac_os,       // DevicePlatform.macOS (AppKit — the port's apple backend)
        tizen,        // DevicePlatform.Tizen
        win_ui,       // DevicePlatform.WinUI (== DevicePlatform.UWP — same value in C#)
        wpf,          // DevicePlatform.Create("WPF") — probed by OnPlatformExtension
    };

    enum class device_idiom : std::uint8_t
    {
        unknown, // DeviceIdiom.Unknown
        phone,   // DeviceIdiom.Phone
        tablet,  // DeviceIdiom.Tablet
        desktop, // DeviceIdiom.Desktop
        tv,      // DeviceIdiom.TV
        watch,   // DeviceIdiom.Watch
    };

    struct xaml_runtime_environment
    {
        device_platform platform = device_platform::unknown;
        device_idiom idiom = device_idiom::unknown;

        // The environment compiled into this binary (see the header comment) — what current() starts as.
        [[nodiscard]] static xaml_runtime_environment build_default();

        // DeviceInfo.Platform / DeviceInfo.Idiom: the process-wide environment the extensions select by.
        [[nodiscard]] static const xaml_runtime_environment& current();
        // DeviceInfo.SetCurrent — the test seam (MockDeviceInfo).
        static void set_current(xaml_runtime_environment value);
    };
} // namespace maui::xaml
