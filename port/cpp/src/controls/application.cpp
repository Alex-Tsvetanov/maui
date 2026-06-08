// maui::controls::application — open/close windows + the one-time start (application.hpp).
#include "maui/controls/application.hpp"

#include <algorithm>

#include "maui/controls/window.hpp"

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
        value.send_destroying(); // Deactivates (page Disappears + Unloads) then destroys
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
} // namespace maui::controls
