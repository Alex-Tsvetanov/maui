// maui::controls::routing — the process-wide route registry. See routing.hpp.
//
// The statics mirror C#'s (s_routes / s_implicitPageRoutes / s_routeCount); get_route_keys is rebuilt
// on demand instead of cached (the C# s_routeKeys cache is an allocation optimization, not behavior).

#include "maui/controls/shell/routing.hpp"

#include <algorithm>
#include <cstddef>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/shell/shell.hpp"
#include "maui/controls/shell/shell_item.hpp"
#include "maui/controls/shell/shell_section.hpp"
#include "maui/core/bindable_object.hpp"

namespace maui::controls
{
    namespace
    {
        struct routing_state
        {
            // Registration order is behavior (first-match-wins loops), so keep ordered vectors.
            std::vector<std::pair<std::string, std::shared_ptr<route_factory>>> routes;
            std::vector<std::pair<std::string, content_page*>> implicit_page_routes; // NON-owning pages
            std::map<const maui::core::bindable_object*, std::string> element_routes;
            int route_count = 0;
        };

        routing_state& state()
        {
            static routing_state instance;
            return instance;
        }

        std::string normalize_separators(std::string route)
        {
            std::ranges::replace(route, '\\', '/');
            return route;
        }

        bool is_blank(std::string_view route)
        {
            return std::ranges::all_of(route, [](char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; });
        }

        void validate_route(const std::string& route, const std::shared_ptr<route_factory>& factory)
        {
            if (route.empty() || is_blank(route))
            {
                throw std::invalid_argument{"Route cannot be an empty string"};
            }
            if (factory == nullptr)
            {
                throw std::invalid_argument{"Route Factory cannot be null"};
            }
            std::string_view rest{route};
            while (!rest.empty())
            {
                const std::size_t cut = rest.find_first_of("/\\");
                const std::string_view part = cut == std::string_view::npos ? rest : rest.substr(0, cut);
                rest = cut == std::string_view::npos ? std::string_view{} : rest.substr(cut + 1);
                if (!part.empty() && routing::is_implicit(part))
                {
                    throw std::invalid_argument{"Route contains invalid characters in \"" + std::string{part} + "\""};
                }
            }
            for (const auto& [existing_route, existing_factory] : state().routes)
            {
                if (existing_route == route && !existing_factory->equals(*factory))
                {
                    throw std::invalid_argument{"Duplicated Route: \"" + route + "\""};
                }
            }
        }
    } // namespace

    void routing::register_route(const std::string& route, std::shared_ptr<route_factory> factory)
    {
        validate_route(route, factory);
        auto& routes = state().routes;
        const auto it =
            std::ranges::find(routes, route, &std::pair<std::string, std::shared_ptr<route_factory>>::first);
        if (it != routes.end())
        {
            it->second = std::move(factory);
        }
        else
        {
            routes.emplace_back(route, std::move(factory));
        }
    }

    void routing::unregister_route(const std::string& route)
    {
        std::erase_if(state().routes, [&route](const auto& entry) { return entry.first == route; });
    }

    void routing::clear()
    {
        state().routes.clear();
        state().implicit_page_routes.clear();
        state().element_routes.clear();
    }

    std::string routing::get_route(const maui::core::bindable_object& source)
    {
        auto& routes = state().element_routes;
        const auto it = routes.find(&source);
        if (it != routes.end())
        {
            return it->second;
        }
        // The C# defaultValueCreator: $"D_FAULT_{TypeName}{++count}". The reflection-free port mints
        // the counter-only form — only the prefix and uniqueness are behavior.
        std::string minted = std::string{default_prefix} + std::to_string(++state().route_count);
        routes.emplace(&source, minted);
        return minted;
    }

    void routing::set_route(const maui::core::bindable_object& element, std::string value)
    {
        state().element_routes.insert_or_assign(&element, std::move(value));
    }

    void routing::remove_route(const maui::core::bindable_object& element)
    {
        state().element_routes.erase(&element);
    }

    std::string routing::generate_implicit_route(const std::string& source)
    {
        if (is_implicit(source))
        {
            return source;
        }
        return std::string{implicit_prefix} + source;
    }

    bool routing::is_implicit(std::string_view route)
    {
        return route.starts_with(implicit_prefix);
    }

    bool routing::is_implicit(const maui::core::bindable_object& source)
    {
        return is_implicit(get_route(source));
    }

    bool routing::is_default(std::string_view route)
    {
        return route.starts_with(default_prefix);
    }

    bool routing::is_default(const maui::core::bindable_object& source)
    {
        return is_default(get_route(source));
    }

    bool routing::is_user_defined(std::string_view route)
    {
        return !(is_default(route) || is_implicit(route));
    }

    bool routing::is_user_defined(const maui::core::bindable_object* source)
    {
        if (source == nullptr)
        {
            return false;
        }
        return is_user_defined(get_route(*source));
    }

    std::string routing::get_route_path_if_not_implicit(const maui::core::bindable_object& element)
    {
        const std::string source = get_route(element);
        if (is_implicit(source))
        {
            return {};
        }
        return source + "/";
    }

    std::string routing::format_route(const std::vector<std::string>& segments)
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
        return format_route(std::move(joined));
    }

    std::string routing::format_route(std::string route)
    {
        return route; // the C# body is the identity too
    }

    void routing::register_implicit_page_route(content_page& page)
    {
        const std::string route = get_route(page);
        if (!is_user_defined(route))
        {
            auto& pages = state().implicit_page_routes;
            const auto it = std::ranges::find(pages, route, &std::pair<std::string, content_page*>::first);
            if (it != pages.end())
            {
                it->second = &page;
            }
            else
            {
                pages.emplace_back(route, &page);
            }
        }
    }

    void routing::register_implicit_page_routes(shell& host)
    {
        for (const std::shared_ptr<shell_item>& item : host.items())
        {
            for (const std::shared_ptr<shell_section>& section : item->items())
            {
                const std::vector<content_page*>& stack = section->stack();
                for (std::size_t i = 1; i < stack.size(); ++i)
                {
                    register_implicit_page_route(*stack[i]);
                }
                // C# also walks the modal stack here; the port's shell has no modal stack (STATUS.md).
            }
        }
    }

    void routing::clear_implicit_page_routes()
    {
        state().implicit_page_routes.clear();
    }

    std::vector<std::string> routing::get_route_keys()
    {
        std::vector<std::string> keys;
        keys.reserve(state().routes.size() + state().implicit_page_routes.size());
        for (const auto& [route, factory] : state().routes)
        {
            keys.push_back(normalize_separators(route)); // ShellUriHandler.FormatUri(key)
        }
        for (const auto& [route, page] : state().implicit_page_routes)
        {
            if (std::ranges::find(keys, route) == keys.end())
            {
                keys.push_back(normalize_separators(route));
            }
        }
        return keys;
    }

    std::shared_ptr<content_page> routing::get_or_create_content(const std::string& route)
    {
        const auto& pages = state().implicit_page_routes;
        const auto page_it = std::ranges::find(pages, route, &std::pair<std::string, content_page*>::first);
        if (page_it != pages.end())
        {
            // The live page, borrowed: a non-owning shared_ptr (the page's owner controls its life).
            return {page_it->second, [](content_page*) {}};
        }

        const auto& routes = state().routes;
        const auto it =
            std::ranges::find(routes, route, &std::pair<std::string, std::shared_ptr<route_factory>>::first);
        std::shared_ptr<content_page> result;
        if (it != routes.end())
        {
            result = it->second->get_or_create();
        }
        if (result != nullptr)
        {
            set_route(*result, route);
        }
        return result;
    }

    void routing::validate_for_duplicates(const maui::core::bindable_object& element, std::string_view route,
                                          const std::vector<const maui::core::bindable_object*>& siblings)
    {
        // Re-setting the same route is always fine; only user-defined routes are validated.
        if (get_route(element) == route || route.empty() || !is_user_defined(route))
        {
            return;
        }
        for (const maui::core::bindable_object* sibling : siblings)
        {
            if (sibling == &element || sibling == nullptr)
            {
                continue;
            }
            if (get_route(*sibling) == route)
            {
                throw std::invalid_argument{"Duplicated Route: \"" + std::string{route} +
                                            "\" is already registered to another element. Routes must be unique "
                                            "among siblings to avoid navigation conflicts."};
            }
        }
    }
} // namespace maui::controls
