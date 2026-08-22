#pragma once
// maui::platform::android::apphost::viewport — the Android host's VIEWPORT + SAFE-AREA PRODUCER.
//
// Shared verbatim by BOTH real-Activity gallery hosts, which are otherwise independent APKs with their own
// JNI entry point and their own page dispatch: the C++ builder host (src/platform/android/apphost/
// app_host.cpp, dev.mauicpp.apphost) and the C++&XAML host (examples/gallery_xaml/apphost/app_host.cpp,
// dev.mauicpp.apphost.xaml). Both already have src/platform/android/ on their include path (they include
// "jni/app_context.hpp"), and both drive exactly the same window over exactly the same device, so this
// lives in ONE place: the two hosts previously carried a line-for-line copy of the JNI display walk, and
// the safe-area producer must not become a third and fourth copy of anything.
//
// What it answers: "what rectangle does this Activity lay out over, and which parts of it are covered by
// the system bars?" — plus the layout pass that routes the answer to the views that inset by it.

#include <jni.h>

#include <algorithm>
#include <array>

#include "jni/app_context.hpp"
#include "jni/jni_ref.hpp"

#include "maui/controls/window.hpp"
#include "maui/core/i_content_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/hosting/app_host.hpp"

namespace maui::platform::android::apphost
{
    // The Activity's viewport in framework POINTS (px / density): the FULL, edge-to-edge window size plus
    // the device SAFE AREA (status bar / navigation bar / display cutout) in the same units.
    struct viewport
    {
        double width;
        double height;
        maui::core::thickness safe_area;
    };

    // The system-chrome height (in PIXELS) the Activity's content view does NOT get: the status bar ABOVE
    // the content frame plus the system navigation bar BELOW it. (NO action/title bar — see below.)
    //
    // NO ACTION BAR (2026-07-01): MauiHostActivity now uses MauiAppHost.Theme, which parents on the
    // NoActionBar framework theme (res/values/styles.xml). Real .NET MAUI's Android gallery renders these
    // native-default ContentPages with NO top app-title bar, so the port previously painting one (the
    // "MAUI C++ Gallery" toolbar) was a parity DIFF; MAUI's render is the ground truth, so the bar is gone.
    // Consequently this function NO LONGER measures/subtracts the theme's actionBarSize — with NoActionBar
    // there is no title bar above setContentView's content frame, and the content starts directly below the
    // status bar (exactly where a native-default MAUI ContentPage's content starts). The action-bar
    // measurement block was removed; only the status bar (top) + navigation bar (bottom) remain.
    //
    // display_size lays the page out over the device display, so without this subtraction a page whose
    // bottom child is anchored to the content bottom (a Grid `*`-over-`Auto` row, or a FlexLayout column's
    // FOOTER after a Grow="1" body) is placed BELOW the visible content frame and never appears — the
    // FlexLayout footer bug this height reconciles. Both remaining heights are read from the framework's
    // `status_bar_height` / `navigation_bar_height` dimen resources (getIdentifier/getDimensionPixelSize).
    // Returns 0 on any failure (page still mounts, over the full display as before). Note:
    // `navigation_bar_height` is the classic (3-button) inset value; on a gesture-nav device the bottom
    // inset is smaller, so this may over-subtract a little there — the safe direction (the footer sits just
    // inside the content bottom, never off it).
    [[nodiscard]] inline jint content_chrome_height_px(JNIEnv* env, jobject activity)
    {
        if (env == nullptr || activity == nullptr)
        {
            return 0;
        }
        const auto clear = [&]() {
            if (env->ExceptionCheck() == JNI_TRUE)
            {
                env->ExceptionClear();
            }
        };
        jint total = 0;
        const maui::platform::android::local_ref<jclass> activity_class{env, env->GetObjectClass(activity)};
        if (!activity_class)
        {
            clear();
            return 0;
        }
        jmethodID get_resources =
            env->GetMethodID(activity_class.get(), "getResources", "()Landroid/content/res/Resources;");
        if (get_resources == nullptr)
        {
            clear();
            return 0;
        }
        const maui::platform::android::local_ref<jobject> resources{env,
                                                                    env->CallObjectMethod(activity, get_resources)};
        if (env->ExceptionCheck() == JNI_TRUE || !resources)
        {
            clear();
            return 0;
        }
        const maui::platform::android::local_ref<jclass> resources_class{env, env->GetObjectClass(resources.get())};

        // --- status bar (top) + navigation bar (bottom): for each, getDimensionPixelSize(getIdentifier(
        //     "<name>", "dimen", "android")). Both framework dimens read identically; sum whichever resolve. ---
        jmethodID get_identifier = env->GetMethodID(resources_class.get(), "getIdentifier",
                                                    "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I");
        jmethodID get_dimension_pixel_size = env->GetMethodID(resources_class.get(), "getDimensionPixelSize", "(I)I");
        if (get_identifier != nullptr && get_dimension_pixel_size != nullptr)
        {
            const maui::platform::android::local_ref<jstring> deftype{env, env->NewStringUTF("dimen")};
            const maui::platform::android::local_ref<jstring> defpkg{env, env->NewStringUTF("android")};
            for (const char* const dimen_name : {"status_bar_height", "navigation_bar_height"})
            {
                const maui::platform::android::local_ref<jstring> name{env, env->NewStringUTF(dimen_name)};
                const jint res_id =
                    env->CallIntMethod(resources.get(), get_identifier, name.get(), deftype.get(), defpkg.get());
                if (env->ExceptionCheck() == JNI_TRUE)
                {
                    clear();
                    continue;
                }
                if (res_id <= 0)
                {
                    continue;
                }
                const jint bar_px = env->CallIntMethod(resources.get(), get_dimension_pixel_size, res_id);
                if (env->ExceptionCheck() == JNI_TRUE)
                {
                    clear();
                    continue;
                }
                if (bar_px > 0)
                {
                    total += bar_px;
                }
            }
        }

        // NO action/title-bar measurement: MauiAppHost.Theme is NoActionBar (res/values/styles.xml), so
        // there is no title bar above the content frame to reserve height for. Removed 2026-07-01 to match
        // MAUI's Android gallery, which renders these native-default ContentPages with no top app-title bar.
        return total;
    }

    // MauiHostActivity.windowFramePx() via JNI — { width, height, insetLeft, insetTop, insetRight,
    // insetBottom } in PIXELS: the FULL edge-to-edge window (getCurrentWindowMetrics().getBounds(), API 30+)
    // and the device safe area over it. Writes `out` and returns true on success; returns false on older API
    // / any failure, so display_size falls back to the legacy DisplayMetrics - content_chrome_height_px path
    // (which yields a ZERO safe area, i.e. exactly the pre-edge-to-edge behavior).
    [[nodiscard]] inline bool window_frame_px(JNIEnv* env, jobject activity, std::array<jint, 6>& out)
    {
        if (env == nullptr || activity == nullptr)
        {
            return false;
        }
        const auto clear = [&]() {
            if (env->ExceptionCheck() == JNI_TRUE)
            {
                env->ExceptionClear();
            }
        };
        const maui::platform::android::local_ref<jclass> activity_class{env, env->GetObjectClass(activity)};
        if (!activity_class)
        {
            clear();
            return false;
        }
        const jmethodID mid = env->GetMethodID(activity_class.get(), "windowFramePx", "()[I");
        if (mid == nullptr)
        {
            clear();
            return false;
        }
        const maui::platform::android::local_ref<jobject> result{env, env->CallObjectMethod(activity, mid)};
        if (env->ExceptionCheck() == JNI_TRUE || !result)
        {
            clear();
            return false;
        }
        auto* const array = static_cast<jintArray>(result.get());
        if (env->GetArrayLength(array) != static_cast<jsize>(out.size()))
        {
            clear();
            return false;
        }
        env->GetIntArrayRegion(array, 0, static_cast<jsize>(out.size()), out.data());
        if (env->ExceptionCheck() == JNI_TRUE)
        {
            clear();
            return false;
        }
        return out[0] > 0 && out[1] > 0;
    }

    // The Activity's viewport in framework POINTS. The `density` (px per dp) always comes from
    // activity.getResources().getDisplayMetrics(); the RECTANGLE comes from windowFramePx() above — the FULL
    // edge-to-edge window plus the safe area over it — because MauiHostActivity turns decor fitting off
    // (MauiAppCompatActivity.cs:31) and MAUI lays out over the whole window, insetting per view.
    //
    // Legacy fallback (API < 30, where the decor still fits): DisplayMetrics.{widthPixels,heightPixels}
    // minus the system chrome the content view does not receive (status bar + navigation bar; NO action bar
    // — see content_chrome_height_px), with a ZERO safe area, i.e. the pre-edge-to-edge behavior unchanged.
    // Falls back further to a portrait phone viewport (the headless/ios default) when any step fails, so the
    // mount still settles. The Activity is reached through app_context() — the host's JNI entry pinned it as
    // the process context.
    inline viewport display_size(JNIEnv* env)
    {
        constexpr viewport fallback{402.0, 874.0, {}}; // the ios/headless gallery default (host_run.cpp)
        jobject activity = maui::platform::android::app_context();
        if (env == nullptr || activity == nullptr)
        {
            return fallback;
        }
        // activity.getResources() : android.content.res.Resources
        const maui::platform::android::local_ref<jclass> context_class{env, env->GetObjectClass(activity)};
        if (!context_class)
        {
            return fallback;
        }
        jmethodID get_resources =
            env->GetMethodID(context_class.get(), "getResources", "()Landroid/content/res/Resources;");
        if (get_resources == nullptr || env->ExceptionCheck() == JNI_TRUE)
        {
            env->ExceptionClear();
            return fallback;
        }
        const maui::platform::android::local_ref<jobject> resources{env,
                                                                    env->CallObjectMethod(activity, get_resources)};
        if (env->ExceptionCheck() == JNI_TRUE || !resources)
        {
            env->ExceptionClear();
            return fallback;
        }
        // resources.getDisplayMetrics() : android.util.DisplayMetrics
        const maui::platform::android::local_ref<jclass> resources_class{env, env->GetObjectClass(resources.get())};
        jmethodID get_metrics =
            env->GetMethodID(resources_class.get(), "getDisplayMetrics", "()Landroid/util/DisplayMetrics;");
        if (get_metrics == nullptr || env->ExceptionCheck() == JNI_TRUE)
        {
            env->ExceptionClear();
            return fallback;
        }
        const maui::platform::android::local_ref<jobject> metrics{env,
                                                                  env->CallObjectMethod(resources.get(), get_metrics)};
        if (env->ExceptionCheck() == JNI_TRUE || !metrics)
        {
            env->ExceptionClear();
            return fallback;
        }
        const maui::platform::android::local_ref<jclass> metrics_class{env, env->GetObjectClass(metrics.get())};
        jfieldID width_field = env->GetFieldID(metrics_class.get(), "widthPixels", "I");
        jfieldID height_field = env->GetFieldID(metrics_class.get(), "heightPixels", "I");
        jfieldID density_field = env->GetFieldID(metrics_class.get(), "density", "F");
        if (width_field == nullptr || height_field == nullptr)
        {
            env->ExceptionClear();
            return fallback;
        }
        const jint width_px = env->GetIntField(metrics.get(), width_field);
        const jint height_px = env->GetIntField(metrics.get(), height_field);
        // density (px per dp) converts the device PIXELS the metrics report into the density-independent
        // POINTS the framework lays out in (the android handlers to_pixels back at the seam). A metrics
        // object always carries density; guard anyway and treat <=0 as 1.0 (px == dp).
        jfloat density = density_field != nullptr ? env->GetFloatField(metrics.get(), density_field) : 1.0F;
        if (density <= 0.0F)
        {
            density = 1.0F;
        }
        if (width_px <= 0 || height_px <= 0)
        {
            return fallback;
        }
        // EDGE-TO-EDGE (API 30+): the layout area is the WHOLE window, and the system bars overlay it —
        // MauiHostActivity.onCreate turns decor fitting off exactly as MauiAppCompatActivity.cs:31 does, so
        // the port must lay out over getCurrentWindowMetrics().getBounds() and let the views that respond to
        // the safe area inset themselves (drive_layout_viewport below). Measured with `uiautomator dump`:
        // MAUI's content root spans 0..2340 on this emulator; this function used to return 2138 (the window
        // minus the bars), which is what put every centred page 35px low.
        //
        // Fallback (API < 30 / helper unavailable): the legacy heightPixels - dimen-chrome with a ZERO safe
        // area — i.e. the pre-edge-to-edge behavior, unchanged, because the decor still fits there. Clamped
        // so a bogus read can never zero/invert the height.
        std::array<jint, 6> frame{};
        if (window_frame_px(env, activity, frame))
        {
            const auto to_dip = [density](jint px) { return static_cast<double>(px) / density; };
            return {to_dip(frame[0]), to_dip(frame[1]),
                    maui::core::thickness{to_dip(frame[2]), to_dip(frame[3]), to_dip(frame[4]), to_dip(frame[5])}};
        }
        jint content_height_px = height_px;
        const jint chrome_px = content_chrome_height_px(env, activity);
        if (chrome_px > 0 && chrome_px < height_px)
        {
            content_height_px -= chrome_px;
        }
        return {static_cast<double>(width_px) / density, static_cast<double>(content_height_px) / density, {}};
    }

    // The Android SAFE-AREA INSET PRODUCER. MAUI does this natively — an IOnApplyWindowInsetsListener
    // (src/Core/src/Platform/Android/MauiWindowInsetListener.cs) hands each view the window insets and
    // SafeAreaExtensions.ApplyAdjustedSafeAreaInsetsPx SetPadding()s the portion that view actually overlaps.
    // The port arranges CROSS-PLATFORM (layout::measure/arrange, scroll_view::arrange and
    // content_page::layout_inset already shrink by the realized insets and add them back), so there is no
    // native padding to set: the producer's whole job is to compute the right number and hand it to the
    // generic two-rect maui::hosting::drive_layout, which pushes it to the page and then to the page's
    // content (withholding, per edge, whatever the page itself consumed).
    //
    // The only piece that is genuinely Android's is the OVERLAP RULE (SafeAreaExtensions.cs:160-215): a view
    // receives only the part of an inset its own on-screen rect actually intersects, so a view that sits in
    // the MIDDLE of the page gets ZERO. That is what keeps MAUI's `border` page centred at 2340/2 rather than
    // in the band between the bars. Two passes, because the rule needs the arranged rect: pass 1 with no
    // insets yields the frame, pass 2 applies what the rule derived from it. (MAUI needs the same two rounds
    // — SetPadding triggers a re-layout.) Fill-height roots and centred roots are both stable under a third
    // pass: a Fill root's frame does not move when its CHILDREN are inset, and a centred root resolves to a
    // zero inset.
    inline void drive_layout_viewport(maui::controls::window& window, const viewport& view)
    {
        const maui::graphics::rect full{0, 0, view.width, view.height};
        if (view.safe_area.is_empty())
        {
            maui::hosting::drive_layout(window, full, full);
            return;
        }

        maui::hosting::drive_layout(window, full, full); // pass 1: no insets, purely to realize the frames

        // C# SafeAreaExtensions.cs:150-155 — the view's rect is taken RELATIVE TO ITS PARENT, i.e. with its
        // own margin backed out of the top/left and added onto the right/bottom, so that "margins and safe
        // area insets are additive rather than overlapping" (its words: 20px margin + 30px safe area = 50px
        // total offset). frame() is already the post-margin frame (compute_frame / LayoutExtensions.
        // ComputeFrame), so backing the margin out returns the parent-relative origin the C# uses.
        //
        // The C# reads the rect off the native view (GetLocationOnScreen), i.e. in SCREEN coordinates; here
        // frame() is PAGE-local, and the two agree because this host's page IS the window's content view and
        // the window is now edge-to-edge, so the page origin is the screen origin. A host that ever insets or
        // offsets the page again would have to add that offset back before the comparison.
        maui::core::thickness insets = view.safe_area;
        const auto* page = dynamic_cast<const maui::core::i_content_view*>(window.content());
        const maui::core::i_view* content = page != nullptr ? page->content() : nullptr;
        if (content != nullptr)
        {
            const maui::graphics::rect frame = content->frame();
            const maui::core::thickness margin = content->margin();
            const double view_left = std::max(0.0, frame.x - margin.left);
            const double view_top = std::max(0.0, frame.y - margin.top);
            const double view_right = frame.x + frame.width + margin.right;
            const double view_bottom = frame.y + frame.height + margin.bottom;
            // Each edge: the overlap amount, or 0 when the view does not reach into that band at all.
            // ponytail: the C# also carries `viewIsAnimatingHorizontally/Vertically` escape hatches for a
            // view sliding in off-screen during a Shell/fragment transition (SafeAreaExtensions.cs:130-146).
            // This host mounts ONE page and never animates a navigation, so the frames it measures are always
            // the settled ones; add the guards if a navigating host ever lands on Android.
            insets = maui::core::thickness{
                (insets.left > 0.0 && view_left < insets.left) ? std::min(insets.left - view_left, insets.left) : 0.0,
                (insets.top > 0.0 && view_top < insets.top) ? std::min(insets.top - view_top, insets.top) : 0.0,
                (insets.right > 0.0 && view_right > view.width - insets.right)
                    ? std::min(view_right - (view.width - insets.right), insets.right)
                    : 0.0,
                (insets.bottom > 0.0 && view_bottom > view.height - insets.bottom)
                    ? std::min(view_bottom - (view.height - insets.bottom), insets.bottom)
                    : 0.0};
        }

        maui::hosting::drive_layout(window, full,
                                    maui::graphics::rect{insets.left, insets.top,
                                                         view.width - insets.left - insets.right,
                                                         view.height - insets.top - insets.bottom});
    }
} // namespace maui::platform::android::apphost
