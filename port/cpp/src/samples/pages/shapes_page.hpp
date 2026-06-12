#pragma once
// shapes_page — a self-contained demo page for the W2-23 drawing set: a vertical stack hosting a
// graphics_view (a custom drawable painting through the canvas stack), a box_view (color + corner
// radius), and the shape family — a rounded rectangle, an ellipse, a dashed line, a star polygon
// (EvenOdd fill rule) and a path built from markup ("M…") with a render transform — plus a readout
// label echoing the graphics_view interactions (the C# gallery-page convention, code-first).
//
// The page OWNS its whole element tree (the containers_page pattern). It is backend-agnostic — a
// sample main attaches handlers bottom-up via the hosting layer and hosts page() in a window; the
// headless/apple/ios test trees exercise the same wiring directly.

#include <array>
#include <cstdio>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "maui/controls/box_view.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/graphics_view.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/shapes/ellipse.hpp"
#include "maui/controls/shapes/fill_rule.hpp"
#include "maui/controls/shapes/line.hpp"
#include "maui/controls/shapes/path.hpp"
#include "maui/controls/shapes/path_markup_parser.hpp"
#include "maui/controls/shapes/polygon.hpp"
#include "maui/controls/shapes/rectangle.hpp"
#include "maui/controls/shapes/rotate_transform.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/corner_radius.hpp"
#include "maui/graphics/i_canvas.hpp"
#include "maui/graphics/i_drawable.hpp"
#include "maui/graphics/point_f.hpp"
#include "maui/graphics/rect_f.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    // The demo drawable: a framed diagonal cross (every op flows through the W1-13 canvas core).
    class cross_drawable final : public maui::graphics::i_drawable
    {
    public:
        void draw(maui::graphics::i_canvas& canvas, const maui::graphics::rect_f& dirty_rect) override
        {
            canvas.set_stroke_size(2);
            canvas.set_stroke_color(maui::graphics::color(0.16F, 0.50F, 0.73F));
            canvas.draw_rectangle(dirty_rect.x, dirty_rect.y, dirty_rect.width, dirty_rect.height);
            canvas.draw_line(dirty_rect.left(), dirty_rect.top(), dirty_rect.right(), dirty_rect.bottom());
            canvas.draw_line(dirty_rect.left(), dirty_rect.bottom(), dirty_rect.right(), dirty_rect.top());
        }
    };

    class shapes_page
    {
    public:
        shapes_page()
        {
            page_.set_title("Shapes");
            stack_.set_spacing(12);

            readout_.set_text("Touch the canvas");

            // graphics_view — a custom drawable + the interaction events into the readout.
            canvas_.set_drawable(std::make_shared<cross_drawable>());
            canvas_.set_height_request(120);
            canvas_.start_interaction.connect([this](const std::vector<maui::graphics::point_f>& points) {
                if (!points.empty())
                {
                    std::array<char, 64> buffer{};
                    (void)std::snprintf(buffer.data(), buffer.size(), "Touched at %.0f, %.0f",
                                        static_cast<double>(points[0].x), static_cast<double>(points[0].y));
                    readout_.set_text(buffer.data());
                }
            });

            // box_view — a rounded solid block.
            box_.set_color(maui::graphics::color(0.86F, 0.20F, 0.27F));
            box_.set_corner_radius(maui::graphics::corner_radius(8));
            box_.set_height_request(40);

            // rectangle — rounded, filled + stroked.
            rounded_rect_.set_radius_x(10);
            rounded_rect_.set_radius_y(10);
            rounded_rect_.set_fill(
                std::make_shared<maui::graphics::solid_paint>(maui::graphics::color(1.0F, 0.76F, 0.03F)));
            rounded_rect_.set_stroke(
                std::make_shared<maui::graphics::solid_paint>(maui::graphics::color(0.13F, 0.13F, 0.13F)));
            rounded_rect_.set_stroke_thickness(2);
            rounded_rect_.set_height_request(60);

            // ellipse — filled only.
            blob_.set_fill(std::make_shared<maui::graphics::solid_paint>(maui::graphics::color(0.18F, 0.65F, 0.41F)));
            blob_.set_height_request(60);

            // line — dashed stroke.
            divider_.set_x2(240);
            divider_.set_stroke(
                std::make_shared<maui::graphics::solid_paint>(maui::graphics::color(0.45F, 0.45F, 0.45F)));
            divider_.set_stroke_thickness(2);
            divider_.set_stroke_dash_array({4.0, 2.0});

            // polygon — the classic five-point star, EvenOdd so the core stays open.
            star_.set_points({{96, 0}, {153, 192}, {0, 72}, {192, 72}, {39, 192}});
            star_.set_fill_rule(maui::controls::shapes::fill_rule::even_odd);
            star_.set_fill(std::make_shared<maui::graphics::solid_paint>(maui::graphics::color(0.55F, 0.36F, 0.80F)));
            star_.set_height_request(80);

            // path — geometry from markup + a render transform.
            auto geometry = std::make_shared<maui::controls::shapes::path_geometry>();
            maui::controls::shapes::parse_path_figure_collection(geometry->figures(),
                                                                 "M0,0 L60,0 C70,10 70,30 60,40 L0,40 Z");
            marker_.set_data(std::move(geometry));
            marker_.set_render_transform(std::make_shared<maui::controls::shapes::rotate_transform>(8.0));
            marker_.set_fill(std::make_shared<maui::graphics::solid_paint>(maui::graphics::color(0.95F, 0.55F, 0.20F)));
            marker_.set_height_request(60);

            stack_.add(readout_);
            stack_.add(canvas_);
            stack_.add(box_);
            stack_.add(rounded_rect_);
            stack_.add(blob_);
            stack_.add(divider_);
            stack_.add(star_);
            stack_.add(marker_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }
        [[nodiscard]] maui::controls::graphics_view& canvas()
        {
            return canvas_;
        }
        [[nodiscard]] maui::controls::box_view& box()
        {
            return box_;
        }
        [[nodiscard]] maui::controls::shapes::rectangle& rounded_rect()
        {
            return rounded_rect_;
        }
        [[nodiscard]] maui::controls::shapes::ellipse& blob()
        {
            return blob_;
        }
        [[nodiscard]] maui::controls::shapes::line& divider()
        {
            return divider_;
        }
        [[nodiscard]] maui::controls::shapes::polygon& star()
        {
            return star_;
        }
        [[nodiscard]] maui::controls::shapes::path& marker()
        {
            return marker_;
        }

    private:
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label readout_;
        maui::controls::graphics_view canvas_;
        maui::controls::box_view box_;
        maui::controls::shapes::rectangle rounded_rect_;
        maui::controls::shapes::ellipse blob_;
        maui::controls::shapes::line divider_;
        maui::controls::shapes::polygon star_;
        maui::controls::shapes::path marker_;
    };
} // namespace maui::samples
