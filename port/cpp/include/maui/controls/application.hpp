#pragma once
// maui::controls::application  <=  Microsoft.Maui.Controls.Application (+ Microsoft.Maui.IApplication)
//
// The root of a running app: it owns the set of open windows, the one-time start hook, the resume/sleep
// drive, and the color-theme state. open_window adds a window, starts the app the first time (Window.Created
// → Application.SendStart), wires the window's resume/sleep back to this app, and activates it; close_window
// destroys + removes it. An application is an element, so each window is a logical child and the
// application's BindingContext inherits down to the windows (and on to their pages). Ported from
// Application.cs (OpenWindow/CloseWindow + SendStart/SendResume/SendSleep + the UserAppTheme/
// PlatformAppTheme/RequestedTheme theming). Multi-window orchestration subtleties, persisted state, and the
// platform application object are out of scope (STATUS.md).

#include <functional>
#include <vector>

#include "maui/controls/element.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/app_theme.hpp"
#include "maui/core/event.hpp"

namespace maui::controls
{
    class application : public element
    {
    public:
        // The open windows (IApplication.Windows). NON-owning — the caller owns the windows' lifetime.
        [[nodiscard]] const std::vector<window*>& windows() const
        {
            return windows_;
        }
        // The first-opened window, or null when none are open (a convenience for the common single-window app).
        [[nodiscard]] window* main_window() const
        {
            return windows_.empty() ? nullptr : windows_.front();
        }

        // IApplication.OpenWindow: add the window, start the app on the first open, wire its resume/sleep
        // back to this app, then create + activate it (so its page Appears + Loads). Opening an already-open
        // window is a no-op.
        void open_window(window& value);
        // IApplication.CloseWindow: destroy the window (Deactivating/Unloading its page) and drop it.
        void close_window(window& value);

        // Fired once, when the application starts (the first window opens) — Application.OnStart's seam.
        maui::core::event<> started;
        // Application.Resumed / Stopped (the OnResume / OnSleep seam) — driven by a window's send_resumed /
        // send_stopped through the resume hook open_window installs. SendResume/SendSleep are idempotent in
        // the sense that they simply re-raise; the port raises them on every window resume/stop.
        maui::core::event<> resumed;
        maui::core::event<> stopped;

        // ---- Theming (Application.UserAppTheme / PlatformAppTheme / RequestedTheme / RequestedThemeChanged) ----
        // The user's preferred theme, overriding the platform theme (Unspecified = follow the platform).
        [[nodiscard]] maui::core::app_theme user_app_theme() const
        {
            return user_app_theme_;
        }
        void set_user_app_theme(maui::core::app_theme value);
        // The OS theme (set by the platform; a backend pushes it via IApplication.ThemeChanged).
        [[nodiscard]] maui::core::app_theme platform_app_theme() const
        {
            return platform_app_theme_;
        }
        void set_platform_app_theme(maui::core::app_theme value);
        // The effective theme: UserAppTheme when set (≠ Unspecified), else PlatformAppTheme (RequestedTheme).
        [[nodiscard]] maui::core::app_theme requested_theme() const
        {
            return user_app_theme_ != maui::core::app_theme::unspecified ? user_app_theme_ : platform_app_theme_;
        }
        // IApplication.ThemeChanged: the platform notifies the app the OS theme changed (sets PlatformAppTheme).
        void theme_changed(maui::core::app_theme platform_theme)
        {
            set_platform_app_theme(platform_theme);
        }
        // Application.RequestedThemeChanged — fired (with the new RequestedTheme) when the effective theme
        // actually changes (Application.TriggerThemeChangedActual: only on a real change, re-entrancy-guarded).
        maui::core::event<maui::core::app_theme> requested_theme_changed;

    protected:
        // Each open window is a logical child, so the application's BindingContext inherits down to it.
        void for_each_logical_child(const std::function<void(element&)>& visit) const override
        {
            for (window* const value : windows_)
            {
                visit(*value);
            }
        }
        // Application.OnStart override point (a subclass may create its main window here).
        virtual void on_start()
        {
        }

    private:
        void send_start();            // idempotent: raise `started` + call on_start once
        void trigger_theme_changed(); // Application.TriggerThemeChangedActual: fire on a real change only

        std::vector<window*> windows_; // NON-owning open windows (IApplication.Windows)
        maui::core::app_theme user_app_theme_ = maui::core::app_theme::unspecified;
        maui::core::app_theme platform_app_theme_ = maui::core::app_theme::unspecified;
        maui::core::app_theme last_app_theme_ = maui::core::app_theme::unspecified; // _lastAppTheme
        bool theme_changed_firing_ = false;                                         // _themeChangedFiring guard
        bool started_ = false;
    };
} // namespace maui::controls
