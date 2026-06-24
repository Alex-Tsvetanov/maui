#pragma once
// maui::samples::ios_swipe_transition_page — ports iOSSwipeViewTransitionModePage.xaml (+ .xaml.cs)
//
// The C# iOSSwipeViewTransitionModePage is a StackLayout with: a horizontal row holding a
// "SwipeTransitionMode:" Label + an EnumPicker over the SwipeTransitionMode enum (Reveal / Drag,
// SelectedIndex="1" → Drag), and a SwipeView (x:Name="swipeView") carrying ios:SwipeView.SwipeTransitionMode
// ="Drag" with a single LeftItems "Delete" SwipeItem (calculator.png, LightPink) over a gray "Swipe right"
// Grid. OnSwipeViewTransitionModeChanged sets swipeView.On<iOS>().SetSwipeTransitionMode(picked);
// OnDeleteSwipeItemInvoked shows DisplayAlert("SwipeView", "Delete invoked.", "OK").
//
// THE subject of the page is the iOSSpecific SwipeView.SwipeTransitionMode knob (Reveal vs Drag). In the
// port that knob is realized directly through swipe_view::set_transition_mode / transition_mode (the
// cross-platform SwipeTransitionMode the iOSSpecific attached property writes through — there is no
// separate ios_specific/swipe_view config header in the port; the C# On<iOS>().SetSwipeTransitionMode
// maps onto the same SwipeView.SwipeTransitionMode bindable). This page wires exactly that, HEADLESS-SAFE:
//   - the SwipeView is built with SwipeTransitionMode = Drag (the XAML default + SelectedIndex="1");
//   - two buttons (Reveal / Drag) stand in for the EnumPicker — each sets the transition mode and refreshes
//     a readout, reproducing OnSwipeViewTransitionModeChanged across both enum values;
//   - the LeftItems "Delete" SwipeItem's Invoked drives the readout to "Delete invoked." (the C#
//     OnDeleteSwipeItemInvoked shows a DisplayAlert; the port has no modal alert at this layer, so the
//     observable effect is the readout — the gallery convention used by basic_swipe_page);
//   - on mount, the SwipeView is synthetically opened toward its populated LeftItems side so a static
//     capture shows the revealed item (the swipe-to-reveal gesture has no headless analogue; open() routes
//     through the now-attached handler — see basic_swipe_page / swipe_view_seam tests).
//
// PORT NOTES (faithful best-effort, never invented):
//   note: the EnumPicker over {x:Type SwipeTransitionMode} is a custom sample control + a Picker-of-enum
//         binding with no headless analogue, so it is ported as two explicit Reveal / Drag buttons. They
//         exercise the SAME setter (set_transition_mode) across BOTH enum values the picker would offer.
//   note: the Delete SwipeItem's IconImageSource="calculator.png" is carried as a file_image_source exactly
//         as the XAML names it; the headless backend resolves no bitmap, so it is an inert reference (the
//         item still shows its "Delete" text), matching the inert-source contract.
//   note: the SwipeView's BackgroundColor / the selector row's HorizontalOptions="Center" are NOT ported
//         (a separate layout-option deferral). The outer StackLayout Margin="20" IS ported (set_margin);
//         the Grid HeightRequest=60/WidthRequest=300 + the swipe item BackgroundColor LightPink ARE set
//         where the XAML names them.
//
// Self-contained: the page OWNS its whole element tree, exposes page().

#include <memory>
#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/file_image_source.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/swipe_item.hpp"
#include "maui/controls/swipe_view.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/open_swipe_item.hpp"
#include "maui/core/swipe_transition_mode.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

namespace maui::samples
{
    class ios_swipe_transition_page
    {
    public:
        ios_swipe_transition_page()
        {
            page_.set_title("SwipeView SwipeTransitionMode");
            stack_.set_margin(maui::core::thickness(20)); // XAML StackLayout Margin="20"
            stack_.set_spacing(12);

            // ---- the "SwipeTransitionMode:" row + the Reveal/Drag selector (stands in for the EnumPicker) --
            mode_caption_.set_text("SwipeTransitionMode:");
            reveal_button_.set_text("Reveal");
            reveal_button_.clicked.connect([this] { set_mode(maui::core::swipe_transition_mode::reveal); });
            drag_button_.set_text("Drag");
            drag_button_.clicked.connect([this] { set_mode(maui::core::swipe_transition_mode::drag); });
            selector_.set_spacing(8);
            selector_.add(mode_caption_);
            selector_.add(reveal_button_);
            selector_.add(drag_button_);

            // ---- the SwipeView: LeftItems Delete (calculator.png, LightPink) over a gray "Swipe right" Grid -
            content_.set_height_request(60);
            content_.set_width_request(300);
            content_.set_background(gray());
            content_label_.set_text("Swipe right");
            content_.add(content_label_);

            delete_item_.set_text("Delete");
            delete_item_.set_icon_image_source(std::make_shared<maui::controls::file_image_source>("calculator.png"));
            delete_item_.set_background_color(maui::graphics::colors::light_pink);
            // C# OnDeleteSwipeItemInvoked: DisplayAlert(...); → readout (the gallery convention).
            delete_item_.invoked.connect([this] { on_delete_invoked(); });

            swipe_.left_items_collection().add(delete_item_);
            // ios:SwipeView.SwipeTransitionMode="Drag" — the XAML default + the EnumPicker SelectedIndex="1".
            // Realized through the cross-platform SwipeTransitionMode the iOSSpecific knob writes through.
            swipe_.set_transition_mode(maui::core::swipe_transition_mode::drag);
            swipe_.set_height_request(60);
            swipe_.set_width_request(300);
            swipe_.set_content(content_);

            readout_.set_text("Swipe right to reveal Delete; pick a SwipeTransitionMode above");
            refresh_status();

            stack_.add(selector_);
            stack_.add(swipe_);
            stack_.add(readout_);
            stack_.add(status_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // POST-MOUNT hook (gallery_host.hpp gallery_post_mount): run AFTER the generic mount attaches every
        // handler + builds the native tree. Synthetically open the SwipeView toward its populated LeftItems
        // side so a static capture shows the revealed item. All per-control attach + re-host plumbing is now
        // the generic mount's job.
        void on_mounted(maui::hosting::maui_app& /*app*/)
        {
            // Static-capture seam: reveal the populated LeftItems (the swipe-to-reveal gesture has no
            // headless analogue). open() routes through the now-attached handler (swipe_view_seam tests).
            swipe_.open(maui::core::open_swipe_item::left_items);
        }

        // ---- owned controls, exposed for the hosting main / tests ----
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::swipe_view& swipe()
        {
            return swipe_;
        }
        [[nodiscard]] maui::controls::swipe_item& delete_item()
        {
            return delete_item_;
        }
        [[nodiscard]] maui::controls::button& reveal_button()
        {
            return reveal_button_;
        }
        [[nodiscard]] maui::controls::button& drag_button()
        {
            return drag_button_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }
        [[nodiscard]] maui::controls::label& status()
        {
            return status_;
        }
        [[nodiscard]] maui::core::swipe_transition_mode transition_mode() const
        {
            return swipe_.transition_mode();
        }
        [[nodiscard]] int invoked_count() const
        {
            return invoked_count_;
        }

    private:
        // C# OnSwipeViewTransitionModeChanged: swipeView.On<iOS>().SetSwipeTransitionMode(picked).
        void set_mode(maui::core::swipe_transition_mode mode)
        {
            swipe_.set_transition_mode(mode);
            refresh_status();
        }

        // C# OnDeleteSwipeItemInvoked: DisplayAlert("SwipeView", "Delete invoked.", "OK"); → readout.
        void on_delete_invoked()
        {
            ++invoked_count_;
            readout_.set_text("Delete invoked.");
        }

        void refresh_status()
        {
            const bool drag = swipe_.transition_mode() == maui::core::swipe_transition_mode::drag;
            status_.set_text(std::string("SwipeTransitionMode: ") + (drag ? "Drag" : "Reveal"));
        }

        static std::shared_ptr<maui::graphics::solid_paint> gray()
        {
            return std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::light_gray);
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;

        // the SwipeTransitionMode selector row (stands in for the EnumPicker)
        maui::controls::horizontal_stack_layout selector_;
        maui::controls::label mode_caption_;
        maui::controls::button reveal_button_;
        maui::controls::button drag_button_;

        // the SwipeView + its LeftItems Delete over a gray content Grid
        maui::controls::swipe_view swipe_;
        maui::controls::grid content_;
        maui::controls::label content_label_;
        maui::controls::swipe_item delete_item_;

        maui::controls::label readout_;
        maui::controls::label status_;

        int invoked_count_ = 0;
    };
} // namespace maui::samples
