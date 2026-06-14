#pragma once
// maui::controls::effect  <=  Microsoft.Maui.Controls.Effect
//                             (+ Microsoft.Maui.Controls.NullEffect — the cluster's no-op fallback)
//
// "A collection of styles and properties that can be added to an element at run time." The abstract base
// of every effect. An effect is ATTACHED to an element by the element's effect collection (see the G3
// block in element.hpp): the element registers the effect with its effect-control-provider, sets the
// back-reference, and drives the attach/detach lifecycle. C# marks the lifecycle entry points `internal`
// (only Element calls them); the reflection-free port has no `internal`, so they are public here but are
// the Element-only seam — application code uses the Effects collection, not these directly.
//
// Ownership (PROFILE §8): an effect is a SUBSCRIBER on its element (the publisher). It holds a NON-owning
// raw pointer back to the element, cleared on detach; the element holds the owning shared_ptr<effect> in
// its collection. So in tests declare the element BEFORE the effect — the effect must outlive nothing.

#include <memory>
#include <string>
#include <string_view>

namespace maui::controls
{
    class element; // forward — the attached element (non-owning back-ref); set by element on attach

    class effect
    {
    public:
        effect(const effect&) = delete;
        effect(effect&&) = delete;
        effect& operator=(const effect&) = delete;
        effect& operator=(effect&&) = delete;
        virtual ~effect() = default;

        // Effect.Element — the element the effect is attached to (null when detached). NON-owning.
        [[nodiscard]] element* attached_element() const
        {
            return element_;
        }

        // Effect.IsAttached — whether the effect is currently attached to an element.
        [[nodiscard]] bool is_attached() const
        {
            return is_attached_;
        }

        // Effect.ResolveId — the id used to resolve this effect at runtime (set by resolve()).
        [[nodiscard]] const std::string& resolve_id() const
        {
            return resolve_id_;
        }

        // Effect.Resolve(name): the registered effect for `name`, or a null_effect when the id is unknown;
        // the result's resolve_id is always set to `name` (matching the C# static factory). Lives in
        // effect.cpp because it constructs null_effect + consults the effect registry.
        [[nodiscard]] static std::shared_ptr<effect> resolve(std::string_view name);

        // ---- The Element-only lifecycle seam (C# Effect's `internal` members) ----------------------------
        // Effect.SendAttached: idempotent — OnAttached + flip IsAttached on the first call only, then forward
        // to the platform effect.
        virtual void send_attached();
        // Effect.SendDetached: idempotent — OnDetached + clear IsAttached only when currently attached, then
        // forward to the platform effect.
        virtual void send_detached();
        // Effect.ClearEffect: detach if attached, then drop the element back-reference (the collection's
        // remove/clear path).
        virtual void clear_effect();
        // Effect.SendOnElementPropertyChanged: routed from the element on every property change. The base is a
        // no-op (platform_effect overrides to forward to OnElementPropertyChanged when attached).
        virtual void send_on_element_property_changed(std::string_view property_name);

        // Element-set accessors (Effect.Element / ResolveId internal setters; the platform-effect link).
        void set_attached_element(element* value)
        {
            element_ = value;
        }
        void set_resolve_id(std::string id)
        {
            resolve_id_ = std::move(id);
        }
        // Effect.PlatformEffect — the platform-specific inner effect link. Typed as the effect base (the
        // forwarding callers only use the effect interface — send_*/clear_effect/set_attached_element — so
        // they need no platform_effect.hpp); a caller that wants the typed platform_effect_base downcasts.
        // set_platform_effect takes shared_ptr<effect>; passing a shared_ptr<platform_effect_base> upcasts
        // at the (complete-type) call site.
        [[nodiscard]] effect* platform_effect() const
        {
            return platform_effect_.get();
        }
        void set_platform_effect(std::shared_ptr<effect> value)
        {
            platform_effect_ = std::move(value);
        }

    protected:
        // Effect() is `internal` in C# (only the framework subclasses it); the port can't express that, so
        // the constructor is protected — only subclasses construct an effect.
        effect() = default;

        // Effect.OnAttached — "Received after Control/Container/Element made valid." Override to wire native.
        virtual void on_attached() = 0;
        // Effect.OnDetached — "Received after Control/Container made invalid." Override to unwire native.
        virtual void on_detached() = 0;

    private:
        element* element_ = nullptr; // Effect.Element — non-owning (the element owns this effect)
        bool is_attached_ = false;   // Effect.IsAttached
        std::string resolve_id_;     // Effect.ResolveId
        // Effect.PlatformEffect — the platform-specific inner effect a routing_effect / element wraps. Owned
        // here (the element creates it via the registry and hands it over), released with this effect (§8).
        // Stored as the effect base so the forwarding TUs need no platform_effect.hpp (see the accessor).
        std::shared_ptr<effect> platform_effect_;
    };

    // Microsoft.Maui.Controls.NullEffect — the no-op effect Effect.Resolve returns for an unknown id.
    class null_effect : public effect
    {
    public:
        null_effect() = default;

    protected:
        void on_attached() override
        {
        }
        void on_detached() override
        {
        }
    };
} // namespace maui::controls
