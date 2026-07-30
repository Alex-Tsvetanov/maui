// Per-source image services + decode_image_bytes — WINDOWS (WinUI 3) backend. The real-native twin of
// src/platform/headless/image_source_services.cpp: file/uri/stream decode into a genuine
// Microsoft.UI.Xaml.Media.Imaging.BitmapImage, boxed into the result the same way image_handler.cpp boxes
// the control's own native view (winui_interop.hpp's take/ref/drop). Ported from
// FileImageSourceService.Windows.cs / UriImageSourceService.Windows.cs / StreamImageSourceService.Windows.cs.
// FontImageSourceService.Windows.cs stays MIRROR-ONLY — see the FONT section below for why.
//
// ASYNC DECODE, NO SYNC WAIT (image_handler.cpp's header note 2, restated here because this file hits it
// too): WinUI's BitmapImage has no synchronous decode API. image_handler.cpp's own file fast-path already
// solved this for a path already ON DISK: construct `BitmapImage{Uri}` and let the decode run in the
// background — the control's Source is set immediately; DesiredSize catches up on the harness's
// post-launch relayout. decode_image_bytes below reuses EXACTLY that trick for in-memory bytes (the
// uri/stream kinds): spool the bytes to a uniquely-named file under the temp directory, then build a
// BitmapImage from ITS Uri. This is deliberately NOT InMemoryRandomAccessStream + SetSourceAsync().get():
// a synchronous .get() on an IAsyncAction whose completion is marshalled back onto the calling thread
// (BitmapImage is DispatcherQueue-affine) deadlocks when called ON that thread — precisely the case here,
// since image_source_loader has no windows dispatcher partial (image_source_loader.hpp's set_dispatcher is
// never called for this backend), so every apply runs INLINE, synchronously, on whatever thread called
// update_source (the UI thread for every real caller). The Uri-ctor path sidesteps the deadlock entirely by
// never blocking on the decode. The spooled temp file is deliberately NOT deleted synchronously after the
// BitmapImage is constructed (the decode may still be reading it) — it is removed by the RESULT's disposer
// instead, which fires only once this result is superseded or dropped, by which point the decode has long
// finished in practice (the same timing assumption image_handler.cpp's note 2 already documents).
//
// FILE — mirrors image_handler.cpp's load_file_source_sync (bare name -> the exe's own directory; a rooted
// path or an already-schemed uri -> used as-is; see that file's header note 1 for why bare names resolve
// against the exe directory rather than `ms-appx:///` on this unpackaged exe). The resolution helpers below
// are a deliberate DUPLICATE of image_handler.cpp's anonymous-namespace ones (TU-local, not exported) —
// the same split the apple backend keeps between image_handler.mm's own file resolution and
// image_source_services.mm's separate image_from_file. This service is not on the Image control's own hot
// path today (ImageHandler special-cases file sources synchronously via load_file_source_sync BEFORE
// reaching the shared loader — see image_handler.cpp's map_source; ImageButtonHandler does the same via its
// own headless load_file_source_sync, since image_button_handler.cpp is not swapped for this backend), but
// it is the contract every OTHER i_file_image_source consumer resolves through (a missing definition here
// is a link error), so it is implemented for real rather than mirrored.
//
// STREAM — i_stream_image_source::get_bytes() already produces the encoded bytes; decode_image_bytes does
// the rest.
//
// URI — the STANDALONE (non-loader-fast-path) service below, uri_image_source_service::load, keeps the same
// shape as the headless/apple/android twins: read_uri_bytes(uri) (the cross-platform file://+local-path
// reader; http(s) stays unsupported there, per uri_bytes.hpp's own documented scope — this file does not
// touch it) then decode_image_bytes. This matches apple's/android's identical STANDALONE bodies: on every
// backend, the real network fetch lives in the LOADER's fast path, not here.
//
// The loader's actual hot path for a real Image control (image_source_loader.cpp's update_uri_source) does
// NOT call this service at all: it calls the injected uri_fetch seam (image_source_loader::set_uri_fetch),
// then decode_image_bytes directly. fetch_uri_async, below, IS that seam for this backend now — a real async
// Windows.Web.Http.HttpClient GET, installed via image_handler.cpp's configure_loader
// (loader.set_uri_fetch(&fetch_uri_async)), the same wiring shape as apple's NSURLSession fetch_uri_async
// (image_handler.mm) and android's Java-download fetch_uri_async (image_handler.cpp). A local file:// / bare
// path uri still reads synchronously via read_uri_bytes (no network, matching apple's identical branch); an
// http(s) uri starts an async GetBufferAsync request whose completion is explicitly re-posted onto the
// calling (UI) thread's DispatcherQueue before invoking the loader's sink. This hop is NOT optional: the
// loader has no windows dispatcher installed (image_handler.cpp's configure_loader never calls
// set_dispatcher, matching apple), so image_source_loader.cpp's deliver() runs the cache-write + apply
// INLINE on whatever thread invoked the sink — without the explicit hop, an HttpClient completion landing on
// a background/threadpool thread would touch the loader's cache map and the WinUI Image control off the UI
// thread (a real crash risk, not just a style concern). Cancellation is advisory only, checked at completion
// (matching apple's identical non-abort behavior) — the in-flight HTTP request itself is never cancelled,
// only its bytes are dropped. The on-disk cache layer (set_disk_cache_directory) is DELIBERATELY left
// unconfigured by image_handler.cpp's configure_loader — see this file's fetch_uri_async for why (capture-
// run determinism is an open question, not a guess this file resolves on its own).
//
// FONT stays MIRROR-ONLY: FontImageSourceService.Windows.cs (RenderImageSource/GetPlatformImage) rasterizes
// the glyph via Win2D (Microsoft.Graphics.Canvas: CanvasDevice/CanvasTextFormat/CanvasTextLayout/
// CanvasImageSource) — a separate native dependency this port does not link on ANY backend. There is no
// plain-WinUI (non-Win2D) glyph-to-image rasterizer to fall back to; inventing one (e.g. hand-drawing via
// Direct2D/DirectWrite) would be a new rasterizer, which this file's own task explicitly says to leave
// mirrored rather than invent. The Image page's two "Font Image Source" rows (Ionicons glyphs) stay blank
// bands on this backend, same as before this file existed.

#include "maui/core/file_image_source_service.hpp"
#include "maui/core/font_image_source_service.hpp"
#include "maui/core/stream_image_source_service.hpp"
#include "maui/core/uri_image_source_service.hpp"

#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.Web.Http.h>

#include <windows.h>

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <ios>
#include <memory>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include "maui/core/cancellation_token.hpp"
#include "maui/core/crc64.hpp"
#include "maui/core/i_font_image_source.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_stream_image_source.hpp"
#include "maui/core/i_uri_image_source.hpp"
#include "maui/core/image_decode.hpp"
#include "maui/core/image_source_loader.hpp"
#include "maui/core/image_source_result.hpp"
#include "maui/core/uri_bytes.hpp"
#include "winui_interop.hpp"

namespace
{
    // Named `winui`, NOT `xaml` — see image_handler.cpp's identical note (this port's own maui::xaml would
    // shadow it inside namespace maui::*).
    namespace winui = winrt::Microsoft::UI::Xaml;
    using bitmap_image = winui::Media::Imaging::BitmapImage;

    // ---- file:// Uri construction, shared by the FILE resolver and the spooled-bytes decoder below -----

    // Build a `file:///` Uri string from an absolute Windows path (forward slashes throughout — Uri parses
    // either, this keeps the scheme + path unambiguous). Duplicated from image_handler.cpp's identical
    // helper (TU-local, not exported — see this file's header).
    winrt::hstring to_file_uri(std::wstring path)
    {
        std::ranges::replace(path, L'\\', L'/');
        return winrt::hstring{L"file:///" + path};
    }

    // ---- FILE resolution (duplicated from image_handler.cpp's resolve_file_uri — see this file's header)

    bool has_uri_scheme(std::string_view path)
    {
        return path.starts_with("http://") || path.starts_with("https://") || path.starts_with("file://") ||
               path.starts_with("ms-appx://");
    }

    std::wstring to_wstring(std::string_view utf8)
    {
        const winrt::hstring wide = maui::platform::windows::to_hstring(utf8);
        return std::wstring{wide.c_str(), wide.size()};
    }

    bool is_rooted(const std::wstring& path)
    {
        if (path.size() >= 2 && path[1] == L':')
        {
            return true;
        }
        return path.starts_with(L"\\") || path.starts_with(L"/");
    }

    // The running exe's own directory (GetModuleFileNameW then strip the filename) — where
    // maui_add_app.cmake's windows branch copies an example's RESOURCES flat. Empty on any WinAPI failure.
    std::wstring exe_directory()
    {
        std::wstring buffer(MAX_PATH, L'\0');
        for (;;)
        {
            const DWORD written = ::GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
            if (written == 0)
            {
                return {};
            }
            if (written < buffer.size())
            {
                buffer.resize(written);
                break;
            }
            buffer.resize(buffer.size() * 2);
        }
        const auto slash = buffer.find_last_of(L"\\/");
        return slash == std::wstring::npos ? std::wstring{} : buffer.substr(0, slash);
    }

    // Resolve a from_file() path (relative or rooted, or an already-schemed uri) into a loadable Uri — see
    // this file's header FILE section for why a bare filename resolves against the EXE directory.
    winrt::Windows::Foundation::Uri resolve_file_uri(std::string_view utf8_path)
    {
        if (has_uri_scheme(utf8_path))
        {
            return winrt::Windows::Foundation::Uri{maui::platform::windows::to_hstring(utf8_path)};
        }
        std::wstring path = to_wstring(utf8_path);
        if (!is_rooted(path))
        {
            if (const std::wstring dir = exe_directory(); !dir.empty())
            {
                path = dir + L"\\" + path;
            }
        }
        return winrt::Windows::Foundation::Uri{to_file_uri(std::move(path))};
    }

    // ---- decode_image_bytes' spool-to-temp-file helper --------------------------------------------------

    // Write `bytes` to `path` (binary). Byte-by-byte, matching uri_image_disk_cache::write's identical loop
    // (no reinterpret_cast — std::byte* is not char*-compatible with a bulk ofstream::write). Returns false
    // on any I/O failure.
    bool write_bytes(const std::filesystem::path& path, const maui::core::image_bytes& bytes)
    {
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        if (!file)
        {
            return false;
        }
        for (const std::byte b : bytes)
        {
            file.put(static_cast<char>(std::to_integer<unsigned char>(b)));
        }
        file.flush();
        return static_cast<bool>(file);
    }

    // A fresh, unique path under the temp directory for one decode_image_bytes call. kind+detail (crc64'd,
    // reusing the same hash uri_image_disk_cache already uses to turn a uri into a safe filename) seed the
    // name; a per-process atomic counter guarantees uniqueness even across two loads with identical
    // kind+detail (e.g. the same uri reloaded) so an in-flight decode of the first never reads the second's
    // bytes out from under it.
    std::filesystem::path spool_path(const std::string& kind, const std::string& detail)
    {
        static std::atomic<std::uint64_t> counter{0};
        const std::string name = "maui_img_" + maui::core::crc64_hash_string(kind + "|" + detail) + "_" +
                                 std::to_string(counter.fetch_add(1, std::memory_order_relaxed)) + ".tmp";
        return std::filesystem::temp_directory_path() / name;
    }

    // ---- fetch_uri_async's HTTP GET + IBuffer decode helpers ---------------------------------------------

    // Copy a fetched HTTP response's IBuffer into image_bytes. DataReader stages the bytes into a plain
    // uint8_t buffer first, then each byte is narrowed via static_cast (no reinterpret_cast between
    // std::byte* and uint8_t* — the same byte-by-byte convention write_bytes/read_uri_bytes use elsewhere in
    // this file). Empty for a zero-length buffer.
    maui::core::image_bytes to_image_bytes(const winrt::Windows::Storage::Streams::IBuffer& buffer)
    {
        const std::uint32_t length = buffer.Length();
        if (length == 0)
        {
            return {};
        }
        std::vector<std::uint8_t> raw(length);
        winrt::Windows::Storage::Streams::DataReader::FromBuffer(buffer).ReadBytes(raw);
        maui::core::image_bytes bytes(length);
        std::transform(raw.begin(), raw.end(), bytes.begin(), [](std::uint8_t b) { return static_cast<std::byte>(b); });
        return bytes;
    }

    // Per-fetch state the Completed lambda captures (by shared_ptr): the move-only sink, an OWNED copy of
    // the cancellation token (the loader's token is a const-ref parameter; this struct must outlive the
    // call), and the HttpClient itself. The client is kept alive for the whole request deliberately — the
    // same reason apple's fetch_uri_async uses the process-wide `[NSURLSession sharedSession]` rather than a
    // short-lived per-request session: destroying an HttpClient can tear down its own outstanding requests.
    struct uri_fetch_state
    {
        maui::core::image_source_loader::uri_bytes_sink sink;
        maui::core::cancellation_token token;
        winrt::Windows::Web::Http::HttpClient client;
    };
} // namespace

namespace maui::core
{
    // Spool `bytes` to a uniquely-named temp file, then decode exactly like image_handler.cpp's file
    // fast-path decodes an on-disk file: BitmapImage{Uri} (see this file's header for why not
    // SetSourceAsync().get()). Empty bytes yield a `!loaded()` result (nothing to decode); a spool write
    // failure or a malformed-Uri throw likewise yields `!loaded()` (a best-effort cleanup removes the
    // partial temp file in both cases).
    image_source_result decode_image_bytes(const image_bytes& bytes, std::string kind, std::string detail)
    {
        if (bytes.empty())
        {
            return {};
        }
        const std::filesystem::path path = spool_path(kind, detail);
        if (!write_bytes(path, bytes))
        {
            std::error_code ec;
            std::filesystem::remove(path, ec);
            return {};
        }
        try
        {
            void* const boxed = maui::platform::windows::take<bitmap_image>(
                bitmap_image{winrt::Windows::Foundation::Uri{to_file_uri(path.wstring())}});
            return image_source_result{boxed,
                                       [boxed, path]() {
                                           void* slot = boxed;
                                           maui::platform::windows::drop<bitmap_image>(slot);
                                           std::error_code ec;
                                           std::filesystem::remove(path, ec); // best-effort; ignore failure
                                       },
                                       std::move(kind), std::move(detail)};
        }
        catch (const winrt::hresult_error&)
        {
            std::error_code ec;
            std::filesystem::remove(path, ec);
            return {};
        }
    }

    void file_image_source_service::load(i_image_source& source, const cancellation_token& /*token*/,
                                         completion on_result)
    {
        const auto* file_src = dynamic_cast<const i_file_image_source*>(&source);
        if (file_src == nullptr || file_src->is_empty())
        {
            on_result(image_source_result{});
            return;
        }
        const std::string path(file_src->file());
        try
        {
            void* const boxed = maui::platform::windows::take<bitmap_image>(bitmap_image{resolve_file_uri(path)});
            on_result(image_source_result{boxed,
                                          [boxed]() {
                                              void* slot = boxed;
                                              maui::platform::windows::drop<bitmap_image>(slot);
                                          },
                                          "file", path});
        }
        catch (const winrt::hresult_error&)
        {
            on_result(image_source_result{}); // a malformed path/uri — mirror the apple twin's nil-on-failure
        }
    }

    // The async uri fetch seam (image_source_loader::uri_fetch) — C# UriImageSource.DownloadStreamAsync,
    // translated to a real Windows.Web.Http.HttpClient GET. See this file's header URI section for the full
    // threading/cancellation writeup; installed via image_handler.cpp's configure_loader
    // (loader.set_uri_fetch(&fetch_uri_async)), the windows twin of apple's NSURLSession fetch_uri_async
    // (image_handler.mm) and android's Java-download fetch_uri_async (image_handler.cpp). External linkage
    // (not in the anonymous namespace above) so image_handler.cpp can forward-declare + take its address.
    void fetch_uri_async(const std::string& uri, const cancellation_token& token,
                         image_source_loader::uri_bytes_sink sink)
    {
        // Local files / non-http schemes: read synchronously (the loader's file:// fast-path equivalent;
        // matches apple's fetch_uri_async identical branch).
        if (!uri.starts_with("http://") && !uri.starts_with("https://"))
        {
            sink(read_uri_bytes(uri));
            return;
        }

        // Captured NOW: fetch_uri_async is only ever called from the UI thread (image_source_loader::
        // update_source runs synchronously inside the property mapper's update_properties() pass), so this
        // is the queue the Completed handler below must re-post onto. A null queue (the caller somehow not
        // on a dispatcher-owning thread) falls back to invoking sink() wherever the completion actually
        // landed — best effort, matching "no dispatcher: apply inline" (image_source_loader.hpp).
        const auto ui_queue = winrt::Windows::System::DispatcherQueue::GetForCurrentThread();

        auto state = std::make_shared<uri_fetch_state>(uri_fetch_state{
            .sink = std::move(sink), .token = token, .client = winrt::Windows::Web::Http::HttpClient{}});

        try
        {
            const winrt::Windows::Foundation::Uri url{maui::platform::windows::to_hstring(uri)};
            auto op = state->client.GetBufferAsync(url);
            op.Completed([state, ui_queue](auto const& async_op, auto status) {
                image_bytes bytes;
                if (status == winrt::Windows::Foundation::AsyncStatus::Completed && !state->token.is_cancelled())
                {
                    try
                    {
                        bytes = to_image_bytes(async_op.GetResults());
                    }
                    catch (const winrt::hresult_error&)
                    {
                        bytes = {}; // an HTTP-status/transport failure → nothing fetched
                    }
                }
                if (ui_queue)
                {
                    ui_queue.TryEnqueue([state, bytes = std::move(bytes)]() mutable { state->sink(std::move(bytes)); });
                }
                else
                {
                    state->sink(std::move(bytes));
                }
            });
        }
        catch (const winrt::hresult_error&)
        {
            state->sink(image_bytes{}); // a malformed uri, or GetBufferAsync throwing synchronously
        }
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
        // Standalone (non-cached) fetch + decode — the loader's own uri fast-path adds the two cache
        // layers; see this file's header URI section for the read_uri_bytes local-file-only scope.
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
        // MIRROR ONLY — see this file's header FONT section (FontImageSourceService.Windows.cs rasterizes
        // via Win2D, a dependency this port does not link). Headless-shaped result: no native handle, kind
        // + glyph recorded, resolution_dependent=true (matches the headless/apple twins' font result).
        on_result(image_source_result{nullptr, nullptr, "font", std::string(font_src->glyph()),
                                      /*resolution_dependent*/ true});
    }
} // namespace maui::core
