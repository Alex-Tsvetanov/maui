#pragma once
// maui::hosting::i_maui_handlers_collection  <=  Microsoft.Maui.Hosting.IMauiHandlersCollection
//   (+ Microsoft.Maui.Hosting.MauiHandlersCollectionExtensions.AddHandler / TryAddHandler)
//
// The registration face the builder hands to ConfigureMauiHandlers delegates. In C# the interface is an
// empty marker over IMauiServiceCollection and the typed AddHandler/TryAddHandler surface arrives as
// extension methods; the C++ facade keeps that shape with two erased virtual primitives (register /
// register-if-absent, factory-based — exactly what the underlying registry stores) and the typed
// template sugar layered on top as non-virtual members (the §5 extension-method idiom). The facade
// LAYERS over the existing maui::core::handler_registry (PROFILE §6) — the registry stays the single
// factory table hosting resolves handlers from; this type only writes into it.

#include <memory>
#include <type_traits>
#include <utility>

#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::hosting
{
    class i_maui_handlers_collection
    {
    public:
        virtual ~i_maui_handlers_collection() = default;

        // ---- The erased primitives (the IMauiServiceCollection face) ----
        // Register (or replace) the factory servicing view_type.
        virtual void add_handler(maui::core::type_tag view_type, maui::core::handler_registry::factory factory) = 0;
        // Register only when nothing services view_type yet; true when the registration was added.
        virtual bool try_add_handler(maui::core::type_tag view_type, maui::core::handler_registry::factory factory) = 0;
        [[nodiscard]] virtual bool is_registered(maui::core::type_tag view_type) const = 0;

        // ---- The typed sugar (MauiHandlersCollectionExtensions) ----
        // AddHandler<TType, TTypeRender>: register (or replace) Handler as the handler servicing View.
        template <class View, class Handler> i_maui_handlers_collection& add_handler()
        {
            add_handler(maui::core::type_tag::of<View>(), make_factory<Handler>());
            return *this;
        }
        // AddHandler<TType>(implementationFactory): register (or replace) by factory.
        template <class View> i_maui_handlers_collection& add_handler(maui::core::handler_registry::factory factory)
        {
            add_handler(maui::core::type_tag::of<View>(), std::move(factory));
            return *this;
        }
        // TryAddHandler<TType, TTypeRender>: register only when nothing services View yet.
        template <class View, class Handler> i_maui_handlers_collection& try_add_handler()
        {
            try_add_handler(maui::core::type_tag::of<View>(), make_factory<Handler>());
            return *this;
        }
        template <class View> [[nodiscard]] bool is_registered() const
        {
            return is_registered(maui::core::type_tag::of<View>());
        }

    protected:
        i_maui_handlers_collection() = default;
        i_maui_handlers_collection(const i_maui_handlers_collection&) = default;
        i_maui_handlers_collection(i_maui_handlers_collection&&) = default;
        i_maui_handlers_collection& operator=(const i_maui_handlers_collection&) = default;
        i_maui_handlers_collection& operator=(i_maui_handlers_collection&&) = default;

    private:
        // The default-construct factory the typed registrations share (handler_registry's own idiom).
        template <class Handler> [[nodiscard]] static maui::core::handler_registry::factory make_factory()
        {
            static_assert(std::is_base_of_v<maui::core::i_element_handler, Handler>,
                          "Handler must derive maui::core::i_element_handler");
            return [] { return std::unique_ptr<maui::core::i_element_handler>(std::make_unique<Handler>()); };
        }
    };
} // namespace maui::hosting
