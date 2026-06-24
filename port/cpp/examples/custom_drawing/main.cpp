// custom_drawing — drawing arbitrary shapes and text with a graphics_view.
//
// ONE concept: immediate-mode 2D drawing. A graphics_view renders whatever an i_drawable paints:
//   - subclass i_drawable and implement draw(canvas, dirty_rect). dirty_rect is the area to paint, in
//     device-independent units.
//   - the canvas is a stateful painter: set a fill/stroke/font color, then call a shape/text method.
//     set_fill_color + fill_* paints solid; set_stroke_color + set_stroke_size + draw_* outlines.
//   - colors come from maui::graphics::colors (named constants) or color{r,g,b,a}.
// Set the drawable on the graphics_view; the framework calls draw() whenever the view needs to repaint
// (and graphics_view::invalidate() requests a redraw, e.g. after the drawable's state changes).
//
// 100% PORTABLE C++: no platform headers. Same source builds + runs on headless, macOS, and iOS.

#include "maui/maui_main.hpp"

#include "maui/controls/application.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/graphics_view.hpp"
#include "maui/controls/window.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/i_canvas.hpp"
#include "maui/graphics/i_drawable.hpp"

#include <memory>

// ---- The drawable: paints a filled rectangle, an outlined circle, a line, and a caption ----
class shapes_drawable : public maui::graphics::i_drawable
{
public:
    void draw(maui::graphics::i_canvas& canvas, const maui::graphics::rect_f& dirty_rect) override
    {
        // A solid sky-blue background rectangle filling the whole area.
        canvas.set_fill_color(maui::graphics::colors::sky_blue);
        canvas.fill_rectangle(dirty_rect.x, dirty_rect.y, dirty_rect.width, dirty_rect.height);

        // A filled orange circle centered in the view (an ellipse with equal width/height).
        const float cx = dirty_rect.x + (dirty_rect.width / 2.0F);
        const float cy = dirty_rect.y + (dirty_rect.height / 2.0F);
        const float radius = 40.0F;
        canvas.set_fill_color(maui::graphics::colors::orange);
        canvas.fill_ellipse(cx - radius, cy - radius, radius * 2.0F, radius * 2.0F);

        // A dark-blue outline around that circle: stroke color + width, then the outline-only draw_*.
        canvas.set_stroke_color(maui::graphics::colors::dark_blue);
        canvas.set_stroke_size(3.0F);
        canvas.draw_ellipse(cx - radius, cy - radius, radius * 2.0F, radius * 2.0F);

        // A diagonal line across the top-left corner.
        canvas.draw_line(dirty_rect.x, dirty_rect.y, cx, cy);

        // A caption near the top, left-aligned at (x+12, y+12).
        canvas.set_font_color(maui::graphics::colors::black);
        canvas.set_font_size(16.0F);
        canvas.draw_string("Custom drawing", dirty_rect.x + 12.0F, dirty_rect.y + 12.0F,
                           maui::graphics::horizontal_alignment::left);
    }
};

class custom_drawing_app : public maui::controls::application
{
public:
    custom_drawing_app()
    {
        canvas_view_.set_drawable(std::make_shared<shapes_drawable>());

        page_.set_content(canvas_view_);
        window_.set_content(page_);
        window_.set_title("Custom Drawing");
    }

    maui::core::i_window* create_window() override
    {
        return &window_;
    }

private:
    maui::controls::window window_;
    maui::controls::content_page page_;
    maui::controls::graphics_view canvas_view_;
};

maui::hosting::maui_app_builder use_shared_maui_app(maui::hosting::maui_app_builder builder)
{
    builder.use_maui_app<custom_drawing_app>();
    return builder;
}
