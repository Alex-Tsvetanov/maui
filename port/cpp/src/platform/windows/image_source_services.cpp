// Per-source image services + decode_image_bytes — WINDOWS (WinUI 3) backend. The real-native twin of
// src/platform/headless/image_source_services.cpp: file/uri/stream decode into a genuine
// Microsoft.UI.Xaml.Media.Imaging.BitmapImage, boxed into the result the same way image_handler.cpp boxes
// the control's own native view (winui_interop.hpp's take/ref/drop). Ported from
// FileImageSourceService.Windows.cs / UriImageSourceService.Windows.cs / StreamImageSourceService.Windows.cs.
// FontImageSourceService.Windows.cs is now RASTERIZED FOR REAL through Win2D, the oracle's own rasterizer —
// see the FONT section below.
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
// FONT is now RASTERIZED FOR REAL, through Win2D -- the oracle's own rasterizer.
// FontImageSourceService.Windows.cs:6-8 uses Microsoft.Graphics.Canvas, and :57-97 (RenderImageSource) is
// reproduced line for line by render_font_glyph in the anonymous namespace below. The dependency is
// Microsoft.Graphics.Win2D 1.3.2, the version MAUI itself pins (src/Templates/src/cgmanifest.json:76-84).
// RISK: that same cgmanifest.json pins Microsoft.WindowsAppSDK 1.8.251106002 alongside it (:107-108), while
// provision_winui_sdk.ps1:30 still pins WASDK 1.7.250606001 -- MAUI's own proven pairing is Win2D 1.3.2 +
// WASDK 1.8, not 1.7. The guest already has both restored (C:\maui-winui\packages lists both 1.7 and 1.8),
// so bumping WinAppSdkVersion is available if the projection or activation misbehaves.
//
// It costs the build almost nothing, which is worth stating because it looks like it should cost a lot.
// Nothing is linked (Win2D ships no import library -- it is pure WinRT, activated at runtime), and no
// include directory is added: provision_winui_sdk.ps1 hands Win2D's winmd to the SAME cppwinrt run that
// already generates the App SDK projection into MAUI_WINUI_GENERATED. And this unpackaged exe needs NO
// registration-free-WinRT manifest to activate a third-party WinRT class: C++/WinRT's own
// get_runtime_activation_factory_impl (generated winrt/base.h:6088-6131) falls back on any activation
// failure to trimming the class name at each '.' and LoadLibrary + GetProcAddress("DllGetActivationFactory"),
// over LOAD_LIBRARY_SEARCH_DEFAULT_DIRS (base.h:4547) which includes the app directory. So
// `Microsoft.Graphics.Canvas.CanvasDevice` resolves out of Microsoft.Graphics.Canvas.dll sitting beside the
// exe, which maui_add_app.cmake's windows branch copies there next to the App Runtime bootstrap DLL.
//
// ONE DEVIATION, and it is forced rather than chosen: the oracle returns a CanvasImageSource (:86, :96);
// this returns a WriteableBitmap holding the same pixels, drawn into an offscreen CanvasRenderTarget.
// CanvasImageSource derives from SurfaceImageSource, making it a SIBLING of BitmapSource, not a subtype --
// and image_handler.cpp unboxes a "font" result as ref<writeable_bitmap> (:612) and sizes it through
// Source.try_as<bitmap_source>() (:314-322, a port of ImageHandler.Windows.cs:263-275's
// `Source is BitmapSource`). A CanvasImageSource fails both, and the second failure is the 0x0 row collapse
// that commit eb5851a8aa fixed. CanvasRenderTarget also has no XAML/DXGI-surface affinity, so it is the
// safer object to build on a load path that runs inline on the UI thread. NOT YET OBSERVED (Win2D is not
// restored on this guest as of writing this comment): CanvasImageSource and CanvasRenderTarget both round
// their float DIP size to integer pixels through the same Win2D helper, so the raster SHOULD match
// pixel-for-pixel -- but that is an expectation from the API shape, not a measured fact; confirm it on the
// first recapture rather than trusting this comment.
//
// The font FILE resolution is the part with NO oracle to copy: C# goes through IFontRegistrar/IFontManager
// (FontManager.Windows.cs:134-144's `_fontRegistrar.GetFont(...)`), and this port has NEITHER, so
// resolve_font_family below cannot reproduce that registrar rung -- its `<exe dir>/<family>.{ttf,otf}`
// probe is an invention with no oracle line to cite, defensible only because maui_add_app.cmake's windows
// branch copies an example's RESOURCES flat beside the exe (so the gallery's ionicons.ttf happens to land
// exactly there). Only the FALLBACK is oracle-backed: FontManager.Windows.cs:146-147 "Always send the base
// back" -- the bare family name, which Win2D resolves from the system font collection.
//
// ACCEPTANCE CHECK for whoever recaptures on the guest, restated against the CURRENT committed captures
// (measured, not a zero baseline): rows 0-699 already carry <=17 differing px light / <=115 dark against
// MAUI (pre-existing antialias specks + title-bar residue, unrelated to this fix) -- the gate is "stays at
// or under those counts", not "stays at exactly zero". Of the page total (46,468 px light / 46,566 dark),
// ~46,451 sit inside the rows 700-791 band (identical count both themes) and should fall toward a few
// hundred (antialiasing fringes of a ~45x Stretch=Uniform upscale) once the glyph is really drawn. `pixel`
// and `pixel_xaml` should move together: image_page.hpp is the only gallery page with a font source in the
// cpp column, and gallery_xaml/CMakeLists.txt:24 copies ionicons.ttf beside ITS exe too, so the same exe-dir
// probe resolves for both slots. (context_flyout.xaml is the only OTHER font source anywhere, xaml/maui
// columns only -- image_button has none.) If the band does not move AT ALL, Win2D activation failed and the
// catch in font_image_source_service::load produced the pre-Win2D transparent-square fallback -- check that
// Microsoft.Graphics.Canvas.dll actually landed beside gallery.exe before suspecting the render math.
//
// RESOLVED 2026-08-01 -- the above note's "MAUI has saturated-white ink as high as row 700 itself, but our
// canvas has a transparent pad there" contradiction was real, and the culprit was the CANVAS BOX, not the
// 1px pad: `src/` sizes the canvas from LayoutBounds (the LINE BOX -- 81x100 for Ionicons U+F30C at size 90,
// with the ink at rows 15-93), while the SHIPPED 10.0.71 the board renders against uses DrawBounds (the INK
// box). At this page's ~50x upscale a 15% line-box top margin is ~180 output rows, i.e. more than the entire
// 92-row visible band -- which is exactly the all-green band that was scored. See render_font_glyph's
// DOCUMENTED DEVIATION block for the three-line diff and the ruling-11 citation.
//
// And the 1px pad is a non-issue, measured rather than modelled: dumping the size-20 bitmap
// (MAUI_FONT_GLYPH_DUMP, below) gives a 20x20 canvas whose ALPHA ink runs rows 0-17, cols 1-18 -- the
// antialiased raster spills across the nominal 1px pad at the TOP (DrawBounds is the outline box, the
// coverage of the topmost partly-covered row lands above it), so there is no transparent first row to
// blow up into a ~49-row background stripe. That is why MAUI has saturated ink in the band's very first
// pixel row, and why this port now does too. Verified 2026-08-01: band rows 700-791 read
// green=50301 / row700=626 / row790=435 in ALL THREE columns (maui, cpp, xaml), identical.

#include "maui/core/file_image_source_service.hpp"
#include "maui/core/font_image_source_service.hpp"
#include "maui/core/stream_image_source_service.hpp"
#include "maui/core/uri_image_source_service.hpp"

#include <winrt/Microsoft.Graphics.Canvas.Text.h>
#include <winrt/Microsoft.Graphics.Canvas.h>
#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <winrt/Microsoft.UI.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Web.Http.h>

#include <windows.h>
// ::IUnknown, for the buffer_byte_access COM declaration below. NOT redundant with <windows.h>: this
// translation unit builds with WIN32_LEAN_AND_MEAN, which excludes the OLE chain (ole2.h -> objbase.h ->
// unknwn.h) that would otherwise declare it — without this the struct fails with C2504 "'IUnknown': base
// class undefined", and every use of it then cascades into C2259 "cannot instantiate abstract class".
#include <unknwn.h>

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
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
    // render_font_glyph's Win2D output carrier — see this file's header FONT section. A sibling of
    // bitmap_image (both derive Media::ImageSource), never interchangeable with it: boxed and unboxed under
    // its OWN type below, never through the bitmap_image take/ref/drop calls.
    using writeable_bitmap = winui::Media::Imaging::WriteableBitmap;

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

    // ---- FONT rasterization (Win2D) --------------------------------------------------------------------
    // The oracle is FontImageSourceService.Windows.cs, which rasterizes through Microsoft.Graphics.Canvas
    // (Win2D) -- `using Microsoft.Graphics.Canvas;` at :6-8. Nothing is linked for this: Win2D is pure WinRT
    // activated at runtime out of Microsoft.Graphics.Canvas.dll, which maui_add_app.cmake drops beside the
    // exe; see the framework CMakeLists.txt windows block for why that needs no activation manifest.
    namespace canvas = winrt::Microsoft::Graphics::Canvas;

    // Windows.Storage.Streams.IBufferByteAccess -- the COM escape hatch that hands back an IBuffer's raw
    // bytes (WriteableBitmap.PixelBuffer has no projected data accessor). Declared here rather than pulled
    // from <robuffer.h>, whose declarations sit in a GLOBAL ::Windows::Storage::Streams namespace that
    // reads as a collision with winrt::Windows::Storage::Streams at every call site in this file.
    struct __declspec(uuid("905a0fef-bc53-11df-8c49-001e4fc686da")) __declspec(novtable) buffer_byte_access : ::IUnknown
    {
        virtual HRESULT __stdcall Buffer(std::uint8_t** value) = 0;
    };

    // FontImageSourceService.Windows.cs:99-150 GetFontSource, collapsed to what a port with NO IFontManager
    // and NO IFontRegistrar can know. C# asks the registrar for the family's registered file and formats
    // `<uri>#<familyName>` (FontManager.Windows.cs:104-106), then rewrites a packaged scheme to `file://`
    // because "Win2D in unpackaged apps can't load files using packaged schemes, such as ms-appx://"
    // (FontImageSourceService.Windows.cs:127-147) -- that comment is also the oracle's own proof that
    // CanvasTextFormat.FontFamily consumes the `<uri>#<family>` form at all.
    //
    // With no registrar, the `<exe dir>/<family>.{ttf,otf}` probe below has NO oracle to cite -- this port
    // has no IFontRegistrar, so FontManager.Windows.cs:134-144's `_fontRegistrar.GetFont(...)` rung cannot
    // be reproduced; the probe exists only because maui_add_app.cmake's windows branch copies an example's
    // RESOURCES flat beside the exe, so the gallery's ionicons.ttf happens to land exactly there.
    //
    // A MISS returns EMPTY -- deliberately NOT the bare family name. FontManager.Windows.cs:146-147 "Always
    // send the base back" is only half the oracle: GetFontSource then POST-PROCESSES that value, and in an
    // UNPACKAGED app (this port always; MauiReference.csproj:46 sets WindowsPackageType=None too) it runs
    // `new Uri(fontSource, UriKind.RelativeOrAbsolute).LocalPath` on it (:141). A bare family name is a
    // RELATIVE Uri and LocalPath THROWS InvalidOperationException there -- GetImageSourceAsync rethrows
    // (:47-51) and ImageSourcePartExtensions.cs:56-60 swallows it with `setImage?.Invoke(null)`. So an
    // unregistered family renders NOTHING; falling back to Win2D's system font collection painted a glyph
    // the oracle does not. CEILING: this keys on "a font file sits beside the exe" where the oracle keys on
    // "the family was passed to fonts.AddFont". They agree on every call site today (Ionicons: both;
    // Arial: neither); a real registration list -- or a CanvasFontSet lookup -- is the upgrade path.
    //
    // SIMPLIFICATION: the '#' fragment must be the font file's REAL embedded family name, which C# reads
    // out of the file with Win2D's own CanvasFontSet (FontManager.Windows.cs:186-192). Here the REQUESTED
    // family is reused instead. Correct whenever the registration alias equals the embedded family -- true
    // for the gallery's ionicons.ttf, whose name table records family "Ionicons" against a requested
    // "Ionicons". A font whose alias differed would silently fall back to Win2D's default face; the fix is
    // a CanvasFontSet lookup right here, needing no dependency this file does not already have.
    winrt::hstring resolve_font_family(std::string_view family)
    {
        const std::wstring name = to_wstring(family);
        if (name.empty())
        {
            return {};
        }
        if (const std::wstring directory = exe_directory(); !directory.empty())
        {
            for (const wchar_t* const extension : {L".ttf", L".otf"})
            {
                std::wstring path = directory + L"\\" + name + extension;
                std::error_code failure;
                if (std::filesystem::exists(std::filesystem::path{path}, failure))
                {
                    const winrt::hstring uri = to_file_uri(std::move(path));
                    return winrt::hstring{std::wstring{uri.c_str()} + L"#" + name};
                }
            }
        }
        return {};
    }

    // FontImageSourceService.Windows.cs:57-97 RenderImageSource, line for line, with ONE deviation: the
    // oracle RETURNS a CanvasImageSource (:86, :96); this returns a WriteableBitmap carrying the SAME
    // pixels, drawn into an offscreen CanvasRenderTarget.
    //
    // WHY THE SWAP IS FORCED, not a preference: CanvasImageSource derives from SurfaceImageSource, so it is
    // a sibling of BitmapSource, not a subtype. This backend's image_handler.cpp unboxes a "font" result as
    // `ref<writeable_bitmap>` (:612) and measures it through `Source.try_as<bitmap_source>()`'s
    // PixelWidth/PixelHeight (:314-322, itself a port of ImageHandler.Windows.cs:263-275's
    // `Source is BitmapSource`). A CanvasImageSource fails BOTH: the unbox is the wrong type, and even past
    // that get_image_size returns zero, which is precisely the 0x0 row collapse commit eb5851a8aa fixed.
    // The WriteableBitmap shape is also the one already OBSERVED to produce MAUI-matching band geometry on
    // this page (the band's top edge and full width match byte-for-byte today; only the ink is missing).
    //
    // Throws winrt::hresult_error on any Win2D failure -- activation, font load, device loss. The caller
    // catches; see font_image_source_service::load.
    writeable_bitmap render_font_glyph(const maui::core::i_font_image_source& source)
    {
        // Oracle :59-64 -- scale is forced to 1 and the dpi is DeviceDisplay.BaseLogicalDpi, so the raster
        // is exactly 1 pixel per DIP and the result is marked non-resolution-dependent. dotnet/maui#1000.
        constexpr float base_logical_dpi = 96.0F;
        const auto font_size = static_cast<float>(source.font().size());

        // Oracle :70-77. FontAutoScalingEnabled is deliberately NOT consulted -- RenderImageSource reads
        // imageSource.Font.Size raw (:67), so a FontImageSource does not auto-scale on Windows.
        canvas::Text::CanvasTextFormat format;
        format.FontFamily(resolve_font_family(source.font().family()));
        format.FontSize(font_size);
        format.HorizontalAlignment(canvas::Text::CanvasHorizontalAlignment::Center);
        format.VerticalAlignment(canvas::Text::CanvasVerticalAlignment::Center);
        format.Options(canvas::Text::CanvasDrawTextOptions::Default);

        // RISK, the most likely runtime failure of this whole patch: GetSharedDevice() creates a D3D11
        // device on first use. This guest runs virtio-gpu-dod (display-only); WinUI 3 already composites
        // there, so SOME device path works, but Win2D on that driver has not been exercised. If activation
        // throws here, that is where to look first -- not at the font/layout code below.
        const canvas::CanvasDevice device = canvas::CanvasDevice::GetSharedDevice();
        // DOCUMENTED DEVIATION from the read-only `src/` snapshot, per parity ruling 11 (render wins) --
        // this follows the SHIPPED MauiVersion the board renders against, not `src/`. The two revisions
        // of FontImageSourceService.Windows.cs differ in exactly three places:
        //   src/ (post-10.0.71 snapshot)          shipped 10.0.71 (MauiReference.csproj:18)
        //   :80  CanvasTextLayout(.., size, size)  :80  CanvasTextLayout(.., 0, 0)
        //   :83-84 LayoutBounds.{Width,Height}+2   :83-84 DrawBounds.{Width,Height}+2
        //   :90-91 -LayoutBounds.{X,Y} + 1         :90-91 -DrawBounds.{X,Y} + 1
        // LayoutBounds is the LINE BOX (advance width x line height); DrawBounds is the INK box. For
        // Ionicons U+F30C that is the whole `image` page: measured on the guest, a LayoutBounds canvas is
        // 81x100 with the ink at rows 15-93, i.e. a 15% transparent top margin, and the page's ~50x
        // Stretch=Uniform blow-up turns that margin into MORE than the whole visible row -- pure
        // background where MAUI draws glyph. MAUI's own capture has saturated ink in the row's FIRST pixel
        // row, which only a DrawBounds (ink-tight) canvas produces. The 0x0 layout box is carried across
        // too: with Center/Center alignment it just recentres the layout origin, which the draw offset
        // below cancels, but it is what the shipped revision passes.
        const canvas::Text::CanvasTextLayout layout{device, maui::platform::windows::to_hstring(source.glyph()), format,
                                                    0.0F, 0.0F};
        const winrt::Windows::Foundation::Rect bounds = layout.DrawBounds();

        // Oracle :82-84 -- "add a 1px padding all around". This canvas size is the whole ballgame on the
        // `image` page: the Image is Stretch=Uniform into a ~984 DIP wide row, so the bitmap is upscaled
        // ~50x and the band's pixels are decided by bounds.Height, not by the ink's own resolution.
        canvas::CanvasRenderTarget target{device, bounds.Width + 2.0F, bounds.Height + 2.0F, base_logical_dpi};
        {
            const canvas::CanvasDrawingSession session = target.CreateDrawingSession();
            // A fresh CanvasRenderTarget's contents are UNDEFINED, where the oracle's
            // CreateDrawingSession(Colors.Transparent) (:87) clears as it opens -- so clear explicitly.
            session.Clear(winrt::Microsoft::UI::Colors::Transparent());
            // Oracle :89-93 -- "offset by 1px as we added a 1px padding".
            session.DrawTextLayout(layout, -bounds.X + 1.0F, -bounds.Y + 1.0F,
                                   maui::platform::windows::to_ui_color(source.color()));
            // NOT optional, and NOT what C#'s `using` (:87) does implicitly here: a C++/WinRT projected
            // type's destructor releases the reference but never calls Close(), and Close() is what issues
            // D2D's EndDraw. Without it GetPixelBytes below reads a target that was never flushed.
            session.Close();
        }

        const auto pixel_size = target.SizeInPixels();
        const winrt::com_array<std::uint8_t> pixels = target.GetPixelBytes();
        // Env-gated one-line diagnostic. This whole path can produce a fully TRANSPARENT bitmap WITHOUT
        // throwing -- an empty glyph string, or a font family that resolves to nothing, both lay out zero
        // ink -- so "no exception" is NOT evidence the glyph rendered. Log the inputs and the actual ink
        // count instead of inferring from the absence of a failure.
        if (const char* const log_path = std::getenv("MAUI_WINUI_LOG"))
        {
            std::FILE* file = nullptr;
            if (fopen_s(&file, log_path, "a") == 0 && file != nullptr)
            {
                std::size_t opaque = 0;
                for (std::size_t i = 3; i < pixels.size(); i += 4)
                {
                    opaque += (pixels[i] != 0) ? 1 : 0;
                }
                std::fprintf(file,
                             "font_glyph: family='%s' glyph_bytes=%zu size=%.1f bounds=%.2fx%.2f "
                             "px=%ux%u ink=%zu\n",
                             std::string{source.font().family()}.c_str(), source.glyph().size(),
                             static_cast<double>(font_size), static_cast<double>(bounds.Width),
                             static_cast<double>(bounds.Height), pixel_size.Width, pixel_size.Height, opaque);
                std::fclose(file);
            }
        }
        writeable_bitmap bitmap{static_cast<std::int32_t>(pixel_size.Width),
                                static_cast<std::int32_t>(pixel_size.Height)};
        // Both sides are 8-bit BGRA with PREMULTIPLIED alpha (CanvasRenderTarget's default alpha mode, and
        // WriteableBitmap's only pixel format), so this is a straight copy, not a conversion.
        const winrt::Windows::Storage::Streams::IBuffer buffer = bitmap.PixelBuffer();
        std::uint8_t* destination = nullptr;
        winrt::check_hresult(buffer.as<buffer_byte_access>()->Buffer(&destination));
        const std::size_t copied =
            std::min(static_cast<std::size_t>(pixels.size()), static_cast<std::size_t>(buffer.Capacity()));
        std::memcpy(destination, pixels.data(), copied);
        bitmap.Invalidate();
        if (const char* const log_path = std::getenv("MAUI_WINUI_LOG"))
        {
            std::FILE* file = nullptr;
            if (fopen_s(&file, log_path, "a") == 0 && file != nullptr)
            {
                std::size_t dest_ink = 0;
                for (std::size_t i = 3; i < copied; i += 4)
                {
                    dest_ink += (destination[i] != 0) ? 1 : 0;
                }
                std::size_t solid = 0;
                unsigned max_a = 0;
                std::size_t peak = 0;
                for (std::size_t i = 3; i < copied; i += 4)
                {
                    const unsigned a = destination[i];
                    solid += (a >= 250) ? 1 : 0;
                    if (a > max_a)
                    {
                        max_a = a;
                        peak = i - 3;
                    }
                }
                // Env-gated raw dump. The fastest way to answer "is the right glyph in the right place"
                // is to LOOK at the bitmap, not to keep inferring it from counters.
                if (const char* const dump_dir = std::getenv("MAUI_FONT_GLYPH_DUMP"))
                {
                    const std::string out = std::string{dump_dir} + "\\glyph_" + std::to_string(pixel_size.Width) +
                                            "x" + std::to_string(pixel_size.Height) + ".bgra";
                    std::FILE* raw = nullptr;
                    if (fopen_s(&raw, out.c_str(), "wb") == 0 && raw != nullptr)
                    {
                        std::fwrite(destination, 1, copied, raw);
                        std::fclose(raw);
                    }
                }
                const std::size_t row_bytes = static_cast<std::size_t>(pixel_size.Width) * 4;
                std::string rows;
                for (std::size_t r = 0; r < pixel_size.Height && r < 14; ++r)
                {
                    std::size_t n = 0;
                    for (std::size_t i = (r * row_bytes) + 3; i < (r + 1) * row_bytes && i < copied; i += 4)
                    {
                        n += (destination[i] != 0) ? 1 : 0;
                    }
                    rows += std::to_string(n) + ",";
                }
                std::fprintf(file,
                             "font_copy: copied=%zu ink=%zu solid=%zu max_a=%u peak=[%u %u %u %u] "
                             "row_ink[0..13]=%s\n",
                             copied, dest_ink, solid, max_a, destination[peak], destination[peak + 1],
                             destination[peak + 2], destination[peak + 3], rows.c_str());
                std::fclose(file);
            }
        }
        return bitmap;
    }
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
        //
        // Microsoft::UI::Dispatching, NOT Windows::System. This is a WinUI 3 (Windows App SDK) app, whose
        // UI thread owns a Microsoft.UI DispatcherQueue; the identically-named UWP
        // Windows::System::DispatcherQueue has no queue on that thread and GetForCurrentThread() returns
        // NULL. MAUI itself is explicit about which one: src/Core/src/Dispatching/Dispatcher.Windows.cs and
        // src/Essentials/src/MainThread/MainThread.windows.cs both open with `using Microsoft.UI.Dispatching`
        // before calling DispatcherQueue.GetForCurrentThread().
        //
        // This was NOT a compile error and NOT a crash -- both types exist and both project. The null queue
        // silently took the fallback branch, so the sink ran on the HTTP completion's threadpool thread and
        // touched the STA-affine BitmapImage/Image from off-thread. The whole feature was a no-op: the
        // `image` page's captures came back byte-identical, SSIM included, while the guest could reach
        // aka.ms/campus.jpg over HTTP 200 the entire time. The same UWP-vs-WinUI3 namespace collision the
        // `winui` alias note at the top of every handler in this directory warns about.
        const auto ui_queue = winrt::Microsoft::UI::Dispatching::DispatcherQueue::GetForCurrentThread();

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
            // FontImageSource.IsEmpty => string.IsNullOrEmpty(Glyph) (FontImageSource.cs:11), and the
            // oracle returns a null result for it (FontImageSourceService.Windows.cs:24-25).
            on_result(image_source_result{});
            return;
        }
        if (resolve_font_family(font_src->font().family()).empty())
        {
            // An unresolvable family throws out of GetFontSource in an unpackaged app (see
            // resolve_font_family), the throw is swallowed by ImageSourcePartExtensions.cs:56-60's
            // `setImage?.Invoke(null)`, and the WinUI Image measures 0x0. An empty result IS this port's
            // null source: image_handler.cpp's apply_loaded_result routes !loaded() to clear_source_native
            // (reset_platform_max_constraints + Source(nullptr)). Deliberately NOT the catch below -- that
            // one is the missing-Microsoft.Graphics.Canvas.dll degrade (eb5851a8aa) and must stay non-zero.
            on_result(image_source_result{});
            return;
        }
        writeable_bitmap bitmap{nullptr};
        try
        {
            bitmap = render_font_glyph(*font_src);
        }
        catch (const winrt::hresult_error& error)
        {
            // The oracle LOGS as well as rethrowing (:47-51). Only the rethrow is dropped below; keep the
            // log, env-gated exactly like host_run.cpp's boot_log. Without it this fallback is SILENT and
            // indistinguishable from a render-math bug -- which is precisely what happened on the first
            // Win2D run: the DLL and the font were both correctly beside the exe, the band did not move
            // one pixel, and there was nothing to read.
            if (const char* const log_path = std::getenv("MAUI_WINUI_LOG"))
            {
                std::FILE* file = nullptr;
                if (fopen_s(&file, log_path, "a") == 0 && file != nullptr)
                {
                    const std::string message = winrt::to_string(error.message());
                    std::fprintf(file, "font_glyph: FAILED hr=0x%08X %s\n",
                                 static_cast<unsigned int>(error.code().value), message.c_str());
                    std::fclose(file);
                }
            }
            // DEVIATION from FontImageSourceService.Windows.cs:47-51, which logs and RETHROWS. This load
            // runs INLINE on the UI thread -- see this file's header: image_source_loader has no windows
            // dispatcher partial, so every apply runs synchronously on the calling thread -- so a rethrow
            // here unwinds through the WinUI message pump and takes the process down. A missing glyph is a
            // parity defect; a dead gallery is a lost capture run, which is strictly worse.
            //
            // Degrade instead to the pre-Win2D stand-in: a transparent square sized off the font. Its only
            // job is to keep the Image's Stretch=Uniform measure non-zero so the row does not collapse to
            // 0x0 (the regression commit eb5851a8aa fixed). The realistic trigger is a MISSING
            // Microsoft.Graphics.Canvas.dll beside the exe -- maui_add_app.cmake copies it, and fails the
            // configure if it cannot, precisely because this fallback is otherwise silent.
            const auto side = static_cast<std::int32_t>(std::max(1.0, font_src->font().size()) + 2);
            bitmap = writeable_bitmap{side, side};
        }
        void* const boxed = maui::platform::windows::take<writeable_bitmap>(bitmap);
        // resolution_dependent stays TRUE, against FontImageSourceService.Windows.cs:43's explicit
        // `new ImageSourceServiceResult(image, false)`. Deliberate, and inert: in this port the flag drives
        // ONLY image_source_loader.hpp:169's requires_reload(scale) -- it never scales the natural size --
        // and the shared image_seam.resolution_dependent_source_requires_reload_on_density_change test
        // (tests/controls/image_tests.cpp:406-423) asserts TRUE for a font source on every backend.
        // src/platform/android/image_source_services.cpp:29-31 documents the identical choice.
        on_result(image_source_result{boxed,
                                      [boxed]() {
                                          void* slot = boxed;
                                          maui::platform::windows::drop<writeable_bitmap>(slot);
                                      },
                                      "font", std::string(font_src->glyph()),
                                      /*resolution_dependent*/ true});
    }
} // namespace maui::core
