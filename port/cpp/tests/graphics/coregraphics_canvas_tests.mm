// Apple-backend tests for the shared CoreGraphics canvas (src/platform/apple_shared/
// coregraphics_canvas.mm) — run for MAUI_BACKEND=apple AND =ios (the canvas is one TU for both).
// Strategy: render into a CGBitmapContext and READ BACK pixels at sample points. The context's CTM
// is pre-flipped to a top-left origin, mirroring how PlatformCanvas is hosted in MAUI (UIKit /
// flipped NSView contexts), so canvas coordinates match buffer rows directly.
// Compiled as Objective-C++ with ARC.

#import <CoreGraphics/CoreGraphics.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "coregraphics_canvas.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/font.hpp"
#include "maui/graphics/i_canvas.hpp"
#include "maui/graphics/i_graphics_image.hpp"
#include "maui/graphics/linear_gradient_paint.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/rect_f.hpp"
#include "maui/graphics/size_f.hpp"
#include "maui/graphics/solid_paint.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::platform::apple_shared::coregraphics_canvas;
    namespace colors = maui::graphics::colors;

    constexpr std::size_t k_size = 100; // square bitmap, pixels

    struct rgba
    {
        std::uint8_t r, g, b, a;
    };

    // A drawing-layer image backed by a real CGImage (the slice coregraphics_canvas::draw_image
    // needs): to_platform_image() hands back the CGImageRef the blit casts. Owns the CGImage (the
    // canvas only reads it). Stands in for the not-yet-ported production PlatformImage wrapper.
    class coregraphics_image final : public maui::graphics::i_graphics_image
    {
    public:
        coregraphics_image(CGImageRef image, float width, float height)
            : image_(CGImageRetain(image)), width_(width), height_(height)
        {
        }
        ~coregraphics_image() override
        {
            CGImageRelease(image_);
        }
        coregraphics_image(const coregraphics_image&) = delete;
        coregraphics_image(coregraphics_image&&) = delete;
        coregraphics_image& operator=(const coregraphics_image&) = delete;
        coregraphics_image& operator=(coregraphics_image&&) = delete;

        [[nodiscard]] float width() const override
        {
            return width_;
        }
        [[nodiscard]] float height() const override
        {
            return height_;
        }
        [[nodiscard]] void* to_platform_image() const override
        {
            return image_;
        }
        void draw(maui::graphics::i_canvas& canvas, const maui::graphics::rect_f& dirty_rect) override
        {
            canvas.draw_image(*this, dirty_rect.left(), dirty_rect.top(), dirty_rect.width, dirty_rect.height);
        }

    private:
        CGImageRef image_;
        float width_;
        float height_;
    };

    // Builds a width x height RGBA8888 CGImage whose TOP half is `top` and BOTTOM half is `bottom`
    // (image row 0 = top), so a draw can detect an upside-down blit. Caller owns the returned image.
    CGImageRef make_two_tone_image(std::size_t width, std::size_t height, rgba top, rgba bottom)
    {
        std::vector<std::uint8_t> pixels(width * height * 4, 0);
        for (std::size_t row = 0; row < height; row++)
        {
            const rgba& tone = row < height / 2 ? top : bottom;
            for (std::size_t col = 0; col < width; col++)
            {
                const std::size_t offset = ((row * width) + col) * 4;
                pixels[offset] = tone.r;
                pixels[offset + 1] = tone.g;
                pixels[offset + 2] = tone.b;
                pixels[offset + 3] = tone.a;
            }
        }

        CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
        CGContextRef ctx =
            CGBitmapContextCreate(pixels.data(), width, height, 8, width * 4, space, kCGImageAlphaPremultipliedLast);
        CGImageRef image = CGBitmapContextCreateImage(ctx);
        CGContextRelease(ctx);
        CGColorSpaceRelease(space);
        return image;
    }

    // A zero-initialized RGBA8888 (premultiplied-last) DeviceRGB bitmap context with a top-left
    // origin (CTM flipped like the UIKit hosts the C# PlatformCanvas runs under).
    class coregraphics_canvas_pixels : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            buffer_.assign(k_size * k_size * 4, 0);
            CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
            context_ = CGBitmapContextCreate(buffer_.data(), k_size, k_size, 8, k_size * 4, space,
                                             kCGImageAlphaPremultipliedLast);
            CGColorSpaceRelease(space);
            ASSERT_NE(context_, nullptr);

            // Top-left origin: canvas y grows downward, row 0 of the buffer is canvas y = 0.
            CGContextTranslateCTM(context_, 0, k_size);
            CGContextScaleCTM(context_, 1, -1);

            canvas_.set_context(context_);
        }

        void TearDown() override
        {
            CGContextRelease(context_);
        }

        [[nodiscard]] rgba pixel_at(std::size_t x, std::size_t y) const
        {
            const std::size_t offset = ((y * k_size) + x) * 4;
            return {.r = buffer_[offset], .g = buffer_[offset + 1], .b = buffer_[offset + 2], .a = buffer_[offset + 3]};
        }

        [[nodiscard]] bool any_ink_in(std::size_t x0, std::size_t y0, std::size_t x1, std::size_t y1) const
        {
            for (std::size_t y = y0; y < y1; y++)
            {
                for (std::size_t x = x0; x < x1; x++)
                {
                    if (pixel_at(x, y).a != 0)
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        std::vector<std::uint8_t> buffer_;
        CGContextRef context_ = nullptr;
        coregraphics_canvas canvas_;
    };

    TEST_F(coregraphics_canvas_pixels, fill_rectangle_paints_solid_color)
    {
        canvas_.set_fill_color(colors::red);
        canvas_.fill_rectangle(10, 10, 30, 30);

        const rgba inside = pixel_at(20, 20);
        EXPECT_EQ(inside.r, 255);
        EXPECT_EQ(inside.g, 0);
        EXPECT_EQ(inside.b, 0);
        EXPECT_EQ(inside.a, 255);

        EXPECT_EQ(pixel_at(60, 60).a, 0); // untouched outside the rect
    }

    TEST_F(coregraphics_canvas_pixels, draw_line_strokes_with_stroke_color_and_size)
    {
        canvas_.set_stroke_color(colors::blue);
        canvas_.set_stroke_size(4);
        canvas_.draw_line(0, 50, 100, 50);

        const rgba on_line = pixel_at(50, 50);
        EXPECT_EQ(on_line.b, 255);
        EXPECT_EQ(on_line.r, 0);
        EXPECT_EQ(pixel_at(50, 10).a, 0); // far from the line
    }

    TEST_F(coregraphics_canvas_pixels, clip_rectangle_limits_the_fill)
    {
        canvas_.clip_rectangle(0, 0, 20, 20);
        canvas_.set_fill_color(colors::green);
        canvas_.fill_rectangle(0, 0, 100, 100);

        EXPECT_NE(pixel_at(10, 10).g, 0); // inside the clip
        EXPECT_EQ(pixel_at(50, 50).a, 0); // clipped away
    }

    TEST_F(coregraphics_canvas_pixels, subtract_from_clip_punches_a_hole)
    {
        canvas_.subtract_from_clip(40, 40, 20, 20);
        canvas_.set_fill_color(colors::red);
        canvas_.fill_rectangle(0, 0, 100, 100);

        EXPECT_EQ(pixel_at(50, 50).a, 0); // the punched hole
        EXPECT_EQ(pixel_at(10, 10).r, 255);
    }

    TEST_F(coregraphics_canvas_pixels, save_restore_unwinds_translation)
    {
        canvas_.save_state();
        canvas_.translate(40, 40);
        canvas_.set_fill_color(colors::red);
        canvas_.fill_rectangle(0, 0, 10, 10);
        EXPECT_TRUE(canvas_.restore_state());

        canvas_.set_fill_color(colors::blue);
        canvas_.fill_rectangle(0, 0, 10, 10);

        EXPECT_EQ(pixel_at(45, 45).r, 255); // translated fill
        EXPECT_EQ(pixel_at(5, 5).b, 255);   // untranslated after restore
    }

    TEST_F(coregraphics_canvas_pixels, fill_ellipse_fills_center_not_corner)
    {
        canvas_.set_fill_color(colors::red);
        canvas_.fill_ellipse(10, 10, 40, 40);

        EXPECT_EQ(pixel_at(30, 30).r, 255); // ellipse center
        EXPECT_EQ(pixel_at(12, 12).a, 0);   // bounding-box corner stays empty
    }

    TEST_F(coregraphics_canvas_pixels, fill_path_fills_the_triangle_interior)
    {
        maui::graphics::path_f path(10, 10);
        path.line_to(90, 10);
        path.line_to(10, 90);
        path.close();

        canvas_.set_fill_color(colors::purple);
        canvas_.fill_path(path, maui::graphics::winding_mode::non_zero);

        EXPECT_NE(pixel_at(20, 20).a, 0); // inside the triangle
        EXPECT_EQ(pixel_at(80, 80).a, 0); // outside
    }

    TEST_F(coregraphics_canvas_pixels, linear_gradient_fill_shades_across_the_rect)
    {
        maui::graphics::linear_gradient_paint paint(maui::graphics::point{0, 0}, maui::graphics::point{1, 0});
        paint.set_gradient_stops({{0.0F, colors::black}, {1.0F, colors::white}});

        canvas_.set_fill_paint(&paint, maui::graphics::rect_f{0, 0, 100, 100});
        canvas_.fill_rectangle(0, 0, 100, 100);

        const rgba left = pixel_at(2, 50);
        const rgba right = pixel_at(97, 50);
        EXPECT_EQ(left.a, 255);
        EXPECT_EQ(right.a, 255);
        EXPECT_LT(left.r, 30);   // near-black at the start
        EXPECT_GT(right.r, 225); // near-white at the end
        EXPECT_LT(left.r, right.r);
    }

    // C# FillColor/SetFillPaint: a new solid fill clears the staged gradient.
    TEST_F(coregraphics_canvas_pixels, set_fill_color_clears_a_staged_gradient)
    {
        maui::graphics::linear_gradient_paint paint(maui::graphics::point{0, 0}, maui::graphics::point{1, 0});
        paint.set_gradient_stops({{0.0F, colors::black}, {1.0F, colors::white}});
        canvas_.set_fill_paint(&paint, maui::graphics::rect_f{0, 0, 100, 100});

        canvas_.set_fill_color(colors::red); // clears the gradient
        canvas_.fill_rectangle(0, 0, 100, 100);

        EXPECT_EQ(pixel_at(2, 50).r, 255);
        EXPECT_EQ(pixel_at(97, 50).r, 255); // uniform solid red, no gradient ramp
        EXPECT_EQ(pixel_at(97, 50).g, 0);
    }

    TEST_F(coregraphics_canvas_pixels, draw_string_renders_ink_near_the_baseline)
    {
        canvas_.set_font_color(colors::black);
        canvas_.set_font(maui::graphics::font::default_font());
        canvas_.set_font_size(20);
        canvas_.draw_string("Hello", 10, 50, maui::graphics::horizontal_alignment::left);

        EXPECT_TRUE(any_ink_in(10, 30, 90, 55));
        EXPECT_FALSE(any_ink_in(0, 70, 100, 100)); // far below the baseline stays empty
    }

    TEST_F(coregraphics_canvas_pixels, get_string_size_measures_nonzero_extent)
    {
        const maui::graphics::size_f size = canvas_.get_string_size("Hello", maui::graphics::font::default_font(), 20);
        EXPECT_GT(size.width, 0.0F);
        EXPECT_GT(size.height, 0.0F);

        const maui::graphics::size_f wider =
            canvas_.get_string_size("Hello Hello", maui::graphics::font::default_font(), 20);
        EXPECT_GT(wider.width, size.width);

        const maui::graphics::size_f aligned = canvas_.get_string_size("Hello", maui::graphics::font::default_font(),
                                                                       20, maui::graphics::horizontal_alignment::left,
                                                                       maui::graphics::vertical_alignment::top);
        EXPECT_GT(aligned.width, 0.0F);
        EXPECT_GT(aligned.height, 0.0F);
    }

    // C# PlatformCanvas.DrawImage blits the platform image at (x, y) sized (w, h), flipping the CTM so
    // the top-left-origin canvas renders the bottom-up CGImage upright. The image is red on top, blue
    // on the bottom; after the draw the red must land in the upper half of the target rect and blue in
    // the lower half — an upside-down blit (a missing/wrong flip) would swap them.
    TEST_F(coregraphics_canvas_pixels, draw_image_blits_upright_at_canvas_coordinates)
    {
        constexpr rgba red{.r = 255, .g = 0, .b = 0, .a = 255};
        constexpr rgba blue{.r = 0, .g = 0, .b = 255, .a = 255};
        CGImageRef cg_image = make_two_tone_image(40, 40, red, blue);
        const coregraphics_image image(cg_image, 40, 40);
        CGImageRelease(cg_image); // the wrapper retained it

        canvas_.draw_image(image, 10, 10, 40, 40);

        // Upper half of the target rect (canvas y 10..30) is the image top = red.
        const rgba upper = pixel_at(30, 18);
        EXPECT_EQ(upper.r, 255);
        EXPECT_EQ(upper.b, 0);
        // Lower half (canvas y 30..50) is the image bottom = blue.
        const rgba lower = pixel_at(30, 42);
        EXPECT_EQ(lower.b, 255);
        EXPECT_EQ(lower.r, 0);

        // Nothing painted outside the target rect.
        EXPECT_EQ(pixel_at(70, 70).a, 0);
        EXPECT_EQ(pixel_at(5, 5).a, 0);
    }

    // A null platform representation (the headless image case) is a no-op — the guard mirrors C#'s
    // `if (platformRepresentation != null)`.
    TEST_F(coregraphics_canvas_pixels, draw_image_with_null_platform_image_is_a_noop)
    {
        class null_image final : public maui::graphics::i_graphics_image
        {
        public:
            [[nodiscard]] float width() const override
            {
                return 10;
            }
            [[nodiscard]] float height() const override
            {
                return 10;
            }
            [[nodiscard]] void* to_platform_image() const override
            {
                return nullptr;
            }
            void draw(maui::graphics::i_canvas& canvas, const maui::graphics::rect_f& dirty_rect) override
            {
                canvas.draw_image(*this, dirty_rect.left(), dirty_rect.top(), dirty_rect.width, dirty_rect.height);
            }
        } const image;

        canvas_.draw_image(image, 0, 0, 100, 100);

        EXPECT_FALSE(any_ink_in(0, 0, 100, 100)); // nothing drawn
    }
} // namespace
