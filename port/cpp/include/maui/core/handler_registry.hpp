#pragma once
// maui::core::handler_registry — the explicit, RTTI-free handler factory table (PROFILE §6).
//
// MAUI discovers handlers by reflection; C++23 has none, so registration is EXPLICIT and tree-shakeable:
// register_handler<view, handler>() records a factory keyed by the view type's type_tag, and
// create_handler<view>() / create_handler(type_tag) instantiate one. This is the predictable primitive
// the §6 decision settled on; a MAUI_REGISTER_HANDLER self-registration macro (over a CMake OBJECT
// library, so the linker can't drop the registrar) may layer on top at M2 for opt-in convenience.

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
} // namespace maui::core
