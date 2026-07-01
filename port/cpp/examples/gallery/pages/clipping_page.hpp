#pragma once
// maui::samples::clipping_page  <=  ClippingPage.xaml (+ ClippingPage.xaml.cs)
//
// 1:1 with the maui-compare oracle ~/maui-compare/Pages/ClippingPage.cs (itself written to mirror this
// gallery page). The page demonstrates Layout.IsClippedToBounds: a "Toggle clipping" button flips
// IsClippedToBounds on three horizontal stack layouts —
//   - Layout1: an 8-button overflow row that spills over a translucent-red BoxView, the two laid out in a
//     2-column Grid (the row spans both columns; the BoxView sits in the right column);
//   - Layout2: a width-constrained (100) purple-button HorizontalStackLayout on a LightBlue background;
//   - Layout3: a coffee-image HorizontalStackLayout on a LightBlue background (the 2nd image pushed down
//     by a top margin) —
// and rewrites a Status label between "Clipping" / "Not clipping" (the C# ToggleClip.Clicked handler).
//
// PORT MAPPING:
//   - Layout.IsClippedToBounds  -> layout::set_clips_to_bounds / clips_to_bounds (controls/layout.hpp).
//   - VisualElement.Clip / IView.Clip (an IShape)  -> view::set_clip (controls/view.hpp clip_property →
//     graphics::shapes::round_rectangle). While clipping is ON the toggle ALSO sets a rounded-rectangle
//     clip on the first row, and clears it when OFF — both clip surfaces, the way the framework models them.
//   - StackLayout Orientation="Horizontal"  -> stack_layout with stack_orientation::horizontal
//     (the generic stack so the single XAML <StackLayout> ports 1:1); the two <HorizontalStackLayout> rows
//     -> horizontal_stack_layout (the fixed-orientation control).
//   - Grid 2× ColumnDefinition Width="0.5*"  -> grid + add_column_definition(grid_length{0.5, star});
//     Grid.SetColumnSpan(_row1, 2) -> grid::set_column_span; Grid.SetColumn(overlay, 1) -> grid::set_column.
//   - View.HorizontalOptions="Center"  -> view::set_horizontal_layout_alignment(layout_alignment::center).
//   - View.Margin / Layout.Padding  -> view::set_margin / layout::set_padding (thickness).
//   - BoxView Background="Red" Opacity="0.5"  -> view::set_background(solid_paint red) + view::set_opacity.
//     (The box_view's own shape Fill rides its Color property; the C# oracle sets the VisualElement
//     Background brush, so we mirror that paint layer faithfully — the translucent-red square.)
//   - Image Source="coffee.png"  -> image::set_source(file_image_source("coffee.png")).
//     note: coffee.png is a real 48×56 raster packaged into the apphost APK assets, so the android image
//           handler's file fast-path decodes + renders the two coffee-cup images (the earlier "SVG-only /
//           renders blank" note was stale — pre-dated the wave-10 android BitmapFactory decode). The
//           LightBlue row background + 50×50 sizing are correct alongside.
//
// HEADLESS-SAFE maui:: API only; the page owns its whole element tree (the generic mount in app_host.hpp
// attaches every owned view's handler and hosts the tree).

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "maui/controls/box_view.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/file_image_source.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/controls/image.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/stack_layout.hpp"
#include "maui/controls/stack_orientation.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/grid_unit_type.hpp"
#include "maui/core/layout_alignment.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/corner_radius.hpp"
#include "maui/graphics/shapes/round_rectangle.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class clipping_page
    {
    public:
        clipping_page()
        {
            page_.set_title("Clipping");
            // Background = new SolidColorBrush(Colors.Orange) (ClippingPage.xaml root).
            page_.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::orange));

            // var root = new VerticalStackLayout { Spacing = 5 };
            root_.set_spacing(5);

            // readonly Label _status = new() { Text = "Not clipping", Margin = new Thickness(10) };
            status_.set_text("Not clipping");
            status_.set_margin(maui::core::thickness{10});

            // var toggleClip = new Button { Text = "...", HorizontalOptions = Center, Margin = new Thickness(5) };
            toggle_clip_.set_text("Toggle clipping on horizontal stack layouts");
            toggle_clip_.set_horizontal_layout_alignment(maui::core::layout_alignment::center);
            toggle_clip_.set_margin(maui::core::thickness{5});
            // toggleClip.Clicked += (_, _) => OnToggleClip();
            toggle_clip_.command = [this] { on_toggle_clip(); };

            root_.add(status_);      // root.Add(_status);
            root_.add(toggle_clip_); // root.Add(toggleClip);

            // ---- Layout1: the 8-button overflow row over a translucent-red overlay (a 2-column Grid) ----
            // readonly StackLayout _row1 = new() { Orientation = StackOrientation.Horizontal };
            row1_.set_orientation(maui::controls::stack_orientation::horizontal);
            for (int i = 0; i < kRow1Buttons; ++i) // for (int i = 0; i < 8; i++)
            {
                // _row1.Add(new Button { Text = (i + 1).ToString(), Margin = 1, HeightRequest = 50 });
                auto button = std::make_shared<maui::controls::button>();
                button->set_text(std::to_string(i + 1));
                button->set_margin(maui::core::thickness{1});
                button->set_height_request(50);
                row1_.add(*button);
                row1_buttons_.push_back(std::move(button));
            }
            // var overflowGrid = new Grid { WidthRequest = 400, HeightRequest = 200, ColumnDefinitions = { 0.5*, 0.5* }
            // };
            overflow_grid_.set_width_request(400);
            overflow_grid_.set_height_request(200);
            overflow_grid_.add_column_definition(maui::core::grid_length{0.5, maui::core::grid_unit_type::star});
            overflow_grid_.add_column_definition(maui::core::grid_length{0.5, maui::core::grid_unit_type::star});
            // var overlay = new BoxView { Background = new SolidColorBrush(Colors.Red), Opacity = 0.5 };
            overlay_.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::red));
            overlay_.set_opacity(0.5);
            // overflowGrid.Add(_row1); Grid.SetColumnSpan(_row1, 2);
            overflow_grid_.add(row1_);
            overflow_grid_.set_column_span(row1_, 2);
            // overflowGrid.Add(overlay); Grid.SetColumn(overlay, 1);
            overflow_grid_.add(overlay_);
            overflow_grid_.set_column(overlay_, 1);
            root_.add(overflow_grid_); // root.Add(overflowGrid);

            // ---- Layout2: the width-constrained purple-button row (HorizontalStackLayout) ----
            // readonly HorizontalStackLayout _row2 = new() { WidthRequest = 100, Padding = 10 };
            row2_.set_width_request(100);
            row2_.set_padding(maui::core::thickness{10});
            // _row2.Background = new SolidColorBrush(Colors.LightBlue);
            row2_.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::light_blue));
            for (int i = 0; i < kRow2Buttons; ++i) // for (int i = 0; i < 4; i++)
            {
                // _row2.Add(new Button { Text = "Hey", WidthRequest = 50, HeightRequest = 50, Background = Purple });
                auto button = std::make_shared<maui::controls::button>();
                button->set_text("Hey");
                button->set_width_request(50);
                button->set_height_request(50);
                button->set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::purple));
                row2_.add(*button);
                row2_buttons_.push_back(std::move(button));
            }
            root_.add(row2_); // root.Add(_row2);

            // ---- Layout3: the coffee-image row (HorizontalStackLayout) ----
            // readonly HorizontalStackLayout _row3 = new() { HeightRequest = 30, Padding = 3 };
            row3_.set_height_request(30);
            row3_.set_padding(maui::core::thickness{3});
            // _row3.Background = new SolidColorBrush(Colors.LightBlue);
            row3_.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::light_blue));
            // _row3.Add(new Image { Source = "coffee.png", WidthRequest = 50, HeightRequest = 50 });
            // note: coffee.png is a real 48×56 raster packaged into the apphost APK assets — the android
            //       image handler decodes + renders it (the row's two coffee-cup images show); the source is
            //       set faithfully and the row sizing/background is correct.
            auto image0 = std::make_shared<maui::controls::image>();
            image0->set_source(std::make_shared<maui::controls::file_image_source>("coffee.png"));
            image0->set_width_request(50);
            image0->set_height_request(50);
            row3_.add(*image0);
            row3_images_.push_back(std::move(image0));
            // _row3.Add(new Image { Source = "coffee.png", WidthRequest = 50, HeightRequest = 50, Margin = (0,20,0,0)
            // });
            auto image1 = std::make_shared<maui::controls::image>();
            image1->set_source(std::make_shared<maui::controls::file_image_source>("coffee.png"));
            image1->set_width_request(50);
            image1->set_height_request(50);
            image1->set_margin(maui::core::thickness{0, 20, 0, 0});
            row3_.add(*image1);
            row3_images_.push_back(std::move(image1));
            root_.add(row3_); // root.Add(_row3);

            page_.set_content(root_); // Content = root;
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment / inspection.
        [[nodiscard]] maui::controls::vertical_stack_layout& root()
        {
            return root_;
        }
        [[nodiscard]] maui::controls::label& status()
        {
            return status_;
        }
        [[nodiscard]] maui::controls::button& toggle_clip()
        {
            return toggle_clip_;
        }
        [[nodiscard]] maui::controls::grid& overflow_grid()
        {
            return overflow_grid_;
        }
        [[nodiscard]] maui::controls::stack_layout& row1()
        {
            return row1_;
        }
        [[nodiscard]] maui::controls::box_view& overlay()
        {
            return overlay_;
        }
        [[nodiscard]] maui::controls::horizontal_stack_layout& row2()
        {
            return row2_;
        }
        [[nodiscard]] maui::controls::horizontal_stack_layout& row3()
        {
            return row3_;
        }

    private:
        // The C# ToggleClip.Clicked handler: flip IsClippedToBounds on the three rows, echo the first row's
        // new state into the status label, and (the geometry-clip extension) set/clear a rounded-rectangle
        // Clip on row1 while clipping is on.
        void on_toggle_clip()
        {
            const bool clipping = !row1_.clips_to_bounds(); // bool clipping = !_row1.IsClippedToBounds;
            row1_.set_clips_to_bounds(clipping);            // _row1.IsClippedToBounds = clipping;
            row2_.set_clips_to_bounds(clipping);            // _row2.IsClippedToBounds = clipping;
            row3_.set_clips_to_bounds(clipping);            // _row3.IsClippedToBounds = clipping;

            // _row1.Clip = clipping ? new RoundRectangleGeometry { CornerRadius = 8, Rect = (0,0,400,50) } : null;
            // The port's round_rectangle paints its rounded rect over the bounds handed at draw time, so the
            // explicit Rect(0,0,400,50) is implicit in the row's own 400×50 bounds; CornerRadius = 8 here.
            row1_.set_clip(
                clipping ? std::make_shared<maui::graphics::shapes::round_rectangle>(maui::graphics::corner_radius{8})
                         : nullptr);

            status_.set_text(clipping ? "Clipping" : "Not clipping"); // _status.Text = clipping ? ... : ...;
        }

        static constexpr int kRow1Buttons = 8;
        static constexpr int kRow2Buttons = 4;
        static constexpr int kRow3Images = 2;

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout root_;
        maui::controls::label status_;
        maui::controls::button toggle_clip_;
        maui::controls::grid overflow_grid_;           // the 2-column Grid wrapping row1 + the overlay
        maui::controls::stack_layout row1_;            // StackLayout Orientation="Horizontal" (Layout1)
        maui::controls::box_view overlay_;             // the translucent-red BoxView the row spills over
        maui::controls::horizontal_stack_layout row2_; // Layout2
        maui::controls::horizontal_stack_layout row3_; // Layout3
        std::vector<std::shared_ptr<maui::controls::button>> row1_buttons_;
        std::vector<std::shared_ptr<maui::controls::button>> row2_buttons_;
        std::vector<std::shared_ptr<maui::controls::image>> row3_images_;
    };
} // namespace maui::samples
