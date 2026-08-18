# DevFlow agent protocol

A minimal, **debug-gated** in-app test/automation agent for the C++26 MAUI port, modeled on Microsoft's
experimental [.NET MAUI DevFlow agent](https://learn.microsoft.com/en-us/dotnet/maui/developer-tools/devflow/).
It exposes a tiny JSON-over-HTTP API on **localhost** so an external driver (the parity test runner's Python
`DevFlowDriver`) can introspect and poke the running UI.

- Module: `maui::devflow` — header `include/maui/devflow/agent.hpp`, impl `src/devflow/agent.cpp`.
- Compiled **only** when the CMake option `MAUI_DEVFLOW` is `ON` (default ON for dev/test; set
  `-DMAUI_DEVFLOW=OFF` for release, and the whole module — socket server included — is absent). This is the
  port's analog of MAUI's `#if DEBUG` + `AddMauiDevFlowAgent()`.
- **Never auto-started.** A host opts in explicitly by calling `start_agent(...)` once its window is mounted.
- Bind is `127.0.0.1` only. Transport is HTTP/1.1, one request per connection (`Connection: close`).

## Starting the agent (opt-in hook)

```cpp
#if defined(MAUI_DEVFLOW)
#include "maui/devflow/agent.hpp"
// ...once the window is mounted (e.g. application::on_post_mount):
const std::uint16_t port = maui::devflow::start_agent(
    [this] { return &this->content_root(); },      // root_provider: current mounted root element (or null)
    8765,                                           // port (0 = ephemeral; returns the actual bound port)
    {.app = "gallery", .version = "1.0", .commit = "abc123"},
    run_on_ui);                                     // optional UI-thread executor (see "Threading")
#endif
```

`start_agent` returns the bound port. It owns a process-lifetime agent; a second call is a no-op that
returns the first agent's port. For finer control, construct `maui::devflow::agent` directly and call
`start(port)` / `stop()` (this is what the tests do).

## Routes

| Method | Path          | Body                          | Success                                                        |
| ------ | ------------- | ----------------------------- | ------------------------------------------------------------- |
| GET    | `/ready`      | —                             | `200 {"ready":bool,"app":str,"version":str,"commit":str}`     |
| GET    | `/tree`       | —                             | `200 {"tree":<node>\|null}`                                   |
| POST   | `/tap`        | `{"automation_id":"id"}`      | `200 {"found":bool,"activated":bool}`                         |
| GET    | `/screenshot` | —                             | `501 {"error":"unsupported","detail":"..."}`                  |
| POST   | `/shutdown`   | —                             | `200 {"ok":true}` then the process exits                      |

Any other method/path → `404 {"error":"unknown_route"}`. A query string on the path is ignored.

### `GET /ready`

Is the app mounted **and** past first layout?

```json
{ "ready": true, "app": "gallery", "version": "1.0", "commit": "abc123" }
```

`ready` is `true` when the root element has a handler attached (mounted) **and** its arranged frame has a
non-zero size (first layout ran). `false` before mount or before the first layout pass, or if the root
provider returns null.

### `GET /tree`

The logical element hierarchy. Each node:

```json
{
  "type": "button",
  "automation_id": "tap_me",
  "bounds": [x, y, w, h],
  "children": [ /* nested nodes */ ]
}
```

- `type` — the readable leaf type name (e.g. `button`, `content_page`, `vertical_stack_layout`).
- `automation_id` — `View.AutomationId` (empty string if unset / not a view).
- `bounds` — the arranged frame `[x, y, width, height]` in the page's coordinate space (`i_view::frame`).
  Non-view elements report `[0,0,0,0]`.
- `children` — the direct logical children (the same tree the generic mount driver walks).

Root is `null` when the app is not yet mounted.

### `POST /tap`

Find an element by id and activate it.

```json
// request
{ "automation_id": "tap_me" }     // "id" is also accepted as the key
// response
{ "found": true, "activated": true }
```

Lookup order: first any element whose `AutomationId` matches; then, as a fallback, `x:Name` via
`Element.FindByName` (so XAML-loaded pages that set `x:Name` but no `AutomationId` still resolve).
`found` = an element matched. `activated` = the match was a button (`i_button`) and `send_clicked()` was
invoked (`IsEnabled`-gated by the control, exactly as a native tap). A non-button match returns
`found:true, activated:false` (v1 only activates buttons — this is the minimal command set).

### `GET /screenshot`

Not implemented in v1 (`501`). The external screencapture in the test runner is the ground-truth image;
the agent does not build native capture from scratch.

### `POST /shutdown`

Clean process exit. The `200 {"ok":true}` is flushed to the client **before** the process exits.

## Threading

Tree reads and taps must run on the UI thread (PROFILE §8). `agent` takes an optional `ui_executor`
(`void(const std::function<void()>&)`) that runs a closure synchronously on the UI thread; the socket
thread blocks on it per request. The default (headless / tests) runs inline. Native backends pass a
dispatcher-backed executor, e.g.:

```cpp
auto run_on_ui = [dispatcher](const std::function<void()>& work) {
    if (!dispatcher->is_dispatch_required()) { work(); return; }
    std::promise<void> done;
    auto fut = done.get_future();
    dispatcher->dispatch([&work, &done]() mutable { work(); done.set_value(); });
    fut.wait();
};
```

## Example Python client (the `DevFlowDriver` shape)

```python
import json, urllib.request

class DevFlowDriver:
    def __init__(self, port, host="127.0.0.1"):
        self.base = f"http://{host}:{port}"

    def _req(self, method, path, body=None):
        data = json.dumps(body).encode() if body is not None else None
        req = urllib.request.Request(self.base + path, data=data, method=method)
        with urllib.request.urlopen(req) as r:
            return r.status, json.loads(r.read() or "null")

    def ready(self):      return self._req("GET",  "/ready")[1]["ready"]
    def tree(self):       return self._req("GET",  "/tree")[1]["tree"]
    def tap(self, aid):   return self._req("POST", "/tap", {"automation_id": aid})[1]
    def shutdown(self):   return self._req("POST", "/shutdown")
```

## Scope (v1, deliberately minimal)

Only the five commands above. Explicitly **deferred**: fill/set-text, scroll-by-id, profiling, native
screenshot capture, MCP, and any non-localhost networking. The JSON extractor is intentionally a flat
string-field reader, not a full JSON parser — enough for the tiny request bodies; upgrade if payloads grow.
