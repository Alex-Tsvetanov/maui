// Android navigation + window seam tests (W4-34d) — ON the emulator inside the app_process widget test
// host (tools/android-testhost-run.sh): the REAL maui::controls::window / navigation_page controls +
// their handlers drive a REAL android.widget.FrameLayout through the cross-platform handlers' android
// partials (src/platform/android/{window,navigation}_handler.cpp), and every assertion reads the
// FrameLayout's children BACK through JNI — both halves of the seam:
//   window  : set window content → the root content-view FrameLayout hosts the content's native View.
//   nav     : push/pop → the FrameLayout's visible content child swaps to the current page's native View;
//             push/pop modal → the modal's native View overlays as the top-most child / is removed.
//
// A "page" here is a maui::controls::button: the button is the one control whose android partial creates a
// REAL android.view.View (an android.widget.Button — see button_handler.cpp), reachable through its
// handler's native_view(). The content_page handler stays headless on this backend (its native_view() is
// null — W4-34d ports nav + window only, not content_page), so the button stands in as the real-native
// content the container hosts. This exercises the EXACT seam the C# oracle uses: the container hosts
// whatever native View the content's handler provides (WindowHandler.Android SetContentView(rootView);
// StackNavigationManager hosts the current page's PlatformView).
//
// Characterization target: WindowHandler.Android.cs (MapContent → SetContentView) +
// NavigationViewHandler.Android.cs / StackNavigationManager.Android.cs (the FrameLayout/fragment page
// stack), reduced to the library-independent FrameLayout-child shape the partials document.

#include <memory>
#include <string>

#include <gtest/gtest.h>
#include <jni.h>

#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/navigation_page.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/navigation_page_handler.hpp"
#include "maui/core/window_handler.hpp"
#include "testhost/test_host.hpp"

namespace
{
    using maui::controls::button;
    using maui::controls::navigation_page;
    using maui::controls::window;
    using maui::core::button_handler;
    using maui::core::navigation_page_handler;
    using maui::core::window_handler;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;
    using maui::platform::android::testhost::host_context;

    constexpr const char* k_view_group_class = "android/view/ViewGroup";

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

    // A button with its handler attached, so it owns a REAL android.widget.Button (its native_view()) the
    // container can host. Torn down in declaration order (the handler detach precedes the control's death;
    // the platform struct releases the widget's global ref).
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

        // The button's real native android.view.View (the global ref the platform struct owns).
        [[nodiscard]] jobject native() const
        {
            return static_cast<jobject>(handler->native_view());
        }
    };

    // The number of children currently in `container` (an android.view.ViewGroup).
    [[nodiscard]] jint child_count(JNIEnv* env, jobject container)
    {
        jmethodID get_child_count = default_jni_cache().method(env, k_view_group_class, "getChildCount", "()I");
        if (get_child_count == nullptr)
        {
            ADD_FAILURE() << "getChildCount not found";
            return -1;
        }
        const jint count = env->CallIntMethod(container, get_child_count);
        return pending_exception_cleared(env, "getChildCount") ? -1 : count;
    }

    // Whether `view` is currently a child of `container` (identity-compared with IsSameObject).
    [[nodiscard]] bool contains_child(JNIEnv* env, jobject container, jobject view)
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
        const jint count = env->CallIntMethod(container, get_child_count);
        if (pending_exception_cleared(env, "getChildCount"))
        {
            return false;
        }
        for (jint i = 0; i < count; ++i)
        {
            const local_ref<jobject> child{env, env->CallObjectMethod(container, get_child_at, i)};
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

    // The TOP-MOST child of `container` (the last in z-order), or null when empty.
    [[nodiscard]] jobject top_child(JNIEnv* env, jobject container, local_ref<jobject>& out)
    {
        auto& cache = default_jni_cache();
        jmethodID get_child_count = cache.method(env, k_view_group_class, "getChildCount", "()I");
        jmethodID get_child_at = cache.method(env, k_view_group_class, "getChildAt", "(I)Landroid/view/View;");
        if (get_child_count == nullptr || get_child_at == nullptr)
        {
            return nullptr;
        }
        const jint count = env->CallIntMethod(container, get_child_count);
        if (pending_exception_cleared(env, "getChildCount") || count <= 0)
        {
            return nullptr;
        }
        out = local_ref<jobject>{env, env->CallObjectMethod(container, get_child_at, count - 1)};
        return pending_exception_cleared(env, "getChildAt") ? nullptr : out.get();
    }

    [[nodiscard]] jobject window_container(const std::shared_ptr<window_handler>& handler)
    {
        auto* platform = handler->typed_platform_view();
        return platform != nullptr ? static_cast<jobject>(platform->native) : nullptr;
    }

    [[nodiscard]] jobject nav_container(const std::shared_ptr<navigation_page_handler>& handler)
    {
        auto* platform = handler->typed_platform_view();
        return platform != nullptr ? static_cast<jobject>(platform->native) : nullptr;
    }

    // ---- window: content → the root content-view FrameLayout ----

    TEST(android_window, attach_creates_a_real_frame_layout_content_view)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        ASSERT_NE(host_context(), nullptr);
        window win;
        auto handler = std::make_shared<window_handler>();
        win.set_handler(handler);
        jobject container = window_container(handler);
        ASSERT_NE(container, nullptr) << "the android window partial did not create a content-view FrameLayout";
        jclass frame_class = default_jni_cache().find_class(env.get(), "android/widget/FrameLayout");
        ASSERT_NE(frame_class, nullptr);
        EXPECT_EQ(env->IsInstanceOf(container, frame_class), JNI_TRUE);
        EXPECT_EQ(child_count(env.get(), container), 0) << "an empty window hosts no content yet";
    }

    TEST(android_window, set_content_hosts_the_contents_native_view_in_the_root_container)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_button content; // a real android.widget.Button stands in for the page content
        ASSERT_NE(content.native(), nullptr);

        window win;
        win.set_content(content.control);
        auto handler = std::make_shared<window_handler>();
        win.set_handler(handler); // MapContent → SetContentView: the FrameLayout hosts the content's View

        jobject container = window_container(handler);
        ASSERT_NE(container, nullptr);
        EXPECT_EQ(child_count(env.get(), container), 1);
        EXPECT_TRUE(contains_child(env.get(), container, content.native()));
        EXPECT_TRUE(handler->typed_platform_view()->content_hosted);
    }

    TEST(android_window, replacing_content_swaps_the_hosted_native_view)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        attached_button first;
        attached_button second;

        window win;
        win.set_content(first.control);
        auto handler = std::make_shared<window_handler>();
        win.set_handler(handler);
        jobject container = window_container(handler);
        ASSERT_NE(container, nullptr);
        EXPECT_TRUE(contains_child(env.get(), container, first.native()));

        win.set_content(second.control); // the previous content leaves, the new one is hosted
        EXPECT_EQ(child_count(env.get(), container), 1);
        EXPECT_TRUE(contains_child(env.get(), container, second.native()));
        EXPECT_FALSE(contains_child(env.get(), container, first.native()));
    }

    // ---- navigation: push/pop → the FrameLayout's visible content child swaps ----

    TEST(android_navigation, attach_creates_a_real_frame_layout_container)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        navigation_page nav;
        auto handler = std::make_shared<navigation_page_handler>();
        nav.set_handler(handler);
        jobject container = nav_container(handler);
        ASSERT_NE(container, nullptr) << "the android navigation partial did not create a container FrameLayout";
        jclass frame_class = default_jni_cache().find_class(env.get(), "android/widget/FrameLayout");
        ASSERT_NE(frame_class, nullptr);
        EXPECT_EQ(env->IsInstanceOf(container, frame_class), JNI_TRUE);
        EXPECT_EQ(child_count(env.get(), container), 0) << "no content until the first push";
    }

    TEST(android_navigation, host_current_hosts_the_current_pages_native_view)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        navigation_page nav;
        auto handler = std::make_shared<navigation_page_handler>();
        nav.set_handler(handler);
        jobject container = nav_container(handler);
        ASSERT_NE(container, nullptr);

        attached_button root;
        // Drive the handler's content swap directly with a real-native page view (the control's content_page
        // stack has no real native view on this backend — see the file header).
        handler->host_current(&root.control, nav, /*animated=*/false);

        EXPECT_EQ(child_count(env.get(), container), 1);
        EXPECT_TRUE(contains_child(env.get(), container, root.native()));
        EXPECT_EQ(handler->typed_platform_view()->hosted_page, &root.control);
    }

    TEST(android_navigation, push_swaps_the_visible_content_view)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        navigation_page nav;
        auto handler = std::make_shared<navigation_page_handler>();
        nav.set_handler(handler);
        jobject container = nav_container(handler);
        ASSERT_NE(container, nullptr);

        attached_button root;
        attached_button second;

        handler->host_current(&root.control, nav, false);
        EXPECT_TRUE(contains_child(env.get(), container, root.native()));

        handler->host_current(&second.control, nav, false); // the previous page's view leaves
        EXPECT_EQ(child_count(env.get(), container), 1);
        EXPECT_TRUE(contains_child(env.get(), container, second.native()));
        EXPECT_FALSE(contains_child(env.get(), container, root.native()));
    }

    TEST(android_navigation, pop_restores_the_revealed_pages_view)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        navigation_page nav;
        auto handler = std::make_shared<navigation_page_handler>();
        nav.set_handler(handler);
        jobject container = nav_container(handler);
        ASSERT_NE(container, nullptr);

        attached_button root;
        attached_button second;

        handler->host_current(&root.control, nav, false);
        handler->host_current(&second.control, nav, false);
        EXPECT_TRUE(contains_child(env.get(), container, second.native()));

        handler->host_current(&root.control, nav, false); // a pop re-hosts the revealed root
        EXPECT_EQ(child_count(env.get(), container), 1);
        EXPECT_TRUE(contains_child(env.get(), container, root.native()));
        EXPECT_FALSE(contains_child(env.get(), container, second.native()));
    }

    TEST(android_navigation, control_push_pop_tracks_the_hosted_page_mirror)
    {
        // The control level (content_page stack — no real native view on this backend) still drives the
        // request through to the handler: the hosted_page mirror tracks the current page across push/pop.
        const scoped_env env;
        ASSERT_TRUE(env);
        maui::controls::content_page root;
        maui::controls::content_page second;
        navigation_page nav(root);
        auto handler = std::make_shared<navigation_page_handler>();
        nav.set_handler(handler); // re-issues the navigation request → host_current(root)
        EXPECT_EQ(handler->typed_platform_view()->hosted_page, dynamic_cast<maui::core::i_view*>(&root));

        nav.push(second);
        EXPECT_EQ(handler->typed_platform_view()->hosted_page, dynamic_cast<maui::core::i_view*>(&second));

        nav.pop();
        EXPECT_EQ(handler->typed_platform_view()->hosted_page, dynamic_cast<maui::core::i_view*>(&root));
    }

    // ---- navigation: the modal overlay covers the container; clearing reveals the content ----

    TEST(android_navigation, host_modal_overlays_then_clears_on_the_container)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        navigation_page nav;
        auto handler = std::make_shared<navigation_page_handler>();
        nav.set_handler(handler);
        jobject container = nav_container(handler);
        ASSERT_NE(container, nullptr);

        attached_button root;
        attached_button modal;

        handler->host_current(&root.control, nav, false); // host the content
        handler->host_modal(&modal.control, false);       // overlay the modal on top

        EXPECT_EQ(child_count(env.get(), container), 2);
        EXPECT_TRUE(contains_child(env.get(), container, root.native()));  // content still hosted under it
        EXPECT_TRUE(contains_child(env.get(), container, modal.native())); // modal overlaid
        // The modal is the TOP-MOST child (covering the content).
        local_ref<jobject> top;
        ASSERT_NE(top_child(env.get(), container, top), nullptr);
        EXPECT_EQ(env->IsSameObject(top.get(), modal.native()), JNI_TRUE);
        EXPECT_EQ(handler->typed_platform_view()->hosted_modal, &modal.control);

        handler->host_modal(nullptr, false); // the modal stack emptied → the overlay is torn down
        EXPECT_EQ(child_count(env.get(), container), 1);
        EXPECT_FALSE(contains_child(env.get(), container, modal.native()));
        EXPECT_TRUE(contains_child(env.get(), container, root.native())); // content revealed
        EXPECT_EQ(handler->typed_platform_view()->hosted_modal, nullptr);
    }

    TEST(android_navigation, navigating_under_a_modal_keeps_the_overlay_on_top)
    {
        const scoped_env env;
        ASSERT_TRUE(env);
        navigation_page nav;
        auto handler = std::make_shared<navigation_page_handler>();
        nav.set_handler(handler);
        jobject container = nav_container(handler);
        ASSERT_NE(container, nullptr);

        attached_button root;
        attached_button second;
        attached_button modal;

        handler->host_current(&second.control, nav, false);
        handler->host_modal(&modal.control, false);

        // Navigate the underlying stack WHILE the modal is up: host_current re-swaps the content but must
        // NOT tear down the modal overlay, and the modal must stay on top.
        handler->host_current(&root.control, nav, false);
        EXPECT_EQ(child_count(env.get(), container), 2);
        EXPECT_TRUE(contains_child(env.get(), container, root.native()));
        EXPECT_TRUE(contains_child(env.get(), container, modal.native()));
        EXPECT_FALSE(contains_child(env.get(), container, second.native())); // the swapped-out page left
        local_ref<jobject> top;
        ASSERT_NE(top_child(env.get(), container, top), nullptr);
        EXPECT_EQ(env->IsSameObject(top.get(), modal.native()), JNI_TRUE) << "the modal must stay on top";
    }
} // namespace
