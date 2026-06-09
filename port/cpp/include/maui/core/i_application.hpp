#pragma once
// maui::core::i_application  <=  Microsoft.Maui.IApplication (: IElement)
//
// The cross-platform virtual-view contract an application_handler would service — the Core seam between the
// concrete maui::controls::application and the platform application object, exactly as MAUI's IApplication
// is the cross-platform face the platform startup talks to (not the concrete Application). Ported from
// src/Core/src/Core/IApplication.cs.
//
// This cut keeps the surface the platform startup actually uses: Windows (the open windows, exposed as the
// cross-platform i_window* — and, since a window is an i_element, the i_element face for the logical tree),
// the create/open/close_window window-management hooks, ThemeChanged (the platform notifies a theme change),
// and the UserAppTheme getter (the user's preferred theme). IActivationState (create_window's argument),
// ActivateWindow, and persisted state are out of scope (STATUS.md) — added when a backend needs them, so
// create_window takes no activation argument here.
//
// An application is an i_element (C# IApplication : IElement) — it has the same parent/handler shape as a
// window, though at this layer it owns no native view (its handler stays null, like a root window keeps a
// null parent). windows() returns the open windows as i_window* (NON-owning — the caller owns the windows'
// lifetime, PROFILE §8), mirroring IApplication.Windows (IReadOnlyList<IWindow>).

#include <vector>

#include "maui/core/app_theme.hpp"
#include "maui/core/i_element.hpp"

namespace maui::core
{
    class i_window;

    class i_application : public i_element
    {
    public:
        ~i_application() override = default;

        // ---- IApplication.Windows — the instantiated windows, as the cross-platform i_window contract.
        // NON-owning: the caller owns the windows' lifetime (PROFILE §8).
        [[nodiscard]] virtual std::vector<i_window*> windows() const = 0;

        // ---- IApplication window management ----
        // IApplication.CreateWindow: instantiate (and return) a new window. The concrete application's
        // override creates the window it manages; the IActivationState argument is out of scope here.
        [[nodiscard]] virtual i_window* create_window() = 0;
        // IApplication.OpenWindow: request the application open this window (adds it, starts the app on the
        // first open, activates it). Takes the cross-platform i_window face.
        virtual void open_window(i_window& value) = 0;
        // IApplication.CloseWindow: request the application close this window (destroys + drops it).
        virtual void close_window(i_window& value) = 0;

        // ---- IApplication theming ----
        // IApplication.UserAppTheme: the user's preferred theme (overrides the platform theme; Unspecified
        // means follow the platform).
        [[nodiscard]] virtual app_theme user_app_theme() const = 0;
        // IApplication.ThemeChanged: the platform notifies the application that the OS theme changed. C#'s
        // ThemeChanged() is parameterless (the platform pushes the new theme through PlatformAppTheme); the
        // concrete application reads the platform theme it was given.
        virtual void theme_changed() = 0;

    protected:
        i_application() = default;
        i_application(const i_application&) = default;
        i_application(i_application&&) = default;
        i_application& operator=(const i_application&) = default;
        i_application& operator=(i_application&&) = default;
    };
} // namespace maui::core
