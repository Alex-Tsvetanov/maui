#pragma once
// maui::samples::invalidate_brush_page — ports InvalidateBrushGallery.xaml
//
// A code-first port of the MAUI Shapes sub-gallery
// Pages/Controls/ShapesGalleries/InvalidateBrushGallery.xaml ("Invalidate Brushes Playground"): a
// VerticalStackLayout (Padding 12) with —
//   - a "Change color" Button (HorizontalOptions=Start), and
//   - a horizontal Line (X1=0, X2=150, StrokeThickness=4, HorizontalOptions=Start)
// — both painted from ONE shared solid brush. Tapping the button advances the brush color through the
// cycle Green → Red → Blue (and wraps), and BOTH the line's stroke and the button's background repaint to
// the new color. This is the brush-invalidation demo: one brush, two consumers, repaint on mutation.
//
// The page OWNS its whole element tree (the shapes_demo_page / value_controls_page pattern). It is
// backend-agnostic — a sample main attaches handlers bottom-up via the hosting layer and hosts page() in a
// window; the headless/apple/ios test trees exercise the same wiring directly.
//
// PORT NOTES (faithful best-effort, never invented):
//   note: the C# page holds ONE SolidColorBrush (_brush) whose Color it mutates in place; the line's
//         Stroke and the button's Background both point at that single brush object, so the in-place
//         Color change repaints both (the WeakBrushChangedProxy resubscription). The port does not model
//         the brush version-counter/proxy resubscription (shape.hpp / view.hpp invalidation rule), so the
//         equivalent here is to build a FRESH solid_paint per color and re-apply it to both consumers —
//         a distinct paint instance fires the change, exactly as the comment in view.hpp / shape.hpp
//         documents. The observable behavior (both repaint to the cycled color on each tap) is identical.
//   note: the color cycle is the C# `_colors = { Green, Red, Blue }` with `_colorIndex = -1`, so the first
//         UpdateBrush() (called in the ctor before any tap) yields Green — the initial state — and each
//         tap advances + wraps. This port reproduces that exact ordering and initial state.
//   note: the button's Background is a paint (VisualElement.Background); set_background over a solid_paint
//         is the documented Brush→Paint bridge (there is no set_background_color on a view). The line's
//         Stroke takes the same paint via set_stroke.
//   note: HorizontalOptions=Start (on the button and the line) and the layout Padding=12 are layout
//         options the port does not model on these controls today, so they are omitted (best-effort);
//         the controls, their wiring, and the color cycle are what the page demonstrates. A status readout
//         label is added (not in the C# XAML) to surface the current color name for the headless tests and
//         the static capture — it never changes the demonstrated brush-invalidation behavior.

#include <array>
#include <memory>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/shapes/line.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class invalidate_brush_page
    {
    public:
        invalidate_brush_page()
        {
            page_.set_title("Invalidate Brushes Playground");
            stack_.set_spacing(4); // C# per-control Margin=4 → an approximate stack spacing (best-effort)

            // The "Change color" button — each tap advances the shared brush color and repaints both.
            change_color_.set_text("Change color");
            change_color_.set_horizontal_layout_alignment(
                maui::core::layout_alignment::start); // C# HorizontalOptions=Start
            change_color_.clicked.connect([this]() {
                advance_color();
                apply_brush();
            });
            stack_.add(change_color_);

            // The horizontal line: (0,0)->(150,0), thickness 4 — its stroke is the same cycled color.
            line_.set_x1(0);
            line_.set_y1(0);
            line_.set_x2(150);
            line_.set_y2(0);
            line_.set_stroke_thickness(4);
            line_.set_width_request(150);
            line_.set_height_request(4);
            stack_.add(line_);

            // A status readout (port addition — surfaces the current color name; see header note).
            stack_.add(readout_);

            // C# ctor: UpdateBrush() runs once (index -1 → 0 → Green) BEFORE the first tap, painting the
            // initial state; reproduce that initial paint here.
            advance_color();
            apply_brush();

            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (the stack's children in add()-order, then the
        // stack, then the page), then re-host the tree built in the ctor (gallery_attach.hpp). The generic
        // auto& parameter preserves each member's concrete static type.
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, change_color_, "change_color_");
            gallery_attach_one(app, line_, "line_");
            gallery_attach_one(app, readout_, "readout_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(stack_); // stack hosts the button + line + readout
            gallery_rehost_content(page_); // page hosts the stack

            // The handlers exist now, so replay the initial brush so both consumers paint the start color.
            apply_brush();
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::button& change_color()
        {
            return change_color_;
        }
        [[nodiscard]] maui::controls::shapes::line& line()
        {
            return line_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }
        // The current cycle color (for the headless tests to assert the Green→Red→Blue ordering).
        [[nodiscard]] maui::graphics::color current_color() const
        {
            return colors().at(static_cast<std::size_t>(color_index_));
        }

    private:
        // C# `_colors = { Colors.Green, Colors.Red, Colors.Blue }`. The named-color constants are
        // `inline const` (not constexpr — from_uint is non-constexpr), so this is a function-local static
        // array rather than a constexpr member.
        static const std::array<maui::graphics::color, 3>& colors()
        {
            static const std::array<maui::graphics::color, 3> table{
                maui::graphics::colors::green, maui::graphics::colors::red, maui::graphics::colors::blue};
            return table;
        }

        // C# UpdateBrush(): `if (++_colorIndex >= _colors.Length) _colorIndex = 0;`.
        void advance_color()
        {
            if (++color_index_ >= static_cast<int>(colors().size()))
            {
                color_index_ = 0;
            }
        }

        // Build a FRESH solid_paint for the current color and apply it to BOTH the line's stroke and the
        // button's background (a distinct paint instance fires the repaint — see header note). Update the
        // readout so the current color is observable.
        void apply_brush()
        {
            line_.set_stroke(std::make_shared<maui::graphics::solid_paint>(current_color()));
            change_color_.set_background(std::make_shared<maui::graphics::solid_paint>(current_color()));
            readout_.set_text(std::string{"Brush color: "} + color_name());
        }

        [[nodiscard]] const char* color_name() const
        {
            switch (color_index_)
            {
                case 0:
                    return "Green";
                case 1:
                    return "Red";
                case 2:
                    return "Blue";
                default:
                    return "?";
            }
        }

        // C# `_colorIndex = -1` — the first advance_color() (in the ctor) yields index 0 (Green).
        int color_index_ = -1;

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::button change_color_;
        maui::controls::shapes::line line_;
        maui::controls::label readout_;
    };
} // namespace maui::samples
