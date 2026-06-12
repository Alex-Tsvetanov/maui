// Tests for the W1-10 demo page (src/samples/pages/tabbed_flyout_page.hpp): the flyout/tabbed wiring a
// sample main hosts — the flyout pane is the titled menu, the detail is the two-tab tabbed_page, the
// menu buttons drive the tab selection, and the toggle button flips IsPresented (mirrored into the
// status label). Backend-agnostic (no handlers attached); the essentials mocks pin the IsPresented
// default + the split-mode computation, exactly like the flyout_page suite.

#include "maui/core/event.hpp"
#include "maui/core/move_only_function.hpp"
#include "src/samples/pages/tabbed_flyout_page.hpp"

#include <memory>
#include <string>
#include <utility>

#include "maui/essentials/device_display.hpp"
#include "maui/essentials/device_info.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::devices::device_idiom;
    using maui::devices::device_info;
    using maui::devices::device_platform;
    using maui::devices::display_info;
    using maui::devices::display_orientation;
    using maui::samples::tabbed_flyout_page;

    class demo_mock_device_info final : public maui::devices::i_device_info
    {
    public:
        [[nodiscard]] std::string model() const override
        {
            return "MockModel";
        }
        [[nodiscard]] std::string manufacturer() const override
        {
            return "MockManufacturer";
        }
        [[nodiscard]] std::string name() const override
        {
            return "MockName";
        }
        [[nodiscard]] std::string version_string() const override
        {
            return "1.0";
        }
        [[nodiscard]] device_platform platform() const override
        {
            return device_platform::unknown();
        }
        [[nodiscard]] device_idiom idiom() const override
        {
            return device_idiom::unknown();
        }
        [[nodiscard]] enum maui::devices::device_type device_type() const override
        {
            return maui::devices::device_type::virtual_;
        }
    };

    class demo_mock_device_display final : public maui::devices::i_device_display
    {
    public:
        [[nodiscard]] bool keep_screen_on() const override
        {
            return false;
        }
        void set_keep_screen_on(bool /*value*/) override
        {
        }
        [[nodiscard]] display_info main_display_info() const override
        {
            return {};
        }
        maui::core::connection_token add_main_display_info_changed(
            maui::core::move_only_function<void(const display_info&)> handler) override
        {
            return changed_.connect(std::move(handler));
        }
        bool remove_main_display_info_changed(maui::core::connection_token token) override
        {
            return changed_.disconnect(token);
        }

    private:
        maui::core::event<display_info> changed_;
    };

    class tabbed_flyout_demo_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            device_info::set_current(std::make_shared<demo_mock_device_info>());
            maui::devices::device_display::set_current(std::make_shared<demo_mock_device_display>());
        }

        void TearDown() override
        {
            maui::devices::device_display::set_current(nullptr);
            device_info::set_current(nullptr);
        }
    };

    TEST_F(tabbed_flyout_demo_test, wires_the_flyout_over_the_two_tab_detail)
    {
        tabbed_flyout_page demo;

        EXPECT_EQ(demo.page().flyout(), &demo.menu_page());
        EXPECT_EQ(demo.page().detail(), &demo.tabs());
        EXPECT_EQ(demo.menu_page().title(), "Menu"); // the FlyoutPage Title requirement
        ASSERT_EQ(demo.tabs().children().size(), 2U);
        EXPECT_EQ(demo.tabs().children()[0]->title(), "Home");
        EXPECT_EQ(demo.tabs().children()[1]->title(), "Settings");
        EXPECT_EQ(demo.tabs().current_page(), &demo.first_tab());
    }

    TEST_F(tabbed_flyout_demo_test, menu_buttons_select_the_tabs)
    {
        tabbed_flyout_page demo;

        demo.settings_button().send_clicked();
        EXPECT_EQ(demo.tabs().current_page(), &demo.second_tab());

        demo.home_button().send_clicked();
        EXPECT_EQ(demo.tabs().current_page(), &demo.first_tab());
    }

    TEST_F(tabbed_flyout_demo_test, toggle_button_flips_is_presented_into_the_status_label)
    {
        tabbed_flyout_page demo;
        EXPECT_FALSE(demo.page().is_presented()); // mocked non-macOS default
        EXPECT_EQ(demo.status().text(), "Flyout dismissed");

        demo.toggle_button().send_clicked();
        EXPECT_TRUE(demo.page().is_presented());
        EXPECT_EQ(demo.status().text(), "Flyout presented");

        demo.toggle_button().send_clicked();
        EXPECT_FALSE(demo.page().is_presented());
        EXPECT_EQ(demo.status().text(), "Flyout dismissed");
    }
} // namespace
