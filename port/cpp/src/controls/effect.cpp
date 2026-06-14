// maui::controls::effect — the abstract effect base lifecycle + Effect.Resolve (effect.hpp).
// Ported from src/Controls/src/Core/Effect.cs (SendAttached / SendDetached / ClearEffect / Resolve) and
// NullEffect.cs.
#include "maui/controls/effect.hpp"

#include <memory>
#include <string>
#include <string_view>

#include "maui/controls/effect_registry.hpp"

namespace maui::controls
{
    std::shared_ptr<effect> effect::resolve(std::string_view name)
    {
        // Effect.Resolve: the registered effect, else a NullEffect; ResolveId is always set to `name`.
        std::shared_ptr<effect> result = resolve_effect(name);
        if (!result)
        {
            result = std::make_shared<null_effect>();
        }
        result->set_resolve_id(std::string{name});
        return result;
    }

    void effect::send_attached()
    {
        // Effect.SendAttached: idempotent — attach once, then forward to the platform effect.
        if (is_attached_)
        {
            return;
        }
        on_attached();
        is_attached_ = true;
        if (platform_effect_)
        {
            platform_effect_->send_attached();
        }
    }

    void effect::send_detached()
    {
        // Effect.SendDetached: idempotent — detach only when attached, then forward to the platform effect.
        if (!is_attached_)
        {
            return;
        }
        on_detached();
        is_attached_ = false;
        if (platform_effect_)
        {
            platform_effect_->send_detached();
        }
    }

    void effect::clear_effect()
    {
        // Effect.ClearEffect: detach if attached, then drop the element back-reference.
        if (is_attached_)
        {
            send_detached();
        }
        element_ = nullptr;
    }

    void effect::send_on_element_property_changed(std::string_view property_name)
    {
        // Effect.SendOnElementPropertyChanged: no-op on the base (platform_effect overrides it).
        (void)property_name;
    }
} // namespace maui::controls
