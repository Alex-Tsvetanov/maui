#pragma once
// maui::controls::shell_navigation_parameters  <=  Microsoft.Maui.Controls.ShellNavigationParameters
//
// The internal bag a shell navigation travels as: the target state (or the page being pushed for
// route-less Navigation.PushAsync-style calls), animation, relative-shell-route opt-in, the
// deferred Navigating args of a resumed navigation, and the parameter dictionary. Ported from
// ShellNavigationParameters.cs (a plain aggregate there too).

#include <memory>
#include <optional>

#include "maui/controls/shell/shell_navigating_event_args.hpp"
#include "maui/controls/shell/shell_navigation_state.hpp"
#include "maui/controls/shell/shell_route_parameters.hpp"

namespace maui::controls
{
    class content_page;

    struct shell_navigation_parameters
    {
        std::shared_ptr<shell_navigating_event_args> deferred_args; // a resumed (deferred) navigation
        std::optional<shell_navigation_state> target_state;
        bool enable_relative_shell_routes = false;
        std::optional<bool> animated;
        bool pop_all_pages_not_specified_on_target_state = false;
        content_page* page_pushing = nullptr; // NON-owning: Navigation.PushAsync without routes
        std::optional<shell_route_parameters> parameters;
        std::optional<bool> can_cancel;
    };
} // namespace maui::controls
