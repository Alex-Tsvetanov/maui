// Android time_picker seam tests (M-android fan-out) — the time-of-day field replayed over JNI, ON the
// emulator inside the app_process widget test host: the REAL maui::controls::time_picker drives the REAL
// android.widget.EditText (made non-editable, the plain-widget stand-in for MauiTimePicker) through the
// cross-platform time_picker_handler's android partial (src/platform/android/time_picker_handler.cpp),
// and every assertion reads the widget state BACK through JNI getters (getText / getCurrentTextColor /
// isEnabled / getVisibility / getAlpha / getContentDescription / getTypeface / getLetterSpacing).
//
// Characterization target: TimePickerHandler.Android.cs + TimePickerExtensions.SetTimeImpl (editText.Text
// = time?.ToFormattedString(format)) + TimeExtensions.ToFormattedString (DateTime.Today.Add(time)
// .ToString(format); empty format → short-time pattern) + UpdateTextColorImpl + the shared TextView
// extensions (EditTextExtensions / TextViewExtensions). The displayed text is the invariant collapse of
// ToFormattedString: the default Format "t" renders the short 12-HOUR en-US time ("h:mm tt") — the SAME
// format_time_span the headless + iOS mirrors use; NO 24h/12h logic is invented here (the C# handler's
// Use24HourView governs only the DEFERRED TimePickerDialog wheel, not the field text). The selection
// DIALOG (the android.app.TimePickerDialog opened on Click) is the documented deferred half (see the
// partial's header); these tests cover the property pushes (virtual → native display state).
//
// NOTE (lesson 3, MACOS_ANDROID_RESUME.md): the time-picker widget is EditText-derived, whose ctor +
// setText can trip the Settings/DeviceConfig ContentProvider query (TextClassification / AutofillManager)
// that the bare shell-uid app_process testhost cannot reach → SecurityException. So like the
// picker/editor/switch/check_box seam files, this one is written for the Android APP HOST (a real
// Activity), and the integrator decides whether to wire it into the testhost. It is kept ready for the
// app-host verification.

#include <memory>
#include <string>

#include <gtest/gtest.h>
#include <jni.h>

#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/controls/time_picker.hpp"
#include "maui/core/date_time.hpp"
#include "maui/core/font.hpp"
#include "maui/core/time_picker_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"

namespace
{
    using maui::controls::time_picker;
    using maui::core::time_picker_handler;
    using maui::core::time_span;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;
    using maui::platform::android::to_utf8;

    constexpr const char* k_edit_text_class = "android/widget/EditText";

    constexpr jint k_view_visible = 0;
    constexpr jint k_view_invisible = 4;
    constexpr jint k_view_gone = 8;

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

    struct attached_time_picker
    {
        time_picker control;
        std::shared_ptr<time_picker_handler> handler = std::make_shared<time_picker_handler>();

        attached_time_picker()
        {
            control.set_handler(handler);
        }

        ~attached_time_picker()
        {
            control.set_handler(nullptr);
        }

        attached_time_picker(const attached_time_picker&) = delete;
        attached_time_picker(attached_time_picker&&) = delete;
        attached_time_picker& operator=(const attached_time_picker&) = delete;
        attached_time_picker& operator=(attached_time_picker&&) = delete;

        [[nodiscard]] jobject widget() const
        {
            auto* platform = handler->typed_platform_view();
            return platform != nullptr ? static_cast<jobject>(platform->native) : nullptr;
        }
    };

    [[nodiscard]] jint call_int(JNIEnv* env, jobject widget, const char* name)
    {
        jmethodID method = default_jni_cache().method(env, k_edit_text_class, name, "()I");
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
        jmethodID method = default_jni_cache().method(env, k_edit_text_class, name, "()F");
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
        jmethodID method = default_jni_cache().method(env, k_edit_text_class, name, "()Z");
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
        jmethodID method = cache.method(env, k_edit_text_class, name, "()Ljava/lang/CharSequence;");
        jmethodID to_string = cache.method(env, "java/lang/Object", "toString", "()Ljava/lang/String;");
        if (method == nullptr || to_string == nullptr)
        {
            ADD_FAILURE() << name << " surface missing";
            return {};
        }
        const local_ref<jobject> sequence{env, env->CallObjectMethod(widget, method)};
        if (pending_exception_cleared(env, name) || !sequence)
        {
            return {}; // getText() is null until set — a benign empty, not a failure
        }
        const local_ref<jstring> text{env, static_cast<jstring>(env->CallObjectMethod(sequence.get(), to_string))};
        if (pending_exception_cleared(env, "CharSequence.toString") || !text)
        {
            return {};
        }
        return to_utf8(env, text.get());
    }

    TEST(android_time_picker, attach_creates_a_real_android_widget_edit_text)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        const attached_time_picker seam;
        ASSERT_NE(seam.widget(), nullptr) << "the android partial did not create a widget";
        jclass edit_text_class = default_jni_cache().find_class(env.get(), k_edit_text_class);
        ASSERT_NE(edit_text_class, nullptr);
        EXPECT_EQ(env->IsInstanceOf(seam.widget(), edit_text_class), JNI_TRUE);
    }

    TEST(android_time_picker, the_field_is_non_editable)
    {
        // MauiTimePicker is a read-only EditText (DefaultMovementMethod = null); the partial reproduces
        // the intent with setFocusable(false) + setClickable(true). The field accepts no keyboard input.
        const scoped_env env;
        ASSERT_TRUE(env);
        const attached_time_picker seam;
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_FALSE(call_bool(env.get(), seam.widget(), "isFocusable"));
        EXPECT_TRUE(call_bool(env.get(), seam.widget(), "isClickable"));
    }

    TEST(android_time_picker, default_zero_time_renders_the_12_hour_short_time)
    {
        // TimeProperty default is TimeSpan.Zero (not null), FormatProperty default is "t" → "h:mm tt"
        // (the invariant short 12-hour en-US time). Zero renders as "12:00 AM". This is the SAME
        // format_time_span the headless/iOS mirrors use — the deliberate 12h default, not invented.
        const scoped_env env;
        ASSERT_TRUE(env);
        const attached_time_picker seam;
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getText"), "12:00 AM");
    }

    TEST(android_time_picker, afternoon_time_renders_pm_in_the_default_format)
    {
        // SetTimeImpl: editText.Text = time.ToFormattedString("t") → "h:mm tt". 13:30 → "1:30 PM".
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_time_picker seam;
        seam.control.set_time(time_span(13, 30, 0));
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getText"), "1:30 PM");
    }

    TEST(android_time_picker, morning_time_renders_am_with_leading_hour_unpadded)
    {
        // "h" is the non-zero-padded 12-hour hour; 09:05 → "9:05 AM".
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_time_picker seam;
        seam.control.set_time(time_span(9, 5, 0));
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getText"), "9:05 AM");
    }

    TEST(android_time_picker, custom_24_hour_format_renders_HH_mm)
    {
        // A custom Format string flows through ToFormattedString unchanged: "HH:mm" is the zero-padded
        // 24-hour pattern. 13:30 → "13:30". (The format change re-renders via MapFormat → SetTimeImpl.)
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_time_picker seam;
        seam.control.set_time(time_span(13, 30, 0));
        seam.control.set_format("HH:mm");
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getText"), "13:30");
    }

    TEST(android_time_picker, changing_the_format_re_renders_the_field)
    {
        // MapFormat routes into SetTimeImpl, so a format change re-pushes the field text without a new
        // Time being set — the same 13:30 first shows 12-hour, then 24-hour.
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_time_picker seam;
        seam.control.set_time(time_span(13, 30, 0));
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getText"), "1:30 PM");
        seam.control.set_format("HH:mm");
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getText"), "13:30");
    }

    TEST(android_time_picker, text_color_reaches_the_widget_as_argb)
    {
        // TimePickerExtensions.UpdateTextColorImpl → SetTextColor(textColor.ToPlatform()).
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_time_picker seam;
        const maui::graphics::color red(1.0F, 0.0F, 0.0F);
        seam.control.set_text_color(red);
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getCurrentTextColor"), static_cast<jint>(red.to_int()));
    }

    TEST(android_time_picker, is_enabled_reaches_the_widget)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_time_picker seam;
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_TRUE(call_bool(env.get(), seam.widget(), "isEnabled"));
        seam.control.set_is_enabled(false);
        EXPECT_FALSE(call_bool(env.get(), seam.widget(), "isEnabled"));
    }

    TEST(android_time_picker, visibility_maps_to_the_three_view_states)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_time_picker seam;
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_visible);
        seam.control.set_visibility(maui::core::visibility::hidden);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_invisible);
        seam.control.set_visibility(maui::core::visibility::collapsed);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_gone);
    }

    TEST(android_time_picker, opacity_reaches_the_widget_alpha)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_time_picker seam;
        seam.control.set_opacity(0.5);
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_NEAR(call_float(env.get(), seam.widget(), "getAlpha"), 0.5F, 1e-4F);
    }

    TEST(android_time_picker, automation_id_reaches_the_content_description)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_time_picker seam;
        seam.control.set_automation_id("alarm_time");
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getContentDescription"), "alarm_time");
    }

    TEST(android_time_picker, font_size_and_bold_reach_the_widget)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_time_picker seam;
        seam.control.set_font(
            maui::core::font::system_font_of_size(20, maui::core::font_weight::bold).with_auto_scaling(false));
        ASSERT_NE(seam.widget(), nullptr);

        auto& cache = default_jni_cache();
        jmethodID get_typeface =
            cache.method(env.get(), k_edit_text_class, "getTypeface", "()Landroid/graphics/Typeface;");
        jmethodID is_bold = cache.method(env.get(), "android/graphics/Typeface", "isBold", "()Z");
        ASSERT_NE(get_typeface, nullptr);
        ASSERT_NE(is_bold, nullptr);
        const local_ref<jobject> typeface{env.get(), env->CallObjectMethod(seam.widget(), get_typeface)};
        ASSERT_FALSE(pending_exception_cleared(env.get(), "getTypeface"));
        ASSERT_TRUE(typeface);
        EXPECT_EQ(env->CallBooleanMethod(typeface.get(), is_bold), JNI_TRUE);
        EXPECT_FALSE(pending_exception_cleared(env.get(), "isBold"));
    }

    TEST(android_time_picker, character_spacing_reaches_letter_spacing_in_em)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_time_picker seam;
        seam.control.set_character_spacing(2.5);
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_NEAR(call_float(env.get(), seam.widget(), "getLetterSpacing"), 2.5F * 0.0624F, 1e-4F);
    }

    TEST(android_time_picker, done_commit_drops_seconds_and_re_renders)
    {
        // The Done-accessory analog (SetVirtualViewTime, seconds dropped): the on_done callback commits the
        // wheel's current `time` as new TimeSpan(hour, minute, 0) back to the virtual view, which re-runs
        // MapTime → SetTimeImpl (re-pushing the field). Drive it directly (the deferred dialog's stand-in).
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_time_picker seam;
        seam.control.set_time(time_span(7, 45, 0));
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getText"), "7:45 AM");

        auto* platform = seam.handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        ASSERT_TRUE(static_cast<bool>(platform->on_done));
        platform->on_done(); // simulate the native pick commit
        EXPECT_EQ(seam.control.time().value(), time_span(7, 45, 0));
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getText"), "7:45 AM");
    }
} // namespace
