// image_handler — WinUI 3 platform recipe: a real Microsoft.UI.Xaml.Controls.Image, the same native type
// ImageHandler.Windows.cs creates. Ported from ImageHandler.Windows.cs + the Windows halves of
// ImageViewExtensions.cs / AspectExtensions.cs / ImageSourcePartExtensions.cs +
// FileImageSourceService.Windows.cs (the file-resolution recipe, adapted — see below).
//
// FOUR DOCUMENTED DEVIATIONS from the C# oracle, each an infrastructure gap or an unpackaged-exe
// consequence, not a behavior guess:
//
//  1. UNPACKAGED FILE RESOLUTION. FileImageSourceService.Windows.cs's GetAppPackage resolves a bare
//     filename (from_file("dotnet_bot.png")) via `new BitmapImage(new Uri("ms-appx:///" + name))` —
//     `ms-appx://` needs a PACKAGE IDENTITY (MSIX) this exe does not have (host_run.cpp's own
//     MddBootstrapInitialize2 dance is precisely the unpackaged workaround for XAML activation; there is
//     no equivalent for `ms-appx:` resource resolution). maui_add_app.cmake's windows branch instead
//     copies RESOURCES FLAT next to the exe (the same convention the apple/headless backends resolve
//     against the CWD), so a bare filename here resolves against the EXE'S OWN DIRECTORY
//     (GetModuleFileNameW) instead of the package root — see resolve_file_uri. An already-rooted path
//     (Path.IsPathRooted true) is used as-is, taking the same branch GetLocal does for a rooted path,
//     just via a `file:///` Uri + the BitmapImage(Uri) ctor instead of StorageFile+OpenReadAsync+
//     SetSourceAsync (both end up decoding the same bytes into a BitmapImage; the Uri ctor needs no
//     coroutine machinery, which load_file_source_sync's synchronous contract does not have room for).
//  2. ASYNC DECODE — ImageOpened NOW WIRED, UpdatePlatformMaxConstraints NOW PORTED. BitmapImage decoding is
//     ALWAYS asynchronous in WinUI — there is no synchronous decode API, unlike apple's NSImage — so
//     get_desired_size may read {0,0} on the very first Measure (the decode has not completed yet).
//     ImageHandler.Windows.cs subscribes ImageOpened precisely to react to this (OnImageOpened ->
//     UpdatePlatformMaxConstraints); this backend does the same subscribe/unsubscribe (on_connect_handler/
//     on_disconnect_handler below, token-revoked like button_handler.cpp's Click). The callback now drives
//     BOTH oracle consequences: update_platform_max_constraints() (this file's port of
//     UpdatePlatformMaxConstraints, ImageHandler.Windows.cs:234-261 — caps the child Image's own
//     MaxWidth/MaxHeight to the decoded intrinsic size on a non-Fill AspectFit axis) ALONGSIDE the
//     pre-existing call to the virtual view's invalidate_measure() — window::request_relayout /
//     set_relayout_hook (window.hpp), installed by this backend's host_run.cpp right after its first
//     drive_layout. That seam is what a real MAUI backend gets FOR FREE (its native OS-owned layout tree
//     bubbles a dirty flag to the root on its own); this port's own C++-side desired_size_ cache does not
//     auto-correct just because the NATIVE WinUI Image re-measures itself internally post-decode, so an
//     explicit replay of THIS port's drive_layout is still needed alongside the new Max* cap — see
//     load_file_source_sync/apply_loaded_result below for the belt-and-braces case (an already-decoded
//     source) and this file's on_connect_handler/on_disconnect_handler for the subscribe/teardown.
//     MapSourceAsync's reset-to-PositiveInfinity (oracle :208-213) is also ported, in
//     reset_platform_max_constraints (anonymous namespace below), called from each of the three source-
//     application primitives (load_file_source_sync / apply_loaded_result / clear_source_native) so a
//     smaller PREVIOUS source's cap cannot leak onto a larger NEW one.
//  3. FONT SOURCES STAY MIRROR-ONLY (URI/STREAM ARE NOW REAL). image_source_services.cpp (swapped in via
//     MAUI_WINDOWS_SWAPS) decodes a uri/stream source into a genuine BitmapImage the same way the FILE fast
//     path below does (spool-to-temp-file + the Uri ctor — see that file's header for why not
//     SetSourceAsync().get()); apply_loaded_result pushes result.image() onto the control's Source exactly
//     like the apple twin. FONT stays mirror-only: FontImageSourceService.Windows.cs rasterizes the glyph
//     via Win2D (Microsoft.Graphics.Canvas), a native dependency this port does not link on any backend —
//     see image_source_services.cpp's header for the full citation.
//  4. IsOpaque stays a mirror (no WinUI analog — Image is a FrameworkElement with nothing resembling
//     C#'s opaque hint, matching the android partial's identical gap).
//
// CONTAINER (NeedsContainer / SetupContainer / MapHeight / MapWidth) — ImageHandler.Windows.cs:34-38
// (NeedsContainer), :40-50 (SetupContainer), :93-106 (MapHeight), :108-123 (MapWidth). The oracle's
// NeedsContainer is CONDITIONAL:
//   VirtualView?.Background != null || VirtualView?.Aspect == Aspect.AspectFill || base.NeedsContainer
// (base.NeedsContainer is ViewExtensions.cs:111-114's Clip/Shadow check — this page sets neither). Checked
// against every <Image> on port/maui-reference/pages/image.xaml: 5 of its 8 images set BackgroundColor
// (dotnet_bot.png/Purple, both FontImageSource/Green rows, stream.png/LightPink, dotnet_bot.png/Black+
// Opacity) => NeedsContainer TRUE; the other 3 (the UriSource row, the WidthRequest=200 animated-gif row,
// and the plain animated-gif row) set no Background and use the default Aspect.AspectFit (not
// AspectFill) => NeedsContainer FALSE for all three.
//
// This port wraps the Image in a Border host UNCONDITIONALLY instead of reproducing the conditional
// attach/detach — the same simplification label_handler.cpp already made for LabelHandler.Windows's own
// conditional NeedsContainer, for the identical reason: a chromeless Border (Padding/BorderThickness/
// CornerRadius never touched) measures and arranges IDENTICALLY to its bare child, so the 3 images that
// would stay unwrapped in real MAUI render pixel-for-pixel the same wrapped or bare. The 5 Background-
// bearing images gain a real paint target: apply_background's existing three-way try_as (winui_visual_
// ops.cpp) already fills a Border, so no change was needed there — this closes the gap image_handler.hpp's
// update_background note used to document (Background wired but "has nowhere to land").
//
// MapHeight/MapWidth have no direct analog to port: the cross-platform port has no per-property
// "height"/"width" mapper key for View at all (only window_handler.cpp has a window-level one) —
// WidthRequest/HeightRequest instead flow into get_desired_size's width_constraint/height_constraint
// through the shared cross-platform layout math, and the one resolved frame is pushed once from
// platform_arrange. So the oracle's "the container gets the size, the child stays Auto" is realized
// structurally instead: get_desired_size and platform_arrange now size/position the HOST border (exactly
// like label_handler.cpp's as_host/get_desired_size/platform_arrange), and the child Image's own Width/
// Height are never touched at all — left permanently at WinUI's own Auto/NaN default, which is what
// SetupContainer's `PlatformView.Height = Dimension.Unset` achieves in the oracle.

#include "maui/core/image_handler.hpp"

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.h>

#include <windows.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "maui/core/aspect.hpp"
#include "maui/core/i_image.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/image_source_loader.hpp"
#include "maui/core/image_source_result.hpp"
#include "maui/core/layout_alignment.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "winui_interop.hpp"
#include "winui_visual_ops.hpp"

namespace
{
    // Named `winui`, NOT `xaml`: the port already has a maui::xaml namespace (the XAML loader), and
    // inside namespace maui::* that name WINS over a file-scope alias - an `xaml::Application` here
    // would resolve to maui::xaml and fail with "'Start': is not a member of 'maui::xaml'".
    namespace winui = winrt::Microsoft::UI::Xaml;
    using image_control = winui::Controls::Image;
    using bitmap_image = winui::Media::Imaging::BitmapImage;
    using border = winui::Controls::Border;

    // The container host — see this file's header CONTAINER note. `native` now boxes the BORDER, not the
    // bare Image; the label_handler.cpp twin (as_host/as_text_block) is the precedent this mirrors.
    border as_host(void* native)
    {
        return maui::platform::windows::ref<winui::UIElement>(native).as<border>();
    }

    image_control as_image(void* native)
    {
        return as_host(native).Child().as<image_control>();
    }

    // AspectExtensions.ToStretch: the enum member names match 1:1 (WStretch = Microsoft.UI.Xaml.Media.
    // Stretch in the oracle), so this is a direct translation, not a guess.
    winui::Media::Stretch to_stretch(maui::core::aspect value)
    {
        switch (value)
        {
            case maui::core::aspect::aspect_fit:
                return winui::Media::Stretch::Uniform;
            case maui::core::aspect::aspect_fill:
                return winui::Media::Stretch::UniformToFill;
            case maui::core::aspect::fill:
                return winui::Media::Stretch::Fill;
            case maui::core::aspect::center:
                return winui::Media::Stretch::None;
        }
        return winui::Media::Stretch::Uniform;
    }

    // A scheme already present passes straight through to Uri (http/https for a real remote source that
    // slipped in as a "file" source, ms-appx for a caller that already built a packaged-style path, file
    // for an already-fully-formed local uri).
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

    // Path.IsPathRooted's two shapes: a drive-letter path ("C:\..." / "C:/...") or a UNC/rooted path
    // ("\\server\share\..." or "\..."). FileImageSourceService.Windows.cs's GetLocal takes exactly this
    // branch for an already-absolute path (StorageFile.GetFileFromPathAsync); this port takes the
    // equivalent branch in resolve_file_uri below (use the path as-is, no exe-directory join).
    bool is_rooted(const std::wstring& path)
    {
        if (path.size() >= 2 && path[1] == L':')
        {
            return true;
        }
        return path.starts_with(L"\\") || path.starts_with(L"/");
    }

    // The running exe's own directory (GetModuleFileNameW then strip the filename) — where maui_add_app.
    // cmake's windows branch copies an example's RESOURCES (see this file's header note 1). Empty on any
    // WinAPI failure, which resolve_file_uri treats as "use the bare relative path" (best effort).
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
            buffer.resize(buffer.size() * 2); // truncated (an exotic, very long install path) - grow + retry
        }
        const auto slash = buffer.find_last_of(L"\\/");
        return slash == std::wstring::npos ? std::wstring{} : buffer.substr(0, slash);
    }

    // Build a `file:///` Uri string from an absolute Windows path. Forward slashes throughout: Uri parses
    // either, but this keeps the scheme + path unambiguous (no backslash-vs-escape questions).
    winrt::hstring to_file_uri(std::wstring path)
    {
        std::ranges::replace(path, L'\\', L'/');
        return winrt::hstring{L"file:///" + path};
    }

    // Resolve a from_file() path (relative or rooted, or an already-schemed uri) into a loadable Uri —
    // see this file's header note 1 for why a bare filename resolves against the EXE directory rather
    // than `ms-appx:///`.
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
} // namespace

namespace maui::core
{
    namespace
    {
        // Unhook the ImageOpened subscription on_connect_handler registered — the button_handler.cpp
        // detach_native_events convention. Called from on_disconnect_handler AND from ~image_platform: the
        // native lambda captures only `self` (a image_platform*, not the handler), but the platform struct
        // itself can still be destroyed while the token is live (a handler torn down without a disconnect,
        // which the element tree does on shutdown) — the next ImageOpened firing would then touch freed
        // memory, so both teardown paths revoke it.
        void detach_native_events(maui::core::image_platform& platform)
        {
            if (platform.native != nullptr && platform.image_opened_token != 0)
            {
                as_image(platform.native).ImageOpened(winrt::event_token{platform.image_opened_token});
            }
            platform.image_opened_token = 0;
        }

        // Belt-and-braces for concern 3 (an already-decoded/cached BitmapImage assigned as a NEW Source):
        // on_connect_handler's comment argues the INITIAL source cannot race the subscription (this port's
        // view_handler.hpp always runs on_connect_handler before the first map_source), but that argument
        // does not cover a LATER source change reusing an already-opened BitmapImage instance, where it is
        // not certain WinUI re-raises Image.ImageOpened for the reassigned source. Checking PixelWidth/
        // Height right after the assignment and invoking the same hook directly closes that gap without
        // depending on the event at all. Harmless if ImageOpened ALSO fires afterward for the same
        // assignment — invalidate_measure() is safe to call twice (on_connect_handler's termination note:
        // each call independently settles in at most one extra drive_layout() pass).
        void notify_if_already_open(image_platform& platform, const image_control& image)
        {
            const auto bitmap = image.Source().try_as<bitmap_image>();
            if (bitmap && bitmap.PixelWidth() > 0 && bitmap.PixelHeight() > 0 && platform.on_image_opened)
            {
                platform.on_image_opened();
            }
        }

        // ImageHandler.Windows.cs's private GetImageSize (oracle
        // src/Core/src/Handlers/Image/ImageHandler.Windows.cs:263-275): the decoded bitmap's PixelWidth/
        // PixelHeight, or {0,0} when not yet decoded -- BitmapSource does not populate PixelWidth/
        // PixelHeight until the image has opened, the same guard notify_if_already_open above already
        // tests for. Used by update_platform_max_constraints below.
        maui::graphics::size get_image_size(const image_control& image)
        {
            const auto bitmap = image.Source().try_as<bitmap_image>();
            if (bitmap && bitmap.PixelWidth() > 0 && bitmap.PixelHeight() > 0)
            {
                return {static_cast<double>(bitmap.PixelWidth()), static_cast<double>(bitmap.PixelHeight())};
            }
            return maui::graphics::size::zero;
        }

        // MapSourceAsync's reset (oracle src/Core/src/Handlers/Image/ImageHandler.Windows.cs:208-213):
        // `PlatformView.MaxWidth = double.PositiveInfinity; PlatformView.MaxHeight = double.PositiveInfinity;`
        // run before EVERY new source is applied, so a smaller PREVIOUS AspectFit source's cap (set by
        // update_platform_max_constraints, above) cannot leak onto a larger NEW one that hasn't decoded (and
        // so hasn't had a chance to re-cap) yet. Called from each of the three source-application primitives
        // below (load_file_source_sync / apply_loaded_result / clear_source_native) -- together those are
        // every place THIS backend ever assigns a new (or null) Source, i.e. "each load" for this port.
        void reset_platform_max_constraints(const image_control& image)
        {
            image.MaxWidth(std::numeric_limits<double>::infinity());
            image.MaxHeight(std::numeric_limits<double>::infinity());
        }
    } // namespace

    image_platform::~image_platform()
    {
        detach_native_events(*this);
        if (native != nullptr)
        {
            maui::platform::windows::drop<winui::UIElement>(native);
        }
    }

    std::unique_ptr<image_platform> image_handler::create_platform_view()
    {
        auto platform = std::make_unique<image_platform>();
        // Border host, unconditionally — see this file's header CONTAINER note for why (mirrors
        // label_handler.cpp's create_platform_view exactly: a chromeless Border wrapping the real
        // control). The Image's own Width/Height are never set anywhere in this file; they stay at
        // WinUI's Auto/NaN default permanently, which is the oracle's SetupContainer effect.
        border host;
        image_control image;
        host.Child(image);
        platform->native = maui::platform::windows::take<winui::UIElement>(host);
        return platform;
    }

    // ImageHandler.Windows.cs's ConnectHandler: `platformView.ImageOpened += OnImageOpened;` — subscribed
    // here, BEFORE the property mapper's first update_properties() pass (view_handler.hpp's
    // set_virtual_view calls on_connect_handler, THEN property_mapper_->update_properties(), which is what
    // runs map_source), so this attaches before ANY Source is ever set on this freshly-created Image —
    // exactly like C#'s CreatePlatformView() handing back a source-less `new WImage()` before ConnectHandler
    // subscribes. That ordering is why a genuine "decode completed before we subscribed" race cannot occur
    // on the INITIAL source; see image_handler.cpp's header note 2 + load_file_source_sync/
    // apply_loaded_result's belt-and-braces check below for the residual (already-decoded/cached source)
    // case this ordering argument does not, by itself, rule out.
    //
    // The callback's `this->IsConnected()` C# guard becomes `virtual_view() != nullptr` here — view_handler
    // ::disconnect_handler() clears virtual_view_ before the platform view is torn down, so a callback that
    // fires after disconnect (queued on the dispatcher before revoke ran) is a safe no-op rather than a
    // stale push. TERMINATION: ImageOpened fires AT MOST ONCE per successfully-decoded source (a WinUI
    // guarantee — decode-complete is a one-time transition per BitmapImage, and re-measuring the Image via
    // get_desired_size's Measure() call does not restart the decode or re-raise the event), and it is
    // delivered via the UI-thread dispatcher, never synchronously re-entrant inside UIElement::Measure() —
    // so the invalidate_measure() this triggers can drive at most ONE extra drive_layout() pass per source
    // load, which itself cannot trigger a second ImageOpened. It does not loop.
    void image_handler::on_connect_handler(image_platform& platform)
    {
        platform.on_image_opened = [this] {
            // OnImageOpened (oracle :218-227): the oracle calls UpdatePlatformMaxConstraints() here so an
            // AspectFit image's non-Fill axis is capped to its now-decoded intrinsic size. Added ALONGSIDE
            // the existing invalidate_measure() (this port's own out-of-cycle re-layout trigger, see this
            // file's header note 2) -- not in place of it.
            update_platform_max_constraints();
            if (auto* view = virtual_view())
            {
                view->invalidate_measure();
            }
        };
        if (platform.native == nullptr)
        {
            return;
        }
        auto* self = &platform;
        platform.image_opened_token =
            as_image(platform.native)
                .ImageOpened([self](const winrt::Windows::Foundation::IInspectable&, const winui::RoutedEventArgs&) {
                    if (self->on_image_opened)
                    {
                        self->on_image_opened();
                    }
                })
                .value;
    }

    void image_handler::on_disconnect_handler(image_platform& platform)
    {
        detach_native_events(platform);
        platform.on_image_opened = nullptr;
    }

    // ImageHandler.Windows.cs's private UpdatePlatformMaxConstraints (oracle
    // src/Core/src/Handlers/Image/ImageHandler.Windows.cs:234-261), ported 1:1. Caps the CHILD Image's own
    // MaxWidth/MaxHeight -- see image_handler.hpp's declaration comment for why the host is not touched.
    void image_handler::update_platform_max_constraints()
    {
        auto* platform = typed_platform_view();
        auto* view = virtual_view();
        // Oracle guard (:236-237): `if (PlatformView is null || VirtualView is null) return;`.
        if (platform == nullptr || platform->native == nullptr || view == nullptr)
        {
            return;
        }

        const image_control image = as_image(platform->native);

        if (view->aspect() == maui::core::aspect::aspect_fit)
        {
            const auto sz = get_image_size(image);

            // Width: cap to intrinsic only if horizontal alignment isn't Fill (oracle :244-247).
            if (view->horizontal_layout_alignment() != maui::core::layout_alignment::fill && sz.width > 0)
            {
                image.MaxWidth(std::min(sz.width, view->maximum_width()));
            }
            else
            {
                image.MaxWidth(view->maximum_width());
            }

            // Height: cap to intrinsic only if vertical alignment isn't Fill (oracle :249-253).
            if (view->vertical_layout_alignment() != maui::core::layout_alignment::fill && sz.height > 0)
            {
                image.MaxHeight(std::min(sz.height, view->maximum_height()));
            }
            else
            {
                image.MaxHeight(view->maximum_height());
            }

            return;
        }

        // Non-AspectFit: mirror the view's declared maximums (oracle :258-260).
        image.MaxWidth(view->maximum_width());
        image.MaxHeight(view->maximum_height());
    }

    // Headless-style no-op: see this file's header note 3 — the async loader's uri/stream/font path
    // still resolves against the (unswapped) headless image_source_service_registry defaults
    // (synchronous read_uri_bytes, no disk cache).
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
        platform->image_aspect = view.aspect(); // headless-style mirror, kept live on every backend
        if (platform->native == nullptr)
        {
            return;
        }
        // ImageViewExtensions.UpdateAspect (Windows): Stretch := aspect.ToStretch(); AspectFill
        // ADDITIONALLY centers both axes. Ported 1:1, including that every OTHER aspect leaves whatever
        // alignment a prior AspectFill left behind — C# does not reset it either.
        const image_control image = as_image(platform->native);
        image.Stretch(to_stretch(platform->image_aspect));
        if (platform->image_aspect == maui::core::aspect::aspect_fill)
        {
            image.HorizontalAlignment(winui::HorizontalAlignment::Center);
            image.VerticalAlignment(winui::VerticalAlignment::Center);
        }
    }

    // IsOpaque: mirror only — see this file's header note 4.
    void image_handler::map_is_opaque(image_handler& handler, i_image& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->opaque = view.is_opaque();
        }
    }

    void image_handler::map_is_animation_playing(image_handler& handler, i_image& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->animation_playing = view.is_animation_playing();
        if (platform->native == nullptr)
        {
            return;
        }
        // ImageViewExtensions.UpdateIsAnimationPlaying: `imageView.Source is BitmapImage bitmapImage &&
        // bitmapImage.IsAnimatedBitmap` gates real multi-frame (GIF) playback; anything else (no source
        // yet, a static image, or — since uri/stream/font sources stay mirror-only here, see
        // apply_loaded_result — a source with no real BitmapImage at all) is a no-op, matching C#'s guard.
        const auto source = as_image(platform->native).Source();
        if (source == nullptr)
        {
            return;
        }
        const auto bitmap = source.try_as<bitmap_image>();
        if (!bitmap || !bitmap.IsAnimatedBitmap())
        {
            return;
        }
        if (platform->animation_playing)
        {
            if (!bitmap.IsPlaying())
            {
                bitmap.Play();
            }
        }
        else if (bitmap.IsPlaying())
        {
            bitmap.Stop();
        }
    }

    // ---- per-backend source primitives (the cross-platform map_source in image_handler.cpp routes here) ----

    // File fast-path: resolve the path (see resolve_file_uri + this file's header note 1) and decode it
    // via the BitmapImage(Uri) ctor — the same bytes FileImageSourceService.Windows.cs's GetLocal decodes
    // via StorageFile+OpenReadAsync+SetSourceAsync, just without the coroutine machinery this synchronous
    // seam has no room for (see note 1). The decode itself is still asynchronous (note 2) — WinUI has no
    // synchronous image decode at all — so this call returns before the bytes are necessarily ready.
    void image_handler::load_file_source_sync(image_platform& platform, const i_file_image_source& file_src)
    {
        platform.source_kind = "file";
        platform.source_file = std::string(file_src.file());
        platform.source_loaded = true;
        if (platform.native == nullptr)
        {
            return;
        }
        const image_control image = as_image(platform.native);
        reset_platform_max_constraints(image);
        try
        {
            image.Source(bitmap_image{resolve_file_uri(platform.source_file)});
            notify_if_already_open(platform, image);
        }
        catch (const winrt::hresult_error&)
        {
            // A malformed path/uri (Uri's ctor throws) - clear rather than leave a stale source, mirroring
            // the apple partial's nil-on-failed-load (load_image_from_file returning nil -> image = nil).
            image.Source(nullptr);
        }
    }

    // The async loader's apply: see this file's header note 3. A !loaded() result clears the view (the
    // SetImageSource(null) analog); a loaded result always updates the string mirrors, and — when it
    // carries a real native handle (uri/stream; not font, see note 3) — pushes it onto the control's
    // Source, mirroring the apple twin's `as_image_view(...).image = ...`.
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
        if (platform.native != nullptr && result.image() != nullptr)
        {
            const image_control image = as_image(platform.native);
            reset_platform_max_constraints(image);
            image.Source(maui::platform::windows::ref<bitmap_image>(result.image()));
            notify_if_already_open(platform, image);
        }
    }

    // Clear the loaded image (both the mirror and, when a native view exists, the real Source).
    void image_handler::clear_source_native(image_platform& platform)
    {
        platform.source_kind.clear();
        platform.source_file.clear();
        platform.source_loaded = false;
        if (platform.native != nullptr)
        {
            const image_control image = as_image(platform.native);
            reset_platform_max_constraints(image);
            image.Source(nullptr);
        }
    }

    maui::graphics::size image_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        // GetDesiredSizeFromHandler's first guard: a negative constraint measures to nothing. XAML's
        // Measure THROWS on a negative Size, so this is a crash guard, not a formality.
        if (width_constraint < 0 || height_constraint < 0)
        {
            return {0, 0};
        }
        // The REAL native measure - the whole point of a native backend: with Stretch=Uniform (AspectFit)
        // WinUI's own Image.MeasureOverride already fits the intrinsic size to the constraint, so unlike
        // the apple partial there is no hand-rolled aspect-fit clamp needed here. See this file's header
        // note 2 for why the reported size can be {0,0} on the very first call (the decode has not
        // completed yet) — the E2E/parity harness's post-launch resize (host_run.cpp's SizeChanged ->
        // drive_layout) supplies the second pass a real app already gets for free.
        //
        // Measure the HOST, not the bare Image — the container port (this file's header CONTAINER note):
        // exactly like label_handler.cpp's get_desired_size, the returned size is what platform_arrange
        // will size/position, and that is now the Border host (a chromeless Border's DesiredSize equals
        // its child's, so this is not expected to change any number, only WHICH element the port measures
        // and arranges).
        const border host = as_host(platform->native);
        const image_control image = as_image(platform->native);
        // Clear the pinned size FIRST, exactly like label/button: platform_arrange stamps an explicit
        // Width/Height on this element every pass (a Canvas child has no other way to be sized), and a
        // FrameworkElement with an explicit Width/Height measures to exactly that instead of re-measuring.
        // Only the HOST is ever pinned now — the child Image's own Width/Height are never set anywhere in
        // this file (SetupContainer's `PlatformView.Height = Dimension.Unset`, permanently).
        const auto auto_size = std::numeric_limits<double>::quiet_NaN();
        host.Width(auto_size);
        host.Height(auto_size);
        // Force MeasureOverride to actually RUN. Measure() is a no-op on an element XAML does not consider
        // measure-dirty: it returns the cached DesiredSize instead of re-deriving one. This port drives its
        // own layout out-of-cycle, so nothing else ever marks the element dirty, and the FIRST measure here
        // happens while the BitmapImage is still decoding -- caching 0x0. Measured on the guest: 20 passes
        // observing a fully decoded bitmap (pixel=1200x694, 400x300) all still returned desired=0x0, which
        // is why waiting longer for the decode changed nothing. Both host and child are invalidated: a
        // fresh Border ancestor does not, by itself, guarantee WinUI treats the already-existing child as
        // measure-dirty too.
        host.InvalidateMeasure();
        image.InvalidateMeasure();
        host.Measure(winrt::Windows::Foundation::Size{maui::platform::windows::measure_constraint(width_constraint),
                                                      maui::platform::windows::measure_constraint(height_constraint)});
        const auto desired = host.DesiredSize();
        // TEMPORARY DIAGNOSTIC (env-gated, remove once the `image` collapse is understood). Eighteen
        // hypotheses about this page have died on captures, and every one of them was an argument from
        // reading source. This prints what the measure ACTUALLY observes at each pass, which distinguishes
        // the two remaining stories that source-reading cannot separate:
        //   * bitmap present, measure ignores it   -> pixel_w/h > 0 but desired == 0
        //   * bitmap absent at every pass we make  -> pixel_w/h == 0, i.e. the decode really never lands
        //     before the last layout, and image_handler.cpp note 2's "the harness resize supplies a second
        //     pass" is false in practice.
        // The measured element is now the HOST border rather than the bare Image (this file's CONTAINER
        // note), so every field below is logged for BOTH: host_* is what get_desired_size actually returns
        // and what the next layout pass sees; img_* is the bare child's own MeasureOverride result, read
        // off the SAME host.Measure() call (Border.MeasureOverride cascades into Child.Measure()), kept for
        // direct comparison against every prior capture of this log (which only ever recorded the image).
        if (const char* const path = std::getenv("MAUI_WINUI_LOG"))
        {
            std::int32_t pixel_w = -1;
            std::int32_t pixel_h = -1;
            if (const auto bitmap = image.Source().try_as<bitmap_image>())
            {
                pixel_w = bitmap.PixelWidth();
                pixel_h = bitmap.PixelHeight();
            }
            // The bare child's own DesiredSize, populated as a side effect of the host.Measure() call just
            // above — captured BEFORE the finite re-probe below overwrites it.
            const auto img_desired = image.DesiredSize();
            // THE PROBE: re-measure with a FINITE height. WinUI's Uniform scale is
            // min(availW/natW, availH/natH); an implementation that special-cases an infinite axis would
            // return 0 for the unbounded call while every input is valid -- which is the one story left
            // standing after fourteen dead hypotheses (fresh binary, decoded bitmap, correct constraint,
            // live tree, default Max*). If finite_desired is non-zero while desired is 0x0, that is the
            // whole bug and the fix is to bound the measure, not to chase the decode. Run on the host
            // (what actually feeds the return value) AND the bare image (the original probe target).
            host.InvalidateMeasure();
            image.InvalidateMeasure();
            host.Measure(winrt::Windows::Foundation::Size{maui::platform::windows::measure_constraint(width_constraint),
                                                          10000.0F});
            const auto host_finite = host.DesiredSize();
            const auto img_finite = image.DesiredSize();
            std::FILE* file = nullptr;
            if (fopen_s(&file, path, "a") == 0 && file != nullptr)
            {
                std::fprintf(
                    file,
                    "image.measure cw=%.1f ch=%.1f -> host_desired=%.1fx%.1f host_finite=%.1fx%.1f "
                    "img_desired=%.1fx%.1f img_finite=%.1fx%.1f actual=%.1fx%.1f pixel=%dx%d src=%d vis=%d "
                    "parent=%d host_wh=%.1fx%.1f img_wh=%.1fx%.1f stretch=%d\n",
                    width_constraint, height_constraint, desired.Width, desired.Height, host_finite.Width,
                    host_finite.Height, img_desired.Width, img_desired.Height, img_finite.Width, img_finite.Height,
                    image.ActualWidth(), image.ActualHeight(), pixel_w, pixel_h, image.Source() != nullptr ? 1 : 0,
                    // A Collapsed UIElement measures to 0x0 UNCONDITIONALLY, whatever its content --
                    // the one cause that fits every observation here (decoded bitmap, valid
                    // constraints, live tree, and InvalidateMeasure/settle/finite-height all inert).
                    image.Visibility() == winui::Visibility::Visible ? 1 : 0, image.Parent() != nullptr ? 1 : 0,
                    // Read the pin BACK after clearing it to NaN. Every measure path in this backend
                    // assumes that clear takes effect; none has ever verified it, and actual=920.0x0.0 is
                    // exactly the residue a height pinned to 0 would leave. A Height of 0 (rather than NaN)
                    // makes Measure return 0x0 for ANY content, which is the last unexamined way to get the
                    // observed result. host_wh is the pin this file now actually sets; img_wh should always
                    // read NaN/NaN — the child's own Width/Height are never assigned anywhere in this file.
                    host.Width(), host.Height(), image.Width(), image.Height(), static_cast<int>(image.Stretch()));
                std::fclose(file);
            }
        }
        return {desired.Width, desired.Height};
    }

    void image_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        // PlatformArrangeHandler's guard, WIDENED to non-finite - see label/button's identical guard for
        // why: an unrecoverable stowed exception (0xC000027B) beats a skipped arrange.
        if (!std::isfinite(frame.x) || !std::isfinite(frame.y) || !std::isfinite(frame.width) ||
            !std::isfinite(frame.height) || frame.width < 0 || frame.height < 0)
        {
            return;
        }
        // Arrange the HOST — see this file's header CONTAINER note; the child Image stretches to fill it
        // (WinUI's own default HorizontalAlignment/VerticalAlignment for a FrameworkElement is Stretch,
        // untouched here except by map_aspect's AspectFill Center/Center override).
        const border host = as_host(platform->native);
        winui::Controls::Canvas::SetLeft(host, frame.x);
        winui::Controls::Canvas::SetTop(host, frame.y);
        host.Width(frame.width);
        host.Height(frame.height);
    }

    // C# uiContext.GetDisplayDensity() (Windows): XamlRoot::RasterizationScale is the DPI scale the
    // element's own window is CURRENTLY rendering at (the WinUI analog of UIScreen.scale /
    // NSWindow.backingScaleFactor). Null before the element is attached to a live visual tree, matching
    // apple's window==nil fallback. Only exercised by a future real font-image decode — resolution-
    // dependent reload stays moot while font sources are mirror-only (note 3).
    float image_handler::query_display_scale() const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return 1.0F;
        }
        const auto root = as_image(platform->native).XamlRoot();
        return root != nullptr ? static_cast<float>(root.RasterizationScale()) : 1.0F;
    }

    // ---- generic-IView property pushes (view_platform_base overrides) ---------------------------
    // Delegated to the shared winui_visual_ops free functions so all controls behave identically; see
    // that header for why they are free functions taking the void* slot. update_background now LANDS: with
    // `native` boxing the Border host (this file's header CONTAINER note), apply_background's existing
    // three-way try_as paints it directly — no change needed there.
    void image_platform::update_visibility(maui::core::visibility value)
    {
        maui::platform::windows::apply_visibility(native, value);
    }

    void image_platform::update_opacity(double value)
    {
        maui::platform::windows::apply_opacity(native, value);
    }

    void image_platform::update_is_enabled(bool value)
    {
        maui::platform::windows::apply_is_enabled(native, value);
    }

    void image_platform::update_automation_id(std::string_view value)
    {
        maui::platform::windows::apply_automation_id(native, value);
    }

    void image_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::windows::apply_background(native, value);
    }
} // namespace maui::core
