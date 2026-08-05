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
