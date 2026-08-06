// Android scroll-range tests — ON the emulator inside the app_process widget test host
// (tools/android-testhost-run.sh): a REAL dev.mauicpp.MauiLayout hosting a child taller than the
// viewport, measured the way a REAL android.widget.ScrollView measures its single document child.
//
// The seam under test is one line of Java and it froze every scrolling page on the backend.
// ScrollView.measureChild ALWAYS measures its child with an UNSPECIFIED height spec — that is how the
// content is allowed to exceed the viewport, and it deliberately IGNORES the child's LayoutParams.
// MauiLayout.onMeasure used to answer resolveSize(0, spec), which returns `size` (0) for UNSPECIFIED, so
// the scroller computed scrollRange = max(0, childHeight - viewport) = 0 and refused to scroll. Nothing
// looked wrong at rest: MauiLayout.onLayout does not position its children (the handler frames them
// absolutely), so they kept drawing at their real offsets and were merely clipped at the fold. Only the
// RANGE was wrong, which is invisible to a still comparison and is why this survived the whole board.
//
// Measured on emulator-5554 before the fix, port `clip` page vs MAUI's, same `input swipe 540 936 540 351
// 800`: MAUI moved 2304956 px below the status bar (96.14% of the frame), the port moved 0. The port's
// ScrollView reported scrollable="false" in the accessibility tree; MAUI's inner one reported "true".
//
// Deliberately built from bare framework views rather than from maui::controls::scroll_view: the defect
// lives entirely in the Java host's measure contract, and the framework's own ScrollView is the oracle
// for what that contract has to satisfy. There is no C# counterpart to characterize — MAUI's Android
// content host (ContentViewGroup) measures its children through the cross-platform layout and so never
// had the degenerate case this host's shortcut created.

#include <gtest/gtest.h>
#include <jni.h>

#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "testhost/test_host.hpp"

namespace
{
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;
    using maui::platform::android::testhost::host_context;

    constexpr const char* k_maui_layout_class = "dev/mauicpp/MauiLayout";
    constexpr const char* k_view_class = "android/view/View";
    constexpr const char* k_view_group_class = "android/view/ViewGroup";
    constexpr const char* k_scroll_view_class = "android/widget/ScrollView";
    constexpr const char* k_measure_spec_class = "android/view/View$MeasureSpec";

    // android.view.View.MeasureSpec modes.
    constexpr jint k_unspecified = 0;
    constexpr jint k_exactly = 1 << 30;
    constexpr jint k_at_most = 2 << 30;

    // The viewport and the (taller) content, in pixels. The content must exceed the viewport or there is
    // no scroll range to assert about.
    constexpr jint k_viewport_w = 400;
    constexpr jint k_viewport_h = 1000;
    constexpr jint k_content_h = 5000;

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

    // new <class>(Context) — the one-arg ctor every android.view.View subclass here shares.
    [[nodiscard]] jobject new_view(JNIEnv* env, const char* class_name)
    {
        auto& cache = default_jni_cache();
        jclass klass = cache.find_class(env, class_name);
        if (klass == nullptr)
        {
            ADD_FAILURE() << "class not found: " << class_name;
            return nullptr;
        }
        jmethodID ctor = cache.method(env, class_name, "<init>", "(Landroid/content/Context;)V");
        if (ctor == nullptr)
        {
            ADD_FAILURE() << "Context ctor not found on " << class_name;
            return nullptr;
        }
        jobject view = env->NewObject(klass, ctor, host_context());
        return pending_exception_cleared(env, class_name) ? nullptr : view;
    }

    [[nodiscard]] jint make_spec(JNIEnv* env, jint size, jint mode)
    {
        auto& cache = default_jni_cache();
        jclass klass = cache.find_class(env, k_measure_spec_class);
        jmethodID make = cache.static_method(env, k_measure_spec_class, "makeMeasureSpec", "(II)I");
        if (klass == nullptr || make == nullptr)
        {
            ADD_FAILURE() << "MeasureSpec.makeMeasureSpec not found";
            return 0;
        }
        const jint spec = env->CallStaticIntMethod(klass, make, size, mode);
        return pending_exception_cleared(env, "makeMeasureSpec") ? 0 : spec;
    }

    // view.measure(widthSpec, heightSpec) — resolved on android.view.View so it dispatches virtually to
    // whichever onMeasure the instance actually overrides.
    void measure(JNIEnv* env, jobject view, jint width_spec, jint height_spec)
    {
        jmethodID method = default_jni_cache().method(env, k_view_class, "measure", "(II)V");
        ASSERT_NE(method, nullptr) << "View.measure not found";
        env->CallVoidMethod(view, method, width_spec, height_spec);
        pending_exception_cleared(env, "View.measure");
    }

    void layout(JNIEnv* env, jobject view, jint left, jint top, jint right, jint bottom)
    {
        jmethodID method = default_jni_cache().method(env, k_view_class, "layout", "(IIII)V");
        ASSERT_NE(method, nullptr) << "View.layout not found";
        env->CallVoidMethod(view, method, left, top, right, bottom);
        pending_exception_cleared(env, "View.layout");
    }

    [[nodiscard]] jint measured_height(JNIEnv* env, jobject view)
    {
        jmethodID method = default_jni_cache().method(env, k_view_class, "getMeasuredHeight", "()I");
        if (method == nullptr)
        {
            ADD_FAILURE() << "View.getMeasuredHeight not found";
            return -1;
        }
        const jint height = env->CallIntMethod(view, method);
        return pending_exception_cleared(env, "getMeasuredHeight") ? -1 : height;
    }

    [[nodiscard]] jint measured_width(JNIEnv* env, jobject view)
    {
        jmethodID method = default_jni_cache().method(env, k_view_class, "getMeasuredWidth", "()I");
        if (method == nullptr)
        {
            ADD_FAILURE() << "View.getMeasuredWidth not found";
            return -1;
        }
        const jint width = env->CallIntMethod(view, method);
        return pending_exception_cleared(env, "getMeasuredWidth") ? -1 : width;
    }

    void add_view(JNIEnv* env, jobject group, jobject child)
    {
        jmethodID method = default_jni_cache().method(env, k_view_group_class, "addView", "(Landroid/view/View;)V");
        ASSERT_NE(method, nullptr) << "ViewGroup.addView not found";
        env->CallVoidMethod(group, method, child);
        pending_exception_cleared(env, "ViewGroup.addView");
    }

    // A MauiLayout holding one child already laid out to `content_h` tall — the state the handler leaves
    // behind, since MauiLayout.onLayout never positions children itself.
    struct host_with_content
    {
        scoped_env env{};
        local_ref<jobject> host;
        local_ref<jobject> child;

        explicit host_with_content(jint content_h = k_content_h)
        {
            host = local_ref<jobject>{env.get(), new_view(env.get(), k_maui_layout_class)};
            child = local_ref<jobject>{env.get(), new_view(env.get(), k_view_class)};
            if (host.get() == nullptr || child.get() == nullptr)
            {
                return;
            }
            add_view(env.get(), host.get(), child.get());
            layout(env.get(), child.get(), 0, 0, k_viewport_w, content_h);
        }
    };
} // namespace

// The defect itself: under the UNSPECIFIED height spec a ScrollView always hands its document child, the
// host must report the CONTENT extent. It used to report 0.
TEST(AndroidScrollRange, MauiLayoutReportsContentExtentUnderUnspecifiedSpec)
{
    host_with_content seam;
    ASSERT_NE(seam.host.get(), nullptr);

    measure(seam.env.get(), seam.host.get(), make_spec(seam.env.get(), k_viewport_w, k_exactly),
            make_spec(seam.env.get(), 0, k_unspecified));

    EXPECT_EQ(measured_height(seam.env.get(), seam.host.get()), k_content_h)
        << "a MauiLayout measured UNSPECIFIED must report its content extent, not 0 — a 0 here is "
           "scrollRange 0 and the page cannot scroll at all";
    EXPECT_EQ(measured_width(seam.env.get(), seam.host.get()), k_viewport_w)
        << "an EXACTLY spec still wins on the cross axis";
}

// AT_MOST clamps to the spec, and an EXACTLY spec still wins outright — the two modes platform_arrange
// uses, whose behaviour must not change.
TEST(AndroidScrollRange, MauiLayoutHonoursExactlyAndAtMostSpecs)
{
    host_with_content seam;
    ASSERT_NE(seam.host.get(), nullptr);

    measure(seam.env.get(), seam.host.get(), make_spec(seam.env.get(), k_viewport_w, k_exactly),
            make_spec(seam.env.get(), k_viewport_h, k_exactly));
    EXPECT_EQ(measured_height(seam.env.get(), seam.host.get()), k_viewport_h)
        << "EXACTLY must report the spec size — platform_arrange frames this host that way";

    measure(seam.env.get(), seam.host.get(), make_spec(seam.env.get(), k_viewport_w, k_exactly),
            make_spec(seam.env.get(), k_viewport_h, k_at_most));
    EXPECT_EQ(measured_height(seam.env.get(), seam.host.get()), k_viewport_h)
        << "AT_MOST must clamp the content extent to the spec, not exceed it";
}

// An empty host has no content, so it reports 0 under UNSPECIFIED — the honest answer, and the one that
// keeps a contentless scroller from claiming a scroll range.
TEST(AndroidScrollRange, EmptyMauiLayoutReportsZeroExtent)
{
    const scoped_env env;
    const local_ref<jobject> host{env.get(), new_view(env.get(), k_maui_layout_class)};
    ASSERT_NE(host.get(), nullptr);

    measure(env.get(), host.get(), make_spec(env.get(), k_viewport_w, k_exactly),
            make_spec(env.get(), 0, k_unspecified));
    EXPECT_EQ(measured_height(env.get(), host.get()), 0);
}

// End to end through the framework's own scroller: a real android.widget.ScrollView whose document child
// is the host must be able to scroll down. canScrollVertically IS the scroll range, and it is the exact
// predicate the accessibility tree's scrollable="false" reported on the frozen pages.
TEST(AndroidScrollRange, RealScrollViewCanScrollWhenContentExceedsViewport)
{
    host_with_content seam;
    ASSERT_NE(seam.host.get(), nullptr);
    const local_ref<jobject> scroller{seam.env.get(), new_view(seam.env.get(), k_scroll_view_class)};
    ASSERT_NE(scroller.get(), nullptr);
    add_view(seam.env.get(), scroller.get(), seam.host.get());

    measure(seam.env.get(), scroller.get(), make_spec(seam.env.get(), k_viewport_w, k_exactly),
            make_spec(seam.env.get(), k_viewport_h, k_exactly));
    layout(seam.env.get(), scroller.get(), 0, 0, k_viewport_w, k_viewport_h);

    jmethodID can_scroll = default_jni_cache().method(seam.env.get(), k_view_class, "canScrollVertically", "(I)Z");
    ASSERT_NE(can_scroll, nullptr) << "View.canScrollVertically not found";
    const jboolean down = seam.env->CallBooleanMethod(scroller.get(), can_scroll, static_cast<jint>(1));
    ASSERT_FALSE(pending_exception_cleared(seam.env.get(), "canScrollVertically"));
    EXPECT_EQ(down, JNI_TRUE) << "content " << k_content_h << "px in a " << k_viewport_h
                              << "px viewport must leave a scroll range — this is the frozen-page defect, measured";
}

// The control: content that FITS leaves no range, so the same scroller correctly refuses to scroll. Without
// this, a host that reported a huge constant would pass the test above and still be wrong.
TEST(AndroidScrollRange, RealScrollViewCannotScrollWhenContentFits)
{
    host_with_content seam{k_viewport_h / 2};
    ASSERT_NE(seam.host.get(), nullptr);
    const local_ref<jobject> scroller{seam.env.get(), new_view(seam.env.get(), k_scroll_view_class)};
    ASSERT_NE(scroller.get(), nullptr);
    add_view(seam.env.get(), scroller.get(), seam.host.get());

    measure(seam.env.get(), scroller.get(), make_spec(seam.env.get(), k_viewport_w, k_exactly),
            make_spec(seam.env.get(), k_viewport_h, k_exactly));
    layout(seam.env.get(), scroller.get(), 0, 0, k_viewport_w, k_viewport_h);

    jmethodID can_scroll = default_jni_cache().method(seam.env.get(), k_view_class, "canScrollVertically", "(I)Z");
    ASSERT_NE(can_scroll, nullptr);
    const jboolean down = seam.env->CallBooleanMethod(scroller.get(), can_scroll, static_cast<jint>(1));
    ASSERT_FALSE(pending_exception_cleared(seam.env.get(), "canScrollVertically"));
    EXPECT_EQ(down, JNI_FALSE) << "content that fits the viewport must leave no scroll range";
}
