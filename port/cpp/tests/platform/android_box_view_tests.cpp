// Android box_view / shape seam tests (M-android fan-out, WAVE 7) — the box_view slice of M6's iOS
// Rosetta Stone replayed over JNI, ON the emulator inside the app_process widget test host
// (tools/android-testhost-run.sh): the REAL maui::controls::box_view drives the REAL host
// dev.mauicpp.MauiShapeView through the cross-platform shape_view_handler's android partial
// (src/platform/android/box_view_handler.cpp).
//
// box_view is NOT its own handler: BoxView is its own IShapeView AND its own IShape, rendered by the
// shared shape_view_handler (box_view.cpp self-registers MAUI_REGISTER_HANDLER(box_view,
// shape_view_handler)). WAVE 7 retired the GradientDrawable shortcut the first cut used and now hosts a
// dev.mauicpp.MauiShapeView whose onDraw replays the shared shape_drawable through android_canvas (an
// i_canvas over android.graphics.Canvas) — the faithful PlatformGraphicsView render. The View is a plain
// View(Context) (no TextView base, no theme-style ctor), so it STILL constructs in the bare app_process
// testhost (LESSON 2/3 in docs/MACOS_ANDROID_RESUME.md), unlike editor/switch/check_box.
//
// What this characterizes here: the host is a MauiShapeView (the drawing seat), the shared shape_drawable
// mirror is wired to the virtual view (the headless mirror that stays live), the box's Fill round-trips
// through the control, the redraw seam survives invalidate (onDraw → native_draw is exercised by the app
// host + the parity capture, which a headless Canvas-less unit test cannot synthesize — the box/ellipse/
// shape gallery captures are the render proof). visibility/opacity push to the real View unchanged.
//
// The actual pixel render (fill/stroke/path) is verified TWO ways the unit seam cannot: (1) the headless
// shape golden-op tests replay the SAME shape_drawable into a recording_canvas; (2) the emulator parity
// captures (docs/comparison/android/cpp/{ellipse,rectangle,line,polygon,...}_gallery.png) show the real
// geometry. Headless-green ≠ emulator-correct, so the captures are the load-bearing proof.

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
#include "maui/graphics/paint.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "testhost/test_host.hpp"

namespace
{
    using maui::controls::box_view;
    using maui::core::shape_view_handler;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;

    constexpr const char* k_view_class = "android/view/View";
    constexpr const char* k_shape_view_class = "dev/mauicpp/MauiShapeView";

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

    // The control + attached handler + the real View, torn down in declaration order (the handler detach
    // precedes the control's death; the platform struct clears the View's nativePtr then releases it).
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

    // ---- virtual → native ----

    TEST(android_box_view, attach_creates_a_real_maui_shape_view)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        const attached_box seam;
        ASSERT_NE(seam.view(), nullptr) << "the android shape partial did not create a host view";
        // The host is the custom drawing View (MauiShapeView : View), not a plain View — onDraw replays
        // the shape drawable through android_canvas.
        jclass shape_view_class = default_jni_cache().find_class(env.get(), k_shape_view_class);
        ASSERT_NE(shape_view_class, nullptr) << "dev.mauicpp.MauiShapeView must be on the dex path";
        EXPECT_EQ(env->IsInstanceOf(seam.view(), shape_view_class), JNI_TRUE)
            << "the shape host must be a MauiShapeView (the PlatformGraphicsView twin)";
        // It IS-A android.view.View, so the generic-IView pushes (visibility/opacity/...) still resolve.
        jclass view_class = default_jni_cache().find_class(env.get(), k_view_class);
        ASSERT_NE(view_class, nullptr);
        EXPECT_EQ(env->IsInstanceOf(seam.view(), view_class), JNI_TRUE);
    }

    TEST(android_box_view, the_drawable_mirror_wires_to_the_virtual_view)
    {
        const attached_box seam;
        auto* platform = seam.handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        // update_shape (run on attach) points the shared shape_drawable at the box_view — the headless
        // mirror the canvas render replays. The drawable's shape_view() is the i_shape_view face of the
        // control, so the SAME drawable draws on android, apple, ios and into the headless recording canvas.
        EXPECT_EQ(platform->drawable.shape_view(), static_cast<const maui::core::i_shape_view*>(&seam.control));
    }

    TEST(android_box_view, color_round_trips_through_the_box_fill)
    {
        attached_box seam;
        const maui::graphics::color purple(0.5F, 0.0F, 0.5F);
        seam.control.set_color(purple); // BoxView.Color → Fill (the shape-fill path the drawable renders)
        // The Fill the drawable paints is the box's own solid paint; the canvas render reads it on every
        // onDraw. (A real fill pixel is asserted by the parity captures + the headless golden-op tests.)
        const auto* fill = dynamic_cast<const maui::graphics::solid_paint*>(seam.control.fill());
        ASSERT_NE(fill, nullptr) << "setting BoxView.Color must produce a solid Fill paint";
        EXPECT_EQ(fill->color().to_int(), purple.to_int());
    }

    TEST(android_box_view, a_color_change_re_invalidates_without_throwing)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_box seam;
        ASSERT_NE(seam.view(), nullptr);
        // Each set re-pushes (invalidate_shape → MauiShapeView.invalidate()); the seam must survive the
        // redraw request (the View schedules onDraw; the pending-exception check guards the JNI path).
        seam.control.set_color(maui::graphics::colors::orange);
        seam.control.set_color(maui::graphics::color(0.0F, 0.0F, 1.0F));
        EXPECT_FALSE(pending_exception_cleared(env.get(), "invalidate after color change"));
        // The latest Fill is the one the next onDraw will paint.
        const auto* fill = dynamic_cast<const maui::graphics::solid_paint*>(seam.control.fill());
        ASSERT_NE(fill, nullptr);
        EXPECT_EQ(fill->color().to_int(), maui::graphics::color(0.0F, 0.0F, 1.0F).to_int());
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
