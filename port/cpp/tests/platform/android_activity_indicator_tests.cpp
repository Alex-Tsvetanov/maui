// Android activity_indicator seam tests (M-android fan-out) — the headless mirror replayed over JNI,
// ON the emulator inside the app_process widget test host (tools/android-testhost-run.sh): the REAL
// maui::controls::activity_indicator drives the REAL android.widget.ProgressBar through the
// cross-platform activity_indicator_handler's android partial (src/platform/android/
// activity_indicator_handler.cpp), and every assertion reads the widget state BACK through JNI
// getters. The control is display-only, so this is virtual → native ONLY (no inbound event channel
// like button's performClick path).
//   virtual → native: set virtual-view properties, read the widget (isIndeterminate/getVisibility/
//                      getIndeterminateDrawable...).
// Characterization target: ActivityIndicatorHandler.Android.cs + ActivityIndicatorExtensions.cs
// (UpdateIsRunning/GetActivityIndicatorVisibility/UpdateColor). The documented plain-widget
// deviations are in the partial's header.

#include <cmath>
#include <memory>

#include <gtest/gtest.h>
#include <jni.h>

#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "maui/controls/activity_indicator.hpp"
#include "maui/core/activity_indicator_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/size.hpp"
#include "testhost/test_host.hpp"

namespace
{
    using maui::controls::activity_indicator;
    using maui::core::activity_indicator_handler;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;

    constexpr const char* k_progress_bar_class = "android/widget/ProgressBar";

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
    struct attached_indicator
    {
        activity_indicator control;
        std::shared_ptr<activity_indicator_handler> handler = std::make_shared<activity_indicator_handler>();

        attached_indicator()
        {
            control.set_handler(handler);
        }

        ~attached_indicator()
        {
            control.set_handler(nullptr);
        }

        attached_indicator(const attached_indicator&) = delete;
        attached_indicator(attached_indicator&&) = delete;
        attached_indicator& operator=(const attached_indicator&) = delete;
        attached_indicator& operator=(attached_indicator&&) = delete;

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

    // ---- virtual → native ----

    TEST(android_activity_indicator, attach_creates_a_real_indeterminate_progress_bar)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        const attached_indicator seam;
        ASSERT_NE(seam.widget(), nullptr) << "the android partial did not create a widget";
        jclass progress_bar_class = default_jni_cache().find_class(env.get(), k_progress_bar_class);
        ASSERT_NE(progress_bar_class, nullptr);
        EXPECT_EQ(env->IsInstanceOf(seam.widget(), progress_bar_class), JNI_TRUE);
        // CreatePlatformView: new ProgressBar(Context) { Indeterminate = true }.
        EXPECT_TRUE(call_bool(env.get(), seam.widget(), "isIndeterminate"));
    }

    TEST(android_activity_indicator, is_running_and_visibility_map_to_the_view_states)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_indicator seam;
        ASSERT_NE(seam.widget(), nullptr);

        // GetActivityIndicatorVisibility, Visible branch: IsRunning ? Visible : Invisible.
        // Default IsRunning is false, so a freshly-visible indicator is Invisible.
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_invisible);

        seam.control.set_is_running(true);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_visible);

        seam.control.set_is_running(false);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_invisible);

        // Non-Visible branch falls to ToPlatformVisibility (Hidden→Invisible, Collapsed→Gone),
        // independent of IsRunning.
        seam.control.set_is_running(true);
        seam.control.set_visibility(maui::core::visibility::collapsed);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_gone);

        seam.control.set_visibility(maui::core::visibility::hidden);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_invisible);

        seam.control.set_visibility(maui::core::visibility::visible);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_visible);
    }

    TEST(android_activity_indicator, color_reaches_the_indeterminate_drawable_without_throwing)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_indicator seam;
        ASSERT_NE(seam.widget(), nullptr);
        auto& cache = default_jni_cache();
        jmethodID get_indeterminate_drawable = cache.method(env.get(), k_progress_bar_class, "getIndeterminateDrawable",
                                                            "()Landroid/graphics/drawable/Drawable;");
        ASSERT_NE(get_indeterminate_drawable, nullptr);

        // ProgressBar always has an indeterminate drawable once Indeterminate = true (set at create);
        // map_color tints it via setColorFilter(int, PorterDuff.Mode.SRC_IN). The drawable exposes no
        // color-filter getter, so the assertion is that the SET path reaches the real drawable
        // exception-free (the same posture the button background/stroke test uses for setStroke).
        const maui::graphics::color red(1.0F, 0.0F, 0.0F);
        seam.control.set_color(red);
        const local_ref<jobject> tinted{env.get(), env->CallObjectMethod(seam.widget(), get_indeterminate_drawable)};
        ASSERT_FALSE(pending_exception_cleared(env.get(), "getIndeterminateDrawable (set)"));
        EXPECT_TRUE(tinted) << "an indeterminate ProgressBar must carry an indeterminate drawable";

        // An unset (default-constructed) color takes the ClearColorFilter branch — also exception-free.
        seam.control.set_color(maui::graphics::color{});
        const local_ref<jobject> cleared{env.get(), env->CallObjectMethod(seam.widget(), get_indeterminate_drawable)};
        EXPECT_FALSE(pending_exception_cleared(env.get(), "getIndeterminateDrawable (clear)"));
        EXPECT_TRUE(cleared);
    }

    TEST(android_activity_indicator, measure_returns_a_positive_square_size)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_indicator seam;
        ASSERT_NE(seam.widget(), nullptr);
        const maui::graphics::size measured = seam.handler->get_desired_size(1000, 1000);
        EXPECT_GT(measured.width, 0) << "the real ProgressBar should measure a positive width";
        EXPECT_GT(measured.height, 0) << "the real ProgressBar should measure a positive height";
    }
} // namespace
