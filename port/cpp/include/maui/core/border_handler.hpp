#pragma once
// maui::core::border_handler  <=  Microsoft.Maui.Handlers.BorderHandler
//
// The handler for a border view (IBorderView) — a native container hosting the single content child
// and drawing the stroke outline (and content clip) along the border shape. Ported from
// BorderHandler.cs + BorderHandler.iOS.cs (+ the StrokeExtensions/MauiCALayer funnel):
//   - Content is hosted exactly like the content view's handler (clear + re-parent the content's
//     native view); the control computes its own geometry (Border.CrossPlatformMeasure/Arrange).
//   - EVERY stroke property map (Shape / Stroke / StrokeThickness / LineCap / LineJoin / DashPattern /
//     DashOffset / MiterLimit) funnels into ONE update_border() refresh, exactly as C#'s per-property
//     StrokeExtensions.Update* all call UpdateMauiCALayer — the platform reads the full IBorderStroke
//     surface off the virtual view each time.
//   - PlatformArrange re-runs the border refresh when the arranged SIZE changes (BorderHandler.
//     PlatformArrange re-issuing UpdateValue(Shape) — the stroke path depends on the bounds).
//
// PLATFORM ADAPTATION (recorded in STATUS): C# iOS draws the border in a custom MauiCALayer
// (DrawInContext). The port builds the equivalent from stock layers — a CAShapeLayer stroke (path =
// shape->path_for_bounds, the existing path_f→CGPath walk) plus the existing apply_clip mask — in the
// per-backend apple_border_ops.hpp / ios_border_ops.hpp. Headless mirrors the resolved stroke spec.
// Gradient strokes are out of scope (the spec mirrors the paint's background color — solid paints);
// C#'s IsConnectingHandler() skip-while-connecting optimization is likewise still deferred (with the
// shared view_mapper note).
//
// Same partial-class split as the other handlers: mapper tables + ctor + the spec snapshot are
// cross-platform (border_handler.cpp); create + set_content + update_border + arrange_native live per
// backend under src/platform/<backend>/border_handler.{cpp,mm}.

#include <algorithm>
#include <any>
#include <memory>
#include <string_view>
#include <vector>

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_border_view.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/i_shape.hpp"
#include "maui/graphics/line_cap.hpp"
#include "maui/graphics/line_join.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    class i_view;

    // The resolved IBorderStroke surface a border push carries — the value snapshot update_border()
    // reads off the virtual view (the platform partials consume it; the headless mirror stores it).
    struct border_stroke_spec
    {
        bool has_stroke = false;            // Stroke != null
        maui::graphics::color stroke_color; // the stroke paint's color (solid paints; see header)
        double thickness = 0;               // StrokeThickness
        maui::graphics::line_cap line_cap = maui::graphics::line_cap::butt;
        maui::graphics::line_join line_join = maui::graphics::line_join::miter;
        std::vector<float> dash_pattern;                // StrokeDashPattern (empty = solid)
        float dash_offset = 0;                          // StrokeDashOffset
        float miter_limit = 0;                          // StrokeMiterLimit
        const maui::graphics::i_shape* shape = nullptr; // non-owning borrow (the control owns it)
        bool shape_self_insets = true;                  // i_border_stroke::shape_self_insets (Frame: false)
    };

    // The extra 0.5 DIP/side inset MAUI's DEFAULT Border.StrokeShape carries — applied by each backend to
    // the bounds it feeds i_shape::path_for_bounds, wherever MAUI's own chain routes through
    // Shape.PathForBounds. STACKS ON TOP of that backend's Border-level (StrokeThickness) deflate.
    //
    // Border.StrokeShapeProperty defaults to a *Controls* Rectangle (Border.cs:81) whose OWN
    // StrokeThickness defaults to 1.0 (Shape.cs:80-81) and feeds Shape.TransformPathForBounds
    // (Shape.cs:312-323) unconditionally — `viewBounds.X += StrokeThickness / 2; Width -=
    // StrokeThickness`. The inset is a CONSTANT 0.5 per side: it comes from the SHAPE's own thickness,
    // never the Border's, and Border does not propagate its StrokeThickness to the shape. The single
    // exception is Border.UpdateStrokeShape (Border.cs:433-439):
    //     if (StrokeShape is Shape strokeShape && StrokeThickness == 0)
    //         strokeShape.StrokeThickness = StrokeThickness;
    // — a one-way LATCH. It only ever zeroes the shape's thickness, only when the Border's own
    // StrokeThickness is set to 0, and never restores it. For the static markup this port renders that
    // is exactly "no inset while the Border is unstroked", which is the thickness > 0 gate below. (A
    // Border driven 5 -> 0 -> 5 at RUNTIME keeps a zeroed shape in MAUI and so keeps no inset; the port
    // does not reproduce that latched state — a deliberately narrow divergence, invisible to markup.)
    //
    // The port's default StrokeShape is graphics::shapes::rectangle, a SIMPLIFIED shape with no
    // StrokeThickness of its own and no self-inset (see its header), so nothing reproduced this
    // anywhere. Reproducing it INSIDE those shared shapes was tried and reverted (f1a5a17658): it leaked
    // into clip paths, which MAUI never deflates (Geometry.PathForBounds has no such step). Hence this
    // helper, called only from the border handlers, on only the paths MAUI derives from a Shape.
    //
    // Measured, not merely derived: subpixel coverage-centroid measurement of border_stroke's stroke
    // edges against the real-MAUI capture columns (docs/comparison/PARITY_REVIEW.md) puts the port's
    // stroke a constant +0.5 DIP further OUT on all four edges at StrokeThickness 1, 5 and 10 —
    // independent of the thickness — on iOS, Android and (pre-fix) Windows, while borderless
    // (StrokeThickness = 0) already matched the MAUI column exactly.
    //
    // PASS THE SHAPE. This helper SUBSTITUTES for C# Shape.TransformPathForBounds on the shapes that omit
    // it (graphics/shapes/*), so it must stand down for a shape that performs that step itself —
    // maui::controls::shapes::shape does, and applying both counts one C# deflate twice. That is not
    // hypothetical: the XAML loader hands the border handler a CONTROLS shape for <Ellipse>/<Rectangle>/
    // <Polygon> while the code-first builder hands it a graphics one (xaml_visitors.cpp:1955), so the two
    // dialects rendered the SAME markup 0.5 DIP/side apart — measured on border_resize_content/ios, where
    // MAUI and the builder column both put the ellipse at 100.0 pt and the loader column at 98.67 pt.
    // The parameter defaults to nullptr for the call sites that synthesize their own default StrokeShape
    // (a graphics shape, so the answer is false anyway) and for the Android handler, which is frozen for a
    // motion measurement and keeps today's behaviour until it can be re-scored with the rest.
    [[nodiscard]] inline maui::graphics::rect shape_self_inset(const maui::graphics::rect& bounds, double thickness,
                                                               const maui::graphics::i_shape* shape = nullptr)
    {
        if (thickness <= 0.0)
        {
            return bounds; // Border.UpdateStrokeShape latched the shape's own thickness to 0
        }
        if (shape != nullptr && shape->applies_own_stroke_inset())
        {
            return bounds; // the shape already took this exact deflate in its own path_for_bounds
        }
        constexpr double k_inset = 0.5; // the default Controls Shape's StrokeThickness (1.0) / 2
        return maui::graphics::rect{bounds.x + k_inset, bounds.y + k_inset, std::max(0.0, bounds.width - (2 * k_inset)),
                                    std::max(0.0, bounds.height - (2 * k_inset))};
    }

    // Derives view_platform_base so the shared view_mapper pushes the generic IView properties onto it.
    struct border_platform : view_platform_base
    {
        border_platform() = default;
        ~border_platform() override; // backend-defined: releases the retained native host on Apple/iOS
        border_platform(const border_platform&) = delete;
        border_platform(border_platform&&) = delete;
        border_platform& operator=(const border_platform&) = delete;
        border_platform& operator=(border_platform&&) = delete;

        void* native = nullptr;
        // The hosted content child — the host's mirror of the control's content (the native builds ALSO
        // re-parent the matching real subview). Null when no content is set.
        i_view* hosted_content = nullptr;
        // The last border push — every backend keeps the mirror current (the headless tests assert on
        // it; the native partials read it as the just-applied snapshot).
        border_stroke_spec border;
        // The border's Shadow borrow (owned by the control). Apple/iOS use it: the host layer is masked to
        // the shape and a masked CALayer cannot cast a shadow, so the shadow is drawn on an unmasked sibling
        // layer that arrange/layout re-apply from this stored borrow.
        const maui::core::i_shadow* shadow = nullptr;
        // The border's Background paint borrow lives in the BASE (view_platform_base::background,
        // view_platform_base.hpp:86) — deliberately NOT redeclared here.
        //
        // It used to be redeclared, and the redeclaration SHADOWED the base member, which is a bug the
        // compiler cannot see: each backend then maintained a DIFFERENT one of the two fields.
        //   iOS   writes the derived one (border_handler.mm:116) and never calls the base body.
        //   Android calls view_platform_base::update_background, so it wrote the BASE one — and then read
        //          `platform->background`, which name lookup resolved to the DERIVED one. Always nullptr.
        // Measured on emulator-5554: at update_background the paint arrives non-null while border.shape is
        // still null, so the convex route installs the GradientDrawable; ~900 ms later the shape is set, and
        // native_draw_border_fill reads bg=0x0 and draws nothing. push_border_to_host has meanwhile CLEARED
        // that drawable for a canvas-routed shape, so BOTH routes are off and the Border renders WHITE —
        // border_resize_content row 3 (a Polygon), and every convex Border too once the canvas route is on.
        //
        // One field, written by whoever overrides update_background and read by everyone. Do not redeclare
        // it in a derived platform struct: the reads here (ios/border_handler.mm 137/210/233,
        // android/border_handler.cpp 1138/1157) must all resolve to the same storage.

#ifdef MAUI_PLATFORM_WINDOWS
        // WinUI 3 backend (src/platform/windows/border_handler.cpp): the generic IView pushes onto the
        // Canvas host via the shared winui_visual_ops helpers, EXCEPT update_background: BorderExtensions
        // / StrokeExtensions.UpdateBorderBackground routes Background onto the STROKE PATH's Fill (not the
        // host), so the fill follows the border SHAPE (rounded corners etc.) instead of painting the
        // host's full rectangular bounds. is_enabled keeps the generic push too (a no-op on a Canvas,
        // which is not a Control - the same degrade-to-no-op the label/content_page Windows partials hit).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        void update_background(const maui::graphics::paint* value) override;
#endif

#ifdef MAUI_PLATFORM_APPLE
        // Apple backend (src/platform/apple/border_handler.mm): the generic IView pushes onto the
        // NSView host. is_enabled keeps the base mirror (a plain NSView has no enabled state).
        // update_clip ALSO keeps the base mirror: the border's SHAPE owns the native layer mask (the
        // *_border_ops install it), so the generic IView.Clip must not clobber it — a Border with an
        // additional view-level Clip is out of scope (C# applies that clip on a WrapperView above).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
        void update_transform(const maui::core::transform_spec& value) override;
        void update_flow_direction(maui::core::flow_direction value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_shadow(const maui::core::i_shadow* value) override;
        void update_semantics(const maui::core::semantics* value) override;
        void update_input_transparent(bool value) override;
#endif

#ifdef MAUI_PLATFORM_IOS
        // iOS backend (src/platform/ios/border_handler.mm): the UIView-host twin. is_enabled and
        // flow_direction keep the base mirrors (matching the content_page partial's scope); transform IS
        // pushed via the shared ios apply_transform helper (the generic-IView ViewMapper widening).
        // update_clip keeps the base mirror for the same shape-owns-the-mask reason as on apple.
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
        // Render transform pushed to the native view via the shared ios apply_transform helper
        // (the generic-IView ViewMapper widening). `native` is this struct's UIView handle.
        void update_transform(const maui::core::transform_spec& value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_shadow(const maui::core::i_shadow* value) override;
        void update_semantics(const maui::core::semantics* value) override;
        void update_input_transparent(bool value) override;
#endif

#ifdef MAUI_PLATFORM_ANDROID
        // Android backend: the border hosts its content child in a dev/mauicpp/MauiLayout and draws the
        // border as a maui-managed GradientDrawable (stroke + corner radius + fill). Generic IView pushes
        // over JNI (src/platform/android/border_handler.cpp); base body FIRST then widget push. No
        // is_enabled (a border is non-interactive). shadow is pushed NATIVELY (setElevation + a colored
        // spot/ambient outline shadow shaped by the border's corner radius — android_visual_ops apply_shadow);
        // clip/input_transparent keep ONLY the base mirror.
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_shadow(const maui::core::i_shadow* value) override;
        void update_transform(const maui::core::transform_spec& value) override;
        void update_flow_direction(maui::core::flow_direction value) override;
        void update_semantics(const maui::core::semantics* value) override;
#endif
    };

    class border_handler : public view_handler<border_handler, i_border_view, border_platform>
    {
    public:
        border_handler();

        static property_mapper<i_border_view, border_handler>& mapper();
        static command_mapper<i_border_view, border_handler>& command_mapper();

        static std::unique_ptr<border_platform> create_platform_view();

        // A border computes its own size through the control (Border.CrossPlatformMeasure), so the
        // handler reports nothing here — like the content view's handler.
        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        // Frame the native host, then re-run the border refresh when the SIZE changed (C# BorderHandler.
        // PlatformArrange → UpdateValue(IBorderStroke.Shape) — the stroke path is bounds-dependent).
        void platform_arrange(const maui::graphics::rect& frame) override;

        // ---- per-backend pieces ----
        // Re-host the content subview from the virtual view's current content (C# UpdateContent).
        void set_content();
        // Push the full IBorderStroke surface to the native border (C# UpdateMauiCALayer).
        void update_border();
        // Frame the native host (the backend half of platform_arrange).
        void arrange_native(const maui::graphics::rect& frame);

        // Snapshot the virtual view's IBorderStroke surface (shared by every backend's update_border).
        [[nodiscard]] static border_stroke_spec make_border_stroke_spec(const i_border_view& view);

        // ---- mapper entries ----
        static void map_content(border_handler& handler, i_border_view& view);
        static void map_border_property(border_handler& handler, i_border_view& view);
        static void map_set_content(border_handler& handler, i_border_view& view, const std::any& args);
        static void map_safe_area_edges(border_handler& handler, i_border_view& view);

    private:
        // C# BorderHandler._lastSize: re-push the (bounds-dependent) stroke only when the size changes.
        maui::graphics::size last_size_{-1, -1};
    };
} // namespace maui::core
