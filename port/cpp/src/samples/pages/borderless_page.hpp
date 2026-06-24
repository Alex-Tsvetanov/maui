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
// Interaction (a gallery-page readout convention, code-first): a toggle_switch flips both borders between
// the borderless style (StrokeThickness 0, the XAML default) and a visible 8-unit black stroke, and a
// readout label echoes which style is active — so the "Borderless" effect is demonstrable live rather
// than only at rest.
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same wiring directly.

#include <memory>

#include "maui/controls/border.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/toggle_switch.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
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

            // The interactive layer: a switch to toggle the borderless style + a readout. This is a
            // headless-safe gallery addition on top of the faithful XAML content (the page renders the two
            // fields exactly as the C# sample at rest, with the switch starting OFF = borderless).
            readout_.set_text("Style: borderless (StrokeThickness 0)");
            stroke_switch_.toggled.connect([this](bool show_stroke) { set_stroke_visible(show_stroke); });

            controls_.set_spacing(8);
            controls_.add(readout_);
            controls_.add(stroke_switch_);
            controls_.add(grid_);
            page_.set_content(controls_);
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
        [[nodiscard]] maui::controls::toggle_switch& stroke_switch()
        {
            return stroke_switch_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }

    private:
        // BorderlessStyle: StrokeThickness = 0 (the page's single Setter).
        static void apply_borderless(maui::controls::border& border)
        {
            border.set_stroke_thickness(0);
        }

        void set_stroke_visible(bool show_stroke)
        {
            if (show_stroke)
            {
                auto stroke = std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::black);
                pink_border_.set_stroke(stroke);
                red_border_.set_stroke(stroke);
                pink_border_.set_stroke_thickness(8);
                red_border_.set_stroke_thickness(8);
                readout_.set_text("Style: bordered (StrokeThickness 8)");
            }
            else
            {
                apply_borderless(pink_border_);
                apply_borderless(red_border_);
                readout_.set_text("Style: borderless (StrokeThickness 0)");
            }
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout controls_;
        maui::controls::label readout_;
        maui::controls::toggle_switch stroke_switch_;
        maui::controls::grid grid_;
        maui::controls::border pink_border_;
        maui::controls::border red_border_;
    };
} // namespace maui::samples
