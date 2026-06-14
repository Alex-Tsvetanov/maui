#pragma once
// maui::controls::i_effect_control_provider  <=  Microsoft.Maui.Controls.IEffectControlProvider
//
// "Provides the functionality to register an Effect to an element." The seam the element drives when an
// effect attaches: the provider (a renderer/handler in real MAUI; a stub in tests) is handed the effect
// so it can create the platform_effect + wire the native control. In the port the element implements the
// IElementController/IVisualElementController role of being settable as the provider (see the G3 block in
// element.hpp); a handler-backed provider supplies the native resolution.

namespace maui::controls
{
    class effect; // forward — the effect being registered (effect.hpp)

    class i_effect_control_provider
    {
    public:
        i_effect_control_provider(const i_effect_control_provider&) = default;
        i_effect_control_provider(i_effect_control_provider&&) = default;
        i_effect_control_provider& operator=(const i_effect_control_provider&) = default;
        i_effect_control_provider& operator=(i_effect_control_provider&&) = default;
        virtual ~i_effect_control_provider() = default;

        // IEffectControlProvider.RegisterEffect: register the effect with the element (create + attach its
        // platform effect, wire the native view). Called by element::attach_effect before send_attached.
        virtual void register_effect(effect& target) = 0;

    protected:
        i_effect_control_provider() = default;
    };
} // namespace maui::controls
