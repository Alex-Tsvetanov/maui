#pragma once
// maui::controls::shell_navigated_event_args  <=  Microsoft.Maui.Controls.ShellNavigatedEventArgs
//
// The Navigated payload: where the shell was (previous — nullopt for the very first navigation),
// where it is now (current), and how the navigation was initiated. Ported from
// ShellNavigatedEventArgs.cs.

#include <optional>
#include <utility>

#include "maui/controls/shell/shell_navigation_source.hpp"
#include "maui/controls/shell/shell_navigation_state.hpp"

namespace maui::controls
{
    class shell_navigated_event_args
    {
    public:
        shell_navigated_event_args(std::optional<shell_navigation_state> previous, shell_navigation_state current,
                                   shell_navigation_source source)
            : previous_(std::move(previous)), current_(std::move(current)), source_(source)
        {
        }

        [[nodiscard]] const std::optional<shell_navigation_state>& previous() const
        {
            return previous_;
        }
        [[nodiscard]] const shell_navigation_state& current() const
        {
            return current_;
        }
        [[nodiscard]] shell_navigation_source source() const
        {
            return source_;
        }

    private:
        std::optional<shell_navigation_state> previous_;
        shell_navigation_state current_;
        shell_navigation_source source_;
    };
} // namespace maui::controls
