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
//
// An application is ALSO an i_application (C# Application : IApplication : IElement), the cross-platform face
// the platform startup talks to — exactly as window is-a i_window. The seam it exposes (windows() as
// i_window*, create/open/close_window, theme_changed, user_app_theme) routes to the concrete API below.
// Like a root window keeps a null parent, an application owns no native handler at this layer (handler() is
// null, set_handler a no-op) — the platform application object is out of scope (STATUS.md). create_window is
// a virtual override point: the base returns null (an application with no windows of its own); a subclass
// overrides it to instantiate the window it manages (Application.CreateWindow).
//
// open_window / close_window come in TWO flavours, mirroring how window exposes content() (the i_element
// face) and content_element() (the concrete face): the i_window& overrides satisfy i_application and
// down-cast to the concrete window; the window& overloads are the developer-facing concrete API. windows()
// is the i_application override (i_window* by value); windows_typed() is the concrete window* list.

#include <functional>
#include <memory>
#include <vector>

#include "maui/controls/element.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/app_theme.hpp"
#include "maui/core/event.hpp"
#include "maui/core/i_application.hpp"
#include "maui/core/i_window.hpp"

namespace maui::hosting
{
    class maui_app; // the hosting boot context passed to the mount hooks below (forward-declared)
} // namespace maui::hosting

namespace maui::controls
{
    class application : public element, public maui::core::i_application
    {
    public:
        // ---- i_element (the handler seam — an application is an IElement, like a window). An application
        // owns no native view at this layer, so it has no handler and no parent (it is the tree root; the
        // windows are its logical children, not parents). ----
        [[nodiscard]] const std::shared_ptr<maui::core::i_element_handler>& handler() const override
        {
            return handler_; // always null — no platform application object at this layer (STATUS.md)
        }
        void set_handler(std::shared_ptr<maui::core::i_element_handler> /*value*/) override
        {
            // No-op: an application has no native view to wire (the platform application object is out of
            // scope). Declared so application is a concrete i_element.
        }
        [[nodiscard]] std::shared_ptr<maui::core::i_element> parent() const override
        {
            return nullptr; // the application is the tree root
        }

        // ---- IApplication.Windows ----
        // The open windows as the cross-platform i_window contract (i_application override). By value +
        // NON-owning — the caller owns the windows' lifetime.
        [[nodiscard]] std::vector<maui::core::i_window*> windows() const override
        {
            return {windows_.begin(), windows_.end()};
        }
        // The open windows as the concrete control type (the developer-facing face). NON-owning.
        [[nodiscard]] const std::vector<window*>& windows_typed() const
        {
            return windows_;
        }
        // The first-opened window, or null when none are open (a convenience for the common single-window app).
        [[nodiscard]] window* main_window() const
        {
            return windows_.empty() ? nullptr : windows_.front();
        }

        // IApplication.CreateWindow: instantiate the window this application manages. The base application
        // manages no window of its own, so it returns null; a subclass overrides this to create one.
        [[nodiscard]] maui::core::i_window* create_window() override
        {
            return nullptr;
        }

        // IApplication.OpenWindow: add the window, start the app on the first open, wire its resume/sleep
        // back to this app, then create + activate it (so its page Appears + Loads). Opening an already-open
        // window is a no-op. The concrete window& overload is the developer-facing API; the i_window&
        // override (i_application) down-casts to it.
        void open_window(window& value);
        void open_window(maui::core::i_window& value) override;
        // IApplication.CloseWindow: destroy the window (Deactivating/Unloading its page) and drop it.
        void close_window(window& value);
        void close_window(maui::core::i_window& value) override;

        // Fired once, when the application starts (the first window opens) — Application.OnStart's seam.
        maui::core::event<> started;
        // Application.Resumed / Stopped (the OnResume / OnSleep seam) — driven by a window's send_resumed /
        // send_stopped through the resume hook open_window installs. SendResume/SendSleep are idempotent in
        // the sense that they simply re-raise; the port raises them on every window resume/stop.
        maui::core::event<> resumed;
        maui::core::event<> stopped;

        // ---- Theming (Application.UserAppTheme / PlatformAppTheme / RequestedTheme / RequestedThemeChanged) ----
        // The user's preferred theme, overriding the platform theme (Unspecified = follow the platform).
        [[nodiscard]] maui::core::app_theme user_app_theme() const override
        {
            return user_app_theme_;
        }
        void set_user_app_theme(maui::core::app_theme value);
        // The OS theme (set by the platform; the port's stand-in for AppInfo.RequestedTheme — a backend
        // pushes it here, then calls theme_changed()).
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
        // IApplication.ThemeChanged: the platform notifies the app the OS theme changed. C#'s ThemeChanged()
        // is parameterless — it sets PlatformAppTheme = AppInfo.RequestedTheme (re-reads the OS theme). The
        // port has no AppInfo singleton, so the backend pushes the OS theme via set_platform_app_theme first;
        // theme_changed() re-triggers from the current platform theme (RequestedThemeChanged fires on a real
        // change). Parameterless, satisfying i_application.
        void theme_changed() override
        {
            trigger_theme_changed();
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

    public:
        // ---- Generic-mount lifecycle hooks (no C# 1:1) ----
        // The generic hosting mount (maui::hosting::mount_window) calls these around the window's mount so an
        // application can do per-app wiring the backend-agnostic driver cannot infer, WITHOUT any platform code
        // in the user's app: on_pre_mount runs BEFORE the page tree is mounted (e.g. register a handler for a
        // user-defined control type into this boot's registry — there is no global fallback), on_post_mount runs
        // AFTER the window is open and every native view exists (e.g. demo seeding that needs live handlers:
        // opening a SwipeView to its revealed state, driving a synthetic gesture, subscribing to the app theme).
        // Both default to no-ops, so an app opts in only by overriding. This generalizes the gallery's former
        // per-page register_handlers / on_mounted plumbing onto the application the builder already minted.
        virtual void on_pre_mount(maui::hosting::maui_app& /*app*/)
        {
        }
        virtual void on_post_mount(maui::hosting::maui_app& /*app*/)
        {
        }

    private:
        void send_start();            // idempotent: raise `started` + call on_start once
        void trigger_theme_changed(); // Application.TriggerThemeChangedActual: fire on a real change only

        std::vector<window*> windows_;                           // NON-owning open windows (IApplication.Windows)
        std::shared_ptr<maui::core::i_element_handler> handler_; // always null — no platform application object
        maui::core::app_theme user_app_theme_ = maui::core::app_theme::unspecified;
        maui::core::app_theme platform_app_theme_ = maui::core::app_theme::unspecified;
        maui::core::app_theme last_app_theme_ = maui::core::app_theme::unspecified; // _lastAppTheme
        bool theme_changed_firing_ = false;                                         // _themeChangedFiring guard
        bool started_ = false;
    };
} // namespace maui::controls
