#pragma once
// maui::core::decode_image_bytes — decode encoded image bytes into a native image result (per backend).
//
// The single per-backend primitive that turns a byte buffer (PNG/JPEG/… contents) into an
// image_source_result. Shared by the stream + uri services (and the loader's cached-uri path) so there is
// exactly ONE place that touches the native image-decode API per platform:
//   * apple (src/platform/apple/image_source_services.mm): NSData → NSImage, retained into the result.
//   * headless (src/platform/headless/image_source_services.cpp): no native image — fills the mirror
//     fields (kind + detail) so tests can observe what decoded; `loaded()` is true for non-empty bytes.
//
// `kind` / `detail` seed the result's headless mirror (e.g. "stream" / "<bytes:42>", or "uri" / the uri).
// Empty `bytes` yield a `!loaded()` result (nothing to decode).

#include <string>

#include "maui/core/i_stream_image_source.hpp" // image_bytes
#include "maui/core/image_source_result.hpp"

namespace maui::core
{
    [[nodiscard]] image_source_result decode_image_bytes(const image_bytes& bytes, std::string kind,
                                                         std::string detail);
} // namespace maui::core
