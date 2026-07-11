#pragma once
// maui::devflow::agent  <=  (no C# 1:1) — the port's analog of Microsoft's experimental .NET MAUI DevFlow
// agent (learn.microsoft.com/dotnet/maui/developer-tools/devflow). A minimal, DEBUG-GATED, in-app
// test/automation agent that exposes a tiny JSON-over-HTTP API on localhost so an external driver (the
// parity test runner's Python DevFlowDriver) can introspect + poke the running UI.
//
// DEBUG-GATED: the whole module compiles ONLY when MAUI_DEVFLOW is defined (the port's `#if DEBUG` +
// `AddMauiDevFlowAgent()` analog — CMake option MAUI_DEVFLOW, ON for dev/test builds, OFF for release).
// It is NEVER auto-started: a host opts in by calling start_agent(...) once its window is mounted.
//
// Protocol (localhost HTTP/1.1, one request per connection, Connection: close). Documented in full in
// docs/DEVFLOW_PROTOCOL.md; the shapes are:
//   GET  /ready                      -> 200 {"ready":bool,"app":"..","version":"..","commit":".."}
//   GET  /tree                       -> 200 {"tree":<node>}  node = {"type":"button",
//                                             "automation_id":"..","bounds":[x,y,w,h],"children":[..]}
//   POST /tap  {"automation_id":"id"}-> 200 {"found":bool,"activated":bool}   (accepts "id" too; falls
//                                             back to x:Name via Element.FindByName)
//   GET  /screenshot                 -> 501 {"error":"unsupported","detail":".."}  (external screencapture
//                                             in the test runner is the ground-truth image)
//   POST /shutdown                   -> 200 {"ok":true}  then the process exits
//
// Backend-agnostic: the socket/HTTP/tree/tap logic is pure C++ over the existing element-tree walk
// (element::visit_logical_children), automation_id (view::automation_id), arrange bounds (i_view::frame),
// and activation (i_button::send_clicked). Only the opt-in wire-in + the UI-thread executor differ per
// backend.

#if defined(MAUI_DEVFLOW)

    #include <cstdint>
    #include <functional>
    #include <memory>
    #include <string>
    #include <string_view>

namespace maui::controls
{
    class element;
} // namespace maui::controls

namespace maui::devflow
{
    // Identity reported by GET /ready. All fields optional — blank is fine.
    struct agent_info
    {
        std::string app;
        std::string version;
        std::string commit;
    };

    // A raw HTTP response the command core produces. No sockets here — the server (agent) writes it.
    struct response
    {
        int status = 200;
        std::string content_type = "application/json";
        std::string body;
    };

    // The PURE, synchronous command core — fully unit-testable with NO sockets/threads. `root` is the
    // mounted root element (nullptr = not yet mounted). `on_shutdown` runs for POST /shutdown so a test can
    // observe it without killing its own process (the server passes std::exit).
    [[nodiscard]] response handle_command(maui::controls::element* root, std::string_view method, std::string_view path,
                                          std::string_view body, const agent_info& info,
                                          const std::function<void()>& on_shutdown);

    // The localhost HTTP server wrapping handle_command on a background accept thread. Bind is 127.0.0.1
    // only (never a routable interface). POSIX sockets (macOS/Linux/iOS/Android); Windows/WinSock is a TODO.
    class agent
    {
    public:
        // Returns the current mounted root element, or nullptr if not yet mounted. Re-queried per request
        // so the agent tracks page changes.
        using root_provider = std::function<maui::controls::element*()>;
        // Run a closure synchronously on the UI thread. Default (headless/tests) runs inline. Native
        // backends pass a dispatcher-backed executor so tree reads/taps happen on the UI thread (PROFILE §8).
        using ui_executor = std::function<void(const std::function<void()>&)>;

        agent(root_provider root, agent_info info, ui_executor run_on_ui = {});
        agent(const agent&) = delete;
        agent& operator=(const agent&) = delete;
        agent(agent&&) = delete;
        agent& operator=(agent&&) = delete;
        ~agent();

        // Bind 127.0.0.1:port (port 0 = ephemeral), start the accept thread, return the actual bound port.
        // Throws std::runtime_error on socket/bind/listen failure. Calling start twice is an error.
        std::uint16_t start(std::uint16_t port);
        // Stop the accept thread and close the socket. Idempotent; also called by the destructor.
        void stop();

    private:
        struct impl; // pimpl keeps POSIX socket headers out of this public header (PROFILE §3)
        std::unique_ptr<impl> impl_;
    };

    // One-line opt-in a host calls (debug-gated at the call site). Owns a process-lifetime agent and returns
    // the bound port. Not auto-started — the host calls this explicitly once its window is mounted. A second
    // call is a no-op that returns the first agent's port (function-local-static: constructed once).
    std::uint16_t start_agent(agent::root_provider root, std::uint16_t port, agent_info info = {},
                              agent::ui_executor run_on_ui = {});
} // namespace maui::devflow

#endif // MAUI_DEVFLOW
