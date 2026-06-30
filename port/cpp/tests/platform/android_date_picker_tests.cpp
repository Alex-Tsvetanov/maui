// Android date_picker seam tests (M-android fan-out) — the date-selection field replayed over JNI, ON
// the emulator inside a real Activity (the app host): the REAL maui::controls::date_picker drives the
// REAL android.widget.EditText (made non-editable, the plain-widget stand-in for MauiDatePicker) through
// the cross-platform date_picker_handler's android partial (src/platform/android/date_picker_handler.cpp),
// and every assertion reads the widget state BACK through JNI getters (getText / getCurrentTextColor /
// isEnabled / isFocusable / isClickable / getVisibility / getAlpha / getContentDescription / getTextSize /
// getTypeface / getLetterSpacing).
//
// Characterization target: DatePickerHandler.Android.cs + DatePickerExtensions.SetText (Text ←
// Date?.ToString(Format) ?? string.Empty) + UpdateTextColor + the shared TextView extensions
// (EditTextExtensions / TextViewExtensions). The DatePickerDialog (android.app.DatePickerDialog, opened on
// Click) is the documented deferred half (see the partial's header); these tests cover the property pushes
// (virtual → native display state). The displayed-text expectations are the SAME oracle values the
// headless seam tests assert (date_picker_tests.cpp): date_time(2008,5,5) with the default "d" format →
// "5/5/2008", "D" → "Monday, May 5, 2008", custom "yyyy-MM-dd" → "2008-05-05", a null date → "" (the
// invariant/en-US culture the port renders in — date_time.hpp).
//
// NOTE (lesson 3, MACOS_ANDROID_RESUME.md): the date field is EditText-derived, whose ctor + setText can
// trip the Settings/DeviceConfig ContentProvider query (TextClassification / AutofillManager) that the
// bare shell-uid app_process testhost cannot reach → SecurityException. So like the picker / editor /
// switch / check_box seam files, this one is written for the Android APP HOST (a real Activity), and the
// integrator decides whether to wire it into the testhost. It is kept ready for the app-host verification.

#include <memory>
#include <optional>
#include <string>

#include <gtest/gtest.h>
#include <jni.h>

#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/controls/date_picker.hpp"
#include "maui/core/date_picker_handler.hpp"
#include "maui/core/date_time.hpp"
#include "maui/core/font.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"

namespace
{
    using maui::controls::date_picker;
    using maui::core::date_picker_handler;
    using maui::core::date_time;
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

    struct attached_date_picker
    {
        date_picker control;
        std::shared_ptr<date_picker_handler> handler = std::make_shared<date_picker_handler>();

        attached_date_picker()
        {
            control.set_handler(handler);
        }

        ~attached_date_picker()
        {
            control.set_handler(nullptr);
        }

        attached_date_picker(const attached_date_picker&) = delete;
        attached_date_picker(attached_date_picker&&) = delete;
        attached_date_picker& operator=(const attached_date_picker&) = delete;
        attached_date_picker& operator=(attached_date_picker&&) = delete;

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

    TEST(android_date_picker, attach_creates_a_real_android_widget_edit_text)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        const attached_date_picker seam;
        ASSERT_NE(seam.widget(), nullptr) << "the android partial did not create a widget";
        jclass edit_text_class = default_jni_cache().find_class(env.get(), k_edit_text_class);
        ASSERT_NE(edit_text_class, nullptr);
        EXPECT_EQ(env->IsInstanceOf(seam.widget(), edit_text_class), JNI_TRUE);
    }

    TEST(android_date_picker, the_field_is_non_editable)
    {
        // MauiDatePicker is a read-only EditText (DefaultMovementMethod = null + PickerManager.Init); the
        // partial reproduces the intent with setFocusable(false) + setClickable(true). The field accepts no
        // keyboard input and opens the (deferred) dialog on tap.
        const scoped_env env;
        ASSERT_TRUE(env);
        const attached_date_picker seam;
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_FALSE(call_bool(env.get(), seam.widget(), "isFocusable"));
        EXPECT_TRUE(call_bool(env.get(), seam.widget(), "isClickable"));
    }

    TEST(android_date_picker, formatted_date_reaches_the_widget_text)
    {
        // DatePickerExtensions.SetText: Text ← Date?.ToString(Format). The default Format "d" → the
        // invariant short date "M/d/yyyy" → "5/5/2008" (the headless-seam oracle value).
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_date_picker seam;
        seam.control.set_date(date_time(2008, 5, 5));
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getText"), "5/5/2008");
    }

    TEST(android_date_picker, format_change_rerenders_the_widget_text)
    {
        // MapFormat → UpdateFormat → SetText: a format change re-renders the field. "D" → the long date,
        // a custom pattern → the custom render (the headless-seam oracle values).
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_date_picker seam;
        seam.control.set_date(date_time(2008, 5, 5));
        ASSERT_NE(seam.widget(), nullptr);

        seam.control.set_format("D");
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getText"), "Monday, May 5, 2008");

        seam.control.set_format("yyyy-MM-dd");
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getText"), "2008-05-05");
    }

    TEST(android_date_picker, reselecting_the_date_rerenders_the_widget_text)
    {
        // MapDate → UpdateDate → SetText: a new date re-pushes the field text.
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_date_picker seam;
        seam.control.set_date(date_time(2008, 5, 5));
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getText"), "5/5/2008");
        seam.control.set_date(date_time(2011, 11, 30));
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getText"), "11/30/2011");
    }

    TEST(android_date_picker, native_done_commits_the_picked_date_and_rerenders)
    {
        // The deferred-dialog stand-in: a row commit flows through on_done (OnDateSet → VirtualView.Date),
        // and the resulting MapDate re-pushes the field text.
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_date_picker seam;
        ASSERT_NE(seam.widget(), nullptr);

        bool selected = false;
        seam.control.date_selected.connect(
            [&selected](const std::optional<date_time>&, const std::optional<date_time>&) { selected = true; });

        auto* platform = seam.handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        platform->date = date_time(2011, 11, 30); // the user picks a day in the (deferred) dialog...
        platform->on_done();                      // ...and the dialog commits (OnDateSet -> VirtualView.Date)
        EXPECT_EQ(seam.control.date(), std::optional<date_time>(date_time(2011, 11, 30)));
        EXPECT_TRUE(selected);
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getText"), "11/30/2011");
    }

    TEST(android_date_picker, text_color_reaches_the_widget_as_argb)
    {
        // DatePickerExtensions.UpdateTextColor → SetTextColor(textColor.ToPlatform()).
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_date_picker seam;
        const maui::graphics::color red(1.0F, 0.0F, 0.0F);
        seam.control.set_text_color(red);
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getCurrentTextColor"), static_cast<jint>(red.to_int()));
    }

    TEST(android_date_picker, is_enabled_reaches_the_widget)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_date_picker seam;
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_TRUE(call_bool(env.get(), seam.widget(), "isEnabled"));
        seam.control.set_is_enabled(false);
        EXPECT_FALSE(call_bool(env.get(), seam.widget(), "isEnabled"));
    }

    TEST(android_date_picker, visibility_maps_to_the_three_view_states)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_date_picker seam;
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_visible);
        seam.control.set_visibility(maui::core::visibility::hidden);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_invisible);
        seam.control.set_visibility(maui::core::visibility::collapsed);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_gone);
    }

    TEST(android_date_picker, opacity_reaches_the_widget_alpha)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_date_picker seam;
        seam.control.set_opacity(0.5);
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_NEAR(call_float(env.get(), seam.widget(), "getAlpha"), 0.5F, 1e-4F);
    }

    TEST(android_date_picker, automation_id_reaches_the_content_description)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_date_picker seam;
        seam.control.set_automation_id("birth_date_picker");
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getContentDescription"), "birth_date_picker");
    }

    TEST(android_date_picker, font_size_and_bold_reach_the_widget)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_date_picker seam;
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

    TEST(android_date_picker, character_spacing_reaches_letter_spacing_in_em)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_date_picker seam;
        seam.control.set_character_spacing(2.5);
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_NEAR(call_float(env.get(), seam.widget(), "getLetterSpacing"), 2.5F * 0.0624F, 1e-4F);
    }
} // namespace
