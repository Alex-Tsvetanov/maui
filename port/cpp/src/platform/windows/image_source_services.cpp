// Per-source image services + decode_image_bytes — WINDOWS (WinUI 3) backend. The headless twin
// (src/platform/headless/image_source_services.cpp) is pure bookkeeping — each load fills the
// image_source_result's MIRROR fields (kind + detail) and marks it loaded, producing NO native image —
// and this first windows cut is deliberately that same bookkeeping (a thin copy, per the lane brief):
// the WINDOWS HANDLER PARTIALS consume the kind/detail mirror and perform the native push themselves
// (src/platform/windows/{image_handler.cpp,image_button_handler.cpp} apply_loaded_result builds a
// BitmapImage over the file path / local uri the detail carries — the ImageSourcePartSetter lane), so
// a native handle in the result would be redundant until the byte-decode seams land.
//
// C# ground truth (what the deferred native production must eventually match):
//   - FileImageSourceService.Windows.cs: rooted path → StorageFile + BitmapImage.SetSourceAsync;
//     otherwise BitmapImage(new Uri("ms-appx:///" + filename)). // deferred: the handlers' file:///
//     BitmapImage lane covers the on-disk case today; the ms-appx packaged lane needs app packaging.
//   - UriImageSourceService.Windows.cs: downloads to the local cache folder (CacheValidity honored),
//     then BitmapImage over the cached file. // deferred: read_uri_bytes only resolves file:// / bare
//     local paths this cut (no HTTP), so the loaded "uri" results the handlers decode are local ones.
//   - StreamImageSourceService.Windows.cs: bytes → InMemoryRandomAccessStream →
//     BitmapImage.SetSourceAsync. // deferred: the InMemoryRandomAccessStream seam.
//   - FontImageSourceService.Windows.cs: renders the glyph through Win2D's CanvasTextLayout into a
//     CanvasImageSource (DWrite rasterization). // deferred: needs the Win2D/DWrite seam — the mirror
//     (kind="font" + glyph, resolution_dependent=true) keeps the load + the density-reload contract
//     (ImageHandler.OnWindowChanged → requires_reload) fully observable meanwhile.
//
// One TU implements all three byte services + the font service + the shared decode primitive
// (mirrors the per-backend partial-class split, PROFILE §5).

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
    // Windows decode (this cut): no native handle in the result — record what decoded in the mirror
    // (kind/detail), loaded iff the byte buffer is non-empty; the handler partials build the
    // BitmapImage from the detail. Empty bytes (an empty/cancelled source) yield a !loaded() result.
    // deferred: bytes → InMemoryRandomAccessStream → BitmapImage.SetSourceAsync, detached into the
    // result's image slot with a disposer (the apple decode_image_bytes twin's shape).
    image_source_result decode_image_bytes(const image_bytes& bytes, std::string kind, std::string detail)
    {
        if (bytes.empty())
        {
            return {};
        }
        return image_source_result{nullptr, nullptr, std::move(kind), std::move(detail)};
    }

    // FileImageSourceService: no native handle here — mirror kind="file" + the resolved path, marked
    // loaded; the handler's apply_loaded_result decodes it (BitmapImage over file:///). C# resolves
    // the BitmapImage in the service instead (header note) — same observable outcome for on-disk files.
    void file_image_source_service::load(i_image_source& source, const cancellation_token& /*token*/,
                                         completion on_result)
    {
        const auto* file_src = dynamic_cast<const i_file_image_source*>(&source);
        if (file_src == nullptr || file_src->is_empty())
        {
            on_result(image_source_result{}); // not a file source / empty → nothing loaded
            return;
        }
        on_result(image_source_result{nullptr, nullptr, "file", std::string(file_src->file())});
    }

    // UriImageSourceService: standalone (non-cached) fetch + decode — the loader's uri fast-path adds
    // the in-memory cache. read_uri_bytes resolves file:// uris + bare local paths only this cut, so
    // http(s) sources yield a !loaded() result (deferred: the download-to-cache lane — header note).
    void uri_image_source_service::load(i_image_source& source, const cancellation_token& /*token*/,
                                        completion on_result)
    {
        auto* uri_src = dynamic_cast<i_uri_image_source*>(&source);
        if (uri_src == nullptr || uri_src->is_empty())
        {
            on_result(image_source_result{});
            return;
        }
        const std::string uri(uri_src->uri());
        on_result(decode_image_bytes(read_uri_bytes(uri), "uri", uri));
    }

    // StreamImageSourceService: drain the stream's bytes; the mirror records "<bytes:N>" (no native
    // decode yet — the InMemoryRandomAccessStream seam, header note).
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

    // FontImageSourceService: no DWrite/Win2D rasterization yet (header note) — mirror kind="font" +
    // the glyph, marked loaded. Font results are RESOLUTION-DEPENDENT (the rasterized glyph depends on
    // display density — C#'s service passes true), so the loader's requires_reload contract stays live.
    void font_image_source_service::load(i_image_source& source, const cancellation_token& /*token*/,
                                         completion on_result)
    {
        const auto* font_src = dynamic_cast<const i_font_image_source*>(&source);
        if (font_src == nullptr || font_src->is_empty())
        {
            on_result(image_source_result{}); // not a font source / empty glyph → nothing rendered
            return;
        }
        on_result(image_source_result{nullptr, nullptr, "font", std::string(font_src->glyph()),
                                      /*resolution_dependent*/ true});
    }
} // namespace maui::core
