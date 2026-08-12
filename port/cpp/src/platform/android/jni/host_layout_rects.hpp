#pragma once
// maui::platform::android — the FULL window rect and the SAFE-AREA rect the Android apphosts hand to
// maui::hosting::drive_layout. Internal seam infrastructure for the Android backend, NOT a ported MAUI
// type: it is the android analog of what src/platform/ios/host_run.mm reads off UIKit
// (`view.bounds` + `view.safeAreaInsets`), which is the pair the generic two-rect drive_layout wants.
//
// WHY TWO RECTS AND NOT ONE INSET CANVAS
//
// MAUI on net10.0-android runs EDGE-TO-EDGE and applies the safe area PER VIEW, not once at the host:
// SafeAreaExtensions.ApplyAdjustedSafeAreaInsetsPx (src/Core/src/Platform/Android/SafeAreaExtensions.cs)
// walks down from the window and pads the first view whose ISafeAreaView2 region for that edge is not
// None, passing the insets THROUGH any view that declines them. So a root whose region is Container (any
// Layout, ScrollView, a page with SafeAreaEdges="Container") insets its children, while a root whose
// region is None (ContentPage's own default-value creator, and `Border` — Border.cs) consumes nothing and
// is laid out over the WHOLE window.
//
// The apphosts used to collapse that into ONE number: display_size() returned the window height MINUS the
// system bars, the Activity fitted its content view below the status bar, and everything was implicitly
// inset. That is indistinguishable from MAUI for a Container root (both land the content at the status-bar
// height) and WRONG for a None root, which MAUI centers on the window mid-line and the port centered on
// the mid-line of the shrunken canvas. Measured on the android board's `border` page (a bare centered
// <Border>, the only None root in the 172-page gallery): MAUI's red stroke spans rows 952..1387 —
// center 1169.5 = (0 + 2339) / 2 — while the port's spanned 986..1423, center 1204.5 = (136 + 2273) / 2.
// A uniform 35 px = ((136 + 66) / 2) - 66 ... i.e. exactly half the asymmetry between the two bars.
//
// So the host now reports both rects and lets each view decide, exactly as the ios lane does. The Activity
// goes edge-to-edge (Window.setDecorFitsSystemWindows(false)) so the content view really does span the
// window, and `full` is the whole window while `safe` is that minus the system-bar insets.
//
// SHARED because both apphosts need the identical answer: src/platform/android/apphost/app_host.cpp (the
// C++-builder column) and examples/gallery_xaml/apphost/app_host.cpp (the C++&XAML column) are captured
// back to back into the same parity board, so any divergence here would read as a port bug in one column.
// They previously carried near-duplicate copies of this JNI walk that had already drifted — the fallback
// chrome height summed status_bar_height + navigation_bar_height in one and only status_bar_height in the
// other.
//
// UNITS: the framework lays out in density-independent POINTS and the android handlers to_pixels at the
// seam, so every value here is divided by DisplayMetrics.density before it is returned. Both rects go
// through the SAME density read — an inset in pixels against bounds in points would be silently wrong.

#include <jni.h>

#include <array>

#include "app_context.hpp"
#include "jni_ref.hpp"
#include "maui/graphics/rect.hpp"

namespace maui::platform::android
{
    // The pair maui::hosting::drive_layout takes. `safe` is contained in `full`; when the platform cannot
    // report insets the two are EQUAL, which makes every safe-area path downstream a no-op (the same
    // degradation the headless and AppKit hosts rely on).
    struct host_layout_rects
    {
        maui::graphics::rect full;
        maui::graphics::rect safe;
    };

    namespace detail
    {
        // A portrait phone viewport — the ios/headless gallery default (host_run.cpp), used when the JNI
        // walk cannot complete at all, so the mount still settles rather than laying out at 0x0.
        inline constexpr double k_fallback_width = 402.0;
        inline constexpr double k_fallback_height = 874.0;

        inline void clear_pending(JNIEnv* env) noexcept
        {
            if (env != nullptr && env->ExceptionCheck() == JNI_TRUE)
            {
                env->ExceptionClear();
            }
        }

        // MauiHostActivity.windowMetricsPx() -> int[6] {width, height, left, top, right, bottom}, all in
        // PIXELS: getCurrentWindowMetrics().getBounds() plus its systemBars() insets (API 30+, and valid at
        // mount time — no view-attachment dependency, unlike reading insets off the decor view). Returns
        // false (leaving `out` untouched) on an older API or any failure, so the caller falls back to the
        // legacy DisplayMetrics + dimen-chrome path.
        [[nodiscard]] inline bool read_window_metrics_px(JNIEnv* env, jobject activity, std::array<jint, 6>& out)
        {
            if (env == nullptr || activity == nullptr)
            {
                return false;
            }
            const local_ref<jclass> activity_class{env, env->GetObjectClass(activity)};
            if (!activity_class)
            {
                clear_pending(env);
                return false;
            }
            const jmethodID mid = env->GetMethodID(activity_class.get(), "windowMetricsPx", "()[I");
            if (mid == nullptr)
            {
                clear_pending(env);
                return false;
            }
            const local_ref<jintArray> values{env, static_cast<jintArray>(env->CallObjectMethod(activity, mid))};
            if (env->ExceptionCheck() == JNI_TRUE || !values)
            {
                clear_pending(env);
                return false;
            }
            if (env->GetArrayLength(values.get()) != 6)
            {
                clear_pending(env);
                return false;
            }
            env->GetIntArrayRegion(values.get(), 0, 6, out.data());
            if (env->ExceptionCheck() == JNI_TRUE)
            {
                clear_pending(env);
                return false;
            }
            // A zero/negative window is nonsense; insets may legitimately be 0 but never negative.
            if (out[0] <= 0 || out[1] <= 0 || out[2] < 0 || out[3] < 0 || out[4] < 0 || out[5] < 0)
            {
                return false;
            }
            return true;
        }

        // The system-chrome height in PIXELS the Activity's content view does NOT get when it is FITTED
        // (status bar above + navigation bar below; no action bar — MauiAppHost.Theme parents on the
        // NoActionBar framework theme, matching MAUI's native-default ContentPage render). Read from the
        // framework `status_bar_height` / `navigation_bar_height` dimens. LEGACY: only reachable below API
        // 30, where windowMetricsPx() is unavailable; there the Activity is not edge-to-edge either, so
        // subtracting the chrome from the canvas is still the right (single-rect) answer.
        [[nodiscard]] inline jint content_chrome_height_px(JNIEnv* env, jobject activity)
        {
            if (env == nullptr || activity == nullptr)
            {
                return 0;
            }
            jint total = 0;
            const local_ref<jclass> activity_class{env, env->GetObjectClass(activity)};
            if (!activity_class)
            {
                clear_pending(env);
                return 0;
            }
            const jmethodID get_resources =
                env->GetMethodID(activity_class.get(), "getResources", "()Landroid/content/res/Resources;");
            if (get_resources == nullptr)
            {
                clear_pending(env);
                return 0;
            }
            const local_ref<jobject> resources{env, env->CallObjectMethod(activity, get_resources)};
            if (env->ExceptionCheck() == JNI_TRUE || !resources)
            {
                clear_pending(env);
                return 0;
            }
            const local_ref<jclass> resources_class{env, env->GetObjectClass(resources.get())};
            const jmethodID get_identifier = env->GetMethodID(
                resources_class.get(), "getIdentifier", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I");
            const jmethodID get_dimension_pixel_size =
                env->GetMethodID(resources_class.get(), "getDimensionPixelSize", "(I)I");
            if (get_identifier == nullptr || get_dimension_pixel_size == nullptr)
            {
                clear_pending(env);
                return 0;
            }
            const local_ref<jstring> deftype{env, env->NewStringUTF("dimen")};
            const local_ref<jstring> defpkg{env, env->NewStringUTF("android")};
            for (const char* const dimen_name : {"status_bar_height", "navigation_bar_height"})
            {
                const local_ref<jstring> name{env, env->NewStringUTF(dimen_name)};
                const jint res_id =
                    env->CallIntMethod(resources.get(), get_identifier, name.get(), deftype.get(), defpkg.get());
                if (env->ExceptionCheck() == JNI_TRUE)
                {
                    clear_pending(env);
                    continue;
                }
                if (res_id <= 0)
                {
                    continue;
                }
                const jint bar_px = env->CallIntMethod(resources.get(), get_dimension_pixel_size, res_id);
                if (env->ExceptionCheck() == JNI_TRUE)
                {
                    clear_pending(env);
                    continue;
                }
                if (bar_px > 0)
                {
                    total += bar_px;
                }
            }
            return total;
        }

        // activity.getResources().getDisplayMetrics() -> {widthPixels, heightPixels, density}. `density` is
        // px per dp; a metrics object always carries one, but guard anyway and treat <= 0 as 1.0.
        [[nodiscard]] inline bool read_display_metrics(JNIEnv* env, jobject activity, jint& width_px, jint& height_px,
                                                       jfloat& density)
        {
            if (env == nullptr || activity == nullptr)
            {
                return false;
            }
            const local_ref<jclass> context_class{env, env->GetObjectClass(activity)};
            if (!context_class)
            {
                clear_pending(env);
                return false;
            }
            const jmethodID get_resources =
                env->GetMethodID(context_class.get(), "getResources", "()Landroid/content/res/Resources;");
            if (get_resources == nullptr || env->ExceptionCheck() == JNI_TRUE)
            {
                clear_pending(env);
                return false;
            }
            const local_ref<jobject> resources{env, env->CallObjectMethod(activity, get_resources)};
            if (env->ExceptionCheck() == JNI_TRUE || !resources)
            {
                clear_pending(env);
                return false;
            }
            const local_ref<jclass> resources_class{env, env->GetObjectClass(resources.get())};
            const jmethodID get_metrics =
                env->GetMethodID(resources_class.get(), "getDisplayMetrics", "()Landroid/util/DisplayMetrics;");
            if (get_metrics == nullptr || env->ExceptionCheck() == JNI_TRUE)
            {
                clear_pending(env);
                return false;
            }
            const local_ref<jobject> metrics{env, env->CallObjectMethod(resources.get(), get_metrics)};
            if (env->ExceptionCheck() == JNI_TRUE || !metrics)
            {
                clear_pending(env);
                return false;
            }
            const local_ref<jclass> metrics_class{env, env->GetObjectClass(metrics.get())};
            const jfieldID width_field = env->GetFieldID(metrics_class.get(), "widthPixels", "I");
            const jfieldID height_field = env->GetFieldID(metrics_class.get(), "heightPixels", "I");
            const jfieldID density_field = env->GetFieldID(metrics_class.get(), "density", "F");
            if (width_field == nullptr || height_field == nullptr)
            {
                clear_pending(env);
                return false;
            }
            width_px = env->GetIntField(metrics.get(), width_field);
            height_px = env->GetIntField(metrics.get(), height_field);
            density = density_field != nullptr ? env->GetFloatField(metrics.get(), density_field) : 1.0F;
            if (density <= 0.0F)
            {
                density = 1.0F;
            }
            return width_px > 0 && height_px > 0;
        }
    } // namespace detail

    // The full + safe rects in POINTS. The Activity is reached through app_context() (the apphost's JNI
    // export pinned it as the process context). Never throws and never leaves a pending JNI exception.
    [[nodiscard]] inline host_layout_rects layout_rects(JNIEnv* env)
    {
        const maui::graphics::rect fallback{0, 0, detail::k_fallback_width, detail::k_fallback_height};
        jobject activity = app_context();
        jint width_px = 0;
        jint height_px = 0;
        jfloat density = 1.0F;
        if (!detail::read_display_metrics(env, activity, width_px, height_px, density))
        {
            return {fallback, fallback};
        }
        const auto to_points = [density](jint pixels) { return static_cast<double>(pixels) / density; };

        std::array<jint, 6> metrics{};
        if (detail::read_window_metrics_px(env, activity, metrics))
        {
            const auto [w, h, left, top, right, bottom] = metrics;
            const maui::graphics::rect full{0, 0, to_points(w), to_points(h)};
            const maui::graphics::rect safe{to_points(left), to_points(top), to_points(w - left - right),
                                            to_points(h - top - bottom)};
            // A bar taller than the window is nonsense; fall back to no insets rather than an inverted rect.
            if (safe.width <= 0.0 || safe.height <= 0.0)
            {
                return {full, full};
            }
            return {full, safe};
        }

        // ponytail: legacy single-rect path (API < 30 only — windowMetricsPx is API 30+). There the Activity
        // still fits system windows, so the pre-inset canvas IS the content area and full == safe is right.
        // Upgrade path if a pre-30 device ever needs per-view safe area: read the insets off the decor view
        // after attachment and re-drive the layout, as the ios lane's tracker does.
        jint content_height_px = height_px;
        const jint chrome_px = detail::content_chrome_height_px(env, activity);
        if (chrome_px > 0 && chrome_px < height_px)
        {
            content_height_px -= chrome_px;
        }
        const maui::graphics::rect legacy{0, 0, to_points(width_px), to_points(content_height_px)};
        return {legacy, legacy};
    }
} // namespace maui::platform::android
