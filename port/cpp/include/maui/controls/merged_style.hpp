#pragma once
// maui::controls::merged_style  <=  Microsoft.Maui.Controls.MergedStyle
//
// Resolves and applies the IMPLICIT (TargetType-keyed) style and the CLASS styles for one element, layered
// beneath its explicit local style. C#'s MergedStyle registers a DynamicResource per implicit-style type up
// the hierarchy; the reflection-free port resolves directly from the owning element's resource chain by the
// element's style_target_type tag (implicit_style_key) and by each selected style class — re-resolving when
// the resources change or the element reparents (element drives refresh()).
//
// Specificity layering (MergedStyle.Apply): implicit at setter_specificity::style_implicit, class styles at
// setter_specificity::style_class (StyleLocal + class byte), the local style at style_local (owned by
// view<>, not here). So implicit < class < local, exactly like C#. refresh() diffs the newly-resolved
// implicit/class styles against the applied ones and re-applies only on a change (SetStyle's
// shouldReApply…), so a no-op resource change doesn't churn the value precedence.
//
// W1-15 adds Style.ApplyToDerivedTypes: the element now declares its base-type CHAIN (most-derived
// first — element::set_style_target_type<TControl, TBases...>, the reflection-free substitute for C#'s
// Type.BaseType walk in MergedStyle.RegisterImplicitStyles). resolve_implicit walks the chain like
// OnImplicitStyleChanged: the exact (front) type's implicit style always applies; a BASE type's only
// when it sets apply_to_derived_types — and the walk continues past a non-matching base style. Class
// styles select via style::can_be_applied_to over the same chain (CanBeAppliedTo).
//
// Scope: implicit + class style resolution/application. CanCascade, system resources, and style sheets
// stay out of scope (STATUS.md).

#include <memory>
#include <string>
#include <vector>

#include "maui/core/type_tag.hpp"

namespace maui::controls
{
    class element;
    class style;

    class merged_style
    {
    public:
        explicit merged_style(element& owner) : owner_(&owner)
        {
        }
        merged_style(const merged_style&) = delete;
        merged_style(merged_style&&) = delete;
        merged_style& operator=(const merged_style&) = delete;
        merged_style& operator=(merged_style&&) = delete;
        ~merged_style() = default;

        // The element's style TargetType (set by element::set_style_target_type<T>()): the implicit-style
        // resource key is derived from it. Until set, no implicit style is resolved.
        void set_target_type(maui::core::type_tag value)
        {
            set_target_chain({value});
        }
        // The element's declared base-type chain, most-derived first (set_style_target_type<T, Bases...>)
        // — the implicit-style lookup tries each entry in order (MergedStyle's _implicitStyles slots).
        void set_target_chain(std::vector<maui::core::type_tag> chain)
        {
            target_chain_ = std::move(chain);
        }

        // The style classes this element selects (VisualElement.StyleClass). Re-resolves the class styles.
        void set_style_classes(std::vector<std::string> classes);

        // Re-resolve the implicit + class styles from the owner's current resource chain and re-apply only
        // what changed (un-applying the previously-applied styles first). Called by element on a resource
        // change / reparent, and by set_style_classes / set_target_type.
        void refresh();

        // Un-apply the currently-applied implicit + class styles (element teardown / when the dictionary is
        // cleared). Idempotent.
        void unapply();

    private:
        // Resolve the implicit style for target_type_ from the owner's resource chain (or null).
        [[nodiscard]] std::shared_ptr<style> resolve_implicit() const;
        // Resolve the class styles for the selected classes from the owner's resource chain.
        [[nodiscard]] std::vector<std::shared_ptr<style>> resolve_classes() const;

        element* owner_;
        std::vector<maui::core::type_tag> target_chain_; // most-derived first; empty until declared
        std::vector<std::string> style_classes_;
        // The currently-applied styles (so refresh can un-apply the old before applying the new).
        std::shared_ptr<style> applied_implicit_;
        std::vector<std::shared_ptr<style>> applied_classes_;
    };
} // namespace maui::controls
