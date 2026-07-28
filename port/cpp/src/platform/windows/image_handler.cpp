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
//  2. ASYNC DECODE, NO ImageOpened WIRING. BitmapImage decoding is ALWAYS asynchronous in WinUI — there is
//     no synchronous decode API, unlike apple's NSImage — so get_desired_size may read {0,0} on the very
//     first Measure (the decode has not completed yet). ImageHandler.Windows.cs subscribes ImageOpened
//     precisely to react to this (OnImageOpened -> UpdatePlatformMaxConstraints), but that only matters
//     once this port drives a SECOND layout pass after the initial one — which it already does: the E2E/
//     parity harness resizes the window after launch (host_run.cpp's SizeChanged -> drive_layout, "the
//     E2E runner pins the window to an explicit rect AFTER launching the process"), by which point a
//     small local file has virtually always finished decoding. Wiring ImageOpened to force an EARLIER
//     relayout would mean adding a new cross-cutting relayout hook to host_run.cpp (the android partial's
//     jni/relayout.hpp analog) — out of this file's scope; get_desired_size instead does the plain real
//     Measure(), matching the label/button shape exactly.
//  3. URI/STREAM/FONT SOURCES STAY MIRROR-ONLY. src/platform/windows/image_source_services.cpp does not
//     exist yet — MAUI_WINDOWS_SWAPS (CMakeLists.txt) does not swap image_source_services.cpp, so this
//     backend still resolves those source kinds through the (headless) mirror registry, whose results
//     never carry a real image() handle. Only the FILE fast path below decodes a real BitmapImage; see
//     apply_loaded_result. // TODO: a windows image_source_services.cpp (BitmapImage-from-uri/stream, a
//     FontIconSource-style glyph rasterization for font sources) would let that path push a real image,
//     matching the apple/android per-source-kind recipes.
//  4. IsOpaque stays a mirror (no WinUI analog — Image is a FrameworkElement with nothing resembling
//     C#'s opaque hint, matching the android partial's identical gap).
//
// NOT PORTED (out of scope for this slice, matching the header's own note): NeedsContainer /
// SetupContainer — C# wraps the Image in a Border when Background is set or Aspect is AspectFill (the
// same reason LabelHandler.Windows wraps its TextBlock). This header has no such container seam for
// Image on ANY backend yet, so Background is wired (see image_platform::update_background below) but has
// nowhere to paint (Image is a FrameworkElement, not a Panel/Control/Border) — apply_background's
// three-way try_as silently finds nothing, a documented gap rather than an invented container.

#include "maui/core/image_handler.hpp"

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.h>

#include <windows.h>

#include <algorithm>
#include <cmath>
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

    image_control as_image(void* native)
    {
        return maui::platform::windows::ref<winui::UIElement>(native).as<image_control>();
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
    image_platform::~image_platform()
    {
        if (native != nullptr)
        {
            maui::platform::windows::drop<winui::UIElement>(native);
        }
    }

    std::unique_ptr<image_platform> image_handler::create_platform_view()
    {
        auto platform = std::make_unique<image_platform>();
        image_control image;
        platform->native = maui::platform::windows::take<winui::UIElement>(image);
        return platform;
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
        try
        {
            image.Source(bitmap_image{resolve_file_uri(platform.source_file)});
        }
        catch (const winrt::hresult_error&)
        {
            // A malformed path/uri (Uri's ctor throws) - clear rather than leave a stale source, mirroring
            // the apple partial's nil-on-failed-load (load_image_from_file returning nil -> image = nil).
            image.Source(nullptr);
        }
    }

    // The async loader's apply: see this file's header note 3. A !loaded() result clears the view (the
    // SetImageSource(null) analog); a loaded uri/stream/font result updates the string mirrors only —
    // result.image() is always null on this backend (no windows image_source_services.cpp yet), so there
    // is no real bitmap to push and the Image control keeps whatever it last displayed.
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
    }

    // Clear the loaded image (both the mirror and, when a native view exists, the real Source).
    void image_handler::clear_source_native(image_platform& platform)
    {
        platform.source_kind.clear();
        platform.source_file.clear();
        platform.source_loaded = false;
        if (platform.native != nullptr)
        {
            as_image(platform.native).Source(nullptr);
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
        const image_control image = as_image(platform->native);
        // Clear the pinned size FIRST, exactly like label/button: platform_arrange stamps an explicit
        // Width/Height on this element every pass (a Canvas child has no other way to be sized), and a
        // FrameworkElement with an explicit Width/Height measures to exactly that instead of re-measuring.
        const auto auto_size = std::numeric_limits<double>::quiet_NaN();
        image.Width(auto_size);
        image.Height(auto_size);
        image.Measure(winrt::Windows::Foundation::Size{maui::platform::windows::measure_constraint(width_constraint),
                                                       maui::platform::windows::measure_constraint(height_constraint)});
        const auto desired = image.DesiredSize();
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
        const image_control image = as_image(platform->native);
        winui::Controls::Canvas::SetLeft(image, frame.x);
        winui::Controls::Canvas::SetTop(image, frame.y);
        image.Width(frame.width);
        image.Height(frame.height);
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
    // that header for why they are free functions taking the void* slot. update_background is wired but
    // is a no-op in practice for Image — see this file's header "NOT PORTED" note.
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
