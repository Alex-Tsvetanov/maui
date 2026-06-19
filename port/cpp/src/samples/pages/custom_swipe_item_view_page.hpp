#pragma once
// maui::samples::custom_swipe_item_view_page — ports CustomSwipeItemViewGallery.xaml
//
// A self-contained, code-first port of the .NET MAUI "CustomSwipeItem" gallery: a message-list row whose
// right swipe reveals a CUSTOM-content swipe item (a swipe_item_view, not a plain swipe_item). The MAUI
// page binds a CollectionView to a MessagesViewModel via a MessageTemplate DataTemplate; the port has no
// CollectionView/ItemsSource here (an out-of-scope item-view host), so it materializes ONE representative
// row instance of that template — the same tree the DataTemplate produces per message:
//
//   swipe_view (HeightRequest 80, RightItems Mode=Reveal)
//     RightItems -> swipe_item_view (custom content):
//        border (Width 100, #FE744D, RoundRectangle CornerRadius 0,6,0,6)
//          label "Favourite" (white, bold, centered)
//     Content -> frame (#2E249E, CornerRadius 6, no shadow, Padding 12)
//        grid (two Auto rows)
//          label Title (#55A1FA, FontSize 10)
//          label Date  (#FFFFFF, FontSize 18)
//
// Interactions demonstrated:
//   - the right swipe item is a CUSTOM swipe_item_view: its invoked event IS the command channel (the
//     W1-11 command-as-event collapse, matching MAUI's Command="...FavouriteCommand" binding). invoked
//     drives the readout to "Favourite invoked".
//   - attach_handlers() synthetically OPENS the right items (open(right_items)) so the static capture
//     shows the revealed custom swipe item content, and reports the open via open_requested → readout.
//
// The page OWNS its whole element tree; it is backend-agnostic. A sample main attaches handlers bottom-up
// and hosts page() in a window; the headless/apple/ios trees exercise the same wiring.
//
// note: MAUI's CollectionView ItemsSource binding to a SwipeViewGalleryViewModel (a list of Message
// {Title, Date}) and the WeakReferenceMessenger "favourite"/"delete" DisplayAlert handlers are not
// ported — the item-view host + messenger + modal alert are out of scope at this layer. The single
// representative row + the invoked-driven readout capture the same observable swipe interaction.

#include <cstdio>
#include <memory>

#include "maui/controls/border.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/frame.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/swipe_item_view.hpp"
#include "maui/controls/swipe_view.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/font.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/open_swipe_item.hpp"
#include "maui/core/swipe_mode.hpp"
#include "maui/core/swipe_view_requests.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/shapes/round_rectangle.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class custom_swipe_item_view_page
    {
    public:
        custom_swipe_item_view_page()
        {
            page_.set_title("CustomSwipeItem");
            stack_.set_spacing(12);
            stack_.set_padding(maui::core::thickness{12, 12, 12, 0});

            readout_.set_text("Swipe a row left to reveal the Favourite item");

            // ---- the custom swipe item content (Border > Label "Favourite") ----
            favourite_label_.set_text("Favourite");
            favourite_label_.set_text_color(maui::graphics::colors::white);
            favourite_label_.set_font(maui::core::font::system_font_of_size(12, maui::core::font_weight::bold));
            favourite_label_.set_horizontal_text_alignment(maui::core::text_alignment::center);
            favourite_label_.set_vertical_text_alignment(maui::core::text_alignment::center);

            favourite_border_.set_width_request(100);
            favourite_border_.set_background(
                std::make_shared<maui::graphics::solid_paint>(maui::graphics::color::from_argb("#FE744D")));
            // StrokeShape RoundRectangle CornerRadius="0, 6, 0, 6" => (TL, TR, BL, BR).
            favourite_border_.set_stroke_shape(
                std::make_shared<maui::graphics::shapes::round_rectangle>(maui::graphics::corner_radius{0, 6, 0, 6}));
            favourite_border_.set_content(favourite_label_);

            // ---- the CUSTOM swipe item (swipe_item_view hosting the border) ----
            // C# SwipeItemView Command="{Binding ...FavouriteCommand}". The port's command channel IS the
            // invoked event (W1-11 collapse), so invoked drives the readout.
            favourite_item_.set_content(favourite_border_);
            favourite_item_.invoked.connect([this] { readout_.set_text("Favourite invoked"); });

            // ---- the row content (Frame > Grid with Title + Date) ----
            title_label_.set_text("Welcome to .NET MAUI");
            title_label_.set_text_color(maui::graphics::color::from_argb("#55A1FA"));
            title_label_.set_font(maui::core::font::system_font_of_size(10));
            title_label_.set_padding(maui::core::thickness{6, 0, 6, 6});

            date_label_.set_text("June 19, 2026");
            date_label_.set_text_color(maui::graphics::color::from_argb("#FFFFFF"));
            date_label_.set_font(maui::core::font::system_font_of_size(18));
            date_label_.set_padding(maui::core::thickness{6, 0, 6, 6});

            row_grid_.add_row_definition(maui::core::grid_length::automatic());
            row_grid_.add_row_definition(maui::core::grid_length::automatic());
            row_grid_.set_row_spacing(0);
            row_grid_.add(title_label_);
            row_grid_.set_row(title_label_, 0);
            row_grid_.add(date_label_);
            row_grid_.set_row(date_label_, 1);

            row_frame_.set_background(
                std::make_shared<maui::graphics::solid_paint>(maui::graphics::color::from_argb("#2E249E")));
            row_frame_.set_corner_radius(6);
            row_frame_.set_has_shadow(false);
            row_frame_.set_padding(maui::core::thickness{12});
            row_frame_.set_content(row_grid_);

            // ---- the swipe_view (HeightRequest 80, RightItems Mode=Reveal -> the custom item) ----
            swipe_.set_height_request(80);
            swipe_.right_items_collection().set_mode(maui::core::swipe_mode::reveal);
            swipe_.right_items_collection().add(favourite_item_);
            swipe_.set_content(row_frame_);
            swipe_.open_requested.connect([this](const maui::core::swipe_view_open_request& /*request*/) {
                readout_.set_text("Right items revealed (Favourite)");
            });

            stack_.add(readout_);
            stack_.add(swipe_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (leaves first, the page last) so each parent can
        // host its child's native view, then re-host the tree built in the ctor (gallery_attach.hpp). The
        // swipe_item_view IS a view (custom content) so it gets a handler + a content re-host; the swipe
        // items collection and the (non-existent) plain swipe_item are non-view and excluded.
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, readout_, "readout_");
            gallery_attach_one(app, favourite_label_, "favourite_label_");
            gallery_attach_one(app, favourite_border_, "favourite_border_");
            gallery_attach_one(app, favourite_item_, "favourite_item_");
            gallery_attach_one(app, title_label_, "title_label_");
            gallery_attach_one(app, date_label_, "date_label_");
            gallery_attach_one(app, row_grid_, "row_grid_");
            gallery_attach_one(app, row_frame_, "row_frame_");
            gallery_attach_one(app, swipe_, "swipe_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, page_, "page_");

            // Replay the host commands the ctor fired before any handler existed.
            gallery_rehost_content(favourite_border_); // border hosts the "Favourite" label
            gallery_rehost_content(favourite_item_);   // custom swipe item hosts the border
            gallery_rehost_layout(row_grid_);          // grid hosts the title + date labels
            gallery_rehost_content(row_frame_);        // frame hosts the grid
            gallery_rehost_content(swipe_);            // swipe_view hosts the frame row
            gallery_rehost_layout(stack_);             // stack hosts the readout + swipe
            gallery_rehost_content(page_);             // page hosts the stack

            // Synthetically OPEN the right items so the static capture shows the revealed custom swipe item.
            swipe_.open(maui::core::open_swipe_item::right_items);
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment + the tests.
        [[nodiscard]] maui::controls::swipe_view& swipe()
        {
            return swipe_;
        }
        [[nodiscard]] maui::controls::swipe_item_view& favourite_item()
        {
            return favourite_item_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }

    private:
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label readout_;

        // The custom swipe item (RightItems): swipe_item_view > border > label.
        maui::controls::swipe_item_view favourite_item_;
        maui::controls::border favourite_border_;
        maui::controls::label favourite_label_;

        // The row content: frame > grid > {title, date}.
        maui::controls::swipe_view swipe_;
        maui::controls::frame row_frame_;
        maui::controls::grid row_grid_;
        maui::controls::label title_label_;
        maui::controls::label date_label_;
    };
} // namespace maui::samples
