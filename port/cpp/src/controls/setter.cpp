// maui::controls::setter — the out-of-line TargetName resolution (setter.hpp).
// Ported from Setter.cs Apply/UnApply + FindTargetByName: when a Setter carries a TargetName string, the
// name is resolved against the apply target's element namescope at apply time, then apply/unapply route to
// the resolved element. Out-of-line so the header-only setter (included widely via style/trigger) does not
// pull in element.hpp / the XAML error type.
#include "maui/controls/setter.hpp"

#include <string>

#include "maui/controls/element.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/xaml/xaml_parse_exception.hpp"

namespace maui::controls
{
    maui::core::bindable_object& setter::resolve_named_target(maui::core::bindable_object& target) const
    {
        // Setter.Apply: `if (!string.IsNullOrEmpty(TargetName) && target is Element element)`. A bare
        // bindable_object has no namescope, so a named setter applied to one is unresolvable (C# falls
        // through to the throw because the `target is Element` guard fails and targetObject stays the
        // non-element target — but then writing the property to the wrong object would be silently wrong;
        // the port treats a non-element target as an explicit resolution failure for the named channel).
        auto* element_target = dynamic_cast<element*>(&target);
        maui::core::bindable_object* resolved =
            element_target != nullptr ? element_target->find_target_by_name(target_name_) : nullptr;
        if (resolved == nullptr)
        {
            // C#: throw new XamlParseException($"Cannot resolve '{TargetName}' as Setter Target for
            // '{target}'."). The reflection-free port has no element ToString for the '{target}' tail, so
            // the message names the unresolved TargetName (the actionable part) and omits the target render.
            throw maui::xaml::xaml_parse_exception("Cannot resolve '" + target_name_ + "' as Setter Target.");
        }
        return *resolved;
    }
} // namespace maui::controls
