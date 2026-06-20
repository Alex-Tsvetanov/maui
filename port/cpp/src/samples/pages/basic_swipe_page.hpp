#pragma once
// maui::samples::basic_swipe_page — ports BasicSwipeGallery.xaml
//
// A code-first port of the MAUI SwipeView sub-gallery
// Pages/Controls/SwipeViewGalleries/BasicSwipeGallery.xaml: a vertical StackLayout of five SwipeViews,
// each demonstrating a different revealed-side / SwipeMode combination over a gray labeled Grid:
//   1. BottomItems, Mode=Execute   — one red "Delete" item (coffee.png), Invoked → readout;
//   2. TopItems,    Mode=Reveal    — green "View", orange "Test (Ajg)" (ic_flag.png), red "Delete"
//                                    (coffee.png, Invoked → readout);
//   3. LeftItems,   Mode=Reveal    — green "Test (Ajg)", orange "Test (Ajg)" (calculator.png), red
//                                    "Delete" (coffee.png, Invoked → readout);
//   4. RightItems,  Mode=Execute   — one red "Delete" item (coffee.png), Invoked → readout;
//   5. all four directions, each Mode=Reveal with one red Invoked item — Left/Top/Right/Bottom.
//
// The page OWNS its whole element tree (the swipe_refresh_page / value_controls_page pattern). It is
// backend-agnostic — a sample main attaches handlers bottom-up via the hosting layer and hosts page() in
// a window; the headless/apple/ios test trees exercise the same control wiring directly.
//
// Interactions demonstrated:
//   - every "Delete" SwipeItem's Invoked → a shared readout label shows "Delete Invoked" (the C#
//     code-behind OnInvoked, which in MAUI shows a DisplayAlert("SwipeView", "Delete Invoked", "OK"); the
//     port has no modal alert at this layer, so the observable effect is the readout, the gallery
//     convention used by swipe_refresh_page).
//   - in attach_handlers, the first SwipeView is synthetically opened toward its populated BottomItems
//     side so a static capture shows the revealed item (the swipe-to-reveal gesture has no headless
//     analogue; open() routes through the now-attached handler — see swipe_view_seam tests).
//
// PORT NOTES (faithful best-effort, never invented):
//   note: the icon images (coffee.png / ic_flag.png / calculator.png) are carried as file_image_source
//         exactly as the XAML IconImageSource names them; the headless backend resolves no bitmap, so they
//         are inert references (the swipe item still shows its Text), matching the inert-source contract.
//   note: each SwipeView's Margin="12" is ported via set_margin (the View.Margin seam). The
//         HorizontalOptions/VerticalOptions="Center" are left at the stack's default fill (a minor visual
//         deferral). The vertical stack still lays the five views out top-to-bottom as the XAML StackLayout does.
//   note: SwipeItems Mode="Execute"/"Reveal" maps to swipe_items::set_mode(swipe_mode::execute/reveal);
//         the readable Grid HeightRequest/WidthRequest are set where the XAML names them.

#include <memory>

#include "maui/controls/content_page.hpp"
#include "maui/controls/file_image_source.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/swipe_item.hpp"
#include "maui/controls/swipe_view.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/open_swipe_item.hpp"
#include "maui/core/swipe_mode.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class basic_swipe_page
    {
    public:
        basic_swipe_page()
        {
            page_.set_title("SwipeView");
            stack_.set_spacing(12);

            readout_.set_text("Swipe a row, then invoke Delete");

            // ---- 1. BottomItems, Execute: one red Delete (coffee.png), Invoked → readout ----
            bottom_content_.set_height_request(60);
            bottom_content_.set_width_request(300);
            bottom_content_.set_background(gray());
            bottom_label_.set_text("Swipe Up (Execute)");
            bottom_content_.add(bottom_label_);

            bottom_delete_.set_text("Delete");
            bottom_delete_.set_icon_image_source(icon("coffee.png"));
            bottom_delete_.set_background_color(maui::graphics::colors::red);
            bottom_delete_.invoked.connect([this] { on_invoked(); });
            bottom_swipe_.bottom_items_collection().set_mode(maui::core::swipe_mode::execute);
            bottom_swipe_.bottom_items_collection().add(bottom_delete_);
            bottom_swipe_.set_content(bottom_content_);
            bottom_swipe_.set_margin(maui::core::thickness(12)); // XAML SwipeView Margin="12"
            stack_.add(bottom_swipe_);

            // ---- 2. TopItems, Reveal: View / Test (Ajg) (ic_flag.png) / Delete (coffee.png, Invoked) ----
            top_content_.set_height_request(60);
            top_content_.set_width_request(300);
            top_content_.set_background(gray());
            top_label_.set_text("Swipe Down (Reveal)");
            top_content_.add(top_label_);

            top_view_.set_text("View");
            top_view_.set_background_color(maui::graphics::colors::green);
            top_test_.set_text("Test (Ajg)");
            top_test_.set_icon_image_source(icon("ic_flag.png"));
            top_test_.set_background_color(maui::graphics::colors::orange);
            top_delete_.set_text("Delete");
            top_delete_.set_icon_image_source(icon("coffee.png"));
            top_delete_.set_background_color(maui::graphics::colors::red);
            top_delete_.invoked.connect([this] { on_invoked(); });
            top_swipe_.top_items_collection().set_mode(maui::core::swipe_mode::reveal);
            top_swipe_.top_items_collection().add(top_view_);
            top_swipe_.top_items_collection().add(top_test_);
            top_swipe_.top_items_collection().add(top_delete_);
            top_swipe_.set_height_request(60);
            top_swipe_.set_width_request(300);
            top_swipe_.set_content(top_content_);
            top_swipe_.set_margin(maui::core::thickness(12)); // XAML SwipeView Margin="12"
            stack_.add(top_swipe_);

            // ---- 3. LeftItems, Reveal: Test (Ajg) / Test (Ajg) (calculator.png) / Delete (coffee, Invoked) -
            left_content_.set_height_request(60);
            left_content_.set_width_request(300);
            left_content_.set_background(gray());
            left_label_.set_text("Swipe Right (Reveal)");
            left_content_.add(left_label_);

            left_view_.set_text("Test (Ajg)");
            left_view_.set_background_color(maui::graphics::colors::green);
            left_test_.set_text("Test (Ajg)");
            left_test_.set_icon_image_source(icon("calculator.png"));
            left_test_.set_background_color(maui::graphics::colors::orange);
            left_delete_.set_text("Delete");
            left_delete_.set_icon_image_source(icon("coffee.png"));
            left_delete_.set_background_color(maui::graphics::colors::red);
            left_delete_.invoked.connect([this] { on_invoked(); });
            left_swipe_.left_items_collection().set_mode(maui::core::swipe_mode::reveal);
            left_swipe_.left_items_collection().add(left_view_);
            left_swipe_.left_items_collection().add(left_test_);
            left_swipe_.left_items_collection().add(left_delete_);
            left_swipe_.set_height_request(60);
            left_swipe_.set_width_request(300);
            left_swipe_.set_content(left_content_);
            left_swipe_.set_margin(maui::core::thickness(12)); // XAML SwipeView Margin="12"
            stack_.add(left_swipe_);

            // ---- 4. RightItems, Execute: one red Delete (coffee.png, Invoked) ----
            right_content_.set_height_request(60);
            right_content_.set_width_request(300);
            right_content_.set_background(gray());
            right_label_.set_text("Swipe Left (Execute)");
            right_content_.add(right_label_);

            right_delete_.set_text("Delete");
            right_delete_.set_icon_image_source(icon("coffee.png"));
            right_delete_.set_background_color(maui::graphics::colors::red);
            right_delete_.invoked.connect([this] { on_invoked(); });
            right_swipe_.right_items_collection().set_mode(maui::core::swipe_mode::execute);
            right_swipe_.right_items_collection().add(right_delete_);
            right_swipe_.set_height_request(60);
            right_swipe_.set_width_request(300);
            right_swipe_.set_content(right_content_);
            right_swipe_.set_margin(maui::core::thickness(12)); // XAML SwipeView Margin="12"
            stack_.add(right_swipe_);

            // ---- 5. all four directions, each Reveal with one red Invoked item ----
            any_content_.set_height_request(60);
            any_content_.set_width_request(300);
            any_content_.set_background(gray());
            any_label_.set_text("Swipe in any direction");
            any_content_.add(any_label_);

            any_left_.set_text("LeftItem");
            any_left_.set_icon_image_source(icon("coffee.png"));
            any_left_.set_background_color(maui::graphics::colors::red);
            any_left_.invoked.connect([this] { on_invoked(); });
            any_top_.set_text("TopItem");
            any_top_.set_icon_image_source(icon("ic_flag.png"));
            any_top_.set_background_color(maui::graphics::colors::red);
            any_top_.invoked.connect([this] { on_invoked(); });
            any_right_.set_text("RightItem");
            any_right_.set_icon_image_source(icon("coffee.png"));
            any_right_.set_background_color(maui::graphics::colors::red);
            any_right_.invoked.connect([this] { on_invoked(); });
            any_bottom_.set_text("BottomItem");
            any_bottom_.set_icon_image_source(icon("coffee.png"));
            any_bottom_.set_background_color(maui::graphics::colors::red);
            any_bottom_.invoked.connect([this] { on_invoked(); });

            any_swipe_.left_items_collection().set_mode(maui::core::swipe_mode::reveal);
            any_swipe_.left_items_collection().add(any_left_);
            any_swipe_.top_items_collection().set_mode(maui::core::swipe_mode::reveal);
            any_swipe_.top_items_collection().add(any_top_);
            any_swipe_.right_items_collection().set_mode(maui::core::swipe_mode::reveal);
            any_swipe_.right_items_collection().add(any_right_);
            any_swipe_.bottom_items_collection().set_mode(maui::core::swipe_mode::reveal);
            any_swipe_.bottom_items_collection().add(any_bottom_);
            any_swipe_.set_height_request(60);
            any_swipe_.set_width_request(300);
            any_swipe_.set_content(any_content_);
            any_swipe_.set_margin(maui::core::thickness(12)); // XAML SwipeView Margin="12"
            stack_.add(any_swipe_);

            stack_.add(readout_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (each swipe's content Grid + its label, then the
        // swipe_view, then the stack, then the page), then re-host the tree built in the ctor. The
        // swipe_item members are NON-view items (no standalone handler) so they are deliberately excluded —
        // attaching one would throw. After re-hosting, synthetically open the first swipe_view toward its
        // populated BottomItems side so a static capture shows the revealed item. (gallery_attach.hpp)
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, bottom_label_, "bottom_label_");
            gallery_attach_one(app, bottom_content_, "bottom_content_");
            gallery_attach_one(app, bottom_swipe_, "bottom_swipe_");
            gallery_attach_one(app, top_label_, "top_label_");
            gallery_attach_one(app, top_content_, "top_content_");
            gallery_attach_one(app, top_swipe_, "top_swipe_");
            gallery_attach_one(app, left_label_, "left_label_");
            gallery_attach_one(app, left_content_, "left_content_");
            gallery_attach_one(app, left_swipe_, "left_swipe_");
            gallery_attach_one(app, right_label_, "right_label_");
            gallery_attach_one(app, right_content_, "right_content_");
            gallery_attach_one(app, right_swipe_, "right_swipe_");
            gallery_attach_one(app, any_label_, "any_label_");
            gallery_attach_one(app, any_content_, "any_content_");
            gallery_attach_one(app, any_swipe_, "any_swipe_");
            gallery_attach_one(app, readout_, "readout_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, page_, "page_");

            // The tree was built in the ctor before any handler existed, so replay the host commands now:
            // each content Grid hosts its label, each swipe_view hosts its content Grid, the stack hosts the
            // five swipe_views + readout, the page hosts the stack.
            gallery_rehost_layout(bottom_content_);
            gallery_rehost_layout(top_content_);
            gallery_rehost_layout(left_content_);
            gallery_rehost_layout(right_content_);
            gallery_rehost_layout(any_content_);
            gallery_rehost_content(bottom_swipe_);
            gallery_rehost_content(top_swipe_);
            gallery_rehost_content(left_swipe_);
            gallery_rehost_content(right_swipe_);
            gallery_rehost_content(any_swipe_);
            gallery_rehost_layout(stack_);
            gallery_rehost_content(page_);

            // Static-capture seam: reveal the first row's BottomItems (the swipe-to-reveal gesture has no
            // headless analogue). open() routes through the now-attached handler (swipe_view_seam tests).
            bottom_swipe_.open(maui::core::open_swipe_item::bottom_items);
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment + the tests.
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }
        [[nodiscard]] maui::controls::swipe_view& bottom_swipe()
        {
            return bottom_swipe_;
        }
        [[nodiscard]] maui::controls::swipe_view& top_swipe()
        {
            return top_swipe_;
        }
        [[nodiscard]] maui::controls::swipe_view& left_swipe()
        {
            return left_swipe_;
        }
        [[nodiscard]] maui::controls::swipe_view& right_swipe()
        {
            return right_swipe_;
        }
        [[nodiscard]] maui::controls::swipe_view& any_swipe()
        {
            return any_swipe_;
        }
        [[nodiscard]] maui::controls::swipe_item& bottom_delete()
        {
            return bottom_delete_;
        }
        [[nodiscard]] int invoked_count() const
        {
            return invoked_count_;
        }

    private:
        // C# BasicSwipeGallery.OnInvoked: shows DisplayAlert("SwipeView", "Delete Invoked", "OK"); with no
        // modal alert at this layer the observable effect is the readout + a counter (the gallery convention).
        void on_invoked()
        {
            ++invoked_count_;
            readout_.set_text("Delete Invoked");
        }

        static std::shared_ptr<maui::graphics::solid_paint> gray()
        {
            return std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::light_gray);
        }
        static std::shared_ptr<maui::controls::file_image_source> icon(const char* file)
        {
            return std::make_shared<maui::controls::file_image_source>(file);
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label readout_;

        // 1. BottomItems / Execute
        maui::controls::swipe_view bottom_swipe_;
        maui::controls::grid bottom_content_;
        maui::controls::label bottom_label_;
        maui::controls::swipe_item bottom_delete_;

        // 2. TopItems / Reveal
        maui::controls::swipe_view top_swipe_;
        maui::controls::grid top_content_;
        maui::controls::label top_label_;
        maui::controls::swipe_item top_view_;
        maui::controls::swipe_item top_test_;
        maui::controls::swipe_item top_delete_;

        // 3. LeftItems / Reveal
        maui::controls::swipe_view left_swipe_;
        maui::controls::grid left_content_;
        maui::controls::label left_label_;
        maui::controls::swipe_item left_view_;
        maui::controls::swipe_item left_test_;
        maui::controls::swipe_item left_delete_;

        // 4. RightItems / Execute
        maui::controls::swipe_view right_swipe_;
        maui::controls::grid right_content_;
        maui::controls::label right_label_;
        maui::controls::swipe_item right_delete_;

        // 5. all four directions / Reveal
        maui::controls::swipe_view any_swipe_;
        maui::controls::grid any_content_;
        maui::controls::label any_label_;
        maui::controls::swipe_item any_left_;
        maui::controls::swipe_item any_top_;
        maui::controls::swipe_item any_right_;
        maui::controls::swipe_item any_bottom_;

        int invoked_count_ = 0;
    };
} // namespace maui::samples
