// maui::hosting::apple_lifecycle_builder — the AppKit registration shell (apple_lifecycle.hpp), the
// macOS twin of ios_lifecycle.cpp (net-new vs C#: MAUI has no AppKit target).

#include "maui/hosting/apple_lifecycle.hpp"

#include <functional>
#include <utility>

#include "maui/hosting/i_lifecycle_builder.hpp"

namespace maui::hosting
{
    apple_lifecycle_builder& apple_lifecycle_builder::did_finish_launching(lifecycle_action action)
    {
        builder_->add_event(apple_lifecycle_events::did_finish_launching, std::move(action));
        return *this;
    }

    apple_lifecycle_builder& apple_lifecycle_builder::did_become_active(lifecycle_action action)
    {
        builder_->add_event(apple_lifecycle_events::did_become_active, std::move(action));
        return *this;
    }

    apple_lifecycle_builder& apple_lifecycle_builder::did_resign_active(lifecycle_action action)
    {
        builder_->add_event(apple_lifecycle_events::did_resign_active, std::move(action));
        return *this;
    }

    apple_lifecycle_builder& apple_lifecycle_builder::will_terminate(lifecycle_action action)
    {
        builder_->add_event(apple_lifecycle_events::will_terminate, std::move(action));
        return *this;
    }

    i_lifecycle_builder& add_apple(i_lifecycle_builder& builder,
                                   const std::function<void(apple_lifecycle_builder&)>& configure)
    {
        if (configure)
        {
            apple_lifecycle_builder shell{builder};
            configure(shell);
        }
        return builder;
    }
} // namespace maui::hosting
