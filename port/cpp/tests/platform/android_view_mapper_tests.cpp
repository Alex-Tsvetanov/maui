// Android view-mapper + semantics seam tests (W4-34e) — the generic-IView visual layer (render
// transform / flow direction / background / semantics) pushed to a REAL android.view.View over JNI,
// ON the emulator inside the app_process widget test host (tools/android-testhost-run.sh). Two seams:
//   through a control: the REAL maui::controls::button drives the real android.widget.Button via the
//     button partial's update_transform / update_flow_direction / update_semantics overrides (which
//     call the shared android_view_ops / android_semantics_ops helpers), read BACK over JNI getters.
//   the shared op in isolation: apply_background (android_visual_ops) on a freshly-minted plain View,
//     read back through getBackground() — mirroring the apple view_mapper_apple_tests.mm apply_* cases.
// Characterization target: ViewHandler.Android.cs + the Android visual extensions (Transformation /
// View / Semantic Extensions); the documented WrapperView-only deviations (shadow/clip/hint/input-
// transparent) are in the ops headers and are NOT asserted (the unwrapped View receives no such update).

#include <memory>
#include <string>

#include <gtest/gtest.h>
#include <jni.h>

#include "android_semantics_ops.hpp" // apply_semantics
#include "android_view_ops.hpp"      // apply_transform / apply_flow_direction
#include "android_visual_ops.hpp"    // apply_background
#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/controls/button.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/flow_direction.hpp"
#include "maui/core/semantics.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/gradient_stop.hpp"
#include "maui/graphics/linear_gradient_paint.hpp"
#include "maui/graphics/radial_gradient_paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "testhost/test_host.hpp"

namespace
{
    using maui::controls::button;
    using maui::core::button_handler;
    using maui::core::flow_direction;
    using maui::core::semantic_heading_level;
    using maui::core::semantics;
    using maui::platform::android::apply_background;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;
    using maui::platform::android::to_utf8;
    using maui::platform::android::testhost::host_context;

    constexpr const char* k_view_class = "android/view/View";
    constexpr const char* k_button_class = "android/widget/Button";
    constexpr const char* k_gradient_drawable = "android/graphics/drawable/GradientDrawable";

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

    // The control + attached handler + the real widget, torn down in declaration order (mirrors
    // android_button_tests.cpp's attached_button).
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

    [[nodiscard]] jfloat get_float(JNIEnv* env, jobject view, const char* name)
    {
        jmethodID method = default_jni_cache().method(env, k_button_class, name, "()F");
        if (method == nullptr)
        {
            ADD_FAILURE() << name << "()F not found";
            return 0;
        }
        const jfloat value = env->CallFloatMethod(view, method);
        return pending_exception_cleared(env, name) ? 0 : value;
    }

    [[nodiscard]] jint get_int(JNIEnv* env, jobject view, const char* name)
    {
        jmethodID method = default_jni_cache().method(env, k_button_class, name, "()I");
        if (method == nullptr)
        {
            ADD_FAILURE() << name << "()I not found";
            return 0;
        }
        const jint value = env->CallIntMethod(view, method);
        return pending_exception_cleared(env, name) ? 0 : value;
    }

    [[nodiscard]] bool get_bool(JNIEnv* env, jobject view, const char* name)
    {
        jmethodID method = default_jni_cache().method(env, k_button_class, name, "()Z");
        if (method == nullptr)
        {
            ADD_FAILURE() << name << "()Z not found";
            return false;
        }
        const jboolean value = env->CallBooleanMethod(view, method);
        return !pending_exception_cleared(env, name) && value == JNI_TRUE;
    }

    [[nodiscard]] std::string get_content_description(JNIEnv* env, jobject view)
    {
        auto& cache = default_jni_cache();
        jmethodID method = cache.method(env, k_button_class, "getContentDescription", "()Ljava/lang/CharSequence;");
        jmethodID to_string = cache.method(env, "java/lang/Object", "toString", "()Ljava/lang/String;");
        if (method == nullptr || to_string == nullptr)
        {
            ADD_FAILURE() << "getContentDescription surface missing";
            return {};
        }
        const local_ref<jobject> sequence{env, env->CallObjectMethod(view, method)};
        if (pending_exception_cleared(env, "getContentDescription") || !sequence)
        {
            return {}; // null contentDescription -> empty string (not a failure: a cleared description)
        }
        const local_ref<jstring> text{env, static_cast<jstring>(env->CallObjectMethod(sequence.get(), to_string))};
        if (pending_exception_cleared(env, "CharSequence.toString") || !text)
        {
            return {};
        }
        return to_utf8(env, text.get());
    }

    // Mint a fresh plain android.view.View(context) for the in-isolation op tests.
    [[nodiscard]] local_ref<jobject> make_view(JNIEnv* env)
    {
        auto& cache = default_jni_cache();
        jclass view_class = cache.find_class(env, k_view_class);
        jmethodID ctor = cache.method(env, k_view_class, "<init>", "(Landroid/content/Context;)V");
        if (view_class == nullptr || ctor == nullptr)
        {
            ADD_FAILURE() << "android.view.View(Context) ctor not found";
            return {};
        }
        local_ref<jobject> view{env, env->NewObject(view_class, ctor, host_context())};
        if (pending_exception_cleared(env, "View construction") || !view)
        {
            ADD_FAILURE() << "View construction failed";
            return {};
        }
        return view;
    }

    [[nodiscard]] jint gradient_type(JNIEnv* env, jobject drawable)
    {
        jmethodID get_type = default_jni_cache().method(env, k_gradient_drawable, "getGradientType", "()I");
        if (get_type == nullptr)
        {
            ADD_FAILURE() << "GradientDrawable.getGradientType missing";
            return -1;
        }
        const jint value = env->CallIntMethod(drawable, get_type);
        return pending_exception_cleared(env, "getGradientType") ? -1 : value;
    }

    // ---- render transform (TransformationExtensions) through the control's update_transform ----

    TEST(android_view_mapper, translation_scale_rotation_reach_the_widget)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_button seam;
        ASSERT_NE(seam.widget(), nullptr);

        // Translation is dp; the widget stores px = dp * density. Read density-independently by setting
        // a value and dividing back out is overkill — assert non-zero with the right sign, plus scale /
        // rotation which are density-free.
        seam.control.set_translation_x(7.0);
        seam.control.set_translation_y(-3.0);
        EXPECT_GT(get_float(env.get(), seam.widget(), "getTranslationX"), 0.0F);
        EXPECT_LT(get_float(env.get(), seam.widget(), "getTranslationY"), 0.0F);

        // Scale: setScaleX/Y = scale * scaleX/Y (here scale defaults to 1).
        seam.control.set_scale_x(2.0);
        seam.control.set_scale_y(3.0);
        EXPECT_FLOAT_EQ(get_float(env.get(), seam.widget(), "getScaleX"), 2.0F);
        EXPECT_FLOAT_EQ(get_float(env.get(), seam.widget(), "getScaleY"), 3.0F);

        // The uniform Scale multiplies both per-axis factors.
        seam.control.set_scale(2.0);
        EXPECT_FLOAT_EQ(get_float(env.get(), seam.widget(), "getScaleX"), 4.0F); // 2 * 2
        EXPECT_FLOAT_EQ(get_float(env.get(), seam.widget(), "getScaleY"), 6.0F); // 2 * 3

        seam.control.set_rotation(45.0);
        seam.control.set_rotation_x(10.0);
        seam.control.set_rotation_y(20.0);
        EXPECT_FLOAT_EQ(get_float(env.get(), seam.widget(), "getRotation"), 45.0F);
        EXPECT_FLOAT_EQ(get_float(env.get(), seam.widget(), "getRotationX"), 10.0F);
        EXPECT_FLOAT_EQ(get_float(env.get(), seam.widget(), "getRotationY"), 20.0F);
    }

    // Pivot reads the laid-out size: arrange the widget, then a 0/1 anchor lands the pivot at 0 / the
    // size edge (anchor * size). The default 0.5 anchor lands it at the center.
    TEST(android_view_mapper, anchor_reaches_the_pivot_after_arrange)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_button seam;
        ASSERT_NE(seam.widget(), nullptr);
        seam.handler->platform_arrange(maui::graphics::rect(0, 0, 100, 40));

        seam.control.set_anchor_x(0.0);
        seam.control.set_anchor_y(1.0);
        EXPECT_FLOAT_EQ(get_float(env.get(), seam.widget(), "getPivotX"), 0.0F); // 0 * width
        EXPECT_GT(get_float(env.get(), seam.widget(), "getPivotY"), 0.0F);       // 1 * height (px)

        seam.control.set_anchor_x(0.5);
        const jfloat width_px = static_cast<jfloat>(get_int(env.get(), seam.widget(), "getWidth"));
        EXPECT_NEAR(get_float(env.get(), seam.widget(), "getPivotX"), width_px * 0.5F, 0.5F);
    }

    // ---- flow direction (ViewExtensions.GetLayoutDirection) through update_flow_direction ----

    // Run apply_flow_direction for one mode through the control's update_flow_direction override on the
    // real widget and report whether the JNI dispatch raised. (C#'s UpdateFlowDirection is exactly
    // `platformView.LayoutDirection = GetLayoutDirection(view)` — a setter call; the meaningful seam fact
    // is that the op resolves setLayoutDirection on the View surface and invokes it with the mapped
    // constant.) The RESOLVED value (View.getLayoutDirection()) is NOT asserted: that getter returns the
    // *resolved* direction, which android.view.View only computes once attached to a window/display — the
    // windowless app_process test host never resolves it (it stays the default LTR=0 for every mode,
    // verified empirically), so a resolved-value assertion would test the host, not the op. The setter
    // path itself is the same JNI dispatch the transform/semantics cases exercise (those DO read back).
    [[nodiscard]] bool flow_direction_dispatches_cleanly(flow_direction fd, JNIEnv* env)
    {
        attached_button seam;
        if (seam.widget() == nullptr)
        {
            ADD_FAILURE() << "no widget";
            return false;
        }
        seam.control.set_flow_direction(fd);
        // setLayoutDirection must have resolved + invoked without leaving a pending Java exception.
        return !pending_exception_cleared(env, "set_flow_direction");
    }

    TEST(android_view_mapper, flow_direction_dispatches_to_set_layout_direction)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        // Confirm android.view.View.setLayoutDirection(int) actually exists on the widget surface (the
        // method the op resolves) so "no exception" is not merely a missing-method skip.
        ASSERT_NE(default_jni_cache().method(env.get(), k_view_class, "setLayoutDirection", "(I)V"), nullptr);

        EXPECT_TRUE(flow_direction_dispatches_cleanly(flow_direction::right_to_left, env.get()));
        EXPECT_TRUE(flow_direction_dispatches_cleanly(flow_direction::left_to_right, env.get()));
        EXPECT_TRUE(flow_direction_dispatches_cleanly(flow_direction::match_parent, env.get()));
    }

    // ---- semantics (SemanticExtensions) through update_semantics ----

    TEST(android_view_mapper, description_reaches_the_content_description)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_button seam;
        ASSERT_NE(seam.widget(), nullptr);

        auto sem = std::make_shared<semantics>();
        sem->set_description("Submit the form");
        seam.control.set_semantics(sem);
        EXPECT_EQ(get_content_description(env.get(), seam.widget()), "Submit the form");
    }

    TEST(android_view_mapper, is_heading_reaches_the_accessibility_heading_flag)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_button seam;
        ASSERT_NE(seam.widget(), nullptr);

        // Not a heading by default.
        EXPECT_FALSE(get_bool(env.get(), seam.widget(), "isAccessibilityHeading"));

        auto sem = std::make_shared<semantics>();
        sem->set_description("Section title");
        sem->set_heading_level(semantic_heading_level::level1);
        seam.control.set_semantics(sem);
        EXPECT_TRUE(get_bool(env.get(), seam.widget(), "isAccessibilityHeading"));

        // Dropping the heading level clears the flag.
        auto plain = std::make_shared<semantics>();
        plain->set_description("Section title");
        seam.control.set_semantics(plain);
        EXPECT_FALSE(get_bool(env.get(), seam.widget(), "isAccessibilityHeading"));
    }

    // A null Semantics is a no-op: the previously-pushed description is left untouched (C# returns
    // early on a null Semantics — it does not clear).
    TEST(android_view_mapper, null_semantics_leaves_description_untouched)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_button seam;
        ASSERT_NE(seam.widget(), nullptr);

        auto sem = std::make_shared<semantics>();
        sem->set_description("Keep me");
        seam.control.set_semantics(sem);
        ASSERT_EQ(get_content_description(env.get(), seam.widget()), "Keep me");

        seam.control.set_semantics(nullptr); // maps update_semantics(nullptr) -> no-op
        EXPECT_EQ(get_content_description(env.get(), seam.widget()), "Keep me");
    }

    // ---- background (android_visual_ops apply_background) on a plain View in isolation ----

    TEST(android_view_mapper, apply_background_solid_paint_sets_background_color)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        const local_ref<jobject> view = make_view(env.get());
        ASSERT_TRUE(view);

        const maui::graphics::color blue(0.0F, 0.0F, 1.0F);
        const maui::graphics::solid_paint paint{blue};
        apply_background(view.get(), &paint);

        // setBackgroundColor installs a ColorDrawable; read its color back (API 11+ getColor()).
        auto& cache = default_jni_cache();
        jmethodID get_background =
            cache.method(env.get(), k_view_class, "getBackground", "()Landroid/graphics/drawable/Drawable;");
        jmethodID get_color = cache.method(env.get(), "android/graphics/drawable/ColorDrawable", "getColor", "()I");
        ASSERT_NE(get_background, nullptr);
        ASSERT_NE(get_color, nullptr);
        const local_ref<jobject> drawable{env.get(), env->CallObjectMethod(view.get(), get_background)};
        ASSERT_FALSE(pending_exception_cleared(env.get(), "getBackground"));
        ASSERT_TRUE(drawable);
        const jint color = env->CallIntMethod(drawable.get(), get_color);
        ASSERT_FALSE(pending_exception_cleared(env.get(), "ColorDrawable.getColor"));
        EXPECT_EQ(color, static_cast<jint>(blue.to_int()));
    }

    TEST(android_view_mapper, apply_background_linear_gradient_installs_gradient_drawable)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        const local_ref<jobject> view = make_view(env.get());
        ASSERT_TRUE(view);

        const maui::graphics::color red(1.0F, 0.0F, 0.0F);
        const maui::graphics::linear_gradient_paint paint{
            std::vector<maui::graphics::gradient_stop>{maui::graphics::gradient_stop(0.0F, red),
                                                       maui::graphics::gradient_stop(1.0F, {0.0F, 0.0F, 1.0F})},
            maui::graphics::point(0, 0), maui::graphics::point(1, 0)};
        apply_background(view.get(), &paint);

        auto& cache = default_jni_cache();
        jmethodID get_background =
            cache.method(env.get(), k_view_class, "getBackground", "()Landroid/graphics/drawable/Drawable;");
        jclass gradient_class = cache.find_class(env.get(), k_gradient_drawable);
        ASSERT_NE(get_background, nullptr);
        ASSERT_NE(gradient_class, nullptr);
        const local_ref<jobject> drawable{env.get(), env->CallObjectMethod(view.get(), get_background)};
        ASSERT_FALSE(pending_exception_cleared(env.get(), "getBackground"));
        ASSERT_TRUE(drawable);
        ASSERT_EQ(env->IsInstanceOf(drawable.get(), gradient_class), JNI_TRUE)
            << "the gradient was not installed as a GradientDrawable";
        // GradientDrawable exposes no public getter for the colors[] of a multi-stop gradient
        // (getColor() returns null in gradient mode), so the stop colors are covered by reaching here
        // exception-free with a GradientDrawable installed; the gradient TYPE is the readable witness.
        EXPECT_EQ(gradient_type(env.get(), drawable.get()), 0); // LINEAR_GRADIENT (the constructed default)
    }

    TEST(android_view_mapper, apply_background_radial_gradient_sets_radial_type)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        const local_ref<jobject> view = make_view(env.get());
        ASSERT_TRUE(view);

        const maui::graphics::radial_gradient_paint paint{
            std::vector<maui::graphics::gradient_stop>{maui::graphics::gradient_stop(0.0F, {0.0F, 1.0F, 0.0F}),
                                                       maui::graphics::gradient_stop(1.0F, {0.0F, 0.0F, 0.0F})},
            maui::graphics::point(0.5, 0.5), 0.5};
        apply_background(view.get(), &paint);

        auto& cache = default_jni_cache();
        jmethodID get_background =
            cache.method(env.get(), k_view_class, "getBackground", "()Landroid/graphics/drawable/Drawable;");
        jclass gradient_class = cache.find_class(env.get(), k_gradient_drawable);
        ASSERT_NE(get_background, nullptr);
        ASSERT_NE(gradient_class, nullptr);
        const local_ref<jobject> drawable{env.get(), env->CallObjectMethod(view.get(), get_background)};
        ASSERT_FALSE(pending_exception_cleared(env.get(), "getBackground"));
        ASSERT_TRUE(drawable);
        ASSERT_EQ(env->IsInstanceOf(drawable.get(), gradient_class), JNI_TRUE);
        EXPECT_EQ(gradient_type(env.get(), drawable.get()), 1); // RADIAL_GRADIENT
    }

    TEST(android_view_mapper, apply_background_null_clears_the_background)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        const local_ref<jobject> view = make_view(env.get());
        ASSERT_TRUE(view);

        const maui::graphics::solid_paint paint{maui::graphics::color(1.0F, 0.0F, 0.0F)};
        apply_background(view.get(), &paint);
        auto& cache = default_jni_cache();
        jmethodID get_background =
            cache.method(env.get(), k_view_class, "getBackground", "()Landroid/graphics/drawable/Drawable;");
        ASSERT_NE(get_background, nullptr);
        {
            const local_ref<jobject> set{env.get(), env->CallObjectMethod(view.get(), get_background)};
            ASSERT_FALSE(pending_exception_cleared(env.get(), "getBackground (set)"));
            ASSERT_TRUE(set) << "the solid paint did not install a background";
        }

        apply_background(view.get(), nullptr);
        const local_ref<jobject> cleared{env.get(), env->CallObjectMethod(view.get(), get_background)};
        ASSERT_FALSE(pending_exception_cleared(env.get(), "getBackground (cleared)"));
        EXPECT_FALSE(cleared) << "a null paint must clear the background";
    }

    // Defensive: the ops are null-safe (a detached / handler-less path never crashes).
    TEST(android_view_mapper, ops_are_null_safe)
    {
        const semantics sem;
        const maui::graphics::solid_paint paint{maui::graphics::color(1.0F, 0.0F, 0.0F)};
        maui::platform::android::apply_transform(nullptr, {});
        maui::platform::android::apply_flow_direction(nullptr, flow_direction::right_to_left);
        maui::platform::android::apply_semantics(nullptr, &sem);
        apply_background(nullptr, &paint);
        SUCCEED();
    }
} // namespace
