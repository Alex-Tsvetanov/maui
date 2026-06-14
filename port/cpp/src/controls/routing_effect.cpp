// maui::controls::routing_effect (routing_effect.hpp) — the platform-independent wrapper that resolves
// and delegates to an inner platform effect. Ported from src/Controls/src/Core/RoutingEffect.cs.
#include "maui/controls/routing_effect.hpp"

#include <string_view>

#include "maui/controls/effect.hpp"

namespace maui::controls
{
    routing_effect::routing_effect(std::string_view effect_id) : inner_(effect::resolve(effect_id))
    {
    }

    void routing_effect::clear_effect()
    {
        // RoutingEffect.ClearEffect: Inner?.ClearEffect(); PlatformEffect?.ClearEffect();
        if (inner_)
        {
            inner_->clear_effect();
        }
        if (platform_effect() != nullptr)
        {
            platform_effect()->clear_effect();
        }
    }

    void routing_effect::send_attached()
    {
        // RoutingEffect.SendAttached: Inner?.SendAttached(); PlatformEffect?.SendAttached();
        if (inner_)
        {
            inner_->send_attached();
        }
        if (platform_effect() != nullptr)
        {
            platform_effect()->send_attached();
        }
    }

    void routing_effect::send_detached()
    {
        // RoutingEffect.SendDetached: Inner?.SendDetached(); PlatformEffect?.SendDetached();
        if (inner_)
        {
            inner_->send_detached();
        }
        if (platform_effect() != nullptr)
        {
            platform_effect()->send_detached();
        }
    }

    void routing_effect::send_on_element_property_changed(std::string_view property_name)
    {
        // RoutingEffect.SendOnElementPropertyChanged: forward to Inner + PlatformEffect.
        if (inner_)
        {
            inner_->send_on_element_property_changed(property_name);
        }
        if (platform_effect() != nullptr)
        {
            platform_effect()->send_on_element_property_changed(property_name);
        }
    }
} // namespace maui::controls
