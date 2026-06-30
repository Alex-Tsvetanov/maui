// Android box_view seam tests (M-android fan-out) — the box_view slice of M6's iOS Rosetta Stone
// replayed over JNI, ON the emulator inside the app_process widget test host
// (tools/android-testhost-run.sh): the REAL maui::controls::box_view drives the REAL host
// android.view.View through the cross-platform shape_view_handler's android partial
// (src/platform/android/box_view_handler.cpp), and every assertion reads the View state BACK through
// JNI getters.
//
// box_view is NOT its own handler: BoxView is its own IShapeView AND its own IShape, rendered by the
// shared shape_view_handler (box_view.cpp self-registers MAUI_REGISTER_HANDLER(box_view,
// shape_view_handler)). On Android that handler hosts a plain android.view.View whose background is a
// maui-managed android.graphics.drawable.GradientDrawable — the GradientDrawable stand-in for MAUI's
// MauiBoxView : PlatformGraphicsView canvas render (the documented deviation in the partial's header,
// the same stand-in the android button partial + android_visual_ops use). So a box_view DOES construct
// in the bare app_process testhost (a plain View has no TextView base and no theme-style ctor → none of
// the ContentProvider/theme ctor traps that keep editor/switch/check_box app-host-only — LESSON 2/3 in
// docs/MACOS_ANDROID_RESUME.md). The box fill color + corner radius are asserted via getBackground()'s
// GradientDrawable, exactly like android_button_tests' drawable test.
//
// Characterization target: BoxView.cs (Color → Fill, CornerRadius) + ShapeViewHandler over the
// MauiBoxView PlatformGraphicsView, expressed through the partial's plain-View+GradientDrawable cut.

#include <cmath>
#include <memory>

#include <gtest/gtest.h>
#include <jni.h>

#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "maui/controls/box_view.hpp"
#include "maui/core/shape_view_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/corner_radius.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "testhost/test_host.hpp"

namespace
{
    using maui::controls::box_view;
    using maui::core::shape_view_handler;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;
    using maui::platform::android::testhost::host_context;

    constexpr const char* k_view_class = "android/view/View";
    constexpr const char* k_gradient_drawable_class = "android/graphics/drawable/GradientDrawable";

    // android.view.View visibility states (ViewExtensions.ToPlatformVisibility's targets).
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

    // The control + attached handler + the real View, torn down in declaration order (the handler
    // detach precedes the control's death; the platform struct releases the View's global ref).
    struct attached_box
    {
        box_view control;
        std::shared_ptr<shape_view_handler> handler = std::make_shared<shape_view_handler>();

        attached_box()
        {
            control.set_handler(handler);
        }

        ~attached_box()
        {
            control.set_handler(nullptr);
        }

        attached_box(const attached_box&) = delete;
        attached_box(attached_box&&) = delete;
        attached_box& operator=(const attached_box&) = delete;
        attached_box& operator=(attached_box&&) = delete;

        [[nodiscard]] jobject view() const
        {
            auto* platform = handler->typed_platform_view();
            return platform != nullptr ? static_cast<jobject>(platform->native) : nullptr;
        }
    };

    // ---- typed JNI read-backs ----

    [[nodiscard]] jint call_int(JNIEnv* env, jobject view, const char* name)
    {
        jmethodID method = default_jni_cache().method(env, k_view_class, name, "()I");
        if (method == nullptr)
        {
            ADD_FAILURE() << name << "()I not found";
            return 0;
        }
        const jint value = env->CallIntMethod(view, method);
        return pending_exception_cleared(env, name) ? 0 : value;
    }

    [[nodiscard]] jfloat call_float(JNIEnv* env, jobject view, const char* name)
    {
        jmethodID method = default_jni_cache().method(env, k_view_class, name, "()F");
        if (method == nullptr)
        {
            ADD_FAILURE() << name << "()F not found";
            return 0;
        }
        const jfloat value = env->CallFloatMethod(view, method);
        return pending_exception_cleared(env, name) ? 0 : value;
    }

    // Context.getResources().getDisplayMetrics().density — read independently here so the expectations
    // stay oracle-shaped (the same conversion factor the partial's ToPixels uses).
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

    // The View's current background as a GradientDrawable, or an empty ref when the maui drawable is not
    // installed (the lazy-install gate keeps the default background until a Fill is set).
    [[nodiscard]] local_ref<jobject> maui_drawable(JNIEnv* env, jobject view)
    {
        auto& cache = default_jni_cache();
        jmethodID get_background =
            cache.method(env, k_view_class, "getBackground", "()Landroid/graphics/drawable/Drawable;");
        jclass gradient_class = cache.find_class(env, k_gradient_drawable_class);
        if (get_background == nullptr || gradient_class == nullptr)
        {
            return {};
        }
        local_ref<jobject> background{env, env->CallObjectMethod(view, get_background)};
        if (pending_exception_cleared(env, "getBackground") || !background)
        {
            return {};
        }
        if (env->IsInstanceOf(background.get(), gradient_class) == JNI_FALSE)
        {
            return {};
        }
        return background;
    }

    // ---- virtual → native ----

    TEST(android_box_view, attach_creates_a_real_android_view)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        const attached_box seam;
        ASSERT_NE(seam.view(), nullptr) << "the android shape partial did not create a host view";
        jclass view_class = default_jni_cache().find_class(env.get(), k_view_class);
        ASSERT_NE(view_class, nullptr);
        EXPECT_EQ(env->IsInstanceOf(seam.view(), view_class), JNI_TRUE);
    }

    TEST(android_box_view, an_unset_color_leaves_the_default_background)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        const attached_box seam; // never sets Color: Fill is null, so no maui GradientDrawable installs
        ASSERT_NE(seam.view(), nullptr);
        EXPECT_FALSE(maui_drawable(env.get(), seam.view()))
            << "the maui GradientDrawable must not be installed before BoxView.Color is set";
    }

    TEST(android_box_view, color_reaches_the_gradient_drawable_fill)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_box seam;
        ASSERT_NE(seam.view(), nullptr);
        const maui::graphics::color purple(0.5F, 0.0F, 0.5F);
        seam.control.set_color(purple); // BoxView.Color → Fill (the shape-fill path)

        const local_ref<jobject> drawable = maui_drawable(env.get(), seam.view());
        ASSERT_TRUE(drawable) << "setting BoxView.Color must install the maui GradientDrawable";

        // GradientDrawable.getColor() (API 24+) returns the fill as a ColorStateList.
        auto& cache = default_jni_cache();
        jmethodID get_color =
            cache.method(env.get(), k_gradient_drawable_class, "getColor", "()Landroid/content/res/ColorStateList;");
        jmethodID get_default_color =
            cache.method(env.get(), "android/content/res/ColorStateList", "getDefaultColor", "()I");
        ASSERT_NE(get_color, nullptr);
        ASSERT_NE(get_default_color, nullptr);
        const local_ref<jobject> fill{env.get(), env->CallObjectMethod(drawable.get(), get_color)};
        ASSERT_FALSE(pending_exception_cleared(env.get(), "getColor"));
        ASSERT_TRUE(fill);
        EXPECT_EQ(env->CallIntMethod(fill.get(), get_default_color), static_cast<jint>(purple.to_int()));
        EXPECT_FALSE(pending_exception_cleared(env.get(), "getDefaultColor"));
    }

    TEST(android_box_view, uniform_corner_radius_reaches_the_gradient_drawable_in_pixels)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_box seam;
        ASSERT_NE(seam.view(), nullptr);
        // A Fill is required for the drawable to install; then the uniform CornerRadius=10 pushes.
        seam.control.set_color(maui::graphics::colors::light_green);
        seam.control.set_corner_radius(maui::graphics::corner_radius(10));

        const local_ref<jobject> drawable = maui_drawable(env.get(), seam.view());
        ASSERT_TRUE(drawable);
        auto& cache = default_jni_cache();
        jmethodID get_corner_radius = cache.method(env.get(), k_gradient_drawable_class, "getCornerRadius", "()F");
        ASSERT_NE(get_corner_radius, nullptr);
        const jfloat radius = env->CallFloatMethod(drawable.get(), get_corner_radius);
        ASSERT_FALSE(pending_exception_cleared(env.get(), "getCornerRadius"));
        const float density = host_density(env.get());
        // BoxView.OnMeasure clamps the radius to the bounds, but corner_radii_of samples a 100dp
        // reference square, so the full 10dp radius survives the recovery → 10dp in pixels.
        EXPECT_NEAR(radius, static_cast<jfloat>(to_pixels(10, density)), 0.5F);
    }

    TEST(android_box_view, color_change_re_pushes_the_fill)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_box seam;
        ASSERT_NE(seam.view(), nullptr);
        seam.control.set_color(maui::graphics::colors::orange);
        const maui::graphics::color blue(0.0F, 0.0F, 1.0F);
        seam.control.set_color(blue); // a second set must re-push, not stick on the first color

        const local_ref<jobject> drawable = maui_drawable(env.get(), seam.view());
        ASSERT_TRUE(drawable);
        auto& cache = default_jni_cache();
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
    }

    TEST(android_box_view, visibility_maps_to_the_three_view_states)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_box seam;
        ASSERT_NE(seam.view(), nullptr);
        EXPECT_EQ(call_int(env.get(), seam.view(), "getVisibility"), k_view_visible);
        seam.control.set_visibility(maui::core::visibility::hidden);
        EXPECT_EQ(call_int(env.get(), seam.view(), "getVisibility"), k_view_invisible);
        seam.control.set_visibility(maui::core::visibility::collapsed);
        EXPECT_EQ(call_int(env.get(), seam.view(), "getVisibility"), k_view_gone);
        seam.control.set_visibility(maui::core::visibility::visible);
        EXPECT_EQ(call_int(env.get(), seam.view(), "getVisibility"), k_view_visible);
    }

    TEST(android_box_view, opacity_reaches_the_view_alpha)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_box seam;
        ASSERT_NE(seam.view(), nullptr);
        seam.control.set_opacity(0.5);
        EXPECT_NEAR(call_float(env.get(), seam.view(), "getAlpha"), 0.5F, 1e-4F);
    }
} // namespace
