// window_handler — Android (JNI) platform recipe, the W4-34d window half. The managed platform view is a
// REAL android.widget.FrameLayout (held as a JNI global reference in window_platform::native) standing in
// for the Activity's CONTENT VIEW — the root container the window sets its content into. host_content
// removes the previous content child and adds the root page's native android.view.View (reached through
// the page handler's native_view()) as the FrameLayout's single child, exactly as the Apple twin sets the
// NSWindow.contentView and the C# oracle calls Activity.SetContentView(rootView).
//
// Ported from WindowHandler.Android.cs + the Android Activity/Application bootstrap:
//   - C# MapContent: rootManager.Connect(window.Content); Activity.SetContentView(rootManager.RootView).
//     The NavigationRootManager wraps the content in a MauiCoordinatorLayout/MauiToolbar host before
//     SetContentView. The port has no Material AppBar / coordinator-layout AAR (the same gradle/AAR gap
//     the button partial documents), so the "root view" the window sets is the page's own native view
//     placed directly into a plain FrameLayout content host — the library-independent shape of
//     SetContentView(rootView). (DEVIATION, below.)
//   - C# MapTitle: Activity.UpdateTitle(window) → Activity.Title. There is no Activity in the test host;
//     the title is kept in the headless mirror (window_platform::title) so the seam stays observable.
//     (DEVIATION, below.)
//   - C# MapX/Y/Width/Height: a window on Android is the whole Activity (it does not move/resize like a
//     desktop NSWindow), so UpdateX/Y/Width/Height are effective no-ops on the platform; the geometry
//     lives in the window's properties (apply_frame stays a no-op, like the headless twin).
//
// DOCUMENTED DEVIATIONS from the C# oracle (each an infrastructure gap of the APK-less / Activity-less
// test host, not a behavior guess):
//   - The root container is a plain android.widget.FrameLayout, NOT a NavigationRootManager-built
//     MauiCoordinatorLayout (+ MauiToolbar / window-insets listener): that whole AppBar stack is a
//     gradle/AAR dependency this backend does not carry. FrameLayout is the stock android.widget host the
//     window's content lands into — the SetContentView(rootView) shape preserved, the chrome layers
//     skipped (the navigation bar's own chrome is the navigation_page_handler's concern). The content's
//     native view is added with MATCH_PARENT layout params so it fills the content area, as a child of an
//     Activity content view would.
//   - There is NO real Activity in the test host (it bootstraps a themed Context via app_process, not an
//     ActivityThread-launched Activity — see testhost/Bootstrap.java). So: the window's "native window" is
//     the content-view FrameLayout itself (the Activity-content-view stand-in, consistent with W1-18's
//     test-host strategy); Activity.Title / the edge-to-edge system-bar config / GetWindowFrame →
//     FrameChanged have no Activity to drive and stay headless-mirror-only. The window lifecycle
//     (activated / destroying) has no Activity onResume/onDestroy to wire either, so connect/disconnect
//     stay the headless no-ops — the lifecycle is driven directly on the window by the application host.
//
// VM-less degradation: like button_handler.cpp, every JNI path checks scoped_env / app_context() and
// quietly skips when no Java VM exists (the pure-native cross-platform suite runs on the emulator without
// one), while the headless mirrors (title / content_hosted) are ALWAYS maintained so that suite observes
// exactly the headless partial's behavior. The widget test host additionally observes the real FrameLayout.

#include "maui/core/window_handler.hpp"

#include <jni.h>

#include <memory>
#include <string>

#include "jni/app_context.hpp"
#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "maui/core/i_element.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/i_window.hpp"

namespace
{
    using maui::platform::android::app_context;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;

    constexpr const char* k_frame_layout_class = "android/widget/FrameLayout";
    constexpr const char* k_view_group_class = "android/view/ViewGroup";
    constexpr const char* k_layout_params_class = "android/view/ViewGroup$LayoutParams";

    // ViewGroup.LayoutParams.MATCH_PARENT — the content fills the Activity content view.
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
    // page_native_view helper.
    [[nodiscard]] jobject page_native_view(maui::core::i_element& page)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(page.handler().get());
        if (handler == nullptr)
        {
            return nullptr;
        }
        return static_cast<jobject>(handler->native_view());
    }

    // Remove every child currently in the content-view FrameLayout (the previous content). FrameLayout
    // hosts the window's content as its single child; SetContentView replaces it whole.
    void remove_all_children(JNIEnv* env, jobject container)
    {
        jmethodID remove_all = default_jni_cache().method(env, k_view_group_class, "removeAllViews", "()V");
        if (remove_all != nullptr)
        {
            env->CallVoidMethod(container, remove_all);
            clear_pending(env);
        }
    }

    // Add `child` to `container` with MATCH_PARENT/MATCH_PARENT layout params so it fills the content area
    // (the Activity content view sizes its child to the window). Detaches `child` from any current parent
    // first (addView throws IllegalStateException when a view already has one — the same re-parent guard
    // the Apple twin's removeFromSuperview provides).
    void add_filling_child(JNIEnv* env, jobject container, jobject child)
    {
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
    // Releases the global reference pinning the content-view FrameLayout (the JNI shape of the
    // pimpl-owned-native-view doctrine: the Apple twin CFReleases its NSWindow here).
    window_platform::~window_platform()
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

    std::unique_ptr<window_platform> window_handler::create_platform_view()
    {
        auto platform = std::make_unique<window_platform>();
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
        // The Activity content view stand-in: a plain FrameLayout the window sets its content into (the
        // SetContentView(rootView) host — see the header deviations).
        const local_ref<jobject> container{env.get(), env->NewObject(frame_class, ctor, context)};
        if (clear_pending(env.get()) || !container)
        {
            return platform;
        }
        platform->native = env->NewGlobalRef(container.get()); // released in ~window_platform
        return platform;
    }

    // C# ConnectHandler — Android: there is no Activity onResume/onDestroy to wire in the test host, so the
    // lifecycle is driven directly on the window by the application host (header deviation). The content +
    // title are pushed by the property mapper that follows set_virtual_view.
    void window_handler::connect()
    {
    }

    // C# DisconnectHandler — Android: nothing native to tear down (no Activity lifecycle observers).
    void window_handler::disconnect() const
    {
    }

    void window_handler::host_content()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // The headless mirror is ALWAYS maintained (the VM-less cross-platform suite observes it): whether
        // a page with an attached view-handler IS the content (C#'s MapContent sets the root view to the
        // page's platform view).
        platform->content_hosted = false;
        if (window_view_ == nullptr)
        {
            return;
        }
        auto* page = window_view_->content();
        if (page != nullptr && dynamic_cast<i_view_handler*>(page->handler().get()) != nullptr)
        {
            platform->content_hosted = true;
        }

        // The real FrameLayout swap (when a VM + the content-view container exist): remove the previous
        // content and add the page's native View filling the content area. C# SetContentView(rootView).
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
        remove_all_children(env.get(), container);
        if (page == nullptr)
        {
            return; // an empty content host (the previous child was just removed)
        }
        if (jobject child = page_native_view(*page))
        {
            add_filling_child(env.get(), container, child);
        }
    }

    void window_handler::apply_title()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || window_view_ == nullptr)
        {
            return;
        }
        // C# MapTitle → Activity.Title. There is no Activity in the test host, so the title is kept in the
        // headless mirror only (header deviation).
        platform->title = std::string(window_view_->title());
    }

    void window_handler::apply_frame()
    {
        // Android: a window IS the whole Activity — it does not move/resize like a desktop window, so
        // UpdateX/Y/Width/Height are effective no-ops on the platform (the geometry lives in the window's
        // properties). The headless twin is likewise a no-op.
    }

    // --- chrome (W1-11): the toolbar / menu bar / title bar surface through the navigation bar
    // (navigation_page_handler) and desktop-only chrome on Android; the C# oracle materializes none of
    // these on a bare Activity content view. Record the borrows in the portable mirror (so the seam stays
    // observable) and skip the native build — the documented Android no-op. ---
    void window_handler::apply_toolbar(i_toolbar* toolbar) const
    {
        if (auto* platform = typed_platform_view())
        {
            platform->hosted_toolbar = toolbar;
        }
    }

    void window_handler::apply_menu_bar(i_menu_bar* menu_bar) const
    {
        if (auto* platform = typed_platform_view())
        {
            platform->hosted_menu_bar = menu_bar;
        }
    }

    void window_handler::apply_title_bar(i_title_bar* title_bar) const
    {
        if (auto* platform = typed_platform_view())
        {
            platform->hosted_title_bar = title_bar;
        }
    }
} // namespace maui::core
