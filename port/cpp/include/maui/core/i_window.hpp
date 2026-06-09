#pragma once
// maui::core::i_window  <=  Microsoft.Maui.IWindow (: ITitledElement : IElement)
//
// The cross-platform virtual-view contract a window_handler services — the Core seam between the concrete
// maui::controls::window and its native NSWindow, exactly as MAUI's WindowHandler services IWindow (not
// the concrete Window). Ported from src/Core/src/Core/IWindow.cs + ITitledElement.
//
// This cut keeps the surface the AppKit host actually uses: Title (ITitledElement), Content (the root
// page), the geometry getters (X/Y/Width/Height), the lifecycle DRIVE the native window calls back into
// (Activated/Deactivated/Destroying + Resumed/Stopped/Backgrounding), and FrameChanged (the native window
// reporting its new frame). Overlays, the visual diagnostics overlay, Min/MaxWidth/Height, FlowDirection,
// BackButtonClicked, and DisplayDensity are out of scope (STATUS.md) — added when a backend needs them.
//
// Content is an i_element* (not i_view*): the handler hosts the page's native view via the page's
// view-handler (content()->handler() down-cast to i_view_handler → native_view()), and a page is an
// i_element. NON-owning — the caller owns the page's lifetime (PROFILE §8); null when no page is set.

#include <string_view>

#include "maui/core/i_element.hpp"
#include "maui/graphics/rect.hpp" // frame_changed's parameter (a small value type — included, not fwd-declared)

namespace maui::core
{
    class i_window : public i_element
    {
    public:
        ~i_window() override = default;

        // ---- ITitledElement.Title + IWindow.Content ----
        [[nodiscard]] virtual std::string_view title() const = 0;
        [[nodiscard]] virtual i_element* content() const = 0;

        // ---- IWindow geometry (X / Y / Width / Height; Dimension.Unset until the platform reports a frame) ----
        [[nodiscard]] virtual double x() const = 0;
        [[nodiscard]] virtual double y() const = 0;
        [[nodiscard]] virtual double width() const = 0;
        [[nodiscard]] virtual double height() const = 0;

        // ---- IWindow lifecycle — the inbound channel the native window drives (send_* names, like a
        // control's send_clicked). The handler's notification trampoline calls these. ----
        virtual void send_activated() = 0;
        virtual void send_deactivated() = 0;
        virtual void send_destroying() = 0;
        virtual void send_resumed() = 0;
        virtual void send_stopped() = 0;
        virtual void send_backgrounding() = 0;

        // IWindow.FrameChanged: the native window reports its new frame (the handler's frame observer calls
        // this; the window sets X/Y/Width/Height at the handler specificity).
        virtual void frame_changed(const maui::graphics::rect& frame) = 0;

    protected:
        i_window() = default;
        i_window(const i_window&) = default;
        i_window(i_window&&) = default;
        i_window& operator=(const i_window&) = default;
        i_window& operator=(i_window&&) = default;
    };
} // namespace maui::core
