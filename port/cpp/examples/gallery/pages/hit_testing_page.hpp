#pragma once
// maui::samples::hit_testing_page — ports HitTestingPage.xaml (+ .xaml.cs)
//                                   (Maui.Controls.Sample.Pages.HitTestingPage).
//
// The C# page is a scrolled VerticalStackLayout of varied, overlapping-when-scaled/rotated views — two
// aligned labels, three Buttons (Scale=1, Scale=2, Rotation=20), an Ellipse, a RoundRectangle, and an
// Image — plus a "Rectangle Selection" CheckBox and a SelectionLabel readout. A WindowOverlay catches
// taps; each tap runs VisualTreeElementExtensions.GetVisualTreeElements(window, x, y) (a point or, in
// rectangle mode, a region) to find which view(s) sit under the hit. The readout lists the hit chain
// ("Selected: Button <- VerticalStackLayout <- ...") and the topmost hit view is highlighted red
// (BackgroundColor = Color(255,0,0)). The CheckBox flips between single-point selection and a two-tap
// rectangle lasso (drawn by the overlay's IDrawable).
//
// The port's headless backend has NO visual-tree hit-tester: window_overlay.hpp documents that the
// Tapped drive (the GetVisualTreeElements walk) is DEFERRED — the surface (the overlay, the Tapped
// event, the element list) exists, but the point->elements walk is not modeled. So this code-first port
// reproduces the page's OBSERVABLE behavior deterministically, the way input_transparent_page models
// InputTransparent routing: it builds the real overlapping view set, and hit_test(point) walks the
// views top-to-bottom and returns the first whose bounds contain the point (a faithful stand-in for the
// native z-order hit-test). The readout reports the hit chain and the hit view is highlighted, mirroring
// the C# SelectionLabel + red highlight. The CheckBox toggles single vs rectangle selection exactly as
// in C#; rectangle mode collects every view intersecting the lasso.
//
// Demonstrated (the views + overlay exist; a synthetic tap drives the readout):
//   - single-selection: a synthetic tap at a point lands on the topmost view whose bounds contain it;
//     the readout names it and it is highlighted (red), the rest cleared,
//   - rectangle-selection: toggling the CheckBox switches modes; a two-point lasso collects every view
//     it intersects and the readout lists the chain,
//   - the varied view set (labels / buttons (Scale/Rotation) / ellipse / rounded box / image) is built
//     so the hit walk has overlapping candidates to disambiguate.
//
// The page OWNS its whole element tree (the input_transparent_page / gestures_page pattern): public
// page().
//
// note: the native GetVisualTreeElements(window, x, y) walk + the WindowOverlay Tapped drive + the
//       IDrawable rectangle-lasso rendering are the documented headless gap (window_overlay.hpp). This
//       port models the SAME selection result with a deterministic bounds-based hit_test over the page's
//       views; the per-view bounds are assigned representative frames here (the headless layout pass does
//       not measure/arrange without a window), so the walk is over assigned rects rather than measured
//       ones — the routing/selection semantics are identical.

#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "maui/controls/button.hpp"
#include "maui/controls/check_box.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/file_image_source.hpp"
#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/controls/image.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/shapes/ellipse.hpp"
#include "maui/controls/shapes/rectangle.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/corner_radius.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class hit_testing_page
    {
    public:
        hit_testing_page()
        {
            page_.set_title("Hit testing");
            stack_.set_spacing(25);
            stack_.set_padding(maui::core::thickness(30));

            // ---- the readout + the rectangle-selection toggle (the C# SelectionLabel + CheckBox) -------
            selection_label_.set_text("Selected: -");
            selection_label_.set_horizontal_layout_alignment(
                maui::core::layout_alignment::center); // XAML HorizontalOptions="Center"
            mode_label_.set_text("Rectangle Selection");
            mode_row_.add(mode_label_);
            mode_row_.add(rectangle_select_check_);
            // CheckedChanged flips single <-> rectangle selection mode (the C# State machine).
            rectangle_select_check_.checked_changed.connect([this](bool checked) { rectangle_mode_ = checked; });

            // ---- the varied, overlapping view set (each given a representative frame for the hit walk) --
            // Two aligned labels (Start / End horizontal options in the C#): the first hugs the leading
            // edge, the second the trailing edge — matching MAUI's HorizontalOptions Start/End.
            left_label_.set_text("Lorem ipsum dolor sit ame");
            left_label_.set_horizontal_layout_alignment(maui::core::layout_alignment::start);
            right_label_.set_text("Lorem ipsum dolor sit ame");
            right_label_.set_horizontal_layout_alignment(maui::core::layout_alignment::end);

            // Three buttons: Scale=1, Scale=2 (overlaps its neighbors when scaled), Rotation=20.
            scale1_btn_.set_text("Scale = 1");
            scale1_btn_.set_scale(1);
            scale2_btn_.set_text("Scale = 2");
            scale2_btn_.set_scale(2); // doubled — its bounds overlap the rows above/below
            rotate_btn_.set_text("Rotation = 20");
            rotate_btn_.set_rotation(20);

            // An ellipse + a rounded (unfilled) Rectangle (the RoundRectangle stand-in) + an image.
            oval_.set_stroke(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::green));
            oval_.set_stroke_thickness(10);
            oval_.set_width_request(150);
            oval_.set_height_request(50);

            // XAML: <Rectangle WidthRequest="300" HeightRequest="200" RadiusX="40" RadiusY="40"
            //        StrokeThickness="10" Stroke="Green" /> — no Fill, so it must stay unfilled
            // (the port previously used a filled box_view stand-in here; that diverged from MAUI).
            rounded_.set_stroke(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::green));
            rounded_.set_stroke_thickness(10);
            rounded_.set_radius_x(40);
            rounded_.set_radius_y(40);
            rounded_.set_width_request(300);
            rounded_.set_height_request(200);

            bot_image_.set_source(std::make_shared<maui::controls::file_image_source>("dotnet_bot.png"));
            bot_image_.set_width_request(125);
            bot_image_.set_height_request(155);

            // Assign each candidate a representative frame (top-to-bottom in z, like a scrolled column;
            // scale2_btn_ deliberately overlaps its neighbors). The hit walk reads these (see hit_test).
            register_target(left_label_, "Label(left)", {30, 120, 200, 24});
            register_target(right_label_, "Label(right)", {300, 120, 200, 24});
            register_target(scale1_btn_, "Button(Scale=1)", {180, 160, 160, 44});
            register_target(scale2_btn_, "Button(Scale=2)", {130, 150, 260, 88}); // scaled: overlaps both rows
            register_target(rotate_btn_, "Button(Rotation=20)", {180, 250, 180, 44});
            register_target(oval_, "Ellipse", {30, 310, 150, 50});
            register_target(rounded_, "RoundRectangle", {30, 380, 300, 200});
            register_target(bot_image_, "Image", {130, 600, 125, 155});

            // ---- assemble (the C# VerticalStackLayout order, inside a ScrollView) -----------------------
            stack_.add(selection_label_);
            stack_.add(mode_row_);
            stack_.add(left_label_);
            stack_.add(right_label_);
            stack_.add(scale1_btn_);
            stack_.add(scale2_btn_);
            stack_.add(rotate_btn_);
            stack_.add(oval_);
            stack_.add(rounded_);
            stack_.add(bot_image_);
            scroller_.set_content(stack_);
            page_.set_content(scroller_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned readout + check, exposed for the hosting main / headless tests.
        [[nodiscard]] maui::controls::label& selection_label()
        {
            return selection_label_;
        }
        [[nodiscard]] maui::controls::check_box& rectangle_select_check()
        {
            return rectangle_select_check_;
        }

        // The deterministic hit-test core (the GetVisualTreeElements stand-in), exposed so headless tests
        // can assert selection without a window: return the topmost target whose frame contains `point`,
        // or -1 when the tap missed every view. Walks top-to-bottom in z (the last-added view is on top —
        // here the registration order is the column order, so we walk it in reverse for a faithful
        // "front-most wins").
        [[nodiscard]] int hit_test(const maui::graphics::point& point) const
        {
            for (int i = static_cast<int>(targets_.size()) - 1; i >= 0; --i)
            {
                if (targets_[static_cast<std::size_t>(i)].frame.contains(point))
                {
                    return i;
                }
            }
            return -1;
        }

        // Single-selection: deliver a tap at `point`, name the hit on the readout, highlight it red and
        // clear the rest (the C# HandleTapped single-selection branch).
        void tap_single(const maui::graphics::point& point)
        {
            clear_highlights();
            const int hit = hit_test(point);
            if (hit < 0)
            {
                selection_label_.set_text("Selected: -");
                return;
            }
            selection_label_.set_text("Selected: " + targets_[static_cast<std::size_t>(hit)].name);
            highlight(targets_[static_cast<std::size_t>(hit)]);
        }

        // Rectangle-selection: collect every target intersecting the lasso [a, b] and list the chain (the
        // C# RectangleSelectionPickSecond branch). The first hit is highlighted red.
        void tap_rectangle(const maui::graphics::point& corner_a, const maui::graphics::point& corner_b)
        {
            clear_highlights();
            const maui::graphics::rect lasso =
                maui::graphics::rect::from_ltrb(std::min(corner_a.x, corner_b.x), std::min(corner_a.y, corner_b.y),
                                                std::max(corner_a.x, corner_b.x), std::max(corner_a.y, corner_b.y));

            std::string chain;
            const target* first = nullptr;
            for (int i = static_cast<int>(targets_.size()) - 1; i >= 0; --i)
            {
                const target& t = targets_[static_cast<std::size_t>(i)];
                if (intersects(lasso, t.frame))
                {
                    if (!chain.empty())
                    {
                        chain += " <- ";
                    }
                    chain += t.name;
                    if (first == nullptr)
                    {
                        first = &t;
                    }
                }
            }
            selection_label_.set_text("Selected: " + (chain.empty() ? std::string("-") : chain));
            if (first != nullptr)
            {
                highlight(*first);
            }
        }

    private:
        // One hit-test candidate: a type-erased background setter (so heterogeneous view<T> instances
        // live in one list), its display name (the C# GetType().Name in the chain), and the
        // representative frame the deterministic walk reads.
        struct target
        {
            std::function<void(std::shared_ptr<maui::graphics::paint>)> set_background; // the highlight seam
            std::string name;
            maui::graphics::rect frame;
        };

        // Register a view as a hit-test candidate with a representative frame. The view's set_background is
        // captured as a closure (each view<T> has its own concrete setter — no common non-template base).
        template <class View> void register_target(View& view, const char* name, maui::graphics::rect frame)
        {
            targets_.push_back(target{
                .set_background =
                    [&view](std::shared_ptr<maui::graphics::paint> paint) { view.set_background(std::move(paint)); },
                .name = name,
                .frame = frame});
        }

        // Two rects intersect (the lasso vs a target frame). graphics::rect has no intersects() helper
        // here, so this is the standard AABB overlap test.
        static bool intersects(const maui::graphics::rect& a, const maui::graphics::rect& b)
        {
            return a.left() < b.right() && b.left() < a.right() && a.top() < b.bottom() && b.top() < a.bottom();
        }

        // Highlight a hit view red (the C# e.BackgroundColor = Color(255,0,0)).
        static void highlight(const target& t)
        {
            t.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::red));
        }

        // Clear every target's highlight (the C# `foreach (var c in _allChildren) c.BackgroundColor = null`).
        void clear_highlights()
        {
            for (const target& t : targets_)
            {
                t.set_background(nullptr);
            }
        }

        maui::controls::content_page page_;
        maui::controls::scroll_view scroller_;
        maui::controls::vertical_stack_layout stack_;

        maui::controls::label selection_label_;
        maui::controls::horizontal_stack_layout mode_row_;
        maui::controls::label mode_label_;
        maui::controls::check_box rectangle_select_check_;

        maui::controls::label left_label_;
        maui::controls::label right_label_;
        maui::controls::button scale1_btn_;
        maui::controls::button scale2_btn_;
        maui::controls::button rotate_btn_;
        maui::controls::shapes::ellipse oval_;
        maui::controls::shapes::rectangle rounded_;
        maui::controls::image bot_image_;

        std::vector<target> targets_; // the hit-test candidates (the C# _allChildren)
        bool rectangle_mode_ = false; // the C# State (single vs rectangle selection)
    };
} // namespace maui::samples
