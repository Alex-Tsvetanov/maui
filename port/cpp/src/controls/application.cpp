// maui::controls::application — open/close windows + the one-time start + resume/sleep + theming
// (application.hpp). Ported from Application.cs.
#include "maui/controls/application.hpp"

#include <algorithm>

#include "maui/controls/window.hpp"
#include "maui/core/app_theme.hpp"
#include "maui/core/i_window.hpp"
#include "maui/essentials/app_info.hpp" // AppInfo.RequestedTheme — the OS theme the ctor seeds from

namespace maui::controls
{
    // Application.Current stand-in: the last-constructed application is the process-current one (C# sets
    // Application.Current = this in the ctor). Cleared in the dtor only if we are still current — a newer
    // application may already have taken over. ponytail: one application per process, so this is enough; if
    // the port ever runs concurrent applications, current() would need scoping.
    application* application::current_ = nullptr;

    application::application()
    {
        current_ = this;
        // THE APP THEME STARTS FROM THE OS — Application.cs:61, verbatim:
        //     _platformAppTheme = AppInfo.RequestedTheme;
        //     _lastAppTheme = _platformAppTheme;
        //
        // This line was MISSING, and its absence is the root of an env-var-vs-OS theme split that reached
        // every backend. Without it platform_app_theme_ stayed `unspecified` for the whole process life
        // unless something called set_platform_app_theme, so an app that did not was permanently on the
        // light branch: {AppThemeBinding} resolves Unspecified to its LIGHT slot (AppThemeBinding.GetValue),
        // while the native window — left at overrideUserInterfaceStyle=Unspecified / NSAppearance=nil /
        // ElementTheme::Default — kept following the OS. Light port-drawn content, dark native controls.
        //
        // MEASURED on a Mac in Dark mode, AppKit gallery, MAUI_APPEARANCE unset, page app_theme_binding:
        // white page, green "…green in light mode, and red in dark mode" text, orange LightPrimaryColor —
        // i.e. the light slot throughout, on a dark desktop, on a page that states its own expected result.
        //
        // The galleries hid this from the board because they seeded the theme from MAUI_APPEARANCE and
        // defaulted it to `light` when unset, so a capture never exercised the unseeded path. Fixing it here
        // rather than per-backend is what makes the OS the single source: the four hosts already push
        // requested_theme() into their native window, so once this agrees with the OS, both layers do.
        platform_app_theme_ = maui::application_model::app_info::requested_theme();
        last_app_theme_ = platform_app_theme_;
    }

    application::~application()
    {
        if (current_ == this)
        {
            current_ = nullptr;
        }
    }

    void application::open_window(window& value)
    {
        if (std::ranges::find(windows_, &value) != windows_.end())
        {
            return; // already open
        }
        windows_.push_back(&value);
        value.set_inherited_binding_context(raw_binding_context()); // the window inherits the app's context
        // Route the window's resume/sleep back to this application (Window.cs Application?.SendResume /
        // SendSleep). A bare raise — SendResume/SendSleep have no extra app-side state in this cut.
        value.set_resume_hook([this] { resumed.raise(); }, [this] { stopped.raise(); });
        send_start();           // C#: the first Window.Created drives Application.SendStart
        value.send_created();   // IWindow.Created
        value.send_activated(); // OpenWindow ultimately activates -> the page Appears + Loads
    }

    void application::close_window(window& value)
    {
        const auto it = std::ranges::find(windows_, &value);
        if (it == windows_.end())
        {
            return;
        }
        value.send_destroying();                 // Deactivates (page Disappears + Unloads) then destroys
        value.set_resume_hook(nullptr, nullptr); // drop the back-reference to this app
        windows_.erase(it);
    }

    // The i_application overrides: the cross-platform i_window& face down-casts to the concrete window this
    // application manages (every window opened here is a maui::controls::window), then routes to the concrete
    // overload. A non-window i_window (none exist at this layer) is ignored.
    void application::open_window(maui::core::i_window& value)
    {
        if (auto* concrete = dynamic_cast<window*>(&value))
        {
            open_window(*concrete);
        }
    }

    void application::close_window(maui::core::i_window& value)
    {
        if (auto* concrete = dynamic_cast<window*>(&value))
        {
            close_window(*concrete);
        }
    }

    void application::send_start()
    {
        if (started_)
        {
            return;
        }
        started_ = true;
        started.raise();
        on_start();
    }

    void application::set_user_app_theme(maui::core::app_theme value)
    {
        if (user_app_theme_ == value)
        {
            return; // C# UserAppTheme setter early-out
        }
        user_app_theme_ = value;
        trigger_theme_changed();
    }

    void application::set_platform_app_theme(maui::core::app_theme value)
    {
        if (platform_app_theme_ == value)
        {
            return; // C# PlatformAppTheme setter early-out
        }
        platform_app_theme_ = value;
        trigger_theme_changed();
    }

    void application::theme_changed()
    {
        // C# IApplication.ThemeChanged (Application.cs:567) is one line — `PlatformAppTheme =
        // AppInfo.RequestedTheme;` — re-reading the OS rather than trusting a pushed value.
        const maui::core::app_theme os_theme = maui::application_model::app_info::requested_theme();
        if (os_theme == maui::core::app_theme::unspecified)
        {
            // DEVIATION, deliberate: C#'s AppInfo always answers, so this branch is dead there. The port
            // still has stub app_info partials on android + windows that answer `unspecified` while the
            // backend DOES know the theme and pushed it through set_platform_app_theme. Assigning the stub's
            // answer would erase a known theme, which is strictly worse than keeping it — so an "I don't
            // know" read leaves platform_app_theme_ alone and only re-triggers, the pre-existing behaviour.
            trigger_theme_changed();
            return;
        }
        set_platform_app_theme(os_theme);
    }

    void application::trigger_theme_changed()
    {
        // C# Application.TriggerThemeChangedActual: fire only when the effective (Requested) theme actually
        // changes, guarded against re-entrancy (the platform may notify more than once).
        const maui::core::app_theme new_theme = requested_theme();
        if (theme_changed_firing_ || new_theme == last_app_theme_)
        {
            return;
        }
        theme_changed_firing_ = true;
        last_app_theme_ = new_theme;
        requested_theme_changed.raise(new_theme);
        theme_changed_firing_ = false;
    }
} // namespace maui::controls
