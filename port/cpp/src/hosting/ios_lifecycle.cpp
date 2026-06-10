// maui::hosting::ios_lifecycle_builder — the iOS registration shell (ios_lifecycle.hpp). Ported from
// src/Core/src/LifecycleEvents/iOS/{iOSLifecycleBuilderExtensions,iOSLifecycleExtensions}.cs.

#include "maui/hosting/ios_lifecycle.hpp"

#include <functional>
#include <utility>

#include "maui/hosting/i_lifecycle_builder.hpp"

namespace maui::hosting
{
    ios_lifecycle_builder& ios_lifecycle_builder::will_finish_launching(lifecycle_action action)
    {
        builder_->add_event(ios_lifecycle_events::will_finish_launching, std::move(action));
        return *this;
    }

    ios_lifecycle_builder& ios_lifecycle_builder::finished_launching(lifecycle_action action)
    {
        builder_->add_event(ios_lifecycle_events::finished_launching, std::move(action));
        return *this;
    }

    ios_lifecycle_builder& ios_lifecycle_builder::on_activated(lifecycle_action action)
    {
        builder_->add_event(ios_lifecycle_events::on_activated, std::move(action));
        return *this;
    }

    ios_lifecycle_builder& ios_lifecycle_builder::on_resign_activation(lifecycle_action action)
    {
        builder_->add_event(ios_lifecycle_events::on_resign_activation, std::move(action));
        return *this;
    }

    ios_lifecycle_builder& ios_lifecycle_builder::will_enter_foreground(lifecycle_action action)
    {
        builder_->add_event(ios_lifecycle_events::will_enter_foreground, std::move(action));
        return *this;
    }

    ios_lifecycle_builder& ios_lifecycle_builder::did_enter_background(lifecycle_action action)
    {
        builder_->add_event(ios_lifecycle_events::did_enter_background, std::move(action));
        return *this;
    }

    ios_lifecycle_builder& ios_lifecycle_builder::will_terminate(lifecycle_action action)
    {
        builder_->add_event(ios_lifecycle_events::will_terminate, std::move(action));
        return *this;
    }

    i_lifecycle_builder& add_ios(i_lifecycle_builder& builder,
                                 const std::function<void(ios_lifecycle_builder&)>& configure)
    {
        if (configure)
        {
            ios_lifecycle_builder shell{builder};
            configure(shell);
        }
        return builder;
    }
} // namespace maui::hosting
