// maui::devflow::agent — the debug-gated in-app test/automation agent (devflow/agent.hpp).
//
// Two pieces:
//   - handle_command(...): the PURE synchronous command core over the element tree (no sockets/threads).
//   - agent: a minimal, dependency-free localhost HTTP/1.1 server (POSIX sockets, one background accept
//     thread, poll-based so stop() never races accept). It marshals each request onto the UI thread (a
//     caller-supplied executor; inline by default) and writes the core's JSON response back.
//
// Compiled only when MAUI_DEVFLOW is defined (CMake option MAUI_DEVFLOW) — absent from release builds.

#if defined(MAUI_DEVFLOW)

    #include "maui/devflow/agent.hpp"

    #include <array>
    #include <atomic>
    #include <cctype>
    #include <cstdlib>
    #include <cstring>
    #include <format>
    #include <functional>
    #include <memory>
    #include <stdexcept>
    #include <string>
    #include <string_view>
    #include <thread>
    #include <typeinfo>

    #include <cxxabi.h>

    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <poll.h>
    #include <sys/socket.h>
    #include <unistd.h>

    #include "maui/controls/element.hpp"
    #include "maui/core/i_button.hpp"
    #include "maui/core/i_element_handler.hpp"
    #include "maui/core/i_view.hpp"
    #include "maui/graphics/rect.hpp"

namespace maui::devflow
{
    namespace
    {
        // ---- tiny JSON emit/extract (the payloads are trivial; no dependency) ---------------------------

        std::string json_escape(std::string_view s)
        {
            std::string out;
            out.reserve(s.size() + 2);
            for (const char c : s)
            {
                switch (c)
                {
                    case '"':
                        out += "\\\"";
                        break;
                    case '\\':
                        out += "\\\\";
                        break;
                    case '\n':
                        out += "\\n";
                        break;
                    case '\r':
                        out += "\\r";
                        break;
                    case '\t':
                        out += "\\t";
                        break;
                    default:
                        if (static_cast<unsigned char>(c) < 0x20)
                        {
                            out += std::format("\\u{:04x}", static_cast<unsigned>(static_cast<unsigned char>(c)));
                        }
                        else
                        {
                            out += c;
                        }
                        break;
                }
            }
            return out;
        }

        std::string jstr(std::string_view s)
        {
            return "\"" + json_escape(s) + "\"";
        }

        std::string jbool(bool b)
        {
            return b ? "true" : "false";
        }

        // Value of a "key":"string" field in a small JSON body, "" if absent. ponytail: not a full JSON
        // parser — enough for the {"automation_id":"x"} bodies the DevFlowDriver sends; swap for a real
        // parser if request payloads ever grow beyond flat string fields.
        std::string json_string_field(std::string_view body, std::string_view key)
        {
            const std::string needle = "\"" + std::string(key) + "\"";
            const auto k = body.find(needle);
            if (k == std::string_view::npos)
            {
                return "";
            }
            const auto colon = body.find(':', k + needle.size());
            if (colon == std::string_view::npos)
            {
                return "";
            }
            const auto q1 = body.find('"', colon + 1);
            if (q1 == std::string_view::npos)
            {
                return "";
            }
            std::string val;
            for (std::size_t i = q1 + 1; i < body.size(); ++i)
            {
                const char c = body[i];
                if (c == '\\' && i + 1 < body.size())
                {
                    val += body[i + 1];
                    ++i;
                    continue;
                }
                if (c == '"')
                {
                    break;
                }
                val += c;
            }
            return val;
        }

        // ---- element-tree introspection -----------------------------------------------------------------

        // The readable leaf type name of an element (debug-only): typeid + __cxa_demangle, keeping the leaf
        // after the last "::" (e.g. maui::controls::button -> "button"). Mirrors table_view_handler.cpp's
        // typeid-name-as-key, made human-readable for the tree.
        std::string type_name_of(const maui::controls::element& e)
        {
            int status = 0;
            // __cxa_demangle mallocs the result; own it with a free-deleter unique_ptr (no explicit free()).
            const std::unique_ptr<char, void (*)(void*)> demangled(
                abi::__cxa_demangle(typeid(e).name(), nullptr, nullptr, &status), std::free);
            std::string full = (status == 0 && demangled) ? demangled.get() : typeid(e).name();
            const auto pos = full.rfind("::");
            return pos == std::string::npos ? full : full.substr(pos + 2);
        }

        // Serialize `e` and its logical subtree as a JSON node: {type, automation_id, bounds:[x,y,w,h],
        // children:[...]}. bounds come from i_view::frame (the arranged frame); a non-view element (no
        // i_view face) reports a zero rect. Recurses via element::visit_logical_children — the same tree
        // edges the generic mount driver walks (app_host.hpp), no parallel tree.
        void append_node(std::string& out, maui::controls::element& e)
        {
            out += "{";
            out += "\"type\":" + jstr(type_name_of(e));

            const auto* view = dynamic_cast<const maui::core::i_view*>(&e);
            out += ",\"automation_id\":" + jstr(view != nullptr ? view->automation_id() : std::string_view{});
            const maui::graphics::rect f = view != nullptr ? view->frame() : maui::graphics::rect{};
            out += std::format(",\"bounds\":[{},{},{},{}]", f.x, f.y, f.width, f.height);

            out += ",\"children\":[";
            bool first = true;
            e.visit_logical_children([&out, &first](maui::controls::element& child) {
                if (!first)
                {
                    out += ",";
                }
                first = false;
                append_node(out, child);
            });
            out += "]}";
        }

        // First element in the tree (root inclusive) whose i_view::automation_id == id, or nullptr.
        maui::controls::element* find_by_automation_id(maui::controls::element& root, std::string_view id)
        {
            maui::controls::element* found = nullptr;
            std::function<void(maui::controls::element&)> walk = [&](maui::controls::element& e) {
                if (found != nullptr)
                {
                    return;
                }
                const auto* view = dynamic_cast<const maui::core::i_view*>(&e);
                if (view != nullptr && view->automation_id() == id)
                {
                    found = &e;
                    return;
                }
                e.visit_logical_children(walk);
            };
            walk(root);
            return found;
        }

        // ---- the command handlers -----------------------------------------------------------------------

        response make_json(int status, std::string body)
        {
            return response{.status = status, .content_type = "application/json", .body = std::move(body)};
        }

        response cmd_ready(maui::controls::element* root, const agent_info& info)
        {
            const auto* view = dynamic_cast<maui::core::i_view*>(root);
            const bool mounted = view != nullptr && view->handler() != nullptr;
            bool laid_out = false;
            if (view != nullptr)
            {
                const maui::graphics::rect f = view->frame();
                laid_out = f.width > 0.0 && f.height > 0.0;
            }
            const bool ready = mounted && laid_out;
            return make_json(200, std::format("{{\"ready\":{},\"app\":{},\"version\":{},\"commit\":{}}}", jbool(ready),
                                              jstr(info.app), jstr(info.version), jstr(info.commit)));
        }

        response cmd_tree(maui::controls::element* root)
        {
            if (root == nullptr)
            {
                return make_json(200, "{\"tree\":null}");
            }
            std::string tree;
            append_node(tree, *root);
            return make_json(200, "{\"tree\":" + tree + "}");
        }

        response cmd_tap(maui::controls::element* root, std::string_view body)
        {
            std::string id = json_string_field(body, "automation_id");
            if (id.empty())
            {
                id = json_string_field(body, "id");
            }

            maui::core::bindable_object* target = nullptr;
            if (root != nullptr && !id.empty())
            {
                if (auto* e = find_by_automation_id(*root, id))
                {
                    target = e; // element is-a bindable_object
                }
                else
                {
                    target = root->find_by_name(id); // x:Name fallback (Element.FindByName)
                }
            }

            const bool found = target != nullptr;
            bool activated = false;
            if (auto* btn = dynamic_cast<maui::core::i_button*>(target))
            {
                btn->send_clicked(); // button / image_button activation (IsEnabled-gated in the control)
                activated = true;
            }
            return make_json(200, std::format("{{\"found\":{},\"activated\":{}}}", jbool(found), jbool(activated)));
        }
    } // namespace

    response handle_command(maui::controls::element* root, std::string_view method, std::string_view path,
                            std::string_view body, const agent_info& info, const std::function<void()>& on_shutdown)
    {
        std::string_view route = path;
        if (const auto q = route.find('?'); q != std::string_view::npos)
        {
            route = route.substr(0, q); // ignore any query string
        }

        if (method == "GET" && route == "/ready")
        {
            return cmd_ready(root, info);
        }
        if (method == "GET" && route == "/tree")
        {
            return cmd_tree(root);
        }
        if (method == "POST" && route == "/tap")
        {
            return cmd_tap(root, body);
        }
        if (method == "GET" && route == "/screenshot")
        {
            // v1: no native capture. The external screencapture in the test runner is the ground-truth image.
            return make_json(501, "{\"error\":\"unsupported\",\"detail\":\"screenshot is delegated to the "
                                  "external screencapture in the test runner\"}");
        }
        if (method == "POST" && route == "/shutdown")
        {
            const response r = make_json(200, "{\"ok\":true}");
            on_shutdown(); // the server writes r THEN exits (so the client still gets the 200)
            return r;
        }
        return make_json(404, "{\"error\":\"unknown_route\"}");
    }

    // ================================ HTTP server ========================================================
    namespace
    {
        // The one unavoidable cast the BSD socket C API mandates: bind/connect/getsockname take `sockaddr*`
        // and there is no typed overload. Isolated to a single helper (established pattern — see color.cpp's
        // targeted NOLINT) so the rest of the module stays cast-free.
        sockaddr* as_sockaddr(sockaddr_in& addr)
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            return reinterpret_cast<sockaddr*>(&addr);
        }

        std::string status_text(int status)
        {
            switch (status)
            {
                case 200:
                    return "OK";
                case 404:
                    return "Not Found";
                case 501:
                    return "Not Implemented";
                default:
                    return "OK";
            }
        }

        void send_all(int fd, std::string_view data)
        {
            std::size_t sent = 0;
    #if defined(MSG_NOSIGNAL)
            constexpr int kSendFlags = MSG_NOSIGNAL;
    #else
            constexpr int kSendFlags = 0; // macOS: SO_NOSIGPIPE is set on the socket instead
    #endif
            while (sent < data.size())
            {
                const ssize_t n = ::send(fd, data.data() + sent, data.size() - sent, kSendFlags);
                if (n <= 0)
                {
                    break;
                }
                sent += static_cast<std::size_t>(n);
            }
        }

        std::size_t parse_content_length(std::string_view headers)
        {
            std::string lower;
            lower.reserve(headers.size());
            for (const char c : headers)
            {
                lower += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            }
            const auto k = lower.find("content-length:");
            if (k == std::string::npos)
            {
                return 0;
            }
            std::size_t i = k + std::strlen("content-length:");
            while (i < lower.size() && (lower[i] == ' ' || lower[i] == '\t'))
            {
                ++i;
            }
            std::size_t value = 0;
            while (i < lower.size() && lower[i] >= '0' && lower[i] <= '9')
            {
                value = value * 10 + static_cast<std::size_t>(lower[i] - '0');
                ++i;
            }
            return value;
        }
    } // namespace

    struct agent::impl
    {
        root_provider root;
        agent_info info;
        ui_executor run_on_ui;
        int listen_fd = -1;
        std::atomic<bool> running{false};
        std::thread thread;

        impl(root_provider r, agent_info i, ui_executor u)
            : root(std::move(r)), info(std::move(i)), run_on_ui(std::move(u))
        {
        }

        std::uint16_t start(std::uint16_t port)
        {
            if (listen_fd >= 0)
            {
                throw std::runtime_error("devflow::agent already started");
            }
            const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
            if (fd < 0)
            {
                throw std::runtime_error("devflow::agent socket() failed");
            }
            const int one = 1;
            ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    #if defined(SO_NOSIGPIPE)
            ::setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof(one));
    #endif
            sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1 only — never a routable interface
            if (::bind(fd, as_sockaddr(addr), sizeof(addr)) != 0)
            {
                ::close(fd);
                throw std::runtime_error("devflow::agent bind() failed");
            }
            if (::listen(fd, 8) != 0)
            {
                ::close(fd);
                throw std::runtime_error("devflow::agent listen() failed");
            }
            socklen_t len = sizeof(addr);
            if (::getsockname(fd, as_sockaddr(addr), &len) != 0)
            {
                ::close(fd);
                throw std::runtime_error("devflow::agent getsockname() failed");
            }
            listen_fd = fd;
            running.store(true);
            thread = std::thread([this] { accept_loop(); });
            return ntohs(addr.sin_port);
        }

        void stop()
        {
            if (!running.exchange(false))
            {
                return; // idempotent
            }
            if (thread.joinable())
            {
                thread.join(); // poll's 200ms timeout lets the loop observe running==false and exit
            }
            if (listen_fd >= 0)
            {
                ::close(listen_fd);
                listen_fd = -1;
            }
        }

        void accept_loop()
        {
            while (running.load())
            {
                pollfd pfd{.fd = listen_fd, .events = POLLIN, .revents = 0};
                const int pr = ::poll(&pfd, 1, 200); // re-check running every 200ms; no accept()/close() race
                if (pr <= 0)
                {
                    continue;
                }
                const int fd = ::accept(listen_fd, nullptr, nullptr);
                if (fd < 0)
                {
                    continue;
                }
    #if defined(SO_NOSIGPIPE)
                const int one = 1;
                ::setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof(one));
    #endif
                handle_connection(fd);
                ::close(fd);
            }
        }

        void handle_connection(int fd)
        {
            // Read the request: headers, then Content-Length bytes of body.
            std::string buf;
            std::array<char, 2048> chunk{};
            std::size_t header_end = std::string::npos;
            std::size_t content_length = 0;
            for (;;)
            {
                const ssize_t n = ::recv(fd, chunk.data(), chunk.size(), 0);
                if (n <= 0)
                {
                    break;
                }
                buf.append(chunk.data(), static_cast<std::size_t>(n));
                if (header_end == std::string::npos)
                {
                    header_end = buf.find("\r\n\r\n");
                    if (header_end != std::string::npos)
                    {
                        content_length = parse_content_length(std::string_view{buf}.substr(0, header_end));
                    }
                }
                if (header_end != std::string::npos && buf.size() >= header_end + 4 + content_length)
                {
                    break;
                }
            }
            if (header_end == std::string::npos)
            {
                return; // malformed / closed early
            }

            // Parse "METHOD SP PATH SP HTTP/x".
            const std::string_view head{buf.data(), header_end};
            const auto sp1 = head.find(' ');
            const auto sp2 = sp1 == std::string_view::npos ? std::string_view::npos : head.find(' ', sp1 + 1);
            if (sp1 == std::string_view::npos || sp2 == std::string_view::npos)
            {
                send_all(fd, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
                return;
            }
            const std::string method{head.substr(0, sp1)};
            const std::string path{head.substr(sp1 + 1, sp2 - sp1 - 1)};
            const std::string body = buf.substr(header_end + 4, content_length);

            // Run the command core on the UI thread (PROFILE §8): the tree is mutated/read there. The server
            // sets want_exit (not std::exit) so the 200 is flushed before the process ends.
            response resp;
            bool want_exit = false;
            auto work = [&] {
                resp = handle_command(root ? root() : nullptr, method, path, body, info,
                                      [&want_exit] { want_exit = true; });
            };
            if (run_on_ui)
            {
                run_on_ui(work);
            }
            else
            {
                work();
            }

            const std::string out =
                std::format("HTTP/1.1 {} {}\r\nContent-Type: {}\r\nContent-Length: {}\r\nConnection: close\r\n\r\n{}",
                            resp.status, status_text(resp.status), resp.content_type, resp.body.size(), resp.body);
            send_all(fd, out);

            if (want_exit)
            {
                std::exit(0); // clean process exit AFTER the response is on the wire
            }
        }
    };

    agent::agent(root_provider root, agent_info info, ui_executor run_on_ui)
        : impl_(std::make_unique<impl>(std::move(root), std::move(info), std::move(run_on_ui)))
    {
    }

    agent::~agent()
    {
        impl_->stop();
    }

    std::uint16_t agent::start(std::uint16_t port)
    {
        return impl_->start(port);
    }

    void agent::stop()
    {
        impl_->stop();
    }

    std::uint16_t start_agent(agent::root_provider root, std::uint16_t port, agent_info info,
                              agent::ui_executor run_on_ui)
    {
        // Process-lifetime agent, constructed once (a second call returns the first agent's port).
        static agent instance(std::move(root), std::move(info), std::move(run_on_ui));
        static const std::uint16_t bound = instance.start(port);
        return bound;
    }
} // namespace maui::devflow

#endif // MAUI_DEVFLOW
