// maui::controls::shell_uri_handler — ported 1:1 from ShellUriHandler.cs. See the header.
//
// Value-vs-reference note: C# RouteRequestBuilder is a reference type, but within SearchPath a child
// recursion always COPIES its parent's builder before extending it (new RouteRequestBuilder(parent)),
// and the post-search loops mutate list elements in place — both shapes survive the port's value
// semantics (mutate through references into the vector).

#include "maui/controls/shell/shell_uri_handler.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/shell/request_definition.hpp"
#include "maui/controls/shell/route_request_builder.hpp"
#include "maui/controls/shell/routing.hpp"
#include "maui/controls/shell/shell.hpp"
#include "maui/controls/shell/shell_content.hpp"
#include "maui/controls/shell/shell_item.hpp"
#include "maui/controls/shell/shell_navigation_manager.hpp"
#include "maui/controls/shell/shell_navigation_parameters.hpp"
#include "maui/controls/shell/shell_navigation_request.hpp"
#include "maui/controls/shell/shell_navigation_state.hpp"
#include "maui/controls/shell/shell_section.hpp"
#include "maui/controls/shell/shell_uri.hpp"

namespace maui::controls
{
    namespace
    {
        constexpr std::string_view path_separator = "/";

        bool is_blank(const std::string& value)
        {
            return std::ranges::all_of(value, [](char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; });
        }

        std::string join(const std::vector<std::string>& segments)
        {
            std::string joined;
            for (const std::string& segment : segments)
            {
                if (!joined.empty())
                {
                    joined += path_separator;
                }
                joined += segment;
            }
            return joined;
        }

        bool contains(const std::vector<std::string>& keys, const std::string& value)
        {
            return std::ranges::find(keys, value) != keys.end();
        }

        // C# string.Split(_pathSeparators) KEEPING empty entries (the ".." expansion uses this form).
        std::vector<std::string> split_keep_empty(const std::string& value)
        {
            std::vector<std::string> parts;
            std::size_t start = 0;
            while (true)
            {
                const std::size_t cut = value.find_first_of("/\\", start);
                if (cut == std::string::npos)
                {
                    parts.push_back(value.substr(start));
                    return parts;
                }
                parts.push_back(value.substr(start, cut - start));
                start = cut + 1;
            }
        }

        void search_path(const search_node& node, const route_request_builder* current_matched_path,
                         const std::vector<std::string>& segments, std::vector<route_request_builder>& possible,
                         int depth_to_start, int my_depth = -1, bool ignore_global_routes = true);

        std::vector<route_request_builder> search_for_global_routes(const std::vector<std::string>& segments,
                                                                    const shell_uri& starting_from,
                                                                    const shell_uri_handler::node_location& location,
                                                                    const std::vector<std::string>& route_keys);
    } // namespace

    // ---- global_route_item ------------------------------------------------------------------------

    std::vector<global_route_item> global_route_item::items() const
    {
        std::vector<std::string> segments = shell_uri_handler::retrieve_paths(path_);
        if (segments.size() <= 1)
        {
            return {};
        }
        segments.erase(segments.begin());
        return {global_route_item{routing::format_route(segments), source_route_}};
    }

    std::string global_route_item::route() const
    {
        const std::vector<std::string> segments = shell_uri_handler::retrieve_paths(path_);
        if (segments.empty())
        {
            return {};
        }
        return segments.front();
    }

    bool global_route_item::is_finished() const
    {
        return shell_uri_handler::retrieve_paths(path_).size() <= 1;
    }

    // ---- node_location ----------------------------------------------------------------------------

    search_node shell_uri_handler::node_location::lowest_child() const
    {
        if (content_ != nullptr)
        {
            return content_;
        }
        if (section_ != nullptr)
        {
            return section_;
        }
        if (item_ != nullptr)
        {
            return item_;
        }
        return shell_;
    }

    shell_uri_handler::node_location shell_uri_handler::node_location::create(shell& host)
    {
        node_location location;
        shell_item* item = host.current_item();
        shell_section* section = item != nullptr ? item->current_item() : nullptr;
        shell_content* content = section != nullptr ? section->current_item() : nullptr;
        if (content != nullptr)
        {
            location.set_node(search_node{content});
        }
        else if (section != nullptr)
        {
            location.set_node(search_node{section});
        }
        else if (item != nullptr)
        {
            location.set_node(search_node{item});
        }
        else
        {
            location.set_node(search_node{&host});
        }
        return location;
    }

    void shell_uri_handler::node_location::set_node(const search_node& node)
    {
        if (auto* const* host = std::get_if<shell*>(&node))
        {
            shell_ = *host;
            item_ = nullptr;
            section_ = nullptr;
            content_ = nullptr;
        }
        else if (auto* const* item = std::get_if<shell_item*>(&node))
        {
            item_ = *item;
            section_ = nullptr;
            content_ = nullptr;
            if (shell_ == nullptr && item_ != nullptr)
            {
                shell_ = dynamic_cast<shell*>(item_->logical_parent());
            }
        }
        else if (auto* const* section = std::get_if<shell_section*>(&node))
        {
            section_ = *section;
            if (item_ == nullptr && section_ != nullptr)
            {
                item_ = section_->parent_item();
            }
            if (shell_ == nullptr && item_ != nullptr)
            {
                shell_ = dynamic_cast<shell*>(item_->logical_parent());
            }
            content_ = nullptr;
        }
        else if (auto* const* content = std::get_if<shell_content*>(&node))
        {
            content_ = *content;
            if (section_ == nullptr && content_ != nullptr)
            {
                section_ = dynamic_cast<shell_section*>(content_->logical_parent());
            }
            if (item_ == nullptr && section_ != nullptr)
            {
                item_ = section_->parent_item();
            }
            if (shell_ == nullptr && item_ != nullptr)
            {
                shell_ = dynamic_cast<shell*>(item_->logical_parent());
            }
        }
        // a global_route_item engages no tree cursor (the C# switch has no case for it)
    }

    shell_uri shell_uri_handler::node_location::get_uri() const
    {
        std::vector<std::string> paths;
        paths.push_back(shell_->route_host());
        paths.push_back(shell_->route());
        if (item_ != nullptr && !routing::is_implicit(*item_))
        {
            paths.push_back(item_->route());
        }
        if (section_ != nullptr && !routing::is_implicit(*section_))
        {
            paths.push_back(section_->route());
        }
        if (content_ != nullptr && !routing::is_implicit(*content_))
        {
            paths.push_back(content_->route());
        }
        return shell_uri::parse(shell_->route_scheme() + "://" + join(paths));
    }

    void shell_uri_handler::node_location::pop()
    {
        if (content_ != nullptr)
        {
            content_ = nullptr;
        }
        else if (section_ != nullptr)
        {
            section_ = nullptr;
        }
        else if (item_ != nullptr)
        {
            item_ = nullptr;
        }
        else if (shell_ != nullptr)
        {
            shell_ = nullptr;
        }
    }

    std::optional<shell_uri_handler::node_location> shell_uri_handler::node_location::walk_to_next_node() const
    {
        std::size_t item_index = 0;
        std::size_t section_index = 0;
        std::size_t content_index = 0;

        const auto& items = shell_->items();
        if (item_ != nullptr)
        {
            for (std::size_t i = 0; i < items.size(); ++i)
            {
                if (items[i].get() == item_)
                {
                    item_index = i;
                    break;
                }
            }
        }
        if (section_ != nullptr && item_ != nullptr)
        {
            const auto& sections = item_->items();
            for (std::size_t j = 0; j < sections.size(); ++j)
            {
                if (sections[j].get() == section_)
                {
                    section_index = j;
                    break;
                }
            }
        }
        if (content_ != nullptr && section_ != nullptr)
        {
            const auto& contents = section_->items();
            for (std::size_t k = 0; k < contents.size(); ++k)
            {
                if (contents[k].get() == content_)
                {
                    content_index = k + 1;
                    break;
                }
            }
        }

        for (std::size_t i = item_index; i < items.size(); ++i)
        {
            const auto& sections = items[i]->items();
            for (std::size_t j = section_index; j < sections.size(); ++j)
            {
                const auto& contents = sections[j]->items();
                if (content_index < contents.size())
                {
                    node_location location;
                    location.set_node(search_node{items[i].get()});
                    location.set_node(search_node{sections[j].get()});
                    location.set_node(search_node{contents[content_index].get()});
                    return location;
                }
                content_index = 0;
            }
            section_index = 0;
        }
        return std::nullopt;
    }

    // ---- uri plumbing -----------------------------------------------------------------------------

    shell_uri shell_uri_handler::format_uri(const shell_uri& path, shell* host)
    {
        if (path.original_string().starts_with("..") && host != nullptr && host->current_state() != nullptr)
        {
            const std::string& original = path.original_string();
            const std::size_t question = original.find('?');
            const std::string path_part = original.substr(0, question);
            const std::string query_string = question == std::string::npos ? std::string{} : original.substr(question);

            std::vector<content_page*> pages = shell_navigation_manager::build_flattened_navigation_stack(*host);

            std::vector<std::string> rest_of_path;
            bool dots_all_parsed = false;
            for (const std::string& p : split_keep_empty(path_part))
            {
                if (p != ".." || dots_all_parsed)
                {
                    dots_all_parsed = true;
                    rest_of_path.push_back(p);
                    continue;
                }

                if (pages.empty() || pages.back() == nullptr)
                {
                    break;
                }
                pages.pop_back();

                std::vector<std::string> build_up_pages;
                for (const content_page* page : pages)
                {
                    if (page == nullptr)
                    {
                        continue;
                    }
                    const std::string route = routing::get_route(*page);
                    const std::vector<std::string> collapsed = collapse_path(route, build_up_pages, false);
                    build_up_pages.insert(build_up_pages.end(), collapsed.begin(), collapsed.end());
                }
                rest_of_path = build_up_pages;
            }

            const shell_item* item = host->current_item();
            const shell_section* section = item->current_item();
            const shell_content* content = section->current_item();
            const std::vector<std::string> shell_routes{item->route(), section->route(), content->route()};

            rest_of_path = collapse_path(std::move(rest_of_path), shell_routes, true);
            rest_of_path.insert(rest_of_path.begin(), content->route());
            rest_of_path.insert(rest_of_path.begin(), section->route());
            rest_of_path.insert(rest_of_path.begin(), item->route());

            const std::string result = join(rest_of_path) + query_string;
            // C# passes route: null — the empty segment it inserts is what produces the leading "//".
            const shell_uri standard =
                convert_to_standard_format("scheme", "host", std::string{}, shell_uri::relative(result));
            return shell_uri::relative(format_uri(standard.path_and_query()));
        }

        if (path.is_absolute())
        {
            return shell_uri::parse(format_uri(path.original_string()));
        }
        return shell_uri::relative(format_uri(path.original_string()));
    }

    std::string shell_uri_handler::format_uri(std::string path)
    {
        std::ranges::replace(path, '\\', '/');
        return path;
    }

    shell_uri shell_uri_handler::create_uri(std::string path)
    {
        path = format_uri(std::move(path));
        // A leading "/" forces relative (the C# iOS absolute-uri quirk guard).
        if (path.starts_with(path_separator))
        {
            return shell_uri::relative(std::move(path));
        }
        return shell_uri::parse(std::move(path)); // absolute when a scheme parses, else relative
    }

    bool shell_uri_handler::is_target_relative_pop(const shell_navigation_parameters& request)
    {
        if (!request.target_state)
        {
            return false;
        }
        bool is_relative_popping = false;
        for (const std::string& path : retrieve_paths(request.target_state->location()))
        {
            if (path != "..")
            {
                return false;
            }
            is_relative_popping = true;
        }
        return is_relative_popping;
    }

    shell_uri shell_uri_handler::convert_to_standard_format(shell* host, const shell_uri& request)
    {
        const shell_uri formatted = format_uri(request, host);
        if (host == nullptr)
        {
            return convert_to_standard_format("app", "shell", std::string{}, formatted);
        }
        return convert_to_standard_format(host->route_scheme(), host->route_host(), host->route(), formatted);
    }

    shell_uri shell_uri_handler::convert_to_standard_format(const std::string& route_scheme,
                                                            const std::string& route_host, const std::string& route,
                                                            const shell_uri& request)
    {
        std::string combined =
            request.is_absolute() ? request.host() + "/" + request.path_and_query() : request.original_string();
        std::string query;
        if (const std::size_t question = combined.find('?'); question != std::string::npos)
        {
            query = combined.substr(question); // includes the '?'
            combined = combined.substr(0, question);
        }

        std::vector<std::string> segments = retrieve_paths(combined);
        if (segments.empty() || segments.front() != route_host)
        {
            segments.insert(segments.begin(), route_host);
        }
        if (segments.size() < 2 || segments[1] != route)
        {
            segments.insert(segments.begin() + 1, route); // an empty `route` yields the "//" marker
        }

        return shell_uri::parse(route_scheme + "://" + join(segments) + query);
    }

    std::vector<std::string> shell_uri_handler::retrieve_paths(std::string_view uri)
    {
        std::vector<std::string> paths;
        std::size_t start = 0;
        while (start <= uri.size())
        {
            const std::size_t cut = uri.find_first_of("/\\", start);
            if (cut == std::string_view::npos)
            {
                if (start < uri.size())
                {
                    paths.emplace_back(uri.substr(start));
                }
                break;
            }
            if (cut > start)
            {
                paths.emplace_back(uri.substr(start, cut - start));
            }
            start = cut + 1;
        }
        return paths;
    }

    shell_uri_handler::stack_request_kind shell_uri_handler::calculate_stack_request(const shell_uri& uri)
    {
        if (uri.is_absolute() || uri.original_string().starts_with("//") || uri.original_string().starts_with("\\\\"))
        {
            return stack_request_kind::replace_it;
        }
        return stack_request_kind::push_to_it;
    }

    std::shared_ptr<shell_navigation_request> shell_uri_handler::get_navigation_request(
        shell& host, const shell_uri& uri, bool enable_relative_shell_routes, bool throw_navigation_error_as_exception,
        const shell_navigation_parameters* navigation_parameters)
    {
        (void)navigation_parameters; // C# only forwards it for diagnostics
        const shell_uri formatted = format_uri(uri, &host);

        const stack_request_kind what_do_i_do = calculate_stack_request(formatted);
        const shell_uri request = convert_to_standard_format(&host, formatted);

        std::vector<route_request_builder> possible_route_matches =
            generate_route_paths(host, request, formatted, enable_relative_shell_routes);

        if (possible_route_matches.empty())
        {
            if (throw_navigation_error_as_exception)
            {
                throw std::invalid_argument{"unable to figure out route for: " + formatted.original_string()};
            }
            return nullptr;
        }
        if (possible_route_matches.size() > 1)
        {
            std::string matches_found;
            for (const route_request_builder& match : possible_route_matches)
            {
                if (!matches_found.empty())
                {
                    matches_found += ",";
                }
                matches_found += match.path_full();
            }
            if (throw_navigation_error_as_exception)
            {
                throw std::invalid_argument{"Ambiguous routes matched for: " + formatted.original_string() +
                                            " matches found: " + matches_found};
            }
            return nullptr;
        }

        request_definition definition{possible_route_matches.front(), host};
        return std::make_shared<shell_navigation_request>(std::move(definition), what_do_i_do, request.query(),
                                                          request.fragment());
    }

    std::vector<route_request_builder> shell_uri_handler::generate_route_paths(shell& host, const shell_uri& request)
    {
        const shell_uri formatted = format_uri(request, &host);
        return generate_route_paths(host, formatted, formatted, false);
    }

    namespace
    {
        // ProcessRelativeRoute — the relative-uri resolution against the current location.
        std::vector<route_request_builder> process_relative_route(shell& host,
                                                                  const std::vector<std::string>& route_keys,
                                                                  const std::vector<std::string>& segments,
                                                                  bool enable_relative_shell_routes,
                                                                  const shell_uri& original_request)
        {
            auto current_location = shell_uri_handler::node_location::create(host);

            while (current_location.get_shell() != nullptr)
            {
                std::vector<route_request_builder> pure_routes_match;
                std::vector<route_request_builder> pure_global_routes_match;

                // Relative routes to shell elements aren't supported unless explicitly enabled.
                if (enable_relative_shell_routes)
                {
                    search_path(current_location.lowest_child(), nullptr, segments, pure_routes_match, 0);
                    shell_uri_handler::expand_out_global_routes(pure_routes_match, route_keys);
                    pure_routes_match = shell_uri_handler::get_best_matches(pure_routes_match);
                    if (!pure_routes_match.empty())
                    {
                        return pure_routes_match;
                    }
                }

                search_path(current_location.lowest_child(), nullptr, segments, pure_global_routes_match, 0, -1,
                            /*ignore_global_routes=*/false);
                shell_uri_handler::expand_out_global_routes(pure_global_routes_match, route_keys);

                if (current_location.content() != nullptr && pure_global_routes_match.empty())
                {
                    const shell_navigation_state* current_state = host.current_state();
                    if (current_state != nullptr)
                    {
                        std::vector<route_request_builder> matches = search_for_global_routes(
                            segments, current_state->full_location_uri(), current_location, route_keys);
                        pure_global_routes_match.insert(pure_global_routes_match.end(), matches.begin(), matches.end());
                    }
                }

                pure_global_routes_match = shell_uri_handler::get_best_matches(pure_global_routes_match);
                if (!pure_global_routes_match.empty())
                {
                    const int shell_elements_matched =
                        static_cast<int>(pure_global_routes_match[0].segments_matched().size()) -
                        static_cast<int>(pure_global_routes_match[0].global_route_matches().size());

                    if (!enable_relative_shell_routes && shell_elements_matched > 0)
                    {
                        throw std::runtime_error{
                            "Relative routing to shell elements is currently not supported. Try prefixing your "
                            "uri with ///: ///" +
                            original_request.original_string()};
                    }
                    return pure_global_routes_match;
                }

                current_location.pop();
            }

            const std::string search_path_joined = join(segments);
            if (contains(route_keys, search_path_joined))
            {
                return {route_request_builder{search_path_joined, search_path_joined, nullptr, segments}};
            }

            std::optional<route_request_builder> builder;
            for (const std::string& segment : segments)
            {
                if (contains(route_keys, segment))
                {
                    if (!builder)
                    {
                        builder.emplace(segment, segment, nullptr, segments);
                    }
                    else
                    {
                        builder->add_global_route(segment, segment);
                    }
                }
            }
            if (builder && builder->is_full_match())
            {
                return {*builder};
            }
            return {};
        }
    } // namespace

    std::vector<route_request_builder> shell_uri_handler::generate_route_paths(shell& host, shell_uri request,
                                                                               const shell_uri& original_request,
                                                                               bool enable_relative_shell_routes)
    {
        const std::vector<std::string> route_keys = routing::get_route_keys();

        request = format_uri(request, &host);
        const shell_uri formatted_original = format_uri(original_request, &host);

        std::vector<route_request_builder> possible_route_paths;
        if (!request.is_absolute())
        {
            request = convert_to_standard_format(&host, request);
        }
        const std::string local_path = request.local_path();

        const bool relative_match =
            !formatted_original.is_absolute() && !formatted_original.original_string().starts_with("//");

        std::vector<std::string> segments = retrieve_paths(local_path);

        int depth_start = 0;
        if (!segments.empty() && segments.front() == host.route())
        {
            segments.erase(segments.begin());
            depth_start = 1;
        }

        if (relative_match && host.current_item() != nullptr)
        {
            std::vector<route_request_builder> result =
                process_relative_route(host, route_keys, segments, enable_relative_shell_routes, formatted_original);
            if (!result.empty())
            {
                return result;
            }
        }

        possible_route_paths.clear();
        search_path(search_node{&host}, nullptr, segments, possible_route_paths, depth_start);

        std::vector<route_request_builder> best_matches = get_best_matches(possible_route_paths);
        if (!best_matches.empty())
        {
            return best_matches;
        }

        expand_out_global_routes(possible_route_paths, route_keys);

        for (route_request_builder& possible_route_path : possible_route_paths)
        {
            if (possible_route_path.is_full_match())
            {
                continue;
            }

            const std::string url = possible_route_path.path_full();
            const node_location current_location = possible_route_path.get_node_location();
            if (current_location.content() == nullptr)
            {
                continue;
            }

            const std::vector<route_request_builder> global_route_matches = search_for_global_routes(
                possible_route_path.remaining_segments(), shell_navigation_state{url, false}.full_location_uri(),
                current_location, route_keys);
            if (global_route_matches.size() != 1)
            {
                continue;
            }
            const route_request_builder& global_route_match = global_route_matches.front();

            while (!possible_route_path.next_segment().empty())
            {
                const auto& matched = global_route_match.segments_matched();
                const auto it = std::ranges::find(matched, possible_route_path.next_segment());
                if (it == matched.end())
                {
                    break;
                }
                const std::size_t match_index = static_cast<std::size_t>(it - matched.begin());
                possible_route_path.add_global_route(global_route_match.global_route_matches()[match_index],
                                                     matched[match_index]);
            }
        }

        possible_route_paths = get_best_matches(possible_route_paths);

        if (possible_route_paths.empty())
        {
            for (const std::string& route_key : route_keys)
            {
                if (route_key == formatted_original.original_string())
                {
                    return {route_request_builder{route_key, route_key, nullptr, {route_key}}};
                }
            }

            if (!relative_match)
            {
                for (const std::string& route : route_keys)
                {
                    const shell_uri uri = convert_to_standard_format(&host, create_uri(route));
                    if (uri == request)
                    {
                        std::string replaced = formatted_original.original_string();
                        std::size_t pos = 0;
                        while ((pos = replaced.find("//", pos)) != std::string::npos)
                        {
                            replaced.erase(pos, 2);
                        }
                        throw std::runtime_error{
                            "Global routes currently cannot be the only page on the stack, so absolute routing to "
                            "global routes is not supported. For now, just navigate to: " +
                            replaced};
                    }
                }
            }
        }
        return possible_route_paths;
    }

    namespace
    {
        std::vector<route_request_builder> search_for_global_routes(const std::vector<std::string>& segments,
                                                                    const shell_uri& starting_from,
                                                                    const shell_uri_handler::node_location& location,
                                                                    const std::vector<std::string>& route_keys)
        {
            std::vector<route_request_builder> pure_global_routes_match;
            const std::string new_path = join(segments);
            const std::vector<std::string> current_segments =
                shell_uri_handler::retrieve_paths(starting_from.original_string());
            const std::vector<std::string> new_segments =
                shell_uri_handler::collapse_path(new_path, current_segments, true);

            std::vector<std::string> full_route_with_new_segments = current_segments;
            full_route_with_new_segments.insert(full_route_with_new_segments.end(), new_segments.begin(),
                                                new_segments.end());

            // This is used to calculate whether the global route matches.
            std::vector<route_request_builder> probe;
            probe.emplace_back(full_route_with_new_segments);
            route_request_builder& route_request = probe.front();

            // add shell element routes
            (void)route_request.add_match(location);

            // add routes contributed by global routes already on the current location
            for (std::size_t i = 0; i < current_segments.size(); ++i)
            {
                const std::string& current_seg = current_segments[i];
                if (route_request.full_segments().size() <= i || current_seg != route_request.full_segments()[i])
                {
                    route_request.add_global_route(current_seg, current_seg);
                }
            }

            const std::vector<std::string> existing_global_routes = route_request.global_route_matches();
            shell_uri_handler::expand_out_global_routes(probe, route_keys);
            if (probe.front().is_full_match())
            {
                route_request_builder request_builder_with_new_segments{new_segments};
                const std::vector<std::string>& additional_route_matches = probe.front().global_route_matches();
                for (std::size_t i = existing_global_routes.size(); i < additional_route_matches.size(); ++i)
                {
                    request_builder_with_new_segments.add_global_route(additional_route_matches[i],
                                                                       segments[i - existing_global_routes.size()]);
                }
                pure_global_routes_match.push_back(std::move(request_builder_with_new_segments));
            }

            return pure_global_routes_match;
        }
    } // namespace

    std::vector<std::string> shell_uri_handler::collapse_path(const std::string& my_route,
                                                              const std::vector<std::string>& current_route_stack,
                                                              bool remove_user_defined_route)
    {
        return collapse_path(retrieve_paths(my_route), current_route_stack, remove_user_defined_route);
    }

    std::vector<std::string> shell_uri_handler::collapse_path(std::vector<std::string> my_route,
                                                              const std::vector<std::string>& current_route_stack,
                                                              bool remove_user_defined_route)
    {
        std::vector<std::string> local_route_stack = current_route_stack;
        for (std::size_t i = local_route_stack.size(); i-- > 0;)
        {
            const std::string& route = local_route_stack[i];
            if (routing::is_implicit(route) || (routing::is_default(route) && remove_user_defined_route))
            {
                local_route_stack.erase(local_route_stack.begin() + static_cast<std::ptrdiff_t>(i));
            }
        }

        std::vector<std::string> paths = std::move(my_route);

        // collapse similar leaves
        std::ptrdiff_t walk_back_index = -1;
        if (!paths.empty())
        {
            const auto it = std::ranges::find(local_route_stack, paths.front());
            walk_back_index = it == local_route_stack.end() ? -1 : it - local_route_stack.begin();
        }

        while (paths.size() > 1 && walk_back_index >= 0)
        {
            if (static_cast<std::size_t>(walk_back_index) >= local_route_stack.size())
            {
                break;
            }
            if (paths.front() == local_route_stack[static_cast<std::size_t>(walk_back_index)])
            {
                paths.erase(paths.begin());
            }
            else
            {
                break;
            }
            ++walk_back_index;
        }

        return paths;
    }

    namespace
    {
        // FindAndAddSegmentMatch — one expansion step of ExpandOutGlobalRoutes.
        bool find_and_add_segment_match(route_request_builder& possible_route_path,
                                        const std::vector<std::string>& route_keys)
        {
            // First search by collapsing global routes ("route1/route2/route3" registrations).
            for (const std::string& route_key : route_keys)
            {
                const std::vector<std::string> collapsed_routes =
                    shell_uri_handler::collapse_path(route_key, possible_route_path.segments_matched(), true);
                std::string collapsed_route = join(collapsed_routes);

                if (route_key.starts_with("//"))
                {
                    const std::vector<std::string> route_key_paths = shell_uri_handler::retrieve_paths(route_key);
                    if (!route_key_paths.empty() && !collapsed_routes.empty() &&
                        route_key_paths.front() == collapsed_routes.front())
                    {
                        collapsed_route.insert(0, "//");
                    }
                }

                const std::string collapsed_match = possible_route_path.get_next_segment_match(collapsed_route);
                if (!collapsed_match.empty() && !is_blank(collapsed_match))
                {
                    possible_route_path.add_global_route(route_key, collapsed_match);
                    return true;
                }

                // A registration mixing shell items and global routes may need a leaf jump.
                if (possible_route_path.get_shell() != nullptr &&
                    (possible_route_path.item() == nullptr || possible_route_path.section() == nullptr ||
                     possible_route_path.content() == nullptr))
                {
                    auto next_node = possible_route_path.get_node_location().walk_to_next_node();
                    while (next_node)
                    {
                        // A branch that no longer corresponds with the path searched so far.
                        if ((possible_route_path.item() != nullptr &&
                             next_node->item() != possible_route_path.item()) ||
                            (possible_route_path.section() != nullptr &&
                             next_node->section() != possible_route_path.section()) ||
                            (possible_route_path.content() != nullptr &&
                             next_node->content() != possible_route_path.content()))
                        {
                            next_node = next_node->walk_to_next_node();
                            continue;
                        }

                        route_request_builder leaf_search{possible_route_path};
                        if (!leaf_search.add_match(*next_node))
                        {
                            next_node = next_node->walk_to_next_node();
                            continue;
                        }

                        std::string collapsed_leaf_route =
                            join(shell_uri_handler::collapse_path(route_key, leaf_search.segments_matched(), true));
                        if (route_key.starts_with("//"))
                        {
                            collapsed_leaf_route.insert(0, "//");
                        }

                        const std::string segment_match = leaf_search.get_next_segment_match(collapsed_leaf_route);
                        if (!segment_match.empty() && !is_blank(segment_match))
                        {
                            (void)possible_route_path.add_match(*next_node);
                            possible_route_path.add_global_route(route_key, segment_match);
                            return true;
                        }

                        next_node = next_node->walk_to_next_node();
                    }
                }
            }

            // check for exact matches
            const std::string next = possible_route_path.next_segment();
            if (!next.empty() && contains(route_keys, next))
            {
                possible_route_path.add_global_route(next, next);
                return true;
            }

            return false;
        }
    } // namespace

    void shell_uri_handler::expand_out_global_routes(std::vector<route_request_builder>& possible_route_paths,
                                                     const std::vector<std::string>& route_keys)
    {
        for (route_request_builder& possible_route_path : possible_route_paths)
        {
            while (find_and_add_segment_match(possible_route_path, route_keys))
            {
            }

            while (!possible_route_path.is_full_match())
            {
                node_location location;
                location.set_node(possible_route_path.lowest_child());
                std::vector<route_request_builder> pure_global_routes_match;
                while (location.get_shell() != nullptr && pure_global_routes_match.empty())
                {
                    search_path(location.lowest_child(), nullptr, possible_route_path.remaining_segments(),
                                pure_global_routes_match, 0, -1, /*ignore_global_routes=*/false);
                    location.pop();
                }

                // nothing found or too many things found
                if (pure_global_routes_match.size() != 1 ||
                    pure_global_routes_match.front().global_route_matches().empty())
                {
                    break;
                }

                const route_request_builder& match = pure_global_routes_match.front();
                for (std::size_t i = 0; i < match.global_route_matches().size(); ++i)
                {
                    possible_route_path.add_global_route(match.global_route_matches()[i], match.segments_matched()[i]);
                }
            }
        }
    }

    std::vector<route_request_builder> shell_uri_handler::get_best_matches(
        const std::vector<route_request_builder>& possible_route_paths)
    {
        std::vector<const route_request_builder*> best_matches;
        for (const route_request_builder& match : possible_route_paths)
        {
            if (!match.is_full_match())
            {
                continue;
            }
            bool match_found = false;
            // remove duplicates (C#: the LAST shape-equal candidate decides, no break)
            for (const route_request_builder* best_match : best_matches)
            {
                if (best_match->item() == match.item() && best_match->section() == match.section() &&
                    best_match->content() == match.content() &&
                    best_match->global_route_matches().size() == match.global_route_matches().size())
                {
                    bool all_match = true;
                    for (std::size_t i = 0; i < best_match->global_route_matches().size(); ++i)
                    {
                        if (best_match->global_route_matches()[i] != match.global_route_matches()[i])
                        {
                            all_match = false;
                            break;
                        }
                    }
                    match_found = all_match;
                }
            }
            if (!match_found)
            {
                best_matches.push_back(&match);
            }
        }

        while (best_matches.size() > 1)
        {
            std::vector<const route_request_builder*> better_matches;
            const auto contains_ptr = [&better_matches](const route_request_builder* candidate) {
                return std::ranges::find(better_matches, candidate) != better_matches.end();
            };
            for (std::ptrdiff_t i = static_cast<std::ptrdiff_t>(best_matches.size()) - 1; i >= 0; --i)
            {
                for (std::ptrdiff_t j = i - 1; j >= 0; --j)
                {
                    const route_request_builder* better_match = nullptr;
                    if (best_matches[static_cast<std::size_t>(j)]->matched_parts() >
                        best_matches[static_cast<std::size_t>(i)]->matched_parts())
                    {
                        better_match = best_matches[static_cast<std::size_t>(j)];
                    }
                    else if (best_matches[static_cast<std::size_t>(j)]->matched_parts() <
                             best_matches[static_cast<std::size_t>(i)]->matched_parts())
                    {
                        better_match = best_matches[static_cast<std::size_t>(i)];
                    }

                    if (better_match == nullptr)
                    {
                        // nobody wins
                        if (!contains_ptr(best_matches[static_cast<std::size_t>(i)]))
                        {
                            better_matches.push_back(best_matches[static_cast<std::size_t>(i)]);
                        }
                        if (!contains_ptr(best_matches[static_cast<std::size_t>(j)]))
                        {
                            better_matches.push_back(best_matches[static_cast<std::size_t>(j)]);
                        }
                    }
                    else if (!contains_ptr(better_match))
                    {
                        better_matches.push_back(better_match);
                    }
                }
            }

            const bool stagnated = best_matches.size() == better_matches.size();
            best_matches = std::move(better_matches);
            if (stagnated)
            {
                break; // nothing was trimmed on the last pass
            }
        }

        std::vector<route_request_builder> result;
        result.reserve(best_matches.size());
        for (const route_request_builder* match : best_matches)
        {
            result.push_back(*match);
        }
        return result;
    }

    namespace
    {
        void search_path(const search_node& node, const route_request_builder* current_matched_path,
                         const std::vector<std::string>& segments, std::vector<route_request_builder>& possible,
                         int depth_to_start, int my_depth, bool ignore_global_routes)
        {
            if (std::holds_alternative<global_route_item>(node) && ignore_global_routes)
            {
                return;
            }

            ++my_depth;

            if (depth_to_start > my_depth)
            {
                for (const search_node& next_node : shell_uri_handler::node_items(node))
                {
                    search_path(next_node, nullptr, segments, possible, depth_to_start, my_depth, ignore_global_routes);
                }
                return;
            }

            const std::string shell_segment = shell_uri_handler::node_route(node);
            const std::string user_segment = current_matched_path == nullptr
                                                 ? (segments.empty() ? std::string{} : segments.front())
                                                 : current_matched_path->next_segment();

            if (user_segment.empty())
            {
                return;
            }

            std::optional<route_request_builder> builder;
            if (shell_segment == user_segment || routing::is_implicit(shell_segment))
            {
                if (current_matched_path == nullptr)
                {
                    builder.emplace(shell_segment, user_segment, &node, segments);
                }
                else
                {
                    builder.emplace(*current_matched_path);
                    builder->add_match(shell_segment, user_segment, node);
                }

                if (!routing::is_implicit(shell_segment) || shell_segment == user_segment)
                {
                    possible.push_back(*builder);
                }
            }

            for (const search_node& next_node : shell_uri_handler::node_items(node))
            {
                search_path(next_node, builder ? &*builder : nullptr, segments, possible, depth_to_start, my_depth,
                            ignore_global_routes);
            }
        }
    } // namespace

    std::string shell_uri_handler::node_route(const search_node& node)
    {
        if (auto* const* host = std::get_if<shell*>(&node))
        {
            return (*host)->route();
        }
        if (auto* const* item = std::get_if<shell_item*>(&node))
        {
            return (*item)->route();
        }
        if (auto* const* section = std::get_if<shell_section*>(&node))
        {
            return (*section)->route();
        }
        if (auto* const* content = std::get_if<shell_content*>(&node))
        {
            return (*content)->route();
        }
        return std::get<global_route_item>(node).route();
    }

    std::vector<search_node> shell_uri_handler::node_items(const search_node& node)
    {
        std::vector<search_node> results;
        if (auto* const* host = std::get_if<shell*>(&node))
        {
            for (shell_item* item : (*host)->visible_items())
            {
                results.emplace_back(item);
            }
        }
        else if (auto* const* item = std::get_if<shell_item*>(&node))
        {
            for (shell_section* section : (*item)->visible_items())
            {
                results.emplace_back(section);
            }
        }
        else if (auto* const* section = std::get_if<shell_section*>(&node))
        {
            for (shell_content* content : (*section)->visible_items())
            {
                results.emplace_back(content);
            }
        }
        else if (std::holds_alternative<shell_content*>(node))
        {
            // a content has no children
        }
        else
        {
            for (global_route_item& item : std::get<global_route_item>(node).items())
            {
                results.emplace_back(std::move(item));
            }
            return results; // a global route item contributes no further global-route children
        }

        const std::vector<std::string> keys = routing::get_route_keys();
        const std::string route = node_route(node);
        for (const std::string& key : keys)
        {
            if (key.starts_with(path_separator) && !std::holds_alternative<shell*>(node))
            {
                continue;
            }
            const std::vector<std::string> segments = retrieve_paths(key);
            if (!segments.empty() && segments.front() == route)
            {
                results.emplace_back(global_route_item{key, key});
            }
        }
        return results;
    }
} // namespace maui::controls
