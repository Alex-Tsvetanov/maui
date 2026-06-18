// Tests for the drawing-canvas image/pattern paint family — image_paint, i_pattern + abstract_pattern +
// picture_pattern + paint_pattern + pattern_paint, plus the recording_canvas set_fill_paint + Fill*
// integration for those kinds. The behavioral oracle is the C# source (no GPU-free C# unit tests cover
// these types): src/Graphics/src/Graphics/{ImagePaint,IPattern,AbstractPattern,PaintPattern,
// PicturePattern,PatternPaint,IPicture,Paint}.cs — characterization tests derived line by line.

#include <cstddef>
#include <variant>
#include <vector>

#include "maui/graphics/abstract_pattern.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/headless_image.hpp"
#include "maui/graphics/i_canvas.hpp"
#include "maui/graphics/i_picture.hpp"
#include "maui/graphics/image_paint.hpp"
#include "maui/graphics/paint_pattern.hpp"
#include "maui/graphics/pattern_paint.hpp"
#include "maui/graphics/picture_pattern.hpp"
#include "maui/graphics/recording_canvas.hpp"
#include "maui/graphics/rect_f.hpp"
#include <gtest/gtest.h>

namespace
{
    namespace ops = maui::graphics::canvas_ops;
    using maui::graphics::canvas_op;
    using maui::graphics::color;
    using maui::graphics::recording_canvas;
    using maui::graphics::rect_f;
    namespace colors = maui::graphics::colors;

    // A minimal i_picture for picture_pattern: records that draw() ran and exposes fixed dimensions.
    class fake_picture final : public maui::graphics::i_picture
    {
    public:
        fake_picture(float w, float h) : w_(w), h_(h)
        {
        }
        void draw(maui::graphics::i_canvas& canvas) override
        {
            drawn_ = true;
            // Emit a recognizable op so a caller can confirm the picture's commands reached the canvas.
            canvas.fill_rectangle(0, 0, w_, h_);
        }
        [[nodiscard]] float x() const override
        {
            return 0;
        }
        [[nodiscard]] float y() const override
        {
            return 0;
        }
        [[nodiscard]] float width() const override
        {
            return w_;
        }
        [[nodiscard]] float height() const override
        {
            return h_;
        }
        [[nodiscard]] bool drawn() const
        {
            return drawn_;
        }

    private:
        float w_;
        float h_;
        bool drawn_ = false;
    };

    // A bare i_pattern (not a paint_pattern) used to exercise the pattern_paint wrapping branch.
    class fake_pattern final : public maui::graphics::abstract_pattern
    {
    public:
        fake_pattern(float w, float h, float sx, float sy) : abstract_pattern(w, h, sx, sy)
        {
        }
        void draw(maui::graphics::i_canvas& canvas) override
        {
            canvas.fill_ellipse(0, 0, width(), height());
        }
    };

    template <class Op> const Op& op_at(const recording_canvas& canvas, std::size_t index)
    {
        static const Op fallback{};
        const std::vector<canvas_op>& list = canvas.ops();
        EXPECT_LT(index, list.size());
        if (index >= list.size())
        {
            return fallback;
        }
        const Op* found = std::get_if<Op>(&list[index]);
        EXPECT_NE(found, nullptr) << "unexpected op alternative at index " << index;
        return found != nullptr ? *found : fallback;
    }

    // ---- image_paint ----

    // C# ImagePaint.Image is a get/set reference; IsTransparent is always false; BackgroundColor stays
    // the base default.
    TEST(image_paint_test, holds_image_and_is_never_transparent)
    {
        maui::graphics::headless_image image(64, 48);
        maui::graphics::image_paint paint(&image);

        EXPECT_EQ(paint.image(), &image);
        EXPECT_FALSE(paint.is_transparent());
        EXPECT_EQ(paint.background_color(), color{}); // base default (opaque black)

        maui::graphics::headless_image other(10, 10);
        paint.set_image(&other);
        EXPECT_EQ(paint.image(), &other);
    }

    TEST(image_paint_test, default_image_is_null)
    {
        const maui::graphics::image_paint paint;
        EXPECT_EQ(paint.image(), nullptr);
        EXPECT_FALSE(paint.is_transparent());
    }

    // ---- picture_pattern / abstract_pattern ----

    // C# PicturePattern(picture, stepX, stepY) : base(picture.Width, picture.Height, stepX, stepY).
    TEST(picture_pattern_test, dimensions_from_picture_with_explicit_steps)
    {
        fake_picture picture(20, 30);
        const maui::graphics::picture_pattern pattern(&picture, 25, 35);

        EXPECT_FLOAT_EQ(pattern.width(), 20);
        EXPECT_FLOAT_EQ(pattern.height(), 30);
        EXPECT_FLOAT_EQ(pattern.step_x(), 25);
        EXPECT_FLOAT_EQ(pattern.step_y(), 35);
    }

    // C# PicturePattern(picture) : base(picture.Width, picture.Height) — dims double as the step.
    TEST(picture_pattern_test, dimensions_default_step_to_size)
    {
        fake_picture picture(20, 30);
        const maui::graphics::picture_pattern pattern(&picture);

        EXPECT_FLOAT_EQ(pattern.step_x(), 20);
        EXPECT_FLOAT_EQ(pattern.step_y(), 30);
    }

    // C# PicturePattern.Draw => _picture.Draw(canvas).
    TEST(picture_pattern_test, draw_replays_the_picture)
    {
        fake_picture picture(20, 30);
        maui::graphics::picture_pattern pattern(&picture);

        recording_canvas canvas;
        pattern.draw(canvas);

        EXPECT_TRUE(picture.drawn());
        EXPECT_EQ((op_at<ops::fill_rectangle>(canvas, 0)), (ops::fill_rectangle{0, 0, 20, 30}));
    }

    // ---- paint_pattern ----

    // C# PaintPattern forwards Width/Height/StepX/StepY to Wrapped.
    TEST(paint_pattern_test, dimensions_forward_to_wrapped)
    {
        fake_pattern wrapped(12, 14, 16, 18);
        const maui::graphics::paint_pattern pattern(&wrapped);

        EXPECT_FLOAT_EQ(pattern.width(), 12);
        EXPECT_FLOAT_EQ(pattern.height(), 14);
        EXPECT_FLOAT_EQ(pattern.step_x(), 16);
        EXPECT_FLOAT_EQ(pattern.step_y(), 18);
    }

    // C# PaintPattern dims => Wrapped?.X ?? 0 — a null wrapped pattern reports 0.
    TEST(paint_pattern_test, null_wrapped_reports_zero_dimensions)
    {
        const maui::graphics::paint_pattern pattern(nullptr);
        EXPECT_FLOAT_EQ(pattern.width(), 0);
        EXPECT_FLOAT_EQ(pattern.height(), 0);
        EXPECT_FLOAT_EQ(pattern.step_x(), 0);
        EXPECT_FLOAT_EQ(pattern.step_y(), 0);
    }

    // C# PaintPattern.Draw with no Paint: stroke + fill set to Colors.Black, then Wrapped.Draw.
    TEST(paint_pattern_test, draw_without_paint_uses_black_then_draws_wrapped)
    {
        fake_pattern wrapped(12, 14, 16, 18);
        maui::graphics::paint_pattern pattern(&wrapped);

        recording_canvas canvas;
        pattern.draw(canvas);

        EXPECT_EQ((op_at<ops::set_stroke_color>(canvas, 0)), (ops::set_stroke_color{colors::black}));
        EXPECT_EQ((op_at<ops::set_fill_color>(canvas, 1)), (ops::set_fill_color{colors::black}));
        EXPECT_EQ((op_at<ops::fill_ellipse>(canvas, 2)), (ops::fill_ellipse{0, 0, 12, 14}));
    }

    // C# PaintPattern.Draw with a Paint: stroke + fill set to the paint's ForegroundColor, then Wrapped.Draw.
    // (The BackgroundColor pre-fill branch guards on Alpha > 1, which is unreachable, so it never fires.)
    TEST(paint_pattern_test, draw_with_paint_uses_foreground_then_draws_wrapped)
    {
        fake_pattern wrapped(12, 14, 16, 18);
        maui::graphics::paint_pattern pattern(&wrapped);

        maui::graphics::pattern_paint owner;
        owner.set_background_color(colors::white); // opaque; the dead Alpha>1 branch stays inert
        owner.set_foreground_color(colors::red);
        pattern.set_paint(&owner);

        recording_canvas canvas;
        pattern.draw(canvas);

        // No leading fill_rectangle from the dead background branch — the first ops are the foreground set.
        EXPECT_EQ((op_at<ops::set_stroke_color>(canvas, 0)), (ops::set_stroke_color{colors::red}));
        EXPECT_EQ((op_at<ops::set_fill_color>(canvas, 1)), (ops::set_fill_color{colors::red}));
        EXPECT_EQ((op_at<ops::fill_ellipse>(canvas, 2)), (ops::fill_ellipse{0, 0, 12, 14}));
        EXPECT_EQ(canvas.ops().size(), 3U);
    }

    // ---- pattern_paint ----

    // C# Pattern setter wraps a non-PaintPattern value in a PaintPattern { Paint = this }.
    TEST(pattern_paint_test, set_pattern_wraps_a_bare_pattern_with_back_reference)
    {
        fake_pattern bare(12, 14, 16, 18);
        maui::graphics::pattern_paint paint;
        paint.set_pattern(&bare);

        auto* const effective = dynamic_cast<maui::graphics::paint_pattern*>(paint.pattern());
        ASSERT_NE(effective, nullptr); // the effective pattern is the wrapper, not the bare pattern
        EXPECT_EQ(effective->wrapped(), &bare);
        EXPECT_EQ(effective->paint(), &paint); // Paint = this
        // The wrapper forwards the bare pattern's dimensions.
        EXPECT_FLOAT_EQ(effective->width(), 12);
        EXPECT_FLOAT_EQ(effective->step_x(), 16);
    }

    // C# Pattern setter leaves an already-PaintPattern value untouched (no re-wrap, Paint not reset).
    TEST(pattern_paint_test, set_pattern_keeps_an_existing_paint_pattern)
    {
        fake_pattern bare(12, 14, 16, 18);
        maui::graphics::paint_pattern existing(&bare);

        maui::graphics::pattern_paint paint;
        paint.set_pattern(&existing);

        EXPECT_EQ(paint.pattern(), &existing); // referenced as-is, not re-wrapped
        EXPECT_EQ(existing.paint(), nullptr);  // its Paint back-reference is NOT set (C# leaves it)
    }

    TEST(pattern_paint_test, set_pattern_null_clears)
    {
        fake_pattern bare(12, 14, 16, 18);
        maui::graphics::pattern_paint paint;
        paint.set_pattern(&bare);
        ASSERT_NE(paint.pattern(), nullptr);

        paint.set_pattern(nullptr);
        EXPECT_EQ(paint.pattern(), nullptr);
    }

    // C# PatternPaint.IsTransparent: BackgroundColor null OR alpha < 1 => true; else ForegroundColor.Alpha < 1.
    TEST(pattern_paint_test, is_transparent_when_background_is_unset)
    {
        const maui::graphics::pattern_paint paint; // background never set => null => transparent
        EXPECT_TRUE(paint.is_transparent());
    }

    TEST(pattern_paint_test, is_transparent_when_background_alpha_below_one)
    {
        maui::graphics::pattern_paint paint;
        paint.set_background_color(colors::white.with_alpha(0.5F));
        paint.set_foreground_color(colors::black); // opaque
        EXPECT_TRUE(paint.is_transparent());
    }

    TEST(pattern_paint_test, is_transparent_follows_foreground_when_background_opaque)
    {
        maui::graphics::pattern_paint paint;
        paint.set_background_color(colors::white); // opaque background

        paint.set_foreground_color(colors::black); // opaque foreground => not transparent
        EXPECT_FALSE(paint.is_transparent());

        paint.set_foreground_color(colors::red.with_alpha(0.25F)); // translucent foreground => transparent
        EXPECT_TRUE(paint.is_transparent());
    }

    // ---- recording_canvas integration (set_fill_paint + Fill* do not break on the new kinds) ----

    // The headless recorder projects any paint to its background_color (image paint => base default) and
    // the Fill* op still records its geometry — the kind-specific dispatch is the native CoreGraphics path.
    TEST(recording_canvas_paint_kinds_test, image_paint_records_background_projection_and_fill_geometry)
    {
        maui::graphics::headless_image image(32, 32);
        const maui::graphics::image_paint paint(&image);

        recording_canvas canvas;
        canvas.set_fill_paint(&paint, rect_f{0, 0, 10, 10});
        canvas.fill_rectangle(1, 2, 3, 4);

        EXPECT_EQ(op_at<ops::set_fill_paint>(canvas, 0).background_color, color{}); // ImagePaint base default
        EXPECT_EQ((op_at<ops::fill_rectangle>(canvas, 1)), (ops::fill_rectangle{1, 2, 3, 4}));
    }

    TEST(recording_canvas_paint_kinds_test, pattern_paint_records_background_projection_and_fill_geometry)
    {
        fake_pattern bare(8, 8, 8, 8);
        maui::graphics::pattern_paint paint;
        paint.set_background_color(colors::blue);
        paint.set_pattern(&bare);

        recording_canvas canvas;
        canvas.set_fill_paint(&paint, rect_f{0, 0, 20, 20});
        canvas.fill_ellipse(5, 6, 7, 8);

        EXPECT_EQ(op_at<ops::set_fill_paint>(canvas, 0).background_color, colors::blue);
        EXPECT_EQ((op_at<ops::fill_ellipse>(canvas, 1)), (ops::fill_ellipse{5, 6, 7, 8}));
    }
} // namespace
