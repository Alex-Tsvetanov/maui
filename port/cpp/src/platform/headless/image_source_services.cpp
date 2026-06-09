// Per-source image services + decode_image_bytes — HEADLESS backend. The deterministic unit-test twin of
// src/platform/apple/image_source_services.mm: there is no native image tree, so each load fills the
// image_source_result's MIRROR fields (kind + detail) and marks it loaded (the headless tests assert on
// those). The apple twin produces a real NSImage instead. One TU implements all three services + the
// shared decode primitive (mirrors the per-backend partial-class split, PROFILE §5).

#include "maui/core/file_image_source_service.hpp"
#include "maui/core/font_image_source_service.hpp"
#include "maui/core/stream_image_source_service.hpp"
#include "maui/core/uri_image_source_service.hpp"

#include <string>
#include <utility>

#include "maui/core/cancellation_token.hpp"
#include "maui/core/i_font_image_source.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_stream_image_source.hpp"
#include "maui/core/i_uri_image_source.hpp"
#include "maui/core/image_decode.hpp"
#include "maui/core/image_source_result.hpp"
#include "maui/core/uri_bytes.hpp"

namespace maui::core
{
    // Headless decode: no native image — record what decoded in the mirror (kind/detail), loaded iff the
    // byte buffer is non-empty. Empty bytes (an empty/cancelled source) yield a !loaded() result.
    image_source_result decode_image_bytes(const image_bytes& bytes, std::string kind, std::string detail)
    {
        if (bytes.empty())
        {
            return {};
        }
        return image_source_result{nullptr, nullptr, std::move(kind), std::move(detail)};
    }

    void file_image_source_service::load(i_image_source& source, const cancellation_token& /*token*/,
                                         completion on_result)
    {
        const auto* file_src = dynamic_cast<const i_file_image_source*>(&source);
        if (file_src == nullptr || file_src->is_empty())
        {
            on_result(image_source_result{}); // not a file source / empty → nothing loaded
            return;
        }
        // No native handle headless: mirror kind="file" + the resolved path, marked loaded.
        on_result(image_source_result{nullptr, nullptr, "file", std::string(file_src->file())});
    }

    void uri_image_source_service::load(i_image_source& source, const cancellation_token& /*token*/,
                                        completion on_result)
    {
        auto* uri_src = dynamic_cast<i_uri_image_source*>(&source);
        if (uri_src == nullptr || uri_src->is_empty())
        {
            on_result(image_source_result{});
            return;
        }
        // Standalone (non-cached) fetch + decode — the loader's uri fast-path adds the in-memory cache.
        const std::string uri(uri_src->uri());
        on_result(decode_image_bytes(read_uri_bytes(uri), "uri", uri));
    }

    void stream_image_source_service::load(i_image_source& source, const cancellation_token& token,
                                           completion on_result)
    {
        auto* stream_src = dynamic_cast<i_stream_image_source*>(&source);
        if (stream_src == nullptr || stream_src->is_empty())
        {
            on_result(image_source_result{});
            return;
        }
        const image_bytes bytes = stream_src->get_bytes(token);
        on_result(decode_image_bytes(bytes, "stream", "<bytes:" + std::to_string(bytes.size()) + ">"));
    }

    void font_image_source_service::load(i_image_source& source, const cancellation_token& /*token*/,
                                         completion on_result)
    {
        const auto* font_src = dynamic_cast<const i_font_image_source*>(&source);
        if (font_src == nullptr || font_src->is_empty())
        {
            on_result(image_source_result{}); // not a font source / empty glyph → nothing rendered
            return;
        }
        // No native rasterization headless: mirror kind="font" + the glyph, marked loaded (the apple twin
        // draws the glyph into an NSImage). A non-empty glyph always "renders" in the mirror. Font results
        // are RESOLUTION-DEPENDENT (the rasterized glyph depends on display density — C# passes true).
        on_result(image_source_result{nullptr, nullptr, "font", std::string(font_src->glyph()),
                                      /*resolution_dependent*/ true});
    }
} // namespace maui::core
