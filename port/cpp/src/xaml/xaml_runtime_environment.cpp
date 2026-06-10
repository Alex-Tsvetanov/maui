// maui::xaml::xaml_runtime_environment — the DeviceInfo.Platform/Idiom stand-in
// (xaml_runtime_environment.hpp).
#include "maui/xaml/xaml_runtime_environment.hpp"

// Clang predefines the TARGET_OS_* macros as builtins on Darwin, so the header is only needed for
// compilers that don't (GCC) — and including it unconditionally would be flagged as unused there.
#if defined(__APPLE__) && !defined(TARGET_OS_OSX)
    #include <TargetConditionals.h>
#endif

namespace maui::xaml
{
    namespace
    {
        [[nodiscard]] xaml_runtime_environment& current_storage()
        {
            static xaml_runtime_environment environment = xaml_runtime_environment::build_default();
            return environment;
        }
    } // namespace

    xaml_runtime_environment xaml_runtime_environment::build_default()
    {
        // The platform this binary was COMPILED for — the closest no-Essentials analog of C#'s
        // runtime DeviceInfo (each C# DeviceInfo implementation is itself compiled per platform).
#ifdef __ANDROID__
        return {.platform = device_platform::android, .idiom = device_idiom::unknown};
#elifdef __APPLE__
        // TARGET_OS_* are VALUE macros (TargetConditionals.h defines every one as 0/1), so the apple
        // split is a constexpr-if chain over their values rather than more preprocessor branches.
        if constexpr (TARGET_OS_MACCATALYST != 0)
        {
            return {.platform = device_platform::mac_catalyst, .idiom = device_idiom::desktop};
        }
        else if constexpr (TARGET_OS_IPHONE != 0)
        {
            // iOS/simulator: phone-vs-tablet is a runtime fact (UIUserInterfaceIdiom) — unknown here.
            return {.platform = device_platform::ios, .idiom = device_idiom::unknown};
        }
        else if constexpr (TARGET_OS_OSX != 0)
        {
            // The port's apple (AppKit) backend and the headless preset built on a mac run on macOS.
            return {.platform = device_platform::mac_os, .idiom = device_idiom::desktop};
        }
        else
        {
            return {.platform = device_platform::unknown, .idiom = device_idiom::unknown};
        }
#elifdef _WIN32
        return {.platform = device_platform::win_ui, .idiom = device_idiom::desktop};
#else
        // C# XamlParser treats an unavailable DeviceInfo as "platform unknown" (its try/catch around
        // DeviceInfo.Platform) — the headless port on an unmapped OS does the same.
        return {.platform = device_platform::unknown, .idiom = device_idiom::unknown};
#endif
    }

    const xaml_runtime_environment& xaml_runtime_environment::current()
    {
        return current_storage();
    }

    void xaml_runtime_environment::set_current(xaml_runtime_environment value)
    {
        current_storage() = value;
    }
} // namespace maui::xaml
