#pragma once
// maui::controls::shell_uri_handler  <=  Microsoft.Maui.Controls.ShellUriHandler (internal)
//
// The Shell URI parser/matcher: turns a navigation URI (absolute "//host/path?query", relative,
// "../") into a shell_navigation_request — the winning shell_item/section/content plus the global
// route segments to push. Ported 1:1 from ShellUriHandler.cs (the algorithm IS the contract — the
// ShellUriHandlerTests exercise it exhaustively):
//   - format_uri expands a leading ".." against the flattened current navigation stack;
//   - convert_to_standard_format pins every request to "scheme://host/route/...";
//   - generate_route_paths walks the item tree (search_path) accepting implicit segments for free,
//     then expands GLOBAL routes (Routing.RegisterRoute keys) over the remaining segments;
//   - get_best_matches dedupes + keeps only the most-specific full matches;
//   - get_navigation_request wraps the single winner (0 → std::invalid_argument "unable to figure
//     out route", >1 → std::invalid_argument "Ambiguous routes matched", both gated by
//     throw_navigation_error_as_exception like C#).
// C#'s `object node` tree walk becomes the search_node variant; the lazy GetItems enumerable
// becomes an eager vector (the sequences are tiny). node_location / global_route_item are nested in
// C# — kept as siblings here, one cluster per the PROFILE §3 cohesion rule.

#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "maui/controls/shell/shell_uri.hpp"

namespace maui::controls
{
    class shell;
    class shell_item;
    class shell_section;
    class shell_content;
    class route_request_builder;
    class shell_navigation_request;
    struct shell_navigation_parameters;

    // C# ShellUriHandler.GlobalRouteItem: one registered route key viewed as a chain of segment
    // nodes ("a/b/c" → node "a" whose items() is node "b/c", ...).
    class global_route_item
    {
    public:
        global_route_item(std::string path, std::string source_route)
            : path_(std::move(path)), source_route_(std::move(source_route))
        {
        }

        [[nodiscard]] std::vector<global_route_item> items() const;
        [[nodiscard]] std::string route() const;
        [[nodiscard]] bool is_finished() const;
        [[nodiscard]] const std::string& source_route() const
        {
            return source_route_;
        }

    private:
        std::string path_;
        std::string source_route_;
    };

    // The C# `object node` of the tree walk: exactly one alternative is engaged.
    using search_node = std::variant<shell*, shell_item*, shell_section*, shell_content*, global_route_item>;

    class shell_uri_handler
    {
    public:
        shell_uri_handler() = delete; // all-static, like the C# internal class

        // C# ShellUriHandler.NodeLocation — a cursor into the shell item tree.
        class node_location
        {
        public:
            [[nodiscard]] shell* get_shell() const
            {
                return shell_;
            }
            [[nodiscard]] shell_item* item() const
            {
                return item_;
            }
            [[nodiscard]] shell_section* section() const
            {
                return section_;
            }
            [[nodiscard]] shell_content* content() const
            {
                return content_;
            }
            // The deepest engaged node (Content ?? Section ?? Item ?? Shell).
            [[nodiscard]] search_node lowest_child() const;

            [[nodiscard]] static node_location create(shell& host);
            void set_node(const search_node& node);
            [[nodiscard]] shell_uri get_uri() const;
            void pop();
            // The next content slot in tree order, or nullopt at the end (WalkToNextNode).
            [[nodiscard]] std::optional<node_location> walk_to_next_node() const;

        private:
            shell* shell_ = nullptr;
            shell_item* item_ = nullptr;
            shell_section* section_ = nullptr;
            shell_content* content_ = nullptr;
        };

        // ---- uri plumbing ----
        // Expand a leading ".." against the current stack, then normalize separators.
        [[nodiscard]] static shell_uri format_uri(const shell_uri& path, shell* host);
        [[nodiscard]] static std::string format_uri(std::string path);
        // C# CreateUri: a leading "/" forces relative; otherwise absolute when a scheme parses.
        [[nodiscard]] static shell_uri create_uri(std::string path);
        [[nodiscard]] static bool is_target_relative_pop(const shell_navigation_parameters& request);
        [[nodiscard]] static shell_uri convert_to_standard_format(shell* host, const shell_uri& request);
        [[nodiscard]] static shell_uri convert_to_standard_format(const std::string& route_scheme,
                                                                  const std::string& route_host,
                                                                  const std::string& route, const shell_uri& request);
        [[nodiscard]] static std::vector<std::string> retrieve_paths(std::string_view uri);

        // ---- the matcher ----
        enum class stack_request_kind
        {
            replace_it,
            push_to_it,
        };
        [[nodiscard]] static stack_request_kind calculate_stack_request(const shell_uri& uri);

        [[nodiscard]] static std::shared_ptr<shell_navigation_request> get_navigation_request(
            shell& host, const shell_uri& uri, bool enable_relative_shell_routes = false,
            bool throw_navigation_error_as_exception = true,
            const shell_navigation_parameters* navigation_parameters = nullptr);

        [[nodiscard]] static std::vector<route_request_builder> generate_route_paths(shell& host,
                                                                                     const shell_uri& request);
        [[nodiscard]] static std::vector<route_request_builder> generate_route_paths(shell& host, shell_uri request,
                                                                                     const shell_uri& original_request,
                                                                                     bool enable_relative_shell_routes);

        // Collapse `my_route` against the segments already on the stack (CollapsePath).
        [[nodiscard]] static std::vector<std::string> collapse_path(const std::string& my_route,
                                                                    const std::vector<std::string>& current_route_stack,
                                                                    bool remove_user_defined_route);
        [[nodiscard]] static std::vector<std::string> collapse_path(std::vector<std::string> my_route,
                                                                    const std::vector<std::string>& current_route_stack,
                                                                    bool remove_user_defined_route);

        static void expand_out_global_routes(std::vector<route_request_builder>& possible_route_paths,
                                             const std::vector<std::string>& route_keys);
        [[nodiscard]] static std::vector<route_request_builder> get_best_matches(
            const std::vector<route_request_builder>& possible_route_paths);

        // The route of one tree node (GetRoute(object)).
        [[nodiscard]] static std::string node_route(const search_node& node);
        // The children of one tree node + the global-route items rooted at it (GetItems(object)).
        [[nodiscard]] static std::vector<search_node> node_items(const search_node& node);
    };
} // namespace maui::controls
