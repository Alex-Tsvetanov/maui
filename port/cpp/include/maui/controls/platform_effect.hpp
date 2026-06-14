#pragma once
// maui::controls::platform_effect<TContainer, TControl>
//                              <=  Microsoft.Maui.Controls.PlatformEffect<TContainer, TControl>
//   maui::controls::platform_effect_base
//                              <=  Microsoft.Maui.Controls.Platform.PlatformEffect (the non-generic base)
//
// The platform-specific half of an effect. C# layers two types: the generic
// PlatformEffect<TContainer, TControl> (the typed Container / Control + OnElementPropertyChanged) and the
// non-generic Platform.PlatformEffect : PlatformEffect<PlatformView, PlatformView> (which resolves the
// native views off the element's handler in SendAttached). Effect.PlatformEffect is typed as the
// NON-generic one — so the port mirrors that with a `platform_effect_base` carrier the effect base points
// at, and a `platform_effect<TContainer, TControl>` template that adds the typed accessors. The apple
// backend supplies the concrete native resolution (platform_effect.mm).
//
// Lifecycle (Effect overrides): SendDetached additionally clears Container/Control;
// SendOnElementPropertyChanged forwards to OnElementPropertyChanged only while attached.

#include <string_view>

#include "maui/controls/effect.hpp"

namespace maui::controls
{
    // The non-generic carrier (Microsoft.Maui.Controls.Platform.PlatformEffect's role): what
    // Effect.PlatformEffect points at. It is itself an effect (the generic C# base derives Effect), holds
    // the type-erased native container/control, and routes the property-changed seam. Concrete backends
    // (or the templated subclass) override send_attached to resolve the natives.
    class platform_effect_base : public effect
    {
    public:
        // PlatformEffect<,>.Container / .Control as opaque native pointers (an NSView* on apple, null on
        // headless). The typed platform_effect<TContainer, TControl> reinterprets these.
        [[nodiscard]] void* native_container() const
        {
            return container_;
        }
        [[nodiscard]] void* native_control() const
        {
            return control_;
        }
        void set_native_container(void* value)
        {
            container_ = value;
        }
        void set_native_control(void* value)
        {
            control_ = value;
        }

        // Platform.PlatformEffect.SendAttached: resolve Control + Container from the attached element's
        // handler (Control = the native view; Container = the container view, falling back to the native
        // view), then attach. Defined in platform_effect.cpp because it reaches into i_element /
        // i_view_handler. On headless the native pointers are null (no native tree); on apple they are real
        // NSView*s — the same cross-platform resolution, since i_view_handler::native_view() abstracts the
        // platform view (the analog of C#'s PlatformView). The apple smoke test verifies the real pointers.
        void send_attached() override;

        // Effect.SendDetached override: detach, then null Container + Control (PlatformEffect<,>).
        void send_detached() override
        {
            effect::send_detached();
            container_ = nullptr;
            control_ = nullptr;
        }

        // Effect.SendOnElementPropertyChanged override: forward to OnElementPropertyChanged only while
        // attached (PlatformEffect<,>).
        void send_on_element_property_changed(std::string_view property_name) override
        {
            if (is_attached())
            {
                on_element_property_changed(property_name);
            }
        }

    protected:
        platform_effect_base() = default;

        // PlatformEffect<,>.OnElementPropertyChanged — the platform reacts to an element property change.
        // The base is a no-op (matching the C# virtual's empty body); backends override it.
        virtual void on_element_property_changed(std::string_view property_name)
        {
            (void)property_name;
        }

    private:
        void* container_ = nullptr; // PlatformEffect<,>.Container (native, type-erased)
        void* control_ = nullptr;   // PlatformEffect<,>.Control (native, type-erased)
    };

    // The typed PlatformEffect<TContainer, TControl>: the same carrier with Container() / Control()
    // narrowed to the platform view types. Header-only template (no per-type .cpp), like the C# generic.
    // The void* -> TContainer*/TControl* narrowing is a non-owning, non-ownership-changing reinterpret of
    // the platform handle. Under ARC (.mm translation units) casting void* to an Objective-C class pointer
    // requires `__bridge`; in plain C++ TUs (headless) a reinterpret_cast is used. Both forms are an
    // ownership-neutral view of the already-retained native (the handler owns it; the effect borrows it).
    template <class TContainer, class TControl> class platform_effect : public platform_effect_base
    {
    public:
        [[nodiscard]] TContainer* container() const
        {
#if defined(__OBJC__)
            return (__bridge TContainer*)native_container();
#else
            return reinterpret_cast<TContainer*>(native_container());
#endif
        }
        [[nodiscard]] TControl* control() const
        {
#if defined(__OBJC__)
            return (__bridge TControl*)native_control();
#else
            return reinterpret_cast<TControl*>(native_control());
#endif
        }

    protected:
        platform_effect() = default;
    };
} // namespace maui::controls
