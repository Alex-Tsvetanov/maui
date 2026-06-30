// tabbed_page_handler — Android (JNI) platform partial: a real dev.mauicpp.MauiLayout ViewGroup hosting
// the CURRENT tab page's content plus a bottom TAB BAR (a horizontal android.widget.LinearLayout of one
// android.widget.TextView per tab). The android twin of src/platform/apple/tabbed_page_handler.mm
// (an NSTabViewController) / src/platform/ios/tabbed_page_handler.mm (a UITabBarController) and the
// real-native sibling of the headless tab mirror (src/platform/headless/tabbed_page_handler.cpp).
//
// ──────────────────────────────────────────────────────────────────────────────────────────────────
// WAVE 26: TabbedPage on Android (the gallery page this unblocks)
// ──────────────────────────────────────────────────────────────────────────────────────────────────
// tabbed_flyout (was ⬛ blank) is a flyout_page whose DETAIL pane is a two-tab tabbed_page (Home /
// Settings). The wave-26 Android FlyoutPage partial hosts that detail, but the detail itself is a
// tabbed_page — with no Android TabbedPage partial its native_view() was null, so the flyout host had
// nothing to add and the page stayed blank. This file is that partial: it renders the current tab's
// content (the "This is the Home tab." label) plus a Home/Settings tab bar — exactly the iOS reference.
//
// THE HOST: a plain dev.mauicpp.MauiLayout (the no-op-onLayout ViewGroup the content_page/refresh_view
// hosts use). set_pages re-parents the CURRENT page's native view as a MATCH_PARENT child (filling the
// host; the tabbed_page control's own host-relative arrange — arrange() at {0,0,w,h} — frames the tab
// content within) and builds the tab bar; set_current swaps the visible content child + re-highlights the
// selected tab; update_bar applies the bar/text/selected colors. platform_arrange frames the host EXACTLY
// (measure Exactly + layout) and then explicitly lays out the bar as a bottom strip (MauiLayout.onLayout
// is a no-op for a plain host, so the bar — a real LinearLayout that internally lays out its TextViews —
// must be positioned here).
//
// DOCUMENTED DEVIATIONS from the C# oracle (infrastructure gaps of the AAR-less backend, not behavior
// guesses):
//   - The bar is a plain LinearLayout of TextViews, NOT a Material BottomNavigationView /
//     TabLayout+ViewPager2 (MAUI's TabbedPageManager uses the Material com.google.android.material widgets,
//     a gradle/AAR dependency this backend does not carry — the same constraint button/web_view document).
//     It renders the tab titles + the selected highlight statically; live tab SWITCHING via a tap listener
//     is deferred (the static render shows the current tab — the gallery capture is static). The
//     native→virtual on_tab_selected seam stands ready (a future tap listener calls it), exactly as the
//     apple/ios twins wire their delegates. set_current still swaps the content programmatically so the
//     menu buttons' set_current_page path renders correctly.
//   - ALL tab pages are arranged over the full bounds by tabbed_page::arrange (the controller-insets-the-
//     content convention), but only the CURRENT page is hosted as a child here (a plain ViewGroup cannot
//     page-swipe between them); a tab switch re-hosts. The content fills the host and the bar overlays its
//     bottom strip — matching the iOS reference, whose tab content sits at the top.
//   - The four bar colors are pushed where a stock widget expresses them (bar background → the LinearLayout
//     background; text/selected/unselected → the TextViews' setTextColor); colors the developer never set
//     keep the stock default (the nullopt convention). The BarBackground BRUSH is mirrored (a non-owning
//     aliasing borrow) but only a SOLID fill is painted (gradient bar fills are the deferred half, the
//     label/button update_background precedent).
//
// VM-less degradation (like content_page/refresh_view): every JNI path checks scoped_env / app_context()
// and quietly skips when no Java VM exists, while the headless tab mirror (hosted_pages / tab_titles /
// hosted_current / selected_index / the four colors / the brush borrow) is ALWAYS maintained so the
// pure-native cross-platform suite observes exactly the headless partial's tracking.

#include "maui/core/tabbed_page_handler.hpp"

#include <jni.h>

#include <atomic>
#include <cmath>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "android_semantics_ops.hpp"
#include "android_view_ops.hpp"
#include "android_visual_ops.hpp"
#include "jni/app_context.hpp"
#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/controls/brushes/brush.hpp"
#include "maui/core/flow_direction.hpp"
#include "maui/core/i_tabbed_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
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
    constexpr const char* k_view_class = "android/view/View";
    constexpr const char* k_view_group_class = "android/view/ViewGroup";
    constexpr const char* k_layout_params_class = "android/view/ViewGroup$LayoutParams";
    constexpr const char* k_linear_layout_class = "android/widget/LinearLayout";
    constexpr const char* k_linear_params_class = "android/widget/LinearLayout$LayoutParams";
    constexpr const char* k_text_view_class = "android/widget/TextView";
    constexpr const char* k_measure_spec_class = "android/view/View$MeasureSpec";

    constexpr jint k_view_visible = 0;
    constexpr jint k_view_invisible = 4;
    constexpr jint k_view_gone = 8;
    constexpr jint k_important_for_accessibility_auto = 0;
    constexpr auto k_measure_spec_exactly = static_cast<jint>(0x40000000U);
    constexpr jint k_match_parent = -1;     // ViewGroup.LayoutParams.MATCH_PARENT
    constexpr jint k_wrap_content = -2;     // ViewGroup.LayoutParams.WRAP_CONTENT
    constexpr jint k_linear_horizontal = 0; // LinearLayout.HORIZONTAL
    constexpr jint k_gravity_center = 17;   // android.view.Gravity.CENTER (CENTER_VERTICAL|CENTER_HORIZONTAL)
    constexpr jint k_complex_unit_sp = 2;   // android.util.TypedValue.COMPLEX_UNIT_SP
    constexpr double k_to_pixels_epsilon = 0.0000000001;

    // The tab-bar strip height in dp (an iOS-like bottom tab bar — UITabBar is ~49pt + the safe-area inset;
    // ~56dp is the Android BottomNavigationView default). The static render shows the titles + the selected
    // highlight at the bottom, matching the reference.
    constexpr double k_tab_bar_height_dp = 56.0;
    // The tab-bar text size in sp (BottomNavigationView's active label is ~14sp).
    constexpr float k_tab_text_size_sp = 14.0F;
    // Default tab text colors when the developer set none: a near-black unselected, a system-blue selected.
    constexpr jint k_default_unselected_argb = static_cast<jint>(0xFF333333U);
    constexpr jint k_default_selected_argb = static_cast<jint>(0xFF007AFFU);
    // The default bar background when the developer set none: a light gray strip (the iOS UITabBar look).
    constexpr jint k_default_bar_bg_argb = static_cast<jint>(0xFFF8F8F8U);

    [[nodiscard]] jobject host_of(const maui::core::tabbed_page_platform& platform) noexcept
    {
        return static_cast<jobject>(platform.native);
    }

    bool clear_pending(JNIEnv* env)
    {
        if (env->ExceptionCheck() == JNI_FALSE)
        {
            return false;
        }
        env->ExceptionDescribe();
        env->ExceptionClear();
        return true;
    }

    void call_void_int(JNIEnv* env, jobject host, const char* klass, const char* name, jint value)
    {
        if (jmethodID method = default_jni_cache().method(env, klass, name, "(I)V"))
        {
            env->CallVoidMethod(host, method, value);
            clear_pending(env);
        }
    }

    [[nodiscard]] jint to_pixels(double dp, float density)
    {
        return static_cast<jint>(std::ceil((dp * static_cast<double>(density)) - k_to_pixels_epsilon));
    }

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

    [[nodiscard]] jobject native_child(maui::core::i_view& page)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(page.handler().get());
        if (handler == nullptr)
        {
            return nullptr;
        }
        return static_cast<jobject>(handler->native_view());
    }

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

    // Add `child` to `host` (a MauiLayout / ViewGroup) with the given LayoutParams (w,h in px constants or
    // MATCH_PARENT/WRAP_CONTENT), re-parenting it first.
    void add_child(JNIEnv* env, jobject host, jobject child, jint width, jint height)
    {
        detach_from_parent(env, child);
        auto& cache = default_jni_cache();
        jclass params_class = cache.find_class(env, k_layout_params_class);
        jmethodID params_ctor = cache.method(env, k_layout_params_class, "<init>", "(II)V");
        jmethodID add_view = cache.method(env, k_view_group_class, "addView",
                                          "(Landroid/view/View;Landroid/view/ViewGroup$LayoutParams;)V");
        if (params_class == nullptr || params_ctor == nullptr || add_view == nullptr)
        {
            return;
        }
        const local_ref<jobject> params{env, env->NewObject(params_class, params_ctor, width, height)};
        if (clear_pending(env) || !params)
        {
            return;
        }
        env->CallVoidMethod(host, add_view, child, params.get());
        clear_pending(env);
    }

    // Build one tab TextView: title, centered, fixed text size, the selected/unselected color, with
    // equal-weight LinearLayout.LayoutParams (0-width + weight 1) so the tabs split the bar evenly. Returns
    // a global ref (the caller owns it, releasing it when the bar is rebuilt); nullptr on failure.
    [[nodiscard]] jobject make_tab_text(JNIEnv* env, jobject context, const std::string& title, bool selected,
                                        jint selected_argb, jint unselected_argb)
    {
        auto& cache = default_jni_cache();
        jclass text_class = cache.find_class(env, k_text_view_class);
        jmethodID text_ctor = cache.method(env, k_text_view_class, "<init>", "(Landroid/content/Context;)V");
        if (text_class == nullptr || text_ctor == nullptr)
        {
            return nullptr;
        }
        const local_ref<jobject> text{env, env->NewObject(text_class, text_ctor, context)};
        if (clear_pending(env) || !text)
        {
            return nullptr;
        }
        // Title.
        if (jmethodID set_text = cache.method(env, k_text_view_class, "setText", "(Ljava/lang/CharSequence;)V"))
        {
            const local_ref<jstring> jtitle = to_jstring(env, title);
            env->CallVoidMethod(text.get(), set_text, jtitle.get());
            clear_pending(env);
        }
        // Center the label within its cell.
        call_void_int(env, text.get(), k_text_view_class, "setGravity", k_gravity_center);
        // Fixed text size (sp).
        if (jmethodID set_size = cache.method(env, k_text_view_class, "setTextSize", "(IF)V"))
        {
            env->CallVoidMethod(text.get(), set_size, k_complex_unit_sp, static_cast<jfloat>(k_tab_text_size_sp));
            clear_pending(env);
        }
        // The selected/unselected color.
        call_void_int(env, text.get(), k_text_view_class, "setTextColor", selected ? selected_argb : unselected_argb);
        // Equal-weight LinearLayout.LayoutParams(0, MATCH_PARENT, weight=1) so the tabs split the bar.
        jclass lp_class = cache.find_class(env, k_linear_params_class);
        jmethodID lp_ctor = cache.method(env, k_linear_params_class, "<init>", "(IIF)V");
        jmethodID set_lp =
            cache.method(env, k_view_class, "setLayoutParams", "(Landroid/view/ViewGroup$LayoutParams;)V");
        if (lp_class != nullptr && lp_ctor != nullptr && set_lp != nullptr)
        {
            const local_ref<jobject> lp{env, env->NewObject(lp_class, lp_ctor, 0, k_match_parent, 1.0F)};
            if (!clear_pending(env) && lp)
            {
                env->CallVoidMethod(text.get(), set_lp, lp.get());
                clear_pending(env);
            }
        }
        jobject global = env->NewGlobalRef(text.get());
        return global;
    }
} // namespace

namespace maui::core
{
    // Releases the global references pinning the host + the bar + each tab TextView (the JNI shape of the
    // pimpl-owned-native-view doctrine). The hosted tab pages are owned by their own page handlers
    // (non-owning children) — nothing to release for those.
    tabbed_page_platform::~tabbed_page_platform()
    {
        const scoped_env env; // any-thread teardown, exactly like global_ref::reset
        if (env)
        {
            for (void* const tab : tab_views)
            {
                if (tab != nullptr)
                {
                    env->DeleteGlobalRef(static_cast<jobject>(tab));
                }
            }
            if (tab_bar != nullptr)
            {
                env->DeleteGlobalRef(static_cast<jobject>(tab_bar));
            }
            if (native != nullptr)
            {
                env->DeleteGlobalRef(static_cast<jobject>(native));
            }
        }
        tab_views.clear();
        tab_bar = nullptr;
        native = nullptr;
    }

    // ---- the generic-IView property pushes (the shared view_mapper calls these through
    // view_platform_base). Base body FIRST (the VM-less suite observes the headless mirror), then the real
    // host (the content_page dual-path pattern). ----

    void tabbed_page_platform::update_visibility(maui::core::visibility value)
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
        call_void_int(env.get(), host_of(*this), k_maui_layout_class, "setVisibility", state);
    }

    void tabbed_page_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        if (jmethodID method = default_jni_cache().method(env.get(), k_maui_layout_class, "setAlpha", "(F)V"))
        {
            env->CallVoidMethod(host_of(*this), method, static_cast<jfloat>(value));
            clear_pending(env.get());
        }
    }

    void tabbed_page_platform::update_automation_id(std::string_view value)
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
        jmethodID get_important = cache.method(env.get(), k_maui_layout_class, "getImportantForAccessibility", "()I");
        jmethodID set_description =
            cache.method(env.get(), k_maui_layout_class, "setContentDescription", "(Ljava/lang/CharSequence;)V");
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
            call_void_int(env.get(), host, k_maui_layout_class, "setImportantForAccessibility",
                          k_important_for_accessibility_auto);
        }
    }

    void tabbed_page_platform::update_transform(const maui::core::transform_spec& value)
    {
        view_platform_base::update_transform(value);
        maui::platform::android::apply_transform(native, value);
    }

    void tabbed_page_platform::update_flow_direction(maui::core::flow_direction value)
    {
        view_platform_base::update_flow_direction(value);
        maui::platform::android::apply_flow_direction(native, value);
    }

    void tabbed_page_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        maui::platform::android::apply_background(native, value);
    }

    void tabbed_page_platform::update_semantics(const maui::core::semantics* value)
    {
        view_platform_base::update_semantics(value);
        maui::platform::android::apply_semantics(native, value);
    }

    std::unique_ptr<tabbed_page_platform> tabbed_page_handler::create_platform_view()
    {
        auto platform = std::make_unique<tabbed_page_platform>();
        const scoped_env env;
        jobject context = app_context();
        if (!env || context == nullptr)
        {
            return platform; // VM-less / context-less: the headless-mirror degradation (header note)
        }
        auto& cache = default_jni_cache();
        jclass layout_class = cache.find_class(env.get(), k_maui_layout_class);
        jmethodID ctor = cache.method(env.get(), k_maui_layout_class, "<init>", "(Landroid/content/Context;)V");
        if (layout_class == nullptr || ctor == nullptr)
        {
            return platform;
        }
        const local_ref<jobject> host{env.get(), env->NewObject(layout_class, ctor, context)};
        if (clear_pending(env.get()) || !host)
        {
            return platform;
        }
        platform->native = env->NewGlobalRef(host.get()); // released in ~tabbed_page_platform
        return platform;
    }

    void tabbed_page_handler::on_connect_handler(tabbed_page_platform& /*platform*/)
    {
        // No native tab chrome with a selection callback to wire (the live tap listener is the documented
        // deviation — header); the native→virtual selection sync is exercised through
        // i_tabbed_view::on_tab_selected directly in the unit tests, like the headless twin.
    }

    void tabbed_page_handler::set_pages(i_view& view)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const auto* tabbed = dynamic_cast<i_tabbed_view*>(&view);
        if (tabbed == nullptr)
        {
            return;
        }
        // The headless mirror is ALWAYS maintained (C# MapItemsSource reads the pages + titles).
        platform->hosted_pages = tabbed->tabbed_pages();
        platform->tab_titles = tabbed->tabbed_titles();

        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject context = app_context();
        if (context == nullptr)
        {
            return;
        }
        jobject host = host_of(*platform);
        auto& cache = default_jni_cache();

        // Clear the old host children (the previous content + bar) and release the old tab TextViews.
        if (jmethodID remove_all = cache.method(env.get(), k_view_group_class, "removeAllViews", "()V"))
        {
            env->CallVoidMethod(host, remove_all);
            clear_pending(env.get());
        }
        for (void* const tab : platform->tab_views)
        {
            if (tab != nullptr)
            {
                env->DeleteGlobalRef(static_cast<jobject>(tab));
            }
        }
        platform->tab_views.clear();
        if (platform->tab_bar != nullptr)
        {
            env->DeleteGlobalRef(static_cast<jobject>(platform->tab_bar));
            platform->tab_bar = nullptr;
        }

        // (1) Build the bar (a horizontal LinearLayout) + one centered TextView per tab, then realize the
        //     selection + colors via set_current / update_bar (called by the mapper after set_pages).
        jclass linear_class = cache.find_class(env.get(), k_linear_layout_class);
        jmethodID linear_ctor =
            cache.method(env.get(), k_linear_layout_class, "<init>", "(Landroid/content/Context;)V");
        if (linear_class != nullptr && linear_ctor != nullptr && !platform->hosted_pages.empty())
        {
            const local_ref<jobject> bar{env.get(), env->NewObject(linear_class, linear_ctor, context)};
            if (!clear_pending(env.get()) && bar)
            {
                if (jmethodID set_orientation =
                        cache.method(env.get(), k_linear_layout_class, "setOrientation", "(I)V"))
                {
                    env->CallVoidMethod(bar.get(), set_orientation, k_linear_horizontal);
                    clear_pending(env.get());
                }
                // A bar background so the strip reads as a tab bar (overridden by update_bar when the
                // developer set BarBackgroundColor).
                call_void_int(env.get(), bar.get(), k_view_class, "setBackgroundColor", k_default_bar_bg_argb);

                for (std::size_t i = 0; i < platform->tab_titles.size(); ++i)
                {
                    const bool selected = std::cmp_equal(static_cast<int>(i), platform->selected_index) ||
                                          (platform->selected_index < 0 && i == 0);
                    jobject tab = make_tab_text(env.get(), context, platform->tab_titles[i], selected,
                                                k_default_selected_argb, k_default_unselected_argb);
                    if (tab != nullptr)
                    {
                        // addView (already carries equal-weight LinearLayout.LayoutParams from make_tab_text).
                        if (jmethodID add_view =
                                cache.method(env.get(), k_view_group_class, "addView", "(Landroid/view/View;)V"))
                        {
                            env->CallVoidMethod(bar.get(), add_view, tab);
                            clear_pending(env.get());
                        }
                        platform->tab_views.push_back(tab); // owns the global ref
                    }
                }
                platform->tab_bar = env->NewGlobalRef(bar.get()); // released in ~tabbed_page_platform / next rebuild
            }
        }

        // (2) Host the current page's content + add the bar on top, then apply selection + bar colors.
        set_current(view);
        update_bar(view);
    }

    void tabbed_page_handler::set_current(i_view& view)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const auto* tabbed = dynamic_cast<i_tabbed_view*>(&view);
        if (tabbed == nullptr)
        {
            return;
        }
        // The headless mirror is ALWAYS maintained (C# MapCurrentPage).
        platform->hosted_current = tabbed->tabbed_current_page();
        platform->selected_index = -1;
        for (std::size_t i = 0; i < platform->hosted_pages.size(); ++i)
        {
            if (platform->hosted_pages[i] == platform->hosted_current)
            {
                platform->selected_index = static_cast<int>(i);
                break;
            }
        }

        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject host = host_of(*platform);
        auto& cache = default_jni_cache();

        // Remove only the CONTENT child (everything except the bar), then add the current page's native
        // view filling the host, then re-add the bar on top (the navigation_handler modal-overlay pattern).
        auto* bar = static_cast<jobject>(platform->tab_bar);
        jmethodID get_child_count = cache.method(env.get(), k_view_group_class, "getChildCount", "()I");
        jmethodID get_child_at = cache.method(env.get(), k_view_group_class, "getChildAt", "(I)Landroid/view/View;");
        jmethodID remove_view_at = cache.method(env.get(), k_view_group_class, "removeViewAt", "(I)V");
        if (get_child_count != nullptr && get_child_at != nullptr && remove_view_at != nullptr)
        {
            const jint count = env->CallIntMethod(host, get_child_count);
            if (!clear_pending(env.get()))
            {
                for (jint i = count - 1; i >= 0; --i)
                {
                    const local_ref<jobject> child{env.get(), env->CallObjectMethod(host, get_child_at, i)};
                    if (clear_pending(env.get()) || !child)
                    {
                        continue;
                    }
                    if (bar == nullptr || env->IsSameObject(child.get(), bar) == JNI_FALSE)
                    {
                        env->CallVoidMethod(host, remove_view_at, i);
                        clear_pending(env.get());
                    }
                }
            }
        }
        if (platform->hosted_current != nullptr)
        {
            if (jobject child = native_child(*platform->hosted_current))
            {
                add_child(env.get(), host, child, k_match_parent, k_match_parent);
            }
        }
        if (bar != nullptr)
        {
            // The bar is laid out as a bottom strip by platform_arrange; add it at WRAP_CONTENT (its frame is
            // set explicitly there). Re-adding keeps it on top of the content child.
            add_child(env.get(), host, bar, k_match_parent, k_wrap_content);
        }

        // Re-highlight the selected tab text (the colors are re-applied here so a tab switch recolors).
        update_bar(view);
    }

    void tabbed_page_handler::update_bar(i_view& view)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const auto* tabbed = dynamic_cast<i_tabbed_view*>(&view);
        if (tabbed == nullptr)
        {
            return;
        }
        // The headless mirror is ALWAYS maintained (C# MapBar* keys).
        platform->bar_background_color = tabbed->tab_bar_background_color();
        platform->bar_text_color = tabbed->tab_bar_text_color();
        platform->selected_tab_color = tabbed->tab_selected_color();
        platform->unselected_tab_color = tabbed->tab_unselected_color();
        const std::optional<maui::controls::brush*> brush = tabbed->tab_bar_background_brush();
        platform->bar_background_brush =
            (brush.has_value() && *brush != nullptr)
                ? std::optional{std::shared_ptr<maui::controls::brush>{std::shared_ptr<void>{}, *brush}}
                : std::nullopt;

        if (platform->native == nullptr || platform->tab_bar == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        auto& cache = default_jni_cache();

        // The resolved colors: developer-set wins, else the stock defaults. selected_tab_color falls back to
        // bar_text_color, then the system-blue default; unselected falls back to bar_text_color, then the
        // dark default (C#'s SetTabIconColors / BottomNavigationView item color convention, collapsed).
        const jint selected_argb =
            platform->selected_tab_color.has_value() ? static_cast<jint>(platform->selected_tab_color->to_int())
            : platform->bar_text_color.has_value()   ? static_cast<jint>(platform->bar_text_color->to_int())
                                                     : k_default_selected_argb;
        const jint unselected_argb =
            platform->unselected_tab_color.has_value() ? static_cast<jint>(platform->unselected_tab_color->to_int())
            : platform->bar_text_color.has_value()     ? static_cast<jint>(platform->bar_text_color->to_int())
                                                       : k_default_unselected_argb;

        // The bar background: BarBackgroundColor wins, else the SOLID BarBackground brush, else the default.
        jint bar_bg_argb = k_default_bar_bg_argb;
        if (platform->bar_background_color.has_value())
        {
            bar_bg_argb = static_cast<jint>(platform->bar_background_color->to_int());
        }
        else if (platform->bar_background_brush.has_value() && *platform->bar_background_brush != nullptr)
        {
            // Only a solid fill is painted (gradient bar fills are the deferred half — header). The brush's
            // resolved background color stands in for a SolidColorBrush.
            if (const auto* paint = dynamic_cast<const maui::graphics::paint*>(platform->bar_background_brush->get()))
            {
                bar_bg_argb = static_cast<jint>(paint->background_color().to_int());
            }
        }
        call_void_int(env.get(), static_cast<jobject>(platform->tab_bar), k_view_class, "setBackgroundColor",
                      bar_bg_argb);

        // Recolor each tab text by its selected state.
        for (std::size_t i = 0; i < platform->tab_views.size(); ++i)
        {
            const bool selected = std::cmp_equal(static_cast<int>(i), platform->selected_index) ||
                                  (platform->selected_index < 0 && i == 0);
            if (jmethodID set_color = cache.method(env.get(), k_text_view_class, "setTextColor", "(I)V"))
            {
                env->CallVoidMethod(static_cast<jobject>(platform->tab_views[i]), set_color,
                                    selected ? selected_argb : unselected_argb);
                clear_pending(env.get());
            }
        }
    }

    maui::graphics::size tabbed_page_handler::get_desired_size(double /*width_constraint*/,
                                                               double /*height_constraint*/) const
    {
        // The tabbed page sizes from its current page, not the handler, so it reports nothing here.
        return {0, 0};
    }

    void tabbed_page_handler::platform_arrange(const maui::graphics::rect& frame)
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
        jmethodID make_measure_spec = cache.static_method(env.get(), k_measure_spec_class, "makeMeasureSpec", "(II)I");
        jmethodID measure = cache.method(env.get(), k_maui_layout_class, "measure", "(II)V");
        jmethodID layout = cache.method(env.get(), k_maui_layout_class, "layout", "(IIII)V");
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
        if (clear_pending(env.get()))
        {
            return;
        }

        // Lay out the tab bar as a bottom strip: MauiLayout.onLayout is a no-op for this plain host, so the
        // bar (a real LinearLayout that internally lays out its equal-weight TextViews) must be measured
        // Exactly at [width × barHeight] and positioned at the bottom of the host (local 0-origin coords —
        // the bar is a subview of the host).
        if (platform->tab_bar == nullptr)
        {
            return;
        }
        auto* bar = static_cast<jobject>(platform->tab_bar);
        jmethodID bar_measure = cache.method(env.get(), k_view_class, "measure", "(II)V");
        jmethodID bar_layout = cache.method(env.get(), k_view_class, "layout", "(IIII)V");
        if (bar_measure == nullptr || bar_layout == nullptr)
        {
            return;
        }
        const jint bar_height = to_pixels(k_tab_bar_height_dp, density);
        const jint bar_top = height - bar_height;
        const jint bar_width_spec =
            env->CallStaticIntMethod(measure_spec_class, make_measure_spec, width, k_measure_spec_exactly);
        const jint bar_height_spec =
            env->CallStaticIntMethod(measure_spec_class, make_measure_spec, bar_height, k_measure_spec_exactly);
        if (clear_pending(env.get()))
        {
            return;
        }
        env->CallVoidMethod(bar, bar_measure, bar_width_spec, bar_height_spec);
        if (clear_pending(env.get()))
        {
            return;
        }
        env->CallVoidMethod(bar, bar_layout, 0, bar_top, width, height);
        clear_pending(env.get());
    }
} // namespace maui::core
