// maui::controls::element — the Effects collection + attach/detach lifecycle (the G3 block in
// element.hpp). Ported from Element.cs: Effects, EffectControlProvider (the IVisualElementController
// setter), AttachEffect, EffectsOnCollectionChanged / EffectsOnClearing, EffectIsAttached, and the
// SendOnElementPropertyChanged fan-out in OnPropertyChanged.
//
// Kept out of element.cpp so the hot file's single-block edit stays a declaration-only change (the task's
// "keep the element.hpp edit to one block" with the impl in a new element_effects.cpp).
#include "maui/controls/element.hpp"

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string_view>

#include "maui/controls/effect.hpp"
#include "maui/controls/effect_collection.hpp"
#include "maui/controls/i_effect_control_provider.hpp"
#include "maui/controls/routing_effect.hpp"
#include "maui/core/bindable_object.hpp" // on_property_changed routes through the bindable_object base

namespace maui::controls
{
    effect_collection& element::effects()
    {
        // Element.Effects: lazily create the collection, wiring its attach / clear hooks back to this
        // element (C# subscribes to CollectionChanged + Clearing on first access).
        if (!effects_)
        {
            effects_ = std::make_unique<effect_collection>([this](effect& target) { attach_effect(target); },
                                                           [](effect& target) { clear_effect(target); });
        }
        return *effects_;
    }

    void element::set_effect_control_provider(i_effect_control_provider* value)
    {
        // Element.EffectControlProvider setter: short-circuit on no change; detach all from the old provider,
        // swap, then attach all to the new one.
        if (effect_control_provider_ == value)
        {
            return;
        }
        if (effect_control_provider_ != nullptr && effects_)
        {
            for (const auto& held : effects_->items())
            {
                if (held)
                {
                    held->send_detached();
                }
            }
        }
        effect_control_provider_ = value;
        if (effect_control_provider_ != nullptr && effects_)
        {
            for (const auto& held : effects_->items())
            {
                if (held)
                {
                    attach_effect(*held);
                }
            }
        }
    }

    bool element::effect_is_attached(std::string_view name)
    {
        // Element.EffectIsAttached: any effect whose ResolveId matches `name`. (C# iterates the Effects
        // property, which lazily creates it — match that so the collection exists afterward, harmless.)
        return std::ranges::any_of(effects().items(),
                                   [name](const auto& held) { return held && held->resolve_id() == name; });
    }

    void element::attach_effect(effect& target)
    {
        // Element.AttachEffect: no provider -> nothing to attach yet (the EffectControlProvider setter will
        // attach later); an already-attached effect is an error (it would be bound to two sources).
        if (effect_control_provider_ == nullptr)
        {
            return;
        }
        if (target.is_attached())
        {
            throw std::logic_error("element::attach_effect: cannot attach Effect to multiple sources");
        }

        // For a routing_effect carrying an inner effect, register + own-the-element-on the INNER effect (the
        // platform effect the provider actually wires); otherwise the effect itself.
        effect* to_register = &target;
        if (auto* routing = dynamic_cast<routing_effect*>(&target); routing != nullptr && routing->inner() != nullptr)
        {
            to_register = routing->inner();
        }

        effect_control_provider_->register_effect(*to_register);
        to_register->set_attached_element(this);
        // C# IEffectControlProvider.RegisterEffect also sets platformEffect.Element = this; mirror that so a
        // platform effect created during register_effect can resolve its native Control/Container from this
        // element's handler when its send_attached runs below.
        if (auto* platform = to_register->platform_effect(); platform != nullptr)
        {
            platform->set_attached_element(this);
        }
        target.send_attached();
    }

    void element::clear_effect(effect& target)
    {
        // EffectsOnCollectionChanged (Remove) / EffectsOnClearing: Effect.ClearEffect (detach + drop ref).
        target.clear_effect();
    }

    void element::on_property_changed(std::string_view name)
    {
        // Element.OnPropertyChanged: raise the base notification (property_changed + bindings), then forward
        // SendOnElementPropertyChanged to each effect (only when any exist, like C#'s _effects?.Count > 0).
        maui::core::bindable_object::on_property_changed(name);
        if (effects_ && !effects_->empty())
        {
            for (const auto& held : effects_->items())
            {
                if (held)
                {
                    held->send_on_element_property_changed(name);
                }
            }
        }
    }
} // namespace maui::controls
