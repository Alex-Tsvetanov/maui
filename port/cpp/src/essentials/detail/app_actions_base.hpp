#pragma once
// maui::application_model::detail::app_actions_base  <=  the cross-platform half of
// Microsoft.Maui.ApplicationModel.AppActionsImplementation: the AppActionActivated event (a plain
// C# event field - no listener lifecycle) as the library's add_/remove_ accessor pair, with the
// protected raise the platform activation seam (PerformActionForShortcutItem / the headless
// simulate) drives.

#include <utility>

#include "maui/core/event.hpp"
#include "maui/essentials/app_actions.hpp"

namespace maui::application_model::detail
{
    class app_actions_base : public i_app_actions
    {
    public:
        maui::core::connection_token add_app_action_activated(
            maui::core::move_only_function<void(const app_action&)> handler) final
        {
            return app_action_activated_.connect(std::move(handler));
        }

        bool remove_app_action_activated(maui::core::connection_token token) final
        {
            return app_action_activated_.disconnect(token);
        }

    protected:
        app_actions_base() = default;

        // AppActionActivated?.Invoke(...) - the activation seam.
        void raise_app_action_activated(const app_action& action)
        {
            app_action_activated_.raise(action);
        }

    private:
        maui::core::event<app_action> app_action_activated_;
    };
} // namespace maui::application_model::detail
