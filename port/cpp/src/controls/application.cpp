// maui::controls::application — open/close windows + the one-time start + resume/sleep + theming
// (application.hpp). Ported from Application.cs.
#include "maui/controls/application.hpp"

#include <algorithm>

#include "maui/controls/window.hpp"
#include "maui/core/app_theme.hpp"

namespace maui::controls
{
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
