// Android slider seam tests (M-android per-control fan-out) — the iOS Rosetta Stone replayed over JNI,
// ON the emulator inside the app_process widget test host (tools/android-testhost-run.sh): the REAL
// maui::controls::slider drives the REAL android.widget.SeekBar through the cross-platform
// slider_handler's android partial (src/platform/android/slider_handler.cpp), and every assertion reads
// the widget state BACK through JNI getters (getProgress/getMax/getProgressTintList/
// getProgressBackgroundTintList/getThumbTintList/isEnabled/getVisibility/getAlpha/getContentDescription).
//
//   virtual → native: set virtual-view properties, read the widget.
//   native → virtual: the SeekBar OnSeekBarChangeListener install is DEFERRED (no host listener class —
//                     only dev.mauicpp.NativeOnClickListener exists, for button), exactly like the headless
//                     seam, so the deferred-but-invokable C++ callbacks (on_value_changed / on_drag_started
//                     / on_drag_completed, carrying SliderHandler.OnProgressChanged / OnStartTrackingTouch /
//                     OnStopTrackingTouch) are driven directly — the documented stand-in for the real
//                     listener trampoline (header note in slider_handler.cpp).
//
// Characterization target: SliderHandler.Android.cs + SliderExtensions.cs (the documented plain-widget
// deviations are in the partial's header). SeekBar's range is SliderExtensions.PlatformMaxValue =
// int.MaxValue (NOT the ProgressBar's 10000) — read from the oracle, mirrored in the expected values here.

#include <memory>
#include <string>

#include <gtest/gtest.h>
#include <jni.h>

#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/controls/slider.hpp"
#include "maui/core/slider_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/size.hpp"
#include "testhost/test_host.hpp"

namespace
{
    using maui::controls::slider;
    using maui::core::slider_handler;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;
    using maui::platform::android::to_utf8;

    constexpr const char* k_seek_bar_class = "android/widget/SeekBar";

    // SliderExtensions.PlatformMaxValue — the fixed integer SeekBar range MAUI scales the value onto.
    constexpr jint k_platform_max_value = 2147483647; // = int.MaxValue

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

    // The control + attached handler + the real widget, torn down in declaration order (the handler detach
    // precedes the control's death; the platform struct releases the widget's global ref).
    struct attached_slider
    {
        slider control;
        std::shared_ptr<slider_handler> handler = std::make_shared<slider_handler>();

        attached_slider()
        {
            control.set_handler(handler);
        }

        ~attached_slider()
        {
            control.set_handler(nullptr);
        }

        attached_slider(const attached_slider&) = delete;
        attached_slider(attached_slider&&) = delete;
        attached_slider& operator=(const attached_slider&) = delete;
        attached_slider& operator=(attached_slider&&) = delete;

        [[nodiscard]] jobject widget() const
        {
            auto* platform = handler->typed_platform_view();
            return platform != nullptr ? static_cast<jobject>(platform->native) : nullptr;
        }
    };

    // ---- typed JNI read-backs (each fails the test and returns a benign default on a JNI error) ----

    [[nodiscard]] jint call_int(JNIEnv* env, jobject widget, const char* name)
    {
        jmethodID method = default_jni_cache().method(env, k_seek_bar_class, name, "()I");
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
        jmethodID method = default_jni_cache().method(env, k_seek_bar_class, name, "()F");
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
        jmethodID method = default_jni_cache().method(env, k_seek_bar_class, name, "()Z");
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
        jmethodID method = cache.method(env, k_seek_bar_class, name, "()Ljava/lang/CharSequence;");
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

    // The default color of a ColorStateList-returning tint getter (getProgressTintList / …) — the argb the
    // tint was installed with. Returns the failure path via ADD_FAILURE when no list is installed.
    [[nodiscard]] jint tint_default_color(JNIEnv* env, jobject widget, const char* getter)
    {
        auto& cache = default_jni_cache();
        jmethodID get_tint = cache.method(env, k_seek_bar_class, getter, "()Landroid/content/res/ColorStateList;");
        jmethodID get_default_color = cache.method(env, "android/content/res/ColorStateList", "getDefaultColor", "()I");
        if (get_tint == nullptr || get_default_color == nullptr)
        {
            ADD_FAILURE() << getter << " surface missing";
            return 0;
        }
        const local_ref<jobject> tint{env, env->CallObjectMethod(widget, get_tint)};
        if (pending_exception_cleared(env, getter) || !tint)
        {
            ADD_FAILURE() << "no ColorStateList installed by " << getter;
            return 0;
        }
        const jint argb = env->CallIntMethod(tint.get(), get_default_color);
        return pending_exception_cleared(env, "getDefaultColor") ? 0 : argb;
    }

    // ---- virtual → native ----

    TEST(android_slider, attach_creates_a_real_seek_bar_with_the_platform_max_range)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        const attached_slider seam;
        ASSERT_NE(seam.widget(), nullptr) << "the android partial did not create a widget";
        jclass seek_bar_class = default_jni_cache().find_class(env.get(), k_seek_bar_class);
        ASSERT_NE(seek_bar_class, nullptr);
        EXPECT_EQ(env->IsInstanceOf(seam.widget(), seek_bar_class), JNI_TRUE);
        // CreatePlatformView's object-initializer: { DuplicateParentStateEnabled = false,
        // Max = (int)PlatformMaxValue }.
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getMax"), k_platform_max_value);
    }

    TEST(android_slider, value_scales_onto_the_integer_progress_range)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_slider seam;
        ASSERT_NE(seam.widget(), nullptr);
        // SliderExtensions.UpdateValue: Progress = (int)((value - min) / (max - min) * PlatformMaxValue).
        // Default range is [0,1], so value 0.5 → (int)(0.5 * int.MaxValue).
        seam.control.set_value(0.5);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getProgress"),
                  static_cast<jint>(0.5 * static_cast<double>(k_platform_max_value)));
        seam.control.set_value(1.0);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getProgress"), k_platform_max_value);
        seam.control.set_value(0.0);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getProgress"), 0);
    }

    TEST(android_slider, a_widened_range_recomputes_progress_from_the_new_bounds)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_slider seam;
        ASSERT_NE(seam.widget(), nullptr);
        // SliderExtensions.UpdateMinimum / UpdateMaximum (SeekBar) both re-run UpdateValue, so changing a
        // bound rescales the integer Progress. Range [0,10], value 5 → midpoint → 0.5 * int.MaxValue.
        seam.control.set_maximum(10.0);
        seam.control.set_value(5.0);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getProgress"),
                  static_cast<jint>(0.5 * static_cast<double>(k_platform_max_value)));
        // Narrow the maximum to 5 so the value 5 now sits at the top of [0,5] → full Progress (UpdateMaximum
        // re-runs UpdateValue, rescaling the integer Progress to the new bounds).
        seam.control.set_maximum(5.0);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getProgress"), k_platform_max_value);
    }

    TEST(android_slider, minimum_track_color_reaches_the_progress_tint_list)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_slider seam;
        ASSERT_NE(seam.widget(), nullptr);
        // SliderExtensions.UpdateMinimumTrackColor (SeekBar): ProgressTintList = ColorStateList.ValueOf.
        const maui::graphics::color red(1.0F, 0.0F, 0.0F);
        seam.control.set_minimum_track_color(red);
        EXPECT_EQ(tint_default_color(env.get(), seam.widget(), "getProgressTintList"), static_cast<jint>(red.to_int()));
    }

    TEST(android_slider, maximum_track_color_reaches_the_progress_background_tint_list)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_slider seam;
        ASSERT_NE(seam.widget(), nullptr);
        // SliderExtensions.UpdateMaximumTrackColor (SeekBar): ProgressBackgroundTintList = ValueOf.
        const maui::graphics::color blue(0.0F, 0.0F, 1.0F);
        seam.control.set_maximum_track_color(blue);
        EXPECT_EQ(tint_default_color(env.get(), seam.widget(), "getProgressBackgroundTintList"),
                  static_cast<jint>(blue.to_int()));
    }

    TEST(android_slider, thumb_color_reaches_the_thumb_tint_list)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_slider seam;
        ASSERT_NE(seam.widget(), nullptr);
        // SliderExtensions.UpdateThumbColor: the port pushes the theme-independent ThumbTintList (header
        // deviation — C# mutates the live Thumb Drawable's color filter, which no-ops without a thumb).
        const maui::graphics::color green(0.0F, 1.0F, 0.0F);
        seam.control.set_thumb_color(green);
        EXPECT_EQ(tint_default_color(env.get(), seam.widget(), "getThumbTintList"), static_cast<jint>(green.to_int()));
    }

    TEST(android_slider, is_enabled_reaches_the_widget)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_slider seam;
        ASSERT_NE(seam.widget(), nullptr);
        // A SeekBar is interactive (unlike the determinate ProgressBar), so IsEnabled IS pushed.
        EXPECT_TRUE(call_bool(env.get(), seam.widget(), "isEnabled"));
        seam.control.set_is_enabled(false);
        EXPECT_FALSE(call_bool(env.get(), seam.widget(), "isEnabled"));
        seam.control.set_is_enabled(true);
        EXPECT_TRUE(call_bool(env.get(), seam.widget(), "isEnabled"));
    }

    TEST(android_slider, visibility_maps_to_the_three_view_states)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_slider seam;
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_visible);
        seam.control.set_visibility(maui::core::visibility::hidden);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_invisible);
        seam.control.set_visibility(maui::core::visibility::collapsed);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_gone);
        seam.control.set_visibility(maui::core::visibility::visible);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_visible);
    }

    TEST(android_slider, opacity_reaches_the_widget_alpha)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_slider seam;
        seam.control.set_opacity(0.5);
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_NEAR(call_float(env.get(), seam.widget(), "getAlpha"), 0.5F, 1e-4F);
    }

    TEST(android_slider, automation_id_reaches_the_content_description)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_slider seam;
        seam.control.set_automation_id("volume_slider");
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getContentDescription"), "volume_slider");
    }

    TEST(android_slider, measure_returns_a_positive_size)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_slider seam;
        ASSERT_NE(seam.widget(), nullptr);
        const maui::graphics::size measured = seam.handler->get_desired_size(1000, 1000);
        EXPECT_GT(measured.width, 0);
        EXPECT_GT(measured.height, 0);
    }

    // ---- native → virtual (the deferred, but invokable, listener channel) ----

    TEST(android_slider, value_change_callback_writes_back_through_the_virtual_view)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_slider seam;
        auto* platform = seam.handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        ASSERT_TRUE(static_cast<bool>(platform->on_value_changed));
        // The real SeekBar OnSeekBarChangeListener install is deferred (no host listener class), so drive
        // the deferred-but-invokable callback directly — the documented stand-in for the trampoline. The
        // mirror's `value` is the user's resolved value (OnProgressChanged's computed value); the callback
        // writes it back through i_range::set_value (the fromUser write-back).
        int changes = 0;
        seam.control.value_changed.connect([&changes](double, double) { ++changes; });
        platform->value = 0.75;
        platform->on_value_changed();
        EXPECT_DOUBLE_EQ(seam.control.value(), 0.75);
        EXPECT_EQ(changes, 1);
    }

    TEST(android_slider, drag_callbacks_raise_the_drag_events)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_slider seam;
        auto* platform = seam.handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        ASSERT_TRUE(static_cast<bool>(platform->on_drag_started));
        ASSERT_TRUE(static_cast<bool>(platform->on_drag_completed));
        int started = 0;
        int completed = 0;
        seam.control.drag_started.connect([&started] { ++started; });
        seam.control.drag_completed.connect([&completed] { ++completed; });
        // OnStartTrackingTouch → DragStarted, OnStopTrackingTouch → DragCompleted.
        platform->on_drag_started();
        platform->on_drag_completed();
        EXPECT_EQ(started, 1);
        EXPECT_EQ(completed, 1);
    }

    TEST(android_slider, callbacks_are_dropped_on_disconnect)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        slider control;
        auto handler = std::make_shared<slider_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        ASSERT_TRUE(static_cast<bool>(platform->on_value_changed));
        // DisconnectHandler: ChangeListener.Handler = null → SetOnSeekBarChangeListener(null). The native
        // uninstall is deferred (no host listener), but the callbacks ARE dropped. set_handler(nullptr)
        // would destroy the platform struct before it could be probed, so invoke the static disconnect
        // directly (the same body set_handler runs) while the struct is still alive, then detach.
        slider_handler::on_disconnect_handler(*platform);
        EXPECT_FALSE(static_cast<bool>(platform->on_value_changed));
        EXPECT_FALSE(static_cast<bool>(platform->on_drag_started));
        EXPECT_FALSE(static_cast<bool>(platform->on_drag_completed));
        control.set_handler(nullptr);
    }
} // namespace
