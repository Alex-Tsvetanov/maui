#pragma once
// maui::controls::style  <=  Microsoft.Maui.Controls.Style
//
// A reusable bundle of property setters (with a target type) that can be applied to a bindable_object at
// a given specificity — the code-first equivalent of a XAML <Style>. apply() pushes each setter's value
// through bindable_object::apply_setter (so it lands in the typed property<T> at the style specificity);
// unapply() clears those values. A based_on style is applied first at the lowered (base-style)
// specificity, so the derived style's own setters win — mirroring Style.ApplyCore / SetterSpecificity
// .AsBaseStyle. Ported from Style.cs (the IStyle.Apply/UnApply core).
//
// BasedOn can be supplied two ways (Style.BasedOn vs Style.BaseResourceKey): a directly-assigned based_on
// style, OR a base_resource_key resolved from the target's resource chain. apply()/unapply() take an
// optional resolver (a key → shared_ptr<style> lookup, supplied by merged_style when a style is attached
// to an element) so a key-based base style is resolved at apply time. style_class (Style.Class) selects
// the style when a control's style_class names it; an implicit style omits the class and is keyed by
// target_type (resource_dictionary::add(style)).
//
// Scope (M5d): target_type + setters + based_on (direct or by-key) + style_class. W1-15 adds
// ApplyToDerivedTypes: an implicit or class style whose target type is a BASE type applies to derived
// controls when the flag is set — matched against the control's DECLARED base-type chain
// (element::set_style_target_type<TControl, TBases...>, the reflection-free substitute for C#'s
// Type.BaseType walk in Style.CanBeAppliedTo / MergedStyle.RegisterImplicitStyles). Behaviors/Triggers
// attached to a Style, CanCascade, and the duplicate-key warning stay deferred (STATUS.md). The
// target_type is carried for the implicit-style match (merged_style); applying is by typed descriptor,
// already type-correct at the call site (setter::of).

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/setter.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::controls
{
    class style
    {
    public:
        // A based-on-by-key resolver: maps a resource key to the style registered under it (or null when
        // unresolved). merged_style supplies one bound to the target element's resource chain; the bare
        // apply()/unapply() use a null resolver (only a directly-assigned based_on is honored then).
        using resource_resolver = std::function<std::shared_ptr<style>(std::string_view key)>;

        explicit style(maui::core::type_tag target_type) : target_type_(target_type)
        {
        }

        // Convenience factory: style::of<button>() captures the target type tag.
        template <class TTarget> [[nodiscard]] static style of()
        {
            return style{maui::core::type_tag::of<TTarget>()};
        }

        [[nodiscard]] maui::core::type_tag target_type() const
        {
            return target_type_;
        }

        // Style.ApplyToDerivedTypes: whether this style may apply to types DERIVED from target_type
        // (matched against the chain a control declares via set_style_target_type<TControl, TBases...>).
        void set_apply_to_derived_types(bool value)
        {
            apply_to_derived_types_ = value;
        }
        [[nodiscard]] bool apply_to_derived_types() const
        {
            return apply_to_derived_types_;
        }

        // Style.CanBeAppliedTo(Type): true when target_type is the chain's exact (front) type, or — with
        // apply_to_derived_types set — any of its declared base types. An EMPTY chain (a control that
        // never declared a style target type) matches anything, preserving the pre-W1-15 class-style
        // behavior. Used by merged_style to select class styles.
        [[nodiscard]] bool can_be_applied_to(const std::vector<maui::core::type_tag>& target_chain) const;

        // Add a setter to the bundle (fluent — returns *this so calls can chain).
        style& add(setter value)
        {
            setters_.push_back(std::move(value));
            return *this;
        }

        // The style this one inherits from; its setters apply at a lowered (base-style) specificity so the
        // derived style's own setters take precedence. Setting a direct based_on clears base_resource_key
        // (Style.BasedOn's setter nulls BaseResourceKey), and vice versa — the two are mutually exclusive.
        void set_based_on(std::shared_ptr<style> value)
        {
            based_on_ = std::move(value);
            if (based_on_)
            {
                base_resource_key_.clear();
            }
        }
        [[nodiscard]] const std::shared_ptr<style>& based_on() const
        {
            return based_on_;
        }

        // The resource key of the base style (Style.BaseResourceKey). Resolved from the target's resource
        // chain at apply time via the resolver. Setting a non-empty key clears the direct based_on.
        void set_base_resource_key(std::string value)
        {
            base_resource_key_ = std::move(value);
            if (!base_resource_key_.empty())
            {
                based_on_ = nullptr;
            }
        }
        [[nodiscard]] std::string_view base_resource_key() const
        {
            return base_resource_key_;
        }

        // The style class this style belongs to (Style.Class). Empty for an implicit / locally-assigned
        // style; non-empty styles are selected by a control whose style_class names this class.
        void set_style_class(std::string value)
        {
            style_class_ = std::move(value);
        }
        [[nodiscard]] std::string_view style_class() const
        {
            return style_class_;
        }

        // Apply / un-apply every setter at `specificity` (defaulting to the explicit/local-style level).
        // based_on (direct, or resolved via `resolve` from base_resource_key) is applied first at the
        // lowered base-style specificity (so derived setters win) and un-applied last — Style.ApplyCore /
        // UnApplyCore. `resolve` may be null (then only a direct based_on is honored).
        void apply(maui::core::bindable_object& target,
                   maui::core::setter_specificity specificity = maui::core::setter_specificity::style_local,
                   const resource_resolver& resolve = nullptr) const;
        void unapply(maui::core::bindable_object& target,
                     maui::core::setter_specificity specificity = maui::core::setter_specificity::style_local,
                     const resource_resolver& resolve = nullptr) const;

    private:
        // The effective based-on style: the direct based_on, or base_resource_key resolved via `resolve`.
        [[nodiscard]] std::shared_ptr<style> effective_based_on(const resource_resolver& resolve) const;

        maui::core::type_tag target_type_;
        std::vector<setter> setters_;
        std::shared_ptr<style> based_on_;
        std::string base_resource_key_;
        std::string style_class_;
        bool apply_to_derived_types_ = false; // Style.ApplyToDerivedTypes
    };
} // namespace maui::controls
