#pragma once
// maui::samples::update_path_data_page — ports UpdatePathDataGallery.xaml
//
// A code-first port of the MAUI Shapes sub-gallery
// Pages/Controls/ShapesGalleries/UpdatePathDataGallery.xaml: a 2-row Grid (RowSpacing 0) that proves a
// Path repaints when its Data geometry is replaced at runtime —
//   - Row 0 (Height="*"):    a black-stroked Path drawing a cubic Bézier from the markup
//                            "M 10,100 C 10,300 300,-200 300,100" (the initial Data);
//   - Row 1 (Height="Auto"): an "Update Path Data" Button.
// Each tap bumps an internal counter by 10 and rebuilds the Data string
//   "M 10,100 C 10,{300+counter} {300+counter},-200 {300+counter},100"
// — re-parsing it into a fresh PathGeometry and assigning Path.Data — so the curve visibly grows /
// shifts on every press. This is the path-data-invalidation demo: replace the geometry, the Path
// repaints.
//
// The page OWNS its whole element tree (the shapes_demo_page / path_gallery_page pattern). It is
// backend-agnostic — a sample main attaches handlers bottom-up via the hosting layer and hosts page() in a
// window; the headless/apple/ios test trees exercise the same wiring directly.
//
// PORT NOTES (faithful best-effort, never invented):
//   note: the C# code-behind builds the new Data with a PathFigureCollectionConverter
//         (ConvertFromInvariantString → PathFigureCollection → PathGeometry.Figures). The port parses the
//         same Data markup string with parse_path_geometry (⇐ PathGeometryConverter.ConvertFrom), which
//         yields a path_geometry directly; it is wrapped in a shared_ptr<path_geometry> for Path.set_data
//         — the exact shapes_demo_page / path_gallery_page recipe. The resolved geometry is identical.
//   note: assigning a NEW geometry shared_ptr to Path.set_data fires the "data" mapper (the property
//         change), so the Path repaints — the port equivalent of C#'s `Path.Data = pathGeometry`. (For an
//         IN-PLACE geometry mutation the path exposes invalidate_data(); here we replace the whole
//         geometry, exactly like C#, so set_data alone retriggers.)
//   note: the C# counter math is reproduced verbatim: `_counter += 10` then the format string
//         "M 10,100 C 10,{300 + _counter} {300 + _counter},-200 {300 + _counter},100". The initial Data
//         (before any tap, counter 0) is the XAML "M 10,100 C 10,300 300,-200 300,100".
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
#include "maui/controls/shapes/path.hpp"
#include "maui/controls/shapes/path_geometry.hpp"
#include "maui/controls/shapes/path_markup_parser.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/graphics/colors.hpp"
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

            // Row 0 — the black-stroked cubic Bézier path (initial Data, counter 0).
            path_.set_data(make_geometry());
            path_.set_stroke(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::black));
            outer_.add(path_);
            outer_.set_row(path_, 0);

            // Row 1 — a status readout (port addition — surfaces the current counter / Data; see header note).
            outer_.add(readout_);
            outer_.set_row(readout_, 1);
            update_readout();

            // Row 2 — the "Update Path Data" button: each tap bumps the counter and replaces the geometry.
            update_button_.set_text("Update Path Data");
            update_button_.clicked.connect([this]() {
                counter_ += 10; // C# `_counter += 10`
                path_.set_data(make_geometry());
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
        [[nodiscard]] maui::controls::shapes::path& path()
        {
            return path_;
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

        // Parse the current Data markup into an owned path_geometry for Path.set_data (the path_gallery_page
        // recipe). A NEW shared_ptr fires the "data" mapper so the Path repaints (header note).
        [[nodiscard]] std::shared_ptr<maui::controls::shapes::path_geometry> make_geometry() const
        {
            return maui::controls::shapes::parse_path_geometry(data_markup());
        }

        void update_readout()
        {
            readout_.set_text("counter = " + std::to_string(counter_) + " | Data: " + data_markup());
        }

        int counter_ = 0; // C# `int _counter;`

        maui::controls::content_page page_;
        maui::controls::grid outer_;
        maui::controls::shapes::path path_;
        maui::controls::button update_button_;
        maui::controls::label readout_;
    };
} // namespace maui::samples
