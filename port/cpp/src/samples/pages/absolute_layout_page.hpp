#pragma once
// maui::samples::absolute_layout_page — ports AbsoluteLayoutPage.xaml
//
// A self-contained, code-first demo of the AbsoluteLayout control. It mirrors the C# gallery page
// (Pages/Layouts/AbsoluteLayoutPage.xaml): four colored BoxViews pinned to the edge-midpoints and two
// Labels, every child positioned with PROPORTIONAL X/Y (PositionProportional) and ABSOLUTE width/height,
// plus one Label sized to its own content (AutoSize). It demonstrates the two halves of the
// absolute_layout contract:
//   - per-child LayoutBounds (rect{x, y, width, height}) and LayoutFlags (which components are a fraction
//     of the container vs an absolute device length) — here every child is PositionProportional, so X/Y
//     in [0,1] and width/height in device units; the "AutoSized" label additionally uses auto_size for
//     width+height so it shrinks to its measured text;
//   - the four BoxViews land at the top-center, left-center, right-center and bottom-center; the two
//     Labels sit at the dead center and at (0.2, 0.7).
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same wiring directly.
//
// note: the C# children also carry HorizontalOptions/HorizontalTextAlignment/Background; the cross-platform
//       LayoutBounds/LayoutFlags placement is the point of THIS page, so those secondary visual hints are
//       ported where a direct headless-safe setter exists (text color, background paint, horizontal text
//       alignment) and noted where one does not (HorizontalOptions has no view-base setter in the port).

#include <memory>

#include "maui/controls/absolute_layout.hpp"
#include "maui/controls/box_view.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"
#include "maui/layouts/absolute_layout_flags.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class absolute_layout_page
    {
    public:
        absolute_layout_page()
        {
            page_.set_title("AbsoluteLayout");

            // Each child: Color/Text, then the proportional-position bounds + the PositionProportional flag
            // (X and Y are fractions of the container; width and height are absolute device lengths).
            using maui::layouts::absolute_layout_flags;
            const auto position_proportional = absolute_layout_flags::position_proportional;

            // Top-center blue bar — LayoutBounds="0.5,0,100,25".
            top_box_.set_color(maui::graphics::colors::blue);
            layout_.add(top_box_);
            layout_.set_layout_bounds(top_box_, maui::graphics::rect{0.5, 0.0, 100.0, 25.0});
            layout_.set_layout_flags(top_box_, position_proportional);

            // Left-center green bar — LayoutBounds="0,0.5,25,100".
            left_box_.set_color(maui::graphics::colors::green);
            layout_.add(left_box_);
            layout_.set_layout_bounds(left_box_, maui::graphics::rect{0.0, 0.5, 25.0, 100.0});
            layout_.set_layout_flags(left_box_, position_proportional);

            // Right-center red bar — LayoutBounds="1,0.5,25,100".
            right_box_.set_color(maui::graphics::colors::red);
            layout_.add(right_box_);
            layout_.set_layout_bounds(right_box_, maui::graphics::rect{1.0, 0.5, 25.0, 100.0});
            layout_.set_layout_flags(right_box_, position_proportional);

            // Bottom-center black bar — LayoutBounds="0.5,1,100,25".
            bottom_box_.set_color(maui::graphics::colors::black);
            layout_.add(bottom_box_);
            layout_.set_layout_bounds(bottom_box_, maui::graphics::rect{0.5, 1.0, 100.0, 25.0});
            layout_.set_layout_flags(bottom_box_, position_proportional);

            // Dead-center label — LayoutBounds="0.5,0.5,110,25", centered text.
            centered_label_.set_text("Centered text");
            centered_label_.set_horizontal_text_alignment(maui::core::text_alignment::center);
            layout_.add(centered_label_);
            layout_.set_layout_bounds(centered_label_, maui::graphics::rect{0.5, 0.5, 110.0, 25.0});
            layout_.set_layout_flags(centered_label_, position_proportional);

            // Auto-sized label at (0.2, 0.7) — LayoutBounds="0.2,0.7,AutoSize,AutoSize": width/height are
            // auto_size (-1), so the child is sized to its own measured text. White text on a blue paint.
            auto_label_.set_text("AutoSized");
            auto_label_.set_text_color(maui::graphics::colors::white);
            auto_label_.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::blue));
            // note: C# also sets HorizontalOptions="Center"; the port's view base has no horizontal_options
            //       setter, so the LayoutBounds anchor + AutoSize width is the faithful placement here.
            layout_.add(auto_label_);
            layout_.set_layout_bounds(auto_label_,
                                      maui::graphics::rect{0.2, 0.7, maui::controls::absolute_layout::auto_size,
                                                           maui::controls::absolute_layout::auto_size});
            layout_.set_layout_flags(auto_label_, position_proportional);

            page_.set_content(layout_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (the placed children first, then the
        // absolute_layout, then the page) so each parent can host its child's native view, then re-host the
        // tree built in the ctor (gallery_attach.hpp). The generic lambda preserves each member's concrete
        // static type — attach_handler keys on the static type, so an i_view& parameter would erase it.
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, top_box_, "top_box_");
            gallery_attach_one(app, left_box_, "left_box_");
            gallery_attach_one(app, right_box_, "right_box_");
            gallery_attach_one(app, bottom_box_, "bottom_box_");
            gallery_attach_one(app, centered_label_, "centered_label_");
            gallery_attach_one(app, auto_label_, "auto_label_");
            gallery_attach_one(app, layout_, "layout_");
            gallery_attach_one(app, page_, "page_");

            // The tree was built in the ctor before any handler existed, so replay the host commands now.
            gallery_rehost_layout(layout_); // absolute_layout hosts its six placed children
            gallery_rehost_content(page_);  // page hosts the absolute_layout
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::absolute_layout& layout()
        {
            return layout_;
        }
        [[nodiscard]] maui::controls::box_view& top_box()
        {
            return top_box_;
        }
        [[nodiscard]] maui::controls::box_view& left_box()
        {
            return left_box_;
        }
        [[nodiscard]] maui::controls::box_view& right_box()
        {
            return right_box_;
        }
        [[nodiscard]] maui::controls::box_view& bottom_box()
        {
            return bottom_box_;
        }
        [[nodiscard]] maui::controls::label& centered_label()
        {
            return centered_label_;
        }
        [[nodiscard]] maui::controls::label& auto_label()
        {
            return auto_label_;
        }

    private:
        maui::controls::content_page page_;
        maui::controls::absolute_layout layout_;
        maui::controls::box_view top_box_;
        maui::controls::box_view left_box_;
        maui::controls::box_view right_box_;
        maui::controls::box_view bottom_box_;
        maui::controls::label centered_label_;
        maui::controls::label auto_label_;
    };
} // namespace maui::samples
