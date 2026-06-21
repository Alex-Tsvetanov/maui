#pragma once
// maui::samples::shape_app_theme_page — ports ShapeAppThemeGallery.xaml
//
// A code-first port of the MAUI Shapes sub-gallery
// Pages/Controls/ShapesGalleries/ShapeAppThemeGallery.xaml: a StackLayout (Padding 12) holding a
// caption Label and a 200x80 Rectangle, all themed via {AppThemeBinding}. The XAML defines three
// theme-bound styles:
//   LayoutAppThemeStyle  — StackLayout.BackgroundColor = {AppThemeBinding White, Light=White, Dark=Black}
//   LabelAppThemeStyle   — Label.TextColor            = {AppThemeBinding Black, Light=Green, Dark=Red}
//   ShapeAppThemeStyle   — Rectangle.Stroke / Fill    = {AppThemeBinding Black, Light=Green, Dark=Red}
// The shape and the label thus turn GREEN in the light theme and RED in the dark theme, on a White
// (light) / Black (dark) page — the gallery's whole point: a Shape's Fill/Stroke tracking the OS theme.
//
// Demonstrated (all headless-safe maui:: API):
//   an AppThemeBinding applier that mirrors C# AppThemeBinding.GetValue — Dark → Dark-if-set else
//   Default; Light/Unspecified → Light-if-set else Default — driving rectangle::set_fill / set_stroke,
//   label::set_text_color, and the layout's background, re-applied on application::requested_theme_changed
//   (the live theme seam the XAML's DynamicResource styles ride).
//
// The page OWNS its whole element tree (the shapes_demo_page pattern). It is backend-agnostic — a
// sample main attaches handlers bottom-up via the hosting layer and hosts page() in a window; the
// headless/apple/ios test trees exercise the same wiring directly.
//
// PORT NOTES (faithful best-effort, never invented):
//   note: the C# code-behind is just InitializeComponent() — no logic to port; all behavior is the
//         three {AppThemeBinding} styles, reproduced here as an explicit theme applier (no XAML binding
//         engine / DynamicResource styling, layer 6). apply_theme() runs AppThemeBinding's own slot
//         pick: light/unspecified → Light slot, dark → Dark slot (both falling back to the Default).
//   note: attach_handlers reads the app's requested_theme() to seed the initial colors and subscribes
//         to requested_theme_changed to re-apply — the cross-platform stand-in for AppThemeBinding's
//         "__MAUI_ApplicationTheme__" resubscription. Until handlers are attached the page sits at its
//         default (Default-slot) colors, so the headless tree is still valid before hosting.
//   note: the C# styles' Default slot is White (layout) / Black (label, shape) — the value used when a
//         theme has no matching slot (here both Light and Dark are always set, so the Default only shows
//         for an Unspecified theme that the pick routes to the Light slot anyway, per GetValue).
//   note: the layout BackgroundColor is a brush in the port (view::set_background takes a paint), so the
//         themed background is a solid_paint over the picked color — the documented brush→paint bridge.

#include <memory>
#include <utility>

#include "maui/controls/application.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/shapes/rectangle.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/app_theme.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class shape_app_theme_page
    {
    public:
        shape_app_theme_page()
        {
            page_.set_title("Shapes AppTheme Gallery");
            stack_.set_padding(maui::core::thickness{12}); // C# LayoutAppThemeStyle Padding="12"

            label_.set_text("Shape using AppTheme"); // C# Label content
            stack_.add(label_);

            // C# Rectangle: HorizontalOptions Start, 200x80, themed Stroke + Fill.
            shape_.set_width_request(200);
            shape_.set_height_request(80);
            shape_.set_horizontal_layout_alignment(maui::core::layout_alignment::start); // C# HorizontalOptions=Start
            stack_.add(shape_);

            page_.set_content(stack_);

            // Seed at the Default-slot colors (no application yet); attach_handlers re-applies for the
            // live theme once a host is present.
            apply_theme(maui::core::app_theme::unspecified);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (the stack's children in add()-order, then the
        // stack, the page), re-host the ctor-built tree, then bind the live OS theme: seed from the app's
        // current requested_theme() and re-apply on every requested_theme_changed (the AppThemeBinding
        // resubscription stand-in).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, label_, "label_");
            gallery_attach_one(app, shape_, "shape_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(stack_); // stack hosts the label + the rectangle
            gallery_rehost_content(page_); // page hosts the stack

            if (const std::shared_ptr<maui::controls::application>& application = app.application())
            {
                apply_theme(application->requested_theme());
                application->requested_theme_changed.connect(
                    [this](maui::core::app_theme theme) { apply_theme(theme); });
            }
        }

        // The owned controls, exposed for the hosting main / tests.
        [[nodiscard]] maui::controls::shapes::rectangle& shape()
        {
            return shape_;
        }
        [[nodiscard]] maui::controls::label& label()
        {
            return label_;
        }
        [[nodiscard]] maui::controls::vertical_stack_layout& layout()
        {
            return stack_;
        }

    private:
        static std::shared_ptr<maui::graphics::solid_paint> solid(maui::graphics::color value)
        {
            return std::make_shared<maui::graphics::solid_paint>(value);
        }

        // C# AppThemeBinding.GetValue: dark picks the Dark slot; light AND unspecified pick the Light
        // slot (an unspecified theme defaults to the light branch). Both slots are always set in this
        // gallery, so the (Default) fallback is never reached here — it is kept to mirror the contract.
        struct theme_slots
        {
            maui::graphics::color default_value;
            maui::graphics::color light;
            maui::graphics::color dark;

            [[nodiscard]] maui::graphics::color pick(maui::core::app_theme theme) const
            {
                return theme == maui::core::app_theme::dark ? dark : light;
            }
        };

        // Re-apply the three themed styles for `theme`. Setting a fresh solid_paint / color retriggers the
        // mappers (the shape.hpp / view.hpp invalidation rule).
        void apply_theme(maui::core::app_theme theme)
        {
            // ShapeAppThemeStyle: Stroke/Fill = {AppThemeBinding Black, Light=Green, Dark=Red}.
            const maui::graphics::color shape_color =
                theme_slots{maui::graphics::colors::black, maui::graphics::colors::green, maui::graphics::colors::red}
                    .pick(theme);
            shape_.set_fill(solid(shape_color));
            shape_.set_stroke(solid(shape_color));

            // LabelAppThemeStyle: TextColor = {AppThemeBinding Black, Light=Green, Dark=Red}.
            label_.set_text_color(
                theme_slots{maui::graphics::colors::black, maui::graphics::colors::green, maui::graphics::colors::red}
                    .pick(theme));

            // LayoutAppThemeStyle: BackgroundColor = {AppThemeBinding White, Light=White, Dark=Black}.
            stack_.set_background(solid(
                theme_slots{maui::graphics::colors::white, maui::graphics::colors::white, maui::graphics::colors::black}
                    .pick(theme)));
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label label_;
        maui::controls::shapes::rectangle shape_;
    };
} // namespace maui::samples
