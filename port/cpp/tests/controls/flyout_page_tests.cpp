// Tests for flyout_page — ported from FlyoutPageUnitTests.cs: the pane setter guards (null / missing
// flyout Title / already-parented), the IsPresented surface (+ the split-mode hiding guard over the
// device idiom/orientation seams, mocked exactly like the C# fixture's MockDeviceInfo /
// MockDeviceDisplay), the pane lifecycle propagation (window-driven appearing/disappearing), the
// FlyoutPage-level back-button event branch, and the headless handler seam (the flyout_page_platform
// mirrors + the computed FlyoutBehavior).
//
// Deferred oracle cases (documented in flyout_page.hpp / STATUS.md): the per-page navigation events
// (NavigatingFrom/NavigatedFrom/To suites), the IFlyoutPageController DetailBounds/FlyoutBounds throws
// (the legacy renderer seam), and VerifyToolbarButtonVisibilityWhenFlyoutReset (needs the unported
// Toolbar element).

#include "maui/controls/flyout_page.hpp"

#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/flyout_layout_behavior.hpp"
#include "maui/controls/navigation_page.hpp"
#include "maui/controls/tabbed_page.hpp"
#include "maui/core/flyout_behavior.hpp"
#include "maui/core/flyout_page_handler.hpp"
#include "maui/essentials/device_display.hpp"
#include "maui/essentials/device_info.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::back_button_pressed_event_args;
    using maui::controls::content_page;
    using maui::controls::flyout_layout_behavior;
    using maui::controls::flyout_page;
    using maui::core::flyout_page_handler;
    using maui::devices::device_display;
    using maui::devices::device_idiom;
    using maui::devices::device_info;
    using maui::devices::device_platform;
    using maui::devices::display_info;
    using maui::devices::display_orientation;

    // ---- the C# test fakes (MockDeviceInfo / MockDeviceDisplay), behind the essentials seams ----

    class mock_device_display final : public maui::devices::i_device_display
    {
    public:
        explicit mock_device_display(display_info info) : info_(info)
        {
        }

        [[nodiscard]] bool keep_screen_on() const override
        {
            return keep_screen_on_;
        }
        void set_keep_screen_on(bool value) override
        {
            keep_screen_on_ = value;
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

        void set_main_display_orientation(display_orientation orientation)
        {
            info_.orientation = orientation;
            changed_.raise(info_);
        }

    private:
        display_info info_;
        maui::core::event<display_info> changed_;
        bool keep_screen_on_ = false;
    };

    class mock_device_info final : public maui::devices::i_device_info
    {
    public:
        mock_device_info(device_platform platform, device_idiom idiom)
            : platform_(std::move(platform)), idiom_(std::move(idiom))
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
            return platform_;
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
        device_platform platform_;
        device_idiom idiom_;
    };

    // The C# fixture installs both mocks in its constructor; reset after each test so nothing leaks.
    class flyout_page_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            use_idiom(device_idiom::unknown());
            use_display(display_orientation::unknown);
        }

        void TearDown() override
        {
            device_display::set_current(nullptr);
            device_info::set_current(nullptr);
        }

        static void use_idiom(const device_idiom& idiom)
        {
            device_info::set_current(std::make_shared<mock_device_info>(device_platform::unknown(), idiom));
        }

        static std::shared_ptr<mock_device_display> use_display(display_orientation orientation)
        {
            auto display = std::make_shared<mock_device_display>(display_info{.orientation = orientation});
            device_display::set_current(display);
            return display;
        }
    };

    // ---- FlyoutPageUnitTests.cs ----

    TEST_F(flyout_page_test, constructor_defaults)
    {
        flyout_page page;
        EXPECT_EQ(page.flyout(), nullptr);
        EXPECT_EQ(page.detail(), nullptr);
        EXPECT_FALSE(page.is_presented());
    }

    // The IsPresented defaultValueCreator quirk: presented by default ONLY on classic macOS.
    TEST_F(flyout_page_test, presented_by_default_on_classic_macos)
    {
        device_info::set_current(
            std::make_shared<mock_device_info>(device_platform::mac_os(), device_idiom::desktop()));
        flyout_page page;
        EXPECT_TRUE(page.is_presented());
    }

    TEST_F(flyout_page_test, flyout_setter)
    {
        flyout_page page;
        content_page child;
        child.set_title("Foo");
        page.set_flyout(&child);

        EXPECT_EQ(page.flyout(), &child);
        EXPECT_EQ(child.logical_parent(), &page); // InternalChildren.Add
    }

    TEST_F(flyout_page_test, flyout_set_null_throws)
    {
        flyout_page page;
        content_page child;
        child.set_title("Foo");
        page.set_flyout(&child);

        EXPECT_THROW(page.set_flyout(nullptr), std::invalid_argument);
    }

    TEST_F(flyout_page_test, flyout_changed_fires_property_changed)
    {
        flyout_page page;
        content_page child;
        child.set_title("Foo");

        bool changed = false;
        page.property_changed.connect([&changed](std::string_view name) {
            if (name == "flyout")
            {
                changed = true;
            }
        });

        page.set_flyout(&child);
        EXPECT_TRUE(changed);
    }

    TEST_F(flyout_page_test, detail_setter)
    {
        flyout_page page;
        content_page child;
        page.set_detail(&child);

        EXPECT_EQ(page.detail(), &child);
        EXPECT_EQ(child.logical_parent(), &page);
    }

    TEST_F(flyout_page_test, detail_set_null_throws)
    {
        flyout_page page;
        content_page child;
        page.set_detail(&child);

        EXPECT_THROW(page.set_detail(nullptr), std::invalid_argument);
    }

    TEST_F(flyout_page_test, detail_changed_fires_property_changed)
    {
        flyout_page page;
        content_page child;

        bool changed = false;
        page.property_changed.connect([&changed](std::string_view name) {
            if (name == "detail")
            {
                changed = true;
            }
        });

        page.set_detail(&child);
        EXPECT_TRUE(changed);
    }

    TEST_F(flyout_page_test, throws_when_flyout_set_without_valid_title)
    {
        flyout_page page;
        content_page untitled; // the C# [Theory] null/"" titles collapse to the one empty string
        EXPECT_THROW(page.set_flyout(&untitled), std::runtime_error);
    }

    TEST_F(flyout_page_test, throws_when_packed_without_setting)
    {
        flyout_page page;
        maui::controls::tabbed_page tabs;
        EXPECT_THROW(tabs.add(page), std::runtime_error); // OnParentSet: both panes must be set
    }

    TEST_F(flyout_page_test, does_not_throw_when_packed_with_setting)
    {
        content_page flyout;
        flyout.set_title("Foo");
        content_page detail;
        flyout_page page;
        page.set_flyout(&flyout);
        page.set_detail(&detail);

        maui::controls::tabbed_page tabs;
        EXPECT_NO_THROW(tabs.add(page));
        EXPECT_EQ(page.logical_parent(), &tabs);
    }

    TEST_F(flyout_page_test, flyout_visible)
    {
        flyout_page page;
        EXPECT_FALSE(page.is_presented());

        bool signaled = false;
        page.property_changed.connect([&signaled](std::string_view name) {
            if (name == "is_presented")
            {
                signaled = true;
            }
        });
        bool event_raised = false;
        page.is_presented_changed.connect([&event_raised] { event_raised = true; });

        page.set_is_presented(true);

        EXPECT_TRUE(page.is_presented());
        EXPECT_TRUE(signaled);
        EXPECT_TRUE(event_raised); // C# IsPresentedChanged
    }

    TEST_F(flyout_page_test, flyout_visible_double_set)
    {
        flyout_page page;

        bool signaled = false;
        page.property_changed.connect([&signaled](std::string_view name) {
            if (name == "is_presented")
            {
                signaled = true;
            }
        });

        page.set_is_presented(page.is_presented());

        EXPECT_FALSE(signaled);
    }

    TEST_F(flyout_page_test, throws_in_set_is_presented_on_split_mode_on_tablet)
    {
        use_idiom(device_idiom::tablet());
        flyout_page page;
        content_page flyout;
        flyout.set_title("Foo");
        content_page detail;
        page.set_flyout(&flyout);
        page.set_detail(&detail);
        page.set_layout_behavior(flyout_layout_behavior::split); // forces IsPresented = true (split mode)

        EXPECT_TRUE(page.is_presented());
        EXPECT_THROW(page.set_is_presented(false), std::runtime_error);
    }

    TEST_F(flyout_page_test, throws_in_set_is_presented_on_split_portrait_mode_on_tablet)
    {
        use_idiom(device_idiom::tablet());
        use_display(display_orientation::portrait);
        flyout_page page;
        content_page flyout;
        flyout.set_title("Foo");
        content_page detail;
        page.set_flyout(&flyout);
        page.set_detail(&detail);
        page.set_layout_behavior(flyout_layout_behavior::split_on_portrait);

        EXPECT_THROW(page.set_is_presented(false), std::runtime_error);
    }

    TEST_F(flyout_page_test, set_is_presented_on_popover_mode)
    {
        use_display(display_orientation::landscape);
        flyout_page page;
        content_page flyout;
        flyout.set_title("Foo");
        content_page detail;
        page.set_flyout(&flyout);
        page.set_detail(&detail);
        page.set_layout_behavior(flyout_layout_behavior::popover);

        page.set_is_presented(true);
        EXPECT_TRUE(page.is_presented());
    }

    // SendsBackEventToPresentedFlyoutFirst / EmitsCorrectlyWhenPresentedOnBackPressed — the portable
    // BackButtonPressed event branch (the per-page SendBackButtonPressed legs are deferred).
    TEST_F(flyout_page_test, back_button_event_can_dismiss_the_presented_flyout)
    {
        flyout_page page;
        content_page flyout;
        flyout.set_title("Flyout");
        content_page detail;
        page.set_flyout(&flyout);
        page.set_detail(&detail);
        page.set_is_presented(true);

        page.back_button_pressed.connect([&page](back_button_pressed_event_args& args) {
            args.handled = page.is_presented();
            page.set_is_presented(false);
        });

        EXPECT_TRUE(page.send_back_button_pressed());
        EXPECT_FALSE(page.is_presented());
        // A second press finds the flyout dismissed -> unhandled.
        EXPECT_FALSE(page.send_back_button_pressed());
    }

    TEST_F(flyout_page_test, throws_when_adding_already_parented_detail)
    {
        content_page detail;
        maui::controls::navigation_page nav(detail); // gives detail a parent

        flyout_page page;
        EXPECT_THROW(page.set_detail(&detail), std::runtime_error);
    }

    TEST_F(flyout_page_test, throws_when_adding_already_parented_flyout)
    {
        content_page flyout;
        flyout.set_title("Foo");
        maui::controls::navigation_page nav(flyout); // gives the flyout a parent

        flyout_page page;
        EXPECT_THROW(page.set_flyout(&flyout), std::runtime_error);
    }

    TEST_F(flyout_page_test, appearing_and_disappearing_propagates_to_flyout)
    {
        int disappearing = 0;
        int appearing = 0;

        content_page flyout;
        flyout.set_title("flyout");
        content_page detail;
        detail.set_title("detail");
        flyout_page page;
        page.set_flyout(&flyout);
        page.set_detail(&detail);

        page.send_appearing(); // the TestWindow role: the page appears once (window-driven in C#)
        flyout.appearing.connect([&appearing] { ++appearing; });
        flyout.disappearing.connect([&disappearing] { ++disappearing; });

        EXPECT_EQ(disappearing, 0);
        EXPECT_EQ(appearing, 0);

        page.send_disappearing();
        EXPECT_EQ(disappearing, 1);
        EXPECT_EQ(appearing, 0);

        page.send_appearing();
        EXPECT_EQ(disappearing, 1);
        EXPECT_EQ(appearing, 1);
    }

    TEST_F(flyout_page_test, appearing_and_disappearing_propagates_to_detail)
    {
        int disappearing = 0;
        int appearing = 0;

        content_page flyout;
        flyout.set_title("flyout");
        content_page detail;
        detail.set_title("detail");
        flyout_page page;
        page.set_flyout(&flyout);
        page.set_detail(&detail);

        page.send_appearing(); // the TestWindow role (see above)
        detail.appearing.connect([&appearing] { ++appearing; });
        detail.disappearing.connect([&disappearing] { ++disappearing; });

        EXPECT_EQ(disappearing, 0);
        EXPECT_EQ(appearing, 0);

        page.send_disappearing();
        EXPECT_EQ(disappearing, 1);
        EXPECT_EQ(appearing, 0);

        page.send_appearing();
        EXPECT_EQ(disappearing, 1);
        EXPECT_EQ(appearing, 1);
    }

    // Replacing a pane while the page has appeared hands the lifecycle over (the C# setter's
    // HasAppeared branch).
    TEST_F(flyout_page_test, replacing_detail_after_appearing_swaps_the_lifecycle)
    {
        content_page flyout;
        flyout.set_title("flyout");
        content_page first_detail;
        content_page second_detail;
        flyout_page page;
        page.set_flyout(&flyout);
        page.set_detail(&first_detail);
        page.send_appearing();
        EXPECT_TRUE(first_detail.has_appeared());

        page.set_detail(&second_detail);

        EXPECT_FALSE(first_detail.has_appeared());
        EXPECT_TRUE(second_detail.has_appeared());
        EXPECT_EQ(first_detail.logical_parent(), nullptr);
        EXPECT_EQ(second_detail.logical_parent(), &page);
    }

    // ---- the headless handler seam (flyout_page_platform mirrors + the computed behavior) ----

    TEST_F(flyout_page_test, handler_mirrors_panes_presented_and_behavior)
    {
        content_page flyout;
        flyout.set_title("Menu");
        content_page detail;
        flyout_page page;
        page.set_flyout(&flyout);
        page.set_detail(&detail);

        auto handler = std::make_shared<flyout_page_handler>();
        page.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        EXPECT_EQ(platform->hosted_flyout, &flyout);
        EXPECT_EQ(platform->hosted_detail, &detail);
        EXPECT_FALSE(platform->presented);
        EXPECT_EQ(platform->behavior, maui::core::flyout_behavior::flyout);
        EXPECT_TRUE(platform->gesture_enabled); // IsGestureEnabled default true

        page.set_is_presented(true);
        EXPECT_TRUE(platform->presented);

        page.set_is_gesture_enabled(false);
        EXPECT_FALSE(platform->gesture_enabled);
    }

    TEST_F(flyout_page_test, split_mode_locks_the_computed_behavior)
    {
        use_idiom(device_idiom::tablet());
        content_page flyout;
        flyout.set_title("Menu");
        content_page detail;
        flyout_page page;
        page.set_flyout(&flyout);
        page.set_detail(&detail);

        auto handler = std::make_shared<flyout_page_handler>();
        page.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        page.set_layout_behavior(flyout_layout_behavior::split);

        EXPECT_TRUE(platform->presented); // UpdateFlyoutLayoutBehavior presented the flyout
        EXPECT_EQ(platform->behavior, maui::core::flyout_behavior::locked);
        EXPECT_FALSE(page.can_change_is_presented());
    }

    TEST_F(flyout_page_test, orientation_change_remaps_the_computed_behavior)
    {
        use_idiom(device_idiom::tablet());
        const auto display = use_display(display_orientation::portrait);
        content_page flyout;
        flyout.set_title("Menu");
        content_page detail;
        flyout_page page;
        page.set_flyout(&flyout);
        page.set_detail(&detail);
        page.set_layout_behavior(flyout_layout_behavior::split_on_landscape);

        auto handler = std::make_shared<flyout_page_handler>();
        page.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_EQ(platform->behavior, maui::core::flyout_behavior::flyout);

        // The MainDisplayInfoChanged subscription re-reads the computed behavior on the handler.
        display->set_main_display_orientation(display_orientation::landscape);
        EXPECT_EQ(platform->behavior, maui::core::flyout_behavior::locked);
    }

    TEST_F(flyout_page_test, appearing_in_split_mode_presents_the_flyout)
    {
        use_idiom(device_idiom::tablet());
        use_display(display_orientation::landscape);
        content_page flyout;
        flyout.set_title("Menu");
        content_page detail;
        flyout_page page;
        page.set_flyout(&flyout);
        page.set_detail(&detail);
        // default behavior + landscape + non-phone => split mode on appearing.
        EXPECT_FALSE(page.is_presented());

        page.send_appearing();

        EXPECT_TRUE(page.is_presented());
        EXPECT_TRUE(page.can_change_is_presented()); // default behavior never locks it
    }
} // namespace
