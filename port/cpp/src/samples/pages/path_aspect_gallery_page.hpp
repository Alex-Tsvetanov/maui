#pragma once
// maui::samples::path_aspect_gallery_page — ports PathAspectGallery.xaml
//
// A self-contained, code-first port of the MAUI Shapes sub-gallery
// Pages/Controls/ShapesGalleries/PathAspectGallery.xaml: a StackLayout (Padding 12) that demonstrates
// the four Path Aspect modes on one identical geometry. Each Path is 100x100, BackgroundColor LightGray,
// Stroke Yellow, Fill Red, StrokeThickness 1, captioned, and differs only by Aspect —
//   - None:          the geometry is drawn at its natural size/offset, NOT stretched to the box;
//   - Fill:          stretched non-uniformly to fill the whole 100x100 box (the C# Stretch.Fill);
//   - Uniform:       scaled uniformly to fit inside the box (aspect ratio preserved, letterboxed);
//   - UniformToFill: scaled uniformly to cover the box (aspect ratio preserved, clipped).
//
// The geometry is the C# bird/duck-silhouette markup (a ~32x28 abbreviated-geometry "M…" string),
// re-used verbatim across all four so the only visual difference is the aspect fitting.
//
// The page OWNS its whole element tree (the shapes_demo_page pattern). It is backend-agnostic — a
// sample main attaches handlers bottom-up via the hosting layer and hosts page() in a window; the
// headless/apple/ios test trees exercise the same wiring directly.
//
// PORT NOTES (faithful best-effort, never invented):
//   note: the C# Path Aspect→Stretch mapping is reproduced by maui::core::path_aspect exactly as
//         shape.hpp documents the IShapeView.Aspect collapse: None→none, Fill→stretch,
//         Uniform→aspect_fit, UniformToFill→aspect_fill.
//   note: the C# <Style TargetType="Path"> sets HorizontalOptions="Start". This IS in the ported view
//         surface (View.HorizontalOptions -> set_horizontal_layout_alignment, honored at arrange time
//         by LayoutExtensions.ComputeFrame), so style_path sets layout_alignment::start on every Path —
//         matching the C# style. Without it the port default (Fill) on a 100-wide Path leaves the Path
//         CENTERED in the stack band instead of LEFT-aligned (the visual-parity divergence this fixes).
//   note: the C# Path FontSize="9" captions use Label.FontSize, which is not in the ported label
//         surface; the captions are plain labels with the same text (FontSize deferred).
//   note: BackgroundColor="LightGray" → set_background(solid_paint(colors::light_gray)); Stroke="Yellow"
//         / Fill="Red" → solid_paint over colors::yellow / colors::red (the named-brush → paint bridge).
//   note: the geometry string is parsed once into a shared path_geometry per Path via the WPF
//         abbreviated-geometry parser (parse_path_figure_collection). Each Path owns its own geometry
//         instance (geometry is not a logical child; a shared instance would still render identically,
//         but per-instance ownership mirrors the four distinct C# <Path Data="…"> elements).

#include <memory>
#include <string_view>

#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/shapes/path.hpp"
#include "maui/controls/shapes/path_geometry.hpp"
#include "maui/controls/shapes/path_markup_parser.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/layout_alignment.hpp"
#include "maui/core/path_aspect.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class path_aspect_gallery_page
    {
    public:
        path_aspect_gallery_page()
        {
            page_.set_title("Path Aspect Gallery");
            stack_.set_padding(maui::core::thickness(12)); // C# StackLayout Padding="12"

            // --- None.
            none_label_.set_text("None");
            stack_.add(none_label_);
            style_path(none_, maui::core::path_aspect::none);
            stack_.add(none_);

            // --- Fill.
            fill_label_.set_text("Fill");
            stack_.add(fill_label_);
            style_path(fill_, maui::core::path_aspect::stretch);
            stack_.add(fill_);

            // --- Uniform.
            uniform_label_.set_text("Uniform");
            stack_.add(uniform_label_);
            style_path(uniform_, maui::core::path_aspect::aspect_fit);
            stack_.add(uniform_);

            // --- UniformToFill.
            uniform_to_fill_label_.set_text("UniformToFill");
            stack_.add(uniform_to_fill_label_);
            style_path(uniform_to_fill_, maui::core::path_aspect::aspect_fill);
            stack_.add(uniform_to_fill_);

            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (caption + path pairs in stack add()-order, then
        // the stack, then the page), then re-host the tree built in the ctor (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, none_label_, "none_label_");
            gallery_attach_one(app, none_, "none_");
            gallery_attach_one(app, fill_label_, "fill_label_");
            gallery_attach_one(app, fill_, "fill_");
            gallery_attach_one(app, uniform_label_, "uniform_label_");
            gallery_attach_one(app, uniform_, "uniform_");
            gallery_attach_one(app, uniform_to_fill_label_, "uniform_to_fill_label_");
            gallery_attach_one(app, uniform_to_fill_, "uniform_to_fill_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(stack_); // stack hosts the captions + paths
            gallery_rehost_content(page_); // page hosts the stack
        }

        // The owned controls, exposed for the hosting main / tests.
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::shapes::path& none_path()
        {
            return none_;
        }
        [[nodiscard]] maui::controls::shapes::path& fill_path()
        {
            return fill_;
        }
        [[nodiscard]] maui::controls::shapes::path& uniform_path()
        {
            return uniform_;
        }
        [[nodiscard]] maui::controls::shapes::path& uniform_to_fill_path()
        {
            return uniform_to_fill_;
        }

    private:
        // The shared C# Path Data — the bird/duck-silhouette abbreviated geometry, re-used across all four.
        static constexpr std::string_view aspect_geometry_data{
            "M8.0886959,0L8.687694,0C12.279728,0.2989963 14.275696,2.2949993 15.971676,4.9890003 "
            "16.271724,4.5899982 16.470699,4.1909961 16.670711,3.8920001 18.765678,0.89799553 "
            "23.056684,-1.0980074 27.247655,0.79800445 28.544711,1.3970038 29.842683,2.2949993 "
            "30.740692,3.5919966 31.239652,4.3909931 31.837675,5.6880059 31.93765,6.8849973 "
            "32.336696,10.677006 30.740692,13.470998 29.442659,15.866003L26.648658,15.866003C26.149696,"
            "15.168005 26.050697,14.069998 25.351663,13.571004 24.453716,14.369009 24.353679,15.966009 "
            "23.75572,17.064001 23.156721,17.263006 22.457687,16.96401 21.759691,17.163 21.260667,"
            "17.761999 20.960681,19.359001 20.761707,20.257011 20.761707,19.458 20.561695,17.761999 "
            "20.462695,16.664007 20.262683,14.668997 20.162708,12.472997 19.963674,10.278004 19.863698,"
            "9.3800086 19.963674,8.1830015 19.164724,8.1830015 18.566703,8.1830015 18.466728,9.3800086 "
            "18.466728,9.9790077 18.266715,12.07401 17.867731,14.27001 17.468683,16.465002 16.969722,"
            "15.467001 16.670711,14.27001 16.171687,14.27001 15.57269,14.668997 15.27368,15.36701 "
            "14.973692,15.966009L13.975708,15.966009C13.876709,15.666998 13.576723,15.567007 13.277712,"
            "15.567007 12.878725,15.567007 12.47974,15.966009 12.47974,16.465002 12.47974,16.96401 "
            "12.878725,17.362997 13.277712,17.362997 13.476686,17.362997 13.776735,17.263006 13.876709,"
            "17.064001 14.375732,17.163 15.073729,17.064001 15.57269,17.064001 15.871701,16.664007 "
            "15.971676,16.265005 16.171687,15.966009 16.76971,16.763998 16.670711,18.161003 17.767694,"
            "18.660011 18.166679,18.361 18.266715,17.961998 18.366691,17.463003 18.566703,16.066 "
            "18.865714,14.569006 19.065725,13.071996 19.065725,12.873006 19.164724,11.675008 19.264699,"
            "11.375997 19.464712,14.069998 19.763723,17.761999 19.963674,20.556007 20.062671,21.354011 "
            "20.262683,21.554008 20.861682,21.953011 21.360704,21.554008 21.459703,21.454002 21.659715,"
            "20.855003 21.958665,20.157005 22.0587,19.359001 22.258712,18.560005 22.757675,18.461006 "
            "23.75572,18.760002 24.353679,18.461006 24.852703,17.662008 25.052713,16.364996 25.4517,"
            "15.567007 25.750711,16.066 25.950662,16.763998 26.249671,17.163L28.844699,17.163C28.445651,"
            "17.761999 27.846654,18.361 27.447667,18.760002 24.253703,22.352013 20.162708,25.545008 "
            "16.071712,27.641001 10.982733,24.84701 5.6937417,20.955009 2.4007186,15.567007 0.90371192,"
            "13.071996 -0.79226869,8.9810066 0.40475065,5.3889946 0.60476232,4.8900012 0.90371192,"
            "4.4899921 1.2037603,3.9909992 2.4007183,1.7959909 5.0947441,2.1702817E-07 8.0886959,0z"};

        // Configure one Path: parse the shared geometry, set BackgroundColor / Stroke / Fill / thickness /
        // size from the C# implicit Path style + per-element attributes, and apply the per-instance aspect.
        static void style_path(maui::controls::shapes::path& shape, maui::core::path_aspect aspect)
        {
            auto geometry = std::make_shared<maui::controls::shapes::path_geometry>();
            maui::controls::shapes::parse_path_figure_collection(geometry->figures(), aspect_geometry_data);
            shape.set_data(std::move(geometry));
            shape.set_aspect(aspect);
            // C# <Style TargetType="Path"> Setter: HorizontalOptions="Start" — left-align each Path at
            // its 100px width (honored by view::arrange -> compute_frame), not the Fill default.
            shape.set_horizontal_layout_alignment(maui::core::layout_alignment::start);
            shape.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::light_gray));
            shape.set_stroke(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::yellow));
            shape.set_fill(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::red));
            shape.set_stroke_thickness(1);
            shape.set_width_request(100);
            shape.set_height_request(100);
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label none_label_;
        maui::controls::shapes::path none_;
        maui::controls::label fill_label_;
        maui::controls::shapes::path fill_;
        maui::controls::label uniform_label_;
        maui::controls::shapes::path uniform_;
        maui::controls::label uniform_to_fill_label_;
        maui::controls::shapes::path uniform_to_fill_;
    };
} // namespace maui::samples
