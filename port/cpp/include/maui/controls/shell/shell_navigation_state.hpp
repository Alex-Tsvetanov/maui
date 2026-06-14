#pragma once
// maui::controls::shell_navigation_state  <=  Microsoft.Maui.Controls.ShellNavigationState
//
// A shell location expressed as a URI: full_location is the complete path (every implicit/default
// segment included), location is the user-facing trim (TrimDownImplicitAndDefaultPaths — implicit
// and default shell-element segments removed, pushed pages always kept; when everything was
// default, the last content segment survives so the location is never empty). Ported from
// ShellNavigationState.cs; the implicit string/Uri conversions become explicit constructors.

#include <string>

#include "maui/controls/shell/shell_uri.hpp"

namespace maui::controls
{
    class shell_navigation_state
    {
    public:
        shell_navigation_state() = default;
        // ShellNavigationState(string location) — trims for the user-facing Location.
        explicit shell_navigation_state(const std::string& location) : shell_navigation_state(location, true)
        {
        }
        explicit shell_navigation_state(const char* location) : shell_navigation_state(std::string{location}, true)
        {
        }
        // The internal (location, trimForUser) constructor.
        shell_navigation_state(const std::string& location, bool trim_for_user);

        // The user-facing trimmed location (C# Location).
        [[nodiscard]] const std::string& location() const
        {
            return location_;
        }
        // The complete location (C# internal FullLocation).
        [[nodiscard]] const std::string& full_location() const
        {
            return full_location_;
        }
        [[nodiscard]] shell_uri full_location_uri() const
        {
            return shell_uri::relative(full_location_);
        }

    private:
        std::string full_location_;
        std::string location_;
    };
} // namespace maui::controls
