#pragma once
// maui::controls::binding  <=  Microsoft.Maui.Controls.Binding
//
// The string-path binding: connects a target property (by registered name) to a Path resolved
// against the target's BindingContext — or an explicit Source object, or a relative_binding_source —
// through the core binding_expression engine, with an optional i_value_converter, ConverterParameter,
// and the binding_base policy surface (Mode / StringFormat / TargetNullValue / FallbackValue).
//
// The relative-source ORCHESTRATION (C# RelativeBindingSource.Apply + BindingExpression's
// SubscribeToAncestryChanges / OnElementParentSet / OnElementBindingContextChanged) lives here: the
// ancestry chain is per-application state, so it belongs to the binding, not the shared descriptor.
// TemplatedParent is a documented stub (no control templates in the port yet): it resolves to no
// source, so the target receives FallbackValue / its default until templates land.
//
// Port notes: C#'s untyped `object Source` becomes the typed set_source overloads (a shared_ptr to
// any X — walkable when X derives bindable_object, value-only otherwise — or a relative source);
// the re-application C# marshals via the target Dispatcher runs inline (single-threaded tree, §8).

#include <any>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "maui/controls/bindings/binding_base.hpp"
#include "maui/controls/bindings/relative_binding_source.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/binding_expression.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/event.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::controls
{
    class i_value_converter;

    class binding final : public binding_base
    {
    public:
        static constexpr std::string_view self_path = "."; // C# Binding.SelfPath

        // C# `new Binding()`: a null Path that resolves as the self path at apply time.
        binding();
        // C# Binding(path, mode, converter, converterParameter, stringFormat, source): path must be
        // non-empty and non-whitespace (std::invalid_argument otherwise).
        explicit binding(std::string path, maui::core::binding_mode mode = maui::core::binding_mode::default_mode,
                         std::shared_ptr<i_value_converter> converter = nullptr, std::any converter_parameter = {},
                         std::string string_format = {});

        [[nodiscard]] const std::string& path() const
        {
            return path_;
        }
        void set_path(std::string value);

        [[nodiscard]] const std::shared_ptr<i_value_converter>& converter() const
        {
            return converter_;
        }
        void set_converter(std::shared_ptr<i_value_converter> value);

        [[nodiscard]] const std::any& converter_parameter() const
        {
            return converter_parameter_;
        }
        void set_converter_parameter(std::any value);

        // An explicit source object: walkable when X derives bindable_object; otherwise a value-only
        // source (usable by the self path / as a leaf). Pinned sources skip context re-application.
        template <class X>
            requires(!std::is_same_v<X, relative_binding_source>)
        void set_source(std::shared_ptr<X> source)
        {
            throw_if_applied();
            relative_source_.reset();
            source_node_ = maui::core::binding_source_node{};
            has_source_ = source != nullptr;
            if (source)
            {
                source_node_.boxed = source;
                if constexpr (std::is_base_of_v<maui::core::bindable_object, X>)
                {
                    source_node_.object = source.get();
                    source_node_.alive = source;
                }
            }
        }
        void set_source(std::shared_ptr<relative_binding_source> source)
        {
            throw_if_applied();
            source_node_ = maui::core::binding_source_node{};
            relative_source_ = std::move(source);
            has_source_ = relative_source_ != nullptr;
        }
        [[nodiscard]] bool has_source() const
        {
            return has_source_;
        }
        [[nodiscard]] const std::shared_ptr<relative_binding_source>& relative_source() const
        {
            return relative_source_;
        }

        [[nodiscard]] std::shared_ptr<binding_base> clone() const override;

        // ---- the internal seam ----
        void apply(const maui::core::bindable_object::binding_context_box& context, maui::core::bindable_object& target,
                   std::string_view target_property, bool from_binding_context_changed,
                   maui::core::setter_specificity specificity) override;
        void apply(bool from_target) override;
        void unapply(bool from_binding_context_changed = false) override;
        [[nodiscard]] std::any get_source_value(std::any value, maui::core::type_tag target_type) const override;
        [[nodiscard]] std::any get_target_value(std::any value, maui::core::type_tag source_type) const override;

        ~binding() override;
        binding(const binding&) = delete;
        binding(binding&&) = delete;
        binding& operator=(const binding&) = delete;
        binding& operator=(binding&&) = delete;

    private:
        [[nodiscard]] maui::core::binding_expression::policy make_policy() const;

        // ---- relative-source orchestration (C# RelativeBindingSource.Apply + ancestry subs) ----
        void apply_relative_source(element& relative_target, maui::core::bindable_object& target,
                                   std::string_view target_property, maui::core::setter_specificity specificity);
        void apply_ancestor_type_binding(element& relative_target, maui::core::bindable_object& target,
                                         std::string_view target_property, maui::core::setter_specificity specificity);
        void subscribe_to_ancestry_changes(const std::vector<element*>& chain, bool include_binding_context,
                                           bool root_is_source);
        void clear_ancestry_subscriptions(std::size_t beginning_with = 0);
        [[nodiscard]] std::ptrdiff_t find_ancestry_index(const element* candidate) const;
        void on_ancestor_parent_set(element& changed);
        void on_ancestor_binding_context_changed();

        std::string path_;
        std::unique_ptr<maui::core::binding_expression> expression_;
        std::shared_ptr<i_value_converter> converter_;
        std::any converter_parameter_;
        maui::core::binding_source_node source_node_;
        std::shared_ptr<relative_binding_source> relative_source_;
        bool has_source_ = false;

        // the application site (for ancestry-driven re-application; target held weakly per §8)
        maui::core::bindable_object* applied_target_ = nullptr;
        std::weak_ptr<void> applied_target_alive_;
        std::string applied_property_;
        maui::core::setter_specificity applied_specificity_;

        struct ancestry_subscription
        {
            element* node = nullptr;
            std::weak_ptr<void> alive;
            maui::core::connection_token parent_token = 0;
            maui::core::connection_token context_token = 0;
        };
        std::vector<ancestry_subscription> ancestry_;
    };
} // namespace maui::controls
