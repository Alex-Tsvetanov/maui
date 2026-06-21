#pragma once
// maui::core::shape_view_handler  <=  Microsoft.Maui.Handlers.ShapeViewHandler
//   (+ the Controls per-shape sub-handler family it absorbs — see the collapse note below)
//
// The handler for a shape-rendering view (i_shape_view): the shape renders through the canvas stack
// via a shape_drawable hosted on the SAME native drawing seat as graphics_view (C#'s MauiShapeView
// is a PlatformGraphicsView subclass). Ported from ShapeViewHandler.cs + ShapeViewHandler.iOS.cs +
// ShapeViewExtensions.cs:
//   - MapShape → UpdateShape (a fresh ShapeDrawable over the virtual view; the port refreshes the
//     held drawable in place — same observable result, no allocation),
//   - every other Map* (Aspect/Fill/Stroke/StrokeThickness/DashPattern/DashOffset/LineCap/LineJoin/
//     MiterLimit/Background/FlowDirection) → InvalidateShape (a redraw request).
//
// PORT COLLAPSE (documented, not stubbed): C# layers per-shape SUB-handlers over ShapeViewHandler
// (Line/Rectangle/Ellipse/Polyline/Polygon/Path/BoxView handlers in
// src/Controls/src/Core/Handlers/Shapes/*) whose extra mapper keys (points, fill rule, data, the
// radii, the line coordinates, render transform) all funnel into InvalidateShape — plus
// MapFillRule/MapRenderTransform pushing into the ShapeDrawable. The port keeps ONE handler whose
// table carries the UNION of those keys; the winding mode and the render transform re-read off the
// i_shape_view port-extension getters on every shape refresh/invalidate (i_shape_view.hpp).
// PolylineHandler's Points.CollectionChanged resubscription disappears with the plain-vector
// collections (re-set the points to retrigger — the geometry.hpp collapse).
//
// Partial split: mapper tables + ctor + the spec reads cross-platform (shape_view_handler.cpp);
// create + update_shape + invalidate_shape + arrange_native per backend under
// src/platform/<backend>/shape_view_handler.{cpp,mm} (the apple/ios builds reuse the graphics_view
// host machinery — graphics_host.hpp).

#include <memory>
#include <string_view>

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_shape_view.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/shape_drawable.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
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
    struct shape_view_platform : view_platform_base
    {
        shape_view_platform() = default;
        ~shape_view_platform() override; // backend-defined: releases the retained native host on Apple/iOS
        shape_view_platform(const shape_view_platform&) = delete;
        shape_view_platform(shape_view_platform&&) = delete;
        shape_view_platform& operator=(const shape_view_platform&) = delete;
        shape_view_platform& operator=(shape_view_platform&&) = delete;

        void* native = nullptr;
        // The host's ShapeDrawable (C# MauiShapeView.Drawable) + the redraw-request mirror. Every
        // backend keeps both current; the native hosts also draw the drawable in drawRect.
        shape_drawable drawable;
        int invalidations = 0;

        // HEADLESS replay seat: draw the shape through the drawable into any canvas (the golden-op
        // tests replay into a recording_canvas — the native drawRect twin).
        void replay(maui::graphics::i_canvas& canvas, const maui::graphics::rect_f& dirty_rect);

#ifdef MAUI_PLATFORM_APPLE
        // Apple backend (src/platform/apple/shape_view_handler.mm): the NSView drawing host (the
        // graphics_view partial's scope — a plain NSView host, is_enabled keeps the base mirror).
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
        // iOS backend (src/platform/ios/shape_view_handler.mm): the UIView-host twin.
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

    class shape_view_handler : public view_handler<shape_view_handler, i_shape_view, shape_view_platform>
    {
    public:
        shape_view_handler();

        static property_mapper<i_shape_view, shape_view_handler>& mapper();
        static command_mapper<i_shape_view, shape_view_handler>& command_mapper();

        static std::unique_ptr<shape_view_platform> create_platform_view();

        // C# ShapeViewHandler.GetDesiredSize NaN-guards the base measure; the port's shape CONTROLS
        // compute their own size (Shape.MeasureOverride — controls/shapes/shape.hpp), so the handler
        // reports nothing here, like the border handler.
        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

        // ---- per-backend pieces ----
        // C# UpdateShape: re-point the host's ShapeDrawable at the virtual view (and redraw).
        void update_shape();
        // C# InvalidateShape: refresh the drawable's winding/render-transform pushes + redraw.
        void invalidate_shape();
        // Frame the native host (the backend half of platform_arrange).
        void arrange_native(const maui::graphics::rect& frame);

        // The shared half of update_shape/invalidate_shape: re-read the winding mode + render
        // transform off the virtual view into the host's drawable (the C# MapFillRule /
        // MapRenderTransform pushes, funneled — see the header collapse note).
        void refresh_drawable_state();

        // ---- mapper entries ----
        static void map_shape(shape_view_handler& handler, i_shape_view& view);
        static void map_invalidate_shape(shape_view_handler& handler, i_shape_view& view);
        static void map_background(shape_view_handler& handler, i_shape_view& view);
        static void map_flow_direction(shape_view_handler& handler, i_shape_view& view);
    };
} // namespace maui::core
