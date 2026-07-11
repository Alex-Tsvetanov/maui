// Tests for maui::devflow::agent — the debug-gated in-app test/automation agent (devflow/agent.hpp).
// Two layers, per the port's TDD loop:
//   (1) handle_command — the PURE synchronous command core, driven directly (no socket). Backend-agnostic.
//   (2) agent — the localhost HTTP server, driven over a real TCP socket by a tiny in-test client. This is
//       the exact wire the external Python DevFlowDriver uses, so it proves a client can drive it. Headless
//       only (the socket loop touches the tree off the AppKit main thread; the core tests cover every
//       backend).
//
// The whole file is gated on MAUI_DEVFLOW — with the option OFF (release) the module is absent, so there is
// nothing to test and the TU compiles to nothing.
#if defined(MAUI_DEVFLOW)

    #include "maui/devflow/agent.hpp"

    #include <atomic>
    #include <memory>
    #include <string>

    #include <gtest/gtest.h>

    #include "maui/controls/application.hpp"
    #include "maui/controls/button.hpp"
    #include "maui/controls/content_page.hpp"
    #include "maui/controls/vertical_stack_layout.hpp"
    #include "maui/controls/window.hpp"
    #include "maui/core/event.hpp"
    #include "maui/core/i_window.hpp"
    #include "maui/hosting/app_host.hpp"
    #include "maui/hosting/maui_app.hpp"
    #include "maui/hosting/maui_app_builder.hpp"

    #if defined(MAUI_PLATFORM_HEADLESS)
        #include <array>
        #include <cerrno>
        #include <cstring>

        #include <arpa/inet.h>
        #include <netinet/in.h>
        #include <sys/socket.h>
        #include <unistd.h>
    #endif

namespace
{
    // window -> content_page -> vertical_stack_layout -> button("Tap me", AutomationId "tap_me"), built in
    // the ctor (before any handler exists). Counts button.clicked so a tap is observable.
    class devflow_test_app final : public maui::controls::application
    {
    public:
        devflow_test_app()
        {
            button_.set_text("Tap me");
            button_.set_automation_id("tap_me");
            click_token_ = maui::core::connect_scoped(button_.clicked, [this] { clicks_.fetch_add(1); });
            stack_.add(button_);
            page_.set_content(stack_);
            window_.set_content(page_);
            window_.set_title("DevFlow test");
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
        maui::controls::button& btn()
        {
            return button_;
        }
        int clicks() const
        {
            return clicks_.load();
        }

    private:
        maui::controls::window window_;
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::button button_;
        std::atomic<int> clicks_{0};
        maui::core::scoped_connection click_token_;
    };

    struct built_app
    {
        std::unique_ptr<maui::hosting::maui_app> app;
        devflow_test_app* view = nullptr;
    };

    // Build (and optionally mount + lay out) the test app through the builder, the way hosting composes it.
    built_app make_app(bool mount)
    {
        built_app b;
        b.app = maui::hosting::maui_app::create_builder().use_maui_app<devflow_test_app>().build();
        b.view = b.app->application_as<devflow_test_app>().get();
        if (mount)
        {
            maui::hosting::mount_window(*b.app, b.view->win());
            maui::hosting::drive_layout(b.view->win(), 402.0, 874.0);
        }
        return b;
    }

    const maui::devflow::agent_info kInfo{.app = "DevFlowTest", .version = "1.2.3", .commit = "abc123"};

    // Convenience: call the core with a no-op shutdown (so a /shutdown test never kills the process).
    maui::devflow::response call(maui::controls::element* root, std::string_view method, std::string_view path,
                                 std::string_view body = "")
    {
        return maui::devflow::handle_command(root, method, path, body, kInfo, [] {});
    }
} // namespace

// ---- (1) handle_command core -------------------------------------------------------------------------

TEST(devflow_core, ready_is_false_before_mount)
{
    auto b = make_app(/*mount=*/false);
    const auto r = call(&b.view->page(), "GET", "/ready");
    EXPECT_EQ(r.status, 200);
    EXPECT_NE(r.body.find("\"ready\":false"), std::string::npos) << r.body;
}

TEST(devflow_core, ready_is_true_after_mount_and_reports_identity)
{
    auto b = make_app(/*mount=*/true);
    const auto r = call(&b.view->page(), "GET", "/ready");
    EXPECT_EQ(r.status, 200);
    EXPECT_NE(r.body.find("\"ready\":true"), std::string::npos) << r.body;
    EXPECT_NE(r.body.find("DevFlowTest"), std::string::npos) << r.body;
    EXPECT_NE(r.body.find("1.2.3"), std::string::npos) << r.body;
}

TEST(devflow_core, ready_is_false_when_root_is_null)
{
    const auto r = call(nullptr, "GET", "/ready");
    EXPECT_EQ(r.status, 200);
    EXPECT_NE(r.body.find("\"ready\":false"), std::string::npos) << r.body;
}

TEST(devflow_core, tree_lists_the_button_with_id_type_and_bounds)
{
    auto b = make_app(/*mount=*/true);
    const auto r = call(&b.view->page(), "GET", "/tree");
    EXPECT_EQ(r.status, 200);
    // The button node: its automation_id, a type mentioning button, and a non-degenerate bounds array.
    EXPECT_NE(r.body.find("\"automation_id\":\"tap_me\""), std::string::npos) << r.body;
    EXPECT_NE(r.body.find("button"), std::string::npos) << r.body;
    EXPECT_NE(r.body.find("\"bounds\":["), std::string::npos) << r.body;
    // The page is the root of the reported tree, and its child stack + button nest under "children".
    EXPECT_NE(r.body.find("\"children\":["), std::string::npos) << r.body;
}

TEST(devflow_core, tap_activates_button_by_automation_id)
{
    auto b = make_app(/*mount=*/true);
    EXPECT_EQ(b.view->clicks(), 0);
    const auto r = call(&b.view->page(), "POST", "/tap", "{\"automation_id\":\"tap_me\"}");
    EXPECT_EQ(r.status, 200);
    EXPECT_NE(r.body.find("\"found\":true"), std::string::npos) << r.body;
    EXPECT_NE(r.body.find("\"activated\":true"), std::string::npos) << r.body;
    EXPECT_EQ(b.view->clicks(), 1);
}

TEST(devflow_core, tap_accepts_the_short_id_key)
{
    auto b = make_app(/*mount=*/true);
    const auto r = call(&b.view->page(), "POST", "/tap", "{\"id\":\"tap_me\"}");
    EXPECT_NE(r.body.find("\"found\":true"), std::string::npos) << r.body;
    EXPECT_EQ(b.view->clicks(), 1);
}

TEST(devflow_core, tap_unknown_id_is_not_found)
{
    auto b = make_app(/*mount=*/true);
    const auto r = call(&b.view->page(), "POST", "/tap", "{\"automation_id\":\"nope\"}");
    EXPECT_EQ(r.status, 200);
    EXPECT_NE(r.body.find("\"found\":false"), std::string::npos) << r.body;
    EXPECT_EQ(b.view->clicks(), 0);
}

TEST(devflow_core, screenshot_is_unsupported)
{
    auto b = make_app(/*mount=*/true);
    const auto r = call(&b.view->page(), "GET", "/screenshot");
    EXPECT_EQ(r.status, 501);
    EXPECT_NE(r.body.find("unsupported"), std::string::npos) << r.body;
}

TEST(devflow_core, shutdown_invokes_the_callback)
{
    auto b = make_app(/*mount=*/true);
    bool shut = false;
    const auto r =
        maui::devflow::handle_command(&b.view->page(), "POST", "/shutdown", "", kInfo, [&shut] { shut = true; });
    EXPECT_EQ(r.status, 200);
    EXPECT_NE(r.body.find("\"ok\":true"), std::string::npos) << r.body;
    EXPECT_TRUE(shut);
}

TEST(devflow_core, unknown_route_is_404)
{
    auto b = make_app(/*mount=*/true);
    const auto r = call(&b.view->page(), "GET", "/bogus");
    EXPECT_EQ(r.status, 404);
}

    // ---- (2) agent HTTP server over a real socket (headless only) ----------------------------------------
    #if defined(MAUI_PLATFORM_HEADLESS)
namespace
{
    // Send one raw HTTP/1.1 request to 127.0.0.1:port and return the full response text (headers + body).
    // Returns "" on any socket error. Minimal — the exact wire the Python DevFlowDriver speaks.
    std::string http_request(std::uint16_t port, const std::string& request)
    {
        const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0)
        {
            return "";
        }
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0)
        {
            ::close(fd);
            return "";
        }
        (void)::send(fd, request.data(), request.size(), 0);
        std::string out;
        std::array<char, 1024> buf{};
        for (;;)
        {
            const ssize_t n = ::recv(fd, buf.data(), buf.size(), 0);
            if (n <= 0)
            {
                break;
            }
            out.append(buf.data(), static_cast<std::size_t>(n));
        }
        ::close(fd);
        return out;
    }
} // namespace

TEST(devflow_server, drives_ready_and_tap_over_tcp)
{
    auto b = make_app(/*mount=*/true);
    maui::devflow::agent agent([&b] { return static_cast<maui::controls::element*>(&b.view->page()); }, kInfo);
    const std::uint16_t port = agent.start(0); // ephemeral
    ASSERT_NE(port, 0);

    const std::string ready = http_request(port, "GET /ready HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n");
    EXPECT_NE(ready.find("200"), std::string::npos) << ready;
    EXPECT_NE(ready.find("\"ready\":true"), std::string::npos) << ready;

    const std::string body = "{\"automation_id\":\"tap_me\"}";
    const std::string tap =
        http_request(port, "POST /tap HTTP/1.1\r\nHost: x\r\nContent-Length: " + std::to_string(body.size()) +
                               "\r\nConnection: close\r\n\r\n" + body);
    EXPECT_NE(tap.find("\"found\":true"), std::string::npos) << tap;
    EXPECT_EQ(b.view->clicks(), 1);

    agent.stop();
}
    #endif // MAUI_PLATFORM_HEADLESS

#endif // MAUI_DEVFLOW
