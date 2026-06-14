#pragma once
// maui::core::i_window_overlay  <=  Microsoft.Maui.IWindowOverlay
//
// The contract for a drawing/touch layer that floats over an i_window's content. Ported from
// src/Core/src/Core/IWindowOverlay.cs. An i_drawable: Draw replays every window element onto the
// overlay's canvas. Window.add_overlay/remove_overlay manage a set of these and drive
// initialize()/deinitialize() + invalidate().
//
// SCOPE (recorded in port/STATUS.md): the surface kept is the part the window + the concrete
// window_overlay actually drive — the element list, IsVisible/IsPlatformViewInitialized, the
// initialize/deinitialize/invalidate lifecycle, and Draw. The touch-passthrough knobs
// (DisableUITouchEventPassthrough / EnableDrawableTouchHandling), Density, and the Tapped event live
// on the concrete window_overlay (they need the visual-tree hit-test the port does not yet model) —
// not on this minimal core contract.

#include <vector>

#include "maui/graphics/i_drawable.hpp"

namespace maui::core
{
    class i_window;
    class i_window_overlay_element;

    class i_window_overlay : public maui::graphics::i_drawable
    {
    public:
        // C# IWindowOverlay.Window — the containing window (set at construction). NON-owning.
        [[nodiscard]] virtual i_window* window() const = 0;

        // C# IWindowOverlay.WindowElements — the drawable elements on this overlay (NON-owning borrows).
        [[nodiscard]] virtual std::vector<i_window_overlay_element*> window_elements() const = 0;

        // C# IWindowOverlay.IsVisible — whether the overlay draws (false = Draw is a no-op).
        [[nodiscard]] virtual bool is_visible() const = 0;
        virtual void set_is_visible(bool value) = 0;

        // C# IWindowOverlay.IsPlatformViewInitialized — whether initialize() has wired the draw layer.
        [[nodiscard]] virtual bool is_platform_view_initialized() const = 0;

        // C# IWindowOverlay.Invalidate() — force the layer to redraw.
        virtual void invalidate() = 0;

        // C# IWindowOverlay.AddWindowElement / RemoveWindowElement / RemoveWindowElements — manage the
        // element list (each returns whether the set changed).
        virtual bool add_window_element(i_window_overlay_element& element) = 0;
        virtual bool remove_window_element(i_window_overlay_element& element) = 0;
        virtual void remove_window_elements() = 0;

        // C# IWindowOverlay.Initialize() / Deinitialize() — wire / tear down the draw layer. The window
        // calls initialize() on add_overlay and deinitialize() on remove_overlay.
        virtual bool initialize() = 0;
        virtual bool deinitialize() = 0;

    protected:
        i_window_overlay() = default;
        i_window_overlay(const i_window_overlay&) = default;
        i_window_overlay(i_window_overlay&&) = default;
        i_window_overlay& operator=(const i_window_overlay&) = default;
        i_window_overlay& operator=(i_window_overlay&&) = default;
    };
} // namespace maui::core
