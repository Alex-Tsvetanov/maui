#pragma once
// maui::samples::clip_views_page — ports ClipViewsGallery.xaml
//
// The C# page (Pages/Controls/ShapesGalleries/ClipViewsGallery.xaml; no code-behind beyond an empty
// InitializeComponent) is a ScrollView over a StackLayout (Padding=12) that proves the Clip surface
// (VisualElement.Clip / IView.Clip) works on EVERY kind of view — not just images. SEVEN different
// controls, each BackgroundColor="Red", all share the SAME ellipse clip declared once as a resource:
//
//   <EllipseGeometry x:Key="EllipseClip" RadiusX="300" RadiusY="50" />
//
// applied via Clip="{StaticResource EllipseClip}" to:
//   - a Button   (Text="Button"),
//   - a DatePicker,
//   - an Entry   (Placeholder="Entry"),
//   - an Editor  (Placeholder="Editor"),
//   - a Grid     (HeightRequest=50) containing a Label Text="Grid",
//   - a SearchBar, and
//   - a TimePicker.
//
// The shared resource means all seven views reference ONE EllipseGeometry instance — the port mirrors
// that exactly: a single std::shared_ptr<ellipse_geometry> is set_clip()'d onto every control. The C#
// EllipseGeometry has no Center, so it defaults to (0,0); ported as ellipse_geometry({0,0}, 300, 50).
//
// PORT MAPPING:
//   - EllipseGeometry(RadiusX=300, RadiusY=50)  -> shapes::ellipse_geometry(point{0,0}, 300, 50)
//     (geometry : i_shape — controls/shapes/geometry.hpp — so it ports 1:1 to view::set_clip).
//   - the StaticResource shared across views    -> ONE shared_ptr, set_clip()'d onto each control.
//   - BackgroundColor="Red" on each control     -> view::set_background(solid_paint(Red)).
//
// HEADLESS-SAFE maui:: API only; the page OWNS its whole element tree and attaches every owned view
// bottom-up, then re-hosts (the clip_page / clipping_page convention — gallery_attach.hpp). This is a
// VISUAL clip demo: each red control renders clipped to the ellipse once a backend draws pixels.
//
// note: several of these value controls (date_picker / time_picker, and the others) may have no AppKit
//       handler registered — gallery_attach_one swallows the per-control attach throw and logs+continues,
//       so the page still mounts the controls that DO resolve. The Clip property is set on all of them
//       regardless, faithfully to the XAML.

#include <memory>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/date_picker.hpp"
#include "maui/controls/editor.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/search_bar.hpp"
#include "maui/controls/shapes/ellipse_geometry.hpp"
#include "maui/controls/time_picker.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class clip_views_page
    {
    public:
        clip_views_page()
        {
            page_.set_title("ClipViews Gallery");
            stack_.set_padding(maui::core::thickness{12}); // StackLayout Padding="12"
            stack_.set_spacing(6);

            // The single shared resource: <EllipseGeometry x:Key="EllipseClip" RadiusX="300" RadiusY="50"/>.
            // EllipseGeometry's Center defaults to (0,0). One instance, referenced by all seven controls.
            ellipse_clip_ =
                std::make_shared<maui::controls::shapes::ellipse_geometry>(maui::graphics::point{0, 0}, 300, 50);

            // Button BackgroundColor="Red" Text="Button" Clip="{StaticResource EllipseClip}".
            button_.set_text("Button");
            paint_red(button_);
            button_.set_clip(ellipse_clip_);
            stack_.add(button_);

            // DatePicker BackgroundColor="Red" Clip="{StaticResource EllipseClip}".
            paint_red(date_picker_);
            date_picker_.set_clip(ellipse_clip_);
            stack_.add(date_picker_);

            // Entry BackgroundColor="Red" Placeholder="Entry" Clip="{StaticResource EllipseClip}".
            entry_.set_placeholder("Entry");
            paint_red(entry_);
            entry_.set_clip(ellipse_clip_);
            stack_.add(entry_);

            // Editor BackgroundColor="Red" Placeholder="Editor" Clip="{StaticResource EllipseClip}".
            editor_.set_placeholder("Editor");
            paint_red(editor_);
            editor_.set_clip(ellipse_clip_);
            stack_.add(editor_);

            // Grid BackgroundColor="Red" HeightRequest=50 Clip="{StaticResource EllipseClip}" with a
            // single child <Label Text="Grid" />.
            grid_.set_height_request(50);
            paint_red(grid_);
            grid_.set_clip(ellipse_clip_);
            grid_label_.set_text("Grid");
            grid_.add(grid_label_);
            stack_.add(grid_);

            // SearchBar BackgroundColor="Red" Clip="{StaticResource EllipseClip}".
            paint_red(search_bar_);
            search_bar_.set_clip(ellipse_clip_);
            stack_.add(search_bar_);

            // TimePicker BackgroundColor="Red" Clip="{StaticResource EllipseClip}".
            paint_red(time_picker_);
            time_picker_.set_clip(ellipse_clip_);
            stack_.add(time_picker_);

            scroll_.set_content(stack_);
            page_.set_content(scroll_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (leaves first: the grid's label, then the grid,
        // then the remaining controls, then the stack, scroll, page), then re-host the tree built in the
        // ctor. gallery_attach_one swallows any per-control attach throw so an unregistered value control
        // can't abort the page. (gallery_attach.hpp)
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, button_, "button_");
            gallery_attach_one(app, date_picker_, "date_picker_");
            gallery_attach_one(app, entry_, "entry_");
            gallery_attach_one(app, editor_, "editor_");
            gallery_attach_one(app, grid_label_, "grid_label_");
            gallery_attach_one(app, grid_, "grid_");
            gallery_attach_one(app, search_bar_, "search_bar_");
            gallery_attach_one(app, time_picker_, "time_picker_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, scroll_, "scroll_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(grid_);    // the grid hosts its single label child
            gallery_rehost_layout(stack_);   // the stack hosts every clipped control
            gallery_rehost_content(scroll_); // the scroll hosts the stack
            gallery_rehost_content(page_);   // the page hosts the scroll
        }

        // Owned controls exposed for the hosting main / inspection.
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::scroll_view& scroll()
        {
            return scroll_;
        }
        [[nodiscard]] maui::controls::button& button()
        {
            return button_;
        }
        [[nodiscard]] maui::controls::grid& grid()
        {
            return grid_;
        }
        [[nodiscard]] maui::controls::entry& entry()
        {
            return entry_;
        }
        [[nodiscard]] maui::controls::editor& editor()
        {
            return editor_;
        }
        [[nodiscard]] maui::controls::search_bar& search_bar()
        {
            return search_bar_;
        }

    private:
        // BackgroundColor="Red" — Red = #FF0000. The generic `auto&` keeps each control's concrete type.
        template <class View> static void paint_red(View& view)
        {
            view.set_background(
                std::make_shared<maui::graphics::solid_paint>(maui::graphics::color::from_rgb(255, 0, 0)));
        }

        maui::controls::content_page page_;
        maui::controls::scroll_view scroll_;
        maui::controls::vertical_stack_layout stack_;

        maui::controls::button button_;
        maui::controls::date_picker date_picker_;
        maui::controls::entry entry_;
        maui::controls::editor editor_;
        maui::controls::grid grid_;
        maui::controls::label grid_label_;
        maui::controls::search_bar search_bar_;
        maui::controls::time_picker time_picker_;

        // The single shared ellipse clip — all seven controls reference this one instance (mirroring the
        // XAML StaticResource).
        std::shared_ptr<maui::controls::shapes::ellipse_geometry> ellipse_clip_;
    };
} // namespace maui::samples
