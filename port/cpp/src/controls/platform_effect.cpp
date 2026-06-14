// maui::controls::platform_effect_base — the native Control / Container resolution
// (platform_effect.hpp). Ported from src/Controls/src/Core/Platform/PlatformEffect.cs (SendAttached):
//   Control = (PlatformView)Element.Handler.PlatformView;
//   Container = (PlatformView)(viewHandler.ContainerView ?? viewHandler.PlatformView);   // else Control
// In the port the native pointers come through the backend-agnostic i_view_handler accessors
// (native_view() / container_view()), so this resolution is cross-platform: null on headless, real
// NSView*s on apple. effect.hpp's Effect.PlatformEffect link is what carries this onto an effect.
#include "maui/controls/platform_effect.hpp"

#include <memory>

#include "maui/controls/element.hpp"
#include "maui/core/i_element.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_view_handler.hpp"

namespace maui::controls
{
    void platform_effect_base::send_attached()
    {
        // The element a platform effect attaches to is also an i_element (every view<> is both); its handler
        // owns the native view. A handler-less / non-view element leaves Control + Container null (the
        // headless case, and the "no element/handler yet" case — C# would throw, but the port keeps the
        // null-safe path so attach before a handler is wired is a no-op resolution rather than a crash).
        if (auto* as_element = dynamic_cast<maui::core::i_element*>(attached_element()); as_element != nullptr)
        {
            const std::shared_ptr<maui::core::i_element_handler>& handler = as_element->handler();
            if (handler)
            {
                if (auto* view_handler = dynamic_cast<maui::core::i_view_handler*>(handler.get());
                    view_handler != nullptr)
                {
                    // Control = the native platform view (ToPlatform / IElementHandler.PlatformView's
                    // documented meaning — i_view_handler::native_view(), not the owning pimpl); Container =
                    // the container view, falling back to the native view.
                    void* const native = view_handler->native_view();
                    set_native_control(native);
                    void* const container = view_handler->container_view();
                    set_native_container(container != nullptr ? container : native);
                }
                else
                {
                    // A non-view element handler has only the pimpl handle; use it for both.
                    set_native_control(handler->platform_view());
                    set_native_container(native_control());
                }
            }
        }
        effect::send_attached();
    }
} // namespace maui::controls
