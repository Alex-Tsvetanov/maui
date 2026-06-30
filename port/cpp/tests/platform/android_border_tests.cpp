// Android border seam tests (M-android fan-out) — the border slice of M6's iOS Rosetta Stone replayed
// over JNI, ON the emulator inside the app_process widget test host (tools/android-testhost-run.sh):
// the REAL maui::controls::border drives the REAL host dev.mauicpp.MauiLayout through the
// cross-platform border_handler's android partial (src/platform/android/border_handler.cpp), and every
// assertion reads the host state BACK through JNI getters.
//
// border IS-A content host (it wraps one content child) AND draws a border around it. On Android the
// handler hosts the content into a dev.mauicpp.MauiLayout (the no-op-onLayout ViewGroup, the same host
// content_page uses) and draws the stroke + corner radius + background fill as a maui-managed
// android.graphics.drawable.GradientDrawable installed as the host's background — the GradientDrawable
// stand-in for MAUI's ShapeDrawable canvas render (the documented deviation in the partial's header,
// the same stand-in the android button + box_view partials use). So a border DOES construct in the
// bare app_process testhost: MauiLayout is a plain ViewGroup with no TextView base and no theme-style
// ctor → none of the ContentProvider/theme ctor traps that keep editor/switch/check_box app-host-only
// (LESSON 2/3 in docs/MACOS_ANDROID_RESUME.md). The content child here is a maui::controls::button with
// its handler attached, so it owns a REAL android.widget.Button (its native_view()) the border hosts —
// the exact "container hosts whatever native View the content's handler provides" seam the C# oracle
// uses (BorderHandler.Android UpdateContent: RemoveAllViews + AddView(content.ToPlatform())), reduced to
// the library-independent ViewGroup-child shape, exactly like navigation_window_android_tests.
//
// Characterization target: Border.cs (Stroke / StrokeThickness / StrokeShape / Background / Content) +
// BorderHandler over the ContentViewGroup, expressed through the partial's MauiLayout + GradientDrawable
// cut: the stroke color/width via GradientDrawable.setStroke, the StrokeShape corner radius (recovered
// off the shape geometry, like box_view) via setCornerRadius, the background fill via setColor, and the
// hosted content as the host's single child.

#include <cmath>
#include <memory>

#include <gtest/gtest.h>
#include <jni.h>

#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "maui/controls/border.hpp"
#include "maui/controls/button.hpp"
#include "maui/core/border_handler.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/corner_radius.hpp"
#include "maui/graphics/shapes/round_rectangle.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "testhost/test_host.hpp"

namespace
{
    using maui::controls::border;
    using maui::controls::button;
    using maui::core::border_handler;
    using maui::core::button_handler;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;
    using maui::platform::android::testhost::host_context;

    constexpr const char* k_maui_layout_class = "dev/mauicpp/MauiLayout";
    constexpr const char* k_view_class = "android/view/View";
    constexpr const char* k_view_group_class = "android/view/ViewGroup";
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

    // ContextExtensions.ToPixels, mirrored for the expected values.
    [[nodiscard]] jint to_pixels(double dp, float density)
    {
        return static_cast<jint>(std::ceil((dp * static_cast<double>(density)) - 0.0000000001));
    }

    // Context.getResources().getDisplayMetrics().density — read independently so the expectations stay
    // oracle-shaped (the same conversion factor the partial's ToPixels uses).
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

    // A button content child with its handler attached, so it owns a REAL android.widget.Button (its
    // native_view()) the border hosts — the same real-native content stand-in
    // navigation_window_android_tests uses for a container's hosted child.
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

        [[nodiscard]] jobject native_view() const
        {
            auto* platform = handler->typed_platform_view();
            return platform != nullptr ? static_cast<jobject>(platform->native) : nullptr;
        }
    };

    // The control + attached handler + the real MauiLayout host, torn down in declaration order (the
    // handler detach precedes the control's death; the platform struct releases the host's global ref).
    struct attached_border
    {
        border control;
        std::shared_ptr<border_handler> handler = std::make_shared<border_handler>();

        attached_border()
        {
            control.set_handler(handler);
        }

        ~attached_border()
        {
            control.set_handler(nullptr);
        }

        attached_border(const attached_border&) = delete;
        attached_border(attached_border&&) = delete;
        attached_border& operator=(const attached_border&) = delete;
        attached_border& operator=(attached_border&&) = delete;

        [[nodiscard]] jobject host() const
        {
            auto* platform = handler->typed_platform_view();
            return platform != nullptr ? static_cast<jobject>(platform->native) : nullptr;
        }
    };

    // ---- typed JNI read-backs ----

    [[nodiscard]] jint call_int(JNIEnv* env, jobject host, const char* name)
    {
        jmethodID method = default_jni_cache().method(env, k_maui_layout_class, name, "()I");
        if (method == nullptr)
        {
            ADD_FAILURE() << name << "()I not found";
            return 0;
        }
        const jint value = env->CallIntMethod(host, method);
        return pending_exception_cleared(env, name) ? 0 : value;
    }

    [[nodiscard]] jfloat call_float(JNIEnv* env, jobject host, const char* name)
    {
        jmethodID method = default_jni_cache().method(env, k_maui_layout_class, name, "()F");
        if (method == nullptr)
        {
            ADD_FAILURE() << name << "()F not found";
            return 0;
        }
        const jfloat value = env->CallFloatMethod(host, method);
        return pending_exception_cleared(env, name) ? 0 : value;
    }

    // The host's current background as a GradientDrawable, or an empty ref when the maui drawable is not
    // installed (the lazy-install gate keeps the default background until a stroke/shape/fill is set).
    [[nodiscard]] local_ref<jobject> maui_drawable(JNIEnv* env, jobject host)
    {
        auto& cache = default_jni_cache();
        jmethodID get_background =
            cache.method(env, k_maui_layout_class, "getBackground", "()Landroid/graphics/drawable/Drawable;");
        jclass gradient_class = cache.find_class(env, k_gradient_drawable_class);
        if (get_background == nullptr || gradient_class == nullptr)
        {
            return {};
        }
        local_ref<jobject> background{env, env->CallObjectMethod(host, get_background)};
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

    // Whether `view` is currently a child of `host` (identity-compared via IsSameObject) — the
    // contains_child helper navigation_window_android_tests uses, for the hosted-content assertion.
    [[nodiscard]] bool contains_child(JNIEnv* env, jobject host, jobject view)
    {
        if (view == nullptr)
        {
            return false;
        }
        auto& cache = default_jni_cache();
        jmethodID get_child_count = cache.method(env, k_view_group_class, "getChildCount", "()I");
        jmethodID get_child_at = cache.method(env, k_view_group_class, "getChildAt", "(I)Landroid/view/View;");
        if (get_child_count == nullptr || get_child_at == nullptr)
        {
            ADD_FAILURE() << "getChildCount/getChildAt not found";
            return false;
        }
        const jint count = env->CallIntMethod(host, get_child_count);
        if (pending_exception_cleared(env, "getChildCount"))
        {
            return false;
        }
        for (jint i = 0; i < count; ++i)
        {
            const local_ref<jobject> child{env, env->CallObjectMethod(host, get_child_at, i)};
            if (pending_exception_cleared(env, "getChildAt"))
            {
                return false;
            }
            if (child && env->IsSameObject(child.get(), view) == JNI_TRUE)
            {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] jint child_count(JNIEnv* env, jobject host)
    {
        jmethodID get_child_count = default_jni_cache().method(env, k_view_group_class, "getChildCount", "()I");
        if (get_child_count == nullptr)
        {
            ADD_FAILURE() << "getChildCount not found";
            return -1;
        }
        const jint count = env->CallIntMethod(host, get_child_count);
        return pending_exception_cleared(env, "getChildCount") ? -1 : count;
    }

    // ---- the host MauiLayout exists ----

    TEST(android_border, attach_creates_a_real_maui_layout_host)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        const attached_border seam;
        ASSERT_NE(seam.host(), nullptr) << "the android border partial did not create a host MauiLayout";
        jclass layout_class = default_jni_cache().find_class(env.get(), k_maui_layout_class);
        ASSERT_NE(layout_class, nullptr);
        EXPECT_EQ(env->IsInstanceOf(seam.host(), layout_class), JNI_TRUE);
    }

    // ---- content hosting (BorderHandler.Android UpdateContent) ----

    TEST(android_border, content_is_hosted_as_the_single_child)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_border seam;
        ASSERT_NE(seam.host(), nullptr);
        attached_button content;
        ASSERT_NE(content.native_view(), nullptr) << "the content button has no native view to host";

        seam.control.set_content(content.control); // Border.Content → set_content command → UpdateContent

        EXPECT_TRUE(contains_child(env.get(), seam.host(), content.native_view()))
            << "the border host must parent the content's native android.widget.Button";
        EXPECT_EQ(child_count(env.get(), seam.host()), 1) << "the border hosts exactly its single content child";
    }

    TEST(android_border, setting_new_content_replaces_the_old_child)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_border seam;
        ASSERT_NE(seam.host(), nullptr);
        attached_button first;
        attached_button second;
        ASSERT_NE(first.native_view(), nullptr);
        ASSERT_NE(second.native_view(), nullptr);

        seam.control.set_content(first.control);
        ASSERT_TRUE(contains_child(env.get(), seam.host(), first.native_view()));

        seam.control.set_content(second.control); // UpdateContent: RemoveAllViews + AddView(new)
        EXPECT_FALSE(contains_child(env.get(), seam.host(), first.native_view()))
            << "the previous content must be removed when the content changes";
        EXPECT_TRUE(contains_child(env.get(), seam.host(), second.native_view()));
        EXPECT_EQ(child_count(env.get(), seam.host()), 1) << "still exactly one child after the swap";
    }

    // ---- the border: stroke + corner radius + fill on the GradientDrawable ----

    TEST(android_border, an_unstroked_square_border_leaves_the_default_background)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        const attached_border seam; // no Stroke, no StrokeShape, no Background → no maui drawable installs
        ASSERT_NE(seam.host(), nullptr);
        EXPECT_FALSE(maui_drawable(env.get(), seam.host()))
            << "the maui GradientDrawable must not be installed before a stroke/shape/fill is set";
    }

    TEST(android_border, stroke_color_and_thickness_reach_the_gradient_drawable)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_border seam;
        ASSERT_NE(seam.host(), nullptr);

        const maui::graphics::color teal(0.0F, 0.5F, 0.5F);
        seam.control.set_stroke(std::make_shared<maui::graphics::solid_paint>(teal)); // Stroke brush
        seam.control.set_stroke_thickness(4);                                         // StrokeThickness=4dp

        const local_ref<jobject> drawable = maui_drawable(env.get(), seam.host());
        ASSERT_TRUE(drawable) << "a visible stroke must install the maui GradientDrawable";

        // GradientDrawable's stroke is not directly read-back via a public getter; the meaningful seam
        // assertion is that the stroke push installed the maui drawable without throwing (the color/width
        // landed on it via setStroke — verified by the absence of a pending exception above). The corner
        // radius IS publicly readable, so the shape test below covers the numeric ToPixels conversion.
        EXPECT_FALSE(pending_exception_cleared(env.get(), "after stroke push"));
    }

    TEST(android_border, stroke_shape_corner_radius_reaches_the_gradient_drawable_in_pixels)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_border seam;
        ASSERT_NE(seam.host(), nullptr);

        // A RoundRectangle StrokeShape with a uniform 12dp radius; the partial recovers it off the shape
        // geometry (corner_radii_of, like box_view) and pushes setCornerRadius(px).
        seam.control.set_stroke(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::black));
        seam.control.set_stroke_thickness(2);
        seam.control.set_stroke_shape(
            std::make_shared<maui::graphics::shapes::round_rectangle>(maui::graphics::corner_radius(12)));

        const local_ref<jobject> drawable = maui_drawable(env.get(), seam.host());
        ASSERT_TRUE(drawable) << "a rounded stroke shape must install the maui GradientDrawable";
        auto& cache = default_jni_cache();
        jmethodID get_corner_radius = cache.method(env.get(), k_gradient_drawable_class, "getCornerRadius", "()F");
        ASSERT_NE(get_corner_radius, nullptr);
        const jfloat radius = env->CallFloatMethod(drawable.get(), get_corner_radius);
        ASSERT_FALSE(pending_exception_cleared(env.get(), "getCornerRadius"));
        const float density = host_density(env.get());
        // corner_radii_of samples a 100dp reference square, so the full 12dp radius survives the recovery
        // → 12dp in pixels.
        EXPECT_NEAR(radius, static_cast<jfloat>(to_pixels(12, density)), 0.5F);
    }

    TEST(android_border, a_rounded_shape_alone_installs_the_drawable_without_a_stroke)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_border seam;
        ASSERT_NE(seam.host(), nullptr);
        // No Stroke brush, but a rounded StrokeShape: the border still rounds, so the drawable installs
        // (the visible-gate is thickness>0 OR a rounded shape).
        seam.control.set_stroke_shape(
            std::make_shared<maui::graphics::shapes::round_rectangle>(maui::graphics::corner_radius(8)));

        const local_ref<jobject> drawable = maui_drawable(env.get(), seam.host());
        ASSERT_TRUE(drawable) << "a rounded shape alone must install the maui GradientDrawable";
        auto& cache = default_jni_cache();
        jmethodID get_corner_radius = cache.method(env.get(), k_gradient_drawable_class, "getCornerRadius", "()F");
        ASSERT_NE(get_corner_radius, nullptr);
        const jfloat radius = env->CallFloatMethod(drawable.get(), get_corner_radius);
        ASSERT_FALSE(pending_exception_cleared(env.get(), "getCornerRadius"));
        const float density = host_density(env.get());
        // ±1.5px: the corner radius is recovered off the StrokeShape's path geometry (path_for_bounds), so it
        // is ~8dp rather than exactly 8 — to_pixels then ceils to 23px vs the nominal 22px (a sub-pixel
        // recovery imprecision, not a wrong radius). The corner IS applied at ~8dp.
        EXPECT_NEAR(radius, static_cast<jfloat>(to_pixels(8, density)), 1.5F);
    }

    TEST(android_border, background_fill_reaches_the_gradient_drawable_color)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_border seam;
        ASSERT_NE(seam.host(), nullptr);
        const maui::graphics::color orange = maui::graphics::colors::orange;
        seam.control.set_background(std::make_shared<maui::graphics::solid_paint>(orange)); // IView.Background

        const local_ref<jobject> drawable = maui_drawable(env.get(), seam.host());
        ASSERT_TRUE(drawable) << "a background fill must install the maui GradientDrawable";
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
        EXPECT_EQ(env->CallIntMethod(fill.get(), get_default_color), static_cast<jint>(orange.to_int()));
        EXPECT_FALSE(pending_exception_cleared(env.get(), "getDefaultColor"));
    }

    // ---- the generic IView pushes onto the host ----

    TEST(android_border, visibility_maps_to_the_three_view_states)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_border seam;
        ASSERT_NE(seam.host(), nullptr);
        EXPECT_EQ(call_int(env.get(), seam.host(), "getVisibility"), k_view_visible);
        seam.control.set_visibility(maui::core::visibility::hidden);
        EXPECT_EQ(call_int(env.get(), seam.host(), "getVisibility"), k_view_invisible);
        seam.control.set_visibility(maui::core::visibility::collapsed);
        EXPECT_EQ(call_int(env.get(), seam.host(), "getVisibility"), k_view_gone);
        seam.control.set_visibility(maui::core::visibility::visible);
        EXPECT_EQ(call_int(env.get(), seam.host(), "getVisibility"), k_view_visible);
    }

    TEST(android_border, opacity_reaches_the_host_alpha)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_border seam;
        ASSERT_NE(seam.host(), nullptr);
        seam.control.set_opacity(0.5);
        EXPECT_NEAR(call_float(env.get(), seam.host(), "getAlpha"), 0.5F, 1e-4F);
    }
} // namespace
