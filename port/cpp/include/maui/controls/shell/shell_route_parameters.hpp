#pragma once
// maui::controls::shell_route_parameters  <=  Microsoft.Maui.Controls.ShellRouteParameters
//
// The parameter dictionary a shell navigation carries: string keys to boxed (std::any) values —
// query-string parameters arrive as std::string values; the typed GoToAsync(state, parameters)
// overload passes caller-boxed values straight through. Ported from ShellRouteParameters.cs:
//   - the PREFIX constructor filters "route.key" entries down to "key" for one route's element;
//   - set_query_string_parameters parses "?a=1&b=2" (WebUtils.UnpackParameters) — keys already
//     present win over query-string values (C# only adds missing keys); '%XX' and '+' are decoded.
// DEVIATION (documented): C#'s ShellNavigationQueryParameters single-use sub-dictionary (one-shot
// parameters cleared after they reach a ContentPage) is not modeled — every parameter behaves like a
// regular dictionary entry. reset_to_query_parameters is therefore a no-op kept for call-site parity.

#include <any>
#include <map>
#include <string>
#include <string_view>

namespace maui::controls
{
    class shell_route_parameters
    {
    public:
        using map_type = std::map<std::string, std::any, std::less<>>;
        using const_iterator = map_type::const_iterator;

        shell_route_parameters() = default;
        explicit shell_route_parameters(map_type values) : values_(std::move(values))
        {
        }
        // The C# prefix-filter constructor: keep only keys starting with `prefix`, stripped of it,
        // dropping any remaining dotted keys (they belong to a deeper route).
        shell_route_parameters(const shell_route_parameters& query, std::string_view prefix);

        // Merge the query string into this dictionary; existing keys win (C# only adds missing ones).
        void set_query_string_parameters(std::string_view query);

        // C# ResetToQueryParameters — a no-op here (see the deviation note above).
        void reset_to_query_parameters()
        {
        }

        [[nodiscard]] bool contains(std::string_view key) const
        {
            return values_.find(key) != values_.end();
        }
        // The boxed value, or nullptr when absent (borrowed; valid until the map mutates).
        [[nodiscard]] const std::any* try_get(std::string_view key) const;
        // The value as a string, or "" when absent / not a string (the query-string common case).
        [[nodiscard]] std::string get_string(std::string_view key) const;
        void set(std::string key, std::any value)
        {
            values_.insert_or_assign(std::move(key), std::move(value));
        }
        void erase(std::string_view key);

        [[nodiscard]] std::size_t size() const
        {
            return values_.size();
        }
        [[nodiscard]] bool empty() const
        {
            return values_.empty();
        }
        [[nodiscard]] const_iterator begin() const
        {
            return values_.begin();
        }
        [[nodiscard]] const_iterator end() const
        {
            return values_.end();
        }

    private:
        map_type values_;
    };
} // namespace maui::controls
