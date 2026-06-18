// maui::controls::binding — the string-path binding over the core engine (binding.hpp).
// Ported from src/Controls/src/Core/Binding.cs (Apply/Unapply/GetSourceValue/GetTargetValue/Clone),
// RelativeBindingSource.cs (the Apply orchestration + ApplyAncestorTypeBinding walk), and
// BindingExpression.cs (SubscribeToAncestryChanges / OnElementParentSet / OnElementBindingContext-
// Changed — the ancestry chain is per-application state, so it lives on the binding here).
#include "maui/controls/bindings/binding.hpp"

#include <algorithm>
#include <any>
#include <cctype>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/bindings/binding_diagnostics.hpp"
#include "maui/controls/bindings/i_value_converter.hpp"
#include "maui/controls/bindings/relative_binding_source.hpp"
#include "maui/controls/element.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/binding_expression.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::controls
{
    namespace
    {
        [[nodiscard]] bool is_blank(std::string_view text)
        {
            return std::ranges::all_of(text, [](char c) { return std::isspace(static_cast<unsigned char>(c)) != 0; });
        }
    } // namespace

    binding::binding() : path_(self_path), expression_(std::make_unique<maui::core::binding_expression>(self_path))
    {
    }

    binding::binding(std::string path, maui::core::binding_mode mode, std::shared_ptr<i_value_converter> converter,
                     std::any converter_parameter, std::string string_format)
        : converter_(std::move(converter)), converter_parameter_(std::move(converter_parameter))
    {
        if (is_blank(path))
        {
            throw std::invalid_argument("binding: path cannot be an empty string");
        }
        set_mode(mode);
        if (!string_format.empty())
        {
            set_string_format(std::move(string_format));
        }
        set_path(std::move(path));
    }

    binding::~binding()
    {
        clear_ancestry_subscriptions();
    }

    void binding::set_path(std::string value)
    {
        throw_if_applied();
        expression_ =
            std::make_unique<maui::core::binding_expression>(is_blank(value) ? self_path : std::string_view{value});
        path_ = std::move(value);
    }

    void binding::set_converter(std::shared_ptr<i_value_converter> value)
    {
        throw_if_applied();
        converter_ = std::move(value);
    }

    void binding::set_converter_parameter(std::any value)
    {
        throw_if_applied();
        converter_parameter_ = std::move(value);
    }

    void binding::set_update_source_event_name(std::string value)
    {
        throw_if_applied(); // C# Binding.UpdateSourceEventName setter: ThrowIfApplied()
        update_source_event_name_ = std::move(value);
    }

    std::shared_ptr<binding_base> binding::clone() const
    {
        auto clone = std::make_shared<binding>(path_, mode());
        clone->converter_ = converter_;
        clone->converter_parameter_ = converter_parameter_;
        clone->update_source_event_name_ = update_source_event_name_; // C# Clone(): UpdateSourceEventName
        if (!string_format().empty())
        {
            clone->set_string_format(string_format());
        }
        clone->set_target_null_value(target_null_value());
        clone->set_fallback_value(fallback_value());
        clone->source_node_ = source_node_;
        clone->relative_source_ = relative_source_;
        clone->has_source_ = has_source_;
        return clone;
    }

    void binding::apply(const maui::core::bindable_object::binding_context_box& context,
                        maui::core::bindable_object& target, std::string_view target_property,
                        bool from_binding_context_changed, maui::core::setter_specificity specificity)
    {
        const bool was_applied = is_applied();
        binding_base::apply(context, target, target_property, from_binding_context_changed, specificity);

        applied_target_ = &target;
        applied_target_alive_ = target.weak_token();
        applied_property_ = std::string{target_property};
        applied_specificity_ = specificity;

        // C#: a pinned source needs no re-application when the binding context changes.
        if (has_source_ && was_applied && from_binding_context_changed)
        {
            return;
        }

        if (relative_source_)
        {
            element* relative_target = relative_source_target_override();
            if (relative_target == nullptr)
            {
                relative_target = dynamic_cast<element*>(&target);
            }
            if (relative_target == nullptr)
            {
                throw std::runtime_error("binding: cannot apply a relative binding when the target is not an element");
            }
            apply_relative_source(*relative_target, target, target_property, specificity);
        }
        else
        {
            const maui::core::binding_source_node node =
                has_source_ ? source_node_ : maui::core::binding_source_node::from_context(context);
            expression_->apply(node, target, target_property, specificity, make_policy());
        }

        // C# PlatformBindingHelpers.SetBinding: after the binding is wired, hook the named target event
        // (if any) so its raise drives the source update. Re-subscribes per apply (the prior connection,
        // held in the scoped handle, drops when reassigned) — covers a context-driven re-application.
        subscribe_update_source_event(target);
    }

    void binding::apply(bool from_target)
    {
        binding_base::apply(from_target);
        expression_->apply(from_target);
    }

    void binding::unapply(bool from_binding_context_changed)
    {
        // C#: a pinned (non-relative) source survives a context change un-applied.
        if (has_source_ && !relative_source_ && from_binding_context_changed && is_applied())
        {
            return;
        }
        binding_base::unapply(from_binding_context_changed);
        expression_->unapply();
        clear_ancestry_subscriptions();
        update_source_event_connection_.reset(); // drop the named-event hook (C# EventWrapper teardown)
    }

    void binding::subscribe_update_source_event(maui::core::bindable_object& target)
    {
        // Reassigning the scoped handle drops any prior hook (move-assign resets it). Empty name =>
        // nothing to do; C# only builds an EventWrapper when UpdateSourceEventName is non-empty.
        update_source_event_connection_.reset();
        if (update_source_event_name_.empty())
        {
            return;
        }
        // The named-event seam is element-only (register_named_event lives on element). A non-element
        // target (or a value-only source binding whose target isn't an element) has no channel to hook —
        // C#'s reflective lookup over an arbitrary platform view has no reflection-free analog here.
        auto* const element_target = dynamic_cast<element*>(&target);
        if (element_target == nullptr)
        {
            send_binding_failure("binding: UpdateSourceEventName is only honored on element targets in the "
                                 "port (no reflective event lookup); ignoring '" +
                                 update_source_event_name_ + "'");
            return;
        }
        // C# EventWrapper.OnPropertyChanged raises INPC(targetProperty), which the proxy's mode-gated
        // handler turns into a source update. The port's analog: on each raise, re-apply from the target —
        // binding_expression::apply(from_target:true) is itself mode-gated (a no-op for OneWay/OneTime),
        // so this matches C#'s "binding.Mode != OneWay" guard without re-checking it here.
        // connect_named_event returns an EMPTY connection for an unregistered name (C# logs + attaches
        // nothing); the binding then simply never pushes on that event.
        update_source_event_connection_ = element_target->connect_named_event(update_source_event_name_, [this] {
            expression_->apply(
                /*from_target=*/true);
        });
    }

    std::any binding::get_source_value(std::any value, maui::core::type_tag target_type) const
    {
        if (converter_)
        {
            value = converter_->convert(value, target_type, converter_parameter_);
        }
        return binding_base::get_source_value(std::move(value), target_type);
    }

    std::any binding::get_target_value(std::any value, maui::core::type_tag source_type) const
    {
        if (converter_)
        {
            value = converter_->convert_back(value, source_type, converter_parameter_);
        }
        return binding_base::get_target_value(std::move(value), source_type);
    }

    maui::core::binding_expression::policy binding::make_policy() const
    {
        maui::core::binding_expression::policy policy;
        policy.mode = mode();
        policy.get_source_value = [this](std::any value, maui::core::type_tag target_type) {
            return get_source_value(std::move(value), target_type);
        };
        policy.get_target_value = [this](std::any value, maui::core::type_tag source_type) {
            return get_target_value(std::move(value), source_type);
        };
        policy.fallback_value = fallback_value();
        policy.on_failure = [](const std::string& message) { send_binding_failure(message); };
        return policy;
    }

    // ---- relative sources ------------------------------------------------------------------------

    void binding::apply_relative_source(element& relative_target, maui::core::bindable_object& target,
                                        std::string_view target_property, maui::core::setter_specificity specificity)
    {
        switch (relative_source_->mode())
        {
            case relative_binding_source_mode::self:
                expression_->apply(maui::core::binding_source_node::from_element(relative_target), target,
                                   target_property, specificity, make_policy());
                return;
            case relative_binding_source_mode::templated_parent:
                // STUB until control templates land (TemplateUtilities.FindTemplatedParentAsync): no
                // templated parent exists, so resolve to no source (FallbackValue / target default).
                send_binding_failure("binding: TemplatedParent relative sources need control templates, "
                                     "which the port has not merged yet");
                expression_->apply(maui::core::binding_source_node{}, target, target_property, specificity,
                                   make_policy());
                return;
            case relative_binding_source_mode::find_ancestor:
            case relative_binding_source_mode::find_ancestor_binding_context:
                apply_ancestor_type_binding(relative_target, target, target_property, specificity);
                return;
        }
    }

    void binding::apply_ancestor_type_binding(element& relative_target, maui::core::bindable_object& target,
                                              std::string_view target_property,
                                              maui::core::setter_specificity specificity)
    {
        // RelativeBindingSource.ApplyAncestorTypeBinding, iteratively: walk up Element.Parent
        // recording the chain; the first ancestor that fits the type/level resolves the source.
        const bool by_context = relative_source_->mode() == relative_binding_source_mode::find_ancestor_binding_context;
        clear_ancestry_subscriptions();

        std::vector<element*> chain{&relative_target};
        int level = 0;
        const void* last_matching_context = nullptr;
        element const* current = &relative_target;
        while (true)
        {
            element* parent = current->logical_parent();
            if (parent == nullptr)
            {
                // No fitting ancestor (yet): resolve null for now and watch the whole chain — an
                // ancestor added later re-runs this walk (rootIsSource: false).
                expression_->apply(maui::core::binding_source_node{}, target, target_property, specificity,
                                   make_policy());
                subscribe_to_ancestry_changes(chain, by_context, /*root_is_source=*/false);
                return;
            }
            chain.push_back(parent);

            // ElementFitsAncestorTypeAndLevel (the by-context arm dedupes an INHERITED same context).
            bool fits = false;
            if (by_context)
            {
                const auto& box = parent->raw_binding_context();
                if (box.value && relative_source_->matches_context(box))
                {
                    if (last_matching_context != box.value.get())
                    {
                        last_matching_context = box.value.get();
                        ++level;
                    }
                    fits = level >= relative_source_->ancestor_level();
                }
            }
            else if (relative_source_->matches_ancestor(*parent))
            {
                ++level;
                fits = level >= relative_source_->ancestor_level();
            }

            if (fits)
            {
                const maui::core::binding_source_node node =
                    by_context ? maui::core::binding_source_node::from_context(parent->raw_binding_context())
                               : maui::core::binding_source_node::from_element(*parent);
                expression_->apply(node, target, target_property, specificity, make_policy());
                subscribe_to_ancestry_changes(chain, by_context, /*root_is_source=*/true);
                return;
            }
            current = parent;
        }
    }

    void binding::subscribe_to_ancestry_changes(const std::vector<element*>& chain, bool include_binding_context,
                                                bool root_is_source)
    {
        clear_ancestry_subscriptions();
        ancestry_.reserve(chain.size());
        for (std::size_t i = 0; i < chain.size(); ++i)
        {
            element* node = chain[i];
            ancestry_subscription subscription;
            subscription.node = node;
            subscription.alive = node->weak_token();
            if (i != chain.size() - 1 || !root_is_source)
            {
                // a successfully resolved source's own parents don't matter (C# comment)
                subscription.parent_token = node->parent_set.connect([this, node] { on_ancestor_parent_set(*node); });
            }
            if (include_binding_context)
            {
                subscription.context_token =
                    node->binding_context_changed.connect([this] { on_ancestor_binding_context_changed(); });
            }
            ancestry_.push_back(std::move(subscription));
        }
    }

    void binding::clear_ancestry_subscriptions(std::size_t beginning_with) noexcept
    {
        for (std::size_t i = beginning_with; i < ancestry_.size(); ++i)
        {
            ancestry_subscription const& subscription = ancestry_[i];
            if (subscription.node != nullptr && !subscription.alive.expired())
            {
                if (subscription.parent_token != 0)
                {
                    subscription.node->parent_set.disconnect(subscription.parent_token);
                }
                if (subscription.context_token != 0)
                {
                    subscription.node->binding_context_changed.disconnect(subscription.context_token);
                }
            }
        }
        // erase, not resize: resize's visible grow branch reads as a may-throw to the
        // exception-escape analysis; shrinking only destroys (noexcept).
        ancestry_.erase(ancestry_.begin() + static_cast<std::ptrdiff_t>(beginning_with), ancestry_.end());
    }

    std::ptrdiff_t binding::find_ancestry_index(const element* candidate) const
    {
        for (std::size_t i = 0; i < ancestry_.size(); ++i)
        {
            if (ancestry_[i].alive.expired())
            {
                return -1; // the chain is no longer valid (C# FindAncestryIndex)
            }
            if (ancestry_[i].node == candidate)
            {
                return static_cast<std::ptrdiff_t>(i);
            }
        }
        return -1;
    }

    void binding::on_ancestor_parent_set(element& changed)
    {
        // BindingExpression.OnElementParentSet.
        if (applied_target_ == nullptr || applied_target_alive_.expired())
        {
            return;
        }
        maui::core::bindable_object& target = *applied_target_;
        if (changed.logical_parent() == nullptr)
        {
            const std::ptrdiff_t index = find_ancestry_index(&changed);
            if (index == -1)
            {
                unapply();
                return;
            }
            clear_ancestry_subscriptions(static_cast<std::size_t>(index) + 1);
            // Force the expression to resolve null until someone in the chain gets a new parent.
            expression_->apply(maui::core::binding_source_node{}, target, applied_property_, applied_specificity_,
                               make_policy());
        }
        else
        {
            unapply();
            apply(maui::core::bindable_object::binding_context_box{}, target, applied_property_, false,
                  applied_specificity_);
        }
    }

    void binding::on_ancestor_binding_context_changed()
    {
        // BindingExpression.OnElementBindingContextChanged (the repeat-notice optimization — skipping
        // when the resolved source is unchanged — is perf-only and not ported; re-applying is
        // idempotent).
        if (applied_target_ == nullptr || applied_target_alive_.expired())
        {
            return;
        }
        maui::core::bindable_object& target = *applied_target_;
        unapply();
        apply(maui::core::bindable_object::binding_context_box{}, target, applied_property_, false,
              applied_specificity_);
    }
} // namespace maui::controls
