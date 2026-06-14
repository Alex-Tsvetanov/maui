#pragma once
// maui::controls::routing  <=  Microsoft.Maui.Controls.Routing (+ RouteFactory)
//
// The process-wide URI-route registry shell navigation resolves against. Ported from Routing.cs:
//   - register_route(route, factory) / register_route<TPage>(route) — the developer registration
//     (C# RegisterRoute(string, RouteFactory) / RegisterRoute(string, Type); the Type overload's
//     Activator.CreateInstance becomes the typed make_shared factory — no reflection, PROFILE §6);
//     validation throws std::invalid_argument exactly where C# throws ArgumentException /
//     ArgumentNullException (empty route, "IMPL_" segments, duplicate route with a different factory).
//   - get_route/set_route — C#'s attached RouteProperty. The port has no attached-property store, so
//     routes live in a process-wide pointer-keyed side map (the same pattern grid uses for its
//     attached cell info); the first get_route on an unrouted object mints the "D_FAULT_<n>" default
//     (C# defaultValueCreator). CAVEAT (documented): the map cannot observe object death — an entry
//     for a destroyed object lingers until clear() (base_shell_item removes its own entry in its
//     destructor; plain pages rely on clear(), which the tests run between cases like C#'s
//     Routing.Clear()).
//   - implicit routes: generate_implicit_route ("IMPL_" + source), is_implicit / is_default /
//     is_user_defined, and the implicit PAGE route set (register_implicit_page_route(s) /
//     clear_implicit_page_routes) that lets the whole nav stack be expressed as a URI.
//   - get_or_create_content(route) — resolve a route to its page: an implicit page route returns the
//     live page (wrapped in a NON-OWNING shared_ptr — the stack's keep-alive map just won't extend
//     its life); a registered factory mints (or returns) its element. C#'s IServiceProvider overload
//     (DI activation) is not ported — factories encapsulate construction instead.
//   - get_route_keys() returns the keys in REGISTRATION ORDER (C# iterates the Dictionary's
//     insertion order; the matching loops are first-match-wins, so order is behavior).

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "maui/core/bindable_object.hpp"

namespace maui::controls
{
    class content_page; // forward — factories mint pages; routing.cpp includes the real header
    class shell;        // forward — register_implicit_page_routes walks the shell's item tree

    // C# Microsoft.Maui.Controls.RouteFactory: creates (or returns) the element for one route.
    class route_factory
    {
    public:
        virtual ~route_factory() = default;
        [[nodiscard]] virtual std::shared_ptr<content_page> get_or_create() = 0;
        // C# RouteFactory.Equals — duplicate registration with an EQUAL factory is allowed.
        [[nodiscard]] virtual bool equals(const route_factory& other) const
        {
            return this == &other;
        }

    protected:
        route_factory() = default;
        route_factory(const route_factory&) = default;
        route_factory(route_factory&&) = default;
        route_factory& operator=(const route_factory&) = default;
        route_factory& operator=(route_factory&&) = default;
    };

    // The C# TypeRouteFactory: every registration for the same TPage compares equal (so re-registering
    // the same route with the same page type succeeds, a different type throws).
    template <class TPage> class typed_route_factory final : public route_factory
    {
    public:
        [[nodiscard]] std::shared_ptr<content_page> get_or_create() override
        {
            return std::make_shared<TPage>();
        }
        [[nodiscard]] bool equals(const route_factory& other) const override
        {
            return dynamic_cast<const typed_route_factory*>(&other) != nullptr;
        }
    };

    class routing
    {
    public:
        routing() = delete; // a static registry, like the C# static class

        static constexpr std::string_view implicit_prefix = "IMPL_";
        static constexpr std::string_view default_prefix = "D_FAULT_";
        static constexpr std::string_view path_separator = "/";

        // ---- registration (Routing.RegisterRoute / UnRegisterRoute / Clear) ----
        static void register_route(const std::string& route, std::shared_ptr<route_factory> factory);
        template <class TPage> static void register_route(const std::string& route)
        {
            register_route(route, std::make_shared<typed_route_factory<TPage>>());
        }
        static void unregister_route(const std::string& route);
        // Clears the factories, the implicit page routes AND the element-route side map (the side map
        // wipe is the port's lifetime hygiene — see the header note).
        static void clear();

        // ---- the attached Route property (Routing.GetRoute / SetRoute) ----
        [[nodiscard]] static std::string get_route(const maui::core::bindable_object& source);
        static void set_route(const maui::core::bindable_object& element, std::string value);
        // Drop the side-map entry for a dying element (called by base_shell_item's destructor).
        static void remove_route(const maui::core::bindable_object& element);

        // ---- implicit/default classification ----
        [[nodiscard]] static std::string generate_implicit_route(const std::string& source);
        [[nodiscard]] static bool is_implicit(std::string_view route);
        [[nodiscard]] static bool is_implicit(const maui::core::bindable_object& source);
        [[nodiscard]] static bool is_default(std::string_view route);
        [[nodiscard]] static bool is_default(const maui::core::bindable_object& source);
        [[nodiscard]] static bool is_user_defined(std::string_view route);
        [[nodiscard]] static bool is_user_defined(const maui::core::bindable_object* source);

        // "route/" when user-defined, "" when implicit (Routing.GetRoutePathIfNotImplicit).
        [[nodiscard]] static std::string get_route_path_if_not_implicit(const maui::core::bindable_object& element);

        // FormatRoute — the C# bodies are pass-throughs (join + identity); kept for call-site parity.
        [[nodiscard]] static std::string format_route(const std::vector<std::string>& segments);
        [[nodiscard]] static std::string format_route(std::string route);

        // ---- the implicit PAGE route set (pushed pages tracked so ".." can re-derive the stack) ----
        static void register_implicit_page_route(content_page& page);
        static void register_implicit_page_routes(shell& host);
        static void clear_implicit_page_routes();

        // Every known route key (registered + implicit page routes), '\\' normalized to '/', in
        // registration order (first-match-wins loops iterate this).
        [[nodiscard]] static std::vector<std::string> get_route_keys();

        // Resolve `route` to a page: the live implicit page (non-owning), or a factory-minted page
        // (owning), or nullptr. A resolved page gets the route stamped on it (C# SetRoute(result, route)).
        [[nodiscard]] static std::shared_ptr<content_page> get_or_create_content(const std::string& route);

        // Routing.ValidateForDuplicates: a user-defined route must be unique among the element's
        // siblings; throws std::invalid_argument on a clash. `siblings` is supplied by the caller
        // (base_shell_item::set_route) because the port's logical-children walk is protected.
        static void validate_for_duplicates(const maui::core::bindable_object& element, std::string_view route,
                                            const std::vector<const maui::core::bindable_object*>& siblings);
    };
} // namespace maui::controls
