// Tests for the state triggers + the VisualStateGroups attached-storage face (W1-15). Ported from
// StateTriggerTests.cs (InitialStateIsNormalIfAvailable, StateTriggerDefault/ChangedVisualState),
// AdaptiveTriggerTests.cs (ResizingWindowPageActivatesTrigger, ValidateAdaptiveTriggerDisconnects),
// OrientationStateTriggerTests.cs (CorrectStateIsAppliedWhenAttached, OrientationPropertyChange…,
// DeviceOrientationChange…, TriggerDeactivatesWhenDetached, MultipleTriggersWithDifferentOrientations)
// and DeviceStateTriggerTests.cs — with the C# Mock{DeviceDisplay,DeviceInfo} ported as local fakes
// behind the essentials set_current seams. State setters use label.text (the port's VSM test idiom)
// where C# styles Label.Background.
#include "maui/controls/state_trigger.hpp"

#include <memory>
#include <string>
#include <utility>

#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/setter.hpp"
#include "maui/controls/visual_state_manager.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/event.hpp"
#include "maui/essentials/device_display.hpp"
#include "maui/essentials/device_info.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::adaptive_trigger;
    using maui::controls::compare_state_trigger;
    using maui::controls::content_page;
    using maui::controls::device_state_trigger;
    using maui::controls::label;
    using maui::controls::orientation_state_trigger;
    using maui::controls::setter;
    using maui::controls::state_trigger;
    using maui::controls::visual_state;
    using maui::controls::visual_state_group;
    using maui::controls::visual_state_manager;
    using maui::controls::window;
    using maui::devices::device_display;
    using maui::devices::device_info;
    using maui::devices::display_info;
    using maui::devices::display_orientation;

    // ---- the C# test fakes, behind the essentials set_current seams ----

    // MockDeviceDisplay: a fixed display_info with a settable orientation that raises the change event.
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

    // MockDeviceInfo: a fixed platform.
    class mock_device_info final : public maui::devices::i_device_info
    {
    public:
        explicit mock_device_info(maui::devices::device_platform platform) : platform_(std::move(platform))
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
        [[nodiscard]] maui::devices::device_platform platform() const override
        {
            return platform_;
        }
        [[nodiscard]] maui::devices::device_idiom idiom() const override
        {
            return maui::devices::device_idiom::unknown();
        }
        [[nodiscard]] enum maui::devices::device_type device_type() const override
        {
            return maui::devices::device_type::virtual_;
        }

    private:
        maui::devices::device_platform platform_;
    };

    // Reset the facade seams after each test so the fakes never leak across tests.
    class state_trigger_test : public ::testing::Test
    {
    protected:
        void TearDown() override
        {
            device_display::set_current(nullptr);
            device_info::set_current(nullptr);
        }
    };

    // ---- StateTriggerTests.cs (CompareStateTrigger over the group list) ----

    // CreateTestStateGroups: Normal (no triggers) + Green (entry text == "Test") + Red (text == "").
    // C# snapshots TestEntry.Text into CompareStateTrigger.Property; the typed port snapshots the string.
    visual_state_manager make_compare_groups(const std::string& entry_text)
    {
        auto green_trigger = std::make_shared<compare_state_trigger<std::string>>();
        green_trigger->set_property(entry_text);
        green_trigger->set_value("Test");
        auto red_trigger = std::make_shared<compare_state_trigger<std::string>>();
        red_trigger->set_property(entry_text);
        red_trigger->set_value("");

        visual_state normal{"Normal"};
        visual_state green{"Green"};
        green.add(setter::of(label::text_property(), std::string("green")));
        green.add_state_trigger(std::move(green_trigger));
        visual_state red{"Red"};
        red.add(setter::of(label::text_property(), std::string("red")));
        red.add_state_trigger(std::move(red_trigger));

        visual_state_group group{"CommonStates"};
        group.add(std::move(normal));
        group.add(std::move(green));
        group.add(std::move(red));

        visual_state_manager manager;
        manager.add_group(std::move(group));
        return manager;
    }

    TEST_F(state_trigger_test, initial_state_is_normal_if_available)
    {
        // InitialStateIsNormalIfAvailable — no trigger active (C#: the entry's null Text matches
        // neither comparand; the typed port uses a non-matching snapshot), so ChangeVisualState's
        // Normal stays current after the groups are stored.
        label target;
        target.set_visual_state_groups(make_compare_groups("x"));
        EXPECT_EQ(target.visual_states().groups()[0].current_state_name(), "Normal");
    }

    TEST_F(state_trigger_test, state_trigger_selects_the_default_visual_state)
    {
        // StateTriggerDefaultVisualState: the empty text matches Red's trigger ("" == "").
        label target;
        target.set_visual_state_groups(make_compare_groups(""));
        EXPECT_EQ(target.visual_states().groups()[0].current_state_name(), "Red");
        EXPECT_EQ(target.text(), "red"); // the state's setters applied
    }

    TEST_F(state_trigger_test, state_trigger_selects_the_changed_visual_state)
    {
        // StateTriggerChangedVisualState: "Test" matches Green's trigger.
        label target;
        target.set_visual_state_groups(make_compare_groups("Test"));
        EXPECT_EQ(target.visual_states().groups()[0].current_state_name(), "Green");
        EXPECT_EQ(target.text(), "green");
    }

    TEST_F(state_trigger_test, manual_state_trigger_activates_its_state)
    {
        // StateTrigger.IsActive: flipping it transitions the group (and back off leaves the state —
        // GetActiveTrigger returns null with no active trigger, leaving the current state alone).
        auto trigger = std::make_shared<state_trigger>();
        visual_state active{"Active"};
        active.add(setter::of(label::text_property(), std::string("on")));
        active.add_state_trigger(trigger);
        visual_state_group group{"CommonStates"};
        group.add(std::move(active));
        visual_state_manager manager;
        manager.add_group(std::move(group));

        label target;
        target.set_visual_state_groups(std::move(manager));
        EXPECT_EQ(target.text(), "");

        trigger->set_is_active(true);
        EXPECT_EQ(target.text(), "on");
        EXPECT_EQ(target.visual_states().groups()[0].current_state_name(), "Active");
    }

    // ---- AdaptiveTriggerTests.cs ----

    TEST_F(state_trigger_test, resizing_the_window_activates_the_matching_adaptive_trigger)
    {
        // ResizingWindowPageActivatesTrigger: Large (MinWindowWidth 300) vs Small (MinWindowWidth 0);
        // at width 500 BOTH are active and the conflict resolution picks the LARGEST MinWindowWidth.
        auto large_trigger = std::make_shared<adaptive_trigger>();
        large_trigger->set_min_window_width(300);
        auto small_trigger = std::make_shared<adaptive_trigger>();
        small_trigger->set_min_window_width(0);

        visual_state large{"Large"};
        large.add(setter::of(label::text_property(), std::string("large")));
        large.add_state_trigger(large_trigger);
        visual_state small{"Small"};
        small.add(setter::of(label::text_property(), std::string("small")));
        small.add_state_trigger(small_trigger);
        visual_state_group group{"AdaptiveStates"};
        group.add(std::move(large));
        group.add(std::move(small));
        visual_state_manager manager;
        manager.add_group(std::move(group));

        label target;
        content_page page;
        page.set_content(target);
        target.set_visual_state_groups(std::move(manager));

        window win;
        win.set_content(page);
        win.send_created();
        win.send_activated(); // the page (+ label) enters the window → triggers attach

        win.frame_changed({0, 0, 100, 100});
        EXPECT_EQ(target.text(), "small");

        win.frame_changed({0, 0, 500, 100});
        EXPECT_EQ(target.text(), "large"); // both active → largest MinWindowWidth wins

        win.frame_changed({0, 0, 100, 100});
        EXPECT_EQ(target.text(), "small");
    }

    TEST_F(state_trigger_test, adaptive_trigger_attaches_with_the_window_and_detaches_without_it)
    {
        // ValidateAdaptiveTriggerDisconnects: IsAttached follows the host's window membership.
        auto trigger = std::make_shared<adaptive_trigger>();
        trigger->set_min_window_width(300);
        visual_state large{"Large"};
        large.add_state_trigger(trigger);
        visual_state_group group{"AdaptiveStates"};
        group.add(std::move(large));
        visual_state_manager manager;
        manager.add_group(std::move(group));

        label target;
        content_page page;
        page.set_content(target);
        target.set_visual_state_groups(std::move(manager));
        EXPECT_FALSE(trigger->is_attached());

        window win;
        win.set_content(page);
        win.send_created();
        win.send_activated();
        EXPECT_TRUE(trigger->is_attached());

        label replacement;
        page.set_content(replacement); // the label leaves the window subtree → unloaded → detach
        EXPECT_FALSE(trigger->is_attached());
    }

    // ---- OrientationStateTriggerTests.cs ----

    visual_state_manager make_orientation_groups(const std::shared_ptr<orientation_state_trigger>& trigger,
                                                 std::string text)
    {
        visual_state state{"OrientationState"};
        state.add(setter::of(label::text_property(), std::move(text)));
        state.add_state_trigger(trigger);
        visual_state_group group{"OrientationStates"};
        group.add(std::move(state));
        visual_state_manager manager;
        manager.add_group(std::move(group));
        return manager;
    }

    TEST_F(state_trigger_test, orientation_state_applies_when_the_orientation_matches)
    {
        // CorrectStateIsAppliedWhenAttached(Portrait, Portrait, true) — no window needed: the trigger
        // computes IsActive from DeviceDisplay at construction/property-set, like C#.
        device_display::set_current(std::make_shared<mock_device_display>(
            display_info{.width = 100, .height = 200, .density = 2, .orientation = display_orientation::portrait}));

        auto trigger = std::make_shared<orientation_state_trigger>();
        trigger->set_orientation(display_orientation::portrait);
        label target;
        target.set_visual_state_groups(make_orientation_groups(trigger, "green"));
        EXPECT_EQ(target.text(), "green");
    }

    TEST_F(state_trigger_test, orientation_state_does_not_apply_when_the_orientation_differs)
    {
        // CorrectStateIsAppliedWhenAttached(Portrait, Landscape, false) + the Unknown rows (an Unknown
        // trigger orientation never activates).
        device_display::set_current(std::make_shared<mock_device_display>(
            display_info{.width = 100, .height = 200, .density = 2, .orientation = display_orientation::landscape}));

        auto portrait_trigger = std::make_shared<orientation_state_trigger>();
        portrait_trigger->set_orientation(display_orientation::portrait);
        label portrait_target;
        portrait_target.set_visual_state_groups(make_orientation_groups(portrait_trigger, "green"));
        EXPECT_EQ(portrait_target.text(), "");

        auto unknown_trigger = std::make_shared<orientation_state_trigger>();
        unknown_trigger->set_orientation(display_orientation::unknown);
        label unknown_target;
        unknown_target.set_visual_state_groups(make_orientation_groups(unknown_trigger, "green"));
        EXPECT_EQ(unknown_target.text(), "");
    }

    TEST_F(state_trigger_test, changing_the_trigger_orientation_re_evaluates_the_state)
    {
        // OrientationPropertyChangeTriggersStateUpdate: device is Landscape; a Portrait trigger does
        // not apply until its Orientation is changed to Landscape.
        device_display::set_current(std::make_shared<mock_device_display>(
            display_info{.width = 100, .height = 200, .density = 2, .orientation = display_orientation::landscape}));

        auto trigger = std::make_shared<orientation_state_trigger>();
        trigger->set_orientation(display_orientation::portrait);
        label target;
        target.set_visual_state_groups(make_orientation_groups(trigger, "green"));
        EXPECT_EQ(target.text(), "");

        trigger->set_orientation(display_orientation::landscape);
        EXPECT_EQ(target.text(), "green");
    }

    TEST_F(state_trigger_test, a_device_orientation_change_re_evaluates_an_attached_trigger)
    {
        // DeviceOrientationChangeTriggersStateUpdate: the DeviceDisplay subscription only exists while
        // attached (in a window).
        auto display = std::make_shared<mock_device_display>(
            display_info{.width = 100, .height = 200, .density = 2, .orientation = display_orientation::portrait});
        device_display::set_current(display);

        auto trigger = std::make_shared<orientation_state_trigger>();
        trigger->set_orientation(display_orientation::landscape);
        label target;
        content_page page;
        page.set_content(target);
        target.set_visual_state_groups(make_orientation_groups(trigger, "green"));

        window win;
        win.set_content(page);
        win.send_created();
        win.send_activated();
        EXPECT_EQ(target.text(), ""); // still portrait

        display->set_main_display_orientation(display_orientation::landscape);
        EXPECT_EQ(target.text(), "green");
    }

    TEST_F(state_trigger_test, orientation_trigger_detaches_when_the_element_leaves_the_window)
    {
        // TriggerDeactivatesWhenDetached (the IsAttached half — C# swaps Window.Page; the port swaps
        // the page content, the same subtree-leaves-the-window transition).
        device_display::set_current(std::make_shared<mock_device_display>(
            display_info{.width = 100, .height = 200, .density = 2, .orientation = display_orientation::portrait}));

        auto trigger = std::make_shared<orientation_state_trigger>();
        trigger->set_orientation(display_orientation::portrait);
        label target;
        content_page page;
        page.set_content(target);
        target.set_visual_state_groups(make_orientation_groups(trigger, "green"));
        EXPECT_FALSE(trigger->is_attached());

        window win;
        win.set_content(page);
        win.send_created();
        win.send_activated();
        EXPECT_TRUE(trigger->is_attached());

        label replacement;
        page.set_content(replacement);
        EXPECT_FALSE(trigger->is_attached());
    }

    TEST_F(state_trigger_test, multiple_orientation_triggers_pick_the_matching_state)
    {
        // MultipleTriggersWithDifferentOrientations: Landscape device → the Landscape state applies.
        device_display::set_current(std::make_shared<mock_device_display>(
            display_info{.width = 100, .height = 200, .density = 2, .orientation = display_orientation::landscape}));

        auto portrait_trigger = std::make_shared<orientation_state_trigger>();
        portrait_trigger->set_orientation(display_orientation::portrait);
        auto landscape_trigger = std::make_shared<orientation_state_trigger>();
        landscape_trigger->set_orientation(display_orientation::landscape);

        visual_state portrait_state{"PortraitState"};
        portrait_state.add(setter::of(label::text_property(), std::string("green")));
        portrait_state.add_state_trigger(portrait_trigger);
        visual_state landscape_state{"LandscapeState"};
        landscape_state.add(setter::of(label::text_property(), std::string("blue")));
        landscape_state.add_state_trigger(landscape_trigger);
        visual_state_group group{"OrientationStates"};
        group.add(std::move(portrait_state));
        group.add(std::move(landscape_state));
        visual_state_manager manager;
        manager.add_group(std::move(group));

        label target;
        target.set_visual_state_groups(std::move(manager));
        EXPECT_EQ(target.text(), "blue");
    }

    // ---- DeviceStateTriggerTests.cs ----

    TEST_F(state_trigger_test, device_state_trigger_matches_the_current_platform)
    {
        device_info::set_current(std::make_shared<mock_device_info>(maui::devices::device_platform::android()));

        // ("Android", true): the trigger's platform matches the device.
        auto android_trigger = std::make_shared<device_state_trigger>();
        android_trigger->set_device("Android");
        visual_state android_state{"AndroidThings"};
        android_state.add(setter::of(label::text_property(), std::string("green")));
        android_state.add_state_trigger(android_trigger);
        visual_state_group android_group{"DeviceStates"};
        android_group.add(std::move(android_state));
        visual_state_manager android_manager;
        android_manager.add_group(std::move(android_group));
        label android_target;
        android_target.set_visual_state_groups(std::move(android_manager));
        EXPECT_EQ(android_target.text(), "green");

        // ("iOS", false): a different platform never activates.
        auto ios_trigger = std::make_shared<device_state_trigger>();
        ios_trigger->set_device("iOS");
        visual_state ios_state{"AndroidThings"};
        ios_state.add(setter::of(label::text_property(), std::string("green")));
        ios_state.add_state_trigger(ios_trigger);
        visual_state_group ios_group{"DeviceStates"};
        ios_group.add(std::move(ios_state));
        visual_state_manager ios_manager;
        ios_manager.add_group(std::move(ios_group));
        label ios_target;
        ios_target.set_visual_state_groups(std::move(ios_manager));
        EXPECT_EQ(ios_target.text(), "");
    }
} // namespace
