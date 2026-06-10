// Gesture recognizer tests (headless) — the E1 gestures unit. Ports the C# behavioral oracles
// against the synthetic controller-interface drives:
//   - PanGestureRecognizerUnitTests.cs       (the IPanGestureController state machine)
//   - Gestures/PinchGestureRecognizerTests.cs (the IPinchGestureController state machine + the
//                                              one-pinch-per-view validation)
//   - Gestures/SwipeGestureRecognizerTests.cs (SendSwipe/DetectSwipe thresholds + direction flags +
//                                              the TransformSwipeDirectionForRotation theories)
//   - Gestures/TapGestureRecognizerTests.cs   (defaults; the Command tests are not portable — no
//                                              ICommand port — so send_tapped's event raise is pinned)
//   - Gestures/PointerGestureRecognizerTests.cs (Buttons property surface + ClearingGestureRecognizers)
// plus the port's collection → gesture_platform_manager → recognizer pipeline: attachment diffing on
// handler/collection changes and the synthetic dispatch (the headless stand-in for the native
// recognizers), including each bridge filter (tap count + button mask, pan touch points + gesture ids,
// the pinch IsPinching guards, the swipe threshold).

#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "maui/controls/button.hpp"
#include "maui/controls/gestures/buttons_mask.hpp"
#include "maui/controls/gestures/gesture_platform_manager.hpp"
#include "maui/controls/gestures/gesture_recognizer_collection.hpp"
#include "maui/controls/gestures/pan_gesture_recognizer.hpp"
#include "maui/controls/gestures/pinch_gesture_recognizer.hpp"
#include "maui/controls/gestures/pointer_gesture_recognizer.hpp"
#include "maui/controls/gestures/swipe_gesture_recognizer.hpp"
#include "maui/controls/gestures/tap_gesture_recognizer.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/gesture_status.hpp"
#include "maui/core/swipe_direction.hpp"
#include "maui/graphics/point.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::buttons_mask;
    using maui::controls::i_pan_gesture_controller;
    using maui::controls::i_pinch_gesture_controller;
    using maui::controls::i_swipe_gesture_controller;
    using maui::controls::pan_gesture_recognizer;
    using maui::controls::pan_updated_event_args;
    using maui::controls::pinch_gesture_recognizer;
    using maui::controls::pinch_gesture_updated_event_args;
    using maui::controls::pointer_event_args;
    using maui::controls::pointer_event_kind;
    using maui::controls::pointer_gesture_recognizer;
    using maui::controls::swipe_gesture_recognizer;
    using maui::controls::swiped_event_args;
    using maui::controls::tap_gesture_recognizer;
    using maui::controls::tapped_event_args;
    using maui::core::gesture_status;
    using maui::core::swipe_direction;
    using maui::graphics::point;

    // The C# tests' `new View()` stand-in: any concrete view<> works; button is the canonical one.
    using test_view = maui::controls::button;

    // ---- PanGestureRecognizerUnitTests.cs ----

    TEST(pan_gesture_recognizer_test, pan_raises_started_event)
    {
        test_view view;
        pan_gesture_recognizer pan;

        gesture_status target = gesture_status::canceled;
        pan.pan_updated.connect([&target](const pan_updated_event_args& e) { target = e.status_type; });

        static_cast<i_pan_gesture_controller&>(pan).send_pan_started(view, 0);
        EXPECT_EQ(target, gesture_status::started);
    }

    TEST(pan_gesture_recognizer_test, pan_raises_running_event)
    {
        test_view view;
        pan_gesture_recognizer pan;

        gesture_status target = gesture_status::canceled;
        pan.pan_updated.connect([&target](const pan_updated_event_args& e) { target = e.status_type; });

        static_cast<i_pan_gesture_controller&>(pan).send_pan(view, /*total_x=*/5, /*total_y=*/10,
                                                             /*gesture_id=*/0);
        EXPECT_EQ(target, gesture_status::running);
    }

    TEST(pan_gesture_recognizer_test, pan_running_event_contains_total_x)
    {
        test_view view;
        pan_gesture_recognizer pan;

        double target = 0;
        pan.pan_updated.connect([&target](const pan_updated_event_args& e) { target = e.total_x; });

        static_cast<i_pan_gesture_controller&>(pan).send_pan(view, 5, 10, 0);
        EXPECT_EQ(target, 5);
    }

    TEST(pan_gesture_recognizer_test, pan_running_event_contains_total_y)
    {
        test_view view;
        pan_gesture_recognizer pan;

        double target = 0;
        pan.pan_updated.connect([&target](const pan_updated_event_args& e) { target = e.total_y; });

        static_cast<i_pan_gesture_controller&>(pan).send_pan(view, 5, 10, 0);
        EXPECT_EQ(target, 10);
    }

    TEST(pan_gesture_recognizer_test, pan_raises_completed_event)
    {
        test_view view;
        pan_gesture_recognizer pan;

        gesture_status target = gesture_status::canceled;
        pan.pan_updated.connect([&target](const pan_updated_event_args& e) { target = e.status_type; });

        static_cast<i_pan_gesture_controller&>(pan).send_pan_completed(view, 0);
        EXPECT_EQ(target, gesture_status::completed);
    }

    TEST(pan_gesture_recognizer_test, pan_raises_canceled_event)
    {
        test_view view;
        pan_gesture_recognizer pan;

        gesture_status target = gesture_status::started;
        pan.pan_updated.connect([&target](const pan_updated_event_args& e) { target = e.status_type; });

        static_cast<i_pan_gesture_controller&>(pan).send_pan_canceled(view, 0);
        EXPECT_EQ(target, gesture_status::canceled);
    }

    TEST(pan_gesture_recognizer_test, touch_points_defaults_to_one)
    {
        const pan_gesture_recognizer pan;
        EXPECT_EQ(pan.touch_points(), 1); // TouchPointsProperty default
    }

    // ---- Gestures/PinchGestureRecognizerTests.cs ----

    TEST(pinch_gesture_recognizer_test, constructor)
    {
        const pinch_gesture_recognizer pinch;
        EXPECT_FALSE(pinch.is_pinching());
    }

    TEST(pinch_gesture_recognizer_test, pinch_started)
    {
        test_view view;
        pinch_gesture_recognizer pinch;

        gesture_status result = gesture_status::canceled;
        const point origin(10, 10);
        point result_point = point::zero;
        pinch.pinch_updated.connect([&](const pinch_gesture_updated_event_args& e) {
            result = e.status;
            result_point = e.scale_origin;
        });

        static_cast<i_pinch_gesture_controller&>(pinch).send_pinch_started(view, origin);
        EXPECT_EQ(result, gesture_status::started);
        EXPECT_EQ(result_point.x, origin.x);
        EXPECT_EQ(result_point.y, origin.y);
        EXPECT_TRUE(pinch.is_pinching());
    }

    TEST(pinch_gesture_recognizer_test, pinch_completed)
    {
        test_view view;
        pinch_gesture_recognizer pinch;

        gesture_status result = gesture_status::canceled;
        pinch.pinch_updated.connect([&result](const pinch_gesture_updated_event_args& e) { result = e.status; });

        static_cast<i_pinch_gesture_controller&>(pinch).send_pinch_ended(view);
        EXPECT_EQ(result, gesture_status::completed);
        EXPECT_FALSE(pinch.is_pinching());
    }

    TEST(pinch_gesture_recognizer_test, pinch_updated_carries_scale)
    {
        test_view view;
        pinch_gesture_recognizer pinch;
        const point origin(10, 10);
        double result = -1;
        pinch.pinch_updated.connect([&result](const pinch_gesture_updated_event_args& e) { result = e.scale; });

        static_cast<i_pinch_gesture_controller&>(pinch).send_pinch(view, 2, origin);
        EXPECT_EQ(result, 2);
    }

    TEST(pinch_gesture_recognizer_test, only_one_pinch_gesture_per_view)
    {
        test_view view;
        view.gesture_recognizers().add(std::make_shared<pinch_gesture_recognizer>());
        EXPECT_THROW(view.gesture_recognizers().add(std::make_shared<pinch_gesture_recognizer>()), std::runtime_error);
    }

    // ---- Gestures/SwipeGestureRecognizerTests.cs ----

    TEST(swipe_gesture_recognizer_test, constructor)
    {
        const swipe_gesture_recognizer swipe;
        EXPECT_EQ(swipe.threshold(), 100U);
        EXPECT_EQ(swipe.direction(), swipe_direction::none); // default(SwipeDirection)
    }

    TEST(swipe_gesture_recognizer_test, swiped_event_direction_matches_total_x)
    {
        test_view view;
        swipe_gesture_recognizer swipe;

        swipe_direction direction = swipe_direction::up;
        swipe.swiped.connect([&direction](const swiped_event_args& e) { direction = e.direction; });

        auto& controller = static_cast<i_swipe_gesture_controller&>(swipe);
        controller.send_swipe(view, /*total_x=*/-150, /*total_y=*/10);
        (void)controller.detect_swipe(view, swipe_direction::left);
        EXPECT_EQ(direction, swipe_direction::left);
    }

    TEST(swipe_gesture_recognizer_test, swiped_event_direction_matches_total_y)
    {
        test_view view;
        swipe_gesture_recognizer swipe;

        swipe_direction direction = swipe_direction::left;
        swipe.swiped.connect([&direction](const swiped_event_args& e) { direction = e.direction; });

        auto& controller = static_cast<i_swipe_gesture_controller&>(swipe);
        controller.send_swipe(view, /*total_x=*/10, /*total_y=*/-150);
        (void)controller.detect_swipe(view, swipe_direction::up);
        EXPECT_EQ(direction, swipe_direction::up);
    }

    TEST(swipe_gesture_recognizer_test, swipe_ignored_if_below_threshold)
    {
        test_view view;
        swipe_gesture_recognizer swipe;
        swipe.set_threshold(200); // custom threshold for the test

        bool detected = false;
        swipe.swiped.connect([&detected](const swiped_event_args& /*e*/) { detected = true; });

        auto& controller = static_cast<i_swipe_gesture_controller&>(swipe);
        controller.send_swipe(view, 0, -175);
        EXPECT_FALSE(controller.detect_swipe(view, swipe_direction::up));
        EXPECT_FALSE(detected);
    }

    TEST(swipe_gesture_recognizer_test, swiped_event_direction_matches_total_x_with_flags)
    {
        test_view view;
        swipe_gesture_recognizer swipe;

        swipe_direction direction = swipe_direction::up;
        swipe.swiped.connect([&direction](const swiped_event_args& e) { direction = e.direction; });

        auto& controller = static_cast<i_swipe_gesture_controller&>(swipe);
        controller.send_swipe(view, -150, 10);
        (void)controller.detect_swipe(view, swipe_direction::left | swipe_direction::right);
        EXPECT_EQ(direction, swipe_direction::left);
    }

    TEST(swipe_gesture_recognizer_test, swiped_event_direction_matches_total_y_with_flags)
    {
        test_view view;
        swipe_gesture_recognizer swipe;

        swipe_direction direction = swipe_direction::left;
        swipe.swiped.connect([&direction](const swiped_event_args& e) { direction = e.direction; });

        auto& controller = static_cast<i_swipe_gesture_controller&>(swipe);
        controller.send_swipe(view, 10, -150);
        (void)controller.detect_swipe(view, swipe_direction::up | swipe_direction::down);
        EXPECT_EQ(direction, swipe_direction::up);
    }

    TEST(swipe_gesture_recognizer_test, send_swiped_raises_with_direction)
    {
        test_view view;
        swipe_gesture_recognizer swipe;

        std::vector<swipe_direction> raised;
        swipe.swiped.connect([&raised](const swiped_event_args& e) { raised.push_back(e.direction); });

        swipe.send_swiped(view, swipe_direction::left);
        ASSERT_EQ(raised.size(), 1U);
        EXPECT_EQ(raised[0], swipe_direction::left);
    }

    // The TransformSwipeDirectionForRotation theories (SwipeGestureRecognizerTests.cs [InlineData]).
    struct rotation_case
    {
        swipe_direction direction;
        double rotation;
        swipe_direction expected;
    };

    TEST(swipe_gesture_recognizer_test, transform_swipe_direction_for_rotation_90_degree_clockwise)
    {
        const std::vector<rotation_case> cases{
            {.direction = swipe_direction::up, .rotation = 90.0, .expected = swipe_direction::right},
            {.direction = swipe_direction::right, .rotation = 90.0, .expected = swipe_direction::down},
            {.direction = swipe_direction::down, .rotation = 90.0, .expected = swipe_direction::left},
            {.direction = swipe_direction::left, .rotation = 90.0, .expected = swipe_direction::up},
        };
        for (const auto& c : cases)
        {
            EXPECT_EQ(maui::controls::transform_swipe_direction_for_rotation(c.direction, c.rotation), c.expected);
        }
    }

    TEST(swipe_gesture_recognizer_test, transform_swipe_direction_for_rotation_180_degree)
    {
        const std::vector<rotation_case> cases{
            {.direction = swipe_direction::up, .rotation = 180.0, .expected = swipe_direction::down},
            {.direction = swipe_direction::right, .rotation = 180.0, .expected = swipe_direction::left},
            {.direction = swipe_direction::down, .rotation = 180.0, .expected = swipe_direction::up},
            {.direction = swipe_direction::left, .rotation = 180.0, .expected = swipe_direction::right},
        };
        for (const auto& c : cases)
        {
            EXPECT_EQ(maui::controls::transform_swipe_direction_for_rotation(c.direction, c.rotation), c.expected);
        }
    }

    TEST(swipe_gesture_recognizer_test, transform_swipe_direction_for_rotation_270_degree_clockwise)
    {
        const std::vector<rotation_case> cases{
            {.direction = swipe_direction::up, .rotation = 270.0, .expected = swipe_direction::left},
            {.direction = swipe_direction::right, .rotation = 270.0, .expected = swipe_direction::up},
            {.direction = swipe_direction::down, .rotation = 270.0, .expected = swipe_direction::right},
            {.direction = swipe_direction::left, .rotation = 270.0, .expected = swipe_direction::down},
        };
        for (const auto& c : cases)
        {
            EXPECT_EQ(maui::controls::transform_swipe_direction_for_rotation(c.direction, c.rotation), c.expected);
        }
    }

    TEST(swipe_gesture_recognizer_test, transform_swipe_direction_for_rotation_invalid_rotation)
    {
        const std::vector<rotation_case> cases{
            {.direction = swipe_direction::up,
             .rotation = std::numeric_limits<double>::quiet_NaN(),
             .expected = swipe_direction::up},
            {.direction = swipe_direction::right,
             .rotation = std::numeric_limits<double>::infinity(),
             .expected = swipe_direction::right},
            {.direction = swipe_direction::down,
             .rotation = -std::numeric_limits<double>::infinity(),
             .expected = swipe_direction::down},
        };
        for (const auto& c : cases)
        {
            EXPECT_EQ(maui::controls::transform_swipe_direction_for_rotation(c.direction, c.rotation), c.expected);
        }
    }

    // ---- Gestures/TapGestureRecognizerTests.cs ----

    TEST(tap_gesture_recognizer_test, constructor)
    {
        const tap_gesture_recognizer tap;
        EXPECT_EQ(tap.number_of_taps_required(), 1);
        EXPECT_EQ(tap.buttons(), buttons_mask::primary);
        // The C# Constructor test also pins Command/CommandParameter == null — ICommand is not ported
        // (documented deviation), so there is nothing to pin here.
    }

    TEST(tap_gesture_recognizer_test, send_tapped_raises_with_buttons_and_position)
    {
        test_view view;
        tap_gesture_recognizer tap;
        tap.set_buttons(buttons_mask::secondary);

        std::vector<tapped_event_args> raised;
        tap.tapped.connect([&raised](const tapped_event_args& e) { raised.push_back(e); });

        tap.send_tapped(view, point(3, 4));
        ASSERT_EQ(raised.size(), 1U);
        EXPECT_EQ(raised[0].buttons, buttons_mask::secondary); // TappedEventArgs.Buttons = recognizer mask
        // Compare whole optionals (avoids an unchecked .value() after the has_value() guard).
        EXPECT_EQ(raised[0].position, std::optional<point>(point(3, 4)));
    }

    // ---- Gestures/PointerGestureRecognizerTests.cs (the portable, command-free subset) ----

    TEST(pointer_gesture_recognizer_test, buttons_property_default_value)
    {
        const pointer_gesture_recognizer gesture;
        EXPECT_EQ(gesture.buttons(), buttons_mask::primary);
    }

    TEST(pointer_gesture_recognizer_test, buttons_property_can_be_set)
    {
        pointer_gesture_recognizer gesture;

        gesture.set_buttons(buttons_mask::secondary);
        EXPECT_EQ(gesture.buttons(), buttons_mask::secondary);

        gesture.set_buttons(buttons_mask::primary | buttons_mask::secondary);
        EXPECT_EQ(gesture.buttons(), buttons_mask::primary | buttons_mask::secondary);
    }

    TEST(pointer_gesture_recognizer_test, buttons_property_changed_event)
    {
        pointer_gesture_recognizer gesture;
        bool property_changed = false;
        std::string changed_property;

        gesture.property_changed.connect([&](std::string_view name) {
            property_changed = true;
            changed_property = std::string(name);
        });

        gesture.set_buttons(buttons_mask::secondary);

        EXPECT_TRUE(property_changed);
        EXPECT_EQ(changed_property, "buttons");
    }

    TEST(pointer_gesture_recognizer_test, buttons_property_change_triggers_notification_once)
    {
        pointer_gesture_recognizer gesture;
        int property_changed_count = 0;

        gesture.property_changed.connect([&property_changed_count](std::string_view name) {
            if (name == "buttons")
            {
                ++property_changed_count;
            }
        });

        gesture.set_buttons(buttons_mask::secondary);
        EXPECT_EQ(property_changed_count, 1);

        gesture.set_buttons(buttons_mask::primary | buttons_mask::secondary);
        EXPECT_EQ(property_changed_count, 2);

        // Setting the same value shouldn't trigger a change.
        gesture.set_buttons(buttons_mask::primary | buttons_mask::secondary);
        EXPECT_EQ(property_changed_count, 2);
    }

    TEST(pointer_gesture_recognizer_test, buttons_property_default_value_matches_tap_gesture)
    {
        const pointer_gesture_recognizer pointer_gesture;
        const tap_gesture_recognizer tap_gesture;
        EXPECT_EQ(tap_gesture.buttons(), pointer_gesture.buttons());
        EXPECT_EQ(pointer_gesture.buttons(), buttons_mask::primary);
    }

    TEST(pointer_gesture_recognizer_test, send_methods_raise_their_events)
    {
        test_view view;
        pointer_gesture_recognizer gesture;

        std::vector<std::string> raised;
        gesture.pointer_entered.connect([&raised](const pointer_event_args&) { raised.emplace_back("entered"); });
        gesture.pointer_exited.connect([&raised](const pointer_event_args&) { raised.emplace_back("exited"); });
        gesture.pointer_moved.connect([&raised](const pointer_event_args&) { raised.emplace_back("moved"); });
        gesture.pointer_pressed.connect([&raised](const pointer_event_args&) { raised.emplace_back("pressed"); });
        gesture.pointer_released.connect([&raised](const pointer_event_args&) { raised.emplace_back("released"); });

        gesture.send_pointer_entered(view, point(1, 1));
        gesture.send_pointer_moved(view, point(2, 2));
        gesture.send_pointer_pressed(view, point(2, 2));
        gesture.send_pointer_released(view, point(2, 2));
        gesture.send_pointer_exited(view, point(9, 9));

        const std::vector<std::string> expected{"entered", "moved", "pressed", "released", "exited"};
        EXPECT_EQ(raised, expected);
    }

    // PointerGestureRecognizerTests.ClearingGestureRecognizers, minus the PointerOver-VSM composite
    // recognizer (SetupForPointerOverVSM is not ported — documented deviation): clearing unparents.
    TEST(pointer_gesture_recognizer_test, clearing_gesture_recognizers)
    {
        test_view view;
        auto gesture = std::make_shared<tap_gesture_recognizer>();

        view.gesture_recognizers().add(gesture);
        EXPECT_EQ(view.gesture_recognizers().count(), 1U);
        EXPECT_EQ(gesture->logical_parent(), &view); // item.Parent = view

        view.gesture_recognizers().clear();
        EXPECT_EQ(view.gesture_recognizers().count(), 0U);
        EXPECT_EQ(gesture->logical_parent(), nullptr); // Parent cleared on removal
    }

    // ---- the collection → manager → recognizer pipeline (GestureManager + GesturePlatformManager) ----

    struct bound_context
    {
        std::string name;
    };

    TEST(gesture_pipeline_test, binding_context_propagates_to_recognizers)
    {
        test_view view;
        auto tap = std::make_shared<tap_gesture_recognizer>();
        view.gesture_recognizers().add(tap);

        // On a later context change, View.OnBindingContextChanged propagates to the recognizers.
        auto context = std::make_shared<bound_context>(bound_context{.name = "vm"});
        view.set_binding_context(context);
        EXPECT_EQ(tap->binding_context<bound_context>(), context);

        // A recognizer added AFTER the context is set inherits it immediately (attach_logical_child).
        auto pan = std::make_shared<pan_gesture_recognizer>();
        view.gesture_recognizers().add(pan);
        EXPECT_EQ(pan->binding_context<bound_context>(), context);
    }

    TEST(gesture_pipeline_test, handler_attach_loads_existing_recognizers)
    {
        test_view view;
        view.gesture_recognizers().add(std::make_shared<tap_gesture_recognizer>());
        view.gesture_recognizers().add(std::make_shared<pan_gesture_recognizer>());
        EXPECT_EQ(view.gesture_manager().attached_count(), 0U); // no handler yet — nothing to attach to

        view.set_handler(std::make_shared<maui::core::button_handler>());
        EXPECT_EQ(view.gesture_manager().attached_count(), 2U); // LoadRecognizers on handler attach
    }

    TEST(gesture_pipeline_test, collection_changes_resync_attachments)
    {
        test_view view;
        view.set_handler(std::make_shared<maui::core::button_handler>());

        auto tap = std::make_shared<tap_gesture_recognizer>();
        view.gesture_recognizers().add(tap);
        EXPECT_EQ(view.gesture_manager().attached_count(), 1U);
        EXPECT_TRUE(view.gesture_manager().is_attached(*tap));

        auto swipe = std::make_shared<swipe_gesture_recognizer>();
        view.gesture_recognizers().add(swipe);
        EXPECT_EQ(view.gesture_manager().attached_count(), 2U);

        EXPECT_TRUE(view.gesture_recognizers().remove(tap));
        EXPECT_EQ(view.gesture_manager().attached_count(), 1U);
        EXPECT_FALSE(view.gesture_manager().is_attached(*tap));

        view.gesture_recognizers().clear();
        EXPECT_EQ(view.gesture_manager().attached_count(), 0U);
    }

    TEST(gesture_pipeline_test, clearing_handler_detaches_everything)
    {
        test_view view;
        view.gesture_recognizers().add(std::make_shared<tap_gesture_recognizer>());
        view.set_handler(std::make_shared<maui::core::button_handler>());
        EXPECT_EQ(view.gesture_manager().attached_count(), 1U);

        view.set_handler(nullptr); // GestureManager.DisconnectGestures
        EXPECT_EQ(view.gesture_manager().attached_count(), 0U);
    }

    TEST(gesture_pipeline_test, synthetic_tap_honors_count_and_button_filters)
    {
        test_view view;
        view.set_handler(std::make_shared<maui::core::button_handler>());

        auto single_tap = std::make_shared<tap_gesture_recognizer>();
        auto double_tap = std::make_shared<tap_gesture_recognizer>();
        double_tap->set_number_of_taps_required(2);
        auto secondary_tap = std::make_shared<tap_gesture_recognizer>();
        secondary_tap->set_buttons(buttons_mask::secondary);
        view.gesture_recognizers().add(single_tap);
        view.gesture_recognizers().add(double_tap);
        view.gesture_recognizers().add(secondary_tap);

        int singles = 0;
        int doubles = 0;
        int secondaries = 0;
        single_tap->tapped.connect([&singles](const tapped_event_args&) { ++singles; });
        double_tap->tapped.connect([&doubles](const tapped_event_args&) { ++doubles; });
        secondary_tap->tapped.connect([&secondaries](const tapped_event_args&) { ++secondaries; });

        view.gesture_manager().synthetic_tap(/*number_of_taps=*/1, buttons_mask::primary);
        EXPECT_EQ(singles, 1);
        EXPECT_EQ(doubles, 0);     // NumberOfTapsRequired filter
        EXPECT_EQ(secondaries, 0); // ButtonMask filter

        view.gesture_manager().synthetic_tap(2, buttons_mask::primary);
        EXPECT_EQ(doubles, 1);

        view.gesture_manager().synthetic_tap(1, buttons_mask::secondary);
        EXPECT_EQ(secondaries, 1);
        EXPECT_EQ(singles, 1); // a secondary press is not within the primary-only mask
    }

    TEST(gesture_pipeline_test, synthetic_pan_drives_the_state_machine_with_gesture_ids)
    {
        test_view view;
        view.set_handler(std::make_shared<maui::core::button_handler>());

        auto pan = std::make_shared<pan_gesture_recognizer>();
        view.gesture_recognizers().add(pan);

        std::vector<pan_updated_event_args> updates;
        pan->pan_updated.connect([&updates](const pan_updated_event_args& e) { updates.push_back(e); });

        auto& manager = view.gesture_manager();
        manager.synthetic_pan(gesture_status::started);
        manager.synthetic_pan(gesture_status::running, 5, 10);
        manager.synthetic_pan(gesture_status::completed);
        manager.synthetic_pan(gesture_status::started); // a NEW gesture gets the incremented id
        manager.synthetic_pan(gesture_status::canceled);

        ASSERT_EQ(updates.size(), 5U);
        EXPECT_EQ(updates[0].status_type, gesture_status::started);
        EXPECT_EQ(updates[1].status_type, gesture_status::running);
        EXPECT_EQ(updates[1].total_x, 5);
        EXPECT_EQ(updates[1].total_y, 10);
        EXPECT_EQ(updates[2].status_type, gesture_status::completed);
        EXPECT_EQ(updates[3].status_type, gesture_status::started);
        EXPECT_EQ(updates[4].status_type, gesture_status::canceled);
        // The same id all the way through one gesture; CurrentId.Increment() on completed/canceled.
        EXPECT_EQ(updates[0].gesture_id, updates[2].gesture_id);
        EXPECT_EQ(updates[3].gesture_id, updates[0].gesture_id + 1);
        EXPECT_EQ(updates[4].gesture_id, updates[3].gesture_id);
    }

    TEST(gesture_pipeline_test, synthetic_pan_honors_touch_points)
    {
        test_view view;
        view.set_handler(std::make_shared<maui::core::button_handler>());

        auto pan = std::make_shared<pan_gesture_recognizer>();
        pan->set_touch_points(2);
        view.gesture_recognizers().add(pan);

        int updates = 0;
        pan->pan_updated.connect([&updates](const pan_updated_event_args&) { ++updates; });

        view.gesture_manager().synthetic_pan(gesture_status::started, 0, 0, /*touch_points=*/1);
        EXPECT_EQ(updates, 0); // NumberOfTouches != TouchPoints → ignored
        view.gesture_manager().synthetic_pan(gesture_status::started, 0, 0, /*touch_points=*/2);
        EXPECT_EQ(updates, 1);
    }

    TEST(gesture_pipeline_test, synthetic_pinch_honors_is_pinching_guards)
    {
        test_view view;
        view.set_handler(std::make_shared<maui::core::button_handler>());

        auto pinch = std::make_shared<pinch_gesture_recognizer>();
        view.gesture_recognizers().add(pinch);

        std::vector<pinch_gesture_updated_event_args> updates;
        pinch->pinch_updated.connect([&updates](const pinch_gesture_updated_event_args& e) { updates.push_back(e); });

        auto& manager = view.gesture_manager();
        manager.synthetic_pinch(gesture_status::completed); // not pinching yet → guarded, no event
        EXPECT_TRUE(updates.empty());

        manager.synthetic_pinch(gesture_status::started, 1, point(0.5, 0.5));
        manager.synthetic_pinch(gesture_status::running, 1.5, point(0.5, 0.5));
        manager.synthetic_pinch(gesture_status::completed);
        manager.synthetic_pinch(gesture_status::canceled); // already ended → guarded

        ASSERT_EQ(updates.size(), 3U);
        EXPECT_EQ(updates[0].status, gesture_status::started);
        EXPECT_EQ(updates[0].scale_origin.x, 0.5);
        EXPECT_EQ(updates[1].status, gesture_status::running);
        EXPECT_EQ(updates[1].scale, 1.5);
        EXPECT_EQ(updates[2].status, gesture_status::completed);
        EXPECT_FALSE(pinch->is_pinching());
    }

    TEST(gesture_pipeline_test, synthetic_swipe_detects_against_threshold_and_direction)
    {
        test_view view;
        view.set_handler(std::make_shared<maui::core::button_handler>());

        auto swipe = std::make_shared<swipe_gesture_recognizer>();
        swipe->set_direction(swipe_direction::left | swipe_direction::right);
        view.gesture_recognizers().add(swipe);

        std::vector<swipe_direction> raised;
        swipe->swiped.connect([&raised](const swiped_event_args& e) { raised.push_back(e.direction); });

        view.gesture_manager().synthetic_swipe(-50, 0); // below the 100px threshold
        EXPECT_TRUE(raised.empty());

        view.gesture_manager().synthetic_swipe(-150, 0);
        ASSERT_EQ(raised.size(), 1U);
        EXPECT_EQ(raised[0], swipe_direction::left);

        view.gesture_manager().synthetic_swipe(0, -150); // vertical totals on a horizontal-only mask
        EXPECT_EQ(raised.size(), 1U);
    }

    TEST(gesture_pipeline_test, synthetic_pointer_routes_phases_and_masks_presses)
    {
        test_view view;
        view.set_handler(std::make_shared<maui::core::button_handler>());

        auto pointer = std::make_shared<pointer_gesture_recognizer>();
        view.gesture_recognizers().add(pointer);

        std::vector<std::string> raised;
        pointer->pointer_entered.connect([&raised](const pointer_event_args&) { raised.emplace_back("entered"); });
        pointer->pointer_moved.connect([&raised](const pointer_event_args&) { raised.emplace_back("moved"); });
        pointer->pointer_exited.connect([&raised](const pointer_event_args&) { raised.emplace_back("exited"); });
        pointer->pointer_pressed.connect([&raised](const pointer_event_args&) { raised.emplace_back("pressed"); });
        pointer->pointer_released.connect([&raised](const pointer_event_args&) { raised.emplace_back("released"); });

        auto& manager = view.gesture_manager();
        manager.synthetic_pointer(pointer_event_kind::entered, point(1, 1));
        manager.synthetic_pointer(pointer_event_kind::moved, point(2, 2));
        // A secondary press on a primary-mask recognizer is filtered (the iOS non-hover mask check)…
        manager.synthetic_pointer(pointer_event_kind::pressed, point(2, 2), buttons_mask::secondary);
        // …but hover phases pass regardless of the mask.
        manager.synthetic_pointer(pointer_event_kind::moved, point(3, 3), buttons_mask::secondary);
        manager.synthetic_pointer(pointer_event_kind::pressed, point(3, 3), buttons_mask::primary);
        manager.synthetic_pointer(pointer_event_kind::released, point(3, 3), buttons_mask::primary);
        manager.synthetic_pointer(pointer_event_kind::exited, point(9, 9));

        const std::vector<std::string> expected{"entered", "moved", "moved", "pressed", "released", "exited"};
        EXPECT_EQ(raised, expected);
    }

    // GesturePlatformManager.iOS.cs's pinch delta math (pinch_scale_delta), pinned headless so both
    // native bridges share a tested translation of the Changed-case arithmetic.
    TEST(gesture_pipeline_test, pinch_scale_delta_matches_the_ios_bridge_math)
    {
        // oldScale < scale → 1 + |scale - old| * starting
        EXPECT_DOUBLE_EQ(maui::controls::pinch_scale_delta(1.0, 1.25, 1.0), 1.25);
        // oldScale > scale → 1 - |scale - old| * starting
        EXPECT_DOUBLE_EQ(maui::controls::pinch_scale_delta(1.25, 1.0, 1.0), 0.75);
        // equal scales → 1 (no change)
        EXPECT_DOUBLE_EQ(maui::controls::pinch_scale_delta(1.5, 1.5, 2.0), 1.0);
        // the starting view scale multiplies the difference
        EXPECT_DOUBLE_EQ(maui::controls::pinch_scale_delta(1.0, 1.5, 2.0), 2.0);
    }
} // namespace
