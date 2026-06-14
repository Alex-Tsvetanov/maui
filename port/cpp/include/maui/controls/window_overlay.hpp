#pragma once
// maui::controls::window_overlay  <=  Microsoft.Maui.WindowOverlay
//
// The concrete window overlay: a drawing/touch layer floating over a window's content. Ported from
// src/Core/src/WindowOverlay/WindowOverlay.cs (+ the .Standard / .iOS partials for the
// Initialize/Invalidate/Deinitialize lifecycle). An i_window_overlay (hence an i_drawable): Draw
// replays every window element onto the canvas (skipped when IsVisible is false).
//
// DRAWING SEAM: C#'s WindowOverlay hosts an OverlayGraphicsView (a PlatformGraphicsView) whose
// drawable is the overlay itself, added as a subview of the window root. The port mirrors this with
// an owned graphics_view whose drawable forwards to this overlay — so the overlay renders through the
// SAME graphics_view → graphics_view_handler → native graphics host as a developer's GraphicsView
// (W2-23): headless records the redraw count on the platform mirror; apple draws through the real
// NSView drawing host (coregraphics_canvas). initialize() attaches the graphics_view's handler (from
// the default registry, so it is backend-correct), deinitialize() detaches it.
//
// SCOPE (recorded in port/STATUS.md): the element list + IsVisible + the initialize/invalidate/
// deinitialize lifecycle + Draw are ported. The touch-passthrough knobs
// (DisableUITouchEventPassthrough / EnableDrawableTouchHandling) and the Tapped event are kept on the
// concrete overlay (C# WindowOverlay exposes them), but the Tapped *drive* (the visual-tree
// GetVisualTreeElements hit-test the port does not yet model) is deferred — documented, not stubbed.
// Density routes through the window (C# Window.RequestDisplayDensity → the port returns 1).

#include <memory>
#include <vector>

#include "maui/controls/graphics_view.hpp"
#include "maui/core/event.hpp"
#include "maui/core/i_window_overlay.hpp"
#include "maui/core/i_window_overlay_element.hpp"
#include "maui/graphics/i_drawable.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/rect_f.hpp"

namespace maui::core
{
    class i_canvas;
    class i_window;
} // namespace maui::core

namespace maui::controls
{
    class window_overlay : public maui::core::i_window_overlay
    {
    public:
        // C# WindowOverlay(IWindow window): the parent window is fixed at construction (NON-owning).
        explicit window_overlay(maui::core::i_window* window);
        ~window_overlay() override;
        window_overlay(const window_overlay&) = delete;
        window_overlay(window_overlay&&) = delete;
        window_overlay& operator=(const window_overlay&) = delete;
        window_overlay& operator=(window_overlay&&) = delete;

        // ---- i_window_overlay ----
        [[nodiscard]] maui::core::i_window* window() const override
        {
            return window_;
        }
        [[nodiscard]] std::vector<maui::core::i_window_overlay_element*> window_elements() const override
        {
            return elements_;
        }
        [[nodiscard]] bool is_visible() const override
        {
            return is_visible_;
        }
        // C# WindowOverlay.IsVisible setter: a change invalidates when the platform layer is up.
        void set_is_visible(bool value) override;
        [[nodiscard]] bool is_platform_view_initialized() const override
        {
            return is_platform_view_initialized_;
        }

        // C# WindowOverlay.Draw(ICanvas, RectF): replay every window element (no-op when not visible).
        void draw(maui::graphics::i_canvas& canvas, const maui::graphics::rect_f& dirty_rect) override;

        // C# WindowOverlay.Invalidate(): force the drawing layer to redraw (drives the graphics_view's
        // invalidate command → the native host's setNeedsDisplay / the headless redraw count).
        void invalidate() override;

        // C# WindowOverlay.AddWindowElement / RemoveWindowElement / RemoveWindowElements: manage the
        // element list (insertion-ordered, deduplicated like the C# HashSet) and invalidate.
        bool add_window_element(maui::core::i_window_overlay_element& element) override;
        bool remove_window_element(maui::core::i_window_overlay_element& element) override;
        void remove_window_elements() override;

        // C# WindowOverlay.Initialize() / Deinitialize(): wire / tear down the drawing layer.
        bool initialize() override;
        bool deinitialize() override;

        // ---- C# WindowOverlay extras (the concrete class surface) ----
        // C# WindowOverlay.DisableUITouchEventPassthrough / EnableDrawableTouchHandling — the
        // hit-test passthrough knobs. Stored; the native passthrough wiring is deferred (see header).
        [[nodiscard]] bool disable_ui_touch_event_passthrough() const
        {
            return disable_ui_touch_event_passthrough_;
        }
        void set_disable_ui_touch_event_passthrough(bool value)
        {
            disable_ui_touch_event_passthrough_ = value;
        }
        [[nodiscard]] bool enable_drawable_touch_handling() const
        {
            return enable_drawable_touch_handling_;
        }
        void set_enable_drawable_touch_handling(bool value)
        {
            enable_drawable_touch_handling_ = value;
        }
        // C# WindowOverlay.Density (=> Window?.RequestDisplayDensity() ?? 1f) is intentionally omitted:
        // the port's i_window exposes no display-density request (DisplayDensity is out of scope —
        // STATUS.md / i_window.hpp), so it would be a constant 1f with no source to read. Add it here when
        // i_window gains a density request. (i_adorner keeps its own density() — that contract requires it.)

        // C# WindowOverlay.Tapped — fired when the overlay is tapped. The drive (the visual-tree hit
        // test) is deferred; the event surface + the drawable-element hit list are portable. The hit
        // elements ride as const borrows (a tap handler inspects, never mutates them — tighter than C#'s
        // mutable IList, and the inspection path is read-only). Raised by on_tapped_internal (callable by
        // a future native passthrough view).
        maui::core::event<maui::graphics::point, std::vector<const maui::core::i_window_overlay_element*>> tapped;
        // C# WindowOverlay.OnTappedInternal(Point): collect the drawable elements containing `point`
        // (when EnableDrawableTouchHandling) and raise tapped. The visual-tree element collection
        // (DisableUITouchEventPassthrough branch) is deferred — the port has no visual-tree hit test.
        void on_tapped_internal(const maui::graphics::point& point);

        // The owned graphics_view the overlay renders through (the native draw host). Exposed so a
        // window host can place it in the view hierarchy (and the tests can read its handler/mirror).
        [[nodiscard]] graphics_view& graphics_surface()
        {
            return graphics_view_;
        }

    private:
        // The self-drawable adapter: the graphics_view's drawable forwards here to window_overlay::draw,
        // so the overlay's elements render through the shared graphics_view → handler → native host
        // (C#'s OverlayGraphicsView(this) — the overlay IS the graphics view's drawable).
        class overlay_drawable final : public maui::graphics::i_drawable
        {
        public:
            explicit overlay_drawable(window_overlay& owner) : owner_(&owner)
            {
            }
            void draw(maui::graphics::i_canvas& canvas, const maui::graphics::rect_f& dirty_rect) override
            {
                owner_->draw(canvas, dirty_rect);
            }

        private:
            window_overlay* owner_;
        };

        maui::core::i_window* window_ = nullptr;                      // C# WindowOverlay.Window (NON-owning)
        std::vector<maui::core::i_window_overlay_element*> elements_; // C# _windowElements (NON-owning)
        graphics_view graphics_view_;                                 // the owned native draw host
        std::shared_ptr<maui::graphics::i_drawable> self_drawable_;   // set as graphics_view_'s drawable
        bool is_visible_ = true;                                      // C# WindowOverlay._isVisible (default true)
        bool is_platform_view_initialized_ = false;                   // C# IsPlatformViewInitialized
        bool disable_ui_touch_event_passthrough_ = false;             // C# _disableUITouchEventPassthrough
        bool enable_drawable_touch_handling_ = false;                 // C# EnableDrawableTouchHandling
    };
} // namespace maui::controls
