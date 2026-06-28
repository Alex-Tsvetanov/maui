// Smoke test for the maui::ui facade + ui::app base (PUBLIC_API_DESIGN.md, phase P1).
//
// Verifies: (1) a ui::app subclass that overrides the CONCRETE main_window() (never naming i_window) boots
// through the unchanged hosting path; (2) the sealed create_window() forwards to main_window(); (3) mount +
// layout settle and attach handlers headlessly; (4) the ui:: facade aliases resolve to the real maui::*
// entities (zero-cost using-aliases).

#include "maui/ui.hpp"

#include "maui/controls/label.hpp"
#include "maui/graphics/size.hpp"
#include "maui/hosting/app_host.hpp"
#include "maui/hosting/maui_app.hpp"
#include "maui/hosting/maui_app_builder.hpp"

#include <type_traits>

#include <gtest/gtest.h>

namespace
{
    namespace ui = maui::ui;

    // A minimal consumer app on the new base: overrides the concrete main_window(), not create_window().
    class smoke_app final : public ui::app
    {
    public:
        smoke_app()
        {
            label_.set_text("Hello, maui::ui!");
            page_.set_content(label_);
            window_.set_content(page_);
            window_.set_title("ui smoke");
        }

        maui::controls::window& main_window() override
        {
            return window_;
        }

        maui::controls::content_page& page()
        {
            return page_;
        }
        maui::controls::label& text_label()
        {
            return label_;
        }

    private:
        maui::controls::window window_;
        ui::content_page page_; // facade alias for maui::controls::content_page
        maui::controls::label label_;
    };

    // The facade aliases must name the SAME entities (proves they are zero-cost using-aliases, not new types).
    static_assert(std::is_same_v<ui::content_page, maui::controls::content_page>);
    static_assert(std::is_same_v<ui::window, maui::controls::window>);
    static_assert(std::is_same_v<ui::application, maui::controls::application>);
    static_assert(std::is_same_v<ui::vertical_stack_layout, maui::controls::vertical_stack_layout>);
    static_assert(std::is_same_v<ui::color, maui::graphics::color>);
    static_assert(std::is_same_v<ui::thickness, maui::core::thickness>);

    TEST(ui_app, boots_through_hosting_and_hides_i_window)
    {
        auto app = maui::hosting::maui_app::create_builder().use_maui_app<smoke_app>().build();
        auto* a = app->application_as<smoke_app>().get();
        ASSERT_NE(a, nullptr);

        // create_window() is sealed on ui::app and forwards to the concrete main_window().
        EXPECT_EQ(a->create_window(), &a->main_window());

        maui::hosting::mount_window(*app, a->main_window());
        const maui::graphics::size arranged = maui::hosting::drive_layout(a->main_window(), 402.0, 874.0);

        EXPECT_GT(arranged.width, 0.0);
        EXPECT_GT(arranged.height, 0.0);
        EXPECT_NE(a->page().handler(), nullptr);
        EXPECT_NE(a->text_label().handler(), nullptr);
    }

    // The DEFAULT hosting shape: set_content(...) + set_title(...) — ui::app owns the window AND the content
    // root in the correct teardown order, so the consumer declares no window/root members at all.
    class hosted_app final : public ui::app
    {
    public:
        hosted_app()
        {
            set_content(ui::page(ui::vstack(ui::label("hosted")).spacing(8)));
            set_title("hosted");
        }
    };

    TEST(ui_app, set_content_hosts_through_the_owned_window)
    {
        auto app = maui::hosting::maui_app::create_builder().use_maui_app<hosted_app>().build();
        auto* a = app->application_as<hosted_app>().get();
        ASSERT_NE(a, nullptr);
        EXPECT_EQ(a->create_window(), &a->main_window());

        maui::hosting::mount_window(*app, a->main_window());
        const maui::graphics::size arranged = maui::hosting::drive_layout(a->main_window(), 402.0, 874.0);
        EXPECT_GT(arranged.width, 0.0);
        EXPECT_GT(arranged.height, 0.0);
        // Teardown order (window before content root) is owned by ui::app — verified clean under ASan.
    }
} // namespace
