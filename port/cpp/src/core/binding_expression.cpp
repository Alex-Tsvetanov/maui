// maui::core::binding_expression — the string-path binding engine (binding_expression.hpp).
// Ported from src/Controls/src/Core/BindingExpression.cs: ApplyCore's walk + per-hop subscription
// (WeakPropertyChangedProxy becomes the §8 token + weak-liveness pattern) + the failure/fallback
// ladder (FallbackValue ?? target default; TargetNullValue handled by the policy's GetSourceValue).
#include "maui/core/binding_expression.hpp"

#include <any>
#include <cstddef>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "maui/core/bindable_object.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/i_indexable.hpp"
#include "maui/core/property_path.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::core
{
    binding_expression::binding_expression(std::string_view path) : path_(property_path::parse(path))
    {
        parts_.reserve(path_.parts().size());
        for (const property_path::part& part : path_.parts())
        {
            runtime_part runtime;
            runtime.spec = part;
            runtime.indexer_name = "Item"; // the DefaultMemberAttribute default, refined at walk time
            parts_.push_back(std::move(runtime));
        }
    }

    binding_expression::~binding_expression()
    {
        unapply();
    }

    void binding_expression::apply(const binding_source_node& source, bindable_object& target,
                                   std::string_view target_property, setter_specificity specificity,
                                   policy apply_policy)
    {
        // C#: "Binding instances cannot be reused" — applying while already applied to a different
        // target or source (an unapply() in between re-arms the expression).
        if (applied_ && target_ != nullptr && target_ != &target)
        {
            throw std::runtime_error("binding_expression: binding instances cannot be reused");
        }
        if (applied_ && source_.object != nullptr && source.object != nullptr && source_.object != source.object)
        {
            throw std::runtime_error("binding_expression: binding instances cannot be reused");
        }

        target_ = &target;
        target_alive_ = target.weak_token();
        target_property_ = std::string{target_property};
        specificity_ = specificity;
        policy_ = std::move(apply_policy);
        source_ = source;
        applied_ = true;

        apply_core(source_, /*from_target=*/false);
    }

    void binding_expression::apply(bool from_target)
    {
        if (!applied_)
        {
            return;
        }
        if (!target_alive())
        {
            unapply(); // C#: a dead target tears the expression down
            return;
        }
        if (source_.object != nullptr && source_.alive.expired())
        {
            return; // C#: WeakReference.TryGetTarget failed — nothing to re-resolve against
        }
        apply_core(source_, from_target);
    }

    void binding_expression::unapply()
    {
        for (runtime_part& part : parts_)
        {
            unsubscribe_part(part);
        }
        applied_ = false;
        target_ = nullptr;
        target_alive_.reset();
        source_ = binding_source_node{};
    }

    binding_mode binding_expression::realized_mode() const
    {
        // BindingBaseExtensions.GetRealizedMode: Default resolves to the target property's default
        // mode; TwoWay downgrades to OneWayToSource on a read-only target.
        binding_mode mode = policy_.mode;
        if (mode == binding_mode::default_mode)
        {
            mode = target_->property_default_binding_mode(target_property_).value_or(binding_mode::one_way);
        }
        if (mode == binding_mode::default_mode)
        {
            mode = binding_mode::one_way;
        }
        if (mode == binding_mode::two_way && target_->property_is_read_only(target_property_).value_or(false))
        {
            mode = binding_mode::one_way_to_source;
        }
        return mode;
    }

    void binding_expression::apply_core(const binding_source_node& source, bool from_target)
    {
        bindable_object* target = target_; // alive — guaranteed by both apply() entry points
        const binding_mode mode = realized_mode();
        if ((mode == binding_mode::one_way || mode == binding_mode::one_time) && from_target)
        {
            return;
        }

        const bool needs_getter = (mode == binding_mode::two_way && !from_target) || mode == binding_mode::one_way ||
                                  mode == binding_mode::one_time;
        const bool needs_setter = !needs_getter && ((mode == binding_mode::two_way && from_target) ||
                                                    mode == binding_mode::one_way_to_source);
        const bool wants_subscriptions = mode == binding_mode::one_way || mode == binding_mode::two_way;

        // The walk (C# ApplyCore's part loop): step `current` down the chain, subscribing each hop
        // that has a next part. `hold` keeps the freshly resolved hop alive across the iteration.
        binding_source_node current = source;
        std::shared_ptr<bindable_object> hold;
        const std::size_t count = parts_.size();
        bool resolution_failed = false;
        for (std::size_t i = 0; i < count; ++i)
        {
            runtime_part& part = parts_[i];
            const bool last = i == count - 1;

            if (!part.spec.is_self && current.object != nullptr)
            {
                if (part.spec.is_indexer)
                {
                    auto* indexable = dynamic_cast<i_indexable*>(current.object);
                    if (indexable != nullptr)
                    {
                        part.indexer_name = std::string{indexable->indexer_name()};
                    }
                    else if (needs_getter || (needs_setter && last))
                    {
                        // C# PropertyNotFoundErrorMessage (the indexer property is missing).
                        fail("'[" + part.spec.content + "]' indexer not found on source, target property: '" +
                             target_property_ + "'");
                        resolution_failed = true;
                        break;
                    }
                    if (!last)
                    {
                        std::shared_ptr<bindable_object> next =
                            indexable != nullptr ? indexable->try_get_item_object(part.spec.content) : nullptr;
                        hold = next;
                        current.object = next.get();
                        current.alive = next;
                        current.boxed = std::any{};
                    }
                }
                else
                {
                    const bool exists = current.object->has_property(part.spec.content);
                    if (!exists && (needs_getter || (needs_setter && last)))
                    {
                        fail("'" + part.spec.content + "' property not found on source, target property: '" +
                             target_property_ + "'");
                        resolution_failed = true;
                        break;
                    }
                    if (!last)
                    {
                        std::shared_ptr<bindable_object> next =
                            exists ? current.object->try_get_object(part.spec.content) : nullptr;
                        hold = next;
                        current.object = next.get();
                        current.alive = next;
                        current.boxed = std::any{};
                    }
                }
            }

            // C#: `if (part.NextPart != null && mode is OneWay/TwoWay && current is INPC) Subscribe`
            // — part i listens on the node its value resolved to, filtered by part i+1's name.
            if (!last)
            {
                if (wants_subscriptions && current.object != nullptr)
                {
                    subscribe_part(i, *current.object, current.alive);
                }
                else
                {
                    unsubscribe_part(part); // a hop that no longer resolves drops its subscription
                }
            }
        }

        const runtime_part& last_part = parts_.back();
        if (needs_getter)
        {
            // C#: `if (part.TryGetValue(current, out value) || part.IsSelf) value = GetSourceValue(...)
            //      else value = FallbackValue ?? property.GetDefaultValue(target)`.
            std::optional<std::any> value;
            if (!resolution_failed)
            {
                if (last_part.spec.is_self)
                {
                    value = current.boxed; // resolved even when null — TargetNullValue applies
                }
                else if (current.object != nullptr)
                {
                    if (last_part.spec.is_indexer)
                    {
                        if (const auto* indexable = dynamic_cast<const i_indexable*>(current.object))
                        {
                            value = indexable->try_get_item(last_part.spec.content);
                        }
                    }
                    else
                    {
                        value = current.object->try_get_value(last_part.spec.content);
                    }
                }
            }

            std::any result;
            if (value.has_value())
            {
                const type_tag target_type = target->property_type(target_property_).value_or(type_tag::of<void>());
                result = policy_.get_source_value ? policy_.get_source_value(std::move(*value), target_type)
                                                  : std::move(*value);
            }
            else if (policy_.fallback_value.has_value())
            {
                result = policy_.fallback_value;
            }
            else
            {
                result = target->property_default_value(target_property_).value_or(std::any{});
            }

            if (!target->try_set_value(target_property_, result, specificity_))
            {
                fail("value cannot be converted to the type of target property '" + target_property_ + "'");
            }
        }
        else if (needs_setter && !resolution_failed && !last_part.spec.is_self && current.object != nullptr)
        {
            // C#: value = GetTargetValue(target.GetValue(property), part.SetterType), then the leaf
            // setter (the source property's converting setter / the indexer's try_set_item).
            std::any target_value = target->try_get_value(target_property_).value_or(std::any{});
            type_tag setter_type = type_tag::of<void>();
            if (!last_part.spec.is_indexer)
            {
                setter_type = current.object->property_type(last_part.spec.content).value_or(type_tag::of<void>());
            }
            const std::any value = policy_.get_target_value
                                       ? policy_.get_target_value(std::move(target_value), setter_type)
                                       : std::move(target_value);
            bool stored = false;
            if (last_part.spec.is_indexer)
            {
                if (auto* indexable = dynamic_cast<i_indexable*>(current.object))
                {
                    stored = indexable->try_set_item(last_part.spec.content, value);
                }
            }
            else
            {
                stored = current.object->try_set_value(last_part.spec.content, value,
                                                       setter_specificity::manual_value_setter);
            }
            if (!stored)
            {
                fail("value cannot be converted back to source property '" + last_part.spec.content + "'");
            }
        }
    }

    void binding_expression::subscribe_part(std::size_t index, bindable_object& node, std::weak_ptr<void> alive)
    {
        runtime_part& part = parts_[index];
        if (part.subscribed == &node && !part.alive.expired())
        {
            return; // already listening on this node (C# part.Subscribe's ReferenceEquals check)
        }
        unsubscribe_part(part);
        part.subscribed = &node;
        part.alive = std::move(alive);
        part.changed_token = node.property_changed.connect([this, index](std::string_view name) {
            // C# BindingExpressionPart.PropertyChanged: an empty name refreshes everything.
            if (!name.empty() && !name_matches(index, name))
            {
                return;
            }
            apply(/*from_target=*/false);
        });
        // C# WeakPropertyChangedProxy also watches BindingContextChanged on BindableObject sources,
        // surfacing it as a change of the "BindingContext" property (paths like "binding_context.x").
        part.context_token = node.binding_context_changed.connect([this, index] {
            if (!name_matches(index, "binding_context"))
            {
                return;
            }
            apply(/*from_target=*/false);
        });
    }

    void binding_expression::unsubscribe_part(runtime_part& part)
    {
        // §8 token teardown: only a LIVE node still owns its events — a dead one already dropped them.
        if (part.subscribed != nullptr && !part.alive.expired())
        {
            part.subscribed->property_changed.disconnect(part.changed_token);
            part.subscribed->binding_context_changed.disconnect(part.context_token);
        }
        part.subscribed = nullptr;
        part.alive.reset();
        part.changed_token = 0;
        part.context_token = 0;
    }

    bool binding_expression::name_matches(std::size_t index, std::string_view name) const
    {
        // C#: `BindingExpressionPart part = NextPart ?? this;` — the listener installed at hop i
        // filters by the part the changed property feeds (i + 1).
        const std::size_t match_index = index + 1 < parts_.size() ? index + 1 : index;
        const runtime_part& match = parts_[match_index];
        if (match.spec.is_indexer)
        {
            if (name.find('[') != std::string_view::npos)
            {
                return name == match.indexer_name + "[" + match.spec.content + "]";
            }
            return name == match.indexer_name;
        }
        return name == match.spec.content;
    }

    void binding_expression::fail(const std::string& message) const
    {
        if (policy_.on_failure)
        {
            policy_.on_failure(message);
        }
    }

    bool binding_expression::target_alive() const
    {
        return target_ != nullptr && !target_alive_.expired();
    }
} // namespace maui::core
