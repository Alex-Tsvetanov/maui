#pragma once
// maui::application_model::app_info             <=  Microsoft.Maui.ApplicationModel.AppInfo (static facade)
// maui::application_model::i_app_info           <=  Microsoft.Maui.ApplicationModel.IAppInfo
// maui::application_model::app_packaging_model  <=  Microsoft.Maui.ApplicationModel.AppPackagingModel
// maui::application_model::layout_direction     <=  Microsoft.Maui.ApplicationModel.LayoutDirection
//
// Information about the running application. RequestedTheme reuses the existing maui::core::
// app_theme (AppTheme.shared.cs was already ported there for the Controls Application surface);
// Version reuses maui::devices::version_info (the System.Version slice + Utils.ParseVersion port
// from device_info) - every platform partial implements Version as ParseVersion(VersionString),
// mirrored by the i_app_info::version() default body.
//
// Backends (suffix oracle): apple/macOS + ios REAL (AppInfo.ios.tvos.watchos.macos.cs - the
// NSBundle info dictionary; a missing key reads as "" where C# reads null). show_settings_ui:
// ios opens UIApplicationOpenSettingsURLString through the launcher; the macOS partial drives
// System Settings via a ScriptingBridge SBApplication - the port opens the
// "x-apple.systempreferences:" URL through NSWorkspace instead (DOCUMENTED DEVIATION: same user
// outcome, no ScriptingBridge dependency). requested_theme: macOS = NSAppearance best-match
// (aqua/dark-aqua), ios = UIScreen.mainScreen.traitCollection.userInterfaceStyle.
// requested_layout_direction: ios = UIApplication.sharedApplication.userInterfaceLayoutDirection
// (the port has no WindowStateManager, so the C# current-window probe collapses to the
// application-wide value); apple = NSApplication.sharedApplication.userInterfaceLayoutDirection.
// Headless mirrors netstandard (name/package/version/build/settings throw; theme unspecified,
// layout direction unknown, packaging model THROWS) until faked.

#include <memory>
#include <string>

#include "maui/core/app_theme.hpp"
#include "maui/essentials/device_info.hpp" // maui::devices::version_info (the System.Version slice)

namespace maui::application_model
{
    // Packaging options (Windows-specific in practice; every other platform reports packaged).
    enum class app_packaging_model
    {
        packaged = 0,
        unpackaged = 1,
    };

    // Possible layout directions (LayoutDirection.shared.cs).
    enum class layout_direction
    {
        unknown = 0,
        left_to_right = 1,
        right_to_left = 2,
    };

    class i_app_info
    {
    public:
        virtual ~i_app_info() = default;

        [[nodiscard]] virtual std::string package_name() const = 0;
        [[nodiscard]] virtual std::string name() const = 0;
        [[nodiscard]] virtual std::string version_string() const = 0;
        // IAppInfo.Version - every partial implements it as Utils.ParseVersion(VersionString).
        [[nodiscard]] virtual maui::devices::version_info version() const
        {
            return maui::devices::version_info::parse(version_string());
        }
        [[nodiscard]] virtual std::string build_string() const = 0;

        // Open the settings menu or page for this application.
        virtual void show_settings_ui() = 0;

        [[nodiscard]] virtual maui::core::app_theme requested_theme() const = 0;
        [[nodiscard]] virtual app_packaging_model packaging_model() const = 0;
        [[nodiscard]] virtual layout_direction requested_layout_direction() const = 0;

    protected:
        i_app_info() = default;
        i_app_info(const i_app_info&) = default;
        i_app_info(i_app_info&&) = default;
        i_app_info& operator=(const i_app_info&) = default;
        i_app_info& operator=(i_app_info&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (AppInfoImplementation), one per backend under
        // src/platform/<backend>/essentials_app_info.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_app_info> make_app_info();
    } // namespace detail

    // The static facade. Statics forward to current(), exactly like the C# static class.
    class app_info final
    {
    public:
        app_info() = delete;

        [[nodiscard]] static std::string package_name()
        {
            return current().package_name();
        }
        [[nodiscard]] static std::string name()
        {
            return current().name();
        }
        [[nodiscard]] static std::string version_string()
        {
            return current().version_string();
        }
        [[nodiscard]] static maui::devices::version_info version()
        {
            return current().version();
        }
        [[nodiscard]] static std::string build_string()
        {
            return current().build_string();
        }
        static void show_settings_ui()
        {
            current().show_settings_ui();
        }
        [[nodiscard]] static maui::core::app_theme requested_theme()
        {
            return current().requested_theme();
        }
        [[nodiscard]] static app_packaging_model packaging_model()
        {
            return current().packaging_model();
        }
        [[nodiscard]] static layout_direction requested_layout_direction()
        {
            return current().requested_layout_direction();
        }

        // AppInfo.Current (lazy platform default) + SetCurrent (the C# internal test seam made
        // public; nullptr resets to the lazy platform default).
        [[nodiscard]] static i_app_info& current();
        static void set_current(std::shared_ptr<i_app_info> implementation);

        // The owning handle behind current() - the port's analog of passing AppInfo.Current into
        // VersionTrackingImplementation's constructor (shared_ptr ownership doctrine).
        [[nodiscard]] static std::shared_ptr<i_app_info> current_shared();
    };
} // namespace maui::application_model
