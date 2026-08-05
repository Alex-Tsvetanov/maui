// gesture_platform_manager — ANDROID platform partial (the GesturePlatformManager.Android.cs role).
// Ported from src/Controls/src/Core/Platform/GestureManager/GesturePlatformManager.Android.cs plus the
// six Java-side helpers it owns (src/Controls/src/Core/Platform/Android/):
//   TapAndPanGestureDetector.cs, InnerGestureListener.cs, InnerScaleListener.cs,
//   TapGestureHandler.cs, PanGestureHandler.cs, SwipeGestureHandler.cs, PinchGestureHandler.cs,
//   PointerGestureHandler.cs, DragAndDropGestureHandler.cs
//
// >>> THE STRUCTURAL RULING <<<
// The shared seam (gesture_platform_manager.hpp) is PER-RECOGNIZER because UIKit/AppKit recognizers ARE
// per-recognizer objects. Android is not built that way, and the C# oracle proves it: ONE
// GestureDetector + ONE ScaleGestureDetector + one Touch / OnHoverListener / OnDragListener subscription
// on THE VIEW, recomputed from the WHOLE collection by SetupGestures (:183-256), with the per-recognizer
// dispatch happening INSIDE the callbacks (TapGestureHandler.cs:63-71, PanGestureHandler.cs:40-45,
// SwipeGestureHandler.cs:40-45). So here:
//     native_attach(r)   => insert r into the backend table, then re-run ONE whole-collection sync
//     native_detach(r)   => erase r from the backend table, then re-run that same sync
//     native_detach_all()=> invalidate the Java back-ref FIRST, uninstall every listener, clear the table
// There is deliberately NO per-recognizer native object. load_recognizers()
// (src/controls/gestures/gesture_platform_manager.cpp:49-79) calls native_attach once per ADDED
// recognizer and native_detach once per REMOVED one, so the sync runs repeatedly per load — it is cheap
// (a handful of JNI setter calls) and idempotent by construction.
//
// >>> ORDERING TRAP <<<
// The core pushes into attached_ BEFORE native_attach and calls native_detach BEFORE erasing, so
// attached_ is STALE at native_detach time. Every whole-collection decision below therefore reads the
// BACKEND TABLE (gesture_native_state::order), never attached_. That also matches C#, whose
// SetupGestures runs from the CollectionChanged handler — i.e. after the collection already changed.
//
// >>> WHAT LIVES WHERE <<<
// java/MauiGestureBridge.java is dumb plumbing: it owns the two platform detectors and forwards their
// callbacks across JNI as primitives. Every DECISION C# makes in InnerGestureListener and the five
// *GestureHandler classes — the double-tap state machine, the button-mask filters, the px->dp
// translation, the pan id stamping, the swipe accumulate-then-detect, the drag/drop payload — is here,
// where the recognizer collection is. The Java peer is the backend's gesture_native_state (NOT the
// manager): the free entry points below need the table and the state machine, and nothing else.
//
// DEVIATIONS (each cites the C# it departs from):
//   - The RecyclerView branch of SetupGestures (:228-238) has no port equivalent: the port's Android
//     CollectionView is a plain ScrollView + MauiCollectionContent, not a RecyclerView
//     (src/platform/android/collection_view_handler.cpp:41-46), so there is no OnItemTouchListener to
//     install. // TODO: verify against
//     src/Controls/src/Core/Platform/GestureManager/GesturePlatformManager.Android.cs:228-238 when a
//     RecyclerView-backed CollectionView lands.
//   - TapGestureHandler.OnTap's CHILD-element fan-out (:46-58, view.GetChildElements(point) ->
//     GetChildGesturesFor) is not ported: the port has no GestureElement child hit-testing surface.
//     // TODO: verify against src/Controls/src/Core/Platform/Android/TapGestureHandler.cs:46-58.
//   - UpdateDragAndDrop / UpdatePointer (:359-369) are gated in C# on
//     `GetCompositeGestureRecognizers()?.Count > 0`, which exists only to avoid force-creating the
//     Lazy<> handlers; the handlers' own SetupHandlerForDrop / SetupHandlerForPointer
//     (DragAndDropGestureHandler.cs:61-67, PointerGestureHandler.cs:184-200) unconditionally
//     INSTALL-OR-CLEAR. The port has no Lazy<>, so the sync runs unconditionally — which is what makes
//     removing the last drop recognizer actually uninstall the listener.
//   - Only the TEXT drag payload is marshalled. Image / custom-ClipData marshalling
//     (DragAndDropGestureHandler.cs:286-334, :359-377) has no port equivalent yet — see the TODO at
//     drag_on_long_press.
//   - C# Android never calls SendPanCanceled: OnPanComplete (PanGestureHandler.cs:50-65) is the only
//     terminal pan send. Preserved.

#include <jni.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"

#include "android_view_ops.hpp"

#include "maui/controls/data_package.hpp"
#include "maui/controls/data_package_operation.hpp"
#include "maui/controls/element.hpp"
#include "maui/controls/gestures/buttons_mask.hpp"
#include "maui/controls/gestures/drag_gesture_recognizer.hpp"
#include "maui/controls/gestures/drop_gesture_recognizer.hpp"
#include "maui/controls/gestures/gesture_platform_manager.hpp"
#include "maui/controls/gestures/gesture_recognizer.hpp"
#include "maui/controls/gestures/pan_gesture_recognizer.hpp"
#include "maui/controls/gestures/pinch_gesture_recognizer.hpp"
#include "maui/controls/gestures/pointer_gesture_recognizer.hpp"
#include "maui/controls/gestures/swipe_gesture_recognizer.hpp"
#include "maui/controls/gestures/tap_gesture_recognizer.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/graphics/point.hpp"

namespace
{
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;

    constexpr const char* k_bridge_class = "dev/mauicpp/MauiGestureBridge";
    constexpr const char* k_view_class = "android/view/View";

    // android.view.MotionEvent action constants (Android.Views.MotionEventActions).
    constexpr jint k_action_down = 0;
    constexpr jint k_action_up = 1;
    constexpr jint k_action_move = 2;
    constexpr jint k_action_cancel = 3;
    constexpr jint k_action_hover_move = 7;
    constexpr jint k_action_hover_enter = 9;
    constexpr jint k_action_hover_exit = 10;

    // android.view.MotionEvent button-state flags (Android.Views.MotionEventButtonState).
    constexpr jint k_button_primary = 1;
    constexpr jint k_button_secondary = 2;
    constexpr jint k_button_stylus_secondary = 64;

    // android.view.DragEvent action constants (Android.Views.DragAction).
    constexpr jint k_drag_started = 1;
    constexpr jint k_drag_location = 2;
    constexpr jint k_drag_drop = 3;
    constexpr jint k_drag_ended = 4;
    constexpr jint k_drag_entered = 5;
    constexpr jint k_drag_exited = 6;

    // InnerScaleListener.OnScale's span deadband (:41-42).
    constexpr float k_pinch_span_deadband = 10.0F;

    // Turns a pending Java exception into `true` + a cleared VM state (the backend's house style — see
    // button_handler.cpp).
    bool clear_pending(JNIEnv* env)
    {
        if (env->ExceptionCheck() == JNI_TRUE)
        {
            env->ExceptionClear();
            return true;
        }
        return false;
    }
} // namespace

namespace maui::controls
{
    // The manager's backend attachment table (the forward-declared gesture_native_state). Complete only
    // in this TU, where the manager's ctor/dtor — and thus the owning unique_ptr's deleter — are defined.
    // It doubles as the JNI PEER: every native entry point below receives its address as a jlong.
    struct gesture_native_state
    {
        // The ordered backend table. Insertion order == collection order, because every C# fan-out is a
        // `foreach (var gesture in view.GestureRecognizers)`. Raw pointers are safe: the entry is always
        // erased before attached_ drops its strong ref, and native_detach_all runs before
        // attached_.clear() in set_handler (:40-41) and in ~gesture_platform_manager.
        std::vector<gesture_recognizer*> order;

        // Refreshed on every sync from the manager (non-owning; the owning view outlives the manager).
        element* sender = nullptr;
        maui::core::i_view_handler* handler = nullptr;

        maui::platform::android::global_ref<jobject> bridge; // dev.mauicpp.MauiGestureBridge
        maui::platform::android::global_ref<jobject> view;   // the View the listeners are installed on

        // InnerGestureListener state (:19-23).
        bool is_scrolling = false;
        float last_x = 0;
        float last_y = 0;
        bool single_tap_fired_in_sequence = false;

        // PinchGestureHandler._pinchStartingScale (:11).
        double pinch_starting_scale = 1;

        // PointerGestureHandler._activeButton (:13).
        std::optional<buttons_mask> active_button;

        // DragAndDropGestureHandler._currentCustomLocalStateData (:20), minus the fields that only exist
        // to cross the Java boundary. An in-process drag never leaves the process, so the package and the
        // accepted operation stay here on the DRAG SOURCE's state and the drop target reaches them
        // through the source peer the Java LocalState carries.
        std::unique_ptr<data_package> drag_package;
        data_package_operation accepted_operation = data_package_operation::copy;

        // SetupGestures' _focusableDefaultValue (:28, :248, :309).
        std::optional<bool> focusable_default;

        // What is actually installed right now — native_registered_drag_source / _drop_target report off
        // these, not off table membership.
        bool touch_installed = false;
        bool hover_installed = false;
        bool drag_installed = false;
        bool key_installed = false;
        bool long_press_enabled = false;
    };

    namespace
    {
        // ---- table queries (every one reads gesture_native_state::order — never attached_) ------------

        template <typename Recognizer> [[nodiscard]] bool has_any(const gesture_native_state& state)
        {
            return std::ranges::any_of(state.order, [](gesture_recognizer* recognizer) {
                return dynamic_cast<Recognizer*>(recognizer) != nullptr;
            });
        }

        // Fan out to every recognizer of the given kind, in collection order.
        template <typename Recognizer, typename Fn> bool for_each_of(gesture_native_state& state, Fn fn)
        {
            bool any = false;
            for (gesture_recognizer* recognizer : state.order)
            {
                if (auto* typed = dynamic_cast<Recognizer*>(recognizer))
                {
                    fn(*typed);
                    any = true;
                }
            }
            return any;
        }

        // PinchGestureHandler.PinchGesture (:21-22): a view can hold at most one pinch recognizer
        // (enforced by gesture_recognizer_collection), so the first is the only one.
        [[nodiscard]] i_pinch_gesture_controller* pinch_gesture(gesture_native_state& state)
        {
            for (gesture_recognizer* recognizer : state.order)
            {
                if (auto* pinch = dynamic_cast<i_pinch_gesture_controller*>(recognizer))
                {
                    return pinch;
                }
            }
            return nullptr;
        }

        // InnerGestureListener.HasAnyGestures (:64-67).
        [[nodiscard]] bool has_any_gestures(const gesture_native_state& state)
        {
            return has_any<pan_gesture_recognizer>(state) || has_any<tap_gesture_recognizer>(state) ||
                   has_any<swipe_gesture_recognizer>(state);
        }

        // TapGestureHandler.TapGestureRecognizers(count) (:97-104) — used only as an existence test here.
        [[nodiscard]] bool has_tap_handler(const gesture_native_state& state, int count)
        {
            return std::ranges::any_of(state.order, [count](gesture_recognizer* recognizer) {
                auto* tap = dynamic_cast<tap_gesture_recognizer*>(recognizer);
                return tap != nullptr && tap->number_of_taps_required() == count;
            });
        }

        // InnerGestureListener.HasSingleTapHandlerWithPrimaryAndSecondary (:269-282).
        [[nodiscard]] bool has_single_tap_handler_with_primary_and_secondary(const gesture_native_state& state)
        {
            return std::ranges::any_of(state.order, [](gesture_recognizer* recognizer) {
                auto* tap = dynamic_cast<tap_gesture_recognizer*>(recognizer);
                return tap != nullptr && tap->number_of_taps_required() == 1 &&
                       contains(tap->buttons(), buttons_mask::primary | buttons_mask::secondary);
            });
        }

        // SemanticExtensions.HasAccessibleTapGesture (src/Controls/src/Core/Platform/SemanticExtensions.cs
        // :10-25): the FIRST single-tap recognizer decides — and only if it accepts the primary button.
        [[nodiscard]] tap_gesture_recognizer* accessible_tap_gesture(gesture_native_state& state)
        {
            for (gesture_recognizer* recognizer : state.order)
            {
                auto* tap = dynamic_cast<tap_gesture_recognizer*>(recognizer);
                if (tap != nullptr && tap->number_of_taps_required() == 1)
                {
                    return contains(tap->buttons(), buttons_mask::primary) ? tap : nullptr;
                }
            }
            return nullptr;
        }

        // ---- units ------------------------------------------------------------------------------------

        // ContextExtensions.FromPixels — the px->dp translation PanGestureHandler.PixelTranslation
        // (:10-23), SwipeGestureHandler.PixelTranslation (:10-23) and InnerScaleListener (:44, :49) apply.
        // C# returns 0 when there is no Context; a zero density here means "no View" and does the same.
        [[nodiscard]] double from_pixels(gesture_native_state& state, double pixels)
        {
            const scoped_env env;
            if (!env || state.view.get() == nullptr)
            {
                return 0;
            }
            const float density = maui::platform::android::detail::view_display_density(env.get(), state.view.get());
            return density == 0.0F ? 0 : pixels / static_cast<double>(density);
        }

        // MotionEventExtensions.CalculatePosition with relativeElement == sourceElement (:33-36): the
        // view-relative coordinates in DIPs.
        [[nodiscard]] maui::graphics::point view_position(gesture_native_state& state, jfloat x, jfloat y)
        {
            return {from_pixels(state, x), from_pixels(state, y)};
        }

        // ---- tap (TapGestureHandler) --------------------------------------------------------------------

        // TapGestureHandler.OnTap's local CheckButtonMask (:75-87). NOTE the asymmetry: an EXACTLY
        // secondary mask demands a secondary/stylus-secondary button state; every other mask just needs
        // Primary to be in it.
        [[nodiscard]] bool check_tap_button_mask(const tap_gesture_recognizer& tap, jint button_state, bool has_event)
        {
            if (tap.buttons() == buttons_mask::secondary)
            {
                const jint state = has_event ? button_state : k_button_primary; // `motionEvent?.ButtonState ?? Primary`
                return state == k_button_secondary || state == k_button_stylus_secondary;
            }
            return contains(tap.buttons(), buttons_mask::primary);
        }

        // TapGestureHandler.OnTap (:30-88), minus the child-element fan-out (see the file's DEVIATIONS).
        bool on_tap(gesture_native_state& state, int count, jfloat x, jfloat y, jint button_state)
        {
            if (state.sender == nullptr)
            {
                return false; // `if (view == null) return false` (:41-42)
            }
            const maui::graphics::point position = view_position(state, x, y);
            bool captured = false;
            for (gesture_recognizer* recognizer : state.order)
            {
                auto* tap = dynamic_cast<tap_gesture_recognizer*>(recognizer);
                if (tap == nullptr || tap->number_of_taps_required() != count)
                {
                    continue;
                }
                if (!check_tap_button_mask(*tap, button_state, /*has_event=*/true))
                {
                    continue;
                }
                tap->send_tapped(*state.sender, position);
                captured = true;
            }
            return captured;
        }

        // ---- pan (PanGestureHandler) --------------------------------------------------------------------

        // PanGestureHandler.OnPanStarted (:67-82).
        bool on_pan_started(gesture_native_state& state, jint pointer_count)
        {
            if (state.sender == nullptr)
            {
                return false;
            }
            element& sender = *state.sender;
            return for_each_of<pan_gesture_recognizer>(state, [&sender, pointer_count](pan_gesture_recognizer& pan) {
                if (pan.touch_points() == pointer_count)
                {
                    pan.send_pan_started(sender, pan_gesture_recognizer::current_id().value());
                }
            });
        }

        // PanGestureHandler.OnPan (:32-48). The totals arrive in PIXELS and are cumulative since the
        // gesture's first event (InnerGestureListener.StartScrolling :252-253).
        bool on_pan(gesture_native_state& state, jfloat total_x, jfloat total_y, jint pointer_count)
        {
            if (state.sender == nullptr)
            {
                return false;
            }
            const double dip_x = from_pixels(state, total_x);
            const double dip_y = from_pixels(state, total_y);
            element& sender = *state.sender;
            bool result = false;
            for_each_of<pan_gesture_recognizer>(state, [&](pan_gesture_recognizer& pan) {
                if (pan.touch_points() == pointer_count)
                {
                    pan.send_pan(sender, dip_x, dip_y, pan_gesture_recognizer::current_id().value());
                    result = true;
                }
            });
            return result;
        }

        // PanGestureHandler.OnPanComplete (:50-65). NO TouchPoints predicate here (C# omits it), and the
        // id increments exactly once — after the whole sweep, inside the view != null guard (:63).
        bool on_pan_complete(gesture_native_state& state)
        {
            if (state.sender == nullptr)
            {
                return false;
            }
            element& sender = *state.sender;
            const bool result = for_each_of<pan_gesture_recognizer>(state, [&sender](pan_gesture_recognizer& pan) {
                pan.send_pan_completed(sender, pan_gesture_recognizer::current_id().value());
            });
            pan_gesture_recognizer::current_id().increment();
            return result;
        }

        // ---- swipe (SwipeGestureHandler) ------------------------------------------------------------------

        // SwipeGestureHandler.OnSwipe (:32-48): accumulate on every move, detect nothing.
        bool on_swipe(gesture_native_state& state, jfloat total_x, jfloat total_y)
        {
            if (state.sender == nullptr)
            {
                return false;
            }
            const double dip_x = from_pixels(state, total_x);
            const double dip_y = from_pixels(state, total_y);
            element& sender = *state.sender;
            return for_each_of<swipe_gesture_recognizer>(
                state, [&](swipe_gesture_recognizer& swipe) { swipe.send_swipe(sender, dip_x, dip_y); });
        }

        // SwipeGestureHandler.OnSwipeComplete (:50-67): SHORT-CIRCUITS on the first detected swipe.
        // detect_swipe owns the threshold — the native side never sees it. Note this is deliberately NOT
        // send_swiped (the iOS-only path) and applies NO rotation compensation (iOS-only too).
        bool on_swipe_complete(gesture_native_state& state)
        {
            if (state.sender == nullptr)
            {
                return false;
            }
            for (gesture_recognizer* recognizer : state.order)
            {
                auto* swipe = dynamic_cast<swipe_gesture_recognizer*>(recognizer);
                if (swipe != nullptr && swipe->detect_swipe(*state.sender, swipe->direction()))
                {
                    return true;
                }
            }
            return false;
        }

        // ---- scrolling state machine (InnerGestureListener :236-267) --------------------------------------

        void set_starting_position(gesture_native_state& state, jfloat raw_x, jfloat raw_y)
        {
            state.last_x = raw_x;
            state.last_y = raw_y;
        }

        // InnerGestureListener.EndScrolling (:258-267): pan-complete AND swipe-complete, both gated on
        // _isScrolling. Driven from ACTION_UP (TapAndPanGestureDetector :53-54) and OnFling (:129).
        void end_scrolling(gesture_native_state& state)
        {
            if (state.is_scrolling)
            {
                on_pan_complete(state);
            }
            if (state.is_scrolling)
            {
                on_swipe_complete(state);
            }
            state.is_scrolling = false;
        }

        // ---- pointer (PointerGestureHandler) ---------------------------------------------------------------

        // PointerGestureHandler.GetPressedButton (:120-162). `action_button` is non-zero only for the
        // API 23+ ButtonPress/ButtonRelease actions (the Java side applies that gate).
        [[nodiscard]] buttons_mask pressed_button(jint action_button, jint button_state)
        {
            if (action_button != 0)
            {
                if ((action_button & k_button_secondary) == k_button_secondary)
                {
                    return buttons_mask::secondary;
                }
                if ((action_button & k_button_primary) == k_button_primary)
                {
                    return buttons_mask::primary;
                }
            }
            if ((button_state & k_button_secondary) == k_button_secondary)
            {
                return buttons_mask::secondary;
            }
            if ((button_state & k_button_stylus_secondary) == k_button_stylus_secondary)
            {
                return buttons_mask::secondary;
            }
            return buttons_mask::primary;
        }

        // PointerGestureHandler.CheckButtonMask (:170-182). The zero-mask branch (:173-174) has no
        // equivalent in the shared `contains()` — it is spelled out here, exactly as C# has it.
        [[nodiscard]] bool check_pointer_button_mask(const pointer_gesture_recognizer& pointer, buttons_mask current)
        {
            if (static_cast<std::uint8_t>(pointer.buttons()) == 0)
            {
                return current == buttons_mask::primary;
            }
            if (current == buttons_mask::secondary)
            {
                return contains(pointer.buttons(), buttons_mask::secondary);
            }
            return contains(pointer.buttons(), buttons_mask::primary);
        }

        // ---- drag & drop (DragAndDropGestureHandler) -------------------------------------------------------

        // DragAndDropGestureHandler.OnLongPress (:251-357). Long press only reaches here while a drag
        // recognizer is attached — the sync arms IsLongpressEnabled from HasAnyDragGestures
        // (InnerGestureListener :61-62 -> TapAndPanGestureDetector :26-39), because an always-on long
        // press would pre-empt every pan start (the comment at TapAndPanGestureDetector.cs:31-36).
        void drag_on_long_press(gesture_native_state& state)
        {
            if (state.sender == nullptr || state.view.get() == nullptr || !has_any<drag_gesture_recognizer>(state))
            {
                return; // :253-254
            }
            const scoped_env env;
            if (!env)
            {
                return;
            }
            auto& cache = default_jni_cache();
            jclass bridge_class = cache.find_class(env.get(), k_bridge_class);
            jmethodID start_drag = cache.static_method(env.get(), k_bridge_class, "startDrag",
                                                       "(Landroid/view/View;Ljava/lang/String;J)V");
            if (bridge_class == nullptr || start_drag == nullptr)
            {
                return; // the bridge class is host-provided; without it the drag channel stays C++-only
            }

            for (gesture_recognizer* recognizer : state.order)
            {
                auto* drag = dynamic_cast<drag_gesture_recognizer*>(recognizer);
                if (drag == nullptr || !drag->can_drag())
                {
                    continue; // :258-259
                }
                drag_starting_event_args args = drag->send_drag_starting(*state.sender);
                if (args.cancel())
                {
                    continue; // :271-272
                }
                // The package the drop target will read back through this state's peer (C# parks it in
                // CustomLocalStateData.DataPackage :274-275).
                state.drag_package = std::make_unique<data_package>(std::move(args.data()));
                state.accepted_operation = data_package_operation::copy; // CustomLocalStateData :383

                // args.Handled suppresses the platform's default payload fill (:283).
                // TODO: verify against
                // src/Controls/src/Core/Platform/Android/DragAndDropGestureHandler.cs:286-334 + :359-377 —
                // the image / custom-ClipData payload (ConvertToClipDataItem, args.PlatformArgs.ClipData)
                // has no port equivalent yet, so only the TEXT branch is marshalled.
                const std::string text =
                    (!args.handled() && state.drag_package->text()) ? *state.drag_package->text() : std::string{};
                const local_ref<jstring> payload = maui::platform::android::to_jstring(env.get(), text);
                env->CallStaticVoidMethod(bridge_class, start_drag, state.view.get(), payload.get(),
                                          reinterpret_cast<jlong>(&state));
                clear_pending(env.get());
            }
        }

        // DragAndDropGestureHandler.HandleDragOver / HandleDragLeave (:168-201). One DragEventArgs is
        // reused across the sweep, exactly as C# does, and AcceptedOperation is read back after each send.
        void handle_drag_hover(gesture_native_state& state, gesture_native_state& shared, data_package& package,
                               bool leaving)
        {
            drag_event_args args{package};
            for (gesture_recognizer* recognizer : state.order)
            {
                auto* drop = dynamic_cast<drop_gesture_recognizer*>(recognizer);
                if (drop == nullptr || !drop->allow_drop())
                {
                    continue; // :174-175 / :192-193
                }
                if (leaving)
                {
                    drop->send_drag_leave(args);
                }
                else
                {
                    drop->send_drag_over(args);
                }
                shared.accepted_operation = args.accepted_operation();
            }
        }

        // DragAndDropGestureHandler.HandleDrop (:203-249).
        void handle_drop(gesture_native_state& state, gesture_native_state& shared, data_package& package,
                         bool from_outside, const std::optional<std::string>& clip_text)
        {
            if (shared.accepted_operation == data_package_operation::none)
            {
                return; // :205-206
            }
            if (from_outside) // `if (e.LocalState == null)` (:209)
            {
                const bool blank =
                    !package.text() || package.text()->find_first_not_of(" \t\n\v\f\r") == std::string::npos;
                if (blank && clip_text)
                {
                    package.set_text(clip_text); // :227-228
                }
                // :230-231 assigns the coerced text to DataPackage.Image as well; the port's image slot is
                // an i_image_source, not a string, so there is nothing faithful to assign.
                // TODO: verify against
                // src/Controls/src/Core/Platform/Android/DragAndDropGestureHandler.cs:230-231.
            }
            drop_event_args args{package.view()};
            for (gesture_recognizer* recognizer : state.order)
            {
                auto* drop = dynamic_cast<drop_gesture_recognizer*>(recognizer);
                if (drop == nullptr || !drop->allow_drop())
                {
                    continue; // :237-238
                }
                drop->send_drop(args, state.sender);
            }
        }

        // DragAndDropGestureHandler.HandleDropCompleted (:162-166): raised on the DRAG SOURCE's
        // recognizers, not the target's. send_drop_completed is latched once per drag inside the
        // recognizer, which is exactly why C# can fire it from every drop-handling view.
        void handle_drop_completed(gesture_native_state& source)
        {
            const drop_completed_event_args args;
            for_each_of<drag_gesture_recognizer>(
                source, [&args](drag_gesture_recognizer& drag) { drag.send_drop_completed(args); });
        }

        // ---- the whole-collection subscription sync ---------------------------------------------------------

        // Install or clear a View listener (`setOnXListener(bridge or null)`). Returns whether it ended up
        // installed.
        bool set_view_listener(JNIEnv* env, jobject view, jobject bridge, const char* setter, const char* signature,
                               bool install)
        {
            jmethodID method = default_jni_cache().method(env, k_view_class, setter, signature);
            if (method == nullptr)
            {
                return false;
            }
            env->CallVoidMethod(view, method, install ? bridge : nullptr);
            return !clear_pending(env) && install;
        }

        // GestureCollectionChanged (:347-357) = UpdateDragAndDrop(); UpdatePointer(); SetupGestures();
        // UpdateLongPressSettings(). ONE idempotent pass, cheap enough to re-run per added/removed
        // recognizer (see the file header).
        void sync_subscriptions(gesture_native_state& state)
        {
            const scoped_env env;
            jobject view = state.view.get();
            if (!env || view == nullptr)
            {
                return; // `if (platformView == null) return` (:190-191)
            }

            // SetupGestures :203-218. The port's collection has no separate "composite" list, so the
            // C# `Count == 0 -> inspect CompositeGestureRecognizers` branch collapses to "is the
            // collection empty" — which is what that branch computes for a single-collection view too.
            const bool should_add_touch = !state.order.empty();
            const bool has_pointer = has_any<pointer_gesture_recognizer>(state);
            const bool has_drop = has_any<drop_gesture_recognizer>(state);
            const bool has_drag = has_any<drag_gesture_recognizer>(state);
            const bool has_pinch = has_any<pinch_gesture_recognizer>(state);

            jobject bridge = state.bridge.get();
            if (bridge == nullptr)
            {
                return;
            }
            auto& cache = default_jni_cache();

            // SetupGestures :241 (Touch) — the RecyclerView branch (:234-238) has no port equivalent.
            state.touch_installed = set_view_listener(env.get(), view, bridge, "setOnTouchListener",
                                                      "(Landroid/view/View$OnTouchListener;)V", should_add_touch);

            // SetupGestures :245-255 — the accessible-tap key channel plus its Focusable override, and the
            // restore of the captured default when the touch channel goes away.
            const bool wants_keys = should_add_touch && accessible_tap_gesture(state) != nullptr;
            state.key_installed = set_view_listener(env.get(), view, bridge, "setOnKeyListener",
                                                    "(Landroid/view/View$OnKeyListener;)V", wants_keys);
            jmethodID set_focusable = cache.method(env.get(), k_view_class, "setFocusable", "(Z)V");
            jmethodID is_focusable = cache.method(env.get(), k_view_class, "isFocusable", "()Z");
            if (set_focusable != nullptr && is_focusable != nullptr)
            {
                if (wants_keys)
                {
                    if (!state.focusable_default) // `_focusableDefaultValue ??= platformView.Focusable`
                    {
                        state.focusable_default = env->CallBooleanMethod(view, is_focusable) == JNI_TRUE;
                        clear_pending(env.get());
                    }
                    env->CallVoidMethod(view, set_focusable, JNI_TRUE);
                }
                else if (state.focusable_default)
                {
                    // SetupElement :309-310 restores the captured default; :254 just forgets it. Restoring
                    // is the only spelling that does not strand a view focusable after its tap gesture is
                    // removed (the port re-syncs on every collection change, C# only on element teardown).
                    env->CallVoidMethod(view, set_focusable, *state.focusable_default ? JNI_TRUE : JNI_FALSE);
                    state.focusable_default.reset();
                }
                clear_pending(env.get());
            }

            // PointerGestureHandler.SetupHandlerForPointer (:184-200).
            state.hover_installed = set_view_listener(env.get(), view, bridge, "setOnHoverListener",
                                                      "(Landroid/view/View$OnHoverListener;)V", has_pointer);

            // DragAndDropGestureHandler.SetupHandlerForDrop (:61-67).
            state.drag_installed = set_view_listener(env.get(), view, bridge, "setOnDragListener",
                                                     "(Landroid/view/View$OnDragListener;)V", has_drop);

            // TapAndPanGestureDetector.UpdateLongPressSettings (:26-39) + ViewHasPinchGestures (:168-181),
            // pushed onto the bridge so the touch routing stays a field read.
            jmethodID set_long_press = cache.method(env.get(), k_bridge_class, "setLongPressEnabled", "(Z)V");
            jmethodID set_has_pinch = cache.method(env.get(), k_bridge_class, "setHasPinchGestures", "(Z)V");
            if (set_long_press != nullptr && set_has_pinch != nullptr)
            {
                env->CallVoidMethod(bridge, set_long_press, has_drag ? JNI_TRUE : JNI_FALSE);
                env->CallVoidMethod(bridge, set_has_pinch, has_pinch ? JNI_TRUE : JNI_FALSE);
                clear_pending(env.get());
                state.long_press_enabled = has_drag;
            }
        }

        // ---- the native halves of dev.mauicpp.MauiGestureBridge -------------------------------------------
        // The peer is the gesture_native_state (the manager itself is not needed: everything these need is
        // the ordered table, the sender and the state machine). Java zeroes its OWN peer via detach()
        // before the storage dies, so a stale queued MotionEvent lands on peer == 0 and returns here.
        //
        // The DRAG-SOURCE peer cannot be protected that way: it travels inside the OS drag session's
        // LocalState (MauiGestureBridge.LocalState), which outlives any single view, so a drag source torn
        // down mid-drag would leave ACTION_DRAG_ENDED holding a dangling pointer. The live-state registry
        // is the guard — every peer is validated against it before it is dereferenced. (C#'s equivalent
        // hazard is absorbed by the GC; PROFILE §8 makes it explicit here instead.)

        std::mutex& live_states_mutex()
        {
            static std::mutex mutex;
            return mutex;
        }

        std::unordered_set<const gesture_native_state*>& live_states()
        {
            static std::unordered_set<const gesture_native_state*> states;
            return states;
        }

        void register_live_state(const gesture_native_state* state)
        {
            const std::scoped_lock lock(live_states_mutex());
            live_states().insert(state);
        }

        void unregister_live_state(const gesture_native_state* state)
        {
            const std::scoped_lock lock(live_states_mutex());
            live_states().erase(state);
        }

        [[nodiscard]] gesture_native_state* peer_state(jlong peer)
        {
            auto* candidate = reinterpret_cast<gesture_native_state*>(peer);
            if (candidate == nullptr)
            {
                return nullptr;
            }
            const std::scoped_lock lock(live_states_mutex());
            return live_states().contains(candidate) ? candidate : nullptr;
        }

        jboolean JNICALL native_on_down(JNIEnv* /*env*/, jclass /*cls*/, jlong peer, jfloat raw_x, jfloat raw_y)
        {
            // InnerGestureListener.OnDown (:111-125).
            gesture_native_state* state = peer_state(peer);
            if (state == nullptr)
            {
                return JNI_FALSE;
            }
            set_starting_position(*state, raw_x, raw_y);
            return has_any_gestures(*state) ? JNI_TRUE : JNI_FALSE;
        }

        jboolean JNICALL native_on_scroll(JNIEnv* /*env*/, jclass /*cls*/, jlong peer, jfloat start_raw_x,
                                          jfloat start_raw_y, jfloat raw_x, jfloat raw_y, jint pointer_count)
        {
            // InnerGestureListener.OnScroll (:139-147) -> StartScrolling (:242-256).
            gesture_native_state* state = peer_state(peer);
            if (state == nullptr)
            {
                return JNI_FALSE;
            }
            set_starting_position(*state, start_raw_x, start_raw_y);
            if (!state->is_scrolling)
            {
                on_pan_started(*state, pointer_count);
            }
            state->is_scrolling = true;
            const jfloat total_x = raw_x - state->last_x;
            const jfloat total_y = raw_y - state->last_y;
            // `_scrollDelegate(...) || _swipeDelegate(...)` (:255) — the || SHORT-CIRCUITS in C# too, so a
            // view with both a matching pan and a swipe accumulates only the pan.
            return (on_pan(*state, total_x, total_y, pointer_count) || on_swipe(*state, total_x, total_y)) ? JNI_TRUE
                                                                                                           : JNI_FALSE;
        }

        void JNICALL native_end_scrolling(JNIEnv* /*env*/, jclass /*cls*/, jlong peer)
        {
            if (gesture_native_state* state = peer_state(peer))
            {
                end_scrolling(*state);
            }
        }

        void JNICALL native_on_long_press(JNIEnv* /*env*/, jclass /*cls*/, jlong peer, jfloat raw_x, jfloat raw_y)
        {
            // InnerGestureListener.OnLongPress (:133-137).
            gesture_native_state* state = peer_state(peer);
            if (state == nullptr)
            {
                return;
            }
            set_starting_position(*state, raw_x, raw_y);
            drag_on_long_press(*state);
        }

        jboolean JNICALL native_on_single_tap_up(JNIEnv* /*env*/, jclass /*cls*/, jlong peer, jfloat x, jfloat y,
                                                 jint button_state)
        {
            // InnerGestureListener.OnSingleTapUp (:153-175): the single tap fires IMMEDIATELY even when a
            // double-tap handler exists (Windows timing), latching _singleTapFiredInSequence.
            gesture_native_state* state = peer_state(peer);
            if (state == nullptr)
            {
                return JNI_FALSE;
            }
            if (has_tap_handler(*state, 2))
            {
                if (has_tap_handler(*state, 1))
                {
                    on_tap(*state, 1, x, y, button_state);
                    state->single_tap_fired_in_sequence = true;
                }
                return JNI_FALSE; // keep waiting for a potential double tap (:168-169)
            }
            return on_tap(*state, 1, x, y, button_state) ? JNI_TRUE : JNI_FALSE;
        }

        jboolean JNICALL native_on_single_tap_confirmed(JNIEnv* /*env*/, jclass /*cls*/, jlong peer, jfloat x, jfloat y,
                                                        jint button_state)
        {
            // InnerGestureListener.OnSingleTapConfirmed (:177-204).
            gesture_native_state* state = peer_state(peer);
            if (state == nullptr)
            {
                return JNI_FALSE;
            }
            // MotionEventExtensions.IsSecondary (:9-16).
            const bool is_secondary = button_state == k_button_secondary || button_state == k_button_stylus_secondary;
            if (!has_tap_handler(*state, 2) &&
                (!is_secondary || has_single_tap_handler_with_primary_and_secondary(*state)))
            {
                return JNI_FALSE; // OnSingleTapUp already ran the delegate (:186-192)
            }
            if (state->single_tap_fired_in_sequence)
            {
                state->single_tap_fired_in_sequence = false;
                return JNI_FALSE; // :195-199
            }
            return on_tap(*state, 1, x, y, button_state) ? JNI_TRUE : JNI_FALSE;
        }

        jboolean JNICALL native_on_double_tap(JNIEnv* /*env*/, jclass /*cls*/, jlong peer, jfloat x, jfloat y,
                                              jint button_state)
        {
            // InnerGestureListener.OnDoubleTap (:76-95).
            gesture_native_state* state = peer_state(peer);
            if (state == nullptr || !has_tap_handler(*state, 2))
            {
                return JNI_FALSE;
            }
            state->single_tap_fired_in_sequence = false;
            return on_tap(*state, 2, x, y, button_state) ? JNI_TRUE : JNI_FALSE;
        }

        jboolean JNICALL native_on_double_tap_event(JNIEnv* /*env*/, jclass /*cls*/, jlong peer, jint action, jfloat x,
                                                    jfloat y, jint button_state)
        {
            // InnerGestureListener.OnDoubleTapEvent (:97-109): two singles in a row when there is no
            // double-tap handler.
            gesture_native_state* state = peer_state(peer);
            if (state == nullptr)
            {
                return JNI_FALSE;
            }
            if (!has_tap_handler(*state, 2) && has_tap_handler(*state, 1) && action == k_action_up)
            {
                return on_tap(*state, 1, x, y, button_state) ? JNI_TRUE : JNI_FALSE;
            }
            return JNI_FALSE;
        }

        void JNICALL native_on_accessibility_tap(JNIEnv* /*env*/, jclass /*cls*/, jlong peer)
        {
            // GesturePlatformManager.OnKeyPress (:272-284): SendTapped(View, (v) => Point.Zero) on the
            // recognizer HasAccessibleTapGesture picked — no mask re-check, that check IS the picker.
            gesture_native_state* state = peer_state(peer);
            if (state == nullptr || state->sender == nullptr)
            {
                return;
            }
            if (tap_gesture_recognizer* tap = accessible_tap_gesture(*state))
            {
                tap->send_tapped(*state->sender, maui::graphics::point{0, 0});
            }
        }

        jboolean JNICALL native_on_pointer_touch(JNIEnv* /*env*/, jclass /*cls*/, jlong peer, jint action, jfloat x,
                                                 jfloat y, jint button_state, jint action_button)
        {
            // PointerGestureHandler.OnTouch (:58-118). NOTE Cancel maps to EXITED, not Released (:110).
            gesture_native_state* state = peer_state(peer);
            if (state == nullptr || state->sender == nullptr)
            {
                return JNI_FALSE;
            }
            const maui::graphics::point position = view_position(*state, x, y);
            element& sender = *state->sender;
            for (gesture_recognizer* recognizer : state->order)
            {
                auto* pointer = dynamic_cast<pointer_gesture_recognizer*>(recognizer);
                if (pointer == nullptr)
                {
                    continue;
                }
                const buttons_mask current = pressed_button(action_button, button_state);
                // The active-button latch: ACTION_UP/MOVE/CANCEL carry no ActionButton, so they reuse the
                // button the press started with (:12-13, :91, :98, :107).
                buttons_mask effective = current;
                switch (action)
                {
                    case k_action_down:
                        state->active_button = current; // set BEFORE the mask check, exactly as C# does
                        if (!check_pointer_button_mask(*pointer, effective))
                        {
                            continue;
                        }
                        pointer->send_pointer_pressed(sender, position, effective);
                        break;
                    case k_action_move:
                        effective = state->active_button.value_or(current);
                        if (!check_pointer_button_mask(*pointer, effective))
                        {
                            continue;
                        }
                        pointer->send_pointer_moved(sender, position, effective);
                        break;
                    case k_action_up:
                        effective = state->active_button.value_or(current);
                        if (!check_pointer_button_mask(*pointer, effective))
                        {
                            continue;
                        }
                        pointer->send_pointer_released(sender, position, effective);
                        state->active_button.reset();
                        break;
                    case k_action_cancel:
                        effective = state->active_button.value_or(current);
                        if (!check_pointer_button_mask(*pointer, effective))
                        {
                            continue;
                        }
                        pointer->send_pointer_exited(sender, position, effective);
                        state->active_button.reset();
                        break;
                    default:
                        break;
                }
            }
            // TapAndPanGestureDetector :50 — the pointer half reports "handled" purely by existence.
            return has_any<pointer_gesture_recognizer>(*state) ? JNI_TRUE : JNI_FALSE;
        }

        void JNICALL native_on_hover(JNIEnv* /*env*/, jclass /*cls*/, jlong peer, jint action, jfloat x, jfloat y,
                                     jint /*button_state*/)
        {
            // PointerGestureHandler.OnHover (:25-55): hover fans out UNFILTERED (no button mask).
            gesture_native_state* state = peer_state(peer);
            if (state == nullptr || state->sender == nullptr)
            {
                return;
            }
            const maui::graphics::point position = view_position(*state, x, y);
            element& sender = *state->sender;
            for_each_of<pointer_gesture_recognizer>(*state, [&](pointer_gesture_recognizer& pointer) {
                switch (action)
                {
                    case k_action_hover_enter:
                        pointer.send_pointer_entered(sender, position);
                        break;
                    case k_action_hover_move:
                        pointer.send_pointer_moved(sender, position);
                        break;
                    case k_action_hover_exit:
                        pointer.send_pointer_exited(sender, position);
                        break;
                    default:
                        break;
                }
            });
        }

        jboolean JNICALL native_on_scale_begin(JNIEnv* /*env*/, jclass /*cls*/, jlong peer, jfloat focus_x,
                                               jfloat focus_y)
        {
            // InnerScaleListener.OnScaleBegin (:47-50) -> PinchGestureHandler.OnPinchStarted (:52-69).
            gesture_native_state* state = peer_state(peer);
            if (state == nullptr || state->sender == nullptr)
            {
                return JNI_FALSE;
            }
            i_pinch_gesture_controller* pinch = pinch_gesture(*state);
            auto* virtual_view = state->handler != nullptr ? state->handler->virtual_view() : nullptr;
            if (pinch == nullptr || virtual_view == nullptr)
            {
                return JNI_FALSE;
            }
            state->pinch_starting_scale = virtual_view->scale(); // :63
            // :35 / :65 — view-relative UNIT coordinates: FromPixels(focus) / view.Width|Height.
            const maui::graphics::point origin{from_pixels(*state, focus_x) / virtual_view->width(),
                                               from_pixels(*state, focus_y) / virtual_view->height()};
            pinch->send_pinch_started(*state->sender, origin);
            return JNI_TRUE;
        }

        jboolean JNICALL native_on_scale(JNIEnv* /*env*/, jclass /*cls*/, jlong peer, jfloat scale_factor,
                                         jfloat current_span, jfloat previous_span, jfloat focus_x, jfloat focus_y)
        {
            // InnerScaleListener.OnScale (:36-45) -> PinchGestureHandler.OnPinch (:24-39).
            gesture_native_state* state = peer_state(peer);
            if (state == nullptr || state->sender == nullptr)
            {
                return JNI_FALSE;
            }
            if (std::abs(current_span - previous_span) < k_pinch_span_deadband)
            {
                return JNI_FALSE; // the :41-42 deadband
            }
            auto* virtual_view = state->handler != nullptr ? state->handler->virtual_view() : nullptr;
            if (virtual_view == nullptr)
            {
                return JNI_FALSE;
            }
            i_pinch_gesture_controller* pinch = pinch_gesture(*state);
            if (pinch == nullptr)
            {
                return JNI_TRUE; // OnPinch :32-33 returns TRUE with no pinch gesture
            }
            const maui::graphics::point origin{from_pixels(*state, focus_x) / virtual_view->width(),
                                               from_pixels(*state, focus_y) / virtual_view->height()};
            // :36 — Android's ScaleFactor is ALREADY the per-update delta, so the shared
            // pinch_scale_delta helper (a UIPinch/NSMagnification cumulative-reading fixup) is NOT used
            // here; the starting scale is folded in instead.
            pinch->send_pinch(*state->sender,
                              1 + ((static_cast<double>(scale_factor) - 1) * state->pinch_starting_scale), origin);
            return JNI_TRUE;
        }

        void JNICALL native_on_scale_end(JNIEnv* /*env*/, jclass /*cls*/, jlong peer)
        {
            // InnerScaleListener.OnScaleEnd (:52-55) -> PinchGestureHandler.OnPinchEnded (:41-50). NOTE:
            // no IsPinching guard on Android (that guard is the iOS bridge's — see synthetic_pinch).
            gesture_native_state* state = peer_state(peer);
            if (state == nullptr || state->sender == nullptr)
            {
                return;
            }
            if (i_pinch_gesture_controller* pinch = pinch_gesture(*state))
            {
                pinch->send_pinch_ended(*state->sender);
            }
        }

        void JNICALL native_on_drag(JNIEnv* env, jclass /*cls*/, jlong peer, jint action, jfloat /*x*/, jfloat /*y*/,
                                    jlong source_peer, jstring clip_text)
        {
            // DragAndDropGestureHandler.OnDrag (:111-160).
            gesture_native_state* state = peer_state(peer);
            if (state == nullptr || state->sender == nullptr)
            {
                return;
            }
            // `e.LocalState ?? _currentCustomLocalStateData ?? new CustomLocalStateData()` (:114): the
            // drag SOURCE's state owns the package + the accepted operation for the whole drag; a drag
            // from outside the process falls back to this view's own state.
            gesture_native_state* source = peer_state(source_peer);
            gesture_native_state& shared = source != nullptr ? *source : *state;
            if (!shared.drag_package)
            {
                shared.drag_package = std::make_unique<data_package>(); // :122-126
            }
            data_package& package = *shared.drag_package;

            std::optional<std::string> text;
            if (clip_text != nullptr)
            {
                text = maui::platform::android::to_utf8(env, clip_text);
            }

            switch (action)
            {
                case k_drag_ended: // :132-140
                    shared.drag_package.reset();
                    if (source != nullptr && source->sender != nullptr)
                    {
                        handle_drop_completed(*source);
                    }
                    break;
                case k_drag_started: // :141-142 — nothing to do
                    break;
                case k_drag_location: // :143-145
                case k_drag_entered:  // :151-153
                    handle_drag_hover(*state, shared, package, /*leaving=*/false);
                    break;
                case k_drag_exited: // :154-156
                    handle_drag_hover(*state, shared, package, /*leaving=*/true);
                    break;
                case k_drag_drop: // :146-150
                    handle_drop(*state, shared, package, /*from_outside=*/source == nullptr, text);
                    break;
                default:
                    break;
            }
        }

        // Binds every native half to the bridge class. Idempotent (RegisterNatives replaces an existing
        // binding), so re-syncing managers need no once-flag coordination — same contract as
        // button_handler.cpp's register_click_natives.
        [[nodiscard]] bool register_gesture_natives(JNIEnv* env, jclass bridge_class)
        {
            // JNINativeMethod's name/signature are non-const char* and fnPtr is void* for historical
            // JNI-spec reasons — the casts are the API's own shape.
            static const std::array<JNINativeMethod, 14> k_methods{
                JNINativeMethod{.name = const_cast<char*>("nativeOnDown"),
                                .signature = const_cast<char*>("(JFF)Z"),
                                .fnPtr = reinterpret_cast<void*>(&native_on_down)},
                JNINativeMethod{.name = const_cast<char*>("nativeOnScroll"),
                                .signature = const_cast<char*>("(JFFFFI)Z"),
                                .fnPtr = reinterpret_cast<void*>(&native_on_scroll)},
                JNINativeMethod{.name = const_cast<char*>("nativeEndScrolling"),
                                .signature = const_cast<char*>("(J)V"),
                                .fnPtr = reinterpret_cast<void*>(&native_end_scrolling)},
                JNINativeMethod{.name = const_cast<char*>("nativeOnLongPress"),
                                .signature = const_cast<char*>("(JFF)V"),
                                .fnPtr = reinterpret_cast<void*>(&native_on_long_press)},
                JNINativeMethod{.name = const_cast<char*>("nativeOnSingleTapUp"),
                                .signature = const_cast<char*>("(JFFI)Z"),
                                .fnPtr = reinterpret_cast<void*>(&native_on_single_tap_up)},
                JNINativeMethod{.name = const_cast<char*>("nativeOnSingleTapConfirmed"),
                                .signature = const_cast<char*>("(JFFI)Z"),
                                .fnPtr = reinterpret_cast<void*>(&native_on_single_tap_confirmed)},
                JNINativeMethod{.name = const_cast<char*>("nativeOnDoubleTap"),
                                .signature = const_cast<char*>("(JFFI)Z"),
                                .fnPtr = reinterpret_cast<void*>(&native_on_double_tap)},
                JNINativeMethod{.name = const_cast<char*>("nativeOnDoubleTapEvent"),
                                .signature = const_cast<char*>("(JIFFI)Z"),
                                .fnPtr = reinterpret_cast<void*>(&native_on_double_tap_event)},
                JNINativeMethod{.name = const_cast<char*>("nativeOnAccessibilityTap"),
                                .signature = const_cast<char*>("(J)V"),
                                .fnPtr = reinterpret_cast<void*>(&native_on_accessibility_tap)},
                JNINativeMethod{.name = const_cast<char*>("nativeOnPointerTouch"),
                                .signature = const_cast<char*>("(JIFFII)Z"),
                                .fnPtr = reinterpret_cast<void*>(&native_on_pointer_touch)},
                JNINativeMethod{.name = const_cast<char*>("nativeOnHover"),
                                .signature = const_cast<char*>("(JIFFI)V"),
                                .fnPtr = reinterpret_cast<void*>(&native_on_hover)},
                JNINativeMethod{.name = const_cast<char*>("nativeOnScale"),
                                .signature = const_cast<char*>("(JFFFFF)Z"),
                                .fnPtr = reinterpret_cast<void*>(&native_on_scale)},
                JNINativeMethod{.name = const_cast<char*>("nativeOnScaleBegin"),
                                .signature = const_cast<char*>("(JFF)Z"),
                                .fnPtr = reinterpret_cast<void*>(&native_on_scale_begin)},
                JNINativeMethod{.name = const_cast<char*>("nativeOnScaleEnd"),
                                .signature = const_cast<char*>("(J)V"),
                                .fnPtr = reinterpret_cast<void*>(&native_on_scale_end)},
            };
            static const JNINativeMethod k_drag_method{.name = const_cast<char*>("nativeOnDrag"),
                                                       .signature = const_cast<char*>("(JIFFJLjava/lang/String;)V"),
                                                       .fnPtr = reinterpret_cast<void*>(&native_on_drag)};
            if (env->RegisterNatives(bridge_class, k_methods.data(), static_cast<jint>(k_methods.size())) != JNI_OK ||
                env->RegisterNatives(bridge_class, &k_drag_method, 1) != JNI_OK)
            {
                clear_pending(env);
                return false;
            }
            return true;
        }
    } // namespace

    gesture_platform_manager::gesture_platform_manager() = default;

    gesture_platform_manager::~gesture_platform_manager()
    {
        native_detach_all(); // uninstalls every listener; native_state_ then frees itself
    }

    void gesture_platform_manager::native_attach(const std::shared_ptr<gesture_recognizer>& recognizer)
    {
        if (recognizer == nullptr || sender_ == nullptr)
        {
            return;
        }
        if (!native_state_)
        {
            native_state_ = std::make_unique<gesture_native_state>();
            register_live_state(native_state_.get()); // the peer is only valid while it is registered
        }
        gesture_native_state& state = *native_state_;
        state.sender = sender_;
        state.handler = handler_;

        // INSERT into the backend table (never attached_ — see the file header's ordering trap).
        if (std::ranges::find(state.order, recognizer.get()) == state.order.end())
        {
            state.order.push_back(recognizer.get());
        }

        // Create the one per-view bridge on first use (C#'s Lazy<TapAndPanGestureDetector> +
        // Lazy<ScaleGestureDetector> + the two Lazy<> handlers, all collapsed into one object).
        if (state.bridge.get() == nullptr)
        {
            auto* native = handler_ != nullptr ? static_cast<jobject>(handler_->native_view()) : nullptr;
            const scoped_env env;
            if (native != nullptr && env)
            {
                auto& cache = default_jni_cache();
                jclass bridge_class = cache.find_class(env.get(), k_bridge_class);
                jmethodID ctor = cache.method(env.get(), k_bridge_class, "<init>", "(Landroid/view/View;J)V");
                if (bridge_class != nullptr && ctor != nullptr && register_gesture_natives(env.get(), bridge_class))
                {
                    state.view = maui::platform::android::global_ref<jobject>{env.get(), native};
                    const local_ref<jobject> bridge{
                        env.get(), env->NewObject(bridge_class, ctor, native, reinterpret_cast<jlong>(&state))};
                    if (!clear_pending(env.get()) && bridge)
                    {
                        state.bridge = maui::platform::android::global_ref<jobject>{env.get(), bridge.get()};
                    }
                }
                // Without the host-provided bridge class the gesture channel stays C++-only — exactly the
                // VM-less degradation every android handler partial documents.
            }
        }
        sync_subscriptions(state);
    }

    void gesture_platform_manager::native_detach(const gesture_recognizer& recognizer)
    {
        if (!native_state_)
        {
            return;
        }
        gesture_native_state& state = *native_state_;
        // ERASE from the backend table, then re-run the same whole-collection sync.
        std::erase(state.order, &recognizer);
        state.sender = sender_;
        state.handler = handler_;
        sync_subscriptions(state);
    }

    void gesture_platform_manager::native_detach_all()
    {
        if (!native_state_)
        {
            return;
        }
        gesture_native_state& state = *native_state_;
        // Invalidate the Java back-ref FIRST: a MotionEvent already queued in the platform detectors must
        // not reach this storage after it dies (C#'s "resurrect the eagerly-disposed listener" hazard,
        // InnerGestureListener.cs:69-74 — spelled here as an explicit peer zeroing).
        const scoped_env env;
        jobject bridge = state.bridge.get();
        jobject view = state.view.get();
        if (env && bridge != nullptr)
        {
            if (jmethodID detach = default_jni_cache().method(env.get(), k_bridge_class, "detach", "()V"))
            {
                env->CallVoidMethod(bridge, detach);
                clear_pending(env.get());
            }
        }
        // SetupElement's teardown (:306-314): restore Focusable, then clear every subscription.
        if (env && view != nullptr)
        {
            if (state.focusable_default)
            {
                if (jmethodID set_focusable =
                        default_jni_cache().method(env.get(), k_view_class, "setFocusable", "(Z)V"))
                {
                    env->CallVoidMethod(view, set_focusable, *state.focusable_default ? JNI_TRUE : JNI_FALSE);
                    clear_pending(env.get());
                }
            }
            set_view_listener(env.get(), view, nullptr, "setOnTouchListener", "(Landroid/view/View$OnTouchListener;)V",
                              false);
            set_view_listener(env.get(), view, nullptr, "setOnKeyListener", "(Landroid/view/View$OnKeyListener;)V",
                              false);
            set_view_listener(env.get(), view, nullptr, "setOnHoverListener", "(Landroid/view/View$OnHoverListener;)V",
                              false);
            set_view_listener(env.get(), view, nullptr, "setOnDragListener", "(Landroid/view/View$OnDragListener;)V",
                              false);
        }
        // Any peer still in flight (an OS drag session's LocalState) now resolves to null instead of to
        // freed storage. Residual ABA: if a later state lands on this exact address before the drag ends,
        // that drag's Ended reaches the WRONG view — a spurious DropCompleted, not a crash. Not worth a
        // generation counter until a real drag exposes it.
        unregister_live_state(&state);
        native_state_.reset();
    }

    // --- drag&drop (W2-22): unlike apple/ios — whose "attachment-only" stance exists solely because a
    // UIDrag/UIDropSession is undrivable in the spawned simulator lane — Android CAN report the real
    // registration: the OnDragListener is either installed or it is not, and the long-press drag arm is
    // either on or off. ---
    bool gesture_platform_manager::native_registered_drag_source(const gesture_recognizer& recognizer) const
    {
        if (!native_state_ || dynamic_cast<const drag_gesture_recognizer*>(&recognizer) == nullptr)
        {
            return false;
        }
        const gesture_native_state& state = *native_state_;
        return state.long_press_enabled && std::ranges::find(state.order, &recognizer) != state.order.end();
    }

    bool gesture_platform_manager::native_registered_drop_target(const gesture_recognizer& recognizer) const
    {
        if (!native_state_ || dynamic_cast<const drop_gesture_recognizer*>(&recognizer) == nullptr)
        {
            return false;
        }
        const gesture_native_state& state = *native_state_;
        return state.drag_installed && std::ranges::find(state.order, &recognizer) != state.order.end();
    }
    // --- end drag&drop (W2-22) ---
} // namespace maui::controls
