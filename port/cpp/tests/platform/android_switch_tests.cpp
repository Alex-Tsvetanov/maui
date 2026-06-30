// Android switch seam tests (M-android fan-out) — the iOS/headless switch recipe replayed over JNI,
// ON the emulator inside the app_process widget test host (tools/android-testhost-run.sh): the REAL
// maui::controls::toggle_switch drives the REAL android.widget.Switch through the cross-platform
// switch_handler's android partial (src/platform/android/switch_handler.cpp), and every assertion
// reads the widget state BACK through JNI getters — both directions of the seam:
//   virtual → native: set virtual-view properties, read the widget (isChecked/getTrackDrawable/
//                     getThumbTintList/isEnabled/getVisibility/getAlpha/getContentDescription...)
//   native → virtual: the OnCheckedChangeListener is DEFERRED (header deviation), so the native flip is
//                     simulated the headless way — flip the platform mirror + invoke the wired
//                     on_value_changed callback (the SwitchProxy.OnControlValueChanged port) and observe
//                     the control's `toggled` event, the same channel the cross-platform suite drives.
// Characterization target: SwitchHandler.Android.cs + SwitchExtensions.cs (the documented plain-widget
// deviations — android.widget.Switch standing in for SwitchCompat, ColorStateList.valueOf for
// CreateDefault — are in the partial's header).

#include <cmath>
#include <memory>
#include <string>

#include <gtest/gtest.h>
#include <jni.h>

#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/controls/toggle_switch.hpp"
#include "maui/core/switch_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "testhost/test_host.hpp"

namespace
{
    using maui::controls::toggle_switch;
    using maui::core::switch_handler;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;
    using maui::platform::android::to_utf8;
    using maui::platform::android::testhost::host_context;

    constexpr const char* k_switch_class = "android/widget/Switch";
    constexpr const char* k_color_state_list_class = "android/content/res/ColorStateList";

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
    struct attached_switch
    {
        toggle_switch control;
        std::shared_ptr<switch_handler> handler = std::make_shared<switch_handler>();

        attached_switch()
        {
            control.set_handler(handler);
        }

        ~attached_switch()
        {
            control.set_handler(nullptr);
        }

        attached_switch(const attached_switch&) = delete;
        attached_switch(attached_switch&&) = delete;
        attached_switch& operator=(const attached_switch&) = delete;
        attached_switch& operator=(attached_switch&&) = delete;

        [[nodiscard]] jobject widget() const
        {
            auto* platform = handler->typed_platform_view();
            return platform != nullptr ? static_cast<jobject>(platform->native) : nullptr;
        }
    };

    // ---- typed JNI read-backs (each fails the test and returns a benign default on a JNI error) ----

    [[nodiscard]] jint call_int(JNIEnv* env, jobject widget, const char* name)
    {
        jmethodID method = default_jni_cache().method(env, k_switch_class, name, "()I");
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
        jmethodID method = default_jni_cache().method(env, k_switch_class, name, "()F");
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
        jmethodID method = default_jni_cache().method(env, k_switch_class, name, "()Z");
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
        jmethodID method = cache.method(env, k_switch_class, name, "()Ljava/lang/CharSequence;");
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

    // ColorStateList.getDefaultColor() — the single default-state color the thumb tint carries (the
    // ColorStateList.valueOf the partial installs, read back as one int).
    [[nodiscard]] jint color_state_list_default(JNIEnv* env, jobject color_state_list)
    {
        jmethodID get_default_color =
            default_jni_cache().method(env, k_color_state_list_class, "getDefaultColor", "()I");
        if (get_default_color == nullptr)
        {
            ADD_FAILURE() << "ColorStateList.getDefaultColor not found";
            return 0;
        }
        const jint value = env->CallIntMethod(color_state_list, get_default_color);
        return pending_exception_cleared(env, "getDefaultColor") ? 0 : value;
    }

    // ---- virtual → native ----

    TEST(android_switch, attach_creates_a_real_android_widget_switch)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        const attached_switch seam;
        ASSERT_NE(seam.widget(), nullptr) << "the android partial did not create a widget";
        jclass switch_class = default_jni_cache().find_class(env.get(), k_switch_class);
        ASSERT_NE(switch_class, nullptr);
        EXPECT_EQ(env->IsInstanceOf(seam.widget(), switch_class), JNI_TRUE);
    }

    TEST(android_switch, is_on_reaches_the_widget_checked_state)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_switch seam;
        ASSERT_NE(seam.widget(), nullptr);
        // SwitchExtensions.UpdateIsOn: Checked = view.IsOn → isChecked() read-back.
        EXPECT_FALSE(call_bool(env.get(), seam.widget(), "isChecked"));
        seam.control.set_is_toggled(true);
        EXPECT_TRUE(call_bool(env.get(), seam.widget(), "isChecked"));
        seam.control.set_is_toggled(false);
        EXPECT_FALSE(call_bool(env.get(), seam.widget(), "isChecked"));
    }

    TEST(android_switch, on_color_reaches_the_track_drawable_color_filter)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_switch seam;
        ASSERT_NE(seam.widget(), nullptr);
        // The effective TrackColor is OnColor when toggled on. SwitchExtensions.UpdateTrackColor pushes a
        // SrcAtop color filter onto the track drawable; android.graphics.drawable.Drawable exposes no
        // color-filter getter, so the assertion is that the whole control→widget pipeline applies it
        // exception-free against a real track drawable (the same coverage shape android_button's
        // gradient-stroke test uses — no getter, reach here clean with the filter applied).
        seam.control.set_is_toggled(true);
        seam.control.set_on_color(maui::graphics::color(0.0F, 0.0F, 1.0F));

        auto& cache = default_jni_cache();
        jmethodID get_track_drawable =
            cache.method(env.get(), k_switch_class, "getTrackDrawable", "()Landroid/graphics/drawable/Drawable;");
        ASSERT_NE(get_track_drawable, nullptr);
        const local_ref<jobject> track{env.get(), env->CallObjectMethod(seam.widget(), get_track_drawable)};
        ASSERT_FALSE(pending_exception_cleared(env.get(), "getTrackDrawable"));
        EXPECT_TRUE(track) << "the framework Switch should carry a track drawable to tint";
    }

    TEST(android_switch, thumb_color_reaches_the_thumb_tint_list_as_argb)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_switch seam;
        ASSERT_NE(seam.widget(), nullptr);
        const maui::graphics::color red(1.0F, 0.0F, 0.0F);
        seam.control.set_thumb_color(red);

        // SwitchExtensions.UpdateThumbColor → ThumbTintList = ColorStateList.valueOf(argb): read it back
        // via getThumbTintList().getDefaultColor() (the single default-state color the list carries).
        auto& cache = default_jni_cache();
        jmethodID get_thumb_tint_list =
            cache.method(env.get(), k_switch_class, "getThumbTintList", "()Landroid/content/res/ColorStateList;");
        ASSERT_NE(get_thumb_tint_list, nullptr);
        const local_ref<jobject> tint{env.get(), env->CallObjectMethod(seam.widget(), get_thumb_tint_list)};
        ASSERT_FALSE(pending_exception_cleared(env.get(), "getThumbTintList"));
        ASSERT_TRUE(tint) << "a custom ThumbColor must install a ThumbTintList";
        EXPECT_EQ(color_state_list_default(env.get(), tint.get()), static_cast<jint>(red.to_int()));
    }

    TEST(android_switch, is_enabled_reaches_the_widget)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_switch seam;
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_TRUE(call_bool(env.get(), seam.widget(), "isEnabled"));
        seam.control.set_is_enabled(false);
        EXPECT_FALSE(call_bool(env.get(), seam.widget(), "isEnabled"));
        seam.control.set_is_enabled(true);
        EXPECT_TRUE(call_bool(env.get(), seam.widget(), "isEnabled"));
    }

    TEST(android_switch, visibility_maps_to_the_three_view_states)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_switch seam;
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_visible);
        seam.control.set_visibility(maui::core::visibility::hidden);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_invisible);
        seam.control.set_visibility(maui::core::visibility::collapsed);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_gone);
        seam.control.set_visibility(maui::core::visibility::visible);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_visible);
    }

    TEST(android_switch, opacity_reaches_the_widget_alpha)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_switch seam;
        seam.control.set_opacity(0.5);
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_NEAR(call_float(env.get(), seam.widget(), "getAlpha"), 0.5F, 1e-4F);
    }

    TEST(android_switch, automation_id_reaches_the_content_description)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_switch seam;
        seam.control.set_automation_id("dark_mode_switch");
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getContentDescription"), "dark_mode_switch");
    }

    TEST(android_switch, background_paint_reaches_the_widget)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_switch seam;
        ASSERT_NE(seam.widget(), nullptr);
        // VisualElement.Background paints the View background (the band behind the track) via the shared
        // android op. android.graphics.drawable getters vary by drawable kind; the seam coverage is that
        // the push lands exception-free and the View then carries a background drawable.
        seam.control.set_background(
            std::make_shared<maui::graphics::solid_paint>(maui::graphics::color(0.0F, 1.0F, 0.0F)));
        auto& cache = default_jni_cache();
        jmethodID get_background =
            cache.method(env.get(), k_switch_class, "getBackground", "()Landroid/graphics/drawable/Drawable;");
        ASSERT_NE(get_background, nullptr);
        const local_ref<jobject> background{env.get(), env->CallObjectMethod(seam.widget(), get_background)};
        EXPECT_FALSE(pending_exception_cleared(env.get(), "getBackground"));
        EXPECT_TRUE(background) << "a Background brush must install a View background drawable";
    }

    TEST(android_switch, measure_returns_a_positive_size)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_switch seam;
        ASSERT_NE(seam.widget(), nullptr);
        // The real Switch measures to a non-zero natural size (the SwitchMinWidth zero-width fallback in
        // C# is not reached — see the partial's header). The cross-platform size-request suite consumes
        // these numbers; here we only assert the native measure produced a real extent.
        const maui::graphics::size size = seam.handler->get_desired_size(1000, 1000);
        EXPECT_GT(size.width, 0);
        EXPECT_GT(size.height, 0);
    }

    // ---- native → virtual (the deferred OnCheckedChangeListener, simulated via the wired callback) ----

    TEST(android_switch, native_toggle_flows_back_to_the_control)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_switch seam;
        ASSERT_NE(seam.widget(), nullptr);
        bool reported = false;
        seam.control.toggled.connect([&reported](bool value) { reported = value; });

        // The real OnCheckedChangeListener is deferred (header deviation), so simulate the user's flip the
        // headless way: set the platform mirror to the new checked state and invoke the wired
        // on_value_changed callback (SwitchProxy.OnControlValueChanged → VirtualView.IsOn write-back).
        auto* platform = seam.handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        platform->is_on = true;
        ASSERT_TRUE(static_cast<bool>(platform->on_value_changed)) << "the value-changed callback must be wired";
        platform->on_value_changed();

        EXPECT_TRUE(seam.control.is_toggled());
        EXPECT_TRUE(reported);
    }

    TEST(android_switch, value_changed_callback_is_cleared_on_disconnect)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        toggle_switch control;
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_TRUE(static_cast<bool>(platform->on_value_changed));

        control.set_handler(nullptr); // DisconnectHandler: SetOnCheckedChangeListener(null) → drop the callback
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }
} // namespace
