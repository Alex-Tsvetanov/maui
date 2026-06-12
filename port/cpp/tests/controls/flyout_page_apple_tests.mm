// Apple (AppKit) backend tests for the flyout_page seam. The host is a real NSSplitViewController: a
// SIDEBAR NSSplitViewItem hosting the flyout pane's native NSView (through a wrapper
// NSViewController) and a content item hosting the detail pane. IsPresented drives the sidebar item's
// `collapsed`; the computed Locked behavior (split mode) pins it (canCollapse = NO). The
// virtual→native direction only — a user drag-collapse is not observed back (documented deviation).
// The flyout tests install the essentials mocks exactly like the headless suite (the IsPresented
// default reads device_info; the real macOS host would otherwise flip it true). Compiled as
// Objective-C++ with ARC for the `apple` backend.
#import <AppKit/AppKit.h>

#include <memory>
#include <string>
#include <utility>

#include "maui/controls/content_page.hpp"
#include "maui/controls/flyout_layout_behavior.hpp"
#include "maui/controls/flyout_page.hpp"
#include "maui/core/content_page_handler.hpp"
#include "maui/core/flyout_behavior.hpp"
#include "maui/core/flyout_page_handler.hpp"
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

    NSSplitViewController* native_controller(const std::shared_ptr<flyout_page_handler>& handler)
    {
        return (__bridge NSSplitViewController*)handler->typed_platform_view()->controller;
    }

    NSView* attach_page(content_page& page)
    {
        auto page_handler = std::make_shared<content_page_handler>();
        page.set_handler(page_handler);
        return (__bridge NSView*)page_handler->native_view();
    }

    class apple_flyout_page_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
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

    TEST_F(apple_flyout_page_seam, controller_hosts_flyout_as_sidebar_and_detail_as_content)
    {
        content_page flyout;
        flyout.set_title("Menu");
        content_page detail;
        NSView* const flyout_native = attach_page(flyout);
        NSView* const detail_native = attach_page(detail);

        flyout_page page;
        page.set_flyout(&flyout);
        page.set_detail(&detail);

        auto handler = std::make_shared<flyout_page_handler>();
        page.set_handler(handler);

        NSSplitViewController* const split = native_controller(handler);
        ASSERT_NE(split, nil);
        ASSERT_EQ(split.splitViewItems.count, 2U);
        // Controller hierarchy: the sidebar item's wrapper VC's view IS the flyout's native NSView; the
        // content item's the detail's.
        EXPECT_EQ(split.splitViewItems[0].behavior, NSSplitViewItemBehaviorSidebar);
        EXPECT_EQ(split.splitViewItems[0].viewController.view, flyout_native);
        EXPECT_EQ(split.splitViewItems[1].viewController.view, detail_native);
        EXPECT_EQ((__bridge NSView*)handler->typed_platform_view()->native, split.view);
        // Not presented (mocked non-macOS default) -> the sidebar starts collapsed.
        EXPECT_TRUE(split.splitViewItems[0].collapsed);
    }

    TEST_F(apple_flyout_page_seam, is_presented_drives_the_sidebar_collapse)
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
        NSSplitViewController* const split = native_controller(handler);

        page.set_is_presented(true);
        EXPECT_FALSE(split.splitViewItems[0].collapsed);
        EXPECT_TRUE(handler->typed_platform_view()->presented);

        page.set_is_presented(false);
        EXPECT_TRUE(split.splitViewItems[0].collapsed);
        EXPECT_FALSE(handler->typed_platform_view()->presented);
    }

    TEST_F(apple_flyout_page_seam, locked_split_mode_pins_the_sidebar_open)
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
        NSSplitViewController* const split = native_controller(handler);

        page.set_layout_behavior(flyout_layout_behavior::split); // tablet + Split => Locked

        EXPECT_EQ(handler->typed_platform_view()->behavior, maui::core::flyout_behavior::locked);
        EXPECT_FALSE(split.splitViewItems[0].collapsed); // pinned open
        EXPECT_FALSE(split.splitViewItems[0].canCollapse);
        EXPECT_TRUE(page.is_presented());
    }
} // namespace
