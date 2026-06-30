// Android image seam tests (M-android fan-out) — the iOS Rosetta Stone replayed over JNI, ON the
// emulator inside the app_process widget test host (tools/android-testhost-run.sh): the REAL
// maui::controls::image drives the REAL android.widget.ImageView through the cross-platform
// image_handler's android partial (src/platform/android/image_handler.cpp), and the assertions read
// the widget state BACK through JNI getters (getScaleType / getAdjustViewBounds) or observe the headless
// mirror on the platform struct (source_kind / source_file / source_loaded / opaque / animation_playing)
// for the deviations the partial documents (the source decode + generic-IView pushes are deferred).
// Characterization target: ImageHandler.Android.cs + ImageViewExtensions.cs + AspectExtensions.cs (the
// documented plain-widget deviations are in the partial's header).

#include <memory>
#include <string>

#include <gtest/gtest.h>
#include <jni.h>

#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "maui/controls/file_image_source.hpp"
#include "maui/controls/image.hpp"
#include "maui/core/aspect.hpp"
#include "maui/core/image_handler.hpp"
#include "maui/graphics/size.hpp"
#include "testhost/test_host.hpp"

namespace
{
    using maui::controls::image;
    using maui::controls::image_source;
    using maui::core::image_handler;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;

    constexpr const char* k_image_view_class = "android/widget/ImageView";
    constexpr const char* k_scale_type_class = "android/widget/ImageView$ScaleType";

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

    // The control + attached handler + the real widget, torn down in declaration order (the handler detach
    // precedes the control's death; the platform struct releases the widget's global ref).
    struct attached_image
    {
        image control;
        std::shared_ptr<image_handler> handler = std::make_shared<image_handler>();

        attached_image()
        {
            control.set_handler(handler);
        }

        ~attached_image()
        {
            control.set_handler(nullptr);
        }

        attached_image(const attached_image&) = delete;
        attached_image(attached_image&&) = delete;
        attached_image& operator=(const attached_image&) = delete;
        attached_image& operator=(attached_image&&) = delete;

        [[nodiscard]] jobject widget() const
        {
            auto* platform = handler->typed_platform_view();
            return platform != nullptr ? static_cast<jobject>(platform->native) : nullptr;
        }
    };

    // The ImageView.ScaleType static constant named `field_name` (the AspectExtensions targets).
    [[nodiscard]] local_ref<jobject> scale_type_constant(JNIEnv* env, const char* field_name)
    {
        jclass scale_type_class = default_jni_cache().find_class(env, k_scale_type_class);
        if (scale_type_class == nullptr)
        {
            ADD_FAILURE() << "ImageView$ScaleType not found";
            return {};
        }
        const jfieldID field =
            env->GetStaticFieldID(scale_type_class, field_name, "Landroid/widget/ImageView$ScaleType;");
        if (pending_exception_cleared(env, "GetStaticFieldID") || field == nullptr)
        {
            ADD_FAILURE() << "ScaleType." << field_name << " not found";
            return {};
        }
        local_ref<jobject> value{env, env->GetStaticObjectField(scale_type_class, field)};
        pending_exception_cleared(env, "GetStaticObjectField");
        return value;
    }

    // ImageView.getScaleType() (the current scale type the partial pushed).
    [[nodiscard]] local_ref<jobject> current_scale_type(JNIEnv* env, jobject widget)
    {
        jmethodID get_scale_type = default_jni_cache().method(env, k_image_view_class, "getScaleType",
                                                              "()Landroid/widget/ImageView$ScaleType;");
        if (get_scale_type == nullptr)
        {
            ADD_FAILURE() << "getScaleType not found";
            return {};
        }
        local_ref<jobject> value{env, env->CallObjectMethod(widget, get_scale_type)};
        pending_exception_cleared(env, "getScaleType");
        return value;
    }

    // ImageView.getAdjustViewBounds() (API 16+) — the bounds-adjustment flag.
    [[nodiscard]] bool current_adjust_view_bounds(JNIEnv* env, jobject widget)
    {
        jmethodID get_adjust = default_jni_cache().method(env, k_image_view_class, "getAdjustViewBounds", "()Z");
        if (get_adjust == nullptr)
        {
            ADD_FAILURE() << "getAdjustViewBounds not found";
            return false;
        }
        const jboolean value = env->CallBooleanMethod(widget, get_adjust);
        return !pending_exception_cleared(env, "getAdjustViewBounds") && value == JNI_TRUE;
    }

    // ---- virtual → native ----

    TEST(android_image, attach_creates_a_real_android_widget_image_view)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        const attached_image seam;
        ASSERT_NE(seam.widget(), nullptr) << "the android partial did not create a widget";
        jclass image_view_class = default_jni_cache().find_class(env.get(), k_image_view_class);
        ASSERT_NE(image_view_class, nullptr);
        EXPECT_EQ(env->IsInstanceOf(seam.widget(), image_view_class), JNI_TRUE);
    }

    TEST(android_image, aspect_maps_to_the_image_view_scale_type)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_image seam;
        ASSERT_NE(seam.widget(), nullptr);

        // AspectExtensions.ToScaleType: each aspect → its ImageView.ScaleType constant.
        struct expectation
        {
            maui::core::aspect aspect;
            const char* scale_type_field;
        };
        const expectation cases[] = {
            {maui::core::aspect::aspect_fit, "FIT_CENTER"},
            {maui::core::aspect::aspect_fill, "CENTER_CROP"},
            {maui::core::aspect::fill, "FIT_XY"},
            {maui::core::aspect::center, "CENTER"},
        };
        for (const auto& test_case : cases)
        {
            seam.control.set_aspect(test_case.aspect);
            const local_ref<jobject> expected = scale_type_constant(env.get(), test_case.scale_type_field);
            const local_ref<jobject> actual = current_scale_type(env.get(), seam.widget());
            ASSERT_TRUE(expected);
            ASSERT_TRUE(actual);
            // ScaleType constants are singletons, so identity is the right comparison.
            EXPECT_EQ(env->IsSameObject(actual.get(), expected.get()), JNI_TRUE)
                << "ScaleType mismatch for ScaleType." << test_case.scale_type_field;
        }
    }

    TEST(android_image, aspect_fill_disables_adjust_view_bounds)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_image seam;
        ASSERT_NE(seam.widget(), nullptr);
        // ImageViewExtensions.UpdateAspect: AspectFill turns adjust-view-bounds OFF, every other aspect ON.
        seam.control.set_aspect(maui::core::aspect::aspect_fill);
        EXPECT_FALSE(current_adjust_view_bounds(env.get(), seam.widget()));
        seam.control.set_aspect(maui::core::aspect::aspect_fit);
        EXPECT_TRUE(current_adjust_view_bounds(env.get(), seam.widget()));
    }

    TEST(android_image, measure_returns_a_non_negative_size)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_image seam;
        ASSERT_NE(seam.widget(), nullptr);
        // No decoded bitmap this cut (the source decode is deferred — partial header), so the ImageView
        // measures to its padding/wrap-content: a real, finite, non-negative measurement.
        const maui::graphics::size measured = seam.handler->get_desired_size(1000, 1000);
        EXPECT_GE(measured.width, 0);
        EXPECT_GE(measured.height, 0);
    }

    // ---- headless-style mirrors (the documented deviations: source decode + opaque/animation deferred) ----

    TEST(android_image, file_source_updates_the_load_mirror)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_image seam;
        // A FILE source loads synchronously through the cross-platform map_source → the android
        // load_file_source_sync primitive, which records the mirror the VM-less suite observes (the real
        // setImageDrawable / Glide decode is deferred — partial header).
        seam.control.set_source(image_source::from_file("dotnet_bot.png"));
        auto* platform = seam.handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_EQ(platform->source_kind, "file");
        EXPECT_EQ(platform->source_file, "dotnet_bot.png");
        EXPECT_TRUE(platform->source_loaded);

        // Clearing the source (a null/empty source) clears the mirror (SetImageSource(null)).
        seam.control.set_source(nullptr);
        EXPECT_FALSE(platform->source_loaded);
        EXPECT_TRUE(platform->source_kind.empty());
    }

    TEST(android_image, is_opaque_and_is_animation_playing_reach_the_mirror)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_image seam;
        auto* platform = seam.handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        // Both are headless mirrors on android (partial header): IsOpaque has no plain-ImageView analog and
        // UpdateIsAnimationPlaying awaits the deferred drawable decode, so the flags are observed on the mirror.
        seam.control.set_is_opaque(true);
        EXPECT_TRUE(platform->opaque);
        seam.control.set_is_animation_playing(true);
        EXPECT_TRUE(platform->animation_playing);
    }
} // namespace
