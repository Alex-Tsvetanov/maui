// navigation_page_handler — Android (JNI) platform recipe, the W4-34d navigation half. The managed
// platform view is a REAL android.widget.FrameLayout (held as a JNI global reference in
// navigation_page_platform::native) standing in for the fragment-host container: host_current swaps the
// FrameLayout's CONTENT child to the navigation stack's current (top-most) page's native android.view.View
// on each push/pop, and host_modal overlays the top modal's native View on top of the whole container
// (removing it when the modal stack empties). The real-native twin of the headless partial.
//
// Ported from NavigationViewHandler.Android.cs + StackNavigationManager.Android.cs (the AndroidX
// Navigation fragment stack):
//   - C# CreatePlatformView inflates Resource.Layout.fragment_backstack → a FragmentContainerView (a
//     FrameLayout subclass) hosting a NavHostFragment. The port has no AndroidX Navigation / Fragment AAR
//     (the same gradle/AAR gap the button + window partials document), so the container is a plain
//     android.widget.FrameLayout and the "current page" is hosted DIRECTLY as its content child, instead
//     of through a NavHostFragment + FragmentNavigator destinations. (DEVIATION, below.)
//   - C# StackNavigationManager.ApplyNavigationRequest reduces every request to "what is the currently
//     VISIBLE page" (CurrentPage = NavigationStack[last]) and swaps the fragment showing it. The
//     library-independent shape — the seam this unit ports — is exactly that: host_current takes the
//     request's top-most page and makes its native View the container's single content child, swapping
//     out the previous one. push, pop, and pop_to_root all funnel here (each just changes which page is
//     top), matching the C# manager's "we only apply changes when the currently visible page changes".
//   - NavigationFinished is reported synchronously by the cross-platform handler after host_current
//     returns (the FrameLayout child swap is synchronous), standing in for the manager's
//     fragment-lifecycle-driven NavigationFinished. The animated flag is mirrored
//     (navigation_page_platform::last_animated); the port runs no fragment transition animation (no
//     Navigation component). (DEVIATION, below.)
//
// DOCUMENTED DEVIATIONS from the C# oracle (each an infrastructure gap of the APK-less / AAR-less test
// host, not a behavior guess):
//   - The container is a plain android.widget.FrameLayout, NOT a FragmentContainerView + NavHostFragment +
//     FragmentNavigator graph: AndroidX Navigation / Fragment is a gradle/AAR dependency this backend does
//     not carry. The current page's native View is added/removed directly as the FrameLayout's content
//     child (with MATCH_PARENT layout params), which is the visible result of a fragment swap — the page's
//     handler + native View survive the swap (the page is non-owning; only its View is re-parented),
//     exactly the C# note that "the Handler/PlatformView associated with the visible IView remain intact".
//   - No fragment transition ANIMATION (enter/exit anim XML): the swap is instant; last_animated is
//     mirrored for parity. The custom navigation BAR (title / back button) is the navigation_page control's
//     chrome and a Material AppBar concern (skipped like the button's Material widget) — host_current
//     mirrors the chrome state into the platform so the seam stays observable, but builds no native bar.
//   - The MODAL overlay is a plain re-parent of the modal page's native View as the FrameLayout's TOP-MOST
//     child (FrameLayout stacks children; the last added is on top), mirroring the Apple twin's full-
//     container NSView overlay. C#'s modal stack lives on the Window's ModalNavigationManager (a separate
//     host); the port keeps it on the navigation_page (the documented controls-layer simplification), so
//     this partial just overlays/clears the modal View on the same container.
//
// VM-less degradation: like button_handler.cpp, every JNI path checks scoped_env / app_context() and
// quietly skips when no Java VM exists (the pure-native cross-platform suite runs on the emulator without
// one), while the headless mirrors (hosted_page / hosted_modal / bar chrome / last_animated) are ALWAYS
// maintained so that suite observes exactly the headless partial's behavior. The widget test host
// additionally observes the real FrameLayout child swap.

#include "maui/core/navigation_page_handler.hpp"

#include <jni.h>

#include <memory>
#include <string>

#include "jni/app_context.hpp"
#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "maui/core/i_stack_navigation.hpp"
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

    constexpr const char* k_frame_layout_class = "android/widget/FrameLayout";
    constexpr const char* k_view_group_class = "android/view/ViewGroup";
    constexpr const char* k_layout_params_class = "android/view/ViewGroup$LayoutParams";

    // ViewGroup.LayoutParams.MATCH_PARENT — the hosted page fills the content area.
    constexpr jint k_match_parent = -1;

    // Clears any pending Java exception (a handler must never leak JNI pending-exception state into the
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

    // The page's native android.view.View, via its view-handler's native_view() (null if the page is
    // unattached or its handler has no native view). native_view() returns the real View global ref the
    // page handler's pimpl owns — not the pimpl pointer platform_view() returns. Mirrors the Apple twin's
    // native_child helper.
    [[nodiscard]] jobject native_child(maui::core::i_view& page)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(page.handler().get());
        if (handler == nullptr)
        {
            return nullptr;
        }
        return static_cast<jobject>(handler->native_view());
    }

    // Detach `view` from any current parent (addView throws IllegalStateException when a view already has
    // one; this is the re-parent guard the Apple twin's removeFromSuperview provides). The parent is a
    // ViewGroup, so removeView resolves through it.
    void remove_from_parent(JNIEnv* env, jobject view)
    {
        auto& cache = default_jni_cache();
        jmethodID get_parent = cache.method(env, "android/view/View", "getParent", "()Landroid/view/ViewParent;");
        if (get_parent == nullptr)
        {
            return;
        }
        const local_ref<jobject> parent{env, env->CallObjectMethod(view, get_parent)};
        if (clear_pending(env) || !parent)
        {
            return;
        }
        jclass view_group_class = cache.find_class(env, k_view_group_class);
        jmethodID remove_view = cache.method(env, k_view_group_class, "removeView", "(Landroid/view/View;)V");
        if (view_group_class == nullptr || remove_view == nullptr ||
            env->IsInstanceOf(parent.get(), view_group_class) == JNI_FALSE)
        {
            return;
        }
        env->CallVoidMethod(parent.get(), remove_view, view);
        clear_pending(env);
    }

    // Add `child` to `container` filling it (MATCH_PARENT/MATCH_PARENT), re-parenting it first.
    void add_filling_child(JNIEnv* env, jobject container, jobject child)
    {
        remove_from_parent(env, child);
        auto& cache = default_jni_cache();
        jclass params_class = cache.find_class(env, k_layout_params_class);
        jmethodID params_ctor = cache.method(env, k_layout_params_class, "<init>", "(II)V");
        jmethodID add_view = cache.method(env, k_view_group_class, "addView",
                                          "(Landroid/view/View;Landroid/view/ViewGroup$LayoutParams;)V");
        if (params_class == nullptr || params_ctor == nullptr || add_view == nullptr)
        {
            return;
        }
        const local_ref<jobject> params{env, env->NewObject(params_class, params_ctor, k_match_parent, k_match_parent)};
        if (clear_pending(env) || !params)
        {
            return;
        }
        env->CallVoidMethod(container, add_view, child, params.get());
        clear_pending(env);
    }
} // namespace

namespace maui::core
{
    // Releases the global reference pinning the container FrameLayout (the JNI shape of the
    // pimpl-owned-native-view doctrine: the Apple twin CFReleases its NSView container here). The hosted
    // page / modal Views are owned by their own page handlers (non-owning children) — nothing to release.
    navigation_page_platform::~navigation_page_platform()
    {
        if (native != nullptr)
        {
            const scoped_env env; // any-thread teardown, exactly like global_ref::reset
            if (env)
            {
                env->DeleteGlobalRef(static_cast<jobject>(native));
            }
            native = nullptr;
        }
    }

    std::unique_ptr<navigation_page_platform> navigation_page_handler::create_platform_view()
    {
        auto platform = std::make_unique<navigation_page_platform>();
        const scoped_env env;
        jobject context = app_context();
        if (!env || context == nullptr)
        {
            return platform; // VM-less / context-less: the headless-mirror degradation (header note)
        }
        auto& cache = default_jni_cache();
        jclass frame_class = cache.find_class(env.get(), k_frame_layout_class);
        jmethodID ctor = cache.method(env.get(), k_frame_layout_class, "<init>", "(Landroid/content/Context;)V");
        if (frame_class == nullptr || ctor == nullptr)
        {
            return platform;
        }
        // The fragment-host stand-in: a plain FrameLayout the current page's native View is hosted in (the
        // FragmentContainerView shape — see the header deviations).
        const local_ref<jobject> container{env.get(), env->NewObject(frame_class, ctor, context)};
        if (clear_pending(env.get()) || !container)
        {
            return platform;
        }
        platform->native = env->NewGlobalRef(container.get()); // released in ~navigation_page_platform
        return platform;
    }

    void navigation_page_handler::on_connect_handler(navigation_page_platform& /*platform*/)
    {
        // Android has no native bar / back button to wire (the custom-bar chrome is a Material AppBar
        // concern, skipped — header deviations); the back-button routing is exercised through
        // navigation_page::send_back_button_pressed() directly in the unit tests, like the headless twin.
    }

    void navigation_page_handler::host_current(i_view* top, i_view& view, bool animated)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // The headless mirrors are ALWAYS maintained (the VM-less cross-platform suite observes them): the
        // current page + animation flag + the bar chrome state read off the navigation view.
        platform->hosted_page = top;
        platform->last_animated = animated;
        if (auto* navigation = dynamic_cast<i_stack_navigation*>(&view))
        {
            platform->bar_title = std::string(navigation->navigation_bar_title());
            platform->back_button_visible = navigation->navigation_back_button_visible();
            platform->bar_background_color = navigation->navigation_bar_background_color();
            platform->bar_text_color = navigation->navigation_bar_text_color();
            platform->hosted_title_view = navigation->navigation_bar_title_view();
            platform->toolbar_items = navigation->navigation_toolbar_items();
        }

        // The real FrameLayout content swap (when a VM + container exist): remove the previous content
        // child, then add the new current page's native View as the content child. A modal overlay (if
        // present) is the FrameLayout's top-most child and MUST survive a content swap (navigating the
        // underlying stack while a modal covers it) — so re-add it on top after the content swap.
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject container = static_cast<jobject>(platform->native);

        // Remove the previous content child (everything except the modal overlay). The modal overlay, when
        // present, is the modal page's native View — preserve it; remove only the page-content child.
        jobject modal_view = platform->hosted_modal != nullptr ? native_child(*platform->hosted_modal) : nullptr;
        jmethodID get_child_count = default_jni_cache().method(env.get(), k_view_group_class, "getChildCount", "()I");
        jmethodID get_child_at =
            default_jni_cache().method(env.get(), k_view_group_class, "getChildAt", "(I)Landroid/view/View;");
        jmethodID remove_view_at = default_jni_cache().method(env.get(), k_view_group_class, "removeViewAt", "(I)V");
        if (get_child_count != nullptr && get_child_at != nullptr && remove_view_at != nullptr)
        {
            const jint count = env->CallIntMethod(container, get_child_count);
            if (!clear_pending(env.get()))
            {
                // Iterate high→low so removals do not shift the indices still to visit.
                for (jint i = count - 1; i >= 0; --i)
                {
                    const local_ref<jobject> child{env.get(), env->CallObjectMethod(container, get_child_at, i)};
                    if (clear_pending(env.get()) || !child)
                    {
                        continue;
                    }
                    if (modal_view == nullptr || env->IsSameObject(child.get(), modal_view) == JNI_FALSE)
                    {
                        env->CallVoidMethod(container, remove_view_at, i);
                        clear_pending(env.get());
                    }
                }
            }
        }

        if (top != nullptr)
        {
            if (jobject child = native_child(*top))
            {
                // Add below the modal overlay (when present) so the modal stays on top. add_filling_child
                // appends to the end (top of the FrameLayout z-order); re-add the modal afterwards to keep
                // it on top.
                add_filling_child(env.get(), container, child);
            }
        }
        if (modal_view != nullptr)
        {
            add_filling_child(env.get(), container, modal_view); // restore the overlay on top
        }
    }

    void navigation_page_handler::host_modal(i_view* top_modal, bool animated)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // The previous modal (if any) — its native View is the overlay to tear down before presenting the
        // new top modal (or clearing). Read it BEFORE updating the mirror.
        i_view* const previous_modal = platform->hosted_modal;
        platform->hosted_modal = top_modal;
        platform->last_animated = animated;

        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject container = static_cast<jobject>(platform->native);

        // Tear down the previous overlay (clearing or replacing the presented modal): detach the dismissed
        // modal page's native View so the underlying content (never removed) is revealed.
        if (previous_modal != nullptr && previous_modal != top_modal)
        {
            if (jobject old_view = native_child(*previous_modal))
            {
                remove_from_parent(env.get(), old_view);
            }
        }
        if (top_modal == nullptr)
        {
            return; // the modal stack emptied — the underlying content is revealed
        }
        // Overlay the modal's native View as the FrameLayout's TOP-MOST child (added last = top of the
        // z-order), so it covers the bar + content (the Android analog of a presented modal page).
        if (jobject child = native_child(*top_modal))
        {
            add_filling_child(env.get(), container, child);
        }
    }

    maui::graphics::size navigation_page_handler::get_desired_size(double /*width_constraint*/,
                                                                   double /*height_constraint*/) const
    {
        // The navigation page sizes from its current page, not the handler, so it reports nothing here.
        return {0, 0};
    }

    void navigation_page_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject container = static_cast<jobject>(platform->native);
        auto& cache = default_jni_cache();
        // ViewHandler.PlatformArrange (Android): the FrameLayout is laid out at the given frame (in pixels;
        // the frame here is already the resolved device frame the layout system computed). measure Exactly
        // then layout — Android requires a measure pass before layout. The MATCH_PARENT content child is
        // sized by the FrameLayout to fill it.
        jmethodID measure = cache.method(env.get(), "android/view/View", "measure", "(II)V");
        jmethodID layout = cache.method(env.get(), "android/view/View", "layout", "(IIII)V");
        jmethodID make_measure_spec =
            cache.static_method(env.get(), "android/view/View$MeasureSpec", "makeMeasureSpec", "(II)I");
        jclass measure_spec_class = cache.find_class(env.get(), "android/view/View$MeasureSpec");
        if (measure == nullptr || layout == nullptr || make_measure_spec == nullptr || measure_spec_class == nullptr)
        {
            return;
        }
        constexpr auto k_measure_spec_exactly = static_cast<jint>(0x40000000U);
        const jint left = static_cast<jint>(frame.x);
        const jint top = static_cast<jint>(frame.y);
        const jint width = static_cast<jint>(frame.width);
        const jint height = static_cast<jint>(frame.height);
        const jint width_spec =
            env->CallStaticIntMethod(measure_spec_class, make_measure_spec, width, k_measure_spec_exactly);
        const jint height_spec =
            env->CallStaticIntMethod(measure_spec_class, make_measure_spec, height, k_measure_spec_exactly);
        if (clear_pending(env.get()))
        {
            return;
        }
        env->CallVoidMethod(container, measure, width_spec, height_spec);
        if (clear_pending(env.get()))
        {
            return;
        }
        env->CallVoidMethod(container, layout, left, top, left + width, top + height);
        clear_pending(env.get());
    }

    // --- platform configuration (W2-24): the iOSSpecific IsNavigationBarTranslucent push — Android keeps
    // the cross-platform mirror only (an iOS-only knob; nothing native to drive on Android).
    void navigation_page_handler::update_bar_translucent(bool value)
    {
        if (auto* platform = typed_platform_view())
        {
            platform->bar_translucent = value;
        }
    }
} // namespace maui::core
