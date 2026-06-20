#pragma once
// maui::samples::swipe_gesture_page — ports SwipeViewGestureRecognizerGallery.xaml (+ .xaml.cs)
//
// The MAUI SwipeViewGestureRecognizerGallery is a CollectionView of "message" rows; each row's DataTemplate
// is a SwipeView wired three ways, proving gesture recognizers AND swipe-item commands coexist on one
// SwipeView:
//   - the SwipeView itself carries a TapGestureRecognizer (NumberOfTapsRequired=2, Command=TapCommand) —
//     a double-tap on the row fires TapCommand,
//   - LeftItems has a SwipeItem ("Favourite", Command=FavouriteCommand) — invoking it fires FavouriteCommand,
//   - RightItems has a SwipeItemView ("Delete") whose OWN GestureRecognizers carry a TapGestureRecognizer
//     (Command=DeleteCommand) — tapping the custom delete content fires DeleteCommand.
// The .xaml.cs registers WeakReferenceMessenger handlers for "favourite"/"delete"/"tap" that each pop a
// DisplayAlert; the ViewModel's three commands send those messages.
//
// The CollectionView + DataTemplate + ViewModel + WeakReferenceMessenger are a data-binding/navigation
// stack with no headless analog, so — following the gestures_page precedent — this code-first port builds a
// SINGLE representative message row (one instantiation of the template) carrying the same three channels,
// each wired to a readout label, and synthetically drives each so the wiring is observable on one static
// headless capture.
//
// COMMAND MAPPING (the W1-11 collapse): TapGestureRecognizer DOES carry a real i_command (command.hpp), so
// the two TapGestureRecognizers get real maui::controls::command instances (mirroring the XAML Command
// bindings). SwipeItem has NO Command in the port — its command channel IS the invoked/clicked event — so
// the "Favourite" SwipeItem wires through `invoked` instead (the observable equivalent of FavouriteCommand).
//
// Self-contained (the gestures_page pattern): the page OWNS its whole element tree, exposes page() and
// attach_handlers(maui_app); attach_handlers() also OPENS the SwipeView (so the revealed items show) and
// drives the two tap recognizers + the favourite item so the readout reflects all three channels.

#include <any>
#include <cstdio>
#include <memory>
#include <string>

#include "maui/controls/command.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/gestures/tap_gesture_recognizer.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/swipe_item.hpp"
#include "maui/controls/swipe_item_view.hpp"
#include "maui/controls/swipe_view.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/font.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/open_swipe_item.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class swipe_gesture_page
    {
    public:
        swipe_gesture_page()
        {
            page_.set_title("SwipeView GestureRecognizer Gallery");
            root_.set_spacing(8);

            // The C# black banner above the CollectionView.
            banner_.set_text("The SwipeView has a TapGestureRecognizer. Tap two times the SwipeView to show a dialog.");
            banner_.set_background(solid(maui::graphics::colors::black));
            banner_.set_text_color(maui::graphics::colors::white);
            readout_.set_text("Ready (double-tap row / swipe to favourite or delete)");

            // ---- The SwipeView's own TapGestureRecognizer (double-tap -> TapCommand) ----
            tap_recognizer_->set_number_of_taps_required(2);
            tap_recognizer_->set_command(std::make_shared<maui::controls::command>(
                [this](const std::any&) { readout_.set_text("TapCommand (double-tap)"); }));
            swipe_.gesture_recognizers().add(tap_recognizer_);

            // ---- LeftItems: a "Favourite" SwipeItem. C# Command=FavouriteCommand -> the port's invoked. ----
            favourite_item_.set_text("Favourite"); // C# IconImageSource="calculator.png" — omitted headless
            favourite_item_.set_background_color(maui::graphics::colors::yellow);
            favourite_item_.invoked.connect([this] { readout_.set_text("FavouriteCommand"); });
            swipe_.left_items_collection().add(favourite_item_);

            // ---- RightItems: a custom "Delete" SwipeItemView whose own GestureRecognizers tap -> DeleteCommand ----
            delete_content_.set_background(solid(maui::graphics::colors::red));
            delete_content_.set_width_request(100);
            delete_label_.set_text("Delete"); // C# also has an Image (coffee.png) above — omitted headless
            delete_label_.set_text_color(maui::graphics::colors::white);
            delete_content_.add(delete_label_);
            delete_tap_->set_command(std::make_shared<maui::controls::command>(
                [this](const std::any&) { readout_.set_text("DeleteCommand"); }));
            delete_item_view_.gesture_recognizers().add(delete_tap_); // the recognizer lives on the item view
            delete_item_view_.set_content(delete_content_);
            swipe_.right_items_collection().add(delete_item_view_);

            // ---- The SwipeView Content: the white "message" card (Title / Date / SubTitle / Description) ----
            // Oracle: a Grid (RowSpacing="0") with columns [*, Auto] and rows [Auto, Auto, *]. Title and Date
            // share row 0 (title in the * column, date right in the Auto column); SubTitle (row 1) and
            // Description (row 2) each span both columns. Without these definitions + cell placement all four
            // labels land in row 0 / column 0 and render stacked on top of each other. The XAML Title/Date/
            // SubTitle resource styles are inlined here (font size + colour + margin) for visual parity.
            card_.set_background(solid(maui::graphics::colors::white));
            card_.set_row_spacing(0);
            card_.add_column_definition(maui::core::grid_length::star());      // col 0: *
            card_.add_column_definition(maui::core::grid_length::automatic()); // col 1: Auto
            card_.add_row_definition(maui::core::grid_length::automatic());    // row 0: Auto (Title / Date)
            card_.add_row_definition(maui::core::grid_length::automatic());    // row 1: Auto (SubTitle)
            card_.add_row_definition(maui::core::grid_length::star());         // row 2: *    (Description)

            // TitleStyle: FontSize 14, TextColor Black, Margin 6,0,6,6.
            title_.set_text("Welcome to .NET MAUI!");
            title_.set_font(maui::core::font::system_font_of_size(14));
            title_.set_text_color(maui::graphics::colors::black);
            title_.set_margin(maui::core::thickness{6, 0, 6, 6});

            // DateStyle: FontSize 10, TextColor DarkGray, Margin 6,0,6,6.
            date_.set_text("June 2026");
            date_.set_font(maui::core::font::system_font_of_size(10));
            date_.set_text_color(maui::graphics::colors::dark_gray);
            date_.set_margin(maui::core::thickness{6, 0, 6, 6});

            // SubTitleStyle: FontSize 12, TextColor DarkGray, Margin 6,0.
            subtitle_.set_text("A SwipeView with gesture recognizers");
            subtitle_.set_font(maui::core::font::system_font_of_size(12));
            subtitle_.set_text_color(maui::graphics::colors::dark_gray);
            subtitle_.set_margin(maui::core::thickness{6, 0});

            // Description shares SubTitleStyle.
            description_.set_text("Double-tap the card, or swipe to Favourite / Delete.");
            description_.set_font(maui::core::font::system_font_of_size(12));
            description_.set_text_color(maui::graphics::colors::dark_gray);
            description_.set_margin(maui::core::thickness{6, 0});

            card_.add(title_);
            card_.add(date_);
            card_.add(subtitle_);
            card_.add(description_);
            card_.set_column(title_, 0);
            card_.set_row(title_, 0);
            card_.set_column(date_, 1);
            card_.set_row(date_, 0);
            card_.set_column(subtitle_, 0);
            card_.set_column_span(subtitle_, 2);
            card_.set_row(subtitle_, 1);
            card_.set_column(description_, 0);
            card_.set_column_span(description_, 2);
            card_.set_row(description_, 2);
            swipe_.set_content(card_);

            root_.add(banner_);
            root_.add(swipe_);
            root_.add(readout_);
            page_.set_content(root_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED VIEW, BOTTOM-UP (the card + delete content first, the page last),
        // then re-host the ctor-built tree. The "Favourite" SwipeItem is a NON-view menu item with no
        // standalone handler, so it is excluded (attaching it would throw). Headless has no native input, so
        // finish by opening the SwipeView and driving each of the three channels synthetically so the readout
        // reflects the full wiring in a static capture. (gallery_attach.hpp)
        void attach_handlers(maui::hosting::maui_app& app)
        {
            // The card content (deepest first).
            gallery_attach_one(app, title_, "title_");
            gallery_attach_one(app, date_, "date_");
            gallery_attach_one(app, subtitle_, "subtitle_");
            gallery_attach_one(app, description_, "description_");
            gallery_attach_one(app, card_, "card_");

            // The delete custom item view content.
            gallery_attach_one(app, delete_label_, "delete_label_");
            gallery_attach_one(app, delete_content_, "delete_content_");
            gallery_attach_one(app, delete_item_view_, "delete_item_view_");

            gallery_attach_one(app, banner_, "banner_");
            gallery_attach_one(app, readout_, "readout_");
            gallery_attach_one(app, swipe_, "swipe_");
            gallery_attach_one(app, root_, "root_");
            gallery_attach_one(app, page_, "page_");

            // Replay the host commands now that handlers exist.
            gallery_rehost_layout(card_);
            gallery_rehost_layout(delete_content_);
            gallery_rehost_content(delete_item_view_);
            gallery_rehost_content(swipe_);
            gallery_rehost_layout(root_);
            gallery_rehost_content(page_);

            drive_synthetic_gestures();
        }

        // One deterministic drive per channel (the same path the platform bridges + the gesture/swipe unit
        // tests use), leaving the readout on the last channel. Order: open the SwipeView (reveal the items),
        // fire the favourite item, fire the delete recognizer, then the SwipeView's own double-tap.
        void drive_synthetic_gestures()
        {
            // Reveal the LeftItems so the favourite item shows in the capture.
            swipe_.open(maui::core::open_swipe_item::left_items, /*animated=*/false);

            // FavouriteCommand: invoke the SwipeItem (its invoked channel — the port's command equivalent).
            favourite_item_.on_invoked();

            // DeleteCommand: tap the delete item view's recognizer (NumberOfTapsRequired default 1).
            delete_tap_->send_tapped(delete_item_view_, maui::graphics::point{50, 40});

            // TapCommand: the SwipeView's own double-tap recognizer (a single send_tapped fires the command;
            // the C# NumberOfTapsRequired=2 is a native tap-counter — the recognizer's command runs on the
            // recognized tap regardless, as the tap unit tests drive it).
            tap_recognizer_->send_tapped(swipe_, maui::graphics::point{120, 40});
        }

        // The owned controls + recognizers, exposed for the hosting main / tests.
        [[nodiscard]] maui::controls::swipe_view& swipe()
        {
            return swipe_;
        }
        [[nodiscard]] maui::controls::swipe_item& favourite_item()
        {
            return favourite_item_;
        }
        [[nodiscard]] maui::controls::swipe_item_view& delete_item_view()
        {
            return delete_item_view_;
        }
        [[nodiscard]] const std::shared_ptr<maui::controls::tap_gesture_recognizer>& tap_recognizer()
        {
            return tap_recognizer_;
        }
        [[nodiscard]] const std::shared_ptr<maui::controls::tap_gesture_recognizer>& delete_tap()
        {
            return delete_tap_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }

    private:
        static std::shared_ptr<maui::graphics::solid_paint> solid(maui::graphics::color value)
        {
            return std::make_shared<maui::graphics::solid_paint>(value);
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout root_;
        maui::controls::label banner_;
        maui::controls::label readout_;

        maui::controls::swipe_view swipe_;

        // The SwipeView's own double-tap recognizer (TapCommand).
        std::shared_ptr<maui::controls::tap_gesture_recognizer> tap_recognizer_ =
            std::make_shared<maui::controls::tap_gesture_recognizer>();

        // LeftItems: the "Favourite" menu item (FavouriteCommand via invoked).
        maui::controls::swipe_item favourite_item_; // owned: the swipe collection is non-owning

        // RightItems: the custom "Delete" item view + its own tap recognizer (DeleteCommand).
        maui::controls::swipe_item_view delete_item_view_;
        maui::controls::grid delete_content_;
        maui::controls::label delete_label_;
        std::shared_ptr<maui::controls::tap_gesture_recognizer> delete_tap_ =
            std::make_shared<maui::controls::tap_gesture_recognizer>();

        // The SwipeView Content: the white message card.
        maui::controls::grid card_;
        maui::controls::label title_;
        maui::controls::label date_;
        maui::controls::label subtitle_;
        maui::controls::label description_;
    };
} // namespace maui::samples
