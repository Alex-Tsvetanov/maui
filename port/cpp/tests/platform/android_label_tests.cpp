// Android label seam tests (M-android fan-out) — the display-only Rosetta replayed over JNI, ON the
// emulator inside the app_process widget test host (tools/android-testhost-run.sh): the REAL
// maui::controls::label drives the REAL android.widget.TextView through the cross-platform
// label_handler's android partial (src/platform/android/label_handler.cpp), and every assertion reads
// the widget state BACK through JNI getters (getText / getCurrentTextColor / isEnabled / getVisibility /
// getAlpha / getContentDescription / getPadding* / getTextSize / getTypeface / getLetterSpacing /
// getTextAlignment / getGravity / getPaintFlags / getMaxLines).
//
// Characterization target: LabelHandler.Android.cs + TextViewExtensions.cs (the documented plain-widget
// deviations — plain TextView vs MauiTextView, deferred line-break ellipsize / formatted text / solid-
// only background — are in the partial's header).

#include <cmath>
#include <cstdint>
#include <memory>
#include <string>

#include <gtest/gtest.h>
#include <jni.h>

#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/controls/label.hpp"
#include "maui/core/font.hpp"
#include "maui/core/label_handler.hpp"
#include "maui/core/line_break_mode.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/text_decorations.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"

namespace
{
    using maui::controls::label;
    using maui::core::label_handler;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;
    using maui::platform::android::to_utf8;

    constexpr const char* k_text_view_class = "android/widget/TextView";

    // android.view.View visibility states.
    constexpr jint k_view_visible = 0;
    constexpr jint k_view_invisible = 4;
    constexpr jint k_view_gone = 8;

    // android.view.Gravity bits (vertical alignment is observable on an unattached view; horizontal
    // TextAlignment is not — see the horizontal-alignment test's attachment note).
    constexpr jint k_gravity_top = 0x30;
    constexpr jint k_gravity_center_vertical = 0x10;
    constexpr jint k_gravity_bottom = 0x50;
    constexpr jint k_gravity_vertical_mask = 0x70;
    constexpr jint k_paint_underline = 0x08;
    constexpr jint k_paint_strike_thru = 0x10;

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

    // The control + attached handler + the real widget, torn down in declaration order (handler detach
    // precedes the control's death; the platform struct releases the widget's global ref).
    struct attached_label
    {
        label control;
        std::shared_ptr<label_handler> handler = std::make_shared<label_handler>();

        attached_label()
        {
            control.set_handler(handler);
        }

        ~attached_label()
        {
            control.set_handler(nullptr);
        }

        attached_label(const attached_label&) = delete;
        attached_label(attached_label&&) = delete;
        attached_label& operator=(const attached_label&) = delete;
        attached_label& operator=(attached_label&&) = delete;

        [[nodiscard]] jobject widget() const
        {
            auto* platform = handler->typed_platform_view();
            return platform != nullptr ? static_cast<jobject>(platform->native) : nullptr;
        }
    };

    // ---- typed JNI read-backs (each fails the test and returns a benign default on a JNI error) ----

    [[nodiscard]] jint call_int(JNIEnv* env, jobject widget, const char* name)
    {
        jmethodID method = default_jni_cache().method(env, k_text_view_class, name, "()I");
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
        jmethodID method = default_jni_cache().method(env, k_text_view_class, name, "()F");
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
        jmethodID method = default_jni_cache().method(env, k_text_view_class, name, "()Z");
        if (method == nullptr)
        {
            ADD_FAILURE() << name << "()Z not found";
            return false;
        }
        const jboolean value = env->CallBooleanMethod(widget, method);
        return !pending_exception_cleared(env, name) && value == JNI_TRUE;
    }

    [[nodiscard]] std::string call_char_sequence(JNIEnv* env, jobject widget, const char* name)
    {
        auto& cache = default_jni_cache();
        jmethodID method = cache.method(env, k_text_view_class, name, "()Ljava/lang/CharSequence;");
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

    [[nodiscard]] float host_density(JNIEnv* env, jobject widget)
    {
        auto& cache = default_jni_cache();
        jmethodID get_context = cache.method(env, k_text_view_class, "getContext", "()Landroid/content/Context;");
        jmethodID get_resources =
            cache.method(env, "android/content/Context", "getResources", "()Landroid/content/res/Resources;");
        jmethodID get_display_metrics =
            cache.method(env, "android/content/res/Resources", "getDisplayMetrics", "()Landroid/util/DisplayMetrics;");
        jfieldID density_field = cache.field(env, "android/util/DisplayMetrics", "density", "F");
        if (get_context == nullptr || get_resources == nullptr || get_display_metrics == nullptr ||
            density_field == nullptr)
        {
            ADD_FAILURE() << "DisplayMetrics.density surface missing";
            return 1.0F;
        }
        const local_ref<jobject> context{env, env->CallObjectMethod(widget, get_context)};
        if (pending_exception_cleared(env, "getContext") || !context)
        {
            return 1.0F;
        }
        const local_ref<jobject> resources{env, env->CallObjectMethod(context.get(), get_resources)};
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

    [[nodiscard]] jint to_pixels(double dp, float density)
    {
        return static_cast<jint>(std::ceil((dp * static_cast<double>(density)) - 0.0000000001));
    }

    // ---- virtual → native ----

    TEST(android_label, attach_creates_a_real_android_widget_text_view)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        const attached_label seam;
        ASSERT_NE(seam.widget(), nullptr) << "the android partial did not create a widget";
        jclass text_view_class = default_jni_cache().find_class(env.get(), k_text_view_class);
        ASSERT_NE(text_view_class, nullptr);
        EXPECT_EQ(env->IsInstanceOf(seam.widget(), text_view_class), JNI_TRUE);
    }

    TEST(android_label, text_reaches_the_widget_including_supplementary_plane)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_label seam;
        // U+1F9E9 (🧩) pins the real-UTF-8 jstring path through the whole control→widget pipeline.
        const std::string expected = "hello \xF0\x9F\xA7\xA9";
        seam.control.set_text(expected);
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getText"), expected);
    }

    TEST(android_label, text_color_reaches_the_widget_as_argb)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_label seam;
        const maui::graphics::color red(1.0F, 0.0F, 0.0F);
        seam.control.set_text_color(red);
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getCurrentTextColor"), static_cast<jint>(red.to_int()));
    }

    TEST(android_label, is_enabled_reaches_the_widget)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_label seam;
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_TRUE(call_bool(env.get(), seam.widget(), "isEnabled"));
        seam.control.set_is_enabled(false);
        EXPECT_FALSE(call_bool(env.get(), seam.widget(), "isEnabled"));
        seam.control.set_is_enabled(true);
        EXPECT_TRUE(call_bool(env.get(), seam.widget(), "isEnabled"));
    }

    TEST(android_label, visibility_maps_to_the_three_view_states)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_label seam;
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_visible);
        seam.control.set_visibility(maui::core::visibility::hidden);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_invisible);
        seam.control.set_visibility(maui::core::visibility::collapsed);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_gone);
        seam.control.set_visibility(maui::core::visibility::visible);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_visible);
    }

    TEST(android_label, opacity_reaches_the_widget_alpha)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_label seam;
        seam.control.set_opacity(0.5);
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_NEAR(call_float(env.get(), seam.widget(), "getAlpha"), 0.5F, 1e-4F);
    }

    TEST(android_label, automation_id_reaches_the_content_description)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_label seam;
        seam.control.set_automation_id("greeting_label");
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getContentDescription"), "greeting_label");
    }

    TEST(android_label, padding_reaches_the_widget_in_pixels)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_label seam;
        seam.control.set_padding(maui::core::thickness(8, 4, 12, 6));
        ASSERT_NE(seam.widget(), nullptr);
        // SetPaddingRelative; in the host's LTR layout start==left, end==right.
        const float density = host_density(env.get(), seam.widget());
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getPaddingLeft"), to_pixels(8, density));
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getPaddingTop"), to_pixels(4, density));
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getPaddingRight"), to_pixels(12, density));
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getPaddingBottom"), to_pixels(6, density));
    }

    TEST(android_label, font_size_and_bold_reach_the_widget)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_label seam;
        seam.control.set_font(
            maui::core::font::system_font_of_size(20, maui::core::font_weight::bold).with_auto_scaling(false));
        ASSERT_NE(seam.widget(), nullptr);
        const float density = host_density(env.get(), seam.widget());
        EXPECT_NEAR(call_float(env.get(), seam.widget(), "getTextSize"), 20.0F * density, 0.5F);

        auto& cache = default_jni_cache();
        jmethodID get_typeface =
            cache.method(env.get(), k_text_view_class, "getTypeface", "()Landroid/graphics/Typeface;");
        jmethodID is_bold = cache.method(env.get(), "android/graphics/Typeface", "isBold", "()Z");
        ASSERT_NE(get_typeface, nullptr);
        ASSERT_NE(is_bold, nullptr);
        const local_ref<jobject> typeface{env.get(), env->CallObjectMethod(seam.widget(), get_typeface)};
        ASSERT_FALSE(pending_exception_cleared(env.get(), "getTypeface"));
        ASSERT_TRUE(typeface);
        EXPECT_EQ(env->CallBooleanMethod(typeface.get(), is_bold), JNI_TRUE);
        EXPECT_FALSE(pending_exception_cleared(env.get(), "isBold"));
    }

    TEST(android_label, character_spacing_reaches_letter_spacing_in_em)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_label seam;
        seam.control.set_character_spacing(2.5);
        ASSERT_NE(seam.widget(), nullptr);
        // UnitExtensions.ToEm: 2.5pt * 0.0624 = 0.156em.
        EXPECT_NEAR(call_float(env.get(), seam.widget(), "getLetterSpacing"), 2.5F * 0.0624F, 1e-4F);
    }

    TEST(android_label, horizontal_text_alignment_pushes_to_the_widget)
    {
        // ATTACHMENT NOTE: unlike Gravity (a plain stored field, observable below), TextView.TextAlignment
        // only resolves once the view is attached to a window — a direct setTextAlignment(Center) followed
        // by getTextAlignment() still reads the default GRAVITY(1) in the app_process testhost (verified by
        // probe). MAUI's own LabelHandlerTests.Android attaches the view to assert the native value; the
        // testhost has no window, so here we verify the handler ran the push (the mirror reflects the value)
        // without a JNI fault. The handler is faithful to TextViewExtensions.UpdateHorizontalTextAlignment
        // (View.TextAlignment = ToTextAlignment, the Rtl-supported path).
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_label seam;
        ASSERT_NE(seam.widget(), nullptr);
        auto* platform = seam.handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        seam.control.set_horizontal_text_alignment(maui::core::text_alignment::center);
        EXPECT_EQ(platform->horizontal_alignment, maui::core::text_alignment::center);
        seam.control.set_horizontal_text_alignment(maui::core::text_alignment::end);
        EXPECT_EQ(platform->horizontal_alignment, maui::core::text_alignment::end);
        seam.control.set_horizontal_text_alignment(maui::core::text_alignment::start);
        EXPECT_EQ(platform->horizontal_alignment, maui::core::text_alignment::start);
        EXPECT_FALSE(env->ExceptionCheck()) << "a horizontal-alignment JNI push left a pending exception";
    }

    TEST(android_label, vertical_text_alignment_reaches_the_gravity_vertical_bits)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_label seam;
        ASSERT_NE(seam.widget(), nullptr);
        seam.control.set_vertical_text_alignment(maui::core::text_alignment::start);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getGravity") & k_gravity_vertical_mask, k_gravity_top);
        seam.control.set_vertical_text_alignment(maui::core::text_alignment::center);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getGravity") & k_gravity_vertical_mask,
                  k_gravity_center_vertical);
        seam.control.set_vertical_text_alignment(maui::core::text_alignment::end);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getGravity") & k_gravity_vertical_mask, k_gravity_bottom);
    }

    TEST(android_label, text_decorations_toggle_the_paint_flags)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_label seam;
        ASSERT_NE(seam.widget(), nullptr);
        seam.control.set_text_decorations(maui::core::text_decorations::underline);
        EXPECT_NE(call_int(env.get(), seam.widget(), "getPaintFlags") & k_paint_underline, 0);
        seam.control.set_text_decorations(maui::core::text_decorations::strikethrough);
        const jint flags = call_int(env.get(), seam.widget(), "getPaintFlags");
        EXPECT_NE(flags & k_paint_strike_thru, 0);
        EXPECT_EQ(flags & k_paint_underline, 0); // setting strikethrough clears the underline bit
    }

    TEST(android_label, max_lines_reaches_the_widget)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_label seam;
        ASSERT_NE(seam.widget(), nullptr);
        seam.control.set_max_lines(3);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getMaxLines"), 3);
    }
} // namespace
