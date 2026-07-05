#pragma once
// maui::samples::borderless_page — ports Borderless.xaml
//
// A self-contained, code-first demo of a stroke-less Border. It mirrors the C# core gallery page
// (Pages/Core/BorderGalleries/Borderless.xaml): a yellow page hosting a two-row Grid (RowDefinitions
// "*,*", RowSpacing 0) where each cell holds a filled Border (top = Pink, bottom = Red) that carries the
// shared "BorderlessStyle" — a single Setter, StrokeThickness = 0. With no stroke the two color fields
// butt seamlessly against each other, which is the whole point of the page (a Border used purely as a
// background fill with no visible outline).
//
// The C# style (a Border-targeted Style whose only Setter is StrokeThickness="0") is reproduced here by
// setting stroke_thickness(0) directly on each border in the ctor — the headless-safe equivalent of the
// implicit/keyed Style application (the port's style layer would resolve the same value; this page wires
// the resulting property value-for-value so it renders identically on macOS + iOS).
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same wiring directly.

#include <memory>

#include "maui/controls/border.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class borderless_page
    {
    public:
        borderless_page()
        {
            page_.set_title("Border without Stroke");
            // BackgroundColor="Yellow" on the ContentPage.
            page_.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::yellow));

            // RowDefinitions="*,*", RowSpacing="0" — the two cells stack edge-to-edge.
            grid_.add_row_definition(maui::core::grid_length::star());
            grid_.add_row_definition(maui::core::grid_length::star());
            grid_.set_row_spacing(0);

            // Top border — Background="Pink", BorderlessStyle (StrokeThickness 0).
            pink_border_.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::pink));
            apply_borderless(pink_border_);
            grid_.add(pink_border_);
            grid_.set_row(pink_border_, 0);

            // Bottom border — Grid.Row="1", Background="Red", BorderlessStyle (StrokeThickness 0).
            red_border_.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::red));
            apply_borderless(red_border_);
            grid_.add(red_border_);
            grid_.set_row(red_border_, 1);

            // Page content IS the grid, exactly as the C# sample and the canonical shared
            // borderless.xaml (the earlier synthetic readout+switch layer diverged from the twin AND
            // collapsed the star-row grid to zero height inside its unconstrained stack).
            page_.set_content(grid_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::grid& grid()
        {
            return grid_;
        }
        [[nodiscard]] maui::controls::border& pink_border()
        {
            return pink_border_;
        }
        [[nodiscard]] maui::controls::border& red_border()
        {
            return red_border_;
        }

    private:
        // BorderlessStyle: StrokeThickness = 0 (the page's single Setter).
        static void apply_borderless(maui::controls::border& border)
        {
            border.set_stroke_thickness(0);
        }

        maui::controls::content_page page_;
        maui::controls::grid grid_;
        maui::controls::border pink_border_;
        maui::controls::border red_border_;
    };
} // namespace maui::samples
