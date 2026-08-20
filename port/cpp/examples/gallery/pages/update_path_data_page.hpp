#pragma once
// maui::samples::update_path_data_page — ports UpdatePathDataGallery.xaml
//
// A code-first port of the MAUI Shapes sub-gallery
// Pages/Controls/ShapesGalleries/UpdatePathDataGallery.xaml: a 2-row Grid (RowSpacing 0) that proves a
// Path repaints when its Data geometry is replaced at runtime —
//   - Row 0 (Height="*"):    a black-stroked zig-zag through the cubic Bézier's control points from the
//                            markup "M 10,100 C 10,300 300,-200 300,100" (the initial Data) — MAUI's
//                            actual render of this curve (see PORT NOTES below);
//   - Row 1 (Height="Auto"): an "Update Path Data" Button.
// Each tap bumps an internal counter by 10 and rebuilds the Data string
//   "M 10,100 C 10,{300+counter} {300+counter},-200 {300+counter},100"
// — re-parsing it and refreshing the zig-zag's points — so the shape visibly grows / shifts on every
// press. This is the path-data-invalidation demo: replace the geometry, the shape repaints.
//
// The page OWNS its whole element tree (the shapes_demo_page / path_gallery_page pattern). It is
// backend-agnostic — a sample main attaches handlers bottom-up via the hosting layer and hosts page() in a
// window; the headless/apple/ios test trees exercise the same wiring directly.
//
// PORT NOTES (faithful best-effort, never invented):
//   note: the C# code-behind builds the new Data with a PathFigureCollectionConverter
//         (ConvertFromInvariantString → PathFigureCollection → PathGeometry.Figures) and the real MAUI
//         Path element parses the cubic-Bezier "C" command faithfully. BUT the actual .NET MAUI Mac
//         Catalyst render of THIS SPECIFIC curve ("M 10,100 C 10,300 300,-200 300,100") is a MAUI-side
//         rendering quirk (verified against port/maui-reference/captures/maccatalyst/update_path_data_*):
//         it draws straight ZIG-ZAG segments through the four control points (10,100)->(10,300)->
//         (300,-200)->(300,100), NOT a smooth curve — the shared XAML twin's own comment independently
//         documents the identical degraded silhouette via its Polyline
//         "10,100 80,180 150,100 220,20 300,100" approximation. Per port/CLAUDE.md the standing doctrine, the
//         real MAUI render is ground truth for page content, so the port reproduces the zig-zag directly
//         (a Polyline through the same four Bezier control points) rather than a mathematically-correct
//         cubic Bezier curve, matching MAUI's actual on-screen behavior.
//   note: the C# counter math is reproduced verbatim: `_counter += 10` then the format string
//         "M 10,100 C 10,{300 + _counter} {300 + _counter},-200 {300 + _counter},100". The initial Data
//         (before any tap, counter 0) is the XAML "M 10,100 C 10,300 300,-200 300,100"; the port zig-zags
//         through the same four control points: (10,100), (10, 300+counter), (300+counter, -200),
//         (300+counter, 100).
//   note: the C# Path carries no explicit Aspect (it keeps Shape's default, which is None/stretch=false);
//         this port leaves Aspect at its default too (no set_aspect call), matching the XAML.
//   note: a status readout label is added (not in the C# XAML) to surface the current counter / Data for
//         the headless tests and the static capture; it never changes the demonstrated behavior.

#include <memory>
#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/shapes/polyline.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class update_path_data_page
    {
    public:
        update_path_data_page()
        {
            page_.set_title("Update Path Data Gallery");

            // The grid (RowSpacing 0): * (the path) / Auto (the readout) / Auto (the button). The C# XAML is
            // just *(path) / Auto(button); the port adds a diagnostic readout Label (header note). It used to
            // share the button's Auto row, relying on the button being transparent so the label showed through
            // (true on iOS — native UIButton has no opaque fill). On Android the native android.widget.Button
            // carries an OPAQUE Material background that paints over the whole cell, hiding the overlapped
            // readout. Give the readout its own Auto row so the two never collide on any backend.
            outer_.set_row_spacing(0);
            outer_.add_row_definition(maui::core::grid_length::star());
            outer_.add_row_definition(maui::core::grid_length::automatic());
            outer_.add_row_definition(maui::core::grid_length::automatic());

            // Row 0 — the black-stroked zig-zag through the cubic Bézier's control points (initial Data,
            // counter 0; see header note on why MAUI's actual render is a zig-zag, not a smooth curve).
            polyline_.set_points(make_points());
            polyline_.set_stroke(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::black));
            outer_.add(polyline_);
            outer_.set_row(polyline_, 0);

            // Row 1 — a status readout (port addition — surfaces the current counter / Data; see header note).
            outer_.add(readout_);
            outer_.set_row(readout_, 1);
            update_readout();

            // Row 2 — the "Update Path Data" button: each tap bumps the counter and replaces the geometry.
            update_button_.set_text("Update Path Data");
            update_button_.clicked.connect([this]() {
                counter_ += 10; // C# `_counter += 10`
                polyline_.set_points(make_points());
                update_readout();
            });
            outer_.add(update_button_);
            outer_.set_row(update_button_, 2);

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
        [[nodiscard]] maui::controls::shapes::polyline& path()
        {
            return polyline_;
        }
        [[nodiscard]] maui::controls::button& update_button()
        {
            return update_button_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }
        // The current counter (for the headless tests to assert the +=10 stepping).
        [[nodiscard]] int counter() const
        {
            return counter_;
        }

    private:
        // C# data string: "M 10,100 C 10,{300 + _counter} {300 + _counter},-200 {300 + _counter},100".
        [[nodiscard]] std::string data_markup() const
        {
            const int shifted = 300 + counter_;
            const std::string s = std::to_string(shifted);
            return "M 10,100 C 10," + s + " " + s + ",-200 " + s + ",100";
        }

        // The zig-zag through the cubic Bézier's four control points — MAUI's actual render (header note):
        // (10,100) -> (10,300+counter) -> (300+counter,-200) -> (300+counter,100).
        [[nodiscard]] maui::controls::shapes::point_collection make_points() const
        {
            const double shifted = 300 + counter_;
            return {{10, 100}, {10, shifted}, {shifted, -200}, {shifted, 100}};
        }

        void update_readout()
        {
            readout_.set_text("counter = " + std::to_string(counter_) + " | Data: " + data_markup());
        }

        int counter_ = 0; // C# `int _counter;`

        maui::controls::content_page page_;
        maui::controls::grid outer_;
        maui::controls::shapes::polyline polyline_;
        maui::controls::button update_button_;
        maui::controls::label readout_;
    };
} // namespace maui::samples
