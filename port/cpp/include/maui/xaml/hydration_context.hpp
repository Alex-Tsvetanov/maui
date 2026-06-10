#pragma once
// maui::xaml::hydration_context  <=  Microsoft.Maui.Controls.Xaml.HydrationContext
//
// The shared state of one loader pass (src/Controls/src/Xaml/HydrationContext.cs), threaded through
// every visitor:
//   - values  <=  HydrationContext.Values (Dictionary<INode, object>): node → hydrated object. The
//     C# object payload maps onto std::any — a created control is boxed as
//     shared_ptr<maui::core::bindable_object>, a value node's text as std::string, a minted markup
//     extension as shared_ptr<i_markup_extension>, a <ResourceDictionary> as
//     shared_ptr<maui::controls::resource_dictionary>.
//   - types  <=  HydrationContext.Types (Dictionary<ElementNode, Type>): node → the created object's
//     type_tag (keys the xaml_property_registry; C# stores System.Type).
//   - parent_context  <=  HydrationContext.ParentContext — the enclosing load's context when a
//     template body is inflated. Carried for fidelity; unused until templates land (M7+).
//   - exception_handler  <=  HydrationContext.ExceptionHandler (Action<Exception>): the doNotThrow
//     knob. Null (the default) → handle() THROWS the error; set → handle() invokes it and returns,
//     and the visitor skips the failed node and continues. The port narrows Action<Exception> to the
//     XAML error type — every loader-raised error is a xaml_parse_exception (the single M7 channel).
//   - root_element  <=  HydrationContext.RootElement: the object XAML inflates into (load_into's
//     caller-owned root, or the root the create pass minted). Non-owning. (RootAssembly is
//     reflection-only — not ported.)
//
// PORT-SPECIFIC OWNERSHIP (no GC): C#'s Values dictionary keeps every created object alive until the
// parent→child references take over. Here the tree-wiring APIs are non-owning (PROFILE §8), so the
// context also carries the OWNERS the load accumulates — the xaml_object_graph of created controls
// plus the keep-alive list for non-control objects (resource dictionaries) — which the loader moves
// into the returned xaml_load_result when the pass completes.
//
// PORT-SPECIFIC REGISTRIES (no reflection): C# visitors reach types/properties/converters through
// reflection; the port's visitors resolve against the explicit registries (PROFILE §6), so the
// context carries non-owning references to the four of them.

#include <any>
#include <functional>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "maui/core/bindable_object.hpp"
#include "maui/core/type_tag.hpp"
#include "maui/xaml/i_markup_extension.hpp"
#include "maui/xaml/xaml_converter_registry.hpp"
#include "maui/xaml/xaml_node.hpp"
#include "maui/xaml/xaml_object_graph.hpp"
#include "maui/xaml/xaml_parse_exception.hpp"
#include "maui/xaml/xaml_property_registry.hpp"
#include "maui/xaml/xaml_type_registry.hpp"

namespace maui::xaml
{
    class hydration_context
    {
    public:
        // HydrationContext.ExceptionHandler — see the header note for the throw-vs-collect contract.
        using exception_handler = std::function<void(const xaml_parse_exception&)>;

        hydration_context(const xaml_type_registry& types, const xaml_property_registry& properties,
                          const xaml_converter_registry& converters, const markup_extension_registry& extensions,
                          exception_handler handler = nullptr)
            : type_registry_(&types), property_registry_(&properties), converter_registry_(&converters),
              extension_registry_(&extensions), handler_(std::move(handler))
        {
        }

        // ---- Values (node → hydrated object) ----
        void set_value(const i_xaml_node& node, std::any value)
        {
            values_.insert_or_assign(&node, std::move(value));
        }
        [[nodiscard]] const std::any* try_get_value(const i_xaml_node& node) const
        {
            const auto found = values_.find(&node);
            return found != values_.end() ? &found->second : nullptr;
        }

        // ---- Types (element node → created type_tag) ----
        void set_type(const element_node& node, maui::core::type_tag type)
        {
            types_.insert_or_assign(&node, type);
        }
        [[nodiscard]] const maui::core::type_tag* try_get_type(const element_node& node) const
        {
            const auto found = types_.find(&node);
            return found != types_.end() ? &found->second : nullptr;
        }

        // ---- the error knob ----
        // The visitors' single raise site: invokes the handler and RETURNS when one is set (the C#
        // `if (Context.ExceptionHandler != null) { Context.ExceptionHandler(e); return; }` idiom —
        // the caller returns right after), THROWS the error otherwise.
        void handle(const xaml_parse_exception& error) const;
        [[nodiscard]] bool has_handler() const
        {
            return static_cast<bool>(handler_);
        }

        // ---- RootElement ----
        [[nodiscard]] maui::core::bindable_object* root_element() const
        {
            return root_element_;
        }
        void set_root_element(maui::core::bindable_object* value)
        {
            root_element_ = value;
        }

        // ---- ParentContext (templates — carried, unused until they land) ----
        hydration_context* parent_context = nullptr;

        // ---- ownership accumulators (port-specific; moved into the load result) ----
        [[nodiscard]] xaml_object_graph& graph()
        {
            return graph_;
        }
        // Keep a non-control object (a resource_dictionary, …) alive for the loaded tree's lifetime.
        void keep_alive(std::shared_ptr<void> object)
        {
            keep_alive_.push_back(std::move(object));
        }
        [[nodiscard]] std::vector<std::shared_ptr<void>>& kept_alive()
        {
            return keep_alive_;
        }

        // ---- the explicit registries (port-specific; non-owning) ----
        [[nodiscard]] const xaml_type_registry& type_registry() const
        {
            return *type_registry_;
        }
        [[nodiscard]] const xaml_property_registry& property_registry() const
        {
            return *property_registry_;
        }
        [[nodiscard]] const xaml_converter_registry& converter_registry() const
        {
            return *converter_registry_;
        }
        [[nodiscard]] const markup_extension_registry& extension_registry() const
        {
            return *extension_registry_;
        }

    private:
        std::unordered_map<const i_xaml_node*, std::any> values_;
        std::unordered_map<const element_node*, maui::core::type_tag> types_;
        const xaml_type_registry* type_registry_;
        const xaml_property_registry* property_registry_;
        const xaml_converter_registry* converter_registry_;
        const markup_extension_registry* extension_registry_;
        exception_handler handler_;
        maui::core::bindable_object* root_element_ = nullptr;
        xaml_object_graph graph_;
        std::vector<std::shared_ptr<void>> keep_alive_;
    };
} // namespace maui::xaml
