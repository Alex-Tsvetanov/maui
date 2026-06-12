// Android button seam tests (M-android fan-out, unit 28) — M6's iOS Rosetta Stone replayed over
// JNI, ON the emulator inside the app_process widget test host (tools/android-testhost-run.sh):
// the REAL maui::controls::button drives the REAL android.widget.Button through the cross-platform
// button_handler's android partial (src/platform/android/button_handler.cpp), and every assertion
// reads the widget state BACK through JNI getters — both directions of the seam:
//   virtual → native: set virtual-view properties, read the widget (getText/getCurrentTextColor/
//                     isEnabled/getVisibility/getAlpha/getContentDescription/getPadding*/
//                     getTextSize/getTypeface/getBackground...)
//   native → virtual: View.performClick() → the NativeOnClickListener trampoline → send_clicked →
//                     the control's `clicked` event.
// Characterization target: ButtonHandler.Android.cs + the Android platform extensions (the
// documented plain-widget deviations are in the partial's header).

#include <cmath>
#include <memory>
#include <string>

#include <gtest/gtest.h>
#include <jni.h>

#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/controls/button.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/font.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "testhost/test_host.hpp"

namespace
{
    using maui::controls::button;
    using maui::core::button_handler;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;
    using maui::platform::android::to_utf8;
    using maui::platform::android::testhost::host_context;

    constexpr const char* k_button_class = "android/widget/Button";
    constexpr const char* k_gradient_drawable_class = "android/graphics/drawable/GradientDrawable";

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
    struct attached_button
    {
        button control;
        std::shared_ptr<button_handler> handler = std::make_shared<button_handler>();

        attached_button()
        {
            control.set_handler(handler);
        }

        ~attached_button()
        {
            control.set_handler(nullptr);
        }

        attached_button(const attached_button&) = delete;
        attached_button(attached_button&&) = delete;
        attached_button& operator=(const attached_button&) = delete;
        attached_button& operator=(attached_button&&) = delete;

        [[nodiscard]] jobject widget() const
        {
            auto* platform = handler->typed_platform_view();
            return platform != nullptr ? static_cast<jobject>(platform->native) : nullptr;
        }
    };

    // ---- typed JNI read-backs (each fails the test and returns a benign default on a JNI error) ----

    [[nodiscard]] jint call_int(JNIEnv* env, jobject widget, const char* name)
    {
        jmethodID method = default_jni_cache().method(env, k_button_class, name, "()I");
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
        jmethodID method = default_jni_cache().method(env, k_button_class, name, "()F");
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
        jmethodID method = default_jni_cache().method(env, k_button_class, name, "()Z");
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
        jmethodID method = cache.method(env, k_button_class, name, "()Ljava/lang/CharSequence;");
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

    // Context.getResources().getDisplayMetrics().density — the same conversion factor the partial's
    // ToPixels port uses, read independently here so the expectations stay oracle-shaped.
    [[nodiscard]] float host_density(JNIEnv* env)
    {
        auto& cache = default_jni_cache();
        jmethodID get_resources =
            cache.method(env, "android/content/Context", "getResources", "()Landroid/content/res/Resources;");
        jmethodID get_display_metrics =
            cache.method(env, "android/content/res/Resources", "getDisplayMetrics", "()Landroid/util/DisplayMetrics;");
        jfieldID density_field = cache.field(env, "android/util/DisplayMetrics", "density", "F");
        if (get_resources == nullptr || get_display_metrics == nullptr || density_field == nullptr)
        {
            ADD_FAILURE() << "DisplayMetrics.density surface missing";
            return 1.0F;
        }
        const local_ref<jobject> resources{env, env->CallObjectMethod(host_context(), get_resources)};
        if (pending_exception_cleared(env, "getResources") || !resources)
        {
            return 1.0F;
        }
        const local_ref<jobject> metrics{env, env->CallObjectMethod(resources.get(), get_display_metrics)};
        if (pending_exception_cleared(env, "getDisplayMetrics") || !metrics)
        {
            return 1.0F;
        }
        return env->GetFloatField(metrics.get(), density_field);
    }

    // ContextExtensions.ToPixels, mirrored for the expected values.
    [[nodiscard]] jint to_pixels(double dp, float density)
    {
        return static_cast<jint>(std::ceil((dp * static_cast<double>(density)) - 0.0000000001));
    }

    // ---- virtual → native ----

    TEST(android_button, attach_creates_a_real_android_widget_button)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        const attached_button seam;
        ASSERT_NE(seam.widget(), nullptr) << "the android partial did not create a widget";
        jclass button_class = default_jni_cache().find_class(env.get(), k_button_class);
        ASSERT_NE(button_class, nullptr);
        EXPECT_EQ(env->IsInstanceOf(seam.widget(), button_class), JNI_TRUE);
    }

    TEST(android_button, text_reaches_the_widget_including_supplementary_plane)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_button seam;
        // U+1F9E9 (🧩) pins the real-UTF-8 jstring path through the WHOLE control→widget pipeline.
        const std::string expected = "tap me \xF0\x9F\xA7\xA9";
        seam.control.set_text(expected);
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getText"), expected);
    }

    TEST(android_button, text_color_reaches_the_widget_as_argb)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_button seam;
        const maui::graphics::color red(1.0F, 0.0F, 0.0F);
        seam.control.set_text_color(red);
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getCurrentTextColor"), static_cast<jint>(red.to_int()));
    }

    TEST(android_button, is_enabled_reaches_the_widget)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_button seam;
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_TRUE(call_bool(env.get(), seam.widget(), "isEnabled"));
        seam.control.set_is_enabled(false);
        EXPECT_FALSE(call_bool(env.get(), seam.widget(), "isEnabled"));
        seam.control.set_is_enabled(true);
        EXPECT_TRUE(call_bool(env.get(), seam.widget(), "isEnabled"));
    }

    TEST(android_button, visibility_maps_to_the_three_view_states)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_button seam;
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_visible);
        seam.control.set_visibility(maui::core::visibility::hidden);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_invisible);
        seam.control.set_visibility(maui::core::visibility::collapsed);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_gone);
        seam.control.set_visibility(maui::core::visibility::visible);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_visible);
    }

    TEST(android_button, opacity_reaches_the_widget_alpha)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_button seam;
        seam.control.set_opacity(0.5);
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_NEAR(call_float(env.get(), seam.widget(), "getAlpha"), 0.5F, 1e-4F);
    }

    TEST(android_button, automation_id_reaches_the_content_description)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_button seam;
        seam.control.set_automation_id("submit_button");
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getContentDescription"), "submit_button");
    }

    TEST(android_button, padding_reaches_the_widget_in_pixels)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_button seam;
        seam.control.set_padding(maui::core::thickness(8, 4, 12, 6));
        ASSERT_NE(seam.widget(), nullptr);
        const float density = host_density(env.get());
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getPaddingLeft"), to_pixels(8, density));
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getPaddingTop"), to_pixels(4, density));
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getPaddingRight"), to_pixels(12, density));
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getPaddingBottom"), to_pixels(6, density));
    }

    TEST(android_button, font_size_and_bold_reach_the_widget)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_button seam;
        // auto-scaling off → ComplexUnitType.Dip (FontManager.GetFontSize), so the rendered pixel
        // size is size * density — independent of the user's font-scale setting.
        seam.control.set_font(
            maui::core::font::system_font_of_size(20, maui::core::font_weight::bold).with_auto_scaling(false));
        ASSERT_NE(seam.widget(), nullptr);
        const float density = host_density(env.get());
        EXPECT_NEAR(call_float(env.get(), seam.widget(), "getTextSize"), 20.0F * density, 0.5F);

        auto& cache = default_jni_cache();
        jmethodID get_typeface =
            cache.method(env.get(), k_button_class, "getTypeface", "()Landroid/graphics/Typeface;");
        jmethodID is_bold = cache.method(env.get(), "android/graphics/Typeface", "isBold", "()Z");
        ASSERT_NE(get_typeface, nullptr);
        ASSERT_NE(is_bold, nullptr);
        const local_ref<jobject> typeface{env.get(), env->CallObjectMethod(seam.widget(), get_typeface)};
        ASSERT_FALSE(pending_exception_cleared(env.get(), "getTypeface"));
        ASSERT_TRUE(typeface);
        EXPECT_EQ(env->CallBooleanMethod(typeface.get(), is_bold), JNI_TRUE);
        EXPECT_FALSE(pending_exception_cleared(env.get(), "isBold"));
    }

    TEST(android_button, character_spacing_reaches_letter_spacing_in_em)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_button seam;
        seam.control.set_character_spacing(2.5);
        ASSERT_NE(seam.widget(), nullptr);
        // UnitExtensions.ToEm: 2.5pt * 0.0624 = 0.156em.
        EXPECT_NEAR(call_float(env.get(), seam.widget(), "getLetterSpacing"), 2.5F * 0.0624F, 1e-4F);
    }

    TEST(android_button, background_corner_radius_and_stroke_land_in_one_gradient_drawable)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_button seam;
        ASSERT_NE(seam.widget(), nullptr);
        auto& cache = default_jni_cache();
        jmethodID get_background =
            cache.method(env.get(), k_button_class, "getBackground", "()Landroid/graphics/drawable/Drawable;");
        jclass gradient_class = cache.find_class(env.get(), k_gradient_drawable_class);
        ASSERT_NE(get_background, nullptr);
        ASSERT_NE(gradient_class, nullptr);

        // The untouched button keeps its DEFAULT theme background (the lazy-install gate).
        {
            const local_ref<jobject> initial{env.get(), env->CallObjectMethod(seam.widget(), get_background)};
            ASSERT_FALSE(pending_exception_cleared(env.get(), "getBackground (initial)"));
            EXPECT_TRUE(!initial || env->IsInstanceOf(initial.get(), gradient_class) == JNI_FALSE)
                << "the maui GradientDrawable must not be installed before any visual property is set";
        }

        const maui::graphics::color blue(0.0F, 0.0F, 1.0F);
        const maui::graphics::color green(0.0F, 1.0F, 0.0F);
        seam.control.set_background(std::make_shared<maui::graphics::solid_paint>(blue));
        seam.control.set_stroke_color(green);
        seam.control.set_stroke_thickness(2.0);
        seam.control.set_corner_radius(8);

        const local_ref<jobject> drawable{env.get(), env->CallObjectMethod(seam.widget(), get_background)};
        ASSERT_FALSE(pending_exception_cleared(env.get(), "getBackground"));
        ASSERT_TRUE(drawable);
        ASSERT_EQ(env->IsInstanceOf(drawable.get(), gradient_class), JNI_TRUE)
            << "the maui GradientDrawable was not installed";

        const float density = host_density(env.get());
        jmethodID get_corner_radius = cache.method(env.get(), k_gradient_drawable_class, "getCornerRadius", "()F");
        ASSERT_NE(get_corner_radius, nullptr);
        const jfloat radius = env->CallFloatMethod(drawable.get(), get_corner_radius);
        ASSERT_FALSE(pending_exception_cleared(env.get(), "getCornerRadius"));
        EXPECT_NEAR(radius, static_cast<jfloat>(to_pixels(8, density)), 0.5F);

        // GradientDrawable.getColor() (API 24+) returns the fill as a ColorStateList.
        jmethodID get_color =
            cache.method(env.get(), k_gradient_drawable_class, "getColor", "()Landroid/content/res/ColorStateList;");
        jmethodID get_default_color =
            cache.method(env.get(), "android/content/res/ColorStateList", "getDefaultColor", "()I");
        ASSERT_NE(get_color, nullptr);
        ASSERT_NE(get_default_color, nullptr);
        const local_ref<jobject> fill{env.get(), env->CallObjectMethod(drawable.get(), get_color)};
        ASSERT_FALSE(pending_exception_cleared(env.get(), "getColor"));
        ASSERT_TRUE(fill);
        EXPECT_EQ(env->CallIntMethod(fill.get(), get_default_color), static_cast<jint>(blue.to_int()));
        EXPECT_FALSE(pending_exception_cleared(env.get(), "getDefaultColor"));
        // GradientDrawable exposes no stroke getters; the stroke push is covered by reaching here
        // exception-free with the stroke applied to the same drawable.
    }

    TEST(android_button, measure_returns_a_text_dependent_size)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_button seam;
        ASSERT_NE(seam.widget(), nullptr);
        seam.control.set_text("M");
        const maui::graphics::size small = seam.handler->get_desired_size(1000, 1000);
        EXPECT_GT(small.width, 0);
        EXPECT_GT(small.height, 0);
        seam.control.set_text("a considerably longer button title");
        const maui::graphics::size large = seam.handler->get_desired_size(1000, 1000);
        EXPECT_GT(large.width, small.width) << "the real widget should measure wider for longer text";
    }

    // ---- native → virtual ----

    TEST(android_button, perform_click_raises_the_clicked_event)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_button seam;
        ASSERT_NE(seam.widget(), nullptr);
        int clicks = 0;
        seam.control.clicked.connect([&clicks] { ++clicks; });
        // The REAL android event pipeline: View.performClick() invokes the installed
        // NativeOnClickListener, which crosses back over RegisterNatives into send_clicked.
        jmethodID perform_click = default_jni_cache().method(env.get(), k_button_class, "performClick", "()Z");
        ASSERT_NE(perform_click, nullptr);
        const jboolean handled = env->CallBooleanMethod(seam.widget(), perform_click);
        ASSERT_FALSE(pending_exception_cleared(env.get(), "performClick"));
        EXPECT_EQ(handled, JNI_TRUE) << "no OnClickListener was installed";
        EXPECT_EQ(clicks, 1);
    }

    TEST(android_button, perform_click_after_disconnect_is_inert)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        ASSERT_NE(platform->native, nullptr);
        // Keep the widget alive past the disconnect (the platform struct dies with the detach).
        jobject widget = env->NewGlobalRef(static_cast<jobject>(platform->native));
        ASSERT_NE(widget, nullptr);
        int clicks = 0;
        control.clicked.connect([&clicks] { ++clicks; });
        control.set_handler(nullptr); // DisconnectHandler: SetOnClickListener(null)
        jmethodID perform_click = default_jni_cache().method(env.get(), k_button_class, "performClick", "()Z");
        ASSERT_NE(perform_click, nullptr);
        const jboolean handled = env->CallBooleanMethod(widget, perform_click);
        EXPECT_FALSE(pending_exception_cleared(env.get(), "performClick (disconnected)"));
        EXPECT_EQ(handled, JNI_FALSE) << "the click listener must be uninstalled on disconnect";
        EXPECT_EQ(clicks, 0);
        env->DeleteGlobalRef(widget);
    }
} // namespace
