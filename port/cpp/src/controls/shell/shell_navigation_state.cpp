// maui::controls::shell_navigation_state — out-of-line bodies. See the header.

#include "maui/controls/shell/shell_navigation_state.hpp"

#include <cstddef>
#include <string>
#include <vector>

#include "maui/controls/shell/routing.hpp"
#include "maui/controls/shell/shell_uri.hpp"
#include "maui/controls/shell/shell_uri_handler.hpp"

namespace maui::controls
{
    namespace
    {
        // ShellNavigationState.TrimDownImplicitAndDefaultPaths: strip implicit/default shell-element
        // segments from "//a/b/c/..."; pushed pages (index 5+) always survive; when every element
        // segment was trimmed the content segment (index 4) survives so the location stays rooted.
        std::string trim_down_implicit_and_default_paths(const std::string& full)
        {
            std::string normalized = shell_uri_handler::format_uri(full);
            if (!normalized.starts_with("//"))
            {
                return normalized; // don't trim relative pushes
            }

            std::string trimmed = normalized;
            while (trimmed.ends_with('/'))
            {
                trimmed.pop_back();
            }
            // C# string.Split('/') keeps empty entries: "//a/b" → ["", "", "a", "b"].
            std::vector<std::string> parts;
            std::size_t start = 0;
            while (true)
            {
                const std::size_t slash = trimmed.find('/', start);
                if (slash == std::string::npos)
                {
                    parts.push_back(trimmed.substr(start));
                    break;
                }
                parts.push_back(trimmed.substr(start, slash - start));
                start = slash + 1;
            }

            std::vector<std::string> to_keep;
            for (std::size_t i = 2; i < 5 && i < parts.size(); ++i)
            {
                // Keep a non-implicit/non-default element segment; also keep the content segment
                // (index 4) when every prior element was trimmed, so the location stays rooted.
                const bool is_meaningful = !routing::is_default(parts[i]) && !routing::is_implicit(parts[i]);
                if (is_meaningful || (i == 4 && to_keep.empty()))
                {
                    to_keep.push_back(parts[i]);
                }
            }
            for (std::size_t i = 5; i < parts.size(); ++i)
            {
                to_keep.push_back(parts[i]);
            }

            std::string result;
            for (const std::string& segment : to_keep)
            {
                result += "/";
                result += segment;
            }
            return "/" + result; // the two leading inserts of "" in C# produce the "//" prefix
        }
    } // namespace

    shell_navigation_state::shell_navigation_state(const std::string& location, bool trim_for_user)
    {
        shell_uri uri = shell_uri_handler::create_uri(location);
        if (uri.is_absolute())
        {
            uri = shell_uri::relative("/" + uri.path_and_query());
        }
        full_location_ = uri.original_string();
        location_ = trim_for_user ? trim_down_implicit_and_default_paths(full_location_) : full_location_;
    }
} // namespace maui::controls
