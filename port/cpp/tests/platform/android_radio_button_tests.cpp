// Android radio button seam tests (M-android fan-out) — M6's iOS Rosetta Stone replayed over JNI, ON the
// emulator inside the app_process widget test host (tools/android-testhost-run.sh): the REAL
// maui::controls::radio_button drives the REAL android.widget.RadioButton through the cross-platform
// radio_button_handler's android partial (src/platform/android/radio_button_handler.cpp), and every
// assertion reads the widget state BACK through JNI getters — both directions of the seam:
//   virtual → native: set virtual-view properties, read the widget (isChecked/isEnabled/getVisibility/
//                     getAlpha/getContentDescription/getText/getCurrentTextColor...)
//   native → virtual: check the widget + invoke the handler's on_select callback (the deferred-listener
//                     seam — see the partial's header) → send_is_checked(true) → the control's checked
//                     visual-state + group exclusion.
// Characterization target: RadioButtonHandler.Android.cs + RadioButtonExtensions.cs + TextViewExtensions.cs
// (the documented plain-widget deviations — AppCompat widget, deferred CheckedChange listener, deferred
// BorderDrawable stroke/corner — are in the partial's header).
//
// IMPORTANT — this suite is BEST-EFFORT and likely UNWIRED in the testhost. RadioButton extends TextView,
// so per the macOS/Android resume LESSON 3 ("TextView-derived INTERACTIVE widgets cannot be constructed in
// the app_process testhost") its ctor's setText path triggers an AutofillManager / TextClassification
// ContentProvider query the shell-uid process cannot reach → SecurityException, yielding a null widget
// (create_platform_view's documented degradation). The CompoundButton family (check_box) hits the same
// wall. The file is kept for the FUTURE Android app host (a real Activity), where the widget constructs and
// these read-backs become live — exactly the disposition android_check_box_tests.cpp records.

#include <memory>
#include <string>

#include <gtest/gtest.h>
#include <jni.h>

#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/controls/radio_button.hpp"
#include "maui/core/radio_button_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "testhost/test_host.hpp"

namespace
{
    using maui::controls::radio_button;
    using maui::core::radio_button_handler;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;
    using maui::platform::android::to_utf8;

    constexpr const char* k_radio_button_class = "android/widget/RadioButton";

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

    // The control + attached handler + the real widget, torn down in declaration order (the handler
    // detach precedes the control's death; the platform struct releases the widget's global ref).
    struct attached_radio_button
    {
        radio_button control;
        std::shared_ptr<radio_button_handler> handler = std::make_shared<radio_button_handler>();

        attached_radio_button()
        {
            control.set_handler(handler);
        }

        ~attached_radio_button()
        {
            control.set_handler(nullptr);
        }

        attached_radio_button(const attached_radio_button&) = delete;
        attached_radio_button(attached_radio_button&&) = delete;
        attached_radio_button& operator=(const attached_radio_button&) = delete;
        attached_radio_button& operator=(attached_radio_button&&) = delete;

        [[nodiscard]] jobject widget() const
        {
            auto* platform = handler->typed_platform_view();
            return platform != nullptr ? static_cast<jobject>(platform->native) : nullptr;
        }
    };

    // ---- typed JNI read-backs (each fails the test and returns a benign default on a JNI error) ----

    [[nodiscard]] jint call_int(JNIEnv* env, jobject widget, const char* name)
    {
        jmethodID method = default_jni_cache().method(env, k_radio_button_class, name, "()I");
        if (method == nullptr)
        {
            ADD_FAILURE() << name << "()I not found";
            return 0;
        }
        const jint value = env->CallIntMethod(widget, method);
        return pending_exception_cleared(env, name) ? 0 : value;
    }

    [[nodiscard]] jfloat call_float(JNIEnv* env, jobject widget, const char* name)
    {
        jmethodID method = default_jni_cache().method(env, k_radio_button_class, name, "()F");
        if (method == nullptr)
        {
            ADD_FAILURE() << name << "()F not found";
            return 0;
        }
        const jfloat value = env->CallFloatMethod(widget, method);
        return pending_exception_cleared(env, name) ? 0 : value;
    }

    [[nodiscard]] bool call_bool(JNIEnv* env, jobject widget, const char* name)
    {
        jmethodID method = default_jni_cache().method(env, k_radio_button_class, name, "()Z");
        if (method == nullptr)
        {
            ADD_FAILURE() << name << "()Z not found";
            return false;
        }
        const jboolean value = env->CallBooleanMethod(widget, method);
        return !pending_exception_cleared(env, name) && value == JNI_TRUE;
    }

    // CharSequence-returning getter → UTF-8 (empty + a failure on any JNI error).
    [[nodiscard]] std::string call_char_sequence(JNIEnv* env, jobject widget, const char* name)
    {
        auto& cache = default_jni_cache();
        jmethodID method = cache.method(env, k_radio_button_class, name, "()Ljava/lang/CharSequence;");
        jmethodID to_string = cache.method(env, "java/lang/Object", "toString", "()Ljava/lang/String;");
        if (method == nullptr || to_string == nullptr)
        {
            ADD_FAILURE() << name << " surface missing";
            return {};
        }
        const local_ref<jobject> sequence{env, env->CallObjectMethod(widget, method)};
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

    TEST(android_radio_button, attach_creates_a_real_android_widget_radio_button)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        const attached_radio_button seam;
        ASSERT_NE(seam.widget(), nullptr) << "the android partial did not create a widget";
        jclass radio_button_class = default_jni_cache().find_class(env.get(), k_radio_button_class);
        ASSERT_NE(radio_button_class, nullptr);
        EXPECT_EQ(env->IsInstanceOf(seam.widget(), radio_button_class), JNI_TRUE);
    }

    TEST(android_radio_button, is_checked_reaches_the_widget)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_radio_button seam;
        ASSERT_NE(seam.widget(), nullptr);
        // A default radio button is unchecked (RadioButton.IsChecked default false).
        EXPECT_FALSE(call_bool(env.get(), seam.widget(), "isChecked"));
        seam.control.set_is_checked(true);
        EXPECT_TRUE(call_bool(env.get(), seam.widget(), "isChecked"));
        seam.control.set_is_checked(false);
        EXPECT_FALSE(call_bool(env.get(), seam.widget(), "isChecked"));
    }

    TEST(android_radio_button, content_reaches_the_widget_text)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_radio_button seam;
        ASSERT_NE(seam.widget(), nullptr);
        // RadioButtonExtensions.UpdateContent: platformRadioButton.Text = $"{radioButton.Content}".
        seam.control.set_content("Option A");
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getText"), "Option A");
    }

    TEST(android_radio_button, text_color_reaches_the_widget_as_argb)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_radio_button seam;
        ASSERT_NE(seam.widget(), nullptr);
        // TextViewExtensions.UpdateTextColor: SetTextColor(textColor.ToPlatform()); getCurrentTextColor
        // reads it back as a packed ARGB int.
        const maui::graphics::color red(1.0F, 0.0F, 0.0F);
        seam.control.set_text_color(red);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getCurrentTextColor"), static_cast<jint>(red.to_int()));
    }

    TEST(android_radio_button, character_spacing_reaches_the_widget_letter_spacing)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_radio_button seam;
        ASSERT_NE(seam.widget(), nullptr);
        // TextViewExtensions.UpdateCharacterSpacing: LetterSpacing = CharacterSpacing.ToEm()
        // (EmCoefficient 0.0624). getLetterSpacing reads the em value back.
        seam.control.set_character_spacing(10.0);
        EXPECT_NEAR(call_float(env.get(), seam.widget(), "getLetterSpacing"), 10.0F * 0.0624F, 1e-4F);
    }

    TEST(android_radio_button, is_enabled_reaches_the_widget)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_radio_button seam;
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_TRUE(call_bool(env.get(), seam.widget(), "isEnabled"));
        seam.control.set_is_enabled(false);
        EXPECT_FALSE(call_bool(env.get(), seam.widget(), "isEnabled"));
        seam.control.set_is_enabled(true);
        EXPECT_TRUE(call_bool(env.get(), seam.widget(), "isEnabled"));
    }

    TEST(android_radio_button, visibility_maps_to_the_three_view_states)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_radio_button seam;
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_visible);
        seam.control.set_visibility(maui::core::visibility::hidden);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_invisible);
        seam.control.set_visibility(maui::core::visibility::collapsed);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_gone);
        seam.control.set_visibility(maui::core::visibility::visible);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_visible);
    }

    TEST(android_radio_button, opacity_reaches_the_widget_alpha)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_radio_button seam;
        seam.control.set_opacity(0.5);
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_NEAR(call_float(env.get(), seam.widget(), "getAlpha"), 0.5F, 1e-4F);
    }

    TEST(android_radio_button, automation_id_reaches_the_content_description)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_radio_button seam;
        seam.control.set_automation_id("pick_first");
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getContentDescription"), "pick_first");
    }

    TEST(android_radio_button, measure_returns_a_positive_size)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_radio_button seam;
        ASSERT_NE(seam.widget(), nullptr);
        seam.control.set_content("Pick me"); // a radio with content has a measurable text + indicator
        const maui::graphics::size measured = seam.handler->get_desired_size(1000, 1000);
        EXPECT_GT(measured.width, 0);
        EXPECT_GT(measured.height, 0);
    }

    // ---- native → virtual ----

    TEST(android_radio_button, native_tap_selects_through_the_on_select_callback)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_radio_button seam;
        auto* platform = seam.handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        ASSERT_NE(platform->native, nullptr);
        ASSERT_TRUE(static_cast<bool>(platform->on_select))
            << "on_connect must wire the (deferred-listener) select callback";

        // Simulate the user tapping the widget: drive the REAL RadioButton checked state, then invoke the
        // handler's callback (standing in for the deferred CompoundButton.CheckedChange listener — see the
        // partial's header). A radio tap SELECTS: it crosses back through send_is_checked(true) into the
        // control, which never unchecks itself (the group's exclusion is the Controls-layer concern).
        jmethodID set_checked = default_jni_cache().method(env.get(), k_radio_button_class, "setChecked", "(Z)V");
        ASSERT_NE(set_checked, nullptr);
        env->CallVoidMethod(seam.widget(), set_checked, JNI_TRUE);
        ASSERT_FALSE(pending_exception_cleared(env.get(), "setChecked"));
        ASSERT_TRUE(call_bool(env.get(), seam.widget(), "isChecked"));

        EXPECT_FALSE(seam.control.is_checked());
        platform->on_select();
        EXPECT_TRUE(seam.control.is_checked()) << "the native tap must select the virtual view";
    }

    TEST(android_radio_button, disconnect_clears_the_select_callback)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_radio_button seam;
        auto* platform = seam.handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        ASSERT_TRUE(static_cast<bool>(platform->on_select));
        // DisconnectHandler: the callback is cleared. Drive on_disconnect_handler directly on the LIVE
        // struct — set_handler(nullptr) would move-then-destroy it, so reading it back after detach is a
        // use-after-free (the check_box disconnect test sidesteps this the same way).
        radio_button_handler::on_disconnect_handler(*platform);
        EXPECT_FALSE(static_cast<bool>(platform->on_select))
            << "on_disconnect_handler must clear the deferred-listener callback";
    }
} // namespace
