#pragma once
// maui::samples::auto_size_shapes_page — ports AutoSizeShapesGallery.xaml
//
// A code-first port of the MAUI Shapes sub-gallery
// Pages/Controls/ShapesGalleries/AutoSizeShapesGallery.xaml: a 3-row Grid (RowSpacing 0) that proves a
// stroked Ellipse auto-sizes to fill exactly half of the available vertical space —
//   - Row 0 (Height="Auto"): a caption Label "The Ellipse below must occupy half of the available
//                            space." (white text on a black background);
//   - Row 1 (Height="*"):    a yellow-background Grid that hosts a single Ellipse — green fill, blue
//                            stroke, thickness 4 — with NO width/height request, so the shape stretches
//                            to fill its star-sized cell (the auto-sizing behavior the page demonstrates);
//   - Row 2 (Height="*"):    an orange-background Grid (empty) — the second equal star row, so the eye
//                            can confirm the ellipse's row is exactly half the remaining height.
//
// The page OWNS its whole element tree (the shapes_demo_page / path_gallery_page pattern). It is
// backend-agnostic — a sample main attaches handlers bottom-up via the hosting layer and hosts page() in a
// window; the headless/apple/ios test trees exercise the same wiring directly.
//
// PORT NOTES (faithful best-effort, never invented):
//   note: the C# code-behind is just InitializeComponent() — there is no logic to port; the page is a
//         purely visual auto-sizing demo with no mutation buttons (unlike the other two shapes-gallery
//         pages in this batch). The ellipse deliberately carries NO WidthRequest/HeightRequest so its
//         shape measure (Shape.MeasureOverride) lets the star-sized Grid cell drive its size.
//   note: the two star rows (the XAML <RowDefinition /> with no Height) are added as grid_length::star();
//         the first (Height="Auto") row is grid_length::automatic() — the exact GridLength decomposition
//         the XAML row definitions name.
//   note: the C# BackgroundColor brushes (White/Black on the label, Yellow/Orange on the two cell Grids)
//         are reconstructed via set_background over a solid_paint — the documented Brush→Paint bridge
//         (there is no set_background_color on a view; VisualElement.Background is a paint). The label's
//         TextColor="White" maps to set_text_color directly.
//   note: the inner yellow Grid is a single-cell Grid (the XAML <Grid> with no row/column definitions);
//         the port models it as a grid with no definitions (the manager treats an empty axis as one
//         implied star cell), so the ellipse fills it exactly as in C#.

#include <memory>

#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/shapes/ellipse.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class auto_size_shapes_page
    {
    public:
        auto_size_shapes_page()
        {
            page_.set_title("AutoSize Shapes Gallery");

            // The outer 3-row grid (RowSpacing 0): Auto / * / *.
            outer_.set_row_spacing(0);
            outer_.add_row_definition(maui::core::grid_length::automatic());
            outer_.add_row_definition(maui::core::grid_length::star());
            outer_.add_row_definition(maui::core::grid_length::star());

            // Row 0 — the caption: white text on a black background.
            caption_.set_text("The Ellipse below must occupy half of the available space.");
            caption_.set_text_color(maui::graphics::colors::white);
            caption_.set_background(solid(maui::graphics::colors::black));
            outer_.add(caption_);
            outer_.set_row(caption_, 0);

            // Row 1 — a yellow single-cell grid hosting the auto-sizing ellipse (green fill, blue stroke 4).
            yellow_cell_.set_background(solid(maui::graphics::colors::yellow));
            ellipse_.set_fill(solid(maui::graphics::colors::green));
            ellipse_.set_stroke(solid(maui::graphics::colors::blue));
            ellipse_.set_stroke_thickness(4);
            // note: NO width/height request — the star cell sizes the ellipse (the whole point of the page).
            yellow_cell_.add(ellipse_);
            outer_.add(yellow_cell_);
            outer_.set_row(yellow_cell_, 1);

            // Row 2 — the empty orange grid (the second equal star row).
            orange_cell_.set_background(solid(maui::graphics::colors::orange));
            outer_.add(orange_cell_);
            outer_.set_row(orange_cell_, 2);

            page_.set_content(outer_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::grid& outer()
        {
            return outer_;
        }
        [[nodiscard]] maui::controls::grid& yellow_cell()
        {
            return yellow_cell_;
        }
        [[nodiscard]] maui::controls::shapes::ellipse& ellipse_shape()
        {
            return ellipse_;
        }
        [[nodiscard]] maui::controls::label& caption()
        {
            return caption_;
        }

    private:
        // One solid_paint over a color (the C# Brush→Paint bridge for a named fill/stroke/background).
        static std::shared_ptr<maui::graphics::solid_paint> solid(maui::graphics::color value)
        {
            return std::make_shared<maui::graphics::solid_paint>(value);
        }

        maui::controls::content_page page_;
        maui::controls::grid outer_;
        maui::controls::label caption_;
        maui::controls::grid yellow_cell_;
        maui::controls::shapes::ellipse ellipse_;
        maui::controls::grid orange_cell_;
    };
} // namespace maui::samples
