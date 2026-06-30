// Android progress-bar seam tests (M-android per-control fan-out) — the iOS Rosetta Stone replayed
// over JNI, ON the emulator inside the app_process widget test host (tools/android-testhost-run.sh):
// the REAL maui::controls::progress_bar drives the REAL android.widget.ProgressBar through the
// cross-platform progress_bar_handler's android partial (src/platform/android/progress_bar_handler.cpp),
// and every assertion reads the widget state BACK through JNI getters (getProgress/getMax/
// isIndeterminate/getProgressTintList/getVisibility/getAlpha/getContentDescription...).
//
// A progress bar is display-only — there is no native→virtual event channel (unlike button) — so the
// cases are all virtual → native. Characterization target: ProgressBarHandler.Android.cs +
// ProgressBarExtensions.cs (the documented plain-widget deviations are in the partial's header).

#include <memory>
#include <string>

#include <gtest/gtest.h>
#include <jni.h>

#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/controls/progress_bar.hpp"
#include "maui/core/progress_bar_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/size.hpp"
#include "testhost/test_host.hpp"

namespace
{
    using maui::controls::progress_bar;
    using maui::core::progress_bar_handler;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;
    using maui::platform::android::to_utf8;

    constexpr const char* k_progress_bar_class = "android/widget/ProgressBar";

    // ProgressBarExtensions.Maximum — the fixed integer range MAUI scales the [0,1] fraction onto.
    constexpr jint k_progress_maximum = 10000;

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
    struct attached_progress_bar
    {
        progress_bar control;
        std::shared_ptr<progress_bar_handler> handler = std::make_shared<progress_bar_handler>();

        attached_progress_bar()
        {
            control.set_handler(handler);
        }

        ~attached_progress_bar()
        {
            control.set_handler(nullptr);
        }

        attached_progress_bar(const attached_progress_bar&) = delete;
        attached_progress_bar(attached_progress_bar&&) = delete;
        attached_progress_bar& operator=(const attached_progress_bar&) = delete;
        attached_progress_bar& operator=(attached_progress_bar&&) = delete;

        [[nodiscard]] jobject widget() const
        {
            auto* platform = handler->typed_platform_view();
            return platform != nullptr ? static_cast<jobject>(platform->native) : nullptr;
        }
    };

    // ---- typed JNI read-backs (each fails the test and returns a benign default on a JNI error) ----

    [[nodiscard]] jint call_int(JNIEnv* env, jobject widget, const char* name)
    {
        jmethodID method = default_jni_cache().method(env, k_progress_bar_class, name, "()I");
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
        jmethodID method = default_jni_cache().method(env, k_progress_bar_class, name, "()F");
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
        jmethodID method = default_jni_cache().method(env, k_progress_bar_class, name, "()Z");
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
        jmethodID method = cache.method(env, k_progress_bar_class, name, "()Ljava/lang/CharSequence;");
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

    TEST(android_progress_bar, attach_creates_a_real_horizontal_determinate_progress_bar)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        const attached_progress_bar seam;
        ASSERT_NE(seam.widget(), nullptr) << "the android partial did not create a widget";
        jclass progress_bar_class = default_jni_cache().find_class(env.get(), k_progress_bar_class);
        ASSERT_NE(progress_bar_class, nullptr);
        EXPECT_EQ(env->IsInstanceOf(seam.widget(), progress_bar_class), JNI_TRUE);
        // CreatePlatformView's object-initializer: { Indeterminate = false, Max = Maximum }.
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getMax"), k_progress_maximum);
        EXPECT_FALSE(call_bool(env.get(), seam.widget(), "isIndeterminate"));
    }

    TEST(android_progress_bar, progress_fraction_scales_onto_the_widget_range)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_progress_bar seam;
        ASSERT_NE(seam.widget(), nullptr);
        // ProgressBarExtensions.UpdateProgress: Progress = (int)(fraction * Maximum).
        seam.control.set_progress(0.25);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getProgress"), 2500);
        seam.control.set_progress(1.0);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getProgress"), k_progress_maximum);
        seam.control.set_progress(0.0);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getProgress"), 0);
    }

    TEST(android_progress_bar, progress_color_reaches_the_progress_tint_list)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_progress_bar seam;
        ASSERT_NE(seam.widget(), nullptr);
        const maui::graphics::color red(1.0F, 0.0F, 0.0F);
        seam.control.set_progress_color(red);

        // ProgressBarExtensions.UpdateProgressBarColor: ProgressTintList = ColorStateList.ValueOf(argb).
        auto& cache = default_jni_cache();
        jmethodID get_progress_tint = cache.method(env.get(), k_progress_bar_class, "getProgressTintList",
                                                   "()Landroid/content/res/ColorStateList;");
        jmethodID get_default_color =
            cache.method(env.get(), "android/content/res/ColorStateList", "getDefaultColor", "()I");
        ASSERT_NE(get_progress_tint, nullptr);
        ASSERT_NE(get_default_color, nullptr);
        const local_ref<jobject> tint{env.get(), env->CallObjectMethod(seam.widget(), get_progress_tint)};
        ASSERT_FALSE(pending_exception_cleared(env.get(), "getProgressTintList"));
        ASSERT_TRUE(tint) << "no ProgressTintList was installed";
        EXPECT_EQ(env->CallIntMethod(tint.get(), get_default_color), static_cast<jint>(red.to_int()));
        EXPECT_FALSE(pending_exception_cleared(env.get(), "getDefaultColor"));
    }

    TEST(android_progress_bar, visibility_maps_to_the_three_view_states)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_progress_bar seam;
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_visible);
        seam.control.set_visibility(maui::core::visibility::hidden);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_invisible);
        seam.control.set_visibility(maui::core::visibility::collapsed);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_gone);
        seam.control.set_visibility(maui::core::visibility::visible);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_visible);
    }

    TEST(android_progress_bar, opacity_reaches_the_widget_alpha)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_progress_bar seam;
        seam.control.set_opacity(0.5);
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_NEAR(call_float(env.get(), seam.widget(), "getAlpha"), 0.5F, 1e-4F);
    }

    TEST(android_progress_bar, automation_id_reaches_the_content_description)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_progress_bar seam;
        seam.control.set_automation_id("download_progress");
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getContentDescription"), "download_progress");
    }

    TEST(android_progress_bar, measure_returns_a_positive_size)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_progress_bar seam;
        ASSERT_NE(seam.widget(), nullptr);
        const maui::graphics::size measured = seam.handler->get_desired_size(1000, 1000);
        EXPECT_GT(measured.width, 0);
        EXPECT_GT(measured.height, 0);
    }
} // namespace
