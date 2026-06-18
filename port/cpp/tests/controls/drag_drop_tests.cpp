// Drag & drop tests (headless) — the W2-22 unit. Ports the C# behavioral oracles:
//   - DataPackageTests.cs            (Text/Image/Properties round-trip + the View snapshot independence)
//   - Gestures/DragGestureRecognizerTests.cs (CanDrag default + setters; SendDragStarting raising
//                                             drag_starting, the DragSource stamp, the Cancel/Handled
//                                             short-circuit, the GetStringValue text fallback, the
//                                             SendDropCompleted once-per-drag latch)
//   - Gestures/DropGestureRecognizerTests.cs (AllowDrop default + setters; SendDragOver / SendDrop
//                                             raising; the AllowDrop guard; the TrySetValue text
//                                             injection onto a target control; the Handled short-circuit)
// plus the port's collection → gesture_platform_manager → recognizer wiring for the two new recognizers
// (attachment diffing on collection changes; is_attached). The Command-fired assertions ARE now portable
// (U-CMD): the drag/drop commands run via i_command — NOTE the drag/drop ordering is `Command?.Execute(p)`
// with NO CanExecute gate (unlike Tap/Pointer), preserved + pinned below. The TimePicker/DatePicker text
// theories are the documented seam gap (drag_drop_data.hpp) and are omitted.

#include <any>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "maui/controls/button.hpp"
#include "maui/controls/check_box.hpp"
#include "maui/controls/command.hpp"
#include "maui/controls/data_package.hpp"
#include "maui/controls/data_package_operation.hpp"
#include "maui/controls/drag_drop_data.hpp"
#include "maui/controls/editor.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/file_image_source.hpp"
#include "maui/controls/gestures/drag_gesture_recognizer.hpp"
#include "maui/controls/gestures/drop_gesture_recognizer.hpp"
#include "maui/controls/image.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/radio_button.hpp"
#include "maui/controls/toggle_switch.hpp"
#include "maui/core/button_handler.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::check_box;
    using maui::controls::command;
    using maui::controls::data_package;
    using maui::controls::data_package_operation;
    using maui::controls::data_package_view;
    using maui::controls::drag_event_args;
    using maui::controls::drag_gesture_recognizer;
    using maui::controls::drag_starting_event_args;
    using maui::controls::drop_completed_event_args;
    using maui::controls::drop_event_args;
    using maui::controls::drop_gesture_recognizer;
    using maui::controls::editor;
    using maui::controls::entry;
    using maui::controls::image;
    using maui::controls::image_source;
    using maui::controls::label;
    using maui::controls::radio_button;
    using maui::controls::toggle_switch;

    // ---- DataPackageTests.cs ----

    TEST(data_package_test, property_setters)
    {
        // DataPackageTests.PropertySetters.
        data_package package;
        auto img = image_source::from_file("somefile.jpg");
        package.set_text("text");
        package.set_image(img);
        package.properties().set("key", std::string("value"));

        EXPECT_EQ(package.text(), "text");
        EXPECT_EQ(package.image(), img);
        EXPECT_EQ(std::any_cast<std::string>(package.properties().get("key")), "value");
    }

    TEST(data_package_test, data_package_view_getters)
    {
        // DataPackageTests.DataPackageViewGetters.
        data_package package;
        auto img = image_source::from_file("somefile.jpg");
        package.set_text("text");
        package.set_image(img);
        package.properties().set("key", std::string("value"));
        const data_package_view view = package.view();

        EXPECT_EQ(view.text(), "text");
        EXPECT_EQ(view.image(), img);
        EXPECT_EQ(std::any_cast<std::string>(view.properties().get("key")), "value");
    }

    TEST(data_package_test, data_package_view_getters_arent_tied_to_initial_data_package)
    {
        // DataPackageTests.DataPackageViewGettersArentTiedToInitialDataPackage — the View is a SNAPSHOT.
        data_package package;
        auto img = image_source::from_file("somefile.jpg");
        package.set_text("text");
        package.set_image(img);
        package.properties().set("key", std::string("value"));
        const data_package_view view = package.view();

        // Mutate the originating package AFTER taking the view.
        package.set_text("fail");
        package.set_image(image_source::from_file("differentfile.jpg"));
        package.properties().set("key", std::string("fail"));

        EXPECT_EQ(view.text(), "text");
        EXPECT_EQ(view.image(), img);
        EXPECT_EQ(std::any_cast<std::string>(view.properties().get("key")), "value");
    }

    TEST(data_package_test, internal_property_bag_is_separate)
    {
        // The PropertiesInternal bag (DragSource lives here) is distinct from the public Properties bag.
        data_package package;
        package.properties().set("pub", std::string("a"));
        package.properties_internal().set("int", std::string("b"));

        EXPECT_TRUE(package.properties().contains_key("pub"));
        EXPECT_FALSE(package.properties().contains_key("int"));
        EXPECT_TRUE(package.properties_internal().contains_key("int"));
        EXPECT_EQ(package.properties().count(), 1U);
        EXPECT_EQ(package.properties_internal().count(), 1U);
    }

    // ---- DragGestureRecognizerTests.cs ----

    TEST(drag_gesture_recognizer_test, can_drag_default_is_true)
    {
        // DragGestureRecognizer.CanDrag default true (DragGestureRecognizerTests.PropertySetters subset).
        drag_gesture_recognizer drag;
        EXPECT_TRUE(drag.can_drag());
        drag.set_can_drag(false);
        EXPECT_FALSE(drag.can_drag());
    }

    TEST(drag_gesture_recognizer_test, send_drag_starting_raises_event)
    {
        drag_gesture_recognizer drag;
        bool raised = false;
        drag.drag_starting.connect([&raised](drag_starting_event_args&) { raised = true; });

        label element;
        (void)drag.send_drag_starting(element);
        EXPECT_TRUE(raised);
    }

    TEST(drag_gesture_recognizer_test, user_specified_text_isnt_overwritten)
    {
        // DragGestureRecognizerTests.UserSpecifiedTextIsntOverwritten.
        drag_gesture_recognizer drag;
        label element;
        element.set_text("WRONG TEXT");
        drag.drag_starting.connect([](drag_starting_event_args& args) { args.data().set_text("Right Text"); });

        const drag_starting_event_args returned = drag.send_drag_starting(element);
        EXPECT_EQ(returned.data().text(), "Right Text");
    }

    TEST(drag_gesture_recognizer_test, user_specified_image_isnt_overwritten)
    {
        // DragGestureRecognizerTests.UserSpecifiedImageIsntOverwritten.
        drag_gesture_recognizer drag;
        image element;
        auto handler_image = image_source::from_file("yay.jpg");
        drag.drag_starting.connect(
            [&handler_image](drag_starting_event_args& args) { args.data().set_image(handler_image); });

        const drag_starting_event_args returned = drag.send_drag_starting(element);
        EXPECT_EQ(returned.data().image(), handler_image);
    }

    TEST(drag_gesture_recognizer_test, text_extracted_from_label_source)
    {
        // DragGestureRecognizerTests.TextPackageCorrectlyExtractedFromCompatibleElement (Label row): no
        // handler sets the text, so SendDragStarting fills it from the element's GetStringValue.
        drag_gesture_recognizer drag;
        label element;
        element.set_text("LabelTest");
        const drag_starting_event_args returned = drag.send_drag_starting(element);
        EXPECT_EQ(returned.data().text(), "LabelTest");
    }

    TEST(drag_gesture_recognizer_test, text_extracted_from_bool_control_source)
    {
        // Same theory, the Switch / RadioButton rows ("True").
        drag_gesture_recognizer drag;
        toggle_switch sw;
        sw.set_is_toggled(true);
        EXPECT_EQ(drag.send_drag_starting(sw).data().text(), "True");

        drag_gesture_recognizer drag2;
        radio_button rb;
        rb.set_is_checked(true);
        EXPECT_EQ(drag2.send_drag_starting(rb).data().text(), "True");
    }

    TEST(drag_gesture_recognizer_test, drag_source_stamped_into_internal_properties)
    {
        // SendDragStarting stamps the source element under "DragSource" in the INTERNAL bag (unless
        // Handled). Drives the drop-side fallback.
        drag_gesture_recognizer drag;
        label element;
        const drag_starting_event_args returned = drag.send_drag_starting(element);
        ASSERT_TRUE(returned.data().properties_internal().contains_key("DragSource"));
        const auto* stored = returned.data().properties_internal().try_get_value("DragSource");
        ASSERT_NE(stored, nullptr);
        EXPECT_EQ(std::any_cast<maui::controls::element*>(*stored), &element);
    }

    TEST(drag_gesture_recognizer_test, handled_short_circuits_text_extraction)
    {
        // DragGestureRecognizerTests.HandledTest: when the handler marks args Handled, the text fallback
        // does NOT run (and no DragSource is stamped) — the element's text is not copied in.
        drag_gesture_recognizer drag;
        label element;
        element.set_text("test String");
        drag.drag_starting.connect([](drag_starting_event_args& args) { args.set_handled(true); });

        const drag_starting_event_args returned = drag.send_drag_starting(element);
        EXPECT_NE(returned.data().text(), "test String");
        EXPECT_FALSE(returned.data().properties_internal().contains_key("DragSource"));
    }

    TEST(drag_gesture_recognizer_test, cancel_short_circuits_text_extraction)
    {
        // Cancel (unlike Handled) still stamps DragSource but skips the text fallback.
        drag_gesture_recognizer drag;
        label element;
        element.set_text("some text");
        drag.drag_starting.connect([](drag_starting_event_args& args) { args.set_cancel(true); });

        const drag_starting_event_args returned = drag.send_drag_starting(element);
        EXPECT_FALSE(returned.data().text().has_value());
        EXPECT_TRUE(returned.data().properties_internal().contains_key("DragSource"));
    }

    TEST(drag_gesture_recognizer_test, drop_completed_fires_once)
    {
        // DragGestureRecognizerTests.DropCompletedCommandFiresOnce, recast onto the event: the
        // _isDragActive latch lets only the first completion of a drag through.
        drag_gesture_recognizer drag;
        int count = 0;
        drag.drop_completed.connect([&count](const drop_completed_event_args&) { ++count; });

        label element;
        (void)drag.send_drag_starting(element); // arms the latch
        drag.send_drop_completed(drop_completed_event_args{});
        drag.send_drop_completed(drop_completed_event_args{});
        drag.send_drop_completed(drop_completed_event_args{});
        EXPECT_EQ(count, 1);
    }

    TEST(drag_gesture_recognizer_test, drop_completed_ignored_without_active_drag)
    {
        // A completion before any drag started is ignored (the latch starts disarmed).
        drag_gesture_recognizer drag;
        int count = 0;
        drag.drop_completed.connect([&count](const drop_completed_event_args&) { ++count; });
        drag.send_drop_completed(drop_completed_event_args{});
        EXPECT_EQ(count, 0);
    }

    // (U-CMD) SendDragStarting runs DragStartingCommand (with its parameter) BEFORE raising drag_starting,
    // and — per C# — with NO CanExecute gate (the command runs whenever set; a false predicate is ignored).
    TEST(drag_gesture_recognizer_test, send_drag_starting_runs_command_before_event_no_can_execute_gate)
    {
        drag_gesture_recognizer drag;
        std::vector<std::string> order;
        std::any seen;
        drag.set_drag_starting_command(std::make_shared<command>(
            std::function<void(const std::any&)>{[&order, &seen](const std::any& o) {
                seen = o;
                order.emplace_back("command");
            }},
            std::function<bool(const std::any&)>{[](const std::any&) { return false; }})); // gate ignored on drag side
        drag.set_drag_starting_command_parameter(std::any{std::string("payload")});
        drag.drag_starting.connect([&order](drag_starting_event_args&) { order.emplace_back("event"); });

        label element;
        (void)drag.send_drag_starting(element);

        const std::vector<std::string> expected{"command", "event"};
        EXPECT_EQ(order, expected); // command ran (despite can_execute==false) then the event
        ASSERT_TRUE(seen.has_value());
        EXPECT_EQ(std::any_cast<std::string>(seen), "payload");
    }

    // (U-CMD) SendDropCompleted runs DropCompletedCommand once per drag (latch-gated like the event).
    TEST(drag_gesture_recognizer_test, send_drop_completed_runs_command_once_per_drag)
    {
        drag_gesture_recognizer drag;
        int runs = 0;
        drag.set_drop_completed_command(std::make_shared<command>([&runs](const std::any&) { ++runs; }));

        label element;
        (void)drag.send_drag_starting(element); // arms the latch
        drag.send_drop_completed(drop_completed_event_args{});
        drag.send_drop_completed(drop_completed_event_args{}); // latch disarmed → ignored
        EXPECT_EQ(runs, 1);
    }

    // ---- DropGestureRecognizerTests.cs ----

    TEST(drop_gesture_recognizer_test, allow_drop_default_is_true)
    {
        // DropGestureRecognizer.AllowDrop default true.
        drop_gesture_recognizer drop;
        EXPECT_TRUE(drop.allow_drop());
        drop.set_allow_drop(false);
        EXPECT_FALSE(drop.allow_drop());
    }

    TEST(drop_gesture_recognizer_test, send_drag_over_raises_and_reads_accepted_operation)
    {
        // SendDragOver raises drag_over; the handler can set AcceptedOperation (default Copy).
        drop_gesture_recognizer drop;
        data_package package;
        drag_event_args args(package);
        EXPECT_EQ(args.accepted_operation(), data_package_operation::copy);

        bool raised = false;
        drop.drag_over.connect([&raised](drag_event_args& e) {
            raised = true;
            e.set_accepted_operation(data_package_operation::none);
        });
        drop.send_drag_over(args);
        EXPECT_TRUE(raised);
        EXPECT_EQ(args.accepted_operation(), data_package_operation::none);
    }

    TEST(drop_gesture_recognizer_test, send_drop_raises_event)
    {
        drop_gesture_recognizer drop;
        bool raised = false;
        drop.drop.connect([&raised](drop_event_args&) { raised = true; });

        data_package package;
        drop_event_args args(data_package_view(package.clone()));
        drop.send_drop(args);
        EXPECT_TRUE(raised);
    }

    // (U-CMD) SendDragOver / SendDragLeave run their command BEFORE the event, with NO CanExecute gate.
    TEST(drop_gesture_recognizer_test, send_drag_over_and_leave_run_commands_before_events)
    {
        drop_gesture_recognizer drop;
        std::vector<std::string> order;
        // can_execute == false is intentionally ignored on the drop side (command still runs).
        drop.set_drag_over_command(
            std::make_shared<command>(std::function<void()>{[&order] { order.emplace_back("over_cmd"); }},
                                      std::function<bool()>{[] { return false; }}));
        drop.set_drag_leave_command(
            std::make_shared<command>([&order](const std::any&) { order.emplace_back("leave_cmd"); }));
        drop.drag_over.connect([&order](drag_event_args&) { order.emplace_back("over_evt"); });
        drop.drag_leave.connect([&order](drag_event_args&) { order.emplace_back("leave_evt"); });

        data_package package;
        drag_event_args args(package);
        drop.send_drag_over(args);
        drop.send_drag_leave(args);

        const std::vector<std::string> expected{"over_cmd", "over_evt", "leave_cmd", "leave_evt"};
        EXPECT_EQ(order, expected);
    }

    // (U-CMD) SendDrop runs DropCommand (with its parameter) BEFORE the event; suppressed with the event
    // when !AllowDrop (the AllowDrop guard precedes the command, matching C#).
    TEST(drop_gesture_recognizer_test, send_drop_runs_command_before_event_and_respects_allow_drop)
    {
        drop_gesture_recognizer drop;
        std::vector<std::string> order;
        std::any seen;
        drop.set_drop_command(std::make_shared<command>([&order, &seen](const std::any& o) {
            seen = o;
            order.emplace_back("command");
        }));
        drop.set_drop_command_parameter(std::any{std::string("dropped")});
        drop.drop.connect([&order](drop_event_args&) { order.emplace_back("event"); });

        data_package package;
        drop_event_args args(data_package_view(package.clone()));
        drop.send_drop(args);
        const std::vector<std::string> expected{"command", "event"};
        EXPECT_EQ(order, expected);
        ASSERT_TRUE(seen.has_value());
        EXPECT_EQ(std::any_cast<std::string>(seen), "dropped");

        // !AllowDrop short-circuits before the command runs.
        drop_gesture_recognizer disallowed;
        disallowed.set_allow_drop(false);
        bool ran = false;
        disallowed.set_drop_command(std::make_shared<command>([&ran](const std::any&) { ran = true; }));
        data_package package2;
        drop_event_args args2(data_package_view(package2.clone()));
        disallowed.send_drop(args2);
        EXPECT_FALSE(ran);
    }

    TEST(drop_gesture_recognizer_test, send_drop_is_noop_when_disallowed)
    {
        // DropGestureRecognizer.SendDrop early-returns when !AllowDrop (no event, no injection).
        drop_gesture_recognizer drop;
        drop.set_allow_drop(false);
        bool raised = false;
        drop.drop.connect([&raised](drop_event_args&) { raised = true; });

        data_package package;
        package.set_text("payload");
        drop_event_args args(data_package_view(package.clone()));
        label target;
        drop.send_drop(args, &target);
        EXPECT_FALSE(raised);
        EXPECT_TRUE(target.text().empty());
    }

    TEST(drop_gesture_recognizer_test, text_injected_onto_label_target)
    {
        // DropGestureRecognizerTests.TextPackageCorrectlySetsOnCompatibleTarget (Label row): SendDrop sets
        // the dropped text onto the target control via TrySetValue.
        drop_gesture_recognizer drop;
        data_package package;
        package.set_text("LabelTest");
        drop_event_args args(data_package_view(package.clone()));
        label target;
        drop.send_drop(args, &target);
        EXPECT_EQ(target.text(), "LabelTest");
    }

    TEST(drop_gesture_recognizer_test, text_injected_onto_bool_target)
    {
        // Switch / RadioButton rows ("True").
        drop_gesture_recognizer drop;
        data_package sw_package;
        sw_package.set_text("True");
        drop_event_args sw_args(data_package_view(sw_package.clone()));
        toggle_switch sw;
        drop.send_drop(sw_args, &sw);
        EXPECT_TRUE(sw.is_toggled());

        drop_gesture_recognizer drop2;
        data_package rb_package;
        rb_package.set_text("True");
        drop_event_args rb_args(data_package_view(rb_package.clone()));
        radio_button rb;
        drop2.send_drop(rb_args, &rb);
        EXPECT_TRUE(rb.is_checked());
    }

    TEST(drop_gesture_recognizer_test, image_injected_onto_image_target)
    {
        // The image-target injection (Parent is Image → image.Source).
        drop_gesture_recognizer drop;
        data_package package;
        auto dropped = image_source::from_file("dropped.png");
        package.set_image(dropped);
        drop_event_args args(data_package_view(package.clone()));
        image target;
        drop.send_drop(args, &target);
        EXPECT_EQ(target.source(), dropped.get());
    }

    TEST(drop_gesture_recognizer_test, handled_short_circuits_injection)
    {
        // DropGestureRecognizerTests.HandledTest: Handled suppresses the default text/image processing.
        drop_gesture_recognizer drop;
        data_package package;
        package.set_text("test String");
        drop_event_args args(data_package_view(package.clone()));
        args.set_handled(true);
        label target;
        target.set_text("Text Shouldn't change");
        drop.send_drop(args, &target);
        EXPECT_EQ(target.text(), "Text Shouldn't change");
    }

    TEST(drop_gesture_recognizer_test, drag_source_text_fallback)
    {
        // SendDrop with no explicit Data.Text but a "DragSource" in the package: the dropped text falls
        // back to the source element's GetStringValue (the C# DragSource lookup path).
        label source;
        source.set_text("FromSource");

        data_package package;
        package.properties_internal().set("DragSource", std::any(static_cast<maui::controls::element*>(&source)));
        drop_event_args args(data_package_view(package.clone()));

        drop_gesture_recognizer drop;
        label target;
        drop.send_drop(args, &target);
        EXPECT_EQ(target.text(), "FromSource");
    }

    // ---- the cross-control seam (drag_drop_data.hpp) directly ----

    TEST(drag_drop_data_test, get_string_value_covers_text_and_bool_controls)
    {
        label lbl;
        lbl.set_text("L");
        entry ent;
        ent.set_text("E");
        editor edt;
        edt.set_text("D");
        check_box chk;
        chk.set_is_checked(true);

        EXPECT_EQ(maui::controls::get_string_value(lbl), "L");
        EXPECT_EQ(maui::controls::get_string_value(ent), "E");
        EXPECT_EQ(maui::controls::get_string_value(edt), "D");
        EXPECT_EQ(maui::controls::get_string_value(chk), "True");
    }

    TEST(drag_drop_data_test, try_set_string_value_bool_parse_guard)
    {
        // Switch only accepts a parseable bool (C#'s bool.TryParse guard); a non-bool string is rejected.
        toggle_switch sw;
        EXPECT_FALSE(maui::controls::try_set_string_value(sw, "not a bool"));
        EXPECT_FALSE(sw.is_toggled());
        EXPECT_TRUE(maui::controls::try_set_string_value(sw, "true")); // case-insensitive
        EXPECT_TRUE(sw.is_toggled());
    }

    TEST(drag_drop_data_test, check_box_is_extract_only_not_inject)
    {
        // CheckBox is in GetStringValue but NOT Element.TrySetValue (C# parity) — injection is rejected.
        check_box chk;
        EXPECT_FALSE(maui::controls::try_set_string_value(chk, "True"));
    }

    // ---- collection → gesture_platform_manager → recognizer wiring (the W1-12 pipeline, drag/drop) ----

    TEST(drag_drop_wiring_test, recognizers_attach_on_handler_then_diff_on_collection_change)
    {
        maui::controls::button view;
        auto drag = std::make_shared<drag_gesture_recognizer>();
        auto drop = std::make_shared<drop_gesture_recognizer>();
        view.gesture_recognizers().add(drag);
        view.gesture_recognizers().add(drop);
        EXPECT_EQ(view.gesture_manager().attached_count(), 0U); // no handler yet

        view.set_handler(std::make_shared<maui::core::button_handler>());
        EXPECT_EQ(view.gesture_manager().attached_count(), 2U); // LoadRecognizers on attach
        EXPECT_TRUE(view.gesture_manager().is_attached(*drag));
        EXPECT_TRUE(view.gesture_manager().is_attached(*drop));

        view.gesture_recognizers().remove(drag); // CollectionChanged → re-diff
        EXPECT_EQ(view.gesture_manager().attached_count(), 1U);
        EXPECT_FALSE(view.gesture_manager().is_attached(*drag));
        EXPECT_TRUE(view.gesture_manager().is_attached(*drop));
    }

    TEST(drag_drop_wiring_test, native_registration_matches_backend)
    {
        // The native drag/drop registration accessors: headless owns no native registration (the
        // recognizer still joins attached_ — proven above); apple/ios install the real
        // NSDraggingDestination / UIDragInteraction|UIDropInteraction (the deeper native-install assertions
        // live in gesture_apple_tests.mm / gesture_ios_tests.mm; this cross-platform case pins the
        // backend-correct baseline through the shared accessors).
        maui::controls::button view;
        auto drag = std::make_shared<drag_gesture_recognizer>();
        auto drop = std::make_shared<drop_gesture_recognizer>();
        view.gesture_recognizers().add(drag);
        view.gesture_recognizers().add(drop);
        view.set_handler(std::make_shared<maui::core::button_handler>());

#if defined(MAUI_PLATFORM_APPLE) || defined(MAUI_PLATFORM_IOS)
        EXPECT_TRUE(view.gesture_manager().native_registered_drag_source(*drag));
        EXPECT_TRUE(view.gesture_manager().native_registered_drop_target(*drop));
#else
        EXPECT_FALSE(view.gesture_manager().native_registered_drag_source(*drag));
        EXPECT_FALSE(view.gesture_manager().native_registered_drop_target(*drop));
#endif
        // The drag-source accessor is false for a drop recognizer and vice-versa, on every backend.
        EXPECT_FALSE(view.gesture_manager().native_registered_drop_target(*drag));
        EXPECT_FALSE(view.gesture_manager().native_registered_drag_source(*drop));
    }
} // namespace
