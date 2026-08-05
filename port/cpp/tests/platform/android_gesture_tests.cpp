// Android gesture seam tests — ON the emulator inside the app_process widget test host
// (tools/android-testhost-run.sh): the REAL maui::controls::box_view + its recognizer collection drive
// a REAL android.view.View through the gesture platform partial
// (src/platform/android/gesture_platform_manager.cpp) and the host-provided
// dev.mauicpp.MauiGestureBridge (src/platform/android/java/MauiGestureBridge.java).
//
// WHY THIS TARGET AND NOT maui_controls_tests: the android maui_controls_tests binary runs through
// tools/android-emu-run.sh (plain `adb shell` — NO ART runtime, no JavaVM), where the whole native
// gesture channel degrades to the cross-platform bookkeeping by design. Only the app_process widget
// test host has the JVM + Context this seam needs. The backend-agnostic recognizer semantics stay
// covered by tests/controls/gesture_recognizer_tests.cpp, which DOES run on the android preset.
//
// HOW THE GESTURE IS DRIVEN — the port's answer to "no touch synthesis in a spawned test process":
// a fabricated android.view.MotionEvent is pushed through View.dispatchTouchEvent, which hands it to
// the OnTouchListener the subscription sync installed. From there the path is the real one end to end:
//   MauiGestureBridge.onTouch -> android.view.GestureDetector -> the bridge's listener callbacks
//   -> nativeOnDown / nativeOnSingleTapUp / nativeOnScroll (RegisterNatives) -> the fan-out
//   -> tap/pan recognizer send_* -> the port event.
// Nothing is stubbed: a failure here means the C# GesturePlatformManager.Android shape is not wired.
//
// box_view is the host control because its android partial builds a plain View(Context) (no TextView
// base — see android_box_view_tests.cpp for why editor/switch/check_box cannot construct here).

#include <functional>
#include <memory>

#include <gtest/gtest.h>
#include <jni.h>

#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "maui/controls/box_view.hpp"
#include "maui/controls/gestures/drag_gesture_recognizer.hpp"
#include "maui/controls/gestures/drop_gesture_recognizer.hpp"
#include "maui/controls/gestures/pan_gesture_recognizer.hpp"
#include "maui/controls/gestures/tap_gesture_recognizer.hpp"
#include "maui/core/gesture_status.hpp"
#include "maui/core/shape_view_handler.hpp"
#include "testhost/test_host.hpp"

namespace
{
    using maui::controls::box_view;
    using maui::controls::drag_gesture_recognizer;
    using maui::controls::drop_gesture_recognizer;
    using maui::controls::pan_gesture_recognizer;
    using maui::controls::tap_gesture_recognizer;
    using maui::core::shape_view_handler;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;

    constexpr const char* k_view_class = "android/view/View";
    constexpr const char* k_motion_event_class = "android/view/MotionEvent";

    // android.view.MotionEvent action constants.
    constexpr jint k_action_down = 0;
    constexpr jint k_action_up = 1;
    constexpr jint k_action_move = 2;

    bool pending_exception_cleared(JNIEnv* env, const char* stage)
    {
        if (env->ExceptionCheck() == JNI_FALSE)
        {
            return false;
        }
        ADD_FAILURE() << "pending Java exception at " << stage;
        env->ExceptionDescribe();
        env->ExceptionClear();
        return true;
    }

    // A box_view with its handler attached, so it owns a REAL android.view.View reachable through
    // native_view(). The recognizer collection is populated AFTER the handler is set, which is exactly
    // the load_recognizers() path the sync hangs off.
    struct attached_box
    {
        box_view control;
        std::shared_ptr<shape_view_handler> handler = std::make_shared<shape_view_handler>();

        attached_box()
        {
            control.set_handler(handler);
        }

        [[nodiscard]] jobject native() const
        {
            return static_cast<jobject>(handler->native_view());
        }
    };

    // Push one fabricated MotionEvent through View.dispatchTouchEvent. `when` is the event timestamp in
    // millis — GestureDetector only ever compares timestamps to each other, so a monotonic counter is a
    // faithful clock here.
    void dispatch_touch(jobject view, jint action, jfloat x, jfloat y, jlong down_time, jlong when)
    {
        const scoped_env env;
        ASSERT_TRUE(static_cast<bool>(env)) << "no JavaVM (not running in the widget test host)";
        auto& cache = default_jni_cache();
        jclass motion_class = cache.find_class(env.get(), k_motion_event_class);
        jmethodID obtain =
            cache.static_method(env.get(), k_motion_event_class, "obtain", "(JJIFFI)Landroid/view/MotionEvent;");
        jmethodID recycle = cache.method(env.get(), k_motion_event_class, "recycle", "()V");
        jmethodID dispatch =
            cache.method(env.get(), k_view_class, "dispatchTouchEvent", "(Landroid/view/MotionEvent;)Z");
        ASSERT_NE(motion_class, nullptr);
        ASSERT_NE(obtain, nullptr);
        ASSERT_NE(recycle, nullptr);
        ASSERT_NE(dispatch, nullptr);

        const local_ref<jobject> event{
            env.get(), env->CallStaticObjectMethod(motion_class, obtain, down_time, when, action, x, y, jint{0})};
        ASSERT_FALSE(pending_exception_cleared(env.get(), "MotionEvent.obtain"));
        ASSERT_TRUE(static_cast<bool>(event));
        env->CallBooleanMethod(view, dispatch, event.get());
        ASSERT_FALSE(pending_exception_cleared(env.get(), "View.dispatchTouchEvent"));
        env->CallVoidMethod(event.get(), recycle);
        ASSERT_FALSE(pending_exception_cleared(env.get(), "MotionEvent.recycle"));
    }

    // A tap recognizer's Tapped must reach the port from a REAL MotionEvent stream: the sync installed
    // the OnTouchListener, GestureDetector recognized the tap, and TapGestureHandler.OnTap's fan-out ran.
    TEST(AndroidGestureSeam, DispatchedTouchRaisesTapped)
    {
        ASSERT_NE(maui::platform::android::testhost::host_context(), nullptr);
        attached_box box;
        ASSERT_NE(box.native(), nullptr) << "box_view built no native View";

        auto tap = std::make_shared<tap_gesture_recognizer>();
        int tapped = 0;
        tap->tapped.connect([&tapped](const maui::controls::tapped_event_args&) { ++tapped; });
        box.control.gesture_recognizers().add(tap);

        dispatch_touch(box.native(), k_action_down, 10, 20, 1000, 1000);
        dispatch_touch(box.native(), k_action_up, 10, 20, 1000, 1050);

        EXPECT_EQ(tapped, 1);
    }

    // Removing the recognizer must actually uninstall the touch channel: a second stream after the
    // removal raises nothing (the native_detach -> sync_subscriptions path).
    TEST(AndroidGestureSeam, RemovedRecognizerStopsReceivingTouches)
    {
        ASSERT_NE(maui::platform::android::testhost::host_context(), nullptr);
        attached_box box;
        ASSERT_NE(box.native(), nullptr);

        auto tap = std::make_shared<tap_gesture_recognizer>();
        int tapped = 0;
        tap->tapped.connect([&tapped](const maui::controls::tapped_event_args&) { ++tapped; });
        box.control.gesture_recognizers().add(tap);
        dispatch_touch(box.native(), k_action_down, 5, 5, 2000, 2000);
        dispatch_touch(box.native(), k_action_up, 5, 5, 2000, 2050);
        ASSERT_EQ(tapped, 1);

        box.control.gesture_recognizers().remove(tap);
        dispatch_touch(box.native(), k_action_down, 5, 5, 3000, 3000);
        dispatch_touch(box.native(), k_action_up, 5, 5, 3000, 3050);
        EXPECT_EQ(tapped, 1);
    }

    // RE-ENTRANCY. A Tapped handler is user code, and user code may mutate the very collection the
    // fan-out is walking — the classic one-shot handler that removes its own recognizer. The fan-out
    // must therefore walk a defensive COPY, exactly as C# does (EnumerableExtensions.GetGesturesFor,
    // src/Controls/src/Core/EnumerableExtensions.cs:48-62: "The method makes a defensive copy of the
    // gestures"). THREE recognizers, and the FIRST is the one that removes itself: walking the live
    // table instead shifts the tail left under the cached end iterator, which skips the recognizer that
    // moved into the current slot and then re-reads the vacated one past the new size.
    TEST(AndroidGestureSeam, HandlerRemovingItsOwnRecognizerMidDispatchIsSafe)
    {
        ASSERT_NE(maui::platform::android::testhost::host_context(), nullptr);
        attached_box box;
        ASSERT_NE(box.native(), nullptr);

        auto first = std::make_shared<tap_gesture_recognizer>();
        auto second = std::make_shared<tap_gesture_recognizer>();
        auto third = std::make_shared<tap_gesture_recognizer>();
        int first_taps = 0;
        int second_taps = 0;
        int third_taps = 0;
        first->tapped.connect([&](const maui::controls::tapped_event_args&) {
            ++first_taps;
            box.control.gesture_recognizers().remove(first); // detaches + re-syncs FROM INSIDE the sweep
        });
        second->tapped.connect([&second_taps](const maui::controls::tapped_event_args&) { ++second_taps; });
        third->tapped.connect([&third_taps](const maui::controls::tapped_event_args&) { ++third_taps; });
        box.control.gesture_recognizers().add(first);
        box.control.gesture_recognizers().add(second);
        box.control.gesture_recognizers().add(third);

        dispatch_touch(box.native(), k_action_down, 7, 7, 6000, 6000);
        dispatch_touch(box.native(), k_action_up, 7, 7, 6000, 6050);
        EXPECT_EQ(first_taps, 1);
        EXPECT_EQ(second_taps, 1) << "the defensive copy must still reach the recognizers behind the mutation";
        EXPECT_EQ(third_taps, 1) << "and must reach each of them exactly once";

        // Well past the double-tap timeout, so this is a fresh single tap: the removal really took.
        dispatch_touch(box.native(), k_action_down, 7, 7, 8000, 8000);
        dispatch_touch(box.native(), k_action_up, 7, 7, 8000, 8050);
        EXPECT_EQ(first_taps, 1);
        EXPECT_EQ(second_taps, 2);
        EXPECT_EQ(third_taps, 2);
    }

    // GesturePlatformManager.OnTouchEvent :64-91 — a disabled or input-transparent element refuses the
    // whole touch stream (:71-74, off Element.IsEnabled / Element.InputTransparent, :407-425). Neither
    // property has a plain-android.view.View mapping in the port (box_view_handler.cpp:43), so this gate
    // is the ONLY thing standing between a disabled view and its gestures.
    TEST(AndroidGestureSeam, DisabledOrInputTransparentViewRefusesTheTouchStream)
    {
        ASSERT_NE(maui::platform::android::testhost::host_context(), nullptr);
        attached_box box;
        ASSERT_NE(box.native(), nullptr);

        auto tap = std::make_shared<tap_gesture_recognizer>();
        int tapped = 0;
        tap->tapped.connect([&tapped](const maui::controls::tapped_event_args&) { ++tapped; });
        box.control.gesture_recognizers().add(tap);

        box.control.set_is_enabled(false);
        dispatch_touch(box.native(), k_action_down, 4, 4, 12000, 12000);
        dispatch_touch(box.native(), k_action_up, 4, 4, 12000, 12050);
        EXPECT_EQ(tapped, 0) << "a disabled element must not receive gestures";

        box.control.set_is_enabled(true);
        box.control.set_input_transparent(true);
        dispatch_touch(box.native(), k_action_down, 4, 4, 14000, 14000);
        dispatch_touch(box.native(), k_action_up, 4, 4, 14000, 14050);
        EXPECT_EQ(tapped, 0) << "an input-transparent element must not receive gestures";

        // ...and the gate is a gate, not a latch: clearing both restores the stream.
        box.control.set_input_transparent(false);
        dispatch_touch(box.native(), k_action_down, 4, 4, 16000, 16000);
        dispatch_touch(box.native(), k_action_up, 4, 4, 16000, 16050);
        EXPECT_EQ(tapped, 1);
    }

    // The harsher half of the same hazard: the handler tears the whole gesture channel down
    // (set_handler(nullptr) -> GestureManager.DisconnectGestures -> native_detach_all), freeing the
    // manager's grip on the peer while a fan-out is standing on it. The callback holds its own strong
    // ref, so the storage survives the unwind; the sweep stops instead of sending into the dead view.
    TEST(AndroidGestureSeam, HandlerTearingDownTheViewMidDispatchIsSafe)
    {
        ASSERT_NE(maui::platform::android::testhost::host_context(), nullptr);
        attached_box box;
        jobject native = box.native();
        ASSERT_NE(native, nullptr);

        auto first = std::make_shared<tap_gesture_recognizer>();
        auto second = std::make_shared<tap_gesture_recognizer>();
        int first_taps = 0;
        int second_taps = 0;
        first->tapped.connect([&](const maui::controls::tapped_event_args&) {
            ++first_taps;
            box.control.set_handler(nullptr); // the whole platform seam goes away mid-sweep
        });
        second->tapped.connect([&second_taps](const maui::controls::tapped_event_args&) { ++second_taps; });
        box.control.gesture_recognizers().add(first);
        box.control.gesture_recognizers().add(second);

        dispatch_touch(native, k_action_down, 3, 3, 10000, 10000);
        dispatch_touch(native, k_action_up, 3, 3, 10000, 10050);

        EXPECT_EQ(first_taps, 1);
        EXPECT_EQ(second_taps, 0) << "no send may run against a view that the previous handler destroyed";
        EXPECT_EQ(box.handler->native_view(), nullptr);
    }

    // A drag past the touch slop must run the whole InnerGestureListener scroll machine:
    // OnScroll -> OnPanStarted + OnPan, then ACTION_UP -> EndScrolling -> OnPanComplete.
    TEST(AndroidGestureSeam, DispatchedDragRaisesPanStartedRunningCompleted)
    {
        ASSERT_NE(maui::platform::android::testhost::host_context(), nullptr);
        attached_box box;
        ASSERT_NE(box.native(), nullptr);

        auto pan = std::make_shared<pan_gesture_recognizer>();
        int started = 0;
        int running = 0;
        int completed = 0;
        pan->pan_updated.connect([&](const maui::controls::pan_updated_event_args& args) {
            switch (args.status_type)
            {
                case maui::core::gesture_status::started:
                    ++started;
                    break;
                case maui::core::gesture_status::running:
                    ++running;
                    break;
                case maui::core::gesture_status::completed:
                    ++completed;
                    break;
                case maui::core::gesture_status::canceled:
                    ADD_FAILURE() << "android never sends PanCanceled (PanGestureHandler.cs:50-65)";
                    break;
            }
        });
        box.control.gesture_recognizers().add(pan);

        dispatch_touch(box.native(), k_action_down, 0, 0, 4000, 4000);
        // Well past any device's touch slop, so GestureDetector reports a scroll.
        dispatch_touch(box.native(), k_action_move, 0, 400, 4000, 4020);
        dispatch_touch(box.native(), k_action_move, 0, 800, 4000, 4040);
        dispatch_touch(box.native(), k_action_up, 0, 800, 4000, 4060);

        EXPECT_EQ(started, 1);
        EXPECT_GE(running, 1);
        EXPECT_EQ(completed, 1);
    }

    // LONG PRESS NEEDS A PUMPED LOOPER. Unlike tap/scroll — which GestureDetector decides synchronously
    // inside onTouchEvent — the long press is a DELAYED Handler message (GestureDetector.GestureHandler,
    // LONG_PRESS at ~500ms). testhost/Bootstrap.java prepares the main Looper but never loops it (the
    // gtest suite IS that thread), so a fabricated down/up alone never reaches OnLongPress — measured:
    // 0 drag_starting raises after a real 900ms wait. Pump it by hand instead: MessageQueue.next()
    // blocks until the next message is due and Handler.dispatchMessage delivers it. Bounded by
    // max_messages, because next() would block forever once the queue drains.
    void pump_looper(int max_messages, const std::function<bool()>& done)
    {
        const scoped_env env;
        ASSERT_TRUE(static_cast<bool>(env));
        auto& cache = default_jni_cache();
        jclass looper_class = cache.find_class(env.get(), "android/os/Looper");
        jmethodID my_queue =
            cache.static_method(env.get(), "android/os/Looper", "myQueue", "()Landroid/os/MessageQueue;");
        // MessageQueue.next() and Message.target are package-private; JNI resolves them by name, and this
        // app_process host runs without an application package so hidden-API enforcement does not apply
        // (the same reason Bootstrap.java can reflect ActivityThread.systemMain).
        jmethodID next = cache.method(env.get(), "android/os/MessageQueue", "next", "()Landroid/os/Message;");
        jfieldID target = cache.field(env.get(), "android/os/Message", "target", "Landroid/os/Handler;");
        jmethodID dispatch_message =
            cache.method(env.get(), "android/os/Handler", "dispatchMessage", "(Landroid/os/Message;)V");
        ASSERT_NE(looper_class, nullptr);
        ASSERT_NE(my_queue, nullptr);
        ASSERT_NE(next, nullptr);
        ASSERT_NE(target, nullptr);
        ASSERT_NE(dispatch_message, nullptr);

        const local_ref<jobject> queue{env.get(), env->CallStaticObjectMethod(looper_class, my_queue)};
        ASSERT_FALSE(pending_exception_cleared(env.get(), "Looper.myQueue"));
        ASSERT_TRUE(static_cast<bool>(queue));

        for (int i = 0; i < max_messages && !done(); ++i)
        {
            const local_ref<jobject> message{env.get(), env->CallObjectMethod(queue.get(), next)};
            ASSERT_FALSE(pending_exception_cleared(env.get(), "MessageQueue.next"));
            if (!message)
            {
                return;
            }
            const local_ref<jobject> handler{env.get(), env->GetObjectField(message.get(), target)};
            if (handler)
            {
                env->CallVoidMethod(handler.get(), dispatch_message, message.get());
                (void)pending_exception_cleared(env.get(), "Handler.dispatchMessage");
            }
        }
    }

    // RE-ENTRANCY, the drag arm. send_drag_starting is user code like every other send_*, but
    // drag_on_long_press keeps working after it INSIDE the same iteration — it parks the data package on
    // the peer and calls MauiGestureBridge.startDrag on state.view. A DragStarting handler that tears the
    // view down must stop the sweep right there, the same per-element `dead` re-check the other seven
    // fan-outs make (for_each_of, gesture_platform_manager.cpp:355-362), which is the port's spelling of
    // the GC-rooted view C# relies on.
    //
    // WHAT THIS TEST IS AND IS NOT. It is the REACHABILITY + no-crash regression for that path: it pins
    // that a long press really reaches drag_on_long_press here (the ASSERT on drag_starts), and that
    // freeing the element from inside send_drag_starting unwinds cleanly through native_detach_all. It is
    // NOT a mutation test — measured on this emulator, it passes with either guard reverted, because the
    // freed element is read out of an unpoisoned heap and because the only side effect the missing `dead`
    // re-check adds is a View.startDragAndDrop against a window-less test view, which Android no-ops. The
    // mutation-provable half of the same defect lives in tests/controls/drag_drop_tests.cpp
    // (handler_destroying_the_source_mid_send_is_safe) under the asan-ubsan preset.
    TEST(AndroidGestureSeam, DragHandlerTearingDownTheViewMidDispatchIsSafe)
    {
        ASSERT_NE(maui::platform::android::testhost::host_context(), nullptr);
        // HEAP-owned, unlike the other cases: the handler here frees the ELEMENT, not just its handler,
        // which is what makes `state.sender` — the very reference send_drag_starting is holding — dangle.
        auto box = std::make_unique<attached_box>();
        jobject native = box->native();
        ASSERT_NE(native, nullptr);

        auto drag = std::make_shared<drag_gesture_recognizer>();
        int drag_starts = 0;
        drag->drag_starting.connect([&](maui::controls::drag_starting_event_args&) {
            ++drag_starts;
            box.reset(); // box_view + handler + gesture manager, all gone mid-send
        });
        box->control.gesture_recognizers().add(drag);
        ASSERT_TRUE(box->control.gesture_manager().native_registered_drag_source(*drag))
            << "long press must be armed, or nothing below is exercised";

        dispatch_touch(native, k_action_down, 9, 9, 20000, 20000);
        pump_looper(8, [&drag_starts] { return drag_starts > 0; });

        // The positive witness: without it a "didn't crash" result cannot be told from a vacuous run.
        EXPECT_EQ(drag_starts, 1) << "the long press must actually have reached drag_on_long_press";
        EXPECT_EQ(box, nullptr);
    }

    // The drop target's OnDragListener is genuinely installed and genuinely removed — the port reports
    // real registration state here, unlike the apple/ios attachment-only stance.
    TEST(AndroidGestureSeam, DropTargetRegistrationFollowsTheCollection)
    {
        ASSERT_NE(maui::platform::android::testhost::host_context(), nullptr);
        attached_box box;
        ASSERT_NE(box.native(), nullptr);

        auto drop = std::make_shared<drop_gesture_recognizer>();
        box.control.gesture_recognizers().add(drop);
        EXPECT_TRUE(box.control.gesture_manager().native_registered_drop_target(*drop));

        box.control.gesture_recognizers().remove(drop);
        EXPECT_FALSE(box.control.gesture_manager().native_registered_drop_target(*drop));
    }

    // The drag source is armed through IsLongpressEnabled (TapAndPanGestureDetector.cs:26-39) — long
    // press is OFF until a drag recognizer exists, because it would otherwise pre-empt every pan start.
    TEST(AndroidGestureSeam, DragSourceRegistrationFollowsTheCollection)
    {
        ASSERT_NE(maui::platform::android::testhost::host_context(), nullptr);
        attached_box box;
        ASSERT_NE(box.native(), nullptr);

        auto drag = std::make_shared<drag_gesture_recognizer>();
        box.control.gesture_recognizers().add(drag);
        EXPECT_TRUE(box.control.gesture_manager().native_registered_drag_source(*drag));

        box.control.gesture_recognizers().remove(drag);
        EXPECT_FALSE(box.control.gesture_manager().native_registered_drag_source(*drag));
    }
} // namespace
