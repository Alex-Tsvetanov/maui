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
    };

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
