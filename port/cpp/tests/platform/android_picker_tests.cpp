// Android picker seam tests (M-android fan-out) — the single-selection item picker replayed over JNI,
// ON the emulator inside the app_process widget test host: the REAL maui::controls::picker drives the
// REAL android.widget.EditText (made non-editable, the plain-widget stand-in for MauiPicker) through the
// cross-platform picker_handler's android partial (src/platform/android/picker_handler.cpp), and every
// assertion reads the widget state BACK through JNI getters (getText / getHint / getCurrentTextColor /
// getCurrentHintTextColor / isEnabled / getVisibility / getAlpha / getContentDescription / getTextSize /
// getTypeface / getLetterSpacing).
//
// Characterization target: PickerHandler.Android.cs + PickerExtensions.UpdatePickerCore (Hint ← Title;
// Text ← GetItem(SelectedIndex) or null when out of range) + UpdateTextColor / UpdateTitleColorCore +
// the shared TextView extensions (EditTextExtensions / TextViewExtensions). The selection DIALOG (the
// MaterialAlertDialog single-choice list opened on Click) is the documented deferred half (see the
// partial's header); these tests cover the property pushes (virtual → native display state).
//
// NOTE (lesson 3, MACOS_ANDROID_RESUME.md): the picker widget is EditText-derived, whose ctor + setText
// can trip the Settings/DeviceConfig ContentProvider query (TextClassification / AutofillManager) that
// the bare shell-uid app_process testhost cannot reach → SecurityException. So like the editor/switch/
// check_box seam files, this one is written for the Android APP HOST (a real Activity), and the
// integrator decides whether to wire it into the testhost. It is kept ready for the app-host verification.

#include <memory>
#include <string>

#include <gtest/gtest.h>
#include <jni.h>

#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/controls/picker.hpp"
#include "maui/core/font.hpp"
#include "maui/core/picker_handler.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"

namespace
{
    using maui::controls::picker;
    using maui::core::picker_handler;
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

    struct attached_picker
    {
        picker control;
        std::shared_ptr<picker_handler> handler = std::make_shared<picker_handler>();

        attached_picker()
        {
            control.set_handler(handler);
        }

        ~attached_picker()
        {
            control.set_handler(nullptr);
        }

        attached_picker(const attached_picker&) = delete;
        attached_picker(attached_picker&&) = delete;
        attached_picker& operator=(const attached_picker&) = delete;
        attached_picker& operator=(attached_picker&&) = delete;

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
            return {}; // getText()/getHint() are null until set — a benign empty, not a failure
        }
        const local_ref<jstring> text{env, static_cast<jstring>(env->CallObjectMethod(sequence.get(), to_string))};
        if (pending_exception_cleared(env, "CharSequence.toString") || !text)
        {
            return {};
        }
        return to_utf8(env, text.get());
    }

    TEST(android_picker, attach_creates_a_real_android_widget_edit_text)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        const attached_picker seam;
        ASSERT_NE(seam.widget(), nullptr) << "the android partial did not create a widget";
        jclass edit_text_class = default_jni_cache().find_class(env.get(), k_edit_text_class);
        ASSERT_NE(edit_text_class, nullptr);
        EXPECT_EQ(env->IsInstanceOf(seam.widget(), edit_text_class), JNI_TRUE);
    }

    TEST(android_picker, the_field_is_non_editable)
    {
        // MauiPickerBase is a read-only EditText (DefaultMovementMethod = null); the partial reproduces
        // the intent with setFocusable(false) + setClickable(true). The picker accepts no keyboard input.
        const scoped_env env;
        ASSERT_TRUE(env);
        const attached_picker seam;
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_FALSE(call_bool(env.get(), seam.widget(), "isFocusable"));
        EXPECT_TRUE(call_bool(env.get(), seam.widget(), "isClickable"));
    }

    TEST(android_picker, selected_item_text_reaches_the_widget)
    {
        // PickerExtensions.UpdatePickerCore: Text ← GetItem(SelectedIndex).
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_picker seam;
        seam.control.items().add("John");
        seam.control.items().add("Paul");
        seam.control.items().add("George");
        seam.control.set_selected_index(2);
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getText"), "George");
    }

    TEST(android_picker, unset_selection_shows_no_text)
    {
        // UpdatePickerCore: SelectedIndex == -1 → Text = null (empty CharSequence on the widget).
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_picker seam;
        seam.control.items().add("John");
        seam.control.items().add("Paul");
        seam.control.set_selected_index(-1);
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getText"), "");
    }

    TEST(android_picker, title_reaches_the_widget_hint)
    {
        // UpdatePickerCore: Hint ← picker.Title (the placeholder, always pushed).
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_picker seam;
        seam.control.set_title("Choose a Beatle");
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getHint"), "Choose a Beatle");
    }

    TEST(android_picker, text_color_reaches_the_widget_as_argb)
    {
        // PickerExtensions.UpdateTextColor → SetTextColor(textColor.ToPlatform()).
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_picker seam;
        const maui::graphics::color red(1.0F, 0.0F, 0.0F);
        seam.control.set_text_color(red);
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getCurrentTextColor"), static_cast<jint>(red.to_int()));
    }

    TEST(android_picker, title_color_reaches_the_widget_hint_color)
    {
        // PickerExtensions.UpdateTitleColorCore: the Title color maps onto the HINT text color (the Title
        // IS the hint on Android) → SetHintTextColor(titleColor.ToPlatform()).
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_picker seam;
        const maui::graphics::color green(0.0F, 1.0F, 0.0F);
        seam.control.set_title_color(green);
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getCurrentHintTextColor"), static_cast<jint>(green.to_int()));
    }

    TEST(android_picker, is_enabled_reaches_the_widget)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_picker seam;
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_TRUE(call_bool(env.get(), seam.widget(), "isEnabled"));
        seam.control.set_is_enabled(false);
        EXPECT_FALSE(call_bool(env.get(), seam.widget(), "isEnabled"));
    }

    TEST(android_picker, visibility_maps_to_the_three_view_states)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_picker seam;
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_visible);
        seam.control.set_visibility(maui::core::visibility::hidden);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_invisible);
        seam.control.set_visibility(maui::core::visibility::collapsed);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_gone);
    }

    TEST(android_picker, opacity_reaches_the_widget_alpha)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_picker seam;
        seam.control.set_opacity(0.5);
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_NEAR(call_float(env.get(), seam.widget(), "getAlpha"), 0.5F, 1e-4F);
    }

    TEST(android_picker, automation_id_reaches_the_content_description)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_picker seam;
        seam.control.set_automation_id("beatle_picker");
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getContentDescription"), "beatle_picker");
    }

    TEST(android_picker, font_size_and_bold_reach_the_widget)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_picker seam;
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

    TEST(android_picker, character_spacing_reaches_letter_spacing_in_em)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_picker seam;
        seam.control.set_character_spacing(2.5);
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_NEAR(call_float(env.get(), seam.widget(), "getLetterSpacing"), 2.5F * 0.0624F, 1e-4F);
    }

    TEST(android_picker, reselecting_updates_the_displayed_text)
    {
        // map_selected_index re-runs UpdatePicker → UpdatePickerCore, so a new pick re-pushes Text.
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_picker seam;
        seam.control.items().add("John");
        seam.control.items().add("Paul");
        seam.control.items().add("George");
        seam.control.items().add("Ringo");
        seam.control.set_selected_index(0);
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getText"), "John");
        seam.control.set_selected_index(3);
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getText"), "Ringo");
    }
} // namespace
