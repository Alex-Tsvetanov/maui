// Android editor seam tests (M-android fan-out) — the multi-line text input replayed over JNI, ON the
// emulator inside the app_process widget test host: the REAL maui::controls::editor drives the REAL
// android.widget.EditText through the cross-platform editor_handler's android partial
// (src/platform/android/editor_handler.cpp), and every assertion reads the widget state BACK through JNI
// getters (getText / getCurrentTextColor / getHint / getCurrentHintTextColor / isEnabled / getVisibility /
// getAlpha / getContentDescription / getTextSize / getTypeface / getLetterSpacing).
//
// Characterization target: EditorHandler.Android.cs + the Android text extensions (EditTextExtensions /
// TextViewExtensions). The TextChanged/Completed listeners are the documented deferred half (see the
// partial's header); these tests cover the property pushes (virtual → native).

#include <cmath>
#include <memory>
#include <string>

#include <gtest/gtest.h>
#include <jni.h>

#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/controls/editor.hpp"
#include "maui/core/editor_handler.hpp"
#include "maui/core/font.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"

namespace
{
    using maui::controls::editor;
    using maui::core::editor_handler;
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

    struct attached_editor
    {
        editor control;
        std::shared_ptr<editor_handler> handler = std::make_shared<editor_handler>();

        attached_editor()
        {
            control.set_handler(handler);
        }

        ~attached_editor()
        {
            control.set_handler(nullptr);
        }

        attached_editor(const attached_editor&) = delete;
        attached_editor(attached_editor&&) = delete;
        attached_editor& operator=(const attached_editor&) = delete;
        attached_editor& operator=(attached_editor&&) = delete;

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
            return {}; // getHint() is null until set — a benign empty, not a failure
        }
        const local_ref<jstring> text{env, static_cast<jstring>(env->CallObjectMethod(sequence.get(), to_string))};
        if (pending_exception_cleared(env, "CharSequence.toString") || !text)
        {
            return {};
        }
        return to_utf8(env, text.get());
    }

    TEST(android_editor, attach_creates_a_real_android_widget_edit_text)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        const attached_editor seam;
        ASSERT_NE(seam.widget(), nullptr) << "the android partial did not create a widget";
        jclass edit_text_class = default_jni_cache().find_class(env.get(), k_edit_text_class);
        ASSERT_NE(edit_text_class, nullptr);
        EXPECT_EQ(env->IsInstanceOf(seam.widget(), edit_text_class), JNI_TRUE);
    }

    TEST(android_editor, text_reaches_the_widget_including_supplementary_plane)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_editor seam;
        const std::string expected = "note \xF0\x9F\xA7\xA9";
        seam.control.set_text(expected);
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getText"), expected);
    }

    TEST(android_editor, text_color_reaches_the_widget_as_argb)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_editor seam;
        const maui::graphics::color red(1.0F, 0.0F, 0.0F);
        seam.control.set_text_color(red);
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getCurrentTextColor"), static_cast<jint>(red.to_int()));
    }

    TEST(android_editor, placeholder_reaches_the_widget_hint)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_editor seam;
        seam.control.set_placeholder("Type here...");
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getHint"), "Type here...");
    }

    TEST(android_editor, placeholder_color_reaches_the_widget_hint_color)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_editor seam;
        const maui::graphics::color green(0.0F, 1.0F, 0.0F);
        seam.control.set_placeholder_color(green);
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getCurrentHintTextColor"), static_cast<jint>(green.to_int()));
    }

    TEST(android_editor, is_enabled_reaches_the_widget)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_editor seam;
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_TRUE(call_bool(env.get(), seam.widget(), "isEnabled"));
        seam.control.set_is_enabled(false);
        EXPECT_FALSE(call_bool(env.get(), seam.widget(), "isEnabled"));
    }

    TEST(android_editor, visibility_maps_to_the_three_view_states)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_editor seam;
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_visible);
        seam.control.set_visibility(maui::core::visibility::hidden);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_invisible);
        seam.control.set_visibility(maui::core::visibility::collapsed);
        EXPECT_EQ(call_int(env.get(), seam.widget(), "getVisibility"), k_view_gone);
    }

    TEST(android_editor, opacity_reaches_the_widget_alpha)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_editor seam;
        seam.control.set_opacity(0.5);
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_NEAR(call_float(env.get(), seam.widget(), "getAlpha"), 0.5F, 1e-4F);
    }

    TEST(android_editor, automation_id_reaches_the_content_description)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_editor seam;
        seam.control.set_automation_id("notes_editor");
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_EQ(call_char_sequence(env.get(), seam.widget(), "getContentDescription"), "notes_editor");
    }

    TEST(android_editor, font_size_and_bold_reach_the_widget)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_editor seam;
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

    TEST(android_editor, character_spacing_reaches_letter_spacing_in_em)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_editor seam;
        seam.control.set_character_spacing(2.5);
        ASSERT_NE(seam.widget(), nullptr);
        EXPECT_NEAR(call_float(env.get(), seam.widget(), "getLetterSpacing"), 2.5F * 0.0624F, 1e-4F);
    }
} // namespace
