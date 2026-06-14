#pragma once
// maui::controls::routing_effect  <=  Microsoft.Maui.Controls.RoutingEffect
//
// "Platform-independent effect that wraps an inner effect, which is usually platform-specific." A
// routing_effect is constructed with a resolution-id; its constructor resolves that id (Effect.Resolve)
// to a registered platform effect — or a null_effect when the id is unknown. The element's attach path
// registers the INNER effect (not the routing_effect) with the effect-control-provider; the
// routing_effect's own OnAttached/OnDetached are no-ops, and its lifecycle methods forward to the inner
// effect + the platform effect (RoutingEffect overrides every Send* / ClearEffect to delegate).

#include <memory>
#include <string_view>

#include "maui/controls/effect.hpp"

namespace maui::controls
{
    class routing_effect : public effect
    {
    public:
        // RoutingEffect(string effectId): resolve the id now (Effect.Resolve) into the inner effect.
        explicit routing_effect(std::string_view effect_id);
        // RoutingEffect(): the parameterless ctor — no inner effect (a subclass sets one, or there is none).
        routing_effect() = default;

        // RoutingEffect.Inner — the resolved inner (platform) effect, or null. NON-owning view of the
        // owned shared_ptr; the element reads it to register the inner effect instead of this wrapper.
        [[nodiscard]] effect* inner() const
        {
            return inner_.get();
        }
        [[nodiscard]] const std::shared_ptr<effect>& inner_shared() const
        {
            return inner_;
        }

        // RoutingEffect overrides: delegate the whole lifecycle to the inner effect + the platform effect.
        void clear_effect() override;
        void send_attached() override;
        void send_detached() override;
        void send_on_element_property_changed(std::string_view property_name) override;

    protected:
        // RoutingEffect.OnAttached / OnDetached are no-ops (the inner effect does the real work).
        void on_attached() override
        {
        }
        void on_detached() override
        {
        }

    private:
        // RoutingEffect.Inner. Owned (shared_ptr, because the element may hold it after the routing_effect's
        // collection registers it); released with this routing_effect when nothing else retains it (§8).
        std::shared_ptr<effect> inner_;
    };
} // namespace maui::controls
