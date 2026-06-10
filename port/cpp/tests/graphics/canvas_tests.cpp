// Tests for the canvas core — recording_canvas golden ops + the AbstractCanvas/CanvasState
// save/restore semantics. The behavioral oracle is the C# source (no GPU-free C# unit tests cover
// AbstractCanvas directly): src/Graphics/src/Graphics/{AbstractCanvas,CanvasState,CanvasDefaults,
// ICanvas,IDrawable}.cs + Matrix3x2Extensions.cs — characterization tests derived line by line.

#include "maui/graphics/recording_canvas.hpp"

#include <cmath>
#include <cstddef>
#include <numbers>
#include <stdexcept>
#include <variant>
#include <vector>

#include "maui/graphics/canvas_defaults.hpp"
#include "maui/graphics/canvas_state.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/font.hpp"
#include "maui/graphics/horizontal_alignment.hpp"
#include "maui/graphics/i_canvas.hpp"
#include "maui/graphics/i_drawable.hpp"
#include "maui/graphics/matrix3x2.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/rect_f.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/graphics/text/attributed_text.hpp"
#include "maui/graphics/text/attributed_text_run.hpp"
#include "maui/graphics/text/text_attributes.hpp"
#include "maui/graphics/winding_mode.hpp"
#include <gtest/gtest.h>

namespace
{
    namespace ops = maui::graphics::canvas_ops;
    using maui::graphics::canvas_op;
    using maui::graphics::canvas_state;
    using maui::graphics::color;
    using maui::graphics::horizontal_alignment;
    using maui::graphics::matrix3x2;
    using maui::graphics::recording_canvas;
    using maui::graphics::rect_f;
    using maui::graphics::winding_mode;
    namespace colors = maui::graphics::colors;

    // Fetches the op at `index` as Op, flagging (not crashing) the test on a missing index or a
    // different alternative — the value-initialized fallback then fails the value assertions.
    template <class Op> const Op& op_at(const recording_canvas& canvas, std::size_t index)
    {
        static const Op fallback{};
        const std::vector<canvas_op>& ops = canvas.ops();
        EXPECT_LT(index, ops.size());
        if (index >= ops.size())
        {
            return fallback;
        }
        const Op* op = std::get_if<Op>(&ops[index]);
        EXPECT_NE(op, nullptr) << "unexpected op alternative at index " << index;
        return op != nullptr ? *op : fallback;
    }

    // ---- golden op sequences ----

    TEST(recording_canvas_test, records_stroke_state_and_line)
    {
        recording_canvas canvas;
        canvas.set_stroke_color(colors::red);
        canvas.set_stroke_size(3);
        canvas.draw_line(1, 2, 3, 4);

        ASSERT_EQ(canvas.ops().size(), 3U);
        EXPECT_EQ(op_at<ops::set_stroke_color>(canvas, 0).value, colors::red);
        EXPECT_FLOAT_EQ(op_at<ops::set_stroke_size>(canvas, 1).value, 3.0F);
        EXPECT_EQ((op_at<ops::draw_line>(canvas, 2)), (ops::draw_line{1, 2, 3, 4}));
    }

    // AbstractCanvas.EnsureStrokePatternSet: the dash pattern is flushed lazily, before the next
    // stroked draw — and only once until it is dirtied again.
    TEST(recording_canvas_test, dash_pattern_is_flushed_before_next_stroked_draw_only_once)
    {
        recording_canvas canvas;
        canvas.set_stroke_size(4);
        canvas.set_stroke_dash_pattern({2, 1});
        canvas.draw_line(0, 0, 10, 0);
        canvas.draw_line(0, 5, 10, 5);

        ASSERT_EQ(canvas.ops().size(), 4U);
        const auto& flush = op_at<ops::set_stroke_dash_pattern>(canvas, 1);
        EXPECT_EQ(flush.pattern, (std::vector<float>{2, 1}));
        EXPECT_FLOAT_EQ(flush.dash_offset, 1.0F); // CanvasState.StrokeDashOffset default = 1
        EXPECT_FLOAT_EQ(flush.stroke_size, 4.0F);
        EXPECT_TRUE(std::holds_alternative<ops::draw_line>(canvas.ops()[2]));
        EXPECT_TRUE(std::holds_alternative<ops::draw_line>(canvas.ops()[3])); // no second flush
    }

    // C# StrokeDashOffset setter does NOT dirty the pattern — an offset change alone is not
    // re-flushed (quirk preserved).
    TEST(recording_canvas_test, dash_offset_change_alone_is_not_reflushed)
    {
        recording_canvas canvas;
        canvas.set_stroke_dash_pattern({2, 1});
        canvas.draw_line(0, 0, 10, 0); // flush + line
        canvas.set_stroke_dash_offset(5);
        canvas.draw_line(0, 5, 10, 5);

        ASSERT_EQ(canvas.ops().size(), 3U); // flush, line, line — no second flush
        EXPECT_TRUE(std::holds_alternative<ops::draw_line>(canvas.ops()[2]));
    }

    // Setting an identical pattern again does not dirty it (C# compares the array reference; the
    // port compares the value — both skip a redundant platform flush here).
    TEST(recording_canvas_test, same_dash_pattern_is_not_reflushed)
    {
        recording_canvas canvas;
        canvas.set_stroke_dash_pattern({2, 1});
        canvas.draw_line(0, 0, 10, 0);
        canvas.set_stroke_dash_pattern({2, 1});
        canvas.draw_line(0, 5, 10, 5);

        ASSERT_EQ(canvas.ops().size(), 3U);
    }

    // AbstractCanvas.DrawRoundedRectangle clamps the corner radius to half |height| / half |width|.
    TEST(recording_canvas_test, draw_rounded_rectangle_clamps_corner_radius)
    {
        recording_canvas canvas;
        canvas.draw_rounded_rectangle(0, 0, 100, 20, 30);
        EXPECT_FLOAT_EQ(op_at<ops::draw_rounded_rectangle>(canvas, 0).corner_radius, 10.0F);

        canvas.clear();
        canvas.draw_rounded_rectangle(0, 0, 100, -20, 30); // |height|/2 via Math.Abs
        EXPECT_FLOAT_EQ(op_at<ops::draw_rounded_rectangle>(canvas, 0).corner_radius, 10.0F);

        canvas.clear();
        canvas.draw_rounded_rectangle(0, 0, 8, 100, 30); // width is the tighter bound
        EXPECT_FLOAT_EQ(op_at<ops::draw_rounded_rectangle>(canvas, 0).corner_radius, 4.0F);
    }

    // FillRoundedRectangle is NOT clamped by AbstractCanvas (each platform fill clamps itself) —
    // the recorded op carries the raw radius.
    TEST(recording_canvas_test, fill_rounded_rectangle_records_raw_radius)
    {
        recording_canvas canvas;
        canvas.fill_rounded_rectangle(0, 0, 100, 20, 30);
        EXPECT_FLOAT_EQ(op_at<ops::fill_rounded_rectangle>(canvas, 0).corner_radius, 30.0F);
    }

    // AbstractCanvas.StrokeSize with LimitStrokeScaling: below the limit, the size is scaled up to
    // limit / scale (the state's uniform scale, sqrt|det| of the tracked transform).
    TEST(recording_canvas_test, stroke_size_respects_stroke_limit_under_scale)
    {
        recording_canvas canvas;
        canvas.set_limit_stroke_scaling(true);
        canvas.set_stroke_limit(4);
        canvas.scale(0.5F, 0.5F);
        canvas.set_stroke_size(2); // scaled: 0.5 * 2 = 1 < 4 -> size = 4 / 0.5 = 8

        EXPECT_FLOAT_EQ(op_at<ops::set_stroke_size>(canvas, 1).value, 8.0F);
    }

    TEST(recording_canvas_test, stroke_size_unlimited_without_flag)
    {
        recording_canvas canvas;
        canvas.scale(0.5F, 0.5F);
        canvas.set_stroke_size(2);
        EXPECT_FLOAT_EQ(op_at<ops::set_stroke_size>(canvas, 1).value, 2.0F);
    }

    // ---- the save/restore stack (AbstractCanvas.SaveState/RestoreState/ResetState) ----

    TEST(recording_canvas_test, restore_state_pops_the_saved_state)
    {
        recording_canvas canvas;
        canvas.set_stroke_dash_pattern({1});
        canvas.set_stroke_size(5);
        canvas.save_state();
        canvas.set_stroke_size(2);

        EXPECT_TRUE(canvas.restore_state());

        // The restored state carries stroke size 5 + the pattern; restore dirties the pattern, so
        // the next stroked draw flushes with the RESTORED values.
        canvas.draw_line(0, 0, 1, 1);
        const auto& flush = op_at<ops::set_stroke_dash_pattern>(canvas, canvas.ops().size() - 2);
        EXPECT_EQ(flush.pattern, (std::vector<float>{1}));
        EXPECT_FLOAT_EQ(flush.stroke_size, 5.0F);
    }

    TEST(recording_canvas_test, restore_state_on_empty_stack_returns_false_and_resets)
    {
        recording_canvas canvas;
        canvas.set_stroke_dash_pattern({3});
        canvas.set_stroke_size(7);

        EXPECT_FALSE(canvas.restore_state());

        // The fresh state has the defaults (stroke size 1, no pattern): the flush before the next
        // draw reports them.
        canvas.set_stroke_dash_pattern({2});
        canvas.draw_line(0, 0, 1, 1);
        const auto& flush = op_at<ops::set_stroke_dash_pattern>(canvas, canvas.ops().size() - 2);
        EXPECT_EQ(flush.pattern, (std::vector<float>{2}));
        EXPECT_FLOAT_EQ(flush.stroke_size, 1.0F);
    }

    TEST(recording_canvas_test, nested_save_restore_unwinds_in_order)
    {
        recording_canvas canvas;
        canvas.save_state();
        canvas.save_state();
        EXPECT_TRUE(canvas.restore_state());
        EXPECT_TRUE(canvas.restore_state());
        EXPECT_FALSE(canvas.restore_state());
    }

    TEST(recording_canvas_test, reset_state_unwinds_the_whole_stack)
    {
        recording_canvas canvas;
        canvas.save_state();
        canvas.save_state();
        canvas.reset_state();
        EXPECT_FALSE(canvas.restore_state());
    }

    // ---- transforms record the ICanvas-level arguments ----

    TEST(recording_canvas_test, records_transform_ops)
    {
        recording_canvas canvas;
        canvas.rotate(90);
        canvas.rotate(45, 10, 20);
        canvas.scale(2, 3);
        canvas.translate(7, 8);
        const matrix3x2 m{1, 0, 0, 1, 5, 5};
        canvas.concatenate_transform(m);

        ASSERT_EQ(canvas.ops().size(), 5U);
        EXPECT_FLOAT_EQ(op_at<ops::rotate>(canvas, 0).degrees, 90.0F);
        EXPECT_EQ((op_at<ops::rotate_at>(canvas, 1)), (ops::rotate_at{45, 10, 20}));
        EXPECT_EQ((op_at<ops::scale>(canvas, 2)), (ops::scale{2, 3}));
        EXPECT_EQ((op_at<ops::translate>(canvas, 3)), (ops::translate{7, 8}));
        EXPECT_EQ(op_at<ops::concatenate_transform>(canvas, 4).transform, m);
    }

    // ---- the remaining surface records faithfully ----

    TEST(recording_canvas_test, records_fill_clip_text_and_effect_ops)
    {
        recording_canvas canvas;
        canvas.set_fill_color(colors::green);
        canvas.fill_rectangle(1, 2, 3, 4);
        canvas.clip_rectangle(0, 0, 10, 10);
        canvas.subtract_from_clip(2, 2, 4, 4);
        canvas.set_shadow({5, 5}, 2.5F, colors::black);
        canvas.draw_string("hi", 1, 2, horizontal_alignment::center);

        ASSERT_EQ(canvas.ops().size(), 6U);
        EXPECT_EQ(op_at<ops::set_fill_color>(canvas, 0).value, colors::green);
        EXPECT_EQ((op_at<ops::fill_rectangle>(canvas, 1)), (ops::fill_rectangle{1, 2, 3, 4}));
        EXPECT_EQ((op_at<ops::clip_rectangle>(canvas, 2)), (ops::clip_rectangle{0, 0, 10, 10}));
        EXPECT_EQ((op_at<ops::subtract_from_clip>(canvas, 3)), (ops::subtract_from_clip{2, 2, 4, 4}));
        const auto& shadow = op_at<ops::set_shadow>(canvas, 4);
        EXPECT_FLOAT_EQ(shadow.blur, 2.5F);
        EXPECT_EQ(shadow.shadow_color, colors::black);
        const auto& text = op_at<ops::draw_string>(canvas, 5);
        EXPECT_EQ(text.value, "hi");
        EXPECT_EQ(text.h_align, horizontal_alignment::center);
    }

    TEST(recording_canvas_test, records_path_ops_with_winding)
    {
        maui::graphics::path_f path(0, 0);
        path.line_to(10, 0);
        path.line_to(10, 10);
        path.close();

        recording_canvas canvas;
        canvas.draw_path(path);
        canvas.fill_path(path, winding_mode::even_odd);
        canvas.clip_path(path);

        ASSERT_EQ(canvas.ops().size(), 3U);
        EXPECT_EQ(op_at<ops::draw_path>(canvas, 0).path, path);
        EXPECT_EQ(op_at<ops::fill_path>(canvas, 1).winding, winding_mode::even_odd);
        EXPECT_EQ(op_at<ops::clip_path>(canvas, 2).winding, winding_mode::non_zero); // default arg
    }

    TEST(recording_canvas_test, set_fill_paint_records_background_projection_and_null_is_white)
    {
        recording_canvas canvas;
        const maui::graphics::solid_paint solid(colors::orange);
        canvas.set_fill_paint(&solid, rect_f{0, 0, 10, 10});
        canvas.set_fill_paint(nullptr, rect_f{1, 1, 2, 2}); // C#: null -> Colors.White.AsPaint()

        EXPECT_EQ(op_at<ops::set_fill_paint>(canvas, 0).background_color, colors::orange);
        EXPECT_EQ(op_at<ops::set_fill_paint>(canvas, 1).background_color, colors::white);
    }

    TEST(recording_canvas_test, draw_text_snapshots_the_attributed_text)
    {
        maui::graphics::text::text_attributes bold;
        bold.set_bold(true);
        const maui::graphics::text::attributed_text value("Hello",
                                                          {maui::graphics::text::attributed_text_run(0, 5, bold)});

        recording_canvas canvas;
        canvas.draw_text(value, 1, 2, 30, 40);

        const auto& op = op_at<ops::draw_text>(canvas, 0);
        EXPECT_EQ(op.text, "Hello");
        ASSERT_EQ(op.runs.size(), 1U);
        EXPECT_TRUE(op.runs[0].attributes().get_bold());
    }

    // C# PictureCanvas.GetStringSize throws NotSupportedException.
    TEST(recording_canvas_test, get_string_size_is_not_supported)
    {
        const recording_canvas canvas;
        EXPECT_THROW((void)canvas.get_string_size("x", maui::graphics::font::default_font(), 12), std::logic_error);
    }

    TEST(recording_canvas_test, display_scale_defaults_to_one)
    {
        recording_canvas canvas;
        EXPECT_FLOAT_EQ(canvas.display_scale(), 1.0F);
        canvas.set_display_scale(2);
        EXPECT_FLOAT_EQ(canvas.display_scale(), 2.0F);
    }

    // ---- i_drawable: the contract draws onto any i_canvas ----

    class rect_drawable final : public maui::graphics::i_drawable
    {
    public:
        void draw(maui::graphics::i_canvas& canvas, const rect_f& dirty_rect) override
        {
            canvas.set_fill_color(colors::red);
            canvas.fill_rectangle(dirty_rect.x, dirty_rect.y, dirty_rect.width, dirty_rect.height);
        }
    };

    TEST(i_drawable_test, draws_through_the_canvas_contract)
    {
        recording_canvas canvas;
        rect_drawable drawable;
        drawable.draw(canvas, rect_f{0, 0, 20, 10});

        ASSERT_EQ(canvas.ops().size(), 2U);
        EXPECT_EQ((op_at<ops::fill_rectangle>(canvas, 1)), (ops::fill_rectangle{0, 0, 20, 10}));
    }

    // ---- canvas_state (CanvasState.cs) ----

    TEST(canvas_state_test, defaults_match_canvas_state)
    {
        const canvas_state state;
        EXPECT_TRUE(state.stroke_dash_pattern().empty());
        EXPECT_FLOAT_EQ(state.stroke_dash_offset(), 1.0F); // C# default = 1
        EXPECT_FLOAT_EQ(state.stroke_size(), 1.0F);
        EXPECT_EQ(state.transform(), matrix3x2::identity());
        EXPECT_FLOAT_EQ(state.scale(), 1.0F);
        EXPECT_FLOAT_EQ(state.scale_x(), 1.0F);
        EXPECT_FLOAT_EQ(state.scale_y(), 1.0F);
    }

    TEST(canvas_state_test, set_transform_derives_the_scales)
    {
        canvas_state state;
        state.set_transform(matrix3x2{2, 0, 0, 3, 0, 0});
        EXPECT_FLOAT_EQ(state.scale_x(), 2.0F);
        EXPECT_FLOAT_EQ(state.scale_y(), 3.0F);
        EXPECT_FLOAT_EQ(state.scale(), std::sqrt(6.0F)); // sqrt|det|
    }

    TEST(canvas_state_test, mirrored_transform_negates_scale_y)
    {
        canvas_state state;
        state.set_transform(matrix3x2{1, 0, 0, -1, 0, 0}); // det < 0
        EXPECT_FLOAT_EQ(state.scale_y(), -1.0F);
    }

    class observing_state final : public canvas_state
    {
    public:
        int transform_changes = 0;

    protected:
        void transform_changed() override
        {
            transform_changes++;
        }
    };

    TEST(canvas_state_test, transform_changed_fires_only_on_real_change)
    {
        observing_state state;
        state.set_transform(matrix3x2::identity()); // unchanged -> no notification
        EXPECT_EQ(state.transform_changes, 0);
        state.set_transform(matrix3x2{2, 0, 0, 2, 0, 0});
        EXPECT_EQ(state.transform_changes, 1);
    }

    // ---- matrix3x2 additions (System.Numerics factories + Matrix3x2Extensions) ----

    TEST(matrix3x2_canvas_test, factories_match_system_numerics)
    {
        EXPECT_EQ(matrix3x2::create_translation(3, 4), (matrix3x2{1, 0, 0, 1, 3, 4}));
        EXPECT_EQ(matrix3x2::create_scale(2, 5), (matrix3x2{2, 0, 0, 5, 0, 0}));

        const matrix3x2 rot = matrix3x2::create_rotation(std::numbers::pi_v<float> / 2);
        EXPECT_NEAR(rot.m11, 0.0F, 1e-6F);
        EXPECT_NEAR(rot.m12, 1.0F, 1e-6F);
        EXPECT_NEAR(rot.m21, -1.0F, 1e-6F);
        EXPECT_NEAR(rot.m22, 0.0F, 1e-6F);
    }

    TEST(matrix3x2_canvas_test, determinant_and_length_scale)
    {
        const matrix3x2 m{2, 0, 0, 3, 7, 9};
        EXPECT_FLOAT_EQ(m.get_determinant(), 6.0F);
        EXPECT_FLOAT_EQ(maui::graphics::get_length_scale(m), std::sqrt(6.0F));
    }

    // ---- canvas_defaults (CanvasDefaults.cs) ----

    TEST(canvas_defaults_test, values_match_canvas_defaults)
    {
        EXPECT_EQ(maui::graphics::canvas_defaults::default_shadow_color, color(0, 0, 0, 0.5F));
        EXPECT_FLOAT_EQ(maui::graphics::canvas_defaults::default_shadow_offset.width, 5.0F);
        EXPECT_FLOAT_EQ(maui::graphics::canvas_defaults::default_shadow_offset.height, 5.0F);
        EXPECT_FLOAT_EQ(maui::graphics::canvas_defaults::default_shadow_blur, 5.0F);
        EXPECT_FLOAT_EQ(maui::graphics::canvas_defaults::default_miter_limit, 10.0F);
    }

    // ---- graphics font (Font.cs) ----

    TEST(graphics_font_test, defaults_and_equality)
    {
        const maui::graphics::font default_font = maui::graphics::font::default_font();
        EXPECT_TRUE(default_font.is_default());
        EXPECT_EQ(default_font.weight(), maui::graphics::font_weights::normal);

        const maui::graphics::font bold = maui::graphics::font::default_bold();
        EXPECT_EQ(bold.weight(), maui::graphics::font_weights::bold);
        EXPECT_NE(default_font, bold);

        // C# Font.Equals: name comparison is OrdinalIgnoreCase.
        EXPECT_EQ(maui::graphics::font("Helvetica"), maui::graphics::font("helvetica"));
        EXPECT_NE(maui::graphics::font("Helvetica"), maui::graphics::font("Courier"));
    }
} // namespace
