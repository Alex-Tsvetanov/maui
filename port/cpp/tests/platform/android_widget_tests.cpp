// Android widget SMOKE tests (M-android milestone 3) — the passing demonstration that gates the
// android per-control fan-out. These run INSIDE the app_process test host (src/platform/android/
// testhost/, tools/android-testhost-run.sh): a real ART runtime where testhost::host_context()
// is a usable themed android.content.Context, so the cases construct REAL android.widget views
// from C++ through the JNI seam headers (jni_env / jni_ref / jni_cache / jni_string), push text
// into them (the mapper-shaped operation: TextView.setText) and read it back (TextView.getText) —
// proving the exact create-native-view + mapper-push + read-back loop the android handlers need.
// No APK, no gradle, no installation. Characterization target: the views the C# Android handlers
// create (ButtonHandler.Android.cs -> MaterialButton-less AppCompat-less stock android.widget).

#include <optional>
#include <string>
#include <string_view>

#include <gtest/gtest.h>
#include <jni.h>

#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "testhost/test_host.hpp"

namespace
{
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;
    using maui::platform::android::to_jstring;
    using maui::platform::android::to_utf8;
    using maui::platform::android::testhost::host_context;

    // Describes (to System.err, which app_process ties to the shell) and clears any pending Java
    // exception; true when one was pending — call sites turn that into a test failure.
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

    // Drives the create + mapper-push + read-back loop on one widget class: new <class>(context),
    // setText(text), getText().toString() -> UTF-8. Empty optional (plus a recorded failure) when
    // any JNI stage fails.
    std::optional<std::string> text_round_trip(JNIEnv* env, const char* class_name, std::string_view text)
    {
        auto& cache = default_jni_cache();

        jclass widget_class = cache.find_class(env, class_name);
        jmethodID ctor = cache.method(env, class_name, "<init>", "(Landroid/content/Context;)V");
        if (widget_class == nullptr || ctor == nullptr)
        {
            ADD_FAILURE() << class_name << ": class or (Context) constructor not found";
            return std::nullopt;
        }
        const local_ref<jobject> widget{env, env->NewObject(widget_class, ctor, host_context())};
        if (pending_exception_cleared(env, "widget construction") || !widget)
        {
            ADD_FAILURE() << class_name << ": construction failed";
            return std::nullopt;
        }

        const local_ref<jstring> jtext = to_jstring(env, text);
        jmethodID set_text = cache.method(env, class_name, "setText", "(Ljava/lang/CharSequence;)V");
        if (!jtext || set_text == nullptr)
        {
            ADD_FAILURE() << class_name << ": setText surface missing";
            return std::nullopt;
        }
        env->CallVoidMethod(widget.get(), set_text, jtext.get());
        if (pending_exception_cleared(env, "setText"))
        {
            return std::nullopt;
        }

        jmethodID get_text = cache.method(env, class_name, "getText", "()Ljava/lang/CharSequence;");
        jmethodID to_string = cache.method(env, "java/lang/Object", "toString", "()Ljava/lang/String;");
        if (get_text == nullptr || to_string == nullptr)
        {
            ADD_FAILURE() << class_name << ": getText/toString surface missing";
            return std::nullopt;
        }
        const local_ref<jobject> char_sequence{env, env->CallObjectMethod(widget.get(), get_text)};
        if (pending_exception_cleared(env, "getText") || !char_sequence)
        {
            ADD_FAILURE() << class_name << ": getText failed";
            return std::nullopt;
        }
        const local_ref<jstring> round_tripped{
            env, static_cast<jstring>(env->CallObjectMethod(char_sequence.get(), to_string))};
        if (pending_exception_cleared(env, "CharSequence.toString") || !round_tripped)
        {
            ADD_FAILURE() << class_name << ": toString failed";
            return std::nullopt;
        }
        return to_utf8(env, round_tripped.get());
    }

    TEST(android_widget, host_provides_a_context)
    {
        ASSERT_NE(host_context(), nullptr) << "not running inside the app_process test host";
        const scoped_env env;
        ASSERT_TRUE(env) << "no JNIEnv for the test thread (JavaVM not pinned?)";
    }

    TEST(android_widget, textview_text_round_trips)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        const auto actual = text_round_trip(env.get(), "android/widget/TextView", "hello maui");
        ASSERT_TRUE(actual.has_value());
        EXPECT_EQ(*actual, "hello maui");
    }

    TEST(android_widget, button_text_round_trips_including_supplementary_plane)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        // Button is the fan-out's first real control (unit: ButtonHandler), and its constructor
        // additionally exercises the theme lookup (R.attr.buttonStyle) through the bootstrap's
        // ContextThemeWrapper. The text carries U+1F9E9 (🧩) to pin the real-UTF-8 jstring path —
        // the modified-UTF-8 trap NewStringUTF would mangle.
        const std::string expected = "hello maui \xF0\x9F\xA7\xA9";
        const auto actual = text_round_trip(env.get(), "android/widget/Button", expected);
        ASSERT_TRUE(actual.has_value());
        EXPECT_EQ(*actual, expected);
    }
} // namespace
