// collection_view_handler — Android (JNI) platform partial: a real android.widget.ScrollView whose
// single document child is a dev.mauicpp.MauiCollectionContent host ViewGroup, into which the handler
// realizes a native android.view.View per collection element and positions each ABSOLUTELY. The android twin
// of
// src/platform/apple/collection_view_handler.mm (a real NSCollectionView) and the real-native sibling of
// the headless mirror (src/platform/headless/collection_view_handler.cpp).
//
// ──────────────────────────────────────────────────────────────────────────────────────────────────
// WAVE 8: the CollectionView family on Android (the 40 blank gallery pages this unblocks)
// ──────────────────────────────────────────────────────────────────────────────────────────────────
// Every other backend renders the CollectionView family; Android rendered NOTHING (the audit found 40
// gallery pages blank — only the app bar over grey) because there was no Android CV partial. This file
// is that partial. The cross-platform simulator (src/controls/items/collection_view_handler.cpp) still
// runs as the in-memory state mirror on this backend EXACTLY as before — the android preset's pure-native
// cross-platform suite (no JavaVM) observes the headless partial's behavior unchanged. The native render
// is layered ON TOP, behind scoped_env / app_context guards, driven from arrange_native (the one hook
// platform_arrange calls unconditionally on every backend) — so NO shared-code edit was needed and the
// headless build is untouched.
//
// THE REALIZATION MODEL (mirrors the AppKit data source, adapted to the android absolute-frame seam):
//   - C++ drives layout — native android views do NOT self-layout (MauiLayout.onLayout is a no-op, the
//     same reason every container handler's children keep the absolute frames their own platform_arrange
//     set; see src/platform/android/java/MauiLayout.java). So arrange_native reads the handler's
//     i_items_view_source + the control's templates DIRECTLY (the RecyclerView-adapter analog), realizes
//     one native view per element, and lays each out at an absolute frame in the host panel.
//   - An ITEM with an ItemTemplate set realizes the template's content as a real native view bound to the
//     item (realize_template_content: create_handler<TControl>() → set_maui_context → set_handler builds
//     the native view + runs the mapper — the C# TemplatedCell2.Bind path, identical to the apple/ios
//     partials). With no template the default cell is a plain TextView mirroring item.text() (DefaultCell).
//   - HEADER / FOOTER (structured, global) and GROUP HEADER / FOOTER (grouped, per-section) realize the
//     same way: their template's content when set, else a TextView of the value's / group key's text.
//   - EMPTY VIEW (shown while the source is empty) realizes its template content centered, else a centered
//     TextView of the empty value's text — the C# UpdateEmptyView.
//   - GRID: a GridItemsLayout(span, orientation) lays `span` item columns across the cross axis; a linear
//     list is span 1. Both orientations are honored (vertical stacks rows down the main axis, horizontal
//     across it). Each realized view is MEASURED (View.measure at its column width) so a text row takes its
//     natural height — matching the iOS reference where each row is text-height, not a fixed extent.
//
// DOCUMENTED DEVIATIONS from the C# oracle (infrastructure gaps + the resume-doc render-first guidance,
// NOT behavior guesses):
//   - NO RecyclerView view-recycling. MAUI's Android CV is a MauiRecyclerView + ItemsViewAdapter with cell
//     reuse; the gallery pages have small fixed item counts, so this partial favors render correctness and
//     realizes every in-content element directly into the host panel (the resume-doc "favor correctness of
//     render over recycling"). The cross-platform simulator still records the realize/recycle/bind trail
//     for any consumer that wants it. A faithful RecyclerView adapter is a future refinement.
//   - The host is a plain ScrollView → MauiCollectionContent, NOT MauiRecyclerView; it scrolls VERTICALLY
//     only (the
//     content still measures/arranges at full size so the scroller has the right extent). A horizontal
//     CollectionView's content is laid out across the main axis but hosted in the vertical ScrollView —
//     the horizontal scroller swap is deferred with the scroll_view_handler's same documented deviation.
//   - estimated self-sizing is reduced to: each view measured at its column width with an unbounded main
//     axis, then laid out at its measured main extent (deterministic, matches the text-height rows).
//   - the selection highlight IS drawn: a selected cell gets the theme's colorActivatedHighlight fill
//     (SelectableViewHolder.GetSelectedDrawable() — orange on the emulator's Material theme), painted onto
//     the cell View from the cross-platform selected_paths mirror (see apply_selection_highlight below).
//     Interactive reorder-drag has no plain-View analog and is not drawn (the cross-platform mirror carries
//     it as state; the emulator is the asserted surface).
//   - snap points / peek insets / scroll-bar-visibility nuances are iOS-only knobs with no plain-ScrollView
//     analog (the cross-platform mirror carries them as state) — same as the other android container
//     deviations.
//
// ──────────────────────────────────────────────────────────────────────────────────────────────────
// WAVE 25: the CarouselView paged path (one item per page, not the CV's all-items flow)
// ──────────────────────────────────────────────────────────────────────────────────────────────────
// CarouselView reuses this collection_view_handler wholesale (carousel_view.cpp registers it), but a
// carousel PAGES — it shows ONE item at a time, full-viewport (LayoutFactory2.CreateCarouselLayout sizes
// each item to FractionalWidth/Height(1)), not the horizontal CV's all-items-concatenated flow. So when the
// virtual view is a carousel_view (detected by dynamic_cast — the same predicate the iOS layout factory and
// the scroll-end writeback use), arrange_native takes a dedicated branch: it realizes ONLY the item at the
// carousel's current Position (clamped into range) and frames it filling the whole viewport, centered. A
// static capture at Position 0 shows just "Item 1" large/centered, matching the iOS reference — NOT the
// "Item 1Item 2Item 3" concatenation a plain horizontal CV produces. DEFERRED: live swipe paging (the
// android backend has no androidx.viewpager2; a touch-drag that advances Position has no plain-View analog —
// the same swipe-channel cut the SwipeView/scroll partials document). The Prev/Next buttons in the gallery
// drive Position programmatically, which re-runs arrange_native and re-realizes the new current item.
//
// VM-less degradation (like every android handler): create_platform_view / arrange_native check scoped_env
// / app_context() and quietly skip when no Java VM exists (the pure-native cross-platform suite runs on the
// emulator without one) — the simulator state mirror is always live. The gallery app host drives the real
// ScrollView.

#include "maui/controls/items/collection_view_handler.hpp"

#include <jni.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "android_clip_ops.hpp"
#include "android_visual_ops.hpp"
#include "jni/app_context.hpp"
#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/controls/element.hpp"
#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/carousel_view.hpp"
#include "maui/controls/items/groupable_items_view.hpp"
#include "maui/controls/items/items_layout_orientation.hpp"
#include "maui/controls/items/items_view_source.hpp"
#include "maui/controls/items/structured_items_view.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/templates/data_template_selector.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_maui_context.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    using maui::platform::android::app_context;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;
    using maui::platform::android::to_jstring;

    constexpr const char* k_scroll_view_class = "android/widget/ScrollView";
    // The inner content host: a ViewGroup whose onLayout is a no-op (the CV frames children absolutely) and
    // whose onMeasure reports the CONTENT extent so it sizes correctly inside the ScrollView's UNSPECIFIED
    // measure (java/MauiCollectionContent.java — see that file for why it is NOT the plain MauiLayout).
    constexpr const char* k_host_class = "dev/mauicpp/MauiCollectionContent";
    constexpr const char* k_text_view_class = "android/widget/TextView";
    constexpr const char* k_view_class = "android/view/View";
    constexpr const char* k_view_group_class = "android/view/ViewGroup";
    constexpr const char* k_layout_params_class = "android/view/ViewGroup$LayoutParams";
    // ScrollView IS-A FrameLayout, and FrameLayout.measureChildWithMargins casts its child's LayoutParams to
    // MarginLayoutParams — so the host (the scroller's single document child) MUST carry FrameLayout
    // .LayoutParams (a MarginLayoutParams subclass), not a bare ViewGroup.LayoutParams (a ClassCastException
    // otherwise). The host's OWN children use the plain k_layout_params_class (MauiCollectionContent does not
    // require margins).
    constexpr const char* k_frame_params_class = "android/widget/FrameLayout$LayoutParams";
    constexpr const char* k_measure_spec_class = "android/view/View$MeasureSpec";

    constexpr jint k_match_parent = -1; // ViewGroup.LayoutParams.MATCH_PARENT
    constexpr jint k_wrap_content = -2; // ViewGroup.LayoutParams.WRAP_CONTENT
    // android.view.View.MeasureSpec modes.
    constexpr auto k_measure_spec_exactly = static_cast<jint>(0x40000000U);
    constexpr auto k_measure_spec_unspecified = static_cast<jint>(0x00000000U);
    // android.view.Gravity.CENTER (== CENTER_HORIZONTAL | CENTER_VERTICAL == 0x01 | 0x10 == 0x11). Applied to
    // the empty-view TextView so its text centers within the viewport-sized bounds add_and_frame gives it —
    // mirroring MAUI's SimpleViewHolder.FromText fill path, which wraps the text in a
    // Label{HorizontalOptions=Center, VerticalOptions=Center} filling the available space.
    constexpr jint k_gravity_center = 0x11;
    // GeometryUtil.Epsilon — ContextExtensions.ToPixels subtracts it before ceiling.
    constexpr double k_to_pixels_epsilon = 0.0000000001;
    // A modest default supplemental/item extent fallback (dp) for views that measure to nothing (e.g. a
    // realized template whose handler built no measured native view). Keeps a row from collapsing to 0.
    constexpr double k_min_row_extent = 24;

    bool clear_pending(JNIEnv* env)
    {
        if (env->ExceptionCheck() == JNI_FALSE)
        {
            return false;
        }
        env->ExceptionDescribe(); // logcat/stderr breadcrumb, the channel the test host uses
        env->ExceptionClear();
        return true;
    }

    // ContextExtensions.ToPixels: ceil(dp * density - Epsilon).
    [[nodiscard]] jint to_pixels(double dp, float density)
    {
        return static_cast<jint>(std::ceil((dp * static_cast<double>(density)) - k_to_pixels_epsilon));
    }

    // The display density (Context.getResources().getDisplayMetrics().density), memoized process-wide after
    // the first read, exactly like the other android handlers' display_density. 1.0 when any step fails.
    [[nodiscard]] float display_density(JNIEnv* env, jobject view)
    {
        static std::atomic<float> memoized{0.0F}; // 0 = not read yet (a real density is never 0)
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
        const local_ref<jobject> context{env, env->CallObjectMethod(view, get_context)};
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

    // Detach `child` from any ViewGroup parent (removeView), so addView never throws "already has a
    // parent" (the re-parent guard the layout/content_page/scroll partials share).
    void detach_from_parent(JNIEnv* env, jobject child)
    {
        auto& cache = default_jni_cache();
        jmethodID get_parent = cache.method(env, k_view_class, "getParent", "()Landroid/view/ViewParent;");
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

    // Construct a plain android.widget.TextView showing `text` (the DefaultCell label / the supplemental
    // text mirror). Theme-independent ctor (TextView(Context)) so it builds in the bare app_process testhost
    // (the LESSON-2 constraint). Returns a local ref (caller adds it to the host / measures it).
    // When `center` is set, apply Gravity.CENTER so the text centers within the view's own (later
    // Exactly-measured) bounds — used for the empty view, which add_and_frame sizes to the whole viewport,
    // mirroring MAUI's SimpleViewHolder.FromText fill path (a Label{HorizontalOptions=Center,
    // VerticalOptions=Center} filling the available space). Default (top-start gravity) leaves a plain
    // wrap-content caption for header/footer/cell text, matching MAUI's FromText(fill:false) bare TextView.
    [[nodiscard]] local_ref<jobject> make_text_view(JNIEnv* env, jobject context, const std::string& text,
                                                    bool center = false)
    {
        auto& cache = default_jni_cache();
        jclass text_view_class = cache.find_class(env, k_text_view_class);
        jmethodID ctor = cache.method(env, k_text_view_class, "<init>", "(Landroid/content/Context;)V");
        jmethodID set_text = cache.method(env, k_text_view_class, "setText", "(Ljava/lang/CharSequence;)V");
        if (text_view_class == nullptr || ctor == nullptr || set_text == nullptr)
        {
            return {};
        }
        local_ref<jobject> view{env, env->NewObject(text_view_class, ctor, context)};
        if (clear_pending(env) || !view)
        {
            return {};
        }
        const local_ref<jstring> text_str = to_jstring(env, text);
        env->CallVoidMethod(view.get(), set_text, text_str.get());
        clear_pending(env);
        if (center)
        {
            if (jmethodID set_gravity = cache.method(env, k_text_view_class, "setGravity", "(I)V");
                set_gravity != nullptr)
            {
                env->CallVoidMethod(view.get(), set_gravity, k_gravity_center);
                clear_pending(env);
            }
        }
        return view;
    }

    // Measure `view` at an exact cross-axis pixel width and an UNSPECIFIED main axis, then read back its
    // measured main extent in pixels (View.getMeasuredHeight for a vertical list, getMeasuredWidth for a
    // horizontal one). Returns 0 on any failure — the caller floors it to a minimum so a row never
    // collapses. `vertical` selects which measured dimension is the main axis.
    [[nodiscard]] jint measure_main_extent(JNIEnv* env, jobject view, jint cross_px, bool vertical)
    {
        auto& cache = default_jni_cache();
        jmethodID make_measure_spec = cache.static_method(env, k_measure_spec_class, "makeMeasureSpec", "(II)I");
        jmethodID measure = cache.method(env, k_view_class, "measure", "(II)V");
        jmethodID get_measured_width = cache.method(env, k_view_class, "getMeasuredWidth", "()I");
        jmethodID get_measured_height = cache.method(env, k_view_class, "getMeasuredHeight", "()I");
        jclass measure_spec_class = cache.find_class(env, k_measure_spec_class);
        if (make_measure_spec == nullptr || measure == nullptr || get_measured_width == nullptr ||
            get_measured_height == nullptr || measure_spec_class == nullptr)
        {
            return 0;
        }
        // Vertical: cross = width (Exactly), main = height (Unspecified). Horizontal: cross = height
        // (Exactly), main = width (Unspecified).
        const jint cross_spec =
            env->CallStaticIntMethod(measure_spec_class, make_measure_spec, cross_px, k_measure_spec_exactly);
        const jint main_spec =
            env->CallStaticIntMethod(measure_spec_class, make_measure_spec, 0, k_measure_spec_unspecified);
        if (clear_pending(env))
        {
            return 0;
        }
        if (vertical)
        {
            env->CallVoidMethod(view, measure, cross_spec, main_spec);
        }
        else
        {
            env->CallVoidMethod(view, measure, main_spec, cross_spec);
        }
        if (clear_pending(env))
        {
            return 0;
        }
        const jint extent =
            vertical ? env->CallIntMethod(view, get_measured_height) : env->CallIntMethod(view, get_measured_width);
        clear_pending(env);
        return extent;
    }

    // Add `child` to `host` with WRAP/WRAP params then layout(l,t,r,b) it ABSOLUTELY in PIXELS — the
    // android container convention (MauiLayout.onLayout is a no-op so this frame survives). Detaches first.
    void add_and_frame(JNIEnv* env, jobject host, jobject child, jint left, jint top, jint right, jint bottom)
    {
        detach_from_parent(env, child);
        auto& cache = default_jni_cache();
        jclass params_class = cache.find_class(env, k_layout_params_class);
        jmethodID params_ctor = cache.method(env, k_layout_params_class, "<init>", "(II)V");
        jmethodID add_view = cache.method(env, k_view_group_class, "addView",
                                          "(Landroid/view/View;Landroid/view/ViewGroup$LayoutParams;)V");
        jmethodID measure = cache.method(env, k_view_class, "measure", "(II)V");
        jmethodID layout = cache.method(env, k_view_class, "layout", "(IIII)V");
        jmethodID make_measure_spec = cache.static_method(env, k_measure_spec_class, "makeMeasureSpec", "(II)I");
        jclass measure_spec_class = cache.find_class(env, k_measure_spec_class);
        if (params_class == nullptr || params_ctor == nullptr || add_view == nullptr || measure == nullptr ||
            layout == nullptr || make_measure_spec == nullptr || measure_spec_class == nullptr)
        {
            return;
        }
        const local_ref<jobject> params{env, env->NewObject(params_class, params_ctor, k_wrap_content, k_wrap_content)};
        if (clear_pending(env) || !params)
        {
            return;
        }
        env->CallVoidMethod(host, add_view, child, params.get());
        if (clear_pending(env))
        {
            return;
        }
        // Measure Exactly at the final frame size (android requires a measure pass before layout), then
        // layout absolutely — the same two-step every leaf android handler's platform_arrange does.
        const jint width_spec =
            env->CallStaticIntMethod(measure_spec_class, make_measure_spec, right - left, k_measure_spec_exactly);
        const jint height_spec =
            env->CallStaticIntMethod(measure_spec_class, make_measure_spec, bottom - top, k_measure_spec_exactly);
        if (clear_pending(env))
        {
            return;
        }
        env->CallVoidMethod(child, measure, width_spec, height_spec);
        if (clear_pending(env))
        {
            return;
        }
        env->CallVoidMethod(child, layout, left, top, right, bottom);
        clear_pending(env);
    }

    // The CollectionView default "Selected" VisualState fill (the C# CommonStates Selected highlight). The
    // per-backend default selection color is NATIVE, not shared: iOS paints the cell's selectedBackgroundView
    // = systemGray while isSelected (src/platform/ios/collection_view_handler.mm), but ANDROID resolves the
    // theme's activated-highlight color — SelectableViewHolder.GetSelectedDrawable() (Handlers/Items/Android/
    // SelectableViewHolder.cs) does exactly this: context.Theme.ResolveAttribute(colorActivatedHighlight, tv,
    // true) → Color.FromUint((uint)tv.Data) → a StateListDrawable keyed on state_activated. On the emulator's
    // DeviceDefault/Material theme that attribute is the translucent accent (orange), NOT gray — the port
    // previously hardcoded iOS systemGray here, so selected cells rendered gray instead of MAUI's orange.
    //
    // We paint that resolved color onto the realized cell's native View via View.setBackgroundColor when the
    // cell's index path is in the handler's selected_paths mirror (kept current by the cross-platform
    // update_platform_selection on every backend). MAUI's StateListDrawable only shows the fill in the
    // activated state; the port paints only when `selected`, so a solid ColorDrawable of the resolved color
    // is the faithful visible result. An UNselected cell gets a TRANSPARENT background, so a re-realized
    // previously-selected cell (arrange_native rebuilds every cell each pass) does not keep a stale highlight.
    // No-op when the View has no setBackgroundColor (never, for an android.view.View) or on any JNI failure.
    constexpr jint k_selected_highlight_fallback_argb =
        static_cast<jint>(0xFF8E8E93U); // gray, only if theme lookup fails
    // android.R.attr.colorActivatedHighlight — the public framework attribute id
    // (Resources.getIdentifier("colorActivatedHighlight","attr","android") == 0x01010390 on API 24-34; the
    // same id GetSelectedDrawable resolves). On the emulator's DeviceDefault theme this is opaque orange
    // (≈ #FFF17A0A) — the accent-derived activated fill MAUI shows for a selected cell.
    constexpr jint k_attr_color_activated_highlight = 0x01010390;

    // Resolve the theme's activated-highlight color from a cell's Context, mirroring GetSelectedDrawable():
    // getContext().getTheme().resolveAttribute(colorActivatedHighlight, TypedValue, true) → TypedValue.data.
    // Memoized process-wide (the app theme is fixed for a run, exactly like display_density). Returns the gray
    // fallback if any JNI/resolve step fails so a selected cell still gets *a* highlight.
    [[nodiscard]] jint selected_highlight_argb(JNIEnv* env, jobject native)
    {
        static std::atomic<jint> memoized{0}; // 0 (fully transparent) = not resolved yet — a real highlight is never 0
        if (const jint cached = memoized.load(std::memory_order_relaxed); cached != 0)
        {
            return cached;
        }
        auto& cache = default_jni_cache();
        jmethodID get_context = cache.method(env, k_view_class, "getContext", "()Landroid/content/Context;");
        jmethodID get_theme =
            cache.method(env, "android/content/Context", "getTheme", "()Landroid/content/res/Resources$Theme;");
        jmethodID resolve = cache.method(env, "android/content/res/Resources$Theme", "resolveAttribute",
                                         "(ILandroid/util/TypedValue;Z)Z");
        jmethodID tv_ctor = cache.method(env, "android/util/TypedValue", "<init>", "()V");
        jfieldID data_field = cache.field(env, "android/util/TypedValue", "data", "I");
        jclass typed_value_class = cache.find_class(env, "android/util/TypedValue");
        if (get_context == nullptr || get_theme == nullptr || resolve == nullptr || tv_ctor == nullptr ||
            data_field == nullptr || typed_value_class == nullptr)
        {
            return k_selected_highlight_fallback_argb;
        }
        const local_ref<jobject> context{env, env->CallObjectMethod(native, get_context)};
        if (clear_pending(env) || !context)
        {
            return k_selected_highlight_fallback_argb;
        }
        const local_ref<jobject> theme{env, env->CallObjectMethod(context.get(), get_theme)};
        if (clear_pending(env) || !theme)
        {
            return k_selected_highlight_fallback_argb;
        }
        const local_ref<jobject> tv{env, env->NewObject(typed_value_class, tv_ctor)};
        if (clear_pending(env) || !tv)
        {
            return k_selected_highlight_fallback_argb;
        }
        const jboolean ok =
            env->CallBooleanMethod(theme.get(), resolve, k_attr_color_activated_highlight, tv.get(), JNI_TRUE);
        if (clear_pending(env) || ok != JNI_TRUE)
        {
            return k_selected_highlight_fallback_argb;
        }
        const jint data = env->GetIntField(tv.get(), data_field);
        if (clear_pending(env) || data == 0)
        {
            return k_selected_highlight_fallback_argb;
        }
        memoized.store(data, std::memory_order_relaxed);
        return data;
    }

    // `content` is the realized cell's virtual view (its retained bindable_object), or nullptr for the
    // text-mirror default cell. In the port's single-root cell reduction the cell ROOT native view IS the
    // content view (e.g. the chat_example bubble Label), so forcing a TRANSPARENT background on the unselected
    // branch would CLOBBER the content's OWN background — the styled-bubble fill the template staged. C#
    // never has this collision because the "Selected" drawable rides on the ItemContentView WRAPPER
    // (SelectableViewHolder: `ItemView.Background = _selectedDrawable`, null when unselected), a distinct view
    // from the content, so the content's own background is untouched. Mirror that here: SELECTED paints the
    // resolved highlight over the cell; UNSELECTED RESTORES the content's own Background paint (via the shared
    // apply_background) instead of zeroing it, then re-installs the content's own Clip outline so the restored
    // fill keeps its rounding (the chat bubble's RoundRectangle). With no content view (default text cell)
    // there is no own-background to restore, so the historical transparent clear stands.
    void apply_selection_highlight(JNIEnv* env, jobject native, bool selected,
                                   const std::shared_ptr<maui::core::bindable_object>& content, float density)
    {
        if (native == nullptr)
        {
            return;
        }
        if (selected)
        {
            jmethodID set_background_color =
                default_jni_cache().method(env, k_view_class, "setBackgroundColor", "(I)V");
            if (set_background_color != nullptr)
            {
                env->CallVoidMethod(native, set_background_color, selected_highlight_argb(env, native));
                clear_pending(env);
            }
            return;
        }
        // Unselected: restore the content's OWN background (nullptr paint clears to transparent inside
        // apply_background — the same visible result as the old transparent clear for a cell with no own
        // fill), then re-apply the content's own convex Clip so the restored fill stays rounded.
        auto* const view = dynamic_cast<maui::core::i_view*>(content.get());
        maui::graphics::paint* const own_background = view != nullptr ? view->background() : nullptr;
        maui::platform::android::apply_background(native, own_background);
        if (view != nullptr)
        {
            if (const maui::graphics::i_shape* const own_clip = view->clip(); own_clip != nullptr)
            {
                const maui::graphics::rect bounds = view->frame();
                maui::platform::android::apply_outline_clip(native, own_clip, density, bounds.width, bounds.height);
            }
        }
    }
} // namespace

namespace maui::controls
{
    namespace
    {
        // Resolve a possibly-selector template against one item (DataTemplateSelector.SelectTemplate; the
        // container is the items view itself, like C# passes the ItemsView). Mirrors the cross-platform
        // resolve_template (kept local — that one is .cpp-internal).
        std::shared_ptr<data_template> resolve_item_template(const std::shared_ptr<data_template>& candidate,
                                                             const boxed_item& item,
                                                             maui::core::bindable_object* container)
        {
            if (auto selector = std::dynamic_pointer_cast<data_template_selector>(candidate))
            {
                return selector->select_template(item.context_box(), container);
            }
            return candidate;
        }

        // Forward declaration: recursively mount a boxed VIEW / template-content subtree (defined below).
        // realize_template_content calls it so a template whose ROOT is a CONTAINER (a photo_cell /
        // chrome_cell stack owning Image + Label children — header_footer_template) hosts those children:
        // set_handler on the root alone attaches only the root's handler, leaving the children's native views
        // unbuilt (the layout_handler's panel then has nothing to host). ensure_mounted walks the realized
        // subtree post-order, attaches each descendant's handler, and re-fires the container host command —
        // the same on-demand mount the boxed-VIEW header/footer path already uses.
        void ensure_mounted(maui::core::i_maui_context* context, maui::controls::element& root);

        // Realize a type-activated template's content into a native android.view.View (the C#
        // TemplatedCell.Bind: CreateContent → set BindingContext → ToPlatform(mauiContext)). Returns the
        // realized content (which OWNS its attached handler + native view — the caller keeps it alive for as
        // long as it is hosted) and, out-param, its native View as a jobject. Yields {nullptr, nullptr} when
        // the template is loader-only (no static control type) or no handler is registered for that type —
        // the caller then falls back to the item-text mirror. Identical to the apple/ios realize path.
        std::shared_ptr<maui::core::bindable_object> realize_template_content(
            collection_view_handler& handler, const std::shared_ptr<data_template>& tmpl, const boxed_item& value,
            jobject* out_native)
        {
            *out_native = nullptr;
            maui::core::i_maui_context* const context = handler.maui_context();
            if (tmpl == nullptr || context == nullptr || !tmpl->content_type().has_value())
            {
                return nullptr;
            }
            std::shared_ptr<maui::core::bindable_object> content = tmpl->create_content();
            if (!content)
            {
                return nullptr;
            }
            // BindingContext = the item (so the template's staged bindings resolve against it). Set BEFORE
            // attaching the handler so the first mapper pass already sees the bound property values.
            content->set_binding_context_box(value.context_box());

            std::shared_ptr<maui::core::i_element_handler> child_handler =
                context->handlers().create_handler(*tmpl->content_type());
            auto* element = dynamic_cast<maui::core::i_element*>(content.get());
            if (!child_handler || element == nullptr)
            {
                return nullptr; // no registered handler (or non-element content) — fall back to text
            }
            child_handler->set_maui_context(context);
            element->set_handler(child_handler); // creates the platform view + runs the mapper

            // If the realized ROOT is a CONTAINER that owns children (a header_footer_template photo_cell /
            // chrome_cell stack of Image + Label), set_handler attached only the ROOT's handler — its children
            // have no native view yet, so the layout_handler's panel hosts nothing. Mount the subtree so each
            // child's handler is attached and re-hosted (the boxed-VIEW header/footer path's on-demand mount).
            // A single-leaf template root (of<label>()) has no logical children, so this is a no-op there.
            if (auto* chrome = dynamic_cast<maui::controls::element*>(content.get()); chrome != nullptr)
            {
                ensure_mounted(context, *chrome);
            }

            if (auto* view_handler = dynamic_cast<maui::core::i_view_handler*>(child_handler.get()))
            {
                *out_native = static_cast<jobject>(view_handler->native_view());
            }
            return content;
        }

        // Recursively MOUNT a boxed VIEW chrome subtree (the generic mount the hosting driver runs, inlined
        // for the seam where the CV must realize a Header/Footer that is a live View). A boxed Header/Footer
        // (boxed_item::of(grid_/stack_)) is NOT a logical child of the CollectionView, so the page-level
        // mount_tree never walks it — its handler (and every descendant's) is unattached, so its native view
        // does not exist. Mirror app_host::mount_tree EXACTLY: depth-first POST-ORDER (children first, so each
        // child's native view exists before its parent hosts it), attach each element's registered handler
        // by its runtime handler_type_tag (SetMauiContext before SetVirtualView, the C# order), then re-fire
        // the container host command (mount_into_handler) so the now-attached children's native views are
        // hosted. Idempotent: an element that already carries a handler is skipped (the gallery may have
        // mounted it through some other path; re-attaching would rebuild + orphan the old native view). This
        // is what makes the VIEW header/footer (HeaderFooterView / HeaderFooterGrid) realize on Android — the
        // analog of iOS reusing the page-attached native_view, but here the CV builds it on demand.
        void ensure_mounted(maui::core::i_maui_context* context, maui::controls::element& root)
        {
            if (context == nullptr)
            {
                return;
            }
            root.visit_logical_children([context](maui::controls::element& child) { ensure_mounted(context, child); });

            auto* element_face = dynamic_cast<maui::core::i_element*>(&root);
            if (element_face == nullptr)
            {
                return;
            }
            if (!element_face->handler()) // skip an already-mounted element (idempotent re-mount guard)
            {
                if (const std::optional<maui::core::type_tag> tag = root.handler_type_tag(); tag.has_value())
                {
                    if (std::shared_ptr<maui::core::i_element_handler> handler =
                            context->handlers().create_handler(*tag))
                    {
                        handler->set_maui_context(context);            // SetMauiContext precedes SetVirtualView (C#)
                        element_face->set_handler(std::move(handler)); // the view owns its handler (PROFILE §11)
                    }
                }
            }
            root.mount_into_handler(); // re-host the (now-attached) children's native views
        }

        // Reuse a boxed VIEW's native view (a Header/Footer/EmptyView set to a live View via
        // boxed_item::of(view)). The boxed chrome is not a CV logical child, so unlike the gallery's mounted
        // tree it usually arrives UNMOUNTED here — ensure_mounted builds its whole native subtree first (the
        // C# `Header is View` arm where ToPlatform builds the platform view on demand), then this returns the
        // root handler's native_view(). Yields nullptr when the value is not an element or has no view handler
        // even after mounting (the caller then falls back to the text mirror).
        jobject boxed_view_native(maui::core::i_maui_context* context, const boxed_item& value)
        {
            const std::shared_ptr<maui::core::bindable_object>& bindable = value.as_bindable();
            auto* element_face = dynamic_cast<maui::core::i_element*>(bindable.get());
            if (element_face == nullptr)
            {
                return nullptr;
            }
            if (auto* chrome = dynamic_cast<maui::controls::element*>(bindable.get()); chrome != nullptr)
            {
                ensure_mounted(context, *chrome); // build the native subtree on demand if not already mounted
            }
            if (const std::shared_ptr<maui::core::i_element_handler>& existing = element_face->handler())
            {
                if (auto* view_handler = dynamic_cast<maui::core::i_view_handler*>(existing.get()))
                {
                    return static_cast<jobject>(view_handler->native_view());
                }
            }
            return nullptr;
        }

        // Run the cross-platform layout pass on a realized content view (a boxed Header/Footer Grid/StackLayout
        // OR a templated cell whose root is itself a container — e.g. the nested_collection inner CollectionView)
        // so its CHILDREN / inner cells get framed. add_and_frame only native-measures+lays out the realized
        // ROOT; the deeper tree needs the cross-platform arrange:
        //   - a boxed Grid/StackLayout chrome re-arranges its own children HOST-RELATIVE when its MauiLayout
        //     panel's onLayout fires (the wave-13 nativeArrange seam) — but its children must be MEASURED first
        //     so they carry desired sizes; this runs that measure + a full arrange.
        //   - a nested CollectionView cell root needs its OWN arrange_native to run so its inner cells realize;
        //     the arrange drives it.
        // The arrange MUST use the ABSOLUTE frame add_and_frame placed the native at (the port drives arrange
        // in ABSOLUTE coordinates — arrange coordinate convention note): a 0-origin arrange would re-frame the
        // realized root to (0,0), stacking every placed element at the host's top-left. A container re-positions
        // its OWN children host-relative regardless (via its native onLayout), so the absolute root frame and
        // the host-relative child frames stay consistent. `frame_dp` is that absolute placed rect (dp).
        void arrange_realized_view(const std::shared_ptr<maui::core::bindable_object>& bindable,
                                   const maui::graphics::rect& frame_dp)
        {
            auto* const view = dynamic_cast<maui::core::i_view*>(bindable.get());
            if (view == nullptr)
            {
                return; // a non-view realized content (e.g. a bare string mirror) — nothing to arrange
            }
            view->measure(frame_dp.width, frame_dp.height);
            view->arrange(frame_dp);
        }
    } // namespace

    // ---- creation + teardown ----

    collection_view_platform::~collection_view_platform()
    {
        // Release the retained global refs. The scroll view owns the host MauiLayout (its document child),
        // so releasing the global ref to each is enough; the retained_natives subtrees free their own
        // handlers + native views when the vector clears.
        const scoped_env env; // any-thread teardown, like global_ref::reset
        if (env)
        {
            if (empty_view_native != nullptr)
            {
                env->DeleteGlobalRef(static_cast<jobject>(empty_view_native));
            }
            if (host != nullptr)
            {
                env->DeleteGlobalRef(static_cast<jobject>(host));
            }
            if (scroll != nullptr)
            {
                env->DeleteGlobalRef(static_cast<jobject>(scroll));
            }
        }
        empty_view_native = nullptr;
        host = nullptr;
        scroll = nullptr;
        native = nullptr; // aliases `scroll` (not separately retained)
        retained_natives.clear();
    }

    std::unique_ptr<collection_view_platform> collection_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<collection_view_platform>();
        const scoped_env env;
        jobject context = app_context();
        if (!env || context == nullptr)
        {
            return platform; // VM-less / context-less: the headless-mirror degradation (file header)
        }
        auto& cache = default_jni_cache();
        jclass scroll_class = cache.find_class(env.get(), k_scroll_view_class);
        jmethodID scroll_ctor = cache.method(env.get(), k_scroll_view_class, "<init>", "(Landroid/content/Context;)V");
        jclass layout_class = cache.find_class(env.get(), k_host_class);
        jmethodID layout_ctor = cache.method(env.get(), k_host_class, "<init>", "(Landroid/content/Context;)V");
        if (scroll_class == nullptr || scroll_ctor == nullptr || layout_class == nullptr || layout_ctor == nullptr)
        {
            return platform; // MauiLayout is host-provided (java/MauiLayout.java); without it the headless
                             // mirror stands (the VM-less degradation)
        }
        // The composed native: a ScrollView whose single document child is the MauiLayout host panel (the
        // no-op-onLayout ViewGroup the realized children keep absolute frames inside). The same shape the
        // scroll_view_handler builds, plus the inner host panel the CV positions its cells into.
        const local_ref<jobject> scroller{env.get(), env->NewObject(scroll_class, scroll_ctor, context)};
        if (clear_pending(env.get()) || !scroller)
        {
            return platform;
        }
        const local_ref<jobject> host{env.get(), env->NewObject(layout_class, layout_ctor, context)};
        if (clear_pending(env.get()) || !host)
        {
            return platform;
        }
        // Add the host as the scroller's document child (MATCH_PARENT width, WRAP_CONTENT height so the
        // content overflows vertically and scrolls — the android scroll-content convention). FrameLayout
        // .LayoutParams because ScrollView casts its child's params to MarginLayoutParams (see the constant).
        jclass params_class = cache.find_class(env.get(), k_frame_params_class);
        jmethodID params_ctor = cache.method(env.get(), k_frame_params_class, "<init>", "(II)V");
        jmethodID add_view = cache.method(env.get(), k_view_group_class, "addView",
                                          "(Landroid/view/View;Landroid/view/ViewGroup$LayoutParams;)V");
        if (params_class != nullptr && params_ctor != nullptr && add_view != nullptr)
        {
            const local_ref<jobject> params{env.get(),
                                            env->NewObject(params_class, params_ctor, k_match_parent, k_wrap_content)};
            if (!clear_pending(env.get()) && params)
            {
                env->CallVoidMethod(scroller.get(), add_view, host.get(), params.get());
                clear_pending(env.get());
            }
        }
        platform->scroll = env->NewGlobalRef(scroller.get()); // released in ~collection_view_platform
        platform->host = env->NewGlobalRef(host.get());
        platform->native = platform->scroll; // the composed native (aliases scroll; NOT separately retained)
        return platform;
    }

    // arrange_native — the backend half of platform_arrange (the one hook called on every backend). Frame
    // the ScrollView to the arranged rect, then re-realize the WHOLE content into the host panel and
    // position each element absolutely. The shared platform_arrange re-sets the viewport mirror + re-runs
    // the simulator after this returns; the native render here is independent of the windowing simulator
    // (it realizes the full content, the render-first model).
    void collection_view_handler::arrange_native(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->scroll == nullptr || platform->host == nullptr)
        {
            return; // headless / VM-less: no native tree to render
        }
        const scoped_env env_guard;
        if (!env_guard)
        {
            return;
        }
        JNIEnv* env = env_guard.get();
        auto* const scroll = static_cast<jobject>(platform->scroll);
        auto* const host = static_cast<jobject>(platform->host);
        auto& cache = default_jni_cache();
        const float density = display_density(env, scroll);

        // Drop the previous pass: clear the host's children and release the retained template subtrees (the
        // apple prepareForReuse analog — every realized native is rebuilt fresh this pass).
        jmethodID remove_all = cache.method(env, k_view_group_class, "removeAllViews", "()V");
        if (remove_all != nullptr)
        {
            env->CallVoidMethod(host, remove_all);
            clear_pending(env);
        }
        platform->retained_natives.clear();
        if (platform->empty_view_native != nullptr)
        {
            env->DeleteGlobalRef(static_cast<jobject>(platform->empty_view_native));
            platform->empty_view_native = nullptr;
        }

        // The mapped layout mirrors (orientation / span) drive the flow; the cross-platform refresh_layout_
        // mirrors keeps them current from the items_layout.
        const bool vertical = platform->orientation == items_layout_orientation::vertical;
        const int span = std::max(1, platform->span);
        const double cross_extent_dp = vertical ? frame.width : frame.height;
        const double main_viewport_dp = vertical ? frame.height : frame.width;
        const jint cross_px = std::max<jint>(1, to_pixels(cross_extent_dp, density));
        // The per-column cross extent (cross_px / span) is computed inside the items region as item_col_px,
        // where a HORIZONTAL CV further reduces it by the top header band (see item_cross_origin_px below).

        auto* view = virtual_view();
        const std::shared_ptr<i_items_view_source>& src = items_view_source();
        auto* container = dynamic_cast<maui::core::bindable_object*>(view);

        // The total content extent along the main axis (dp), accumulated as elements are placed; the inner
        // host panel is sized to it so the ScrollView has the right scrollable extent (the host's onMeasure
        // does NOT measure children, so the panel must be framed EXPLICITLY here — the same reason every
        // child gets an absolute frame).
        double content_main_dp = 0;
        double cursor_dp = 0; // main-axis position in dp (header → items/empty → footer all advance it)

        // HORIZONTAL header/footer band model (the port's no-horizontal-scroll deviation): MAUI's Android
        // GridLayoutSpanSizeLookup gives a Header/Footer the FULL span (it always occupies the entire cross
        // axis — GridLayoutSpanSizeLookup.GetSpanSize returns GridItemsLayout.Span for a header/footer of ANY
        // orientation), and in MAUI's rendered horizontal frame the header shows as a FULL-WIDTH band with the
        // items flowing beside it. Without real horizontal scrolling the port cannot pin a full-height header
        // to the left while items scroll past it, so the wave-27 one-column bound instead squeezed the header
        // into a narrow left column with 3-line-wrapped text (the header_footer_grid_horizontal red). The
        // faithful match to MAUI's VISIBLE frame is to render the horizontal header/footer as full-VIEWPORT-
        // WIDTH bands at the top / bottom (exactly like the vertical case) and flow the item columns in the
        // vertical band BETWEEN them. `band_top_dp` is the vertical offset the top header band consumes (the
        // horizontal items start below it); the bottom footer band is appended after the items. For a VERTICAL
        // CV these are unused (header/footer advance cursor_dp along the main axis as before).
        double band_top_dp = 0;    // vertical extent already consumed by the horizontal top header band
        double band_bottom_dp = 0; // vertical extent consumed by the horizontal bottom footer band
        // The viewport width in dp — the cross extent of a horizontal band (= the vertical CV's cross extent).
        const double viewport_width_dp = frame.width;
        const jint viewport_width_px = std::max<jint>(1, to_pixels(viewport_width_dp, density));

        // The source is empty (UpdateEmptyView) — but the global Header/Footer STILL render around the empty
        // region (the C# `Header is View` arm is independent of item count; HeaderFooterView's source starts
        // EMPTY yet shows its View header+footer). So the header/footer realization below runs UNCONDITIONALLY;
        // only the ITEMS region is swapped for the empty view.
        const bool empty = view == nullptr || !src || src->item_count() == 0;

        // Place a realized supplemental (header/footer) native FULL VIEWPORT WIDTH and advance the flow. On
        // BOTH orientations a header/footer spans the whole cross axis (MAUI's Android GridLayoutSpanSizeLookup
        // gives it the full Span for any orientation) — so it is always a full-viewport-width band with its
        // natural height, exactly like the vertical case. For a VERTICAL CV that band advances the main-axis
        // cursor (header at the top of the scroll, footer at the bottom). For a HORIZONTAL CV it advances the
        // vertical band offset instead (`is_footer` selects the bottom band), and the item COLUMNS flow in the
        // vertical space between the two bands (band_top_dp .. viewport_height - band_bottom_dp). This mirrors
        // MAUI's rendered horizontal frame (a full-width header band, items beside/below it) — the port cannot
        // pin a full-height header while items scroll horizontally past it (no horizontal scroll; the wave-27
        // narrow-column bound that squeezed the header is removed). Measures the realized MAUI view first (a
        // boxed StackLayout header takes its true height incl. children), falling back to / max-with the native
        // View.measure; then places, arranges the children over the placed rect, and returns the band height.
        auto place_full_width = [&](jobject child, const std::shared_ptr<maui::core::bindable_object>& realized,
                                    bool is_footer) -> double {
            if (child == nullptr)
            {
                return 0.0;
            }
            // The band's height (dp): measure the content at the full viewport width with an UNBOUNDED height
            // (a header stack takes its natural height), then max with the native measure and the min floor.
            double height_dp = 0.0;
            if (auto* const v = dynamic_cast<maui::core::i_view*>(realized.get()); v != nullptr)
            {
                height_dp = v->measure(viewport_width_dp, std::numeric_limits<double>::infinity()).height;
            }
            jint height_px = to_pixels(height_dp, density);
            height_px =
                std::max<jint>(height_px, measure_main_extent(env, child, viewport_width_px, /*vertical=*/true));
            height_px = std::max<jint>(height_px, to_pixels(k_min_row_extent, density));
            const double band_height_dp = static_cast<double>(height_px) / static_cast<double>(density);
            // The band's top (dp): a VERTICAL CV places header/footer inline along the main-axis cursor; a
            // HORIZONTAL CV places the header at the very top (band_top_dp) and the footer at the very bottom
            // (below the viewport-tall items region), the two vertical bands the item columns sit between.
            double top_dp = 0.0;
            if (vertical)
            {
                top_dp = cursor_dp;
            }
            else if (is_footer)
            {
                top_dp = std::max(main_viewport_dp, band_top_dp) + band_bottom_dp; // stack below the items band
            }
            else
            {
                top_dp = band_top_dp; // the top header band grows downward
            }
            const jint top_px = to_pixels(top_dp, density);
            add_and_frame(env, host, child, 0, top_px, viewport_width_px, top_px + height_px);
            // Frame the realized view's CHILDREN (header Image/Label/Buttons) via the cross-platform arrange
            // over the placed rect — add_and_frame only laid out the realized ROOT.
            arrange_realized_view(realized, maui::graphics::rect{0.0, top_dp, viewport_width_dp, band_height_dp});
            if (vertical)
            {
                cursor_dp += band_height_dp;
            }
            else if (is_footer)
            {
                band_bottom_dp += band_height_dp;
            }
            else
            {
                band_top_dp += band_height_dp;
            }
            return band_height_dp;
        };

        // Realize a supplemental (header/footer/group header/footer): template content > boxed view >
        // text mirror, hosted full cross-width. Returns nothing — appends to the host + retains.
        auto realize_supplemental_native = [&](const std::shared_ptr<data_template>& tmpl, const boxed_item& value,
                                               bool is_footer) {
            if (!tmpl && !value.has_value())
            {
                return; // nothing to show
            }
            jobject native = nullptr;
            std::shared_ptr<maui::core::bindable_object> realized =
                realize_template_content(*this, tmpl, value, &native);
            if (native == nullptr)
            {
                // A boxed VIEW (HeaderFooterView/Grid): mount its subtree on demand, then host its native view.
                native = boxed_view_native(maui_context(), value);
                if (native != nullptr)
                {
                    realized = value.as_bindable(); // arrange this boxed view's children via place_full_width
                }
            }
            local_ref<jobject> text_view;
            if (native == nullptr && value.has_value())
            {
                jobject context = app_context();
                if (context != nullptr)
                {
                    text_view = make_text_view(env, context, value.text());
                    native = text_view.get();
                }
            }
            place_full_width(native, realized, is_footer);
            if (realized && realized != value.as_bindable())
            {
                platform->retained_natives.push_back(std::move(realized));
            }
        };

        // ── CarouselView paged path (wave 25): show ONLY the current item, full-viewport ──────────────
        // A carousel reuses this handler but PAGES (one item per page). When the virtual view is a
        // carousel_view, realize ONLY the item at the clamped current Position and frame it filling the whole
        // viewport — NOT the horizontal CV's all-items flow. This early branch fully handles the items region
        // (and a carousel carries no header/footer/empty/group chrome in the gallery), so it skips the
        // header/items/footer flow below via is_carousel and falls straight through to the host-sizing tail.
        const bool is_carousel = dynamic_cast<carousel_view*>(view) != nullptr;
        if (is_carousel && !empty)
        {
            auto* carousel = dynamic_cast<carousel_view*>(view);
            const int item_count = src->item_count();
            // CarouselView.Position clamped into [0, count-1] (the settled page; a fresh carousel is at 0).
            int position = carousel != nullptr ? carousel->position() : 0;
            position = std::clamp(position, 0, item_count - 1);
            const boxed_item value = src->item(index_path{.section = 0, .item = position});
            const std::shared_ptr<data_template> item_t = view->item_template();
            const std::shared_ptr<data_template> resolved =
                item_t ? resolve_item_template(item_t, value, container) : nullptr;

            jobject native = nullptr;
            std::shared_ptr<maui::core::bindable_object> realized =
                realize_template_content(*this, resolved, value, &native);
            local_ref<jobject> text_view;
            if (native == nullptr)
            {
                jobject context = app_context();
                if (context != nullptr)
                {
                    text_view = make_text_view(env, context, value.text());
                    native = text_view.get();
                }
            }
            if (native != nullptr)
            {
                // Frame the single current item filling the WHOLE viewport rect (the C# CreateCarouselLayout's
                // FractionalWidth(1)/FractionalHeight(1) item — one item per page). The page IS the viewport,
                // so the cell spans the full frame in BOTH axes regardless of carousel orientation (width =
                // frame.width, height = frame.height). The templated Label centers its own text (the gallery's
                // cell stages center text alignment), so a page-sized cell reads as a big centered caption.
                const jint w = std::max<jint>(1, to_pixels(frame.width, density));
                const jint h = std::max<jint>(1, to_pixels(frame.height, density));
                add_and_frame(env, host, native, 0, 0, w, h);
                // Frame the realized cell's CHILDREN (the templated Label) via the cross-platform arrange over
                // the page rect — add_and_frame only laid out the realized ROOT.
                arrange_realized_view(realized, maui::graphics::rect{0.0, 0.0, frame.width, frame.height});
                // Advance the content cursor by the viewport's MAIN extent so the host panel sizes to one page.
                cursor_dp += main_viewport_dp;
                if (realized && realized != value.as_bindable())
                {
                    platform->retained_natives.push_back(std::move(realized));
                }
            }
        }

        // The global (structured) header — realized BEFORE the items/empty region (and independent of it).
        auto* structured = dynamic_cast<structured_items_view*>(view);
        if (!is_carousel && view != nullptr && structured != nullptr)
        {
            realize_supplemental_native(structured->header_template(), structured->header(), /*is_footer=*/false);
        }

        if (is_carousel)
        {
            // The carousel paged path above already realized the single current item; skip the CV's
            // header/items/empty/footer flow entirely (a carousel carries none of that chrome).
        }
        else if (empty)
        {
            // ---- the empty view region (UpdateEmptyView): centered between the header and footer ----
            // Only reserve the viewport-height empty region when an empty view is actually SET (a template, a
            // boxed View, or a non-empty text value). HeaderFooterView's source starts empty with NO empty view
            // — in that case the footer must follow the header directly (the C# UpdateEmptyView shows nothing),
            // not be pushed a full viewport down by a blank placeholder.
            jobject context = app_context();
            const bool has_empty_view =
                view != nullptr && (view->empty_view_template() != nullptr || view->empty_view().has_value());
            if (context != nullptr && view != nullptr && has_empty_view)
            {
                jobject empty_native = nullptr;
                std::shared_ptr<maui::core::bindable_object> realized =
                    realize_template_content(*this, view->empty_view_template(), view->empty_view(), &empty_native);
                if (empty_native == nullptr)
                {
                    empty_native = boxed_view_native(maui_context(), view->empty_view());
                    if (empty_native != nullptr)
                    {
                        realized = view->empty_view().as_bindable();
                    }
                }
                local_ref<jobject> text_view; // keep alive until added
                if (empty_native == nullptr)
                {
                    // Gravity.CENTER so the placeholder text centers h+v within the viewport-sized bounds below,
                    // matching MAUI's SimpleViewHolder.FromText fill path (Label centered in the fill container).
                    text_view = make_text_view(env, context, view->empty_view().text(), /*center=*/true);
                    empty_native = text_view.get();
                }
                if (empty_native != nullptr)
                {
                    // Center the empty view filling the viewport (the C# EmptyView centered host).
                    const jint w = cross_px;
                    const jint h = std::max<jint>(1, to_pixels(main_viewport_dp, density));
                    const jint start_px = to_pixels(cursor_dp, density);
                    add_and_frame(env, host, empty_native, 0, start_px, w, start_px + h);
                    arrange_realized_view(realized,
                                          maui::graphics::rect{0.0, cursor_dp, cross_extent_dp, main_viewport_dp});
                    cursor_dp += main_viewport_dp;
                    if (realized && realized != view->empty_view().as_bindable())
                    {
                        platform->retained_natives.push_back(std::move(realized));
                    }
                    // Retain a global ref so the destructor can release a reused boxed/template native that
                    // outlives this local frame (the text-view local is owned by the host after addView).
                    if (!text_view)
                    {
                        platform->empty_view_native = env->NewGlobalRef(empty_native);
                    }
                }
            }
        }
        else
        {
            // ---- the realized items: [group header → items → group footer]* ----
            const auto* groupable = dynamic_cast<const groupable_items_view*>(view);
            const bool grouped = platform->grouped;
            const std::shared_ptr<data_template> group_header_t =
                groupable != nullptr ? groupable->group_header_template() : nullptr;
            const std::shared_ptr<data_template> group_footer_t =
                groupable != nullptr ? groupable->group_footer_template() : nullptr;
            const std::shared_ptr<data_template> item_t = view->item_template();

            // HORIZONTAL item band: the item COLUMNS flow in the vertical space BELOW the top header band (a
            // full-width band consumed band_top_dp). So the columns' cross axis (vertical) starts at band_top
            // and spans the remaining viewport height, divided by the grid span. (A vertical CV is unaffected:
            // its columns span the full width from the top, item_cross_origin_px = 0 and item_band_cross_px =
            // cross_px, so item_col_px == cross_px / span, the prior behavior.)
            const jint item_cross_origin_px = vertical ? 0 : to_pixels(band_top_dp, density);
            const jint item_band_cross_px = vertical ? cross_px : std::max<jint>(1, cross_px - item_cross_origin_px);
            const jint item_col_px = std::max<jint>(1, item_band_cross_px / span);

            const int sections = src->group_count();
            for (int section = 0; section < sections; ++section)
            {
                if (grouped && group_header_t)
                {
                    // A group header/footer only occurs in a (vertical) grouped list — it advances the main
                    // cursor inline (is_footer=false keeps it on the vertical flow, not a horizontal band).
                    realize_supplemental_native(group_header_t, src->group(index_path{.section = section, .item = -1}),
                                                /*is_footer=*/false);
                }
                const int count = src->item_count_in_group(section);
                // Lay items across `span` columns; each row's height is the max measured of its columns.
                for (int first = 0; first < count; first += span)
                {
                    const int row_n = std::min(span, count - first);
                    jint row_extent_px = to_pixels(k_min_row_extent, density);
                    // Realize the row's columns, measure each, then place them across the cross axis at the
                    // shared row extent.
                    struct realized_col
                    {
                        jobject native = nullptr;
                        std::shared_ptr<maui::core::bindable_object> retain;
                        local_ref<jobject> text_view;
                    };
                    std::vector<realized_col> cols;
                    cols.reserve(static_cast<std::size_t>(row_n));
                    for (int c = 0; c < row_n; ++c)
                    {
                        const index_path path{.section = section, .item = first + c};
                        const boxed_item value = src->item(path);
                        realized_col col;
                        const std::shared_ptr<data_template> resolved =
                            item_t ? resolve_item_template(item_t, value, container) : nullptr;
                        col.retain = realize_template_content(*this, resolved, value, &col.native);
                        if (col.native == nullptr)
                        {
                            jobject context = app_context();
                            if (context != nullptr)
                            {
                                col.text_view = make_text_view(env, context, value.text());
                                col.native = col.text_view.get();
                            }
                        }
                        if (col.native != nullptr)
                        {
                            // Main extent: prefer the realized cell's CROSS-PLATFORM measure (it includes the
                            // cell's children — the whole point of a templated row), falling back to / taking
                            // the max with the native View.measure. This mirrors MAUI's Android
                            // ItemContentView.OnMeasure, which delegates to View.Measure(w,h) (the virtual
                            // view's cross-platform measure) rather than a naked ViewGroup measure — and the
                            // port's own place_full_width supplemental path, which already measures the realized
                            // MAUI view first. WITHOUT this a templated cell whose root is a MauiLayout collapses:
                            // MauiLayout.onMeasure resolveSize(0, UNSPECIFIED) == 0, so the native measure returns
                            // 0, every row floors to k_min_row_extent, and adjacent rows OVERLAP (the Android
                            // collectionview / nested_collection parity red).
                            const double col_cross_dp = static_cast<double>(item_col_px) / static_cast<double>(density);
                            if (auto* const cell_view = dynamic_cast<maui::core::i_view*>(col.retain.get());
                                cell_view != nullptr)
                            {
                                const maui::graphics::size desired =
                                    vertical
                                        ? cell_view->measure(col_cross_dp, std::numeric_limits<double>::infinity())
                                        : cell_view->measure(std::numeric_limits<double>::infinity(), col_cross_dp);
                                const double desired_main_dp = vertical ? desired.height : desired.width;
                                row_extent_px = std::max(row_extent_px, to_pixels(desired_main_dp, density));
                            }
                            row_extent_px =
                                std::max(row_extent_px, measure_main_extent(env, col.native, item_col_px, vertical));
                        }
                        cols.push_back(std::move(col));
                    }
                    const jint row_start_px = to_pixels(cursor_dp, density);
                    for (int c = 0; c < static_cast<int>(cols.size()); ++c)
                    {
                        if (cols[static_cast<std::size_t>(c)].native == nullptr)
                        {
                            continue;
                        }
                        // The cross-axis origin of this column: horizontal offsets it below the top header band
                        // (item_cross_origin_px), vertical starts at 0. Column width is item_col_px (the band
                        // cross-extent / span).
                        const jint col_start = item_cross_origin_px + static_cast<jint>(c) * item_col_px;
                        const double col_main_dp = static_cast<double>(row_extent_px) / static_cast<double>(density);
                        const double col_cross_dp = static_cast<double>(item_col_px) / static_cast<double>(density);
                        const double col_cross_origin_dp =
                            static_cast<double>(col_start) / static_cast<double>(density);
                        if (vertical)
                        {
                            add_and_frame(env, host, cols[static_cast<std::size_t>(c)].native, col_start, row_start_px,
                                          col_start + item_col_px, row_start_px + row_extent_px);
                        }
                        else
                        {
                            add_and_frame(env, host, cols[static_cast<std::size_t>(c)].native, row_start_px, col_start,
                                          row_start_px + row_extent_px, col_start + item_col_px);
                        }
                        // The "Selected" VisualState fill (the C# CommonStates Selected highlight; iOS shows
                        // the cell's selectedBackgroundView while isSelected). This realized cell is selected
                        // iff its index path is in the cross-platform selected_paths mirror (kept current by
                        // update_platform_selection on every backend); a selected cell gets the resolved
                        // highlight fill, an unselected cell RESTORES its own content Background (not a blind
                        // transparent clear — which would clobber a styled single-root cell like the
                        // chat_example bubble Label; see apply_selection_highlight). The retained content view
                        // (col.retain) carries that own Background + Clip.
                        const index_path cell_path{.section = section, .item = first + c};
                        const bool selected =
                            std::ranges::find(platform->selected_paths, cell_path) != platform->selected_paths.end();
                        apply_selection_highlight(env, cols[static_cast<std::size_t>(c)].native, selected,
                                                  cols[static_cast<std::size_t>(c)].retain, density);
                        // Frame the cell's CHILDREN / inner cells via the cross-platform arrange at the cell's
                        // ABSOLUTE placed rect — a templated cell whose root is itself a CollectionView (the
                        // nested_collection inner CV) needs its own arrange_native to run so its inner cells
                        // realize. The arrange uses the SAME absolute rect add_and_frame placed the native at
                        // (arrange coordinate convention) so a leaf cell stays in place and a nested CV's
                        // ScrollView lands where it was framed.
                        if (cols[static_cast<std::size_t>(c)].retain)
                        {
                            const maui::graphics::rect cell_rect =
                                vertical
                                    ? maui::graphics::rect{col_cross_origin_dp, cursor_dp, col_cross_dp, col_main_dp}
                                    : maui::graphics::rect{cursor_dp, col_cross_origin_dp, col_main_dp, col_cross_dp};
                            arrange_realized_view(cols[static_cast<std::size_t>(c)].retain, cell_rect);
                            platform->retained_natives.push_back(std::move(cols[static_cast<std::size_t>(c)].retain));
                        }
                    }
                    cursor_dp += static_cast<double>(row_extent_px) / static_cast<double>(density);
                }
                if (grouped && group_footer_t)
                {
                    realize_supplemental_native(group_footer_t, src->group(index_path{.section = section, .item = -1}),
                                                /*is_footer=*/false);
                }
            }
        }

        // The global (structured) footer — realized AFTER the items/empty region (and, like the header,
        // independent of item count, so HeaderFooterView's empty source still shows its View footer + the
        // Add/Clear buttons). A carousel carries no footer (its paged path realized only the current item).
        if (!is_carousel && view != nullptr && structured != nullptr)
        {
            realize_supplemental_native(structured->footer_template(), structured->footer(), /*is_footer=*/true);
        }
        content_main_dp = cursor_dp;

        // ---- size the inner host panel to the content (EXPLICIT pixel LayoutParams + measure + layout) ----
        // MauiLayout.onMeasure does NOT measure children (the no-op-onLayout container contract): given the
        // UNSPECIFIED height spec a ScrollView passes its single document child, resolveSize(0, …) returns
        // ZERO — so the host (and every realized child) would collapse and clip. The fix is to give the host
        // EXPLICIT PIXEL LayoutParams (height = content extent on the scroll axis, not WRAP_CONTENT): the
        // ScrollView then measures the child with an EXACTLY spec for that fixed size, MauiLayout.onMeasure's
        // resolveSize returns it, and the scroller gets a real scrollable extent the absolute child frames
        // land inside. Sized to at least the viewport so a short list still fills the scroller.
        const double host_main_dp = std::max(content_main_dp, main_viewport_dp);
        const jint host_main_px = std::max<jint>(1, to_pixels(host_main_dp, density));
        const jint host_w = vertical ? k_match_parent : host_main_px;
        const jint host_h = vertical ? host_main_px : k_match_parent;
        {
            // FrameLayout.LayoutParams (a MarginLayoutParams) — ScrollView casts its child's params to that.
            jclass params_class = cache.find_class(env, k_frame_params_class);
            jmethodID params_ctor = cache.method(env, k_frame_params_class, "<init>", "(II)V");
            jmethodID set_params =
                cache.method(env, k_view_class, "setLayoutParams", "(Landroid/view/ViewGroup$LayoutParams;)V");
            if (params_class != nullptr && params_ctor != nullptr && set_params != nullptr)
            {
                const local_ref<jobject> params{env, env->NewObject(params_class, params_ctor, host_w, host_h)};
                if (!clear_pending(env) && params)
                {
                    env->CallVoidMethod(host, set_params, params.get());
                    clear_pending(env);
                }
            }
            // Also measure + layout the host at the explicit pixel size so its bounds are correct on the
            // first pass even before the ScrollView's own measure pass runs (the two-step layout convention).
            const jint host_main_full_px = host_main_px;
            const jint full_w = vertical ? cross_px : host_main_full_px;
            const jint full_h = vertical ? host_main_full_px : cross_px;
            jmethodID host_measure = cache.method(env, k_host_class, "measure", "(II)V");
            jmethodID host_layout = cache.method(env, k_host_class, "layout", "(IIII)V");
            jmethodID host_make_spec = cache.static_method(env, k_measure_spec_class, "makeMeasureSpec", "(II)I");
            jclass spec_class = cache.find_class(env, k_measure_spec_class);
            if (host_measure != nullptr && host_layout != nullptr && host_make_spec != nullptr && spec_class != nullptr)
            {
                const jint w_spec =
                    env->CallStaticIntMethod(spec_class, host_make_spec, full_w, k_measure_spec_exactly);
                const jint h_spec =
                    env->CallStaticIntMethod(spec_class, host_make_spec, full_h, k_measure_spec_exactly);
                if (!clear_pending(env))
                {
                    env->CallVoidMethod(host, host_measure, w_spec, h_spec);
                    clear_pending(env);
                    env->CallVoidMethod(host, host_layout, 0, 0, full_w, full_h);
                    clear_pending(env);
                }
            }
        }

        // ---- frame the ScrollView to the arranged rect (measure Exactly + layout) ----
        jmethodID make_measure_spec = cache.static_method(env, k_measure_spec_class, "makeMeasureSpec", "(II)I");
        jmethodID measure = cache.method(env, k_scroll_view_class, "measure", "(II)V");
        jmethodID layout = cache.method(env, k_scroll_view_class, "layout", "(IIII)V");
        jclass measure_spec_class = cache.find_class(env, k_measure_spec_class);
        if (make_measure_spec == nullptr || measure == nullptr || layout == nullptr || measure_spec_class == nullptr)
        {
            return;
        }
        const jint left = to_pixels(frame.x, density);
        const jint top = to_pixels(frame.y, density);
        const jint width = to_pixels(frame.width, density);
        const jint height = to_pixels(frame.height, density);
        const jint width_spec =
            env->CallStaticIntMethod(measure_spec_class, make_measure_spec, width, k_measure_spec_exactly);
        const jint height_spec =
            env->CallStaticIntMethod(measure_spec_class, make_measure_spec, height, k_measure_spec_exactly);
        if (clear_pending(env))
        {
            return;
        }
        env->CallVoidMethod(scroll, measure, width_spec, height_spec);
        if (clear_pending(env))
        {
            return;
        }
        env->CallVoidMethod(scroll, layout, left, top, left + width, top + height);
        clear_pending(env);
    }

    // native_content_size (Android) — the MEASURE-pass content extent, the Android twin of the iOS
    // .mm's collectionViewContentSize read. C# ItemsViewHandler.Android.cs GetDesiredSize sizes the CV to
    // its RecyclerView content; the port's Android render (arrange_native) lays cells at their MEASURED
    // heights, but that runs during ARRANGE — AFTER the parent VerticalStackLayout's MEASURE already asked
    // get_desired_size, which without this hook falls back to the flat item_extent=100/row estimate and
    // over-reports a text list's height, so the parent leaves a large empty gap below the CV
    // (cv_visual_states / items / multiple_bound_selection).
    //
    // Instead of caching arrange's result (too late in the cycle), this reproduces MAUI's DEFAULT
    // ItemSizingStrategy.MeasureFirstItem here in the measure pass: realize + measure ONE representative
    // cell (the first item) OFF-TREE against the per-column cross constraint to get its uniform main-axis
    // extent, then total = header + footer + Σ_sections( groupHeader + rowCount*cellExtent + groupFooter ),
    // rowCount = ceil(itemCount / span). The realized measurement cells are NEVER added to the host (only
    // arrange_native hosts) — realize_template_content builds them off-tree and the cross-platform
    // i_view::measure is pure C++, so the visible list is untouched. Returns nullopt (flat estimate stands)
    // whenever nothing can be measured yet: no native tree / no maui_context (the VM-less cross-platform
    // suite — headless-identical), an empty source, or the horizontal band model (whose main extent the
    // measure-first-item shape does not confidently reproduce — arrange_native still renders it).
    std::optional<maui::graphics::size> collection_view_handler::native_content_size(double width_constraint,
                                                                                     double height_constraint)
    {
        auto* platform = typed_platform_view();
        // No native tree (headless / VM-less android cross-platform suite) → keep the flat estimate so the
        // vertical_list_measurement contract (3 rows × 100 = 300) is byte-identical to headless.
        if (platform == nullptr || platform->scroll == nullptr || platform->host == nullptr)
        {
            return std::nullopt;
        }
        auto* view = virtual_view();
        const std::shared_ptr<i_items_view_source>& src = items_view_source();
        if (view == nullptr || !src || src->item_count() == 0)
        {
            return std::nullopt; // empty / pre-realization: a real list never reports 0, so the estimate stands
        }

        // A CAROUSEL pages (one item, full viewport) — its extent is exactly the viewport main axis, which
        // the flat estimate does not model; but the carousel's own arrange fills the viewport and the
        // gallery CarouselView sits in a fixed-size slot, so leave it on the estimate (nullopt) rather than
        // risk a wrong number. The MEASURE-FIRST-ITEM shape below targets the plain CollectionView gap.
        if (dynamic_cast<carousel_view*>(view) != nullptr)
        {
            return std::nullopt;
        }

        // Horizontal CVs use arrange_native's full-width-band model (items flow in the vertical space between
        // header/footer bands); the measure-first-item main (width) extent does not map onto that cleanly.
        // Return nullopt so the existing flat estimate stands (unchanged from today) rather than a wrong
        // width — arrange_native still renders horizontal CVs correctly.
        const bool vertical = platform->orientation == items_layout_orientation::vertical;
        if (!vertical)
        {
            return std::nullopt;
        }

        const scoped_env env_guard;
        if (!env_guard)
        {
            return std::nullopt; // no JNI env → cannot native-measure a default text cell; keep the estimate
        }
        JNIEnv* env = env_guard.get();
        auto* const scroll = static_cast<jobject>(platform->scroll);
        const float density = display_density(env, scroll);

        // Resolve the cross-axis geometry the same way arrange_native does: the vertical CV fills the finite
        // width constraint (the cross axis), falling back to the viewport mirror when it is non-finite (the
        // infinite axis a stack passes); each of `span` grid columns gets 1/span of it.
        const int span = std::max(1, platform->span);
        const double cross_extent_dp =
            std::isfinite(width_constraint) ? width_constraint : platform->viewport_cross_extent;
        const double col_cross_dp = std::max(1.0, cross_extent_dp / static_cast<double>(span));
        const jint col_cross_px = std::max<jint>(1, to_pixels(col_cross_dp, density));
        const jint viewport_width_px = std::max<jint>(1, to_pixels(cross_extent_dp, density));
        auto* container = dynamic_cast<maui::core::bindable_object*>(view);

        // Measure one element (template content > default text mirror) OFF-TREE and return its main-axis dp
        // extent, floored to the k_min_row_extent so a row never collapses — the exact per-column measure
        // arrange_native runs, minus the hosting (add_and_frame). `cross_dp`/`cross_px` are the element's
        // cross-axis constraint (a column width for an item, the full viewport width for a supplemental).
        auto measure_element = [&](const std::shared_ptr<data_template>& tmpl, const boxed_item& value, double cross_dp,
                                   jint cross_px) -> double {
            jint main_px = to_pixels(k_min_row_extent, density);
            const std::shared_ptr<data_template> resolved =
                tmpl ? resolve_item_template(tmpl, value, container) : nullptr;
            jobject native = nullptr;
            std::shared_ptr<maui::core::bindable_object> realized =
                realize_template_content(*this, resolved, value, &native);
            if (auto* const v = dynamic_cast<maui::core::i_view*>(realized.get()); v != nullptr)
            {
                // Prefer the realized cell's cross-platform measure (it includes the cell's children — the
                // whole point of a templated row), exactly like arrange_native's per-column measure.
                const maui::graphics::size desired = v->measure(cross_dp, std::numeric_limits<double>::infinity());
                main_px = std::max(main_px, to_pixels(desired.height, density));
            }
            if (native == nullptr && value.has_value())
            {
                if (jobject context = app_context(); context != nullptr)
                {
                    const local_ref<jobject> text_view = make_text_view(env, context, value.text());
                    main_px = std::max(main_px, measure_main_extent(env, text_view.get(), cross_px, /*vertical=*/true));
                }
            }
            else if (native != nullptr)
            {
                main_px = std::max(main_px, measure_main_extent(env, native, cross_px, /*vertical=*/true));
            }
            // `realized` (and its off-tree native view) is dropped here — never hosted, never retained.
            return static_cast<double>(main_px) / static_cast<double>(density);
        };

        double total_dp = 0;

        // CV-level (structured) header/footer bands — full viewport width, independent of item count.
        auto* structured = dynamic_cast<structured_items_view*>(view);
        double footer_dp = 0;
        if (structured != nullptr)
        {
            if (structured->header_template() != nullptr || structured->header().has_value())
            {
                total_dp += measure_element(structured->header_template(), structured->header(), cross_extent_dp,
                                            viewport_width_px);
            }
            if (structured->footer_template() != nullptr || structured->footer().has_value())
            {
                footer_dp = measure_element(structured->footer_template(), structured->footer(), cross_extent_dp,
                                            viewport_width_px);
            }
        }

        // Grouped supplementals: measure the FIRST section's group header/footer once and reuse the extent
        // per section (the measure-first-item uniform assumption), exactly as the item cell is reused.
        const auto* groupable = dynamic_cast<const groupable_items_view*>(view);
        const bool grouped = platform->grouped;
        const std::shared_ptr<data_template> group_header_t =
            grouped && groupable != nullptr ? groupable->group_header_template() : nullptr;
        const std::shared_ptr<data_template> group_footer_t =
            grouped && groupable != nullptr ? groupable->group_footer_template() : nullptr;
        double group_header_dp = 0;
        double group_footer_dp = 0;
        if (group_header_t)
        {
            group_header_dp = measure_element(group_header_t, src->group(index_path{.section = 0, .item = -1}),
                                              cross_extent_dp, viewport_width_px);
        }
        if (group_footer_t)
        {
            group_footer_dp = measure_element(group_footer_t, src->group(index_path{.section = 0, .item = -1}),
                                              cross_extent_dp, viewport_width_px);
        }

        // MEASURE-FIRST-ITEM: measure the very first item ONCE and treat every row as that height (MAUI's
        // default ItemSizingStrategy.MeasureFirstItem). arrange_native lays rows with no inter-row spacing,
        // so the total below adds none either — keeping the reported extent consistent with the render.
        const std::shared_ptr<data_template> item_t = view->item_template();
        const boxed_item first_value = src->item(index_path{.section = 0, .item = 0});
        const double cell_extent_dp = measure_element(item_t, first_value, col_cross_dp, col_cross_px);

        const int sections = src->group_count();
        for (int section = 0; section < sections; ++section)
        {
            total_dp += group_header_dp;
            const int count = src->item_count_in_group(section);
            const int row_count = (count + span - 1) / span; // ceil(count / span) — grid rows this section
            total_dp += static_cast<double>(row_count) * cell_extent_dp;
            total_dp += group_footer_dp;
        }

        total_dp += footer_dp;

        // A real list never sizes to nothing; if the whole measure yielded nothing (every element measured to
        // 0 — e.g. templates whose handlers built no measurable native view), fall back to the flat estimate.
        if (total_dp <= 0)
        {
            return std::nullopt;
        }
        // Return the pre-clamp MAIN (height) extent; the shared get_desired_size clamps it and fills the
        // cross axis. The cross component is informational only (the shared code reads height for vertical).
        return maui::graphics::size{cross_extent_dp, total_dp};
    }
} // namespace maui::controls
