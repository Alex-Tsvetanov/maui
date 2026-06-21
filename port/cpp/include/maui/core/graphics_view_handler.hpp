#pragma once
// maui::core::graphics_view_handler  <=  Microsoft.Maui.Handlers.GraphicsViewHandler
//
// The handler for a canvas-drawn view (i_graphics_view): the Drawable flows virtual→native through
// the mapper, the Invalidate command requests a native redraw. Ported from GraphicsViewHandler.cs
// (cross-platform) + GraphicsViewHandler.iOS.cs / PlatformTouchGraphicsView.cs (the platform
// recipe):
//   - MapDrawable → UpdateDrawable (point the native canvas host at the virtual view's drawable —
//     the host redraws through the W1-13 canvas core: NSView drawRect → coregraphics_canvas on
//     apple, the UIView twin on ios, an op-recording replay seat headlessly).
//   - MapBackground / MapFlowDirection override the chained generic pushes to ALSO invalidate
//     (background only invalidates when one is set, exactly like C#).
//   - MapInvalidate (command) → InvalidateDrawable.
//
// PLATFORM ADAPTATION (recorded in STATUS): C#'s PlatformTouchGraphicsView adds the touch plumbing into
// IGraphicsView.Start/Drag/End/CancelInteraction. The port's drawing hosts now carry it: on_connect_handler
// points the host at this handler's virtual view, and the host's native pointer events route there —
// AppKit mouseDown/Dragged/Up → start/drag/end on apple, UIKit TouchesBegan/Moved/Ended/Cancelled →
// start/drag/end/cancel on iOS (each gated by IsEnabled, the C# guard). Hover (UIHoverGestureRecognizer)
// stays deferred with the wider gesture seam; the shape host stays draw-only (it connects no target).
//
// Same partial split as the other handlers: mapper tables + ctor cross-platform
// (graphics_view_handler.cpp); create + update_drawable + invalidate_drawable + arrange_native per
// backend under src/platform/<backend>/graphics_view_handler.{cpp,mm}.

#include <any>
#include <memory>
#include <string_view>

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_graphics_view.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/i_drawable.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/rect_f.hpp"
#include "maui/graphics/size.hpp"

namespace maui::graphics
{
    class i_canvas;
} // namespace maui::graphics

namespace maui::core
{
    // Derives view_platform_base so the shared view_mapper pushes the generic IView properties onto it.
    struct graphics_view_platform : view_platform_base
    {
        graphics_view_platform() = default;
        ~graphics_view_platform() override; // backend-defined: releases the retained native host on Apple/iOS
        graphics_view_platform(const graphics_view_platform&) = delete;
        graphics_view_platform(graphics_view_platform&&) = delete;
        graphics_view_platform& operator=(const graphics_view_platform&) = delete;
        graphics_view_platform& operator=(graphics_view_platform&&) = delete;

        void* native = nullptr;
        // The mirrors every backend keeps current (the headless tests assert on them; the native
        // partials also push to the real drawing host): the drawable borrow (the control owns it)
        // and the redraw-request count (each PlatformGraphicsView.InvalidateDrawable).
        maui::graphics::i_drawable* drawable = nullptr;
        int invalidations = 0;

        // HEADLESS replay seat: draw the current drawable into any canvas over the given dirty rect
        // (the golden-op tests replay into a recording_canvas — the native drawRect twin).
        void replay(maui::graphics::i_canvas& canvas, const maui::graphics::rect_f& dirty_rect) const;

#ifdef MAUI_PLATFORM_APPLE
        // Apple backend (src/platform/apple/graphics_view_handler.mm): the generic IView pushes onto
        // the NSView drawing host. is_enabled keeps the base mirror (a plain NSView has no enabled
        // state); the deferred pushes keep the base mirrors per the shared view_mapper note.
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
        void update_transform(const maui::core::transform_spec& value) override;
        void update_flow_direction(maui::core::flow_direction value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_shadow(const maui::core::i_shadow* value) override;
        void update_clip(const maui::graphics::i_shape* value) override;
        void update_semantics(const maui::core::semantics* value) override;
        void update_input_transparent(bool value) override;
#endif

#ifdef MAUI_PLATFORM_IOS
        // iOS backend (src/platform/ios/graphics_view_handler.mm): the UIView-host twin (the same
        // scope as the other ios drawing partials).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
        // Render transform pushed to the native view via the shared ios apply_transform helper
        // (the generic-IView ViewMapper widening). `native` is this struct's UIView handle.
        void update_transform(const maui::core::transform_spec& value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_shadow(const maui::core::i_shadow* value) override;
        void update_clip(const maui::graphics::i_shape* value) override;
        void update_semantics(const maui::core::semantics* value) override;
        void update_input_transparent(bool value) override;
#endif
    };

    class graphics_view_handler : public view_handler<graphics_view_handler, i_graphics_view, graphics_view_platform>
    {
    public:
        graphics_view_handler();

        static property_mapper<i_graphics_view, graphics_view_handler>& mapper();
        static command_mapper<i_graphics_view, graphics_view_handler>& command_mapper();

        static std::unique_ptr<graphics_view_platform> create_platform_view();

        // C# GraphicsViewHandler.ConnectHandler/DisconnectHandler → PlatformTouchGraphicsView.Connect /
        // Disconnect: point the native drawing host's touch plumbing at this handler's virtual view (so
        // its mouse/touch events route into i_graphics_view::send_*_interaction) and clear it on teardown.
        // Apple/iOS only — headless has no native host, so it declares no hook (the CRTP base's
        // `requires` detection then skips the call). The shape host never connects a target (draw-only).
#if defined(MAUI_PLATFORM_APPLE) || defined(MAUI_PLATFORM_IOS)
        void on_connect_handler(graphics_view_platform& platform);
        void on_disconnect_handler(graphics_view_platform& platform);
#endif

        // C# GraphicsViewHandler keeps the base GetDesiredSize (the native host reports no intrinsic
        // size — a plain canvas measures 0 and the size requests drive the layout).
        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

        // ---- per-backend pieces ----
        // C# UpdateDrawable: point the native host at VirtualView.Drawable (and redraw).
        void update_drawable();
        // C# InvalidateDrawable: request a native redraw.
        void invalidate_drawable();
        // Frame the native host (the backend half of platform_arrange).
        void arrange_native(const maui::graphics::rect& frame);

        // ---- mapper entries ----
        static void map_drawable(graphics_view_handler& handler, i_graphics_view& view);
        static void map_background(graphics_view_handler& handler, i_graphics_view& view);
        static void map_flow_direction(graphics_view_handler& handler, i_graphics_view& view);
        static void map_invalidate(graphics_view_handler& handler, i_graphics_view& view, const std::any& args);
    };
} // namespace maui::core
