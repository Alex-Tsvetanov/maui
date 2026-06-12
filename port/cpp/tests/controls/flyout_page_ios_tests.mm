// iOS (UIKit) backend tests for the flyout_page seam, run ON the simulator. The host is a real
// double-column UISplitViewController: the PRIMARY column's child UIViewController hosts the flyout
// pane's native UIView and the SECONDARY the detail's (the child-VC composition the W1-10 task
// asserts); IsPresented drives preferredDisplayMode (oneBesideSecondary vs secondaryOnly), the
// computed Locked behavior tiles the flyout permanently, and IsGestureEnabled drives
// presentsWithGesture. The native→virtual presented sync is exercised through the i_flyout_view
// set_flyout_is_presented seam directly (the displayModeButton callbacks need a live UIWindow — the
// documented deviation). The essentials mocks are installed like the headless suite (IsPresented's
// default reads device_info). Compiled as Objective-C++ with ARC for the `ios` backend.
#import <UIKit/UIKit.h>

#include <memory>
#include <string>
#include <utility>

#include "maui/controls/content_page.hpp"
#include "maui/controls/flyout_layout_behavior.hpp"
#include "maui/controls/flyout_page.hpp"
#include "maui/core/content_page_handler.hpp"
#include "maui/core/flyout_behavior.hpp"
#include "maui/core/flyout_page_handler.hpp"
#include "maui/core/i_flyout_view.hpp"
#include "maui/essentials/device_display.hpp"
#include "maui/essentials/device_info.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::content_page;
    using maui::controls::flyout_layout_behavior;
    using maui::controls::flyout_page;
    using maui::core::content_page_handler;
    using maui::core::flyout_page_handler;
    using maui::devices::device_idiom;
    using maui::devices::device_info;
    using maui::devices::device_platform;
    using maui::devices::display_info;
    using maui::devices::display_orientation;

    // The C# MockDeviceInfo/MockDeviceDisplay pair (the headless flyout suite's fakes, abbreviated).
    class mock_device_info final : public maui::devices::i_device_info
    {
    public:
        explicit mock_device_info(device_idiom idiom) : idiom_(std::move(idiom))
        {
        }
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
            return idiom_;
        }
        [[nodiscard]] enum maui::devices::device_type device_type() const override
        {
            return maui::devices::device_type::virtual_;
        }

    private:
        device_idiom idiom_;
    };

    class mock_device_display final : public maui::devices::i_device_display
    {
    public:
        explicit mock_device_display(display_orientation orientation)
        {
            info_.orientation = orientation;
        }
        [[nodiscard]] bool keep_screen_on() const override
        {
            return false;
        }
        void set_keep_screen_on(bool /*value*/) override
        {
        }
        [[nodiscard]] display_info main_display_info() const override
        {
            return info_;
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
        display_info info_;
        maui::core::event<display_info> changed_;
    };

    UISplitViewController* native_controller(const std::shared_ptr<flyout_page_handler>& handler)
    {
        return (__bridge UISplitViewController*)handler->typed_platform_view()->controller;
    }

    UIView* attach_page(content_page& page)
    {
        auto page_handler = std::make_shared<content_page_handler>();
        page.set_handler(page_handler);
        return (__bridge UIView*)page_handler->native_view();
    }

    class ios_flyout_page_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            device_info::set_current(std::make_shared<mock_device_info>(device_idiom::unknown()));
            maui::devices::device_display::set_current(
                std::make_shared<mock_device_display>(display_orientation::unknown));
        }

        void TearDown() override
        {
            maui::devices::device_display::set_current(nullptr);
            device_info::set_current(nullptr);
        }
    };

    TEST_F(ios_flyout_page_seam, controller_hosts_flyout_primary_and_detail_secondary)
    {
        content_page flyout;
        flyout.set_title("Menu");
        content_page detail;
        UIView* const flyout_native = attach_page(flyout);
        UIView* const detail_native = attach_page(detail);

        flyout_page page;
        page.set_flyout(&flyout);
        page.set_detail(&detail);

        auto handler = std::make_shared<flyout_page_handler>();
        page.set_handler(handler);

        UISplitViewController* const split = native_controller(handler);
        ASSERT_NE(split, nil);
        // Controller hierarchy: each column's child view controller's view IS the pane's native UIView.
        UIViewController* const primary = [split viewControllerForColumn:UISplitViewControllerColumnPrimary];
        UIViewController* const secondary = [split viewControllerForColumn:UISplitViewControllerColumnSecondary];
        ASSERT_NE(primary, nil);
        ASSERT_NE(secondary, nil);
        EXPECT_EQ(primary.view, flyout_native);
        EXPECT_EQ(secondary.view, detail_native);
        EXPECT_EQ((__bridge UIView*)handler->typed_platform_view()->native, split.view);
        // Not presented (mocked non-macOS default) -> the flyout column starts hidden.
        EXPECT_EQ(split.preferredDisplayMode, UISplitViewControllerDisplayModeSecondaryOnly);
    }

    TEST_F(ios_flyout_page_seam, is_presented_drives_the_preferred_display_mode)
    {
        content_page flyout;
        flyout.set_title("Menu");
        content_page detail;
        attach_page(flyout);
        attach_page(detail);

        flyout_page page;
        page.set_flyout(&flyout);
        page.set_detail(&detail);

        auto handler = std::make_shared<flyout_page_handler>();
        page.set_handler(handler);
        UISplitViewController* const split = native_controller(handler);

        page.set_is_presented(true);
        EXPECT_EQ(split.preferredDisplayMode, UISplitViewControllerDisplayModeOneBesideSecondary);
        EXPECT_TRUE(handler->typed_platform_view()->presented);

        page.set_is_presented(false);
        EXPECT_EQ(split.preferredDisplayMode, UISplitViewControllerDisplayModeSecondaryOnly);
        EXPECT_FALSE(handler->typed_platform_view()->presented);
    }

    TEST_F(ios_flyout_page_seam, native_show_hide_reports_through_the_flyout_view_seam)
    {
        content_page flyout;
        flyout.set_title("Menu");
        content_page detail;
        attach_page(flyout);
        attach_page(detail);

        flyout_page page;
        page.set_flyout(&flyout);
        page.set_detail(&detail);

        auto handler = std::make_shared<flyout_page_handler>();
        page.set_handler(handler);
        UISplitViewController* const split = native_controller(handler);

        // The chrome's show/hide callback lands in IFlyoutView.IsPresented (driven directly — the
        // documented deviation: no UIWindow in the bundle-less test process).
        auto& seam = static_cast<maui::core::i_flyout_view&>(page);
        seam.set_flyout_is_presented(true);

        EXPECT_TRUE(page.is_presented());
        EXPECT_EQ(split.preferredDisplayMode, UISplitViewControllerDisplayModeOneBesideSecondary);
    }

    TEST_F(ios_flyout_page_seam, locked_split_mode_tiles_the_flyout)
    {
        device_info::set_current(std::make_shared<mock_device_info>(device_idiom::tablet()));
        content_page flyout;
        flyout.set_title("Menu");
        content_page detail;
        attach_page(flyout);
        attach_page(detail);

        flyout_page page;
        page.set_flyout(&flyout);
        page.set_detail(&detail);

        auto handler = std::make_shared<flyout_page_handler>();
        page.set_handler(handler);
        UISplitViewController* const split = native_controller(handler);

        page.set_layout_behavior(flyout_layout_behavior::split); // tablet + Split => Locked

        EXPECT_EQ(handler->typed_platform_view()->behavior, maui::core::flyout_behavior::locked);
        EXPECT_EQ(split.preferredSplitBehavior, UISplitViewControllerSplitBehaviorTile);
        EXPECT_EQ(split.preferredDisplayMode, UISplitViewControllerDisplayModeOneBesideSecondary);
        EXPECT_TRUE(page.is_presented());
    }

    TEST_F(ios_flyout_page_seam, gesture_enabled_drives_presents_with_gesture)
    {
        content_page flyout;
        flyout.set_title("Menu");
        content_page detail;
        attach_page(flyout);
        attach_page(detail);

        flyout_page page;
        page.set_flyout(&flyout);
        page.set_detail(&detail);

        auto handler = std::make_shared<flyout_page_handler>();
        page.set_handler(handler);
        UISplitViewController* const split = native_controller(handler);
        EXPECT_TRUE(split.presentsWithGesture); // IsGestureEnabled default true

        page.set_is_gesture_enabled(false);
        EXPECT_FALSE(split.presentsWithGesture);
    }
} // namespace
