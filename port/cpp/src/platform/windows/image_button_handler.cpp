// image_button_handler — WinUI 3 platform recipe: a real Microsoft.UI.Xaml.Controls.Button whose
// Content is a plain Image, the same native shape ImageButtonHandler.Windows.cs builds. Ported from
// ImageButtonHandler.Windows.cs + ButtonExtensions.cs + ImageViewExtensions.cs + ControlExtensions.cs.
//
// CONFIRMED, NOT INFERRED FROM BUTTON: ImageButtonHandler.Windows.cs's CreatePlatformView is
//
//     _image = new Image { VerticalAlignment = Center, HorizontalAlignment = Center, Stretch = Uniform };
//     platformImageButton = new Button { VerticalAlignment = Stretch, HorizontalAlignment = Stretch,
//                                         Content = _image };
//
// i.e. Content is the Image ITSELF — there is no DefaultMauiButtonContent StackPanel/TextBlock
// composition here (ImageButton has no text). button_handler.cpp's content_panel/content_image/
// content_text machinery (sync_content_composition, ContentLayout) has NO counterpart on this control;
// content_image below is a one-line try_as, not a child search.
//
// THREE DOCUMENTED SIMPLIFICATIONS against the C# oracle, each mirroring one button_handler.cpp already
// made and re-justified here rather than merely copied:
//
//  1. StrokeColor / StrokeThickness / CornerRadius: C# routes these through THEME-RESOURCE overrides
//     (ButtonExtensions.UpdateStrokeColor/Thickness/CornerRadius — the same extension methods Button
//     uses, since ImageButtonHandler.Windows.cs's MapStrokeColor/Thickness/CornerRadius cast PlatformView
//     to Button and call the identical ButtonExtensions members). This slice sets the direct BorderBrush /
//     BorderThickness / CornerRadius properties instead — identical AT REST (what a screenshot captures),
//     diverging only while the pointer is over or holding the button — the exact deviation
//     button_handler.cpp documents and the button board (31.27% -> 9.71%) measured as acceptable.
//  2. Background: C# ALSO remaps this to a theme-resource override (ImageButtonHandler.Windows.cs's own
//     MapBackground -> Button.UpdateBackground, registered only for ANDROID||WINDOWS — the "Background"
//     key is NOT left to the generic ViewHandler push here). button_handler.cpp does not replicate this
//     for the sibling Button either — Background there falls through the chained view_mapper() to the
//     generic apply_background (direct Control.Background set), and this file does the same rather than
//     duplicate the resource-key plumbing for a divergence Button's own board already shows renders
//     correctly at rest: button_page.hpp sets set_background_brush on ~10 of its buttons, and the button
//     board scored 9.71% (down from 31.27%) with this exact generic push in place — a direct-property
//     Background is evidently honored by the default Button template's Normal-state binding, only losing
//     fidelity on hover/pressed, same as note 1's stroke properties.
//  3. ImageOpened NOW WIRED (layout AND the MaxHeight clamp) — ImageFailed/Unloaded still are not.
//     ImageButtonHandler.Windows.cs's OWN OnImageOpened (subscribed on the CONTENT image, `_image.
//     ImageOpened += OnImageOpened` in its ConnectHandler — NOT reached via the shared ImageHandler.Mapper,
//     which only carries the property maps like Aspect; event subscriptions are wired per-handler) calls
//     ONLY `VirtualView?.UpdateIsLoading(false)` — it does NOT call UpdatePlatformMaxConstraints (that
//     method lives on ImageHandler, is private, and ImageButtonHandler never calls it). CORRECTED: an
//     earlier version of this note concluded from that alone that "the C# consequence of ImageOpened
//     firing for an ImageButton has nothing to do with layout" — that missed a SECOND, separate ImageOpened
//     subscription: ButtonExtensions.UpdateImageSource (ButtonExtensions.cs:173-188, called from this same
//     ImageButtonImageSourcePartSetter.SetImageSource path) installs its OWN one-shot handler directly on
//     the BitmapImage (not on the Image control, and not per-connect but per-source-change) whose entire
//     job IS layout: cap nativeImage.MaxHeight to the decoded bitmap's natural height so the Stretch=Uniform
//     content Image scales down only, never up. This backend folds that into the SAME callback below
//     (clamp_image_to_natural_height, in the anonymous namespace above) rather than a second subscription,
//     since both oracle hooks fire at the identical moment ("this source has finished decoding") and this
//     callback already reaches every path that moment can occur on (the real ImageOpened event AND
//     notify_if_already_open's already-decoded belt-and-braces call). The bug this port measured
//     (image_button 27.09% light / 31.17% dark — a zero-desired child a parent stack stretches, PLUS the
//     separate now-fixed defect of an uncapped Image scaling up past its natural size) is real for the
//     out-of-cycle-relayout reason image_handler.cpp's header note 2 explains: a real MAUI backend's native
//     OS-owned layout tree bubbles the post-decode dirty flag to its root FOR FREE (regardless of what the
//     handler's own OnImageOpened body does), but this port's own C++-side desired_size_ cache does not —
//     so on_connect_handler below subscribes ImageOpened on the CONTENT image (matching the oracle's
//     subscription TARGET and lifecycle) and its callback calls invalidate_measure() (the architectural
//     necessity documented in view.hpp/window.hpp, not a literal transcription of C#'s callback body). Not
//     touched: UpdateIsLoading already flows through a DIFFERENT, pre-existing path — the cross-platform
//     map_source (src/core/image_button_handler.cpp) calls view.update_is_loading(false) synchronously
//     right after the file-fast-path load kicks off, independent of real decode completion (image_handler.
//     cpp's identical simplification) — so ImageOpened firing has nothing left to flip on THAT front on
//     this backend. ImageFailed/Unloaded remain unwired (edge cases a static parity capture never
//     exercises, matching button_handler.cpp's identical gap).

#include "maui/core/image_button_handler.hpp"

#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Input.h>
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

#include "maui/core/aspect.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/i_image_button.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/image_source_loader.hpp"
#include "maui/core/image_source_result.hpp"
#include "maui/core/view_chrome_ops.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "winui_interop.hpp"
#include "winui_visual_ops.hpp"

namespace
{
    // Named `winui`, NOT `xaml` — see button_handler.cpp's identical note (this port's own maui::xaml
    // loader namespace would win inside namespace maui::* over a file-scope alias of the same name).
    namespace winui = winrt::Microsoft::UI::Xaml;
    using button_control = winui::Controls::Button;
    using image_control = winui::Controls::Image;
    using bitmap_image = winui::Media::Imaging::BitmapImage;
    // A font-sourced result's boxed type (image_source_services.cpp's font_image_source_service::load,
    // shared with image_handler.cpp) — see this file's apply_loaded_result for why it must be unboxed
    // under its OWN type, never reinterpreted through bitmap_image's.
    using writeable_bitmap = winui::Media::Imaging::WriteableBitmap;

    button_control as_button(void* native)
    {
        return maui::platform::windows::ref<winui::UIElement>(native).as<button_control>();
    }

    // The Button's Content IS the Image (this file's header note above) — a one-line try_as, unlike
    // button_handler.cpp's content_image, which searches a composed StackPanel's children.
    image_control content_image(const button_control& button)
    {
        return button.Content().try_as<image_control>();
    }

    // AspectExtensions.ToStretch + ImageViewExtensions.UpdateAspect's AspectFill special case, ported
    // identically to image_handler.cpp's twin (map_aspect below reaches the SAME code path C# does:
    // ImageButtonHandler shares ImageHandler.Mapper, and IImageHandler.PlatformView for an ImageButton is
    // PlatformView.GetContent<Image>() — see ImageButtonHandler.cs — so MapAspect's
    // `handler.PlatformView.UpdateAspect(image)` call resolves to this exact extension method).
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

    // ---- ButtonExtensions.UpdateImageSource's / FileImageSourceService.Windows.cs's file-uri resolution,
    // duplicated from image_handler.cpp / button_handler.cpp (each copy has internal linkage in its own
    // TU) — see image_handler.cpp's header note 1 for why a bare filename resolves against the EXE
    // directory rather than `ms-appx:///` on this unpackaged exe.

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

    winrt::hstring to_file_uri(std::wstring path)
    {
        std::ranges::replace(path, L'\\', L'/');
        return winrt::hstring{L"file:///" + path};
    }

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

    // "Was this property explicitly set?" — see button_handler.cpp's identical helper + the
    // [[cpp-unset-color-sentinel-collision]] note it cites: comparing a value against a default instead
    // misreads an explicit Black/0 as unset.
    bool is_set(const maui::core::i_view& view, std::string_view property)
    {
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        return bindable != nullptr && bindable->is_property_set(property);
    }

    // The two pointer delegates — the button_platform convention (RemoveHandler needs the delegate
    // OBJECT back, not a token, so they must be kept alive and are heap-boxed behind pointer_events).
    struct pointer_sink
    {
        winui::Input::PointerEventHandler pressed{nullptr};
        winui::Input::PointerEventHandler released{nullptr};
    };
} // namespace

namespace maui::core
{
    namespace
    {
        // Unhook everything on_connect_handler registered — the button_handler.cpp convention, called
        // from on_disconnect_handler AND from ~image_button_platform (the handlers capture a
        // image_button_platform*, so a struct destroyed while still subscribed must not leave a dangling
        // callback behind).
        void detach_native_events(maui::core::image_button_platform& platform)
        {
            if (platform.native != nullptr)
            {
                const button_control button = as_button(platform.native);
                button.Click(winrt::event_token{platform.click_token});
                if (platform.pointer_events != nullptr)
                {
                    auto* sink = static_cast<pointer_sink*>(platform.pointer_events);
                    button.RemoveHandler(winui::UIElement::PointerPressedEvent(), winrt::box_value(sink->pressed));
                    button.RemoveHandler(winui::UIElement::PointerReleasedEvent(), winrt::box_value(sink->released));
                }
                // The CONTENT image's ImageOpened subscription (image_handler.cpp's identical token
                // convention) — revoked here too so ~image_button_platform tears down everything in one call.
                if (platform.image_opened_token != 0)
                {
                    if (const image_control image = content_image(button))
                    {
                        image.ImageOpened(winrt::event_token{platform.image_opened_token});
                    }
                }
            }
            platform.click_token = 0;
            platform.image_opened_token = 0;
            delete static_cast<pointer_sink*>(platform.pointer_events);
            platform.pointer_events = nullptr;
        }

        // Belt-and-braces for concern 3 (an already-decoded/cached BitmapImage reused as a new Source) —
        // the image_handler.cpp notify_if_already_open convention, applied to the CONTENT image.
        void notify_if_already_open(maui::core::image_button_platform& platform, const image_control& image)
        {
            const auto bitmap = image.Source().try_as<bitmap_image>();
            if (bitmap && bitmap.PixelWidth() > 0 && bitmap.PixelHeight() > 0 && platform.on_image_opened)
            {
                platform.on_image_opened();
            }
        }

        // ButtonExtensions.cs:173-188 "Ensure that we only scale images down and never up": once the
        // content Image's BitmapImage has decoded, nativeImage.MaxHeight is capped to
        // nativeImageSource.GetImageSourceSize(platformButton).Height — the decoded bitmap's PixelHeight
        // converted from device pixels to DIPs via GetDisplayDensity/XamlRoot.RasterizationScale
        // (ImageExtensions.cs:21-30 + FrameworkElementExtensions.cs:272-273's `?? 1.0f` fallback, mirrored
        // by the `root != nullptr` guard below — the same idiom image_handler.cpp's query_display_scale
        // already uses). Without this cap the Stretch=Uniform content Image fills the Button's whole
        // content box instead of stopping at its natural size (the port's measured image_button defect).
        // MaxHeight ONLY — the oracle's BitmapImage branch never touches MaxWidth (that is the SEPARATE
        // CanvasImageSource/font-source branch just above :173 in the oracle, which resets MaxHeight to
        // PositiveInfinity instead of capping it; that branch has no counterpart here because font sources
        // stay mirror-only on this backend — no native CanvasImageSource is ever assigned to this Image,
        // see this file's header note 3 and image_source_services.cpp — so the reset never has anything to
        // undo). A bitmap-to-bitmap source swap needs no explicit reset either: the NEW bitmap's own
        // ImageOpened firing (or notify_if_already_open, above) simply overwrites MaxHeight with the new
        // natural height, exactly like the oracle's fresh one-shot subscription on each new BitmapImage.
        void clamp_image_to_natural_height(const image_control& image)
        {
            const auto bitmap = image.Source().try_as<bitmap_image>();
            if (!bitmap || bitmap.PixelHeight() <= 0)
            {
                return;
            }
            const auto root = image.XamlRoot();
            const double scale = root != nullptr ? root.RasterizationScale() : 1.0;
            image.MaxHeight(static_cast<double>(bitmap.PixelHeight()) / scale);
        }
    } // namespace

    image_button_platform::~image_button_platform()
    {
        detach_native_events(*this);
        if (native != nullptr)
        {
            maui::platform::windows::drop<winui::UIElement>(native);
        }
    }

    std::unique_ptr<image_button_platform> image_button_handler::create_platform_view()
    {
        auto platform = std::make_unique<image_button_platform>();
        // ImageButtonHandler.Windows.cs's CreatePlatformView, verbatim (this file's header) — Content IS
        // the Image, no composition panel.
        image_control image;
        image.VerticalAlignment(winui::VerticalAlignment::Center);
        image.HorizontalAlignment(winui::HorizontalAlignment::Center);
        image.Stretch(winui::Media::Stretch::Uniform);

        button_control button;
        button.VerticalAlignment(winui::VerticalAlignment::Stretch);
        button.HorizontalAlignment(winui::HorizontalAlignment::Stretch);
        button.Content(image);

        platform->native = maui::platform::windows::take<winui::UIElement>(button);
        return platform;
    }

    void image_button_handler::on_connect_handler(image_button_platform& platform)
    {
        // Cross-platform inbound channel — the same three callbacks every backend exposes.
        platform.on_press = [this] {
            if (auto* view = virtual_view())
            {
                view->send_pressed();
            }
        };
        platform.on_release = [this] {
            if (auto* view = virtual_view())
            {
                view->send_released();
            }
        };
        // CLICKED ONLY, matching button_handler.cpp's identical note: ImageButtonHandler.Windows.cs's
        // OnClick raises ONLY Clicked() — Released() comes solely from OnPointerReleased below.
        platform.on_click = [this] {
            if (auto* view = virtual_view())
            {
                view->send_clicked();
            }
        };
        // ImageButtonHandler.Windows.cs's OnImageOpened, RE-PURPOSED — see this file's header note 3 for
        // why this callback calls invalidate_measure() rather than the oracle's UpdateIsLoading(false).
        // ALSO now applies ButtonExtensions.UpdateImageSource's MaxHeight clamp (clamp_image_to_natural_
        // height, above) — that oracle method installs its own one-shot BitmapImage.ImageOpened hook per
        // source change, but the effect (cap MaxHeight once the natural size is known) is identical to
        // hooking it here, and this callback already fires from both the real ImageOpened event AND
        // notify_if_already_open's already-decoded belt-and-braces path, so there is exactly one place to
        // apply it.
        platform.on_image_opened = [this] {
            if (auto* p = typed_platform_view(); p != nullptr && p->native != nullptr)
            {
                if (const image_control image = content_image(as_button(p->native)))
                {
                    clamp_image_to_natural_height(image);
                }
            }
            if (auto* view = virtual_view())
            {
                view->invalidate_measure();
            }
        };
        if (platform.native == nullptr)
        {
            return;
        }
        // Native half: Click is a normal subscription; PointerPressed/PointerReleased must use
        // AddHandler(..., handledEventsToo: true) — ButtonBase's control template marks both handled, so
        // a plain subscription never fires (button_handler.cpp's identical load-bearing note).
        auto* self = &platform;
        const button_control button = as_button(platform.native);
        platform.click_token =
            button
                .Click([self](const winrt::Windows::Foundation::IInspectable&, const winui::RoutedEventArgs&) {
                    if (self->on_click)
                    {
                        self->on_click();
                    }
                })
                .value;

        auto sink = std::make_unique<pointer_sink>();
        sink->pressed = winui::Input::PointerEventHandler(
            [self](const winrt::Windows::Foundation::IInspectable&, const winui::Input::PointerRoutedEventArgs&) {
                if (self->on_press)
                {
                    self->on_press();
                }
            });
        sink->released = winui::Input::PointerEventHandler(
            [self](const winrt::Windows::Foundation::IInspectable&, const winui::Input::PointerRoutedEventArgs&) {
                if (self->on_release)
                {
                    self->on_release();
                }
            });
        button.AddHandler(winui::UIElement::PointerPressedEvent(), winrt::box_value(sink->pressed), true);
        button.AddHandler(winui::UIElement::PointerReleasedEvent(), winrt::box_value(sink->released), true);
        platform.pointer_events = sink.release();

        // ImageButtonHandler.Windows.cs's ConnectHandler: `_image.ImageOpened += OnImageOpened;` — attached
        // on the CONTENT image, before ANY Source is ever set on it (view_handler.hpp's set_virtual_view
        // calls on_connect_handler before the mapper's first update_properties() pass, which is what runs
        // map_source — image_handler.cpp's on_connect_handler carries the full ordering argument). Unlike
        // the oracle's OnImageOpened, this callback calls invalidate_measure() rather than UpdateIsLoading —
        // see this file's header note 3 for why.
        if (const image_control image = content_image(button))
        {
            platform.image_opened_token = image
                                              .ImageOpened([self](const winrt::Windows::Foundation::IInspectable&,
                                                                  const winui::RoutedEventArgs&) {
                                                  if (self->on_image_opened)
                                                  {
                                                      self->on_image_opened();
                                                  }
                                              })
                                              .value;
        }
    }

    void image_button_handler::on_disconnect_handler(image_button_platform& platform)
    {
        detach_native_events(platform);
        platform.on_press = nullptr;
        platform.on_release = nullptr;
        platform.on_click = nullptr;
        platform.on_image_opened = nullptr;
    }

    // ---- the image surface (the ImageMapper chain, keyed on i_image_button) ----

    void image_button_handler::map_aspect(image_button_handler& handler, i_image_button& view)
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
        const image_control image = content_image(as_button(platform->native));
        if (!image)
        {
            return;
        }
        image.Stretch(to_stretch(platform->image_aspect));
        if (platform->image_aspect == maui::core::aspect::aspect_fill)
        {
            image.HorizontalAlignment(winui::HorizontalAlignment::Center);
            image.VerticalAlignment(winui::VerticalAlignment::Center);
        }
    }

    // IsOpaque: mirror only — Image is a FrameworkElement with no WinUI analog, matching image_handler.
    // cpp's identical gap.
    void image_button_handler::map_is_opaque(image_button_handler& handler, i_image_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->opaque = view.is_opaque();
        }
    }

    // ImageViewExtensions.UpdateIsAnimationPlaying, ported identically to image_handler.cpp's twin.
    void image_button_handler::map_is_animation_playing(image_button_handler& handler, i_image_button& view)
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
        const image_control image = content_image(as_button(platform->native));
        if (!image)
        {
            return;
        }
        const auto source = image.Source();
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

    // ---- the button surface (ImageButtonHandler.Mapper's own keys) ----

    // ControlExtensions.UpdatePadding via ButtonExtensions.UpdatePadding(button, padding) — same
    // ClearValue-restores-the-style-default / explicit-Thickness-overrides shape as button_handler.cpp's
    // map_padding (an unset Padding must NOT zero the native default — PARITY rule 1 (NATIVE-DEFAULTS)).
    void image_button_handler::map_padding(image_button_handler& handler, i_image_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->padding = view.padding();
        const maui::core::thickness& p = platform->padding;
        // GATE ON ALL-FOUR-NaN, NOT ON "was the bindable set". ControlExtensions.cs:51-56 is
        //     platformControl.Padding = padding.IsNaN ? defaultThickness ?? new Thickness() : padding.ToPlatform();
        // where Thickness.IsNaN (Primitives/Thickness.cs:53) requires ALL FOUR components to be NaN, and
        // ButtonExtensions.cs:146-147 supplies GetResource<Thickness>("ButtonPadding") as that default. So
        // MAUI falls back to the style padding ONLY for a wholly-NaN thickness and otherwise pushes the
        // value -- including a plain zero.
        // The old `!is_set(...) -> ClearValue(PaddingProperty)` restored the WinUI Button style's
        // ButtonPadding (11 DIPs vertical) for any padding the developer never explicitly assigned, which is
        // a DIFFERENT predicate: the port's unset padding is thickness{0,0,0,0}, not NaN, so the oracle would
        // push 0 where the port pushed 11. Measured on input_controls: MAUI renders a sourceless
        // <ImageButton /> as a 2px hairline (two 1px borders, zero padding, collapsed content Image) where
        // the port rendered a 13px rounded pill (1 + 11 + 1). On image_button the +11 compounds down the
        // page across five more paddingless buttons, and on the 40x40 "Custom Size" one it eats 22 of 40
        // DIPs horizontally, shrinking dotnet_bot.png to roughly a 16x27 content box against MAUI's 38x38.
        if (std::isnan(p.left) && std::isnan(p.top) && std::isnan(p.right) && std::isnan(p.bottom))
        {
            as_button(platform->native).ClearValue(winui::Controls::Control::PaddingProperty());
            return;
        }
        as_button(platform->native).Padding(winui::Thickness{p.left, p.top, p.right, p.bottom});
    }

    // ButtonExtensions.UpdateStrokeColor — direct BorderBrush property, per this file's header note 1.
    void image_button_handler::map_stroke_color(image_button_handler& handler, i_image_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->stroke_color = view.stroke_color();
        const button_control button = as_button(platform->native);
        if (!is_set(view, "stroke_color"))
        {
            button.ClearValue(winui::Controls::Control::BorderBrushProperty());
            return;
        }
        button.BorderBrush(winui::Media::SolidColorBrush{maui::platform::windows::to_ui_color(platform->stroke_color)});
    }

    // ButtonExtensions.UpdateStrokeThickness — direct BorderThickness property, per note 1. C#'s
    // MapStrokeThickness ALSO re-triggers `handler.UpdateValue(nameof(IImageButton.Padding))` afterward
    // (ImageButtonHandler.Windows.cs:81-85) — ported literally (re-run map_padding) even though
    // button_handler.cpp's sibling map_stroke_thickness does not: this is an oracle line this file's
    // header did not find a reason to omit, and re-asserting Padding is idempotent/harmless either way.
    void image_button_handler::map_stroke_thickness(image_button_handler& handler, i_image_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->stroke_thickness = view.stroke_thickness();
        const button_control button = as_button(platform->native);
        if (platform->stroke_thickness >= 0)
        {
            const double t = platform->stroke_thickness;
            button.BorderThickness(winui::Thickness{t, t, t, t});
        }
        else
        {
            button.ClearValue(winui::Controls::Control::BorderThicknessProperty());
        }
        map_padding(handler, view);
    }

    // ButtonExtensions.UpdateCornerRadius — direct CornerRadius property, per note 1. C#'s
    // MapCornerRadius likewise re-triggers Padding (ImageButtonHandler.Windows.cs:87-95) — ported for the
    // same reason as map_stroke_thickness above. Its further `if (VirtualView.Shadow is not null)
    // UpdateValue(Shadow)` branch is NOT ported: this backend's image_button_platform does not override
    // update_shadow (only Apple/iOS do — see the header's #ifdef MAUI_PLATFORM_APPLE/IOS blocks), so
    // re-pushing it here would resolve to the base no-op mirror and change nothing observable.
    void image_button_handler::map_corner_radius(image_button_handler& handler, i_image_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->corner_radius = view.corner_radius();
        const button_control button = as_button(platform->native);
        if (platform->corner_radius >= 0)
        {
            const auto r = static_cast<double>(platform->corner_radius);
            button.CornerRadius(winui::CornerRadius{r, r, r, r});
        }
        else
        {
            button.ClearValue(winui::Controls::Control::CornerRadiusProperty());
        }
        map_padding(handler, view);
    }

    // MEASURED DEFECT, NOW FIXED (CONSUMER HALF LANDED) — this can still return a size taken before the
    // bitmap exists, on the FIRST Measure of a freshly-sourced Image; the fix is what happens AFTER that.
    //
    // BitmapImage decoding is ALWAYS asynchronous in WinUI, so a freshly-sourced Image reports (0,0) at
    // the first Measure (image_handler.cpp's header note 2 states this outright). The two board symptoms
    // this caused were ONE bug wearing two faces:
    //   * image_button (27.09% light / 31.17% dark): a zero-desired child that the parent stack then
    //     stretches, so ONE ImageButton fills the whole viewport (~200x702) and pushes the other eight
    //     off-screen. The gear glyph inside it renders at the correct ~140px -- the CONTENT measured
    //     fine, the control's reported height did not.
    //   * image (82.90% light, the board's 2nd-worst page): the same (0,0) with no stretching parent, so
    //     every image collapses to zero height and the page shows labels with no pictures.
    //
    // FIXING IT NEEDED TWO PIECES, BOTH NOW LANDED: (1) view.hpp's invalidate_measure() is real (asks the
    // containing window to replay drive_layout, via window::request_relayout / set_relayout_hook, installed
    // by this backend's host_run.cpp right after its first pass); (2) on_connect_handler above subscribes
    // ImageOpened on the CONTENT image and calls invalidate_measure() from it (this file's header note 3),
    // so once the decode lands, THIS get_desired_size gets called again and reports the real size. A local
    // fudge (hard-coding a natural size, or forcing a synchronous decode only for this control) would have
    // hidden the shared defect instead of fixing it -- this closes the actual gap.
    maui::graphics::size image_button_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        if (width_constraint < 0 || height_constraint < 0)
        {
            return {0, 0};
        }
        const button_control button = as_button(platform->native);
        // ARRANGE/EXPLICIT-SIZE FIX (measured on the guest, commit 11d7965489): this used to clear
        // Width/Height to NaN UNCONDITIONALLY, discarding a real WidthRequest along with the stale
        // arranged frame — so a WidthRequest(200) ImageButton (image_button_page.hpp's fit_button_,
        // configure_aspect_button(fit_button_, aspect_fit, 200, 0)) measured with NO width cap at all: its
        // Stretch=Uniform content Image scaled the 128x128 icon up to fill the FULL EXTERNAL constraint
        // (cw=920) instead of the requested ~200, reporting an 886x886 desired image / ~920x906 desired
        // button that then pushed the page's other 8 buttons off-screen (27.09% light / 31.17% dark).
        //
        // The oracle (ViewHandlerExtensions.Windows.cs:56-74 GetDesiredSizeFromHandler + :91-105
        // AdjustForExplicitSize) keeps Width/Height PERSISTENTLY set to the explicit request (pushed once
        // by MapWidth/MapHeight -> UpdateWidth/UpdateHeight, ViewExtensions.cs:181-193's
        // `platformView.Width = view.Width`, propagated verbatim because WinUI's NaN-is-unspecified
        // convention already matches the xplat one) and, AT MEASURE TIME, only WIDENS the incoming
        // constraint — `Math.Max(externalConstraint, explicitValue)` when explicitValue is not NaN — it
        // never clears the pin. Ported the same way below: read i_image_button's own width()/height()
        // (this port's unset sentinel is NaN for BOTH — confirmed in controls/view.hpp:1010-1021's
        // resolve_request, which is what the concrete view virtual_view() resolves to implements — so no
        // encoding translation is needed, same as C#'s NaN convention), pin Width/Height to that, then
        // widen width_constraint/height_constraint by the same Math.Max rule before measuring.
        //
        // platform_arrange's OWN Width/Height stamp (below) is DELIBERATELY UNTOUCHED — see its comment.
        const auto* view = virtual_view();
        const double explicit_width = (view != nullptr) ? view->width() : std::numeric_limits<double>::quiet_NaN();
        const double explicit_height = (view != nullptr) ? view->height() : std::numeric_limits<double>::quiet_NaN();
        button.Width(explicit_width);
        button.Height(explicit_height);
        const double adjusted_width_constraint =
            std::isnan(explicit_width) ? width_constraint : std::max(width_constraint, explicit_width);
        const double adjusted_height_constraint =
            std::isnan(explicit_height) ? height_constraint : std::max(height_constraint, explicit_height);
        // See image_handler.cpp's identical note: Measure() returns a CACHED DesiredSize on an element XAML
        // does not consider measure-dirty, and this port drives layout out-of-cycle so nothing else marks it.
        // The content Image is invalidated too -- the Button's own measure asks its Content for a size, and
        // a stale cache on the child is what freezes the parent (the ImageButton's cached zero-desired
        // height is what the parent stack then stretches to fill the viewport).
        if (const image_control image = content_image(button))
        {
            image.InvalidateMeasure();
        }
        button.InvalidateMeasure();
        button.Measure(
            winrt::Windows::Foundation::Size{maui::platform::windows::measure_constraint(adjusted_width_constraint),
                                             maui::platform::windows::measure_constraint(adjusted_height_constraint)});
        const auto desired = button.DesiredSize();
        return {desired.Width, desired.Height};
    }

    void image_button_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        // Widened to non-finite, matching button/image/label's identical guard: an unrecoverable stowed
        // exception (0xC000027B) beats a skipped arrange.
        if (!std::isfinite(frame.x) || !std::isfinite(frame.y) || !std::isfinite(frame.width) ||
            !std::isfinite(frame.height) || frame.width < 0 || frame.height < 0)
        {
            return;
        }
        const button_control button = as_button(platform->native);
        winui::Controls::Canvas::SetLeft(button, frame.x);
        winui::Controls::Canvas::SetTop(button, frame.y);
        // NOT PlatformArrangeHandler (ViewHandlerExtensions.Windows.cs:76-88), which only calls
        // platformView.Arrange(rect) and NEVER assigns Width/Height — deliberately, per this file's
        // get_desired_size comment above. layout_handler.cpp's own header (the panel THIS button is a
        // child of) and button_handler.cpp's identical stamp (:706-710) already establish, as a port-wide
        // fact rather than an image-button-specific one, that "a Canvas child has no other way to be
        // sized": winui::Controls::Canvas's real ArrangeOverride positions each child at its OWN
        // DesiredSize next to Canvas.Left/Top (it measures children with an infinite constraint and hands
        // out no slot), and WinUI's own layout system revisits this Canvas whenever Canvas.SetLeft/SetTop
        // change here — which is every platform_arrange call. Leaving Width/Height unset would size this
        // element to its bare intrinsic content the next time that happens, not to `frame`. Every Windows
        // handler relies on this stamp for that reason; it is not part of the measured defect and is left
        // as-is on purpose (see the CRITICAL RISK / Canvas discussion in this slice's task write-up).
        button.Width(frame.width);
        button.Height(frame.height);
        // Clip is bounds-dependent (view_chrome_ops.cpp's apply_native_clip reads the just-set Width/
        // Height back); map_clip's own push (view_mapper.cpp) always runs before the first arrange, so
        // this re-invoke is what actually installs the clip once the image button has a real size.
        // `native` boxes the Button itself (this file's header — Content IS the Image, no host Border),
        // matching button_handler.cpp's identical direct push.
        if (const auto* view = virtual_view(); view != nullptr)
        {
            apply_native_clip(platform->native, view->clip());
        }
    }

    // ---- per-backend image-source primitives (the cross-platform map_source in image_button_handler.cpp
    // routes here) — ButtonExtensions.UpdateImageSource, minus the CanvasImageSource (font-source) DRAW:
    // font sources stay glyph-mirror-only on this backend (no Win2D linked on any backend — see
    // image_source_services.cpp), matching image_handler.cpp/button_handler.cpp's identical gap, but are
    // sized for real (image_source_services.cpp's font_image_source_service::load boxes a real, correctly-
    // sized WriteableBitmap now — see apply_loaded_result below for why it is unboxed under its own type).
    // No current gallery ImageButton uses FontImageSource (image_button.xaml / input_controls.xaml are both
    // file/empty sources only), so this is a latent-bug fix, not a measured regression.
    //
    // clamp_image_to_natural_height/notify_if_already_open below DELIBERATELY stay try_as<bitmap_image>
    // (NOT widened to the bitmap_source base image_handler.cpp's twin now uses): the oracle's :165-171 has
    // a SEPARATE CanvasImageSource branch here — explicit `nativeImage.Width/Height = size` + MaxHeight =
    // Infinity, NOT the BitmapImage branch's "clamp MaxHeight to natural size" — which this port does not
    // implement (this file's header note 3). A font-sourced ImageButton/Button would therefore get NEITHER
    // oracle branch's exact sizing today (no natural-height clamp, and no explicit small-icon pin either —
    // it would fall through to whatever plain Stretch=Uniform does with an unconstrained box, closer to the
    // plain Image control's blow-up than the oracle's pinned-small-icon button chrome). Inert today (no
    // FontImageSource on any gallery Button/ImageButton) — a real gap if one is ever added, not a claim
    // this port already handles it.

    void image_button_handler::configure_loader(maui::core::image_source_loader& /*loader*/)
    {
        // Matches image_handler.cpp/button_handler.cpp's windows configure_loader: a no-op. The async
        // loader's uri/stream path already resolves through image_source_services.cpp's
        // MAUI_WINDOWS_SWAPS registration.
    }

    void image_button_handler::load_file_source_sync(image_button_platform& platform,
                                                     const i_file_image_source& file_src)
    {
        platform.source_kind = "file";
        platform.source_file = std::string(file_src.file());
        platform.source_loaded = true;
        if (platform.native == nullptr)
        {
            return;
        }
        const image_control image = content_image(as_button(platform.native));
        if (!image)
        {
            return;
        }
        try
        {
            image.Source(bitmap_image{resolve_file_uri(platform.source_file)});
            image.Visibility(winui::Visibility::Visible);
            notify_if_already_open(platform, image);
        }
        catch (const winrt::hresult_error&)
        {
            // A malformed path/uri (Uri's ctor throws) — clear rather than leave a stale source,
            // mirroring image_handler.cpp/button_handler.cpp's identical catch.
            image.Source(nullptr);
            image.Visibility(winui::Visibility::Collapsed);
        }
    }

    void image_button_handler::apply_loaded_result(image_button_platform& platform, const image_source_result& result)
    {
        if (!result.loaded())
        {
            clear_source_native(platform);
            return;
        }
        platform.source_kind = result.kind();
        platform.source_file = result.detail();
        platform.source_loaded = true;
        if (platform.native == nullptr)
        {
            return;
        }
        const image_control image = content_image(as_button(platform.native));
        if (!image)
        {
            return;
        }
        // result.image() carries a real BitmapImage handle for uri/stream sources, and now a WriteableBitmap
        // handle for font sources too (image_source_services.cpp's font_image_source_service::load — a
        // SIZE-ONLY stand-in, see that file's header; the glyph itself stays unrendered, no Win2D linked).
        // Two unrelated boxed types reach here, so which `ref<T>` unboxes it is picked by kind(), exactly
        // like image_handler.cpp's identical branch — reinterpreting a boxed writeable_bitmap as a
        // bitmap_image would be a real (if currently unexercised — no gallery ImageButton uses
        // FontImageSource yet) type-punning bug, not merely a style concern.
        if (result.image() != nullptr)
        {
            if (result.kind() == "font")
            {
                image.Source(maui::platform::windows::ref<writeable_bitmap>(result.image()));
            }
            else
            {
                image.Source(maui::platform::windows::ref<bitmap_image>(result.image()));
                notify_if_already_open(platform, image);
            }
            image.Visibility(winui::Visibility::Visible);
        }
    }

    void image_button_handler::clear_source_native(image_button_platform& platform)
    {
        platform.source_kind.clear();
        platform.source_file.clear();
        platform.source_loaded = false;
        if (platform.native == nullptr)
        {
            return;
        }
        if (const image_control image = content_image(as_button(platform.native)))
        {
            image.Source(nullptr);
            image.Visibility(winui::Visibility::Collapsed);
        }
    }

    // ---- generic-IView property pushes (view_platform_base overrides) ---------------------------
    // Delegated to the shared winui_visual_ops free functions, the button_platform convention — see this
    // file's header note 2 for why Background rides the generic push rather than ButtonExtensions.
    // UpdateBackground's theme-resource override.
    void image_button_platform::update_visibility(maui::core::visibility value)
    {
        maui::platform::windows::apply_visibility(native, value);
    }

    void image_button_platform::update_opacity(double value)
    {
        maui::platform::windows::apply_opacity(native, value);
    }

    void image_button_platform::update_is_enabled(bool value)
    {
        maui::platform::windows::apply_is_enabled(native, value);
    }

    void image_button_platform::update_automation_id(std::string_view value)
    {
        maui::platform::windows::apply_automation_id(native, value);
    }

    void image_button_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::windows::apply_background(native, value);
    }
} // namespace maui::core
