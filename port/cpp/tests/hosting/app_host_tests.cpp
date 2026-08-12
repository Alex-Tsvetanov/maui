// Tests for maui::hosting::app_host — the generic recursive mount + layout driver (app_host.hpp) and the
// run_app entry seam (host_run.hpp). The driver walks an arbitrary element tree via the element generic-
// mount surface (visit_logical_children / handler_type_tag / mount_into_handler), attaches a handler to
// every element WITHOUT per-control special-casing, re-hosts each container, opens the window, and lays
// out. These assert the mount actually happened: every element gets a non-null handler, the label's text
// reaches its handler mirror, and a nested container tree (page -> scroll -> stack -> {label, border ->
// label}) mounts every node. Backend-agnostic — headless mirrors are observed, no native internals.
#include "maui/hosting/app_host.hpp"

#include <memory>

#include <gtest/gtest.h>

#include "maui/controls/application.hpp"
#include "maui/controls/border.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/platform_configuration/ios_specific/page.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/i_window.hpp"
#include "maui/core/label_handler.hpp"
#include "maui/hosting/host_run.hpp"
#include "maui/hosting/maui_app.hpp"
#include "maui/hosting/maui_app_builder.hpp"

namespace
{
    // A minimal app: window -> content_page -> label "Hello, MAUI C++!", built in the ctor (before any
    // handler exists — the construction-order case the driver's re-host handles). Exposes the tree so the
    // test can assert handlers attached + the text reached the headless label mirror.
    class hello_app final : public maui::controls::application
    {
    public:
        hello_app()
        {
            label_.set_text("Hello, MAUI C++!");
            page_.set_content(label_);
            window_.set_content(page_);
            window_.set_title("Hello");
        }

        maui::core::i_window* create_window() override
        {
            return &window_;
        }

        maui::controls::window& win()
        {
            return window_;
        }
        maui::controls::content_page& page()
        {
            return page_;
        }
        maui::controls::label& text_label()
        {
            return label_;
        }

    private:
        maui::controls::window window_;
        maui::controls::content_page page_;
        maui::controls::label label_;
    };

    // page -> vertical_stack_layout -> label. The root-child-is-a-Layout shape (148 of the 172 gallery
    // pages), which is the one where the page and the child can both try to inset the same edge.
    class stack_root_app final : public maui::controls::application
    {
    public:
        stack_root_app()
        {
            leaf_label_.set_text("leaf");
            stack_.add(leaf_label_);
            page_.set_content(stack_);
            window_.set_content(page_);
        }

        maui::core::i_window* create_window() override
        {
            return &window_;
        }

        maui::controls::window& win()
        {
            return window_;
        }
        maui::controls::content_page& page()
        {
            return page_;
        }
        maui::controls::label& leaf_label()
        {
            return leaf_label_;
        }
        maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }

    private:
        maui::controls::window window_;
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label leaf_label_;
    };

    // A deeper tree: page -> scroll_view -> vertical_stack_layout -> { leaf_label, border -> nested_label }.
    // Exercises the generic depth-first mount across a layout (per-child "add"), a single-content host
    // (scroll "set_content"), and a nested content host (border "set_content").
    class nested_app final : public maui::controls::application
    {
    public:
        nested_app()
        {
            leaf_label_.set_text("leaf");
            nested_label_.set_text("nested");
            border_.set_content(nested_label_);
            stack_.add(leaf_label_);
            stack_.add(border_);
            scroll_.set_content(stack_);
            page_.set_content(scroll_);
            window_.set_content(page_);
        }

        maui::core::i_window* create_window() override
        {
            return &window_;
        }

        maui::controls::window& win()
        {
            return window_;
        }
        maui::controls::content_page& page()
        {
            return page_;
        }
        maui::controls::scroll_view& scroll()
        {
            return scroll_;
        }
        maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        maui::controls::label& leaf_label()
        {
            return leaf_label_;
        }
        maui::controls::border& border()
        {
            return border_;
        }
        maui::controls::label& nested_label()
        {
            return nested_label_;
        }

    private:
        maui::controls::window window_;
        maui::controls::content_page page_;
        maui::controls::scroll_view scroll_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label leaf_label_;
        maui::controls::border border_;
        maui::controls::label nested_label_;
    };

    // Build the app through the builder for a given application type (the use_maui_app path the hosting layer
    // composes). Returns the built maui_app; application_as<T>() recovers the concrete app for tree access.
    template <class App> std::unique_ptr<maui::hosting::maui_app> build_app()
    {
        return maui::hosting::maui_app::create_builder().use_maui_app<App>().build();
    }
} // namespace

// mount_window attaches a handler to every element in the page tree (the page + its single label child).
TEST(app_host, mount_window_attaches_handlers_across_the_tree)
{
    auto app = build_app<hello_app>();
    auto* hello = app->application_as<hello_app>().get();
    ASSERT_NE(hello, nullptr);

    // Before the mount, the tree (built in the ctor) has no handlers.
    EXPECT_EQ(hello->page().handler(), nullptr);
    EXPECT_EQ(hello->text_label().handler(), nullptr);

    maui::hosting::mount_window(*app, hello->win());

    // Every element now owns a handler — the page, the label, and the window.
    EXPECT_NE(hello->page().handler(), nullptr);
    EXPECT_NE(hello->text_label().handler(), nullptr);
    EXPECT_NE(hello->win().handler(), nullptr);
}

// The label's text reaches its handler's platform view (the virtual->native push ran via the view_mapper
// when the handler attached) — i.e. the mount is real, not just a handler slot filled. The label_platform
// `text` mirror is the HEADLESS observation point (apple/ios write the NSTextField/UILabel via `native`
// instead, leaving `text` empty), so the text-value assertion is headless-only; the handler-attached +
// non-null platform-view assertions hold on every backend.
TEST(app_host, mount_window_pushes_label_text_to_its_handler_mirror)
{
    auto app = build_app<hello_app>();
    auto* hello = app->application_as<hello_app>().get();
    ASSERT_NE(hello, nullptr);

    maui::hosting::mount_window(*app, hello->win());

    auto label_handler = std::dynamic_pointer_cast<maui::core::label_handler>(hello->text_label().handler());
    ASSERT_NE(label_handler, nullptr);
    ASSERT_NE(label_handler->typed_platform_view(), nullptr);
#if defined(MAUI_PLATFORM_HEADLESS)
    EXPECT_EQ(label_handler->typed_platform_view()->text, "Hello, MAUI C++!");
#endif
}

// drive_layout runs a measure + arrange pass: the content page gets a non-empty arranged frame.
TEST(app_host, drive_layout_sizes_the_content_page)
{
    auto app = build_app<hello_app>();
    auto* hello = app->application_as<hello_app>().get();
    ASSERT_NE(hello, nullptr);
    maui::hosting::mount_window(*app, hello->win());

    const maui::graphics::size arranged = maui::hosting::drive_layout(hello->win(), 402.0, 874.0);

    EXPECT_GT(arranged.width, 0.0);
    EXPECT_GT(arranged.height, 0.0);
    // The page's own frame was stored by arrange.
    EXPECT_GT(hello->page().frame().width, 0.0);
}

// The generic driver mounts a DEEP tree with no per-control knowledge: a layout, a single-content scroll
// host, and a nested content host all attach + the deepest leaf's text reaches its mirror.
TEST(app_host, mount_window_handles_a_nested_container_tree)
{
    auto app = build_app<nested_app>();
    auto* nested = app->application_as<nested_app>().get();
    ASSERT_NE(nested, nullptr);

    maui::hosting::mount_window(*app, nested->win());

    EXPECT_NE(nested->page().handler(), nullptr);
    EXPECT_NE(nested->scroll().handler(), nullptr);
    EXPECT_NE(nested->stack().handler(), nullptr);
    EXPECT_NE(nested->leaf_label().handler(), nullptr);
    EXPECT_NE(nested->border().handler(), nullptr);
    EXPECT_NE(nested->nested_label().handler(), nullptr);

    // The deepest leaf (inside the border, inside the stack, inside the scroll) got a platform view; on
    // headless its text mirror confirms the virtual->native push reached the bottom of the tree.
    auto nested_handler = std::dynamic_pointer_cast<maui::core::label_handler>(nested->nested_label().handler());
    ASSERT_NE(nested_handler, nullptr);
    ASSERT_NE(nested_handler->typed_platform_view(), nullptr);
#if defined(MAUI_PLATFORM_HEADLESS)
    EXPECT_EQ(nested_handler->typed_platform_view()->text, "nested");
#endif
}

// run_app (the host_run.hpp entry the portable main() forwards to) boots, mounts, settles, and returns 0
// for a tiny inline configurator — the same path maui/maui_main.hpp drives, exercised end-to-end here.
// HEADLESS ONLY: run_app's body is the headless lane this Stage (src/platform/headless/host_run.cpp). The
// apple/ios run_app is Stage 2, so this call would be an undefined symbol on those lanes — the mount_window
// / drive_layout tests above stay backend-agnostic and cover the generic driver everywhere.
#if defined(MAUI_PLATFORM_HEADLESS)
TEST(app_host, run_app_boots_and_returns_zero)
{
    const auto configure = [](maui::hosting::maui_app_builder builder) {
        builder.use_maui_app<hello_app>();
        return builder;
    };
    char arg0[] = "maui_test";
    char* argv[] = {static_cast<char*>(arg0), nullptr};

    EXPECT_EQ(maui::hosting::run_app(1, static_cast<char**>(argv), configure), 0);
}
#endif

// ---- real layout invalidation (view.hpp invalidate_measure/invalidate_arrange + window::request_relayout)
// invalidate_measure() (view.hpp) now asks the calling view's containing_window() to replay the layout
// pass its host installed (window::request_relayout / set_relayout_hook) — the generalization of the
// Android-only jni/relayout.hpp hook onto every backend. These exercise that seam directly at the window
// level, then through a REAL call site (View.Margin, view.hpp's on_property_changed) on a mounted tree, and
// confirm the no-window / no-hook cases stay the documented no-ops.

// window::request_relayout invokes whatever hook set_relayout_hook installed, every time it is called.
TEST(app_host, window_request_relayout_invokes_the_installed_hook)
{
    maui::controls::window win;
    int calls = 0;
    win.set_relayout_hook([&calls] { ++calls; });

    win.request_relayout();
    win.request_relayout();

    EXPECT_EQ(calls, 2);
}

// Mirrors C#'s Handler?.Invoke no-op when nobody wired a handler: a window nobody's host has driven yet
// (so no hook was ever installed) is a harmless no-op, not a crash.
TEST(app_host, window_request_relayout_is_a_no_op_with_no_hook_installed)
{
    maui::controls::window win;
    win.request_relayout();
}

// The real proof: on a MOUNTED tree (handlers attached, first drive_layout done, a host-style relayout
// hook installed exactly like every backend's host_run/app_host now does post-boot), the pre-existing
// View.Margin call site (view.hpp's on_property_changed("margin") -> invalidate_measure()) reaches that
// hook — where before this change it was a documented no-op.
TEST(app_host, invalidate_measure_on_a_mounted_view_replays_the_hosts_layout_pass)
{
    auto app = build_app<hello_app>();
    auto* hello = app->application_as<hello_app>().get();
    ASSERT_NE(hello, nullptr);
    maui::hosting::mount_window(*app, hello->win());
    maui::hosting::drive_layout(hello->win(), 402.0, 874.0);

    int relayouts = 0;
    hello->win().set_relayout_hook([&relayouts] { ++relayouts; });

    hello->text_label().set_margin(maui::core::thickness(4));

    EXPECT_EQ(relayouts, 1);
}

// Before mount_window, containing_window() is null (element::set_containing_window only runs on attach —
// element.cpp), so a margin change on a not-yet-mounted view stays a no-op, matching C#'s Handler?.Invoke
// no-op before a handler/window exists.
TEST(app_host, invalidate_measure_before_mount_is_a_no_op)
{
    hello_app hello;
    hello.text_label().set_margin(maui::core::thickness(4)); // must not crash with no containing window
}

// C# `void IView.InvalidateArrange() { }` (VisualElement.cs) is a literal empty method body in the shipped
// source (arrange invalidation was never wired up there either) — faithfully ported as a no-op, so calling
// it directly must never reach a relayout hook.
TEST(app_host, invalidate_arrange_is_still_a_documented_no_op)
{
    auto app = build_app<hello_app>();
    auto* hello = app->application_as<hello_app>().get();
    ASSERT_NE(hello, nullptr);
    maui::hosting::mount_window(*app, hello->win());
    maui::hosting::drive_layout(hello->win(), 402.0, 874.0);

    int relayouts = 0;
    hello->win().set_relayout_hook([&relayouts] { ++relayouts; });

    hello->text_label().invalidate_arrange();

    EXPECT_EQ(relayouts, 0);
}

// LAYOUT MUST BE IDEMPOTENT: a second drive_layout over an UNCHANGED tree has to reproduce the first
// pass exactly. This is not a theoretical property -- it was violated in practice. Making
// invalidate_measure real (commit 0d8e44adb6) caused exactly one observable change, extra layout
// passes, and eight Windows parity pages then shifted ~0.5pp and stayed shifted. A controlled A/B on
// the guest (MAUI_NO_RELAYOUT, commit e5e36a7ef9) put all eight back to their pre-seam values, four of
// them to the exact hundredth. So the relayout seam did not introduce a defect; it EXPOSED one -- the
// port's measure/arrange does not converge to a fixed point.
//
// Guarding it headless matters because the symptom was only ever visible as a fraction of a percent of
// differing pixels on a Windows capture, which needs a VM, a build and a capture run to observe. A
// divergence here is the same bug, reproducible in milliseconds.
TEST(app_host, drive_layout_is_idempotent_over_an_unchanged_tree)
{
    auto app = build_app<hello_app>();
    auto* hello = app->application_as<hello_app>().get();
    ASSERT_NE(hello, nullptr);
    maui::hosting::mount_window(*app, hello->win());

    const maui::graphics::size first = maui::hosting::drive_layout(hello->win(), 402.0, 874.0);
    const maui::graphics::rect first_page_frame = hello->page().frame();

    // Nothing about the tree changed between the two calls -- same window, same content, same bounds.
    const maui::graphics::size second = maui::hosting::drive_layout(hello->win(), 402.0, 874.0);
    const maui::graphics::rect second_page_frame = hello->page().frame();

    EXPECT_DOUBLE_EQ(second.width, first.width);
    EXPECT_DOUBLE_EQ(second.height, first.height);
    EXPECT_DOUBLE_EQ(second_page_frame.x, first_page_frame.x);
    EXPECT_DOUBLE_EQ(second_page_frame.y, first_page_frame.y);
    EXPECT_DOUBLE_EQ(second_page_frame.width, first_page_frame.width);
    EXPECT_DOUBLE_EQ(second_page_frame.height, first_page_frame.height);
}

// The same idempotency contract on the DEEP tree (scroll host -> stack -> border -> nested label). The
// flat hello_app tree above converges, so if the port's layout has a fixed-point bug it needs nesting to
// surface -- and the eight Windows pages that exposed it are exactly that shape: five CollectionView
// pages plus a safe-area and a scroll-hosted one. Frames are compared at every level, because a parent
// that converges can still be hiding a child that does not.
TEST(app_host, drive_layout_is_idempotent_over_a_nested_tree)
{
    auto app = build_app<nested_app>();
    auto* nested = app->application_as<nested_app>().get();
    ASSERT_NE(nested, nullptr);
    maui::hosting::mount_window(*app, nested->win());

    maui::hosting::drive_layout(nested->win(), 402.0, 874.0);
    const maui::graphics::rect page1 = nested->page().frame();
    const maui::graphics::rect scroll1 = nested->scroll().frame();
    const maui::graphics::rect stack1 = nested->stack().frame();
    const maui::graphics::rect border1 = nested->border().frame();
    const maui::graphics::rect leaf1 = nested->nested_label().frame();

    maui::hosting::drive_layout(nested->win(), 402.0, 874.0);

    EXPECT_EQ(nested->page().frame(), page1);
    EXPECT_EQ(nested->scroll().frame(), scroll1);
    EXPECT_EQ(nested->stack().frame(), stack1);
    EXPECT_EQ(nested->border().frame(), border1);
    EXPECT_EQ(nested->nested_label().frame(), leaf1);
}

// --- safe area: the page and its root child must never BOTH inset the same edge -------------------
//
// C# MauiView.IsParentHandlingSafeArea (MauiView.cs:505-526): "a view never insets an edge an ancestor
// already inset". The port has two paths that meet at the page — content_page::layout_inset() (live when
// UseSafeArea makes the page's region Container, which C# Page.cs:120-125 defaults TRUE on Mac Catalyst)
// and drive_layout's push to the root child (which a Layout consumes, region Container). Exactly ONE of
// them must run, and the resulting ABSOLUTE content position must be the same either way — that is what
// makes flipping the Catalyst default a no-op for every page whose root is a Layout.
//
// Frames are host-relative (a child of a host at y=40 reports y=0), so the invariant is asserted on the
// SUM: page inset + child inset == the realized inset, once.
//
// Backend-agnostic: drives the two-rect drive_layout directly, so headless covers it.
TEST(app_host, safe_area_is_applied_once_whichever_view_consumes_it)
{
    constexpr double k_inset = 40.0;
    const maui::graphics::rect full{0, 0, 400, 800};
    const maui::graphics::rect safe{0, k_inset, 400, 800 - k_inset};

    for (const bool page_uses_safe_area : {false, true})
    {
        auto app = build_app<stack_root_app>();
        auto* rooted = app->application_as<stack_root_app>().get();
        ASSERT_NE(rooted, nullptr);
        maui::hosting::mount_window(*app, rooted->win());
        maui::controls::platform_configuration::ios_specific::page::set_use_safe_area(rooted->page(),
                                                                                      page_uses_safe_area);

        maui::hosting::drive_layout(rooted->win(), full, safe);

        // The page itself always spans the FULL bounds — it is the CONTENT that clears the unsafe band.
        EXPECT_DOUBLE_EQ(rooted->page().frame().y, 0.0) << "use_safe_area=" << page_uses_safe_area;
        // Applied exactly once: k_inset, not 0 (nobody applied it) and not 2 * k_inset (both did).
        const double content_top = rooted->stack().frame().y + rooted->leaf_label().frame().y;
        EXPECT_DOUBLE_EQ(content_top, k_inset) << "use_safe_area=" << page_uses_safe_area;
        // ...and WHICH view consumed it decides where the seam falls, without moving the content.
        EXPECT_DOUBLE_EQ(rooted->stack().frame().y, page_uses_safe_area ? k_inset : 0.0);
    }
}
