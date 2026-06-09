// Tests for the shared ViewMapper (maui::core::view_mapper) on the headless backend — the generic IView
// properties (Visibility / Opacity / IsEnabled / AutomationId) flowing control -> handler ->
// view_platform_base mirrors. Exercised through both the button and the label control (each handler's
// platform view derives view_platform_base and the handler chains view_mapper into its own mapper).
#include "maui/controls/button.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/image.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"

#include <memory>

#include "maui/core/button_handler.hpp"
#include "maui/core/entry_handler.hpp"
#include "maui/core/flow_direction.hpp"
#include "maui/core/i_shadow.hpp"
#include "maui/core/image_handler.hpp"
#include "maui/core/label_handler.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/core/semantics.hpp"
#include "maui/core/shadow.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/i_shape.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/rect_f.hpp"
#include "maui/graphics/shapes/ellipse.hpp"
#include "maui/graphics/shapes/rectangle.hpp"
#include "maui/graphics/shapes/round_rectangle.hpp"
#include "maui/graphics/solid_paint.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::button;
    using maui::controls::entry;
    using maui::controls::image;
    using maui::controls::label;
    using maui::controls::vertical_stack_layout;
    using maui::core::button_handler;
    using maui::core::entry_handler;
    using maui::core::flow_direction;
    using maui::core::image_handler;
    using maui::core::label_handler;
    using maui::core::layout_handler;
    using maui::core::view_platform_base;
    using maui::core::visibility;

    // ---- button: the four generic IView properties reach the platform base ----

    TEST(view_mapper_button, platform_base_is_available_after_attach)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        // button_platform derives view_platform_base, so the handler exposes a non-null platform base.
        EXPECT_NE(handler->platform_base(), nullptr);
    }

    TEST(view_mapper_button, defaults_map_on_connect)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);
        // VisualElement defaults: visible (hidden == false), opacity 1, enabled, empty automation id.
        EXPECT_FALSE(base->hidden);
        EXPECT_EQ(base->alpha, 1.0);
        EXPECT_TRUE(base->enabled);
        EXPECT_EQ(base->automation_id, "");
    }

    TEST(view_mapper_button, setting_visibility_maps_to_hidden)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);

        control.set_visibility(visibility::hidden);
        EXPECT_TRUE(base->hidden);

        control.set_visibility(visibility::collapsed);
        EXPECT_TRUE(base->hidden); // collapsed also hides

        control.set_visibility(visibility::visible);
        EXPECT_FALSE(base->hidden);
    }

    TEST(view_mapper_button, setting_opacity_maps_to_alpha)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);

        control.set_opacity(0.25);
        EXPECT_EQ(base->alpha, 0.25);
    }

    TEST(view_mapper_button, opacity_is_clamped_to_unit_range)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);

        // VisualElement.OpacityProperty clamps to [0,1] (coerceValue).
        control.set_opacity(2.0);
        EXPECT_EQ(control.opacity(), 1.0);
        EXPECT_EQ(base->alpha, 1.0);

        control.set_opacity(-1.0);
        EXPECT_EQ(control.opacity(), 0.0);
        EXPECT_EQ(base->alpha, 0.0);
    }

    TEST(view_mapper_button, setting_is_enabled_maps_to_enabled)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);

        control.set_is_enabled(false);
        EXPECT_FALSE(base->enabled);

        control.set_is_enabled(true);
        EXPECT_TRUE(base->enabled);
    }

    TEST(view_mapper_button, setting_automation_id_maps_to_mirror)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);

        control.set_automation_id("submit_button");
        EXPECT_EQ(base->automation_id, "submit_button");
    }

    TEST(view_mapper_button, initial_values_map_on_attach)
    {
        // Values set BEFORE the handler is attached must be pushed when the mapper runs on connect.
        button control;
        control.set_visibility(visibility::collapsed);
        control.set_opacity(0.5);
        control.set_is_enabled(false);
        control.set_automation_id("preset");

        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);
        EXPECT_TRUE(base->hidden);
        EXPECT_EQ(base->alpha, 0.5);
        EXPECT_FALSE(base->enabled);
        EXPECT_EQ(base->automation_id, "preset");
    }

    // ---- button: the render transform (nine ITransform scalars) + flow direction ----

    TEST(view_mapper_transform, identity_defaults_map_on_connect)
    {
        // VisualElement transform defaults: translations/rotations 0, scales 1, anchors 0.5; FlowDirection
        // MatchParent. The shared map_transform pushes the whole identity spec when the mapper runs.
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);

        EXPECT_EQ(base->transform.translation_x, 0.0);
        EXPECT_EQ(base->transform.translation_y, 0.0);
        EXPECT_EQ(base->transform.scale, 1.0);
        EXPECT_EQ(base->transform.scale_x, 1.0);
        EXPECT_EQ(base->transform.scale_y, 1.0);
        EXPECT_EQ(base->transform.rotation, 0.0);
        EXPECT_EQ(base->transform.rotation_x, 0.0);
        EXPECT_EQ(base->transform.rotation_y, 0.0);
        EXPECT_EQ(base->transform.anchor_x, 0.5);
        EXPECT_EQ(base->transform.anchor_y, 0.5);
        EXPECT_EQ(base->flow_direction, flow_direction::match_parent);
    }

    TEST(view_mapper_transform, each_setter_rebuilds_the_whole_spec)
    {
        // Any single scalar change re-pushes the FULL transform_spec (map_transform reads all nine off the
        // view), matching TransformationExtensions which rebuilds the CATransform3D from every scalar — so
        // a later setter must preserve the values set earlier.
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);

        control.set_translation_x(10.0);
        EXPECT_EQ(base->transform.translation_x, 10.0);

        control.set_translation_y(20.0);
        EXPECT_EQ(base->transform.translation_x, 10.0); // preserved across the next change
        EXPECT_EQ(base->transform.translation_y, 20.0);

        control.set_scale(2.0);
        control.set_scale_x(3.0);
        control.set_scale_y(4.0);
        control.set_rotation(45.0);
        control.set_rotation_x(15.0);
        control.set_rotation_y(30.0);
        control.set_anchor_x(0.0);
        control.set_anchor_y(1.0);

        // After all setters, the mirror reflects the full accumulated spec.
        EXPECT_EQ(base->transform.translation_x, 10.0);
        EXPECT_EQ(base->transform.translation_y, 20.0);
        EXPECT_EQ(base->transform.scale, 2.0);
        EXPECT_EQ(base->transform.scale_x, 3.0);
        EXPECT_EQ(base->transform.scale_y, 4.0);
        EXPECT_EQ(base->transform.rotation, 45.0);
        EXPECT_EQ(base->transform.rotation_x, 15.0);
        EXPECT_EQ(base->transform.rotation_y, 30.0);
        EXPECT_EQ(base->transform.anchor_x, 0.0);
        EXPECT_EQ(base->transform.anchor_y, 1.0);
    }

    TEST(view_mapper_transform, setting_flow_direction_maps_to_mirror)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);

        control.set_flow_direction(flow_direction::right_to_left);
        EXPECT_EQ(base->flow_direction, flow_direction::right_to_left);

        control.set_flow_direction(flow_direction::left_to_right);
        EXPECT_EQ(base->flow_direction, flow_direction::left_to_right);
    }

    TEST(view_mapper_transform, initial_transform_values_map_on_attach)
    {
        // Values set BEFORE the handler attaches are pushed (as the whole spec + flow direction) when the
        // mapper runs on connect.
        button control;
        control.set_translation_x(5.0);
        control.set_scale_x(2.5);
        control.set_rotation(90.0);
        control.set_anchor_y(0.25);
        control.set_flow_direction(flow_direction::right_to_left);

        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);

        EXPECT_EQ(base->transform.translation_x, 5.0);
        EXPECT_EQ(base->transform.scale_x, 2.5);
        EXPECT_EQ(base->transform.rotation, 90.0);
        EXPECT_EQ(base->transform.anchor_y, 0.25);
        // Untouched scalars keep their identity defaults even though one sibling was set.
        EXPECT_EQ(base->transform.translation_y, 0.0);
        EXPECT_EQ(base->transform.scale, 1.0);
        EXPECT_EQ(base->transform.anchor_x, 0.5);
        EXPECT_EQ(base->flow_direction, flow_direction::right_to_left);
    }

    // The transform/flow-direction retrofit also reaches a display-only control (label), proving the
    // shared mapper generalizes beyond button.
    TEST(view_mapper_transform, transform_reaches_label_platform)
    {
        label control;
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);

        control.set_scale(3.0);
        EXPECT_EQ(base->transform.scale, 3.0);

        control.set_flow_direction(flow_direction::right_to_left);
        EXPECT_EQ(base->flow_direction, flow_direction::right_to_left);
    }

    // ---- button: the visual-layer properties (Background paint / Shadow / Clip shape) ----

    TEST(view_mapper_visual, visual_layer_defaults_are_null)
    {
        // VisualElement defaults: no background, no shadow, no clip → the mirrors hold null borrows.
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);

        EXPECT_EQ(base->background, nullptr);
        EXPECT_EQ(base->shadow, nullptr);
        EXPECT_EQ(base->clip, nullptr);
        // The control's i_view getters agree (no object owned yet).
        EXPECT_EQ(control.background(), nullptr);
        EXPECT_EQ(control.shadow(), nullptr);
        EXPECT_EQ(control.clip(), nullptr);
    }

    TEST(view_mapper_visual, setting_solid_paint_background_maps_to_mirror)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);

        auto paint = std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::red);
        control.set_background(paint);
        // The mirror borrows the exact object the control owns (i_view returns the same .get()).
        EXPECT_EQ(base->background, paint.get());
        EXPECT_EQ(base->background, control.background());
        // ... and it carries the right color (red is opaque → not transparent).
        ASSERT_NE(base->background, nullptr);
        EXPECT_EQ(base->background->background_color(), maui::graphics::colors::red);
        EXPECT_FALSE(base->background->is_transparent());

        // A semi-transparent solid paint reports transparency (alpha < 1).
        control.set_background(
            std::make_shared<maui::graphics::solid_paint>(maui::graphics::color{0.0F, 0.0F, 1.0F, 0.5F}));
        ASSERT_NE(base->background, nullptr);
        EXPECT_TRUE(base->background->is_transparent());

        // Clearing it (null) maps a null mirror.
        control.set_background(nullptr);
        EXPECT_EQ(base->background, nullptr);
    }

    TEST(view_mapper_visual, setting_shadow_maps_to_mirror_with_defaults)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);

        auto sh = std::make_shared<maui::core::shadow>();
        control.set_shadow(sh);
        EXPECT_EQ(base->shadow, sh.get());
        EXPECT_EQ(base->shadow, control.shadow());
        ASSERT_NE(base->shadow, nullptr);
        // Shadow.cs defaults: radius 10, opacity 1, black paint, zero offset.
        EXPECT_EQ(base->shadow->radius(), 10.0);
        EXPECT_EQ(base->shadow->opacity(), 1.0);
        EXPECT_EQ(base->shadow->offset(), maui::graphics::point(0, 0));
        ASSERT_NE(base->shadow->paint(), nullptr);
        EXPECT_EQ(base->shadow->paint()->background_color(), maui::graphics::colors::black);

        // A customized shadow (distinct instance) flows the same way and carries its own values.
        auto sh2 = std::make_shared<maui::core::shadow>();
        sh2->set_radius(4.0);
        sh2->set_opacity(0.5);
        sh2->set_offset(maui::graphics::point(3, 6));
        control.set_shadow(sh2);
        EXPECT_EQ(base->shadow, sh2.get());
        ASSERT_NE(base->shadow, nullptr);
        EXPECT_EQ(base->shadow->radius(), 4.0);
        EXPECT_EQ(base->shadow->opacity(), 0.5);
        EXPECT_EQ(base->shadow->offset(), maui::graphics::point(3, 6));

        control.set_shadow(nullptr);
        EXPECT_EQ(base->shadow, nullptr);
    }

    TEST(view_mapper_visual, setting_rectangle_clip_maps_to_mirror)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);

        auto clip = std::make_shared<maui::graphics::shapes::rectangle>();
        control.set_clip(clip);
        EXPECT_EQ(base->clip, clip.get());
        EXPECT_EQ(base->clip, control.clip());
        ASSERT_NE(base->clip, nullptr);
        // The clip produces a closed rectangle path fitted to the given bounds: move + 3 lines + close
        // (5 operations); straight edges flatten to exact bounds.
        const maui::graphics::path_f path = base->clip->path_for_bounds(maui::graphics::rect(0, 0, 50, 20));
        EXPECT_EQ(path.operation_count(), 5);
        EXPECT_TRUE(path.closed());
        const maui::graphics::rect_f bounds = path.get_bounds_by_flattening();
        EXPECT_FLOAT_EQ(bounds.width, 50.0F);
        EXPECT_FLOAT_EQ(bounds.height, 20.0F);

        control.set_clip(nullptr);
        EXPECT_EQ(base->clip, nullptr);
    }

    TEST(view_mapper_visual, setting_round_rectangle_clip_maps_to_mirror)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);

        auto clip = std::make_shared<maui::graphics::shapes::round_rectangle>(8.0);
        control.set_clip(clip);
        EXPECT_EQ(base->clip, clip.get());
        ASSERT_NE(base->clip, nullptr);
        // append_rounded_rectangle (uniform radius): move + 4 cubics + 3 lines + close (9 operations); a
        // closed path. Its flattened bounds overshoot slightly for the corner Béziers, so the structure +
        // a near-bounds check (small tolerance, as the graphics path tests do for curves) is asserted.
        const maui::graphics::path_f path = base->clip->path_for_bounds(maui::graphics::rect(0, 0, 60, 40));
        EXPECT_EQ(path.operation_count(), 9);
        EXPECT_TRUE(path.closed());
        const maui::graphics::rect_f bounds = path.get_bounds_by_flattening();
        EXPECT_NEAR(bounds.width, 60.0F, 0.2F);
        EXPECT_NEAR(bounds.height, 40.0F, 0.2F);
    }

    TEST(view_mapper_visual, setting_ellipse_clip_maps_to_mirror)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);

        auto clip = std::make_shared<maui::graphics::shapes::ellipse>();
        control.set_clip(clip);
        EXPECT_EQ(base->clip, clip.get());
        ASSERT_NE(base->clip, nullptr);
        // append_ellipse: move + 4 cubics + close (6 operations); a closed path. The flattened bounds
        // overshoot slightly for the Bézier arcs, so a small tolerance is used (as the graphics tests do).
        const maui::graphics::path_f path = base->clip->path_for_bounds(maui::graphics::rect(0, 0, 30, 30));
        EXPECT_EQ(path.operation_count(), 6);
        EXPECT_TRUE(path.closed());
        const maui::graphics::rect_f bounds = path.get_bounds_by_flattening();
        EXPECT_NEAR(bounds.width, 30.0F, 0.2F);
        EXPECT_NEAR(bounds.height, 30.0F, 0.2F);
    }

    TEST(view_mapper_visual, initial_visual_values_map_on_attach)
    {
        // Values set BEFORE the handler attaches must be pushed when the mapper runs on connect.
        button control;
        auto paint = std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::green);
        auto sh = std::make_shared<maui::core::shadow>();
        auto clip = std::make_shared<maui::graphics::shapes::rectangle>();
        control.set_background(paint);
        control.set_shadow(sh);
        control.set_clip(clip);

        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);
        EXPECT_EQ(base->background, paint.get());
        EXPECT_EQ(base->shadow, sh.get());
        EXPECT_EQ(base->clip, clip.get());
    }

    // The visual-layer retrofit also reaches a display-only control (label), proving the shared mapper
    // generalizes beyond button.
    TEST(view_mapper_visual, visual_layer_reaches_label_platform)
    {
        label control;
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);

        auto paint = std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::blue);
        control.set_background(paint);
        EXPECT_EQ(base->background, paint.get());

        auto clip = std::make_shared<maui::graphics::shapes::ellipse>();
        control.set_clip(clip);
        EXPECT_EQ(base->clip, clip.get());
    }

    // ---- button: Semantics + InputTransparent (M5d) reach the platform base ----

    TEST(view_mapper_a11y, semantics_and_input_transparent_default_then_map)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);

        // Defaults: no semantics object, not input-transparent.
        EXPECT_EQ(base->semantics, nullptr);
        EXPECT_FALSE(base->input_transparent);

        auto sem = std::make_shared<maui::core::semantics>();
        sem->set_description("Submit");
        sem->set_hint("Submits the form");
        sem->set_heading_level(maui::core::semantic_heading_level::level1);
        control.set_semantics(sem);
        // The mirror borrows the exact object the control owns (i_view returns the same .get()).
        EXPECT_EQ(base->semantics, sem.get());
        EXPECT_EQ(base->semantics, control.semantics());
        ASSERT_NE(base->semantics, nullptr);
        EXPECT_EQ(base->semantics->description(), "Submit");
        EXPECT_EQ(base->semantics->hint(), "Submits the form");
        EXPECT_TRUE(base->semantics->is_heading());

        control.set_input_transparent(true);
        EXPECT_TRUE(base->input_transparent);
        EXPECT_TRUE(control.input_transparent());

        control.set_semantics(nullptr); // clearing maps a null mirror
        EXPECT_EQ(base->semantics, nullptr);
    }

    TEST(view_mapper_a11y, initial_semantics_and_input_transparent_map_on_attach)
    {
        button control;
        auto sem = std::make_shared<maui::core::semantics>();
        sem->set_description("Avatar");
        control.set_semantics(sem);
        control.set_input_transparent(true);

        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);
        EXPECT_EQ(base->semantics, sem.get());
        EXPECT_TRUE(base->input_transparent);
    }

    // ---- label: the same generic properties reach its platform base (recipe generalizes) ----

    TEST(view_mapper_label, generic_view_properties_map_to_platform)
    {
        label control;
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);

        control.set_visibility(visibility::hidden);
        EXPECT_TRUE(base->hidden);

        control.set_opacity(0.75);
        EXPECT_EQ(base->alpha, 0.75);

        control.set_is_enabled(false);
        EXPECT_FALSE(base->enabled);

        control.set_automation_id("caption");
        EXPECT_EQ(base->automation_id, "caption");
    }

    // ---- entry: the retrofit reaches the editable-field handler ----

    TEST(view_mapper_entry, generic_view_properties_map_to_platform)
    {
        entry control;
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);

        control.set_visibility(visibility::hidden);
        EXPECT_TRUE(base->hidden);

        control.set_opacity(0.6);
        EXPECT_EQ(base->alpha, 0.6);

        control.set_is_enabled(false);
        EXPECT_FALSE(base->enabled);

        control.set_automation_id("email_entry");
        EXPECT_EQ(base->automation_id, "email_entry");
    }

    // ---- image: the retrofit reaches the (minimal) image handler ----

    TEST(view_mapper_image, generic_view_properties_map_to_platform)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);

        control.set_visibility(visibility::collapsed);
        EXPECT_TRUE(base->hidden);

        control.set_opacity(0.3);
        EXPECT_EQ(base->alpha, 0.3);

        control.set_is_enabled(false);
        EXPECT_FALSE(base->enabled);

        control.set_automation_id("avatar");
        EXPECT_EQ(base->automation_id, "avatar");
    }

    // ---- layout: the panel handler also gets the generic properties. is_enabled keeps the base mirror
    // (a plain NSView panel has no native enabled state); the headless mirror still records every value. ----

    TEST(view_mapper_layout, generic_view_properties_map_to_platform)
    {
        vertical_stack_layout control;
        auto handler = std::make_shared<layout_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);

        control.set_visibility(visibility::hidden);
        EXPECT_TRUE(base->hidden);

        control.set_opacity(0.4);
        EXPECT_EQ(base->alpha, 0.4);

        control.set_is_enabled(false);
        EXPECT_FALSE(base->enabled);

        control.set_automation_id("form_stack");
        EXPECT_EQ(base->automation_id, "form_stack");
    }
} // namespace
