#pragma once
// maui::core::read_uri_bytes — fetch the raw bytes for a URI (this cut: `file://` and bare local paths).
//
// The cross-platform byte fetch behind the uri image pipeline. A production HTTP(S) stack is DEFERRED
// (see i_uri_image_source.hpp): this reads only local files — a `file://` URI (the scheme + optional
// `localhost` authority stripped) or a plain filesystem path. Returns empty on any failure (missing file,
// unreadable, an unsupported scheme like http(s)) — the caller treats empty as "nothing loaded".
//
// Lives in maui_core (pure std::filesystem I/O, no native): the apple backend's uri service may instead
// use NSURL/NSData to also support http(s), but the headless tests use `file://` and never hit the network.

#include <string_view>

#include "maui/core/i_stream_image_source.hpp" // image_bytes

namespace maui::core
{
    // Read the bytes for `uri`. Supports `file://[localhost]/path` and bare `/path`; empty on failure or
    // an unsupported (e.g. http/https) scheme.
    [[nodiscard]] image_bytes read_uri_bytes(std::string_view uri);
} // namespace maui::core
