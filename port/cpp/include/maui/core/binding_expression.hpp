#pragma once
// maui::core::binding_expression  <=  Microsoft.Maui.Controls.BindingExpression (internal)
//
// The string-path binding ENGINE: walks a parsed property_path along a chain of bindable_objects
// (via the name->getter channel) / i_indexable hops, subscribes to property_changed at every hop
// that has a next part, and re-resolves the whole chain when an intermediate value changes — the
// heart of C#'s BindingExpression.cs. The policy half of a binding (mode, converter, StringFormat,
// TargetNullValue/FallbackValue) stays in maui::controls (binding/multi_binding); this engine takes
// it type-erased through `policy`, exactly the BindingBase virtual seam (GetSourceValue /
// GetTargetValue / GetRealizedMode).
//
// Differences from C# (documented, all reflection consequences):
//   - sources are bindable_objects (the port's INotifyPropertyChanged analog); a plain value can
//     only sit at a path LEAF (the self part) through its boxed form;
//   - C#'s WeakPropertyChangedProxy becomes the §8 token pattern: each hop subscription stores a
//     connection token plus the node's weak liveness, and disconnects only while the node is alive
//     (a dead node took its event down with it). The engine never owns a source.
//   - C# marshals re-application through the target's Dispatcher; the port applies inline (the
//     visual tree is single-threaded per §8).
//
// Ownership: the expression is owned by its binding (which the TARGET element owns). It holds the
// target as a raw pointer + weak liveness token and source hops as raw pointers + weak tokens.

#include <any>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "maui/core/bindable_object.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/event.hpp"
#include "maui/core/property_path.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::core
{
    // The engine's type-erased source object (the C# `object sourceObject`): a boxed leaf value, a
    // walkable node form, and the node's liveness. Built from a binding context box, an explicit
    // source object, or a relative-source element.
    struct binding_source_node
    {
        std::any boxed;                    // the value form (self-path leaf); empty = null
        bindable_object* object = nullptr; // the walkable form (property/indexer hops)
        std::weak_ptr<void> alive;         // liveness of `object` (meaningful only when object != null)

        [[nodiscard]] static binding_source_node from_context(const bindable_object::binding_context_box& box)
        {
            binding_source_node node;
            node.boxed = box.boxed;
            node.object = box.object.get();
            node.alive = box.object;
            return node;
        }
        [[nodiscard]] static binding_source_node from_element(bindable_object& object)
        {
            binding_source_node node;
            node.object = &object;
            node.alive = object.weak_token();
            return node;
        }
    };

    class binding_expression
    {
    public:
        // The BindingBase policy seam the owning binding supplies per apply.
        struct policy
        {
            binding_mode mode = binding_mode::default_mode; // unrealized; resolved against the target
            // Binding.GetSourceValue: converter.Convert + TargetNullValue + StringFormat.
            std::function<std::any(std::any value, type_tag target_type)> get_source_value;
            // Binding.GetTargetValue: converter.ConvertBack.
            std::function<std::any(std::any value, type_tag source_type)> get_target_value;
            std::any fallback_value;                                    // BindingBase.FallbackValue (empty = none)
            std::function<void(const std::string& message)> on_failure; // BindingDiagnostics seam
        };

        // ParsePath happens at construction (throws std::invalid_argument on malformed paths).
        explicit binding_expression(std::string_view path);
        binding_expression(const binding_expression&) = delete;
        binding_expression(binding_expression&&) = delete;
        binding_expression& operator=(const binding_expression&) = delete;
        binding_expression& operator=(binding_expression&&) = delete;
        ~binding_expression();

        // Apply to a new source and target (C# Apply(object, BindableObject, BindableProperty,
        // SetterSpecificity)). Throws std::runtime_error if re-applied to a different target or a
        // different source without an unapply() in between ("Binding instances cannot be reused").
        void apply(const binding_source_node& source, bindable_object& target, std::string_view target_property,
                   setter_specificity specificity, policy apply_policy);

        // Re-apply on the previously set source/target (C# Apply(fromTarget)). A no-op when unapplied.
        void apply(bool from_target = false);

        // Tear down: unsubscribe every hop and forget source/target (C# Unapply).
        void unapply();

        [[nodiscard]] const std::string& path() const
        {
            return path_.text();
        }
        [[nodiscard]] bool is_applied() const
        {
            return applied_;
        }

    private:
        struct runtime_part
        {
            property_path::part spec;
            std::string indexer_name;              // resolved i_indexable::indexer_name at walk time
            bindable_object* subscribed = nullptr; // the node this part listens on (raw; see alive)
            std::weak_ptr<void> alive;
            connection_token changed_token = 0;
            connection_token context_token = 0;
        };

        void apply_core(const binding_source_node& source, bool from_target);
        [[nodiscard]] binding_mode realized_mode() const;
        void subscribe_part(std::size_t index, bindable_object& node, std::weak_ptr<void> alive);
        static void unsubscribe_part(runtime_part& part);
        // The part listener's name filter (C# BindingExpressionPart.PropertyChanged): matches against
        // the NEXT part (or this one), with the "<indexer_name>[<content>]" shape for indexer parts.
        [[nodiscard]] bool name_matches(std::size_t index, std::string_view name) const;
        void fail(const std::string& message) const;
        [[nodiscard]] bool target_alive() const;

        property_path path_;
        std::vector<runtime_part> parts_;
        policy policy_;
        bindable_object* target_ = nullptr;
        std::weak_ptr<void> target_alive_;
        std::string target_property_;
        setter_specificity specificity_;
        binding_source_node source_;
        bool applied_ = false;
    };
} // namespace maui::core
