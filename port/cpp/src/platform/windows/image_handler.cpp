// image_handler — Windows (WinUI 3) platform partial: a REAL Microsoft.UI.Xaml.Controls.Image whose
// Source is a BitmapImage and whose Stretch carries the mapped aspect. The windows twin of
// src/platform/apple/image_handler.mm (NSImageView) / the android MauiImageView partial, and the
// real-native sibling of the headless mirror (src/platform/headless/image_handler.cpp).
//
// Ported from ImageHandler.cs (the cross-platform source routing lives in src/core/image_handler.cpp)
// + ImageHandler.Windows.cs + Platform/Windows/{AspectExtensions.cs (ToStretch),
// ImageSourcePartExtensions.cs}:
//   - CreatePlatformView: new Image().
//   - MapAspect → UpdateAspect: Image.Stretch = aspect.ToStretch() (AspectFit=Uniform,
//     AspectFill=UniformToFill, Fill=Fill, Center=None).
//   - The FILE source loads synchronously as a BitmapImage over a file:/// URI (the port's file
//     fast-path; C#'s FileImageSourceService resolves the same BitmapImage) and lands on Image.Source.
//   - GetDesiredSize: FrameworkElement.Measure + DesiredSize (an Image's MeasureOverride reports the
//     decoded natural size fitted to the constraints per Stretch — C#'s base.GetDesiredSize).
//
// DOCUMENTED DEVIATIONS (infrastructure gaps of this first cut, not behavior guesses):
//   - FILE results and locally-resolvable URI results (read_uri_bytes only loads file:// / bare paths
//     this cut, so a loaded "uri" result is always a local uri) ride the BitmapImage lane; stream /
//     font results keep the headless mirror only — decoding downloaded/streamed bytes needs the
//     InMemoryRandomAccessStream seam and the glyph raster needs the CanvasImageSource seam
//     (// deferred). The loader pipeline itself (cache + identity recheck) is fully live through the
//     cross-platform map_source.
//   - IsAnimationPlaying mirrors only (// deferred: animated GIF playback — C# drives
//     BitmapImage.Play/Stop); IsOpaque mirrors only (no direct WinUI seat; C# has no windows
//     UpdateIsOpaque either).
//   - ImageHandler.Windows ConnectHandler's ImageOpened IS wired (on_connect_handler below): the
//     BitmapImage decode is asynchronous, so the Image measures 0×0 until the source opens — the
//     opened callback re-pushes IsAnimationPlaying (C# OnImageOpened → UpdateValue) and calls
//     virtual_view()->invalidate_measure() (the port's stand-in for C#'s UpdatePlatformMaxConstraints
//     + XAML's implicit re-layout). C#'s AspectFit intrinsic-size cap itself (GetImageSize over
//     BitmapSource.PixelWidth/PixelHeight, the alignment head-room + MaxWidth/MaxHeight pins) and the
//     Loaded/container re-push are still // deferred with the container fan-out.
//   - query_display_scale keeps the loader's current scale (the headless body); the real
//     XamlRoot.RasterizationScale seam is // deferred with the window plumbing.
//
// XAML-less degradation (the windows twin of the android VM-less fallback): the windows preset also
// runs the cross-platform suite on the host where NO XAML runtime exists; create_platform_view catches
// the construction failure and keeps native null, while the headless mirrors (aspect / source kind +
// file + loaded / opaque / animation) are ALWAYS maintained so that suite observes exactly the
// headless partial's behavior.

#include "maui/core/image_handler.hpp"

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/base.h>

#include "maui/core/dimension.hpp"
#include "maui/core/i_image.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/image_source_loader.hpp" // configure_loader parameter type
#include "maui/core/image_source_result.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "windows_native.hpp"

namespace
{
    namespace mux = winrt::Microsoft::UI::Xaml;
    namespace muxc = winrt::Microsoft::UI::Xaml::Controls;
    namespace muxm = winrt::Microsoft::UI::Xaml::Media;
    namespace muxmi = winrt::Microsoft::UI::Xaml::Media::Imaging;
    namespace wnative = maui::platform::win;

    [[nodiscard]] muxc::Image image_of(const maui::core::image_platform& platform)
    {
        return wnative::borrow<muxc::Image>(platform.native);
    }

    // AspectExtensions.ToStretch: AspectFit → Uniform; AspectFill → UniformToFill; Fill → Fill;
    // Center → None (no scaling).
    [[nodiscard]] muxm::Stretch to_stretch(maui::core::aspect value)
    {
        switch (value)
        {
            case maui::core::aspect::aspect_fill:
                return muxm::Stretch::UniformToFill;
            case maui::core::aspect::fill:
                return muxm::Stretch::Fill;
            case maui::core::aspect::center:
                return muxm::Stretch::None;
            case maui::core::aspect::aspect_fit:
            default:
                return muxm::Stretch::Uniform;
        }
    }

    // A BitmapImage over a file:/// URI for a local path (the FileImageSourceService recipe). The path
    // rides std::filesystem::absolute through the wide-string lane (the mirrors are UTF-8; the
    // filesystem path must not re-interpret them through the ACP). Null on any failure (a missing
    // XAML runtime, a malformed path) — the caller keeps the mirror and skips the native push.
    [[nodiscard]] muxmi::BitmapImage bitmap_from_file(std::string_view file)
    {
        try
        {
            const winrt::hstring wide = wnative::to_hstring_utf8(file);
            std::error_code ec;
            std::filesystem::path absolute = std::filesystem::absolute(std::filesystem::path{std::wstring_view{wide}},
                                                                       ec);
            if (ec)
            {
                absolute = std::filesystem::path{std::wstring_view{wide}};
            }
            const std::wstring uri = L"file:///" + absolute.generic_wstring();
            return muxmi::BitmapImage{winrt::Windows::Foundation::Uri{winrt::hstring{uri}}};
        }
        catch (const winrt::hresult_error&)
        {
            return muxmi::BitmapImage{nullptr};
        }
        catch (const std::exception&)
        {
            return muxmi::BitmapImage{nullptr};
        }
    }

    // A BitmapImage over an already-resolved URI string (the loader's uri lane). read_uri_bytes only
    // resolves file:// uris + bare local paths this cut, so a LOADED "uri" result is always locally
    // decodable: a scheme-carrying detail parses as a Uri directly, a scheme-less detail (a bare-path
    // uri source) reuses the file lane. Null on any parse failure — the caller keeps the mirror.
    [[nodiscard]] muxmi::BitmapImage bitmap_from_uri(std::string_view uri)
    {
        if (!uri.contains("://"))
        {
            return bitmap_from_file(uri);
        }
        try
        {
            return muxmi::BitmapImage{winrt::Windows::Foundation::Uri{wnative::to_hstring_utf8(uri)}};
        }
        catch (const winrt::hresult_error&)
        {
            return muxmi::BitmapImage{nullptr};
        }
    }
} // namespace

namespace maui::core
{
    // Releases the one strong ref pinning the Image (the wnative shape of the pimpl-owned-native
    // doctrine; the apple twin CFReleases its NSImageView here).
    image_platform::~image_platform()
    {
        wnative::release(native);
    }

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each
    // calls the base body FIRST — the headless mirrors must stay live for the XAML-less cross-platform
    // suite (header note) — then pushes to the real Image when one exists. No is_enabled (a WinUI
    // Image is not a Control) and no background (no Background property on a plain Image) — see the
    // header block in image_handler.hpp.

    void image_platform::update_visibility(maui::core::visibility value)
    {
        view_platform_base::update_visibility(value);
        // ViewExtensions.UpdateVisibility (Windows): Hidden rides Opacity 0, Collapsed collapses.
        wnative::apply_visibility(native, value, alpha);
    }

    void image_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        // ViewExtensions.UpdateOpacity: the Hidden state pins opacity 0 (apply_visibility restores it).
        if (!hidden)
        {
            wnative::apply_opacity(native, value);
        }
    }

    void image_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        // ViewExtensions.UpdateAutomationId: AutomationProperties.SetAutomationId.
        wnative::apply_automation_id(native, value);
    }

    std::unique_ptr<image_platform> image_handler::create_platform_view()
    {
        auto platform = std::make_unique<image_platform>();
        try
        {
            // ImageHandler.Windows.CreatePlatformView: new Image(). The Stretch default (Uniform)
            // matches the port's aspect default (aspect_fit).
            const muxc::Image image;
            platform->native = wnative::store(image); // released in ~image_platform
        }
        catch (const winrt::hresult_error&)
        {
            platform->native = nullptr; // XAML-less degradation (header note)
        }
        return platform;
    }

    // ImageHandler.Windows ConnectHandler: platformView.ImageOpened += OnImageOpened. A BitmapImage
    // decodes ASYNCHRONOUSLY — the Image measures 0×0 until the source opens — so the opened callback
    // re-pushes IsAnimationPlaying (C# OnImageOpened → UpdateValue(nameof(IImage.IsAnimationPlaying)),
    // behind the same still-connected guard) and re-runs layout via invalidate_measure (the port's
    // stand-in for C#'s UpdatePlatformMaxConstraints + XAML's own post-open re-layout — the Canvas
    // model gives XAML no layout authority, so the C++ side must re-measure). The `this` capture is
    // safe: on_disconnect_handler revokes the token before the handler can die (the button partial's
    // stable-peer argument). No-op XAML-less (null native — header note).
    void image_handler::on_connect_handler(image_platform& platform)
    {
        auto image = image_of(platform);
        if (image == nullptr)
        {
            return;
        }
        const winrt::event_token token =
            image.ImageOpened([this](const winrt::Windows::Foundation::IInspectable&, const mux::RoutedEventArgs&) {
                auto* view = virtual_view();
                if (view == nullptr)
                {
                    return; // disconnected mid-flight (C#'s this.IsConnected() guard)
                }
                map_is_animation_playing(*this, *view);
                view->invalidate_measure();
            });
        platform.image_opened_token = token.value;
        // deferred: the Loaded → container re-push (MapAspect's OnImageLoaded) rides the container
        // fan-out (header deviations).
    }

    // ImageHandler.Windows DisconnectHandler: platformView.ImageOpened -= OnImageOpened (the Loaded
    // unhook rides the deferred container fan-out; SourceLoader.Reset's cancel rides the loader's own
    // teardown with the handler).
    void image_handler::on_disconnect_handler(image_platform& platform)
    {
        if (platform.image_opened_token != 0)
        {
            if (auto image = image_of(platform))
            {
                image.ImageOpened(winrt::event_token{platform.image_opened_token});
            }
            platform.image_opened_token = 0;
        }
    }

    // Leave the loader on its defaults (the synchronous read_uri_bytes fetch; disk layer off) — the
    // headless twin's wiring. deferred: an async HttpClient/RandomAccessStream uri fetch + the local
    // app-data disk-cache directory (the apple NSURLSession twin).
    void image_handler::configure_loader(image_source_loader& /*loader*/)
    {
    }

    void image_handler::map_aspect(image_handler& handler, i_image& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->image_aspect = view.aspect(); // the headless mirror
        // ImageHandler.Windows.MapAspect → ImageExtensions.UpdateAspect: Image.Stretch =
        // aspect.ToStretch(). (C#'s container/Loaded re-push rides the deferred container fan-out.)
        if (auto image = image_of(*platform))
        {
            image.Stretch(to_stretch(view.aspect()));
        }
    }

    // IsOpaque (headless mirror only): C# has no windows UpdateIsOpaque — the flag is a renderer hint
    // with no WinUI Image seat.
    void image_handler::map_is_opaque(image_handler& handler, i_image& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->opaque = view.is_opaque();
        }
    }

    // IsAnimationPlaying (headless mirror): C#'s UpdateIsAnimationPlaying drives BitmapImage.Play()/
    // Stop() for a multi-frame (GIF) source. deferred: the port's file fast-path BitmapImage carries
    // the flagged state once GIF sources land on this backend.
    void image_handler::map_is_animation_playing(image_handler& handler, i_image& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->animation_playing = view.is_animation_playing();
        }
    }

    // ---- per-backend source primitives (the cross-platform map_source routes here) ----

    // File fast-path: the mirror is ALWAYS maintained (kind="file" + path, marked loaded — the
    // XAML-less suite observes it); the native push decodes a BitmapImage over the file:/// URI and
    // lands it on Image.Source (the FileImageSourceService recipe).
    void image_handler::load_file_source_sync(image_platform& platform, const i_file_image_source& file_src)
    {
        platform.source_kind = "file";
        platform.source_file = std::string(file_src.file());
        platform.source_loaded = true;
        auto image = image_of(platform);
        if (image == nullptr)
        {
            return;
        }
        if (auto bitmap = bitmap_from_file(file_src.file()))
        {
            image.Source(bitmap);
        }
    }

    // The async loader's apply: copy the result's kind + detail into the mirror (a !loaded() result
    // clears, mirroring SetImageSource(null)). FILE and URI results reach the native Image (the
    // result's detail is the path/uri — both decode over the same BitmapImage-over-Uri lane, the
    // ImageImageSourcePartSetter.SetImageSource push); stream/font byte decodes are deferred (header).
    void image_handler::apply_loaded_result(image_platform& platform, const image_source_result& result)
    {
        if (!result.loaded())
        {
            clear_source_native(platform);
            return;
        }
        platform.source_kind = result.kind();
        platform.source_file = result.detail();
        platform.source_loaded = true;
        if (result.kind() == "file" || result.kind() == "uri")
        {
            auto image = image_of(platform);
            if (image == nullptr)
            {
                return;
            }
            const muxmi::BitmapImage bitmap =
                result.kind() == "file" ? bitmap_from_file(result.detail()) : bitmap_from_uri(result.detail());
            if (bitmap != nullptr)
            {
                image.Source(bitmap);
            }
            return;
        }
        // deferred: stream results decode via an InMemoryRandomAccessStream + BitmapImage.SetSource;
        // font results raster via a CanvasImageSource (FontImageSourceService.Windows) — the mirror
        // above keeps the load observable.
    }

    // Clear the loaded image: mirrors cleared + Image.Source = null (C#'s SetImageSource(null)).
    void image_handler::clear_source_native(image_platform& platform)
    {
        platform.source_kind.clear();
        platform.source_file.clear();
        platform.source_loaded = false;
        if (auto image = image_of(platform))
        {
            image.Source(nullptr);
        }
    }

    maui::graphics::size image_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return {0, 0};
        }
        if (platform->native == nullptr)
        {
            // XAML-less degradation: no decode happens, so there is no intrinsic content size to
            // report (the headless twin's body).
            return {0, 0};
        }
        // ViewHandlerExtensions.Windows.cs GetDesiredSizeFromHandler: FrameworkElement.Measure +
        // DesiredSize — an Image measures its decoded natural size fitted to the constraints per
        // Stretch (C#'s base.GetDesiredSize; the AspectFit intrinsic cap is deferred, header). The
        // explicit width/height feed the AdjustForExplicitSize clamp like every windows partial.
        const double explicit_width = virtual_view() != nullptr ? virtual_view()->width() : dimension::unset;
        const double explicit_height = virtual_view() != nullptr ? virtual_view()->height() : dimension::unset;
        return wnative::measure_native(platform->native, width_constraint, height_constraint, explicit_width,
                                       explicit_height);
    }

    void image_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // XAML-less: no native layout to apply
        }
        // The shared Canvas recipe: Canvas.SetLeft/SetTop + explicit Width/Height pin the Image to the
        // frame (ViewHandlerExtensions.Windows.cs PlatformArrangeHandler on the Canvas layout model).
        wnative::arrange_native(platform->native, frame);
    }

    // No live display seam yet: pass the loader's currently-set scale back through (so map_source's
    // refresh_display_scale is a no-op that preserves whatever a test set via
    // source_loader().set_scale()) — the headless body. deferred: the real XamlRoot.RasterizationScale
    // read (the apple/ios query_display_scale twins) with the window plumbing.
    float image_handler::query_display_scale() const
    {
        return source_loader_.scale();
    }
} // namespace maui::core
