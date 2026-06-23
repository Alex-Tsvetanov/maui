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
