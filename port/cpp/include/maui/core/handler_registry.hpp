#pragma once
// maui::core::handler_registry — the explicit, RTTI-free handler factory table (PROFILE §6).
//
// MAUI discovers handlers by reflection; C++23 has none, so registration is EXPLICIT and tree-shakeable:
// register_handler<view, handler>() records a factory keyed by the view type's type_tag, and
// create_handler<view>() / create_handler(type_tag) instantiate one. That is the predictable primitive
// the §6 decision settled on. Layered on top (also §6) is opt-in SELF-registration: a process-wide
// default_handler_registry(), a macro-free handler_registrar<view, handler> static, and the
// MAUI_REGISTER_HANDLER(view, handler) sugar — see below.

#include <functional>
#include <memory>
#include <type_traits>
#include <unordered_map>

#include "maui/core/i_element_handler.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::core
{
    class handler_registry
    {
    public:
        using factory = std::function<std::unique_ptr<i_element_handler>()>;

        // Register (or replace) the handler type that services View. Handler must be default-
        // constructible (its default ctor wires its static mappers — see view_handler).
        template <class View, class Handler> void register_handler()
        {
            static_assert(std::is_base_of_v<i_element_handler, Handler>,
                          "Handler must derive maui::core::i_element_handler");
            factories_[type_tag::of<View>()] = [] {
                return std::unique_ptr<i_element_handler>(std::make_unique<Handler>());
            };
        }

        template <class View> [[nodiscard]] std::unique_ptr<i_element_handler> create_handler() const
        {
            return create_handler(type_tag::of<View>());
        }

        // Instantiate the handler registered for view_type, or nullptr if none.
        [[nodiscard]] std::unique_ptr<i_element_handler> create_handler(type_tag view_type) const;
        [[nodiscard]] bool is_registered(type_tag view_type) const;

    private:
        std::unordered_map<type_tag, factory> factories_;
    };

    // PROFILE §6 idiom: register_handler<view, handler>(registry).
    template <class View, class Handler> void register_handler(handler_registry& registry)
    {
        registry.register_handler<View, Handler>();
    }

    // A process-wide default registry that the self-registration helpers populate, and that hosting
    // resolves from when no explicit registry is threaded through. Meyers singleton — no static-init-
    // order fiasco even when registrars run during dynamic initialization.
    [[nodiscard]] handler_registry& default_handler_registry();

    // Macro-free self-registration primitive: a `static const handler_registrar<view, handler>` at
    // namespace scope registers the pair in the default registry at load time, via its constructor.
    //
    // Tree-shaking caveat (PROFILE §6): a static library drops an object file that nothing references,
    // taking its registrar with it. Put self-registering translation units in a CMake OBJECT library
    // (target_link_libraries(app PRIVATE handlers_obj)) or link them with --whole-archive.
    template <class View, class Handler> struct handler_registrar
    {
        // noexcept: this runs during dynamic initialization (bugprone-throwing-static-initialization);
        // the only failure mode is OOM on map insertion, where terminating at load time is acceptable.
        handler_registrar() noexcept
        {
            default_handler_registry().register_handler<View, Handler>();
        }
    };
} // namespace maui::core

// MAUI_REGISTER_HANDLER(view, handler): one-line opt-in self-registration, sugar over handler_registrar
// (same OBJECT-library caveat). MAUI_-prefixed macros are allow-listed in the project's .clang-tidy.
#define MAUI_DETAIL_CONCAT_IMPL(a, b) a##b
#define MAUI_DETAIL_CONCAT(a, b) MAUI_DETAIL_CONCAT_IMPL(a, b)
#define MAUI_REGISTER_HANDLER(ViewType, HandlerType)                                                                   \
    namespace                                                                                                          \
    {                                                                                                                  \
        const ::maui::core::handler_registrar<ViewType, HandlerType> MAUI_DETAIL_CONCAT(maui_handler_registrar_,       \
                                                                                        __LINE__){};                   \
    }
