#pragma once
// maui::samples::swipe_item_position_page — ports SwipeItemPositionGallery.xaml
//
// A code-first port of the MAUI SwipeView sub-gallery
// Pages/Controls/SwipeViewGalleries/SwipeItemPositionGallery.xaml: a 2-row Grid (Auto / *) with a Picker
// on top and one SwipeView below that carries TWO SwipeItems in EACH of the four directional collections
// (Left / Top / Right / Bottom). The Picker chooses the SwipeMode (Reveal or Execute) that is applied to
// ALL four collections at once.
//
// Layout:
//   - Row 0 (Auto): a Picker "Select a Mode" with items {"Reveal", "Execute"};
//   - Row 1 (*):    a SwipeView over a translucent white Grid labelled "Swipe in any direction", with
//                   Left {red "Left Item 1", indianred "Left Item 2"}, Top {blue "Top Item 1", darkblue
//                   "Right Item 2"}, Right {orange "Right Item 1", darkorange "Right Item 2"}, Bottom
//                   {darkgreen "Bottom Item 1", lawngreen "Bottom Item 2"}.
//
// The page OWNS its whole element tree (the swipe_refresh_page / value_controls_page pattern). It is
// backend-agnostic — a sample main attaches handlers bottom-up via the hosting layer and hosts page() in a
// window; the headless/apple/ios test trees exercise the same control wiring directly.
//
// Interactions demonstrated:
//   - the Picker's SelectedIndexChanged sets the Mode of all four SwipeItems collections to Reveal
//     (index 0) or Execute (index 1) — the exact C# OnModePickerSelectedIndexChanged fan-out;
//   - the ctor preselects index 0 (Reveal), as the C# code-behind does (ModePicker.SelectedIndex = 0);
//   - in attach_handlers, the SwipeView is synthetically opened toward its populated RightItems side so a
//     static capture shows the two revealed right items (the swipe-to-reveal gesture has no headless
//     analogue; open() routes through the now-attached handler — see swipe_view_seam tests).
//
// PORT NOTES (faithful best-effort, never invented):
//   note: the inner content Grid's Opacity="0.75" + BackgroundColor="White" — opacity is set via
//         set_opacity(0.75); the white background is reconstructed via set_background over a solid_paint
//         (the documented Brush→Paint bridge: there is no set_background_color on a view).
//   note: the eight SwipeItem colours map straight to the named colours (Red / IndianRed / Blue / DarkBlue
//         / Orange / DarkOrange / DarkGreen / LawnGreen); the XAML "Right Item 2" texts on the Top + Right
//         collections are preserved verbatim (they are the gallery's labels, copied as-is, not invented).
//   note: the four x:Name'd SwipeItems collections in the XAML are the swipe_view's own owned collections
//         (left_items_collection() etc.) — the C# names them only to retarget their Mode, which the port
//         does directly on those collections.

#include <memory>

#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/picker.hpp"
#include "maui/controls/swipe_item.hpp"
#include "maui/controls/swipe_view.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/open_swipe_item.hpp"
#include "maui/core/swipe_mode.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class swipe_item_position_page
    {
    public:
        swipe_item_position_page()
        {
            page_.set_title("SwipeItem Position Gallery");

            // The outer 2-row grid: Auto (picker) / * (swipe view).
            outer_.add_row_definition(maui::core::grid_length::automatic());
            outer_.add_row_definition(maui::core::grid_length::star());

            // Row 0 — the Mode picker.
            mode_picker_.set_title("Select a Mode");
            mode_picker_.items().add("Reveal");
            mode_picker_.items().add("Execute");
            mode_picker_.selected_index_changed.connect([this] { apply_mode(); });
            outer_.add(mode_picker_);
            outer_.set_row(mode_picker_, 0);

            // Row 1 — the swipe view over a translucent white labelled grid.
            content_.set_opacity(0.75);
            content_.set_background(white());
            content_label_.set_text("Swipe in any direction");
            content_.add(content_label_);

            // Left {red, indianred}
            left_1_.set_text("Left Item 1");
            left_1_.set_background_color(maui::graphics::colors::red);
            left_2_.set_text("Left Item 2");
            left_2_.set_background_color(maui::graphics::colors::indian_red);
            swipe_.left_items_collection().add(left_1_);
            swipe_.left_items_collection().add(left_2_);

            // Top {blue, darkblue} — texts preserved verbatim from the XAML ("Top Item 1" / "Right Item 2").
            top_1_.set_text("Top Item 1");
            top_1_.set_background_color(maui::graphics::colors::blue);
            top_2_.set_text("Right Item 2");
            top_2_.set_background_color(maui::graphics::colors::dark_blue);
            swipe_.top_items_collection().add(top_1_);
            swipe_.top_items_collection().add(top_2_);

            // Right {orange, darkorange}
            right_1_.set_text("Right Item 1");
            right_1_.set_background_color(maui::graphics::colors::orange);
            right_2_.set_text("Right Item 2");
            right_2_.set_background_color(maui::graphics::colors::dark_orange);
            swipe_.right_items_collection().add(right_1_);
            swipe_.right_items_collection().add(right_2_);

            // Bottom {darkgreen, lawngreen}
            bottom_1_.set_text("Bottom Item 1");
            bottom_1_.set_background_color(maui::graphics::colors::dark_green);
            bottom_2_.set_text("Bottom Item 2");
            bottom_2_.set_background_color(maui::graphics::colors::lawn_green);
            swipe_.bottom_items_collection().add(bottom_1_);
            swipe_.bottom_items_collection().add(bottom_2_);

            swipe_.set_content(content_);
            outer_.add(swipe_);
            outer_.set_row(swipe_, 1);

            page_.set_content(outer_);

            // C# code-behind: ModePicker.SelectedIndex = 0 (Reveal). Setting the index raises
            // SelectedIndexChanged, which runs apply_mode() — so the four collections start in Reveal.
            mode_picker_.set_selected_index(0);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (the content label, the content Grid, the picker,
        // the swipe_view, the outer Grid, then the page), then re-host the tree built in the ctor. The eight
        // swipe_item members are NON-view items (no standalone handler), deliberately excluded — attaching
        // one would throw. After re-hosting, synthetically open the swipe_view toward its RightItems side so
        // a static capture shows the two revealed right items. (gallery_attach.hpp)
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, content_label_, "content_label_");
            gallery_attach_one(app, content_, "content_");
            gallery_attach_one(app, mode_picker_, "mode_picker_");
            gallery_attach_one(app, swipe_, "swipe_");
            gallery_attach_one(app, outer_, "outer_");
            gallery_attach_one(app, page_, "page_");

            // The tree was built in the ctor before any handler existed, so replay the host commands now.
            gallery_rehost_layout(content_); // content grid hosts its label
            gallery_rehost_content(swipe_);  // swipe_view hosts the content grid
            gallery_rehost_layout(outer_);   // outer grid hosts the picker + swipe_view
            gallery_rehost_content(page_);   // page hosts the outer grid

            // Static-capture seam: reveal the RightItems (the swipe gesture has no headless analogue).
            swipe_.open(maui::core::open_swipe_item::right_items);
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment + the tests.
        [[nodiscard]] maui::controls::grid& outer()
        {
            return outer_;
        }
        [[nodiscard]] maui::controls::picker& mode_picker()
        {
            return mode_picker_;
        }
        [[nodiscard]] maui::controls::swipe_view& swipe()
        {
            return swipe_;
        }
        // The current mode the picker selection mapped onto the four collections (for the tests).
        [[nodiscard]] maui::core::swipe_mode current_mode() const
        {
            return current_mode_;
        }

    private:
        // C# OnModePickerSelectedIndexChanged: set all four collections' Mode to Reveal (index 0) or
        // Execute (any other index — the XAML only offers index 1 = Execute).
        void apply_mode()
        {
            current_mode_ =
                mode_picker_.selected_index() == 0 ? maui::core::swipe_mode::reveal : maui::core::swipe_mode::execute;
            swipe_.left_items_collection().set_mode(current_mode_);
            swipe_.top_items_collection().set_mode(current_mode_);
            swipe_.right_items_collection().set_mode(current_mode_);
            swipe_.bottom_items_collection().set_mode(current_mode_);
        }

        static std::shared_ptr<maui::graphics::solid_paint> white()
        {
            return std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::white);
        }

        maui::controls::content_page page_;
        maui::controls::grid outer_;
        maui::controls::picker mode_picker_;
        maui::controls::swipe_view swipe_;
        maui::controls::grid content_;
        maui::controls::label content_label_;

        // The eight directional swipe items (two per side), owned (the collections are non-owning).
        maui::controls::swipe_item left_1_;
        maui::controls::swipe_item left_2_;
        maui::controls::swipe_item top_1_;
        maui::controls::swipe_item top_2_;
        maui::controls::swipe_item right_1_;
        maui::controls::swipe_item right_2_;
        maui::controls::swipe_item bottom_1_;
        maui::controls::swipe_item bottom_2_;

        maui::core::swipe_mode current_mode_ = maui::core::swipe_mode::reveal;
    };
} // namespace maui::samples
