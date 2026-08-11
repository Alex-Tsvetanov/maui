// refresh_view_handler — Android (JNI) platform partial: a real
// androidx.swiperefreshlayout.widget.SwipeRefreshLayout (MAUI's MauiSwipeRefreshLayout) wrapping a
// dev.mauicpp.MauiLayout ViewGroup that HOSTS the single scrollable Content child of a RefreshView. The
// android twin of src/platform/apple/refresh_view_handler.mm (a plain NSView host) /
// src/platform/ios/refresh_view_handler.mm (a UIView host + a real UIRefreshControl) and the real-native
// sibling of the headless mirror (src/platform/headless/refresh_view_handler.cpp).
//
// ──────────────────────────────────────────────────────────────────────────────────────────────────
// WAVE 18: RefreshView on Android (the 2 gallery pages this unblocks)
// ──────────────────────────────────────────────────────────────────────────────────────────────────
// Every other backend renders these pages; Android rendered NEITHER the RefreshView's content because
// there was no Android RefreshView partial:
//   - swipe_refresh (was ⬛ blank): a refresh_view wrapping a vertical_stack(swipe_view, readout). With
//     no host, the column never mounted as a native subview, so the WHOLE page (the swipe row + the
//     "Ready" readout) was missing — only the app bar + background rendered.
//   - refresh_view (was 🟡): a refresh_view wrapping a CollectionView. The header/buttons/labels above
//     the refresh_view rendered, but its hosted content (the bottom status + the list) was missing.
// This file is that partial. Its CONTENT host reuses the content_page/swipe_view single-content hosting
// infra (the MauiLayout no-op-onLayout ViewGroup): set_content re-parents the single Content's native
// view as a MATCH_PARENT child, and platform_arrange frames the outer host EXACTLY (measure + layout) — the
// content's own host-relative platform_arrange then frames it within (refresh_view::arrange was made
// host-relative in this wave, the swipe_view/border/templated_view family fix — the host is framed
// FIRST, then the content is arranged at {inset.left, inset.top} so it lands inside the already-framed
// host rather than double-offsetting by the host's own non-zero page Y).
//
// ──────────────────────────────────────────────────────────────────────────────────────────────────
// THE PULL GESTURE + SPINNER ARE NO LONGER A DEVIATION (2026-08, this file's rewrite)
// ──────────────────────────────────────────────────────────────────────────────────────────────────
// The note that stood here said MAUI's MauiSwipeRefreshLayout : AndroidX.SwipeRefreshLayout was
// "unavailable on this AAR-less backend". THAT PREMISE IS DEAD. tools/parity/lib/android_aar_deps.txt
// stages the full AndroidX + Google Material closure out of the local NuGet cache into both app hosts —
// the swiperefreshlayout AAR is simply one more line in it (added with this change), the way
// androidx.recyclerview already was when the CarouselView deferral note made the identical wrong claim.
//
// It cost 8 board cells. MEASURED ON DEVICE (emulator-5554, page swipe_refresh,
// `adb input swipe 540 351 540 1287 600`), against a freshly rebuilt MauiReference APK:
//     MAUI reference : 0.51% of the frame changes — the spinner visibly renders
//     port cpp       : 0.00% — nothing moved
//     port cpp_xaml  : 0.00% — nothing moved
// The stale deferral had masked it for as long as the reference APK was stale.
//
// WHAT IS BUILT NOW (RefreshViewHandler.Android.cs + Platform/Android/MauiSwipeRefreshLayout.cs):
//   CreatePlatformView -> new SwipeRefreshLayout(Context)      the real pull recognizer + spinner
//   ConnectHandler     -> Refresh += OnSwipeRefresh            java/MauiRefreshBridge.java -> the
//                                                              android_refresh_ops.hpp peer registry ->
//                                                              request_refresh() (IsRefreshing = true)
//   MapIsRefreshing    -> setRefreshing(bool)
//   MapRefreshColor    -> setColorSchemeColors(int[])
//   MapIsRefreshEnabled-> setEnabled(bool)                     see the note below
//
// WHY THE HOST IS TWO NESTED ViewGroups AND NOT ONE. This is the load-bearing design decision, and
// collapsing it would silently break every RefreshView page's layout:
//
//     SwipeRefreshLayout  (`native` — framed absolutely by platform_arrange; owns the spinner)
//       └── MauiLayout    (`content_host` — onLayout is a NO-OP, so absolute child frames survive)
//             └── the Content's native view (framed by ITS OWN host-relative platform_arrange)
//
// SwipeRefreshLayout.onLayout FORCE-LAYS-OUT its single non-spinner child (`mTarget`, found by
// ensureTarget()) to fill its padded bounds: `child.layout(paddingLeft, paddingTop, +w, +h)`. That is
// exactly right for a container that fills, and exactly WRONG for the port's Content, which
// refresh_view::arrange places at {inset.left, inset.top} at its measured size — a direct Content child
// would be stretched and re-originated on every layout pass. Nesting resolves it without a compromise:
// the SwipeRefreshLayout stretches the MauiLayout (which is what we want it to do), and MauiLayout's
// no-op onLayout leaves the grandchild exactly where the port put it, so the nesting adds a spinner
// without moving a pixel of the resting content.
//
// That is only true because build_swipe_refresh explicitly calls setClipChildren(false) on the outer
// host. The claim was originally written here as "BYTE-IDENTICAL" with no such call, and it was wrong:
// the host this replaces was a MauiLayout, which sets clipChildren=false in its ctor
// (java/MauiLayout.java:83), while a stock SwipeRefreshLayout keeps ViewGroup's default of TRUE — so
// content overflowing the RefreshView's arranged bounds would have started being clipped. See the call
// site for why that had to be matched rather than accepted.
//
// The corollary trap: set_content's removeAllViews() must target `content_host`, NEVER `native`. The
// SwipeRefreshLayout's spinner (mCircleView) is one of ITS children, added in its constructor — clearing
// the outer host deletes the spinner permanently and reproduces the 0.00% symptom above.
//
// ALL-OR-NOTHING, and why. Every failure mode of this seam looks the same from the outside: no spinner.
// A HALF-wired SwipeRefreshLayout is strictly worse than none — it would spin forever with no listener
// to run the command, or render a spinner over content it laid out wrong. So build_swipe_refresh
// succeeds completely or the handler keeps the PREVIOUS host (a plain MauiLayout), which at least
// renders the resting content correctly. One WARN per failure path on tag "maui-refresh", because "the
// RefreshView does not spin" has causes as far apart as a missing AAR and a failed RegisterNatives.
// The ONE best-effort step is the progress-view offset below; it is cosmetic, and failing the whole
// build over a few pixels of spinner offset would trade the feature for the polish.
//
// TWO KNOWING DIFFERENCES FROM RefreshViewHandler.Android.cs, so the next reader can tell deliberate
// from missed:
//   - MapBackground -> SetProgressBackgroundColorSchemeColor is NOT ported. On Android alone, MAUI
//     routes IRefreshView.Background to the SPINNER'S CIRCLE background rather than to the view. The
//     port's update_background paints the ViewGroup through the shared apply_background, which is what
//     the currently-green static captures were scored against; changing it is a separate, measurable
//     change, not a free-rider on this one.
//   - MapIsRefreshEnabled -> setEnabled, where MAUI has a distinct RefreshEnabled latch that its
//     CanChildScrollUp override reads. setEnabled(false) gates the pull inside
//     SwipeRefreshLayout.onInterceptTouchEvent without blocking touches to the content, which is the
//     same observable behaviour for this control; MAUI needs the separate latch only because it also
//     maps the generic IsEnabled onto Enabled, which this partial deliberately does not (see the
//     header's is_enabled note). MAUI's CanChildScrollUp walk (content already scrolled down => no
//     pull) is likewise not ported: it needs a Java SUBCLASS of SwipeRefreshLayout, and the default
//     `mTarget.canScrollVertically(-1)` on a non-scrolling MauiLayout is always false, so the pull is
//     always available — a difference only reachable with the content already scrolled.
//
// VM-less degradation (like content_page/swipe_view/button): every JNI path checks scoped_env /
// app_context() and quietly skips when no Java VM exists (the android preset's pure-native cross-platform
// suite runs on the emulator without one), while the headless `hosted_content` mirror + the refresh
// mirrors are ALWAYS maintained so that suite observes exactly the headless partial's content tracking.
// request_refresh() is unchanged and remains the portable entry point every backend shares — the Java
// listener now drives that same function instead of only the tests doing so.

#include "maui/core/refresh_view_handler.hpp"

#include <android/log.h>
#include <jni.h>

#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstdlib> // std::abs(int) — the Math.Abs in MauiSwipeRefreshLayout's progress-offset workaround
#include <memory>
#include <string_view>

#include "android_refresh_ops.hpp"
#include "android_semantics_ops.hpp"
#include "android_view_ops.hpp"
#include "android_visual_ops.hpp"
#include "jni/app_context.hpp"
#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/core/flow_direction.hpp"
#include "maui/core/i_refresh_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"

namespace
{
    using maui::platform::android::app_context;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;
    using maui::platform::android::to_jstring;

    constexpr const char* k_maui_layout_class = "dev/mauicpp/MauiLayout";
    constexpr const char* k_view_group_class = "android/view/ViewGroup";
    constexpr const char* k_layout_params_class = "android/view/ViewGroup$LayoutParams";
    constexpr const char* k_measure_spec_class = "android/view/View$MeasureSpec";
    // `native` is a SwipeRefreshLayout OR (on the fallback path) a MauiLayout, so every method this file
    // calls on it is resolved through android.view.View / android.view.ViewGroup — the common base. A
    // jmethodID is only valid on instances of the class it was looked up on (or a SUBCLASS of it), and
    // SwipeRefreshLayout is not a MauiLayout, so the old k_maui_layout_class lookups would have been
    // undefined behaviour the moment the host changed type.
    constexpr const char* k_view_class = "android/view/View";
    constexpr const char* k_refresh_log_tag = "maui-refresh";

    constexpr jint k_view_visible = 0;
    constexpr jint k_view_invisible = 4;
    constexpr jint k_view_gone = 8;
    constexpr jint k_important_for_accessibility_auto = 0;
    constexpr auto k_measure_spec_exactly = static_cast<jint>(0x40000000U);
    constexpr jint k_match_parent = -1; // ViewGroup.LayoutParams.MATCH_PARENT — the content fills the host
    constexpr double k_to_pixels_epsilon = 0.0000000001;

    // The OUTER host — the view the parent parents and platform_arrange frames. A SwipeRefreshLayout when
    // one could be built, the MauiLayout otherwise.
    [[nodiscard]] jobject host_of(const maui::core::refresh_view_platform& platform) noexcept
    {
        return static_cast<jobject>(platform.native);
    }

    // The ViewGroup that parents the Content. The inner MauiLayout when a SwipeRefreshLayout was built,
    // and `native` itself on the fallback path — where `native` IS the MauiLayout. Never clear or add to
    // host_of() when these differ: the SwipeRefreshLayout's own children include its spinner.
    [[nodiscard]] jobject content_host_of(const maui::core::refresh_view_platform& platform) noexcept
    {
        return static_cast<jobject>(platform.content_host != nullptr ? platform.content_host : platform.native);
    }

    // True when the SwipeRefreshLayout stack was built (so the refresh pushes have somewhere to land).
    // content_host is deliberately left NULL on the fallback path rather than aliasing `native`, which is
    // what makes this one check sufficient — and what keeps the destructor from double-releasing.
    [[nodiscard]] bool has_swipe_refresh(const maui::core::refresh_view_platform& platform) noexcept
    {
        return platform.native != nullptr && platform.content_host != nullptr;
    }

    // Clears any pending Java exception (the partial must never leak JNI pending-exception state into the
    // cross-platform layer); true when one was pending — call sites skip the read-back.
    bool clear_pending(JNIEnv* env)
    {
        if (env->ExceptionCheck() == JNI_FALSE)
        {
            return false;
        }
        env->ExceptionDescribe(); // logcat/stderr breadcrumb, the same channel the test host uses
        env->ExceptionClear();
        return true;
    }

    void call_void_int(JNIEnv* env, jobject host, const char* name, jint value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_view_class, name, "(I)V"))
        {
            env->CallVoidMethod(host, method, value);
            clear_pending(env);
        }
    }

    void call_void_float(JNIEnv* env, jobject host, const char* name, jfloat value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_view_class, name, "(F)V"))
        {
            env->CallVoidMethod(host, method, value);
            clear_pending(env);
        }
    }

    // The two boolean pushes the refresh surface needs: SwipeRefreshLayout.setRefreshing (MapIsRefreshing)
    // and View.setEnabled (MapIsRefreshEnabled). `owner` names which class the id is looked up on — the
    // distinction matters, see k_view_class's note.
    void call_void_bool(JNIEnv* env, jobject host, const char* owner, const char* name, bool value)
    {
        if (jmethodID method = default_jni_cache().method(env, owner, name, "(Z)V"))
        {
            env->CallVoidMethod(host, method, value ? JNI_TRUE : JNI_FALSE);
            clear_pending(env);
        }
    }

    // ContextExtensions.ToPixels: ceil(dp * density - Epsilon).
    [[nodiscard]] jint to_pixels(double dp, float density)
    {
        return static_cast<jint>(std::ceil((dp * static_cast<double>(density)) - k_to_pixels_epsilon));
    }

    // The host's display density (Context.getResources().getDisplayMetrics().density), memoized
    // process-wide after the first read, exactly like content_page_handler.cpp's display_density. 1.0 when
    // any step fails (failures are NOT memoized).
    [[nodiscard]] float display_density(JNIEnv* env, jobject host)
    {
        static std::atomic<float> memoized{0.0F};
        if (const float cached = memoized.load(std::memory_order_relaxed); cached != 0.0F)
        {
            return cached;
        }
        auto& cache = default_jni_cache();
        jmethodID get_context = cache.method(env, k_view_class, "getContext", "()Landroid/content/Context;");
        jmethodID get_resources =
            cache.method(env, "android/content/Context", "getResources", "()Landroid/content/res/Resources;");
        jmethodID get_display_metrics =
            cache.method(env, "android/content/res/Resources", "getDisplayMetrics", "()Landroid/util/DisplayMetrics;");
        jfieldID density_field = cache.field(env, "android/util/DisplayMetrics", "density", "F");
        if (get_context == nullptr || get_resources == nullptr || get_display_metrics == nullptr ||
            density_field == nullptr)
        {
            return 1.0F;
        }
        const local_ref<jobject> context{env, env->CallObjectMethod(host, get_context)};
        if (clear_pending(env) || !context)
        {
            return 1.0F;
        }
        const local_ref<jobject> resources{env, env->CallObjectMethod(context.get(), get_resources)};
        if (clear_pending(env) || !resources)
        {
            return 1.0F;
        }
        const local_ref<jobject> metrics{env, env->CallObjectMethod(resources.get(), get_display_metrics)};
        if (clear_pending(env) || !metrics)
        {
            return 1.0F;
        }
        const jfloat density = env->GetFloatField(metrics.get(), density_field);
        if (clear_pending(env) || density == 0.0F)
        {
            return 1.0F;
        }
        memoized.store(density, std::memory_order_relaxed);
        return density;
    }

    // The content's native android.view.View, via its view-handler's native_view() (C#'s ToPlatform() =
    // ContainerView ?? PlatformView). Null when the content is unattached. Mirrors content_page_handler.cpp.
    [[nodiscard]] jobject native_child(maui::core::i_view& child)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(child.handler().get());
        if (handler == nullptr)
        {
            return nullptr;
        }
        return static_cast<jobject>(handler->native_view());
    }

    // Detach `child` from any ViewGroup parent (removeView), so addView never throws "already has a parent"
    // (the re-parent guard content_page/swipe_view/window handlers share).
    void detach_from_parent(JNIEnv* env, jobject child)
    {
        auto& cache = default_jni_cache();
        jmethodID get_parent = cache.method(env, "android/view/View", "getParent", "()Landroid/view/ViewParent;");
        if (get_parent == nullptr)
        {
            return;
        }
        const local_ref<jobject> parent{env, env->CallObjectMethod(child, get_parent)};
        if (clear_pending(env) || !parent)
        {
            return;
        }
        jclass view_group_class = cache.find_class(env, k_view_group_class);
        if (view_group_class == nullptr || env->IsInstanceOf(parent.get(), view_group_class) == JNI_FALSE)
        {
            return;
        }
        jmethodID remove_view = cache.method(env, k_view_group_class, "removeView", "(Landroid/view/View;)V");
        if (remove_view != nullptr)
        {
            env->CallVoidMethod(parent.get(), remove_view, child);
            clear_pending(env);
        }
    }

    // Add `child` to `host` with MATCH_PARENT/MATCH_PARENT layout params so it fills the host (the refresh
    // Content sizes to the host bounds; the content's own host-relative platform_arrange then frames it
    // within). Same call content_page_handler.cpp's add_filling_child uses. Returns false when the child
    // did not land: set_content ignores that (an empty host renders nothing, which is what an unhosted
    // Content already looked like), but build_swipe_refresh must NOT — a SwipeRefreshLayout with no
    // non-spinner child has no `mTarget`, so its onLayout returns early and the page renders BLANK, which
    // is strictly worse than the spinner-less fallback.
    [[nodiscard]] bool add_filling_child(JNIEnv* env, jobject host, jobject child)
    {
        detach_from_parent(env, child);
        auto& cache = default_jni_cache();
        jclass params_class = cache.find_class(env, k_layout_params_class);
        jmethodID params_ctor = cache.method(env, k_layout_params_class, "<init>", "(II)V");
        jmethodID add_view = cache.method(env, k_view_group_class, "addView",
                                          "(Landroid/view/View;Landroid/view/ViewGroup$LayoutParams;)V");
        if (params_class == nullptr || params_ctor == nullptr || add_view == nullptr)
        {
            return false;
        }
        const local_ref<jobject> params{env, env->NewObject(params_class, params_ctor, k_match_parent, k_match_parent)};
        if (clear_pending(env) || !params)
        {
            return false;
        }
        env->CallVoidMethod(host, add_view, child, params.get());
        return !clear_pending(env);
    }

    // MauiSwipeRefreshLayout's ctor workaround, ported verbatim from
    // src/Core/src/Platform/Android/MauiSwipeRefreshLayout.cs:31:
    //     SetProgressViewOffset(true, ProgressViewStartOffset,
    //                           ProgressViewEndOffset - Math.Abs(ProgressViewStartOffset));
    // (dotnet/maui#17647 / issuetracker 110463864 — the spinner's resting and end offsets disagree out of
    // the box.) BEST-EFFORT, and the only step of the build that is: it only moves the spinner a few
    // pixels, so failing the whole SwipeRefreshLayout over it would trade the feature for the polish.
    void apply_progress_view_offset(JNIEnv* env, jobject swipe_refresh)
    {
        namespace seam = maui::platform::android;
        auto& cache = default_jni_cache();
        jmethodID get_start =
            cache.method(env, seam::detail::k_swipe_refresh_class, "getProgressViewStartOffset", "()I");
        jmethodID get_end = cache.method(env, seam::detail::k_swipe_refresh_class, "getProgressViewEndOffset", "()I");
        jmethodID set_offset =
            cache.method(env, seam::detail::k_swipe_refresh_class, "setProgressViewOffset", "(ZII)V");
        if (get_start == nullptr || get_end == nullptr || set_offset == nullptr)
        {
            clear_pending(env);
            __android_log_print(ANDROID_LOG_WARN, k_refresh_log_tag,
                                "progress-view-offset accessors missing (start=%d end=%d set=%d) — the spinner "
                                "renders at the AndroidX default offset",
                                static_cast<int>(get_start != nullptr), static_cast<int>(get_end != nullptr),
                                static_cast<int>(set_offset != nullptr));
            return;
        }
        const jint start = env->CallIntMethod(swipe_refresh, get_start);
        const jint end = env->CallIntMethod(swipe_refresh, get_end);
        if (clear_pending(env))
        {
            return;
        }
        env->CallVoidMethod(swipe_refresh, set_offset, JNI_TRUE, start, end - std::abs(start));
        clear_pending(env);
    }

    // Build MAUI's host: a SwipeRefreshLayout wrapping `content_host` (see the file header for why the two
    // are nested). ALL-OR-NOTHING — on any failure this returns null, having added nothing to
    // `content_host`, and the caller keeps the bare MauiLayout as the host. The returned jobject is a
    // GLOBAL ref the caller owns.
    //
    // `peer` is minted here rather than in on_connect_handler because the listener install is the one
    // fallible step that decides which host is returned: a SwipeRefreshLayout with no listener spins
    // forever and never runs the RefreshCommand. Its callback body is bound later (no JNI, cannot fail).
    [[nodiscard]] jobject build_swipe_refresh(JNIEnv* env, jobject context, jobject content_host,
                                              std::shared_ptr<maui::platform::android::refresh_trampoline>& peer)
    {
        namespace seam = maui::platform::android;
        auto& cache = default_jni_cache();
        jclass swipe_class = cache.find_class(env, seam::detail::k_swipe_refresh_class);
        jmethodID swipe_ctor =
            cache.method(env, seam::detail::k_swipe_refresh_class, "<init>", "(Landroid/content/Context;)V");
        if (swipe_class == nullptr || swipe_ctor == nullptr)
        {
            clear_pending(env);
            // The two realistic causes are opposite fixes — a missing androidx.swiperefreshlayout AAR (a
            // packaging problem, tools/parity/lib/android_aar_deps.txt) vs a stale staged closure — so name
            // which lookup failed rather than making the next reader bisect it.
            __android_log_print(ANDROID_LOG_WARN, k_refresh_log_tag,
                                "SwipeRefreshLayout missing (class=%d ctor=%d) — the RefreshView falls back to "
                                "the static MauiLayout host and will NOT show a spinner",
                                static_cast<int>(swipe_class != nullptr), static_cast<int>(swipe_ctor != nullptr));
            return nullptr;
        }
        const local_ref<jobject> swipe{env, env->NewObject(swipe_class, swipe_ctor, context)};
        if (clear_pending(env) || !swipe)
        {
            __android_log_print(ANDROID_LOG_WARN, k_refresh_log_tag,
                                "SwipeRefreshLayout ctor threw — the RefreshView falls back to the static host");
            return nullptr;
        }
        // MATCH THE OUTGOING HOST'S CLIPPING, or this rewrite silently changes the RESTING render on every
        // RefreshView page. The host this replaces was a MauiLayout, whose ctor calls setClipChildren(false)
        // (java/MauiLayout.java:83) precisely because the port drives ABSOLUTE child frames and a child may
        // legitimately extend past its parent's arranged bounds. A stock SwipeRefreshLayout keeps
        // ViewGroup's default clipChildren=TRUE, so overflowing content that used to draw would now be cut
        // off — a regression that shows up as pixels lost on pages that are green today, and one no spinner
        // test would catch. Best-effort: if the lookup fails the pull still works, so this must not fail the
        // feature.
        if (jmethodID set_clip = cache.method(env, k_view_group_class, "setClipChildren", "(Z)V"))
        {
            env->CallVoidMethod(swipe.get(), set_clip, static_cast<jboolean>(JNI_FALSE));
            clear_pending(env);
        }
        // `platformView.Refresh += OnSwipeRefresh` BEFORE the content goes in, so no arrangement of the
        // tree can ever exist with a pullable spinner and no listener behind it.
        peer = seam::make_refresh_peer();
        if (!seam::install_refresh_listener(env, swipe.get(), peer.get()))
        {
            peer.reset();
            __android_log_print(ANDROID_LOG_WARN, k_refresh_log_tag,
                                "could not install the OnRefreshListener (missing dev.mauicpp.MauiRefreshBridge in "
                                "the host dex, or RegisterNatives failed) — the RefreshView falls back to the "
                                "static host rather than to a spinner that never stops");
            return nullptr;
        }
        // MauiSwipeRefreshLayout.UpdateContent's `AddView(_contentView, MatchParent/MatchParent)`, one
        // level out: the MauiLayout is the ONLY non-spinner child, so it is the `mTarget` ensureTarget()
        // finds, and SwipeRefreshLayout stretches it to fill — which is exactly the frame the port's
        // absolutely-positioned grandchildren are drawn against. REQUIRED: no mTarget means onLayout
        // bails and the whole page goes blank.
        if (!add_filling_child(env, swipe.get(), content_host))
        {
            peer.reset();
            __android_log_print(ANDROID_LOG_WARN, k_refresh_log_tag,
                                "could not parent the content host into the SwipeRefreshLayout — falling back to "
                                "the static host rather than to a blank page");
            return nullptr;
        }
        apply_progress_view_offset(env, swipe.get());
        return env->NewGlobalRef(swipe.get()); // released in ~refresh_view_platform
    }
} // namespace

namespace maui::core
{
    // Releases the global references pinning the refresh host pair (the JNI shape of the
    // pimpl-owned-native-view doctrine; the apple twin CFReleases its NSView here), and drops the
    // OnRefreshListener peer.
    //
    // ORDER MATTERS: the peer is released FIRST. Its callback captures the handler, and the handler is
    // what owns this struct — so by the time the refs go away the registry must already have stopped
    // resolving the token the still-alive Java bridge is holding. release_refresh_seam clears the
    // callback before unregistering, so a pull already in flight is a no-op rather than a call into a
    // dying handler. This runs for a handler dropped WITHOUT a disconnect too, which is why the teardown
    // lives here rather than in an on_disconnect_handler.
    refresh_view_platform::~refresh_view_platform()
    {
        maui::platform::android::release_refresh_seam(refresh_peer);
        if (native == nullptr && content_host == nullptr)
        {
            return;
        }
        const scoped_env env; // any-thread teardown, exactly like global_ref::reset
        if (env)
        {
            if (native != nullptr)
            {
                env->DeleteGlobalRef(static_cast<jobject>(native));
            }
            // Null on the fallback path (where `native` IS the MauiLayout), so this can never
            // double-release what the line above already dropped.
            if (content_host != nullptr)
            {
                env->DeleteGlobalRef(static_cast<jobject>(content_host));
            }
        }
        native = nullptr;
        content_host = nullptr;
    }

    // ---- the generic-IView property pushes (the shared view_mapper calls these through
    // view_platform_base). Each calls the base body FIRST — the headless mirrors must stay live for the
    // VM-less cross-platform suite — then pushes to the real ViewGroup when one exists (the content_page
    // dual-path pattern). ----

    void refresh_view_platform::update_visibility(maui::core::visibility value)
    {
        view_platform_base::update_visibility(value);
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jint state = k_view_visible;
        if (value == maui::core::visibility::hidden)
        {
            state = k_view_invisible;
        }
        else if (value == maui::core::visibility::collapsed)
        {
            state = k_view_gone;
        }
        call_void_int(env.get(), host_of(*this), "setVisibility", state);
    }

    void refresh_view_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            call_void_float(env.get(), host_of(*this), "setAlpha", static_cast<jfloat>(value));
        }
    }

    void refresh_view_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        if (native == nullptr || value.find_first_not_of(" \t\n\v\f\r") == std::string_view::npos)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject host = host_of(*this);
        auto& cache = default_jni_cache();
        // setContentDescription flips ImportantForAccessibility to YES; restore AUTO when that is what the
        // host had (the content_page/swipe_view partials document the same dance).
        jmethodID get_important = cache.method(env.get(), k_view_class, "getImportantForAccessibility", "()I");
        jmethodID set_description =
            cache.method(env.get(), k_view_class, "setContentDescription", "(Ljava/lang/CharSequence;)V");
        if (get_important == nullptr || set_description == nullptr)
        {
            return;
        }
        const jint important_before = env->CallIntMethod(host, get_important);
        if (clear_pending(env.get()))
        {
            return;
        }
        const local_ref<jstring> description = to_jstring(env.get(), value);
        env->CallVoidMethod(host, set_description, description.get());
        if (clear_pending(env.get()))
        {
            return;
        }
        if (important_before == k_important_for_accessibility_auto)
        {
            call_void_int(env.get(), host, "setImportantForAccessibility", k_important_for_accessibility_auto);
        }
    }

    void refresh_view_platform::update_transform(const maui::core::transform_spec& value)
    {
        view_platform_base::update_transform(value);
        maui::platform::android::apply_transform(native, value);
    }

    void refresh_view_platform::update_flow_direction(maui::core::flow_direction value)
    {
        view_platform_base::update_flow_direction(value);
        maui::platform::android::apply_flow_direction(native, value);
    }

    void refresh_view_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        maui::platform::android::apply_background(native, value);
    }

    void refresh_view_platform::update_semantics(const maui::core::semantics* value)
    {
        view_platform_base::update_semantics(value);
        maui::platform::android::apply_semantics(native, value);
    }

    std::unique_ptr<refresh_view_platform> refresh_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<refresh_view_platform>();
        const scoped_env env;
        jobject context = app_context();
        if (!env || context == nullptr)
        {
            return platform; // VM-less / context-less: the headless-mirror degradation (header note)
        }
        auto& cache = default_jni_cache();
        // new MauiLayout(Context) — the CONTENT host (the no-op-onLayout ViewGroup, host-provided via
        // java/MauiLayout.java), built first and unconditionally because it is both halves of the
        // all-or-nothing choice: the inner child of a SwipeRefreshLayout on the good path, and the whole
        // host on the fallback. The plain View ctor is theme-independent, so it constructs in the bare
        // app_process testhost.
        jclass layout_class = cache.find_class(env.get(), k_maui_layout_class);
        jmethodID ctor = cache.method(env.get(), k_maui_layout_class, "<init>", "(Landroid/content/Context;)V");
        if (layout_class == nullptr || ctor == nullptr)
        {
            return platform; // without MauiLayout the host stays the headless mirror (VM-less degradation)
        }
        const local_ref<jobject> content_host{env.get(), env->NewObject(layout_class, ctor, context)};
        if (clear_pending(env.get()) || !content_host)
        {
            return platform;
        }
        // MAUI's host: new MauiSwipeRefreshLayout(Context), wrapping that MauiLayout. On ANY failure this
        // is null and the MauiLayout becomes the host on its own — the pre-2026-08 render, which shows the
        // resting content correctly and merely never spins (file header, "ALL-OR-NOTHING").
        if (jobject swipe = build_swipe_refresh(env.get(), context, content_host.get(), platform->refresh_peer))
        {
            // The LAST place all-or-nothing can still be violated, and the one window where it would be
            // silent. If this ref fails, content_host stays null, has_swipe_refresh() still reports true
            // off `native`, and content_host_of() falls back to `native` — at which point set_content's
            // removeAllViews() clears the SwipeRefreshLayout itself and deletes mCircleView, permanently
            // reproducing the 0.00% symptom this whole change exists to remove (the "corollary trap" in
            // the header). OOM-only and realistically unreachable, which is exactly why it has to be
            // handled here rather than left to be discovered as a spinner that stopped working.
            if (jobject hosted = env->NewGlobalRef(content_host.get()))
            {
                platform->native = swipe; // global ref, released in the dtor
                platform->content_host = hosted;
                return platform;
            }
            // Unwind to the documented fallback rather than proceed half-wired: drop the swipe host and
            // the peer so nothing can call back into a handler whose content host does not exist.
            env->DeleteGlobalRef(swipe);
            platform->refresh_peer.reset();
        }
        platform->native = env->NewGlobalRef(content_host.get()); // released in ~refresh_view_platform
        return platform;                                          // content_host stays null — see has_swipe_refresh
    }

    // ConnectHandler's `platformView.Refresh += OnSwipeRefresh` — its last, infallible step. The listener
    // is already installed (create_platform_view had to install it to know which host to return); what was
    // missing is the body, which needs the handler. OnSwipeRefresh's whole job in
    // RefreshViewHandler.Android.cs:24-27 is `VirtualView.IsRefreshing = true`, which is exactly what
    // request_refresh() does on every backend — so the native pull and the portable/test drive land
    // through one code path.
    void refresh_view_handler::on_connect_handler(refresh_view_platform& platform)
    {
        if (!platform.refresh_peer)
        {
            return; // the fallback host: no listener to bind
        }
        platform.refresh_peer->on_refresh = [this] { request_refresh(); };
    }

    void refresh_view_handler::set_content()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // The headless mirror is ALWAYS maintained (C# RefreshViewHandler.UpdateContent reads
        // VirtualView.Content).
        platform->hosted_content = virtual_view() != nullptr ? virtual_view()->content() : nullptr;

        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        // THE CONTENT HOST, never host_of(). When `native` is a SwipeRefreshLayout, its children include
        // the spinner it created in its own constructor — removeAllViews() there deletes mCircleView and
        // the RefreshView never spins again. On the fallback path the two are the same object.
        jobject host = content_host_of(*platform);
        // C# MauiSwipeRefreshLayout.UpdateContent: drop the old content, then add the new content's native
        // view filling the host. The same swap content_page_handler.cpp does for its single content.
        jmethodID remove_all = default_jni_cache().method(env.get(), k_view_group_class, "removeAllViews", "()V");
        if (remove_all != nullptr)
        {
            env->CallVoidMethod(host, remove_all);
            clear_pending(env.get());
        }
        if (platform->hosted_content == nullptr)
        {
            return; // an empty refresh host (the previous child was just removed)
        }
        if (jobject child = native_child(*platform->hosted_content))
        {
            static_cast<void>(add_filling_child(env.get(), host, child));
        }
    }

    // MapIsRefreshing: `PlatformView.Refreshing = VirtualView.IsRefreshing`. The mirror is written FIRST
    // and unconditionally — the VM-less cross-platform suite asserts on exactly that value — then pushed
    // to the real spinner when one exists.
    void refresh_view_handler::update_is_refreshing()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        platform->refreshing = virtual_view()->is_refreshing();
        if (!has_swipe_refresh(*platform))
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            call_void_bool(env.get(), host_of(*platform), maui::platform::android::detail::k_swipe_refresh_class,
                           "setRefreshing", platform->refreshing);
        }
    }

    void refresh_view_handler::update_refresh_color()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        // C# UpdateRefreshColor: the spinner tint follows RefreshColor. The mirror is always maintained
        // (headless / AppKit read it), then SetColorSchemeColors(color.ToInt()) pushes it.
        const maui::graphics::paint* const paint = virtual_view()->refresh_color();
        if (paint != nullptr)
        {
            platform->has_refresh_color = true;
            platform->refresh_color_argb = paint->background_color().to_uint();
        }
        else
        {
            platform->has_refresh_color = false;
            platform->refresh_color_argb = 0;
        }
        // `if (RefreshColor == null) return;` — C# leaves the previous scheme in place rather than
        // resetting to the platform default, so an unset color must NOT push anything.
        if (!platform->has_refresh_color || !has_swipe_refresh(*platform))
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        // SetColorSchemeColors is `int...` — a jintArray of one, matching C#'s single `color.Value`.
        jmethodID set_colors = default_jni_cache().method(
            env.get(), maui::platform::android::detail::k_swipe_refresh_class, "setColorSchemeColors", "([I)V");
        if (set_colors == nullptr)
        {
            return;
        }
        const local_ref<jintArray> colors{env.get(), env->NewIntArray(1)};
        if (clear_pending(env.get()) || !colors)
        {
            return;
        }
        const jint argb = static_cast<jint>(platform->refresh_color_argb);
        env->SetIntArrayRegion(colors.get(), 0, 1, &argb);
        if (clear_pending(env.get()))
        {
            return;
        }
        env->CallVoidMethod(host_of(*platform), set_colors, colors.get());
        clear_pending(env.get());
    }

    // MapIsRefreshEnabled. C# routes this to MauiSwipeRefreshLayout.RefreshEnabled, a latch its
    // CanChildScrollUp override reads; the port pushes View.setEnabled instead, which gates the pull in
    // SwipeRefreshLayout.onInterceptTouchEvent without blocking touches to the content — see the file
    // header for why the two are equivalent here.
    void refresh_view_handler::update_is_refresh_enabled()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        platform->refresh_enabled = virtual_view()->is_refresh_enabled();
        if (!has_swipe_refresh(*platform))
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            call_void_bool(env.get(), host_of(*platform), k_view_class, "setEnabled", platform->refresh_enabled);
        }
    }

    // OnSwipeRefresh (RefreshViewHandler.Android.cs:24-27): write IsRefreshing=true back through the
    // virtual view (which re-enters the control's coercion → Refreshing + command). Identical on every
    // backend so the programmatic/test path matches — and, since this rewrite, the destination of the REAL
    // Android pull too: SwipeRefreshLayout → MauiRefreshBridge.onRefresh → the android_refresh_ops peer →
    // here. Deliberately unchanged; the wiring landed around it, not in it.
    void refresh_view_handler::request_refresh()
    {
        if (auto* view = virtual_view())
        {
            view->set_is_refreshing(true);
        }
    }

    void refresh_view_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // headless / VM-less: no native host to position
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject host = host_of(*platform);
        auto& cache = default_jni_cache();
        // ViewHandler.PlatformArrange: the dp frame becomes pixels, the host measures Exactly at the final
        // size and lays out (the two-step every android handler uses). The traversal that follows is what
        // makes the nested host work, and it is worth spelling out: SwipeRefreshLayout.onMeasure measures
        // the MauiLayout Exactly at its own size and its spinner alongside it, then onLayout stretches the
        // MauiLayout to fill and centres the spinner — and MauiLayout.onLayout is a no-op, so the single
        // Content child underneath keeps the host-relative frame its own platform_arrange set. On the
        // fallback path `native` IS that MauiLayout and the middle step simply does not happen.
        jmethodID make_measure_spec = cache.static_method(env.get(), k_measure_spec_class, "makeMeasureSpec", "(II)I");
        jmethodID measure = cache.method(env.get(), k_view_class, "measure", "(II)V");
        jmethodID layout = cache.method(env.get(), k_view_class, "layout", "(IIII)V");
        jclass measure_spec_class = cache.find_class(env.get(), k_measure_spec_class);
        if (make_measure_spec == nullptr || measure == nullptr || layout == nullptr || measure_spec_class == nullptr)
        {
            return;
        }
        const float density = display_density(env.get(), host);
        const jint left = to_pixels(frame.x, density);
        const jint top = to_pixels(frame.y, density);
        const jint width = to_pixels(frame.width, density);
        const jint height = to_pixels(frame.height, density);
        const jint width_spec =
            env->CallStaticIntMethod(measure_spec_class, make_measure_spec, width, k_measure_spec_exactly);
        const jint height_spec =
            env->CallStaticIntMethod(measure_spec_class, make_measure_spec, height, k_measure_spec_exactly);
        if (clear_pending(env.get()))
        {
            return;
        }
        env->CallVoidMethod(host, measure, width_spec, height_spec);
        if (clear_pending(env.get()))
        {
            return;
        }
        env->CallVoidMethod(host, layout, left, top, left + width, top + height);
        clear_pending(env.get());
    }
} // namespace maui::core
