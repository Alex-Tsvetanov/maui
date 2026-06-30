// Android stepper seam tests (M-android per-control fan-out) — the headless/iOS stepper recipe replayed
// over JNI, ON the emulator inside the app_process widget test host (tools/android-testhost-run.sh): the
// REAL maui::controls::stepper drives the REAL MauiStepper (a horizontal android.widget.LinearLayout
// hosting two plain android.widget.Buttons, "－" down + "＋" up) through the cross-platform stepper_handler's
// android partial (src/platform/android/stepper_handler.cpp), and every assertion reads the widget state
// BACK through JNI getters — both directions of the seam:
//   virtual → native: set virtual-view properties, read the container/buttons (getChildCount/getChildAt/
//                     getText/getContentDescription/isEnabled on each button; getVisibility/getAlpha/
//                     getContentDescription on the LinearLayout).
//   native → virtual: the per-button android.view.View.OnClickListener install is DEFERRED (no host
//                     listener class — only dev.mauicpp.NativeOnClickListener exists, wired for a single
//                     button peer), exactly like the slider's OnSeekBarChangeListener, so the deferred-but-
//                     invokable C++ callbacks (on_value_changed carrying StepperProxy.OnValueChanged;
//                     on_minus/on_plus carrying StepperHandlerManager.StepperListener.OnClick's Value ±=
//                     Interval) are driven directly — the documented stand-in for the real trampoline
//                     (header note in stepper_handler.cpp).
//
// Characterization target: StepperHandler.Android.cs + StepperHandlerManager.cs (UpdateButtons + the
// StepperListener) + StepperExtensions.cs + MauiStepper.cs. THE ENABLED LOGIC (read from the oracle, NOT
// guessed): downButton.Enabled = IsEnabled && Value > Minimum; upButton.Enabled = IsEnabled && Value <
// Maximum. The documented plain-widget deviations (a bare LinearLayout standing in for MauiStepper, direct
// button references instead of the Text-matching re-scan, deferred click) are in the partial's header.
//
// TESTHOST CONSTRUCTION CAVEAT (android lesson 3): the two stepper children are android.widget.Buttons —
// TextView-derived. The button seam suite proves a plain Button constructs + accepts setText in the bare
// app_process host (its ctor's TextClassification path is reachable there for a non-interactive Button), so
// the LinearLayout + two Buttons are expected to construct. IF, on the emulator, the child Buttons' setText
// or the LinearLayout.addView trips the AutofillManager/TextClassification SecurityException, the
// integrator leaves THIS file unwired (app-host-only verification, like editor/switch/check_box) and notes
// whether it was the LinearLayout itself or the child Buttons that tripped — the attach test below is the
// canary (it asserts the container constructs AND both buttons attach).

#include <memory>
#include <string>

#include <gtest/gtest.h>
#include <jni.h>

#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/controls/stepper.hpp"
#include "maui/core/stepper_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/size.hpp"
#include "testhost/test_host.hpp"

namespace
{
    using maui::controls::stepper;
    using maui::core::stepper_handler;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;
    using maui::platform::android::to_utf8;

    constexpr const char* k_linear_layout_class = "android/widget/LinearLayout";
    constexpr const char* k_button_class = "android/widget/Button";

    // The oracle's exact down/up glyph Text (StepperHandlerManager.CreateStepperButtons): "－" (U+FF0D
    // FULLWIDTH HYPHEN-MINUS) / "＋" (U+FF0B FULLWIDTH PLUS) — NOT the keyboard dash/plus.
    const std::string k_down_text = "\xEF\xBC\x8D"; // U+FF0D
    const std::string k_up_text = "\xEF\xBC\x8B";   // U+FF0B
    // The phonetic ContentDescriptions: "−" (U+2212 MINUS SIGN) / "+" (U+002B PLUS SIGN).
    const std::string k_down_description = "\xE2\x88\x92"; // U+2212
    const std::string k_up_description = "+";              // U+002B

    // android.view.View visibility states / ViewExtensions.ToPlatformVisibility's targets.
    constexpr jint k_view_visible = 0;
    constexpr jint k_view_invisible = 4;
    constexpr jint k_view_gone = 8;

    // Fails the test (and clears the pending state) when a Java exception is pending.
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

    // The control + attached handler + the real widgets, torn down in declaration order (the handler detach
    // precedes the control's death; the platform struct releases the panel + both buttons' global refs).
    struct attached_stepper
    {
        stepper control;
        std::shared_ptr<stepper_handler> handler = std::make_shared<stepper_handler>();

        attached_stepper()
        {
            control.set_handler(handler);
        }

        ~attached_stepper()
        {
            control.set_handler(nullptr);
        }

        attached_stepper(const attached_stepper&) = delete;
        attached_stepper(attached_stepper&&) = delete;
        attached_stepper& operator=(const attached_stepper&) = delete;
        attached_stepper& operator=(attached_stepper&&) = delete;

        [[nodiscard]] jobject panel() const
        {
            auto* platform = handler->typed_platform_view();
            return platform != nullptr ? static_cast<jobject>(platform->native) : nullptr;
        }
        [[nodiscard]] jobject down_button() const
        {
            auto* platform = handler->typed_platform_view();
            return platform != nullptr ? static_cast<jobject>(platform->down_button) : nullptr;
        }
        [[nodiscard]] jobject up_button() const
        {
            auto* platform = handler->typed_platform_view();
            return platform != nullptr ? static_cast<jobject>(platform->up_button) : nullptr;
        }
    };

    // ---- typed JNI read-backs (each fails the test and returns a benign default on a JNI error) ----

    [[nodiscard]] jint call_int(JNIEnv* env, jobject obj, const char* klass, const char* name)
    {
        jmethodID method = default_jni_cache().method(env, klass, name, "()I");
        if (method == nullptr)
        {
            ADD_FAILURE() << name << "()I not found";
            return 0;
        }
        const jint value = env->CallIntMethod(obj, method);
        return pending_exception_cleared(env, name) ? 0 : value;
    }

    [[nodiscard]] jfloat call_float(JNIEnv* env, jobject obj, const char* klass, const char* name)
    {
        jmethodID method = default_jni_cache().method(env, klass, name, "()F");
        if (method == nullptr)
        {
            ADD_FAILURE() << name << "()F not found";
            return 0;
        }
        const jfloat value = env->CallFloatMethod(obj, method);
        return pending_exception_cleared(env, name) ? 0 : value;
    }

    [[nodiscard]] bool call_bool(JNIEnv* env, jobject obj, const char* klass, const char* name)
    {
        jmethodID method = default_jni_cache().method(env, klass, name, "()Z");
        if (method == nullptr)
        {
            ADD_FAILURE() << name << "()Z not found";
            return false;
        }
        const jboolean value = env->CallBooleanMethod(obj, method);
        return !pending_exception_cleared(env, name) && value == JNI_TRUE;
    }

    // CharSequence-returning getter → UTF-8 (empty + a failure on any JNI error).
    [[nodiscard]] std::string call_char_sequence(JNIEnv* env, jobject obj, const char* klass, const char* name)
    {
        auto& cache = default_jni_cache();
        jmethodID method = cache.method(env, klass, name, "()Ljava/lang/CharSequence;");
        jmethodID to_string = cache.method(env, "java/lang/Object", "toString", "()Ljava/lang/String;");
        if (method == nullptr || to_string == nullptr)
        {
            ADD_FAILURE() << name << " surface missing";
            return {};
        }
        const local_ref<jobject> sequence{env, env->CallObjectMethod(obj, method)};
        if (pending_exception_cleared(env, name) || !sequence)
        {
            ADD_FAILURE() << name << " returned null";
            return {};
        }
        const local_ref<jstring> text{env, static_cast<jstring>(env->CallObjectMethod(sequence.get(), to_string))};
        if (pending_exception_cleared(env, "CharSequence.toString") || !text)
        {
            return {};
        }
        return to_utf8(env, text.get());
    }

    // ---- virtual → native ----

    TEST(android_stepper, attach_creates_a_linear_layout_hosting_two_buttons)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        const attached_stepper seam;
        // The canary (see the testhost caveat in the header): the container constructs AND both buttons
        // attach. If this fails with a SecurityException, the integrator leaves the suite unwired and notes
        // whether the LinearLayout or the child Buttons tripped.
        ASSERT_NE(seam.panel(), nullptr) << "the android partial did not create the MauiStepper LinearLayout";
        jclass layout_class = default_jni_cache().find_class(env.get(), k_linear_layout_class);
        ASSERT_NE(layout_class, nullptr);
        EXPECT_EQ(env->IsInstanceOf(seam.panel(), layout_class), JNI_TRUE);
        // StepperHandler.CreatePlatformView: AddView(downButton); AddView(upButton) — two children.
        EXPECT_EQ(call_int(env.get(), seam.panel(), k_linear_layout_class, "getChildCount"), 2);
        ASSERT_NE(seam.down_button(), nullptr) << "the down button was not created";
        ASSERT_NE(seam.up_button(), nullptr) << "the up button was not created";
        jclass button_class = default_jni_cache().find_class(env.get(), k_button_class);
        ASSERT_NE(button_class, nullptr);
        EXPECT_EQ(env->IsInstanceOf(seam.down_button(), button_class), JNI_TRUE);
        EXPECT_EQ(env->IsInstanceOf(seam.up_button(), button_class), JNI_TRUE);
    }

    TEST(android_stepper, the_two_buttons_carry_the_oracle_glyphs_and_descriptions)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_stepper seam;
        ASSERT_NE(seam.down_button(), nullptr);
        ASSERT_NE(seam.up_button(), nullptr);
        // CreateStepperButtons: the visually-pleasing fullwidth glyph Text + the phonetic ContentDescription.
        EXPECT_EQ(call_char_sequence(env.get(), seam.down_button(), k_button_class, "getText"), k_down_text);
        EXPECT_EQ(call_char_sequence(env.get(), seam.up_button(), k_button_class, "getText"), k_up_text);
        EXPECT_EQ(call_char_sequence(env.get(), seam.down_button(), k_button_class, "getContentDescription"),
                  k_down_description);
        EXPECT_EQ(call_char_sequence(env.get(), seam.up_button(), k_button_class, "getContentDescription"),
                  k_up_description);
    }

    TEST(android_stepper, button_enabled_state_tracks_value_against_the_bounds)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_stepper seam;
        ASSERT_NE(seam.down_button(), nullptr);
        ASSERT_NE(seam.up_button(), nullptr);
        // StepperHandlerManager.UpdateButtons: down enabled iff Value > Minimum; up enabled iff Value <
        // Maximum. Range [0,10]; at the floor only the up button is live.
        seam.control.set_minimum(0.0);
        seam.control.set_maximum(10.0);
        seam.control.set_value(0.0);
        EXPECT_FALSE(call_bool(env.get(), seam.down_button(), k_button_class, "isEnabled"))
            << "minus must be disabled at the minimum";
        EXPECT_TRUE(call_bool(env.get(), seam.up_button(), k_button_class, "isEnabled"))
            << "plus must be enabled below the maximum";
        // Mid-range: both live.
        seam.control.set_value(5.0);
        EXPECT_TRUE(call_bool(env.get(), seam.down_button(), k_button_class, "isEnabled"));
        EXPECT_TRUE(call_bool(env.get(), seam.up_button(), k_button_class, "isEnabled"));
        // At the ceiling: only the down button is live.
        seam.control.set_value(10.0);
        EXPECT_TRUE(call_bool(env.get(), seam.down_button(), k_button_class, "isEnabled"));
        EXPECT_FALSE(call_bool(env.get(), seam.up_button(), k_button_class, "isEnabled"))
            << "plus must be disabled at the maximum";
    }

    TEST(android_stepper, disabling_the_stepper_disables_both_buttons)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_stepper seam;
        ASSERT_NE(seam.down_button(), nullptr);
        ASSERT_NE(seam.up_button(), nullptr);
        // Put the value mid-range so BOTH buttons would otherwise be live (isolates the IsEnabled gate).
        seam.control.set_minimum(0.0);
        seam.control.set_maximum(10.0);
        seam.control.set_value(5.0);
        EXPECT_TRUE(call_bool(env.get(), seam.down_button(), k_button_class, "isEnabled"));
        EXPECT_TRUE(call_bool(env.get(), seam.up_button(), k_button_class, "isEnabled"));
        // StepperHandlerManager.UpdateButtons: both buttons die when IsEnabled is false (the MapIsEnabled →
        // UpdateButtons landing, driven through the platform struct's update_is_enabled).
        seam.control.set_is_enabled(false);
        EXPECT_FALSE(call_bool(env.get(), seam.down_button(), k_button_class, "isEnabled"));
        EXPECT_FALSE(call_bool(env.get(), seam.up_button(), k_button_class, "isEnabled"));
        // Re-enabling restores the value-vs-bounds state (mid-range → both live again).
        seam.control.set_is_enabled(true);
        EXPECT_TRUE(call_bool(env.get(), seam.down_button(), k_button_class, "isEnabled"));
        EXPECT_TRUE(call_bool(env.get(), seam.up_button(), k_button_class, "isEnabled"));
    }

    TEST(android_stepper, visibility_maps_to_the_three_view_states)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_stepper seam;
        ASSERT_NE(seam.panel(), nullptr);
        EXPECT_EQ(call_int(env.get(), seam.panel(), k_linear_layout_class, "getVisibility"), k_view_visible);
        seam.control.set_visibility(maui::core::visibility::hidden);
        EXPECT_EQ(call_int(env.get(), seam.panel(), k_linear_layout_class, "getVisibility"), k_view_invisible);
        seam.control.set_visibility(maui::core::visibility::collapsed);
        EXPECT_EQ(call_int(env.get(), seam.panel(), k_linear_layout_class, "getVisibility"), k_view_gone);
        seam.control.set_visibility(maui::core::visibility::visible);
        EXPECT_EQ(call_int(env.get(), seam.panel(), k_linear_layout_class, "getVisibility"), k_view_visible);
    }

    TEST(android_stepper, opacity_reaches_the_panel_alpha)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_stepper seam;
        seam.control.set_opacity(0.5);
        ASSERT_NE(seam.panel(), nullptr);
        EXPECT_NEAR(call_float(env.get(), seam.panel(), k_linear_layout_class, "getAlpha"), 0.5F, 1e-4F);
    }

    TEST(android_stepper, automation_id_reaches_the_panel_content_description)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_stepper seam;
        seam.control.set_automation_id("quantity_stepper");
        ASSERT_NE(seam.panel(), nullptr);
        EXPECT_EQ(call_char_sequence(env.get(), seam.panel(), k_linear_layout_class, "getContentDescription"),
                  "quantity_stepper");
    }

    TEST(android_stepper, measure_returns_a_positive_size)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_stepper seam;
        ASSERT_NE(seam.panel(), nullptr);
        const maui::graphics::size measured = seam.handler->get_desired_size(1000, 1000);
        EXPECT_GT(measured.width, 0);
        EXPECT_GT(measured.height, 0);
    }

    // ---- native → virtual (the deferred, but invokable, click channel) ----

    TEST(android_stepper, plus_callback_increments_the_value_by_the_interval)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_stepper seam;
        auto* platform = seam.handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        ASSERT_TRUE(static_cast<bool>(platform->on_plus));
        seam.control.set_minimum(0.0);
        seam.control.set_maximum(10.0);
        seam.control.set_increment(2.0);
        seam.control.set_value(4.0);
        int changes = 0;
        seam.control.value_changed.connect([&changes](double, double) { ++changes; });
        // StepperHandlerManager.StepperListener.OnClick (up button): Value += Interval. The real per-button
        // OnClickListener install is deferred (no host listener class), so drive the deferred-but-invokable
        // callback directly — the documented stand-in for the trampoline.
        platform->on_plus();
        EXPECT_DOUBLE_EQ(seam.control.value(), 6.0);
        EXPECT_EQ(changes, 1);
    }

    TEST(android_stepper, minus_callback_decrements_the_value_by_the_interval)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_stepper seam;
        auto* platform = seam.handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        ASSERT_TRUE(static_cast<bool>(platform->on_minus));
        seam.control.set_minimum(0.0);
        seam.control.set_maximum(10.0);
        seam.control.set_increment(2.0);
        seam.control.set_value(4.0);
        // StepperListener.OnClick (down button): increment = -Interval, so Value -= Interval.
        platform->on_minus();
        EXPECT_DOUBLE_EQ(seam.control.value(), 2.0);
    }

    TEST(android_stepper, value_changed_callback_writes_back_through_the_virtual_view)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_stepper seam;
        auto* platform = seam.handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        ASSERT_TRUE(static_cast<bool>(platform->on_value_changed));
        // StepperProxy.OnValueChanged: the mirror's `value` carries the resolved native value; the callback
        // writes it back through i_range::set_value (the same channel the cross-platform suite drives).
        seam.control.set_maximum(10.0);
        platform->value = 7.0;
        platform->on_value_changed();
        EXPECT_DOUBLE_EQ(seam.control.value(), 7.0);
    }

    TEST(android_stepper, callbacks_are_dropped_on_disconnect)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        stepper control;
        auto handler = std::make_shared<stepper_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        ASSERT_TRUE(static_cast<bool>(platform->on_value_changed));
        // DisconnectHandler drops the callbacks (the native per-button listener uninstall is deferred with
        // the install). set_handler(nullptr) would destroy the platform struct before it could be probed,
        // so invoke the static disconnect directly (the same body set_handler runs), then detach.
        stepper_handler::on_disconnect_handler(*platform);
        EXPECT_FALSE(static_cast<bool>(platform->on_value_changed));
        EXPECT_FALSE(static_cast<bool>(platform->on_minus));
        EXPECT_FALSE(static_cast<bool>(platform->on_plus));
        control.set_handler(nullptr);
    }
} // namespace
