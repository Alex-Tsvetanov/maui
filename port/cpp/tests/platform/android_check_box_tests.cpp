// Android check box seam tests (M-android fan-out) — M6's iOS Rosetta Stone replayed over JNI, ON the
// emulator inside the app_process widget test host (tools/android-testhost-run.sh): the REAL
// maui::controls::check_box drives the REAL android.widget.CheckBox through the cross-platform
// check_box_handler's android partial (src/platform/android/check_box_handler.cpp), and every assertion
// reads the widget state BACK through JNI getters — both directions of the seam:
//   virtual → native: set virtual-view properties, read the widget (isChecked/isEnabled/getVisibility/
//                     getAlpha/getContentDescription/getButtonTintList...)
//   native → virtual: flip the widget's checked state + invoke the handler's on_checked_changed callback
//                     (the deferred-listener seam — see the partial's header) → send_is_checked → the
//                     control's `checked_changed` event.
// Characterization target: CheckBoxHandler.Android.cs + CheckBoxExtensions.cs (the documented
// plain-widget deviations — Material widget, deferred CheckedChange listener — are in the partial's
// header).

#include <memory>
#include <string>

#include <gtest/gtest.h>
#include <jni.h>

#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/controls/check_box.hpp"
#include "maui/core/check_box_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "testhost/test_host.hpp"

namespace
{
    using maui::controls::check_box;
    using maui::core::check_box_handler;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;
    using maui::platform::android::to_utf8;

    constexpr const char* k_check_box_class = "android/widget/CheckBox";

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
    struct attached_check_box
    {
        check_box control;
        std::shared_ptr<check_box_handler> handler = std::make_shared<check_box_handler>();

        attached_check_box()
        {
            control.set_handler(handler);
        }

        ~attached_check_box()
        {
            control.set_handler(nullptr);
        }

        attached_check_box(const attached_check_box&) = delete;
        attached_check_box(attached_check_box&&) = delete;
        attached_check_box& operator=(const attached_check_box&) = delete;
        attached_check_box& operator=(attached_check_box&&) = delete;

        [[nodiscard]] jobject widget() const
        {
            auto* platform = handler->typed_platform_view();
            return platform != nullptr ? static_cast<jobject>(platform->native) : nullptr;
        }
    };

    // ---- typed JNI read-backs (each fails the test and returns a benign default on a JNI error) ----

    [[nodiscard]] jint call_int(JNIEnv* env, jobject widget, const char* name)
    {
        jmethodID method = default_jni_cache().method(env, k_check_box_class, name, "()I");
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
        jmethodID method = default_jni_cache().method(env, k_check_box_class, name, "()F");
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
        jmethodID method = default_jni_cache().method(env, k_check_box_class, name, "()Z");
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
        jmethodID method = cache.method(env, k_check_box_class, name, "()Ljava/lang/CharSequence;");
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

    TEST(android_check_box, attach_creates_a_real_android_widget_check_box)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        const attached_check_box seam;
        ASSERT_NE(seam.widget(), nullptr) << "the android partial did not create a widget";
        jclass check_box_class = default_jni_cache().find_class(env.get(), k_check_box_class);
        ASSERT_NE(check_box_class, nullptr);
        EXPECT_EQ(env->IsInstanceOf(seam.widget(), check_box_class), JNI_TRUE);
    }

    TEST(android_check_box, is_checked_reaches_the_widget)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_check_box seam;
        ASSERT_NE(seam.widget(), nullptr);
        // A default check box is unchecked (CheckBox.IsChecked default false).
        EXPECT_FALSE(call_bool(env.get(), seam.widget(), "isChecked"));
        seam.control.set_is_checked(true);
        EXPECT_TRUE(call_bool(env.get(), seam.widget(), "isChecked"));
        seam.control.set_is_checked(false);
        EXPECT_FALSE(call_bool(env.get(), seam.widget(), "isChecked"));
    }

    TEST(android_check_box, foreground_reaches_the_widget_button_tint_as_argb)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_check_box seam;
        ASSERT_NE(seam.widget(), nullptr);
        // CheckBox.Foreground => Color?.AsPaint(): setting Color builds the solid paint the mapper tints
        // with (CheckBoxExtensions.UpdateForeground → ColorStateList.valueOf(argb) on the plain widget).
        const maui::graphics::color red(1.0F, 0.0F, 0.0F);
        seam.control.set_color(red);

        auto& cache = default_jni_cache();
        jmethodID get_button_tint =
            cache.method(env.get(), k_check_box_class, "getButtonTintList", "()Landroid/content/res/ColorStateList;");
        jmethodID get_default_color =
            cache.method(env.get(), "android/content/res/ColorStateList", "getDefaultColor", "()I");
        ASSERT_NE(get_button_tint, nullptr);
        ASSERT_NE(get_default_color, nullptr);
        const local_ref<jobject> tint{env.get(), env->CallObjectMethod(seam.widget(), get_button_tint)};
        ASSERT_FALSE(pending_exception_cleared(env.get(), "getButtonTintList"));
        ASSERT_TRUE(tint) << "the foreground tint was not installed on the widget";
        EXPECT_EQ(env->CallIntMethod(tint.get(), get_default_color), static_cast<jint>(red.to_int()));
        EXPECT_FALSE(pending_exception_cleared(env.get(), "getDefaultColor"));
    }

    TEST(android_check_box, is_enabled_reaches_the_widget)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_check_box seam;
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_TRUE(call_bool(env.get(), seam.widget(), "isEnabled"));
        seam.control.set_is_enabled(false);
        EXPECT_FALSE(call_bool(env.get(), seam.widget(), "isEnabled"));
        seam.control.set_is_enabled(true);
        EXPECT_TRUE(call_bool(env.get(), seam.widget(), "isEnabled"));
    }

    TEST(android_check_box, visibility_maps_to_the_three_view_states)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_check_box seam;
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_visible);
        seam.control.set_visibility(maui::core::visibility::hidden);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_invisible);
        seam.control.set_visibility(maui::core::visibility::collapsed);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_gone);
        seam.control.set_visibility(maui::core::visibility::visible);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_visible);
    }

    TEST(android_check_box, opacity_reaches_the_widget_alpha)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_check_box seam;
        seam.control.set_opacity(0.5);
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_NEAR(call_float(env.get(), seam.widget(), "getAlpha"), 0.5F, 1e-4F);
    }

    TEST(android_check_box, automation_id_reaches_the_content_description)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_check_box seam;
        seam.control.set_automation_id("agree_terms");
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getContentDescription"), "agree_terms");
    }

    TEST(android_check_box, measure_returns_a_positive_size)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_check_box seam;
        ASSERT_NE(seam.widget(), nullptr);
        const maui::graphics::size measured = seam.handler->get_desired_size(1000, 1000);
        EXPECT_GT(measured.width, 0);
        EXPECT_GT(measured.height, 0);
    }

    // ---- native → virtual ----

    TEST(android_check_box, native_toggle_writes_back_through_the_checked_changed_callback)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_check_box seam;
        auto* platform = seam.handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        ASSERT_NE(platform->native, nullptr);
        ASSERT_TRUE(static_cast<bool>(platform->on_checked_changed))
            << "on_connect must wire the (deferred-listener) checked-change callback";

        int changes = 0;
        bool last = false;
        seam.control.checked_changed.connect([&](bool value) {
            ++changes;
            last = value;
        });

        // Simulate the user tapping the widget: drive the REAL CheckBox checked state, then invoke the
        // handler's callback (standing in for the deferred CompoundButton.OnCheckedChangeListener — see
        // the partial's header). It crosses back through send_is_checked into the control's event.
        jmethodID set_checked = default_jni_cache().method(env.get(), k_check_box_class, "setChecked", "(Z)V");
        ASSERT_NE(set_checked, nullptr);
        env->CallVoidMethod(seam.widget(), set_checked, JNI_TRUE);
        ASSERT_FALSE(pending_exception_cleared(env.get(), "setChecked"));
        // The platform struct's is_checked doubles as the native read-back state (mirrors isChecked).
        platform->is_checked = call_bool(env.get(), seam.widget(), "isChecked");
        ASSERT_TRUE(platform->is_checked);

        platform->on_checked_changed();
        EXPECT_EQ(changes, 1);
        EXPECT_TRUE(last);
        EXPECT_TRUE(seam.control.is_checked()) << "the toggle must reach the virtual view";
    }

    TEST(android_check_box, disconnect_clears_the_checked_change_callback)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_check_box seam;
        auto* platform = seam.handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        ASSERT_TRUE(static_cast<bool>(platform->on_checked_changed));
        // DisconnectHandler: the callback is cleared. Drive on_disconnect_handler directly on the LIVE
        // struct — set_handler(nullptr) would move-then-destroy it, so reading it back after detach is a
        // use-after-free (the button disconnect test sidesteps this by keeping only the widget alive).
        check_box_handler::on_disconnect_handler(*platform);
        EXPECT_FALSE(static_cast<bool>(platform->on_checked_changed))
            << "on_disconnect_handler must clear the deferred-listener callback";
    }
} // namespace
