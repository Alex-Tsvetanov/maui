// image_button_handler — Windows (WinUI 3) platform partial: a REAL Microsoft.UI.Xaml.Controls.Button
// whose Content is an inner Microsoft.UI.Xaml.Controls.Image (C#'s _image) — the exact
// ImageButtonHandler.Windows.cs composition. The windows twin of
// src/platform/apple/image_button_handler.mm (NSButton) / the android ShapeableImageView partial, and
// the real-native sibling of the headless mirror (src/platform/headless/image_button_handler.cpp).
//
// Ported DIRECTLY from ImageButtonHandler.Windows.cs (CreatePlatformView's Image-in-Button, the
// Click/PointerPressed/PointerReleased event split, the _image.ImageOpened hook) +
// Platform/Windows/{ButtonExtensions.cs (UpdateStrokeColor/UpdateStrokeThickness/UpdateCornerRadius/
// UpdatePadding/UpdateBackground/UpdateImageSource), ImageViewExtensions.cs (UpdateAspect/Clear),
// AspectExtensions.cs (ToStretch), ViewExtensions.cs} — the button-surface pushes reuse the windows
// button partial's exact patterns, the image-surface pushes the windows image partial's.
//
// DOCUMENTED DEVIATIONS (each an infrastructure gap of this first cut, not a behavior guess):
//   - Background / stroke land on the DIRECT dependency properties (Background / BorderBrush /
//     BorderThickness / CornerRadius). C# additionally writes the themed ButtonBackground*/
//     ButtonBorderBrush* resource keys + RefreshThemeResources so the PointerOver/Pressed/Disabled
//     states track the pushed value — deferred with the resource-dictionary seam (the button
//     partial's identical deviation). Null branches ride ClearValue, discriminated through
//     BindableObject.IsSet where the port's value type has no null.
//   - platform.on_click keeps the HEADLESS iOS-proxy shape (send_released + send_clicked — the
//     cross-platform suite drives it directly and asserts that order; the android twin does the
//     same), while the NATIVE Click event routes straight to send_clicked ONLY (C#'s OnClick →
//     VirtualView.Clicked()): the native PointerReleased already delivered Released, so funnelling
//     Click through platform.on_click would double-send it.
//   - FILE results and locally-resolvable URI results ride the BitmapImage-over-Uri lane into the
//     inner Image (ButtonExtensions.UpdateImageSource's Source push + the null→Collapsed visibility
//     toggle); stream/font results keep the headless mirror (// deferred: InMemoryRandomAccessStream /
//     CanvasImageSource seams). UpdateImageSource's CanvasImageSource font sizing and the
//     BitmapImage "only scale down" MaxHeight clamp are deferred with those seams.
//   - IsAnimationPlaying / IsOpaque mirror only (the image partial's identical deviation).
//   - The Unloaded → Released mid-press safety net (_isPressed) and the _image.ImageFailed logging
//     hook are deferred with the pointer-capture edge cases (the button partial's deviation).
//
// XAML-less degradation (the windows twin of the android VM-less fallback): the windows preset also
// runs the cross-platform suite on the host where NO XAML runtime exists; create_platform_view
// catches the construction failure and keeps native/image null, while the headless mirrors are
// ALWAYS maintained and on_press/on_release/on_click stay invokable C++ callbacks — so that suite
// observes exactly the headless partial's behavior.

#include "maui/core/image_button_handler.hpp"

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h> // ButtonBase.Click consume methods
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Input.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/base.h>

#include "maui/core/bindable_object.hpp"
#include "maui/core/dimension.hpp"
#include "maui/core/i_image_button.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/image_source_loader.hpp" // configure_loader parameter type
#include "maui/core/image_source_result.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "windows_native.hpp"

namespace
{
    namespace mux = winrt::Microsoft::UI::Xaml;
    namespace muxc = winrt::Microsoft::UI::Xaml::Controls;
    namespace muxi = winrt::Microsoft::UI::Xaml::Input;
    namespace muxm = winrt::Microsoft::UI::Xaml::Media;
    namespace muxmi = winrt::Microsoft::UI::Xaml::Media::Imaging;
    namespace wnative = maui::platform::win;

    [[nodiscard]] muxc::Button button_of(const maui::core::image_button_platform& platform)
    {
        return wnative::borrow<muxc::Button>(platform.native);
    }

    // The inner Image (C#'s _image / GetContent<WImage>()) — held in its own detached slot so the
    // source/aspect primitives reach it without walking Button.Content.
    [[nodiscard]] muxc::Image inner_image_of(const maui::core::image_button_platform& platform)
    {
        return wnative::borrow<muxc::Image>(platform.image);
    }

    // AspectExtensions.ToStretch — duplicated from the image partial (not shared) so the two TUs stay
    // independently buildable, the box/border corner_radii_of doctrine.
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

    // A BitmapImage over a file:/// URI for a local path (the FileImageSourceService recipe) —
    // duplicated from the image partial (independent-TU doctrine). Null on any failure.
    [[nodiscard]] muxmi::BitmapImage bitmap_from_file(std::string_view file)
    {
        try
        {
            const winrt::hstring wide = wnative::to_hstring_utf8(file);
            std::error_code ec;
            std::filesystem::path absolute =
                std::filesystem::absolute(std::filesystem::path{std::wstring_view{wide}}, ec);
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

    // A BitmapImage over an already-resolved URI string (the loader's uri lane — read_uri_bytes only
    // resolves file:// / bare local paths this cut, so a loaded "uri" result is locally decodable).
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

    // ButtonExtensions.UpdateImageSource's Source push: set (or clear) the inner Image's Source, with
    // the null→Collapsed / value→Visible visibility toggle C# applies so an image-less button does not
    // reserve content space. The CanvasImageSource font sizing + the BitmapImage "only scale down"
    // MaxHeight clamp are deferred (header deviations).
    void set_inner_image_source(const maui::core::image_button_platform& platform, const muxmi::BitmapImage& bitmap)
    {
        auto image = inner_image_of(platform);
        if (image == nullptr)
        {
            return;
        }
        if (bitmap != nullptr)
        {
            image.Source(bitmap);
            image.Visibility(mux::Visibility::Visible);
        }
        else
        {
            image.Source(nullptr);
            image.Visibility(mux::Visibility::Collapsed);
        }
    }
} // namespace

namespace maui::core
{
    // Releases the strong refs pinning the Button, the detached inner Image, and any event-wiring
    // remainder (the wnative shape of the pimpl-owned-native doctrine; the apple twin CFReleases its
    // NSButton here). The handler slots are normally released in on_disconnect_handler — this is the
    // defensive sweep for a platform that dies without a disconnect.
    image_button_platform::~image_button_platform()
    {
        wnative::release(pointer_pressed_handler);
        wnative::release(pointer_released_handler);
        wnative::release(image);
        wnative::release(native);
    }

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each
    // calls the base body FIRST — the headless mirrors must stay live for the XAML-less cross-platform
    // suite (header note) — then pushes to the real Button when one exists (the button partial's
    // exact bodies).

    void image_button_platform::update_visibility(maui::core::visibility value)
    {
        view_platform_base::update_visibility(value);
        // ViewExtensions.UpdateVisibility (Windows): Hidden rides Opacity 0, Collapsed collapses.
        wnative::apply_visibility(native, value, alpha);
    }

    void image_button_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        // ViewExtensions.UpdateOpacity: the Hidden state pins opacity 0 (apply_visibility restores it).
        if (!hidden)
        {
            wnative::apply_opacity(native, value);
        }
    }

    void image_button_platform::update_is_enabled(bool value)
    {
        view_platform_base::update_is_enabled(value);
        // ViewExtensions.UpdateIsEnabled → ControlExtensions.UpdateIsEnabled: Control.IsEnabled.
        if (auto button = button_of(*this))
        {
            button.IsEnabled(value);
        }
    }

    void image_button_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        // ViewExtensions.UpdateAutomationId: AutomationProperties.SetAutomationId.
        wnative::apply_automation_id(native, value);
    }

    void image_button_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        auto button = button_of(*this);
        if (button == nullptr)
        {
            return;
        }
        // ImageButtonHandler.MapBackground (a windows-specific mapper key) → ButtonExtensions.
        // UpdateBackground: null removes the ButtonBackground* resource keys (→ theme default), a
        // value sets them all. The port pushes the DIRECT Background property (deferred: the
        // per-state resource keys — header) and ClearValue for the null branch.
        if (value == nullptr)
        {
            button.ClearValue(muxc::Control::BackgroundProperty());
            return;
        }
        if (const auto* solid = dynamic_cast<const maui::graphics::solid_paint*>(value))
        {
            button.Background(wnative::to_brush(solid->color()));
            return;
        }
        // deferred: gradient / image-source paints (Paint.ToPlatform) — the base mirror above keeps
        // the borrow observable.
    }

    std::unique_ptr<image_button_platform> image_button_handler::create_platform_view()
    {
        auto platform = std::make_unique<image_button_platform>();
        try
        {
            // ImageButtonHandler.Windows.CreatePlatformView: _image = new Image { VerticalAlignment =
            // Center, HorizontalAlignment = Center, Stretch = Uniform }; new Button { VerticalAlignment
            // = Stretch, HorizontalAlignment = Stretch, Content = _image }.
            muxc::Image image;
            image.VerticalAlignment(mux::VerticalAlignment::Center);
            image.HorizontalAlignment(mux::HorizontalAlignment::Center);
            image.Stretch(muxm::Stretch::Uniform);
            muxc::Button button;
            button.VerticalAlignment(mux::VerticalAlignment::Stretch);
            button.HorizontalAlignment(mux::HorizontalAlignment::Stretch);
            button.Content(image);
            platform->image = wnative::store(image);   // released in ~image_button_platform
            platform->native = wnative::store(button); // released in ~image_button_platform
        }
        catch (const winrt::hresult_error&)
        {
            wnative::release(platform->image);
            platform->native = nullptr; // XAML-less degradation (header note)
        }
        return platform;
    }

    void image_button_handler::on_connect_handler(image_button_platform& platform)
    {
        // The C++ callback seam keeps the HEADLESS shapes — the cross-platform suite drives these
        // directly and asserts on_click's Released-then-Clicked order (the iOS touch-proxy shape the
        // android twin also keeps); the native events below dispatch per ImageButtonHandler.Windows's
        // own split instead (header deviations).
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
        platform.on_click = [this] {
            if (auto* view = virtual_view())
            {
                view->send_released();
                view->send_clicked();
            }
        };
        auto button = button_of(platform);
        if (button == nullptr)
        {
            return;
        }
        auto* peer = &platform;
        // ConnectHandler: platformView.Click += OnClick → VirtualView.Clicked() ONLY. Deliberately
        // NOT routed through platform.on_click: the native PointerReleased below already delivers
        // Released, and on_click's headless shape would double-send it (header deviations). The
        // `this` capture is safe — on_disconnect_handler revokes the token before the handler dies.
        const winrt::event_token click_token =
            button.Click([this](const winrt::Windows::Foundation::IInspectable&, const mux::RoutedEventArgs&) {
                if (auto* view = virtual_view())
                {
                    view->send_clicked();
                }
            });
        platform.click_token = click_token.value;
        // ConnectHandler: AddHandler(UIElement.PointerPressedEvent/PointerReleasedEvent, handler, true)
        // — handledEventsToo, because the Button control class marks the pointer events handled
        // internally. The boxed delegates are retained so DisconnectHandler can RemoveHandler the
        // exact same instances (C# keeps _pointerPressedHandler/_pointerReleasedHandler).
        const muxi::PointerEventHandler pressed{
            [peer](const winrt::Windows::Foundation::IInspectable&, const muxi::PointerRoutedEventArgs&) {
                if (peer->on_press)
                {
                    peer->on_press();
                }
            }};
        const auto boxed_pressed = winrt::box_value(pressed);
        button.AddHandler(mux::UIElement::PointerPressedEvent(), boxed_pressed, true);
        platform.pointer_pressed_handler = wnative::store(boxed_pressed);
        const muxi::PointerEventHandler released{
            [peer](const winrt::Windows::Foundation::IInspectable&, const muxi::PointerRoutedEventArgs&) {
                if (peer->on_release)
                {
                    peer->on_release();
                }
            }};
        const auto boxed_released = winrt::box_value(released);
        button.AddHandler(mux::UIElement::PointerReleasedEvent(), boxed_released, true);
        platform.pointer_released_handler = wnative::store(boxed_released);
        // ConnectHandler: _image.ImageOpened += OnImageOpened → VirtualView.UpdateIsLoading(false).
        // The port additionally re-runs measure: the BitmapImage decode is asynchronous, so the
        // Button's content measures 0×0 until the source opens (the image partial's twin).
        if (auto image = inner_image_of(platform))
        {
            const winrt::event_token opened =
                image.ImageOpened([this](const winrt::Windows::Foundation::IInspectable&, const mux::RoutedEventArgs&) {
                    if (auto* view = virtual_view())
                    {
                        view->update_is_loading(false);
                        view->invalidate_measure();
                    }
                });
            platform.image_opened_token = opened.value;
        }
        // deferred: _image.ImageFailed (logging + UpdateIsLoading(false)) and platformView.Unloaded
        // += OnUnloaded (the mid-press _isPressed Released safety net) — header deviations.
    }

    void image_button_handler::on_disconnect_handler(image_button_platform& platform)
    {
        // DisconnectHandler: Click -= OnClick; RemoveHandler(PointerPressed/ReleasedEvent, handler);
        // _image.ImageOpened -= OnImageOpened; then drop the kept delegates. The C++ callbacks are
        // cleared like the headless twin.
        platform.on_press = nullptr;
        platform.on_release = nullptr;
        platform.on_click = nullptr;
        if (auto button = button_of(platform))
        {
            if (platform.click_token != 0)
            {
                button.Click(winrt::event_token{platform.click_token});
            }
            if (platform.pointer_pressed_handler != nullptr)
            {
                const auto boxed =
                    wnative::borrow<winrt::Windows::Foundation::IInspectable>(platform.pointer_pressed_handler);
                button.RemoveHandler(mux::UIElement::PointerPressedEvent(), boxed);
            }
            if (platform.pointer_released_handler != nullptr)
            {
                const auto boxed =
                    wnative::borrow<winrt::Windows::Foundation::IInspectable>(platform.pointer_released_handler);
                button.RemoveHandler(mux::UIElement::PointerReleasedEvent(), boxed);
            }
        }
        if (platform.image_opened_token != 0)
        {
            if (auto image = inner_image_of(platform))
            {
                image.ImageOpened(winrt::event_token{platform.image_opened_token});
            }
            platform.image_opened_token = 0;
        }
        platform.click_token = 0;
        wnative::release(platform.pointer_pressed_handler);
        wnative::release(platform.pointer_released_handler);
    }

    // Leave the loader on its defaults (the synchronous read_uri_bytes fetch; disk layer off) — the
    // headless twin's wiring. deferred: an async HttpClient/RandomAccessStream uri fetch (the image
    // partial's identical deviation).
    void image_button_handler::configure_loader(image_source_loader& /*loader*/)
    {
    }

    void image_button_handler::map_aspect(image_button_handler& handler, i_image_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->image_aspect = view.aspect(); // the headless mirror
        // ImageHandler.MapAspect over IImageHandler.PlatformView — which for the windows ImageButton
        // is the INNER Image (GetContent<WImage>()) → ImageViewExtensions.UpdateAspect: Stretch =
        // aspect.ToStretch(); AspectFill re-centers the image inside the button (already Center from
        // construction — re-pushed for fidelity).
        if (auto image = inner_image_of(*platform))
        {
            image.Stretch(to_stretch(view.aspect()));
            if (view.aspect() == aspect::aspect_fill)
            {
                image.VerticalAlignment(mux::VerticalAlignment::Center);
                image.HorizontalAlignment(mux::HorizontalAlignment::Center);
            }
        }
    }

    // IsOpaque (headless mirror only): C# has no windows UpdateIsOpaque (the image partial's twin).
    void image_button_handler::map_is_opaque(image_button_handler& handler, i_image_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->opaque = view.is_opaque();
        }
    }

    // IsAnimationPlaying (headless mirror): C#'s UpdateIsAnimationPlaying drives BitmapImage.Play()/
    // Stop() — deferred with the image partial (ImageButton pins the value false anyway).
    void image_button_handler::map_is_animation_playing(image_button_handler& handler, i_image_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->animation_playing = view.is_animation_playing();
        }
    }

    void image_button_handler::map_padding(image_button_handler& handler, i_image_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->padding = view.padding();
        auto button = button_of(*platform);
        if (button == nullptr)
        {
            return;
        }
        // ImageButtonHandler.MapPadding → ButtonExtensions.UpdatePadding(button,
        // GetResource("ButtonPadding")): a NaN (unset) padding falls back to the theme's ButtonPadding
        // — ClearValue restores exactly that style value; an explicit padding is pushed directly (the
        // button partial's body).
        const thickness padding = view.padding();
        if (padding.is_nan())
        {
            button.ClearValue(muxc::Control::PaddingProperty());
        }
        else
        {
            button.Padding(wnative::to_thickness(padding));
        }
    }

    void image_button_handler::map_stroke_color(image_button_handler& handler, i_image_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->stroke_color = view.stroke_color();
        auto button = button_of(*platform);
        if (button == nullptr)
        {
            return;
        }
        // ImageButtonHandler.MapStrokeColor → ButtonExtensions.UpdateStrokeColor: null → RemoveKeys
        // (ButtonBorderBrush*); value → the border-brush resource keys. Direct BorderBrush here
        // (deferred: the per-state resource keys — header), null discriminated through
        // BindableObject.IsSet (the port's color has no null).
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        const bool color_is_set = bindable != nullptr && bindable->is_property_set("stroke_color");
        if (color_is_set)
        {
            button.BorderBrush(wnative::to_brush(view.stroke_color()));
        }
        else
        {
            button.ClearValue(muxc::Control::BorderBrushProperty());
        }
    }

    void image_button_handler::map_stroke_thickness(image_button_handler& handler, i_image_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->stroke_thickness = view.stroke_thickness();
        auto button = button_of(*platform);
        if (button != nullptr)
        {
            // ButtonExtensions.UpdateStrokeThickness: thickness >= 0 → the ButtonBorderThemeThickness
            // key (a uniform Thickness); negative → RemoveKeys (theme default). Direct BorderThickness
            // here (deferred: the resource key — header).
            const double value = view.stroke_thickness();
            if (value >= 0)
            {
                button.BorderThickness(wnative::to_thickness(thickness{value}));
            }
            else
            {
                button.ClearValue(muxc::Control::BorderThicknessProperty());
            }
        }
        // ImageButtonHandler.MapStrokeThickness: handler.UpdateValue(nameof(IImageButton.Padding)) —
        // the padding re-push (the theme pads a bordered button differently).
        map_padding(handler, view);
    }

    void image_button_handler::map_corner_radius(image_button_handler& handler, i_image_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->corner_radius = view.corner_radius();
        auto button = button_of(*platform);
        if (button != nullptr)
        {
            // ButtonExtensions.UpdateCornerRadius: radius >= 0 → the ControlCornerRadius key (a
            // uniform CornerRadius); negative (the control's -1 default = keep the native default) →
            // RemoveKeys. Direct Control.CornerRadius here (deferred: the resource key — header).
            const int radius = view.corner_radius();
            if (radius >= 0)
            {
                const auto uniform = static_cast<double>(radius);
                button.CornerRadius(mux::CornerRadius{
                    .TopLeft = uniform, .TopRight = uniform, .BottomRight = uniform, .BottomLeft = uniform});
            }
            else
            {
                button.ClearValue(muxc::Control::CornerRadiusProperty());
            }
        }
        // ImageButtonHandler.MapCornerRadius: handler.UpdateValue(nameof(IImageButton.Padding)); the
        // Shadow re-map is deferred (no shadow push on this backend yet).
        map_padding(handler, view);
    }

    // ---- per-backend source primitives (the cross-platform map_source routes here) ----

    // File fast-path: the mirror is ALWAYS maintained (kind="file" + path, marked loaded — the
    // XAML-less suite observes it); the native push decodes a BitmapImage over the file:/// URI and
    // lands it on the INNER Image's Source (ButtonExtensions.UpdateImageSource via the
    // ImageButtonImageSourcePartSetter). A failed decode leaves the image collapsed (the nil-decode
    // analog — cog.png is SVG-only and never rasterizes, exactly as on the other backends).
    void image_button_handler::load_file_source_sync(image_button_platform& platform,
                                                     const i_file_image_source& file_src)
    {
        platform.source_kind = "file";
        platform.source_file = std::string(file_src.file());
        platform.source_loaded = true;
        if (platform.image == nullptr)
        {
            return; // XAML-less: mirror only
        }
        set_inner_image_source(platform, bitmap_from_file(file_src.file()));
    }

    // The async loader's apply: copy the result's kind + detail into the mirror (a !loaded() result
    // clears, mirroring SetImageSource(null)). FILE and URI results reach the inner Image over the
    // same BitmapImage lane; stream/font byte decodes are deferred (header deviations).
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
        if (result.kind() == "file" || result.kind() == "uri")
        {
            if (platform.image == nullptr)
            {
                return;
            }
            set_inner_image_source(platform, result.kind() == "file" ? bitmap_from_file(result.detail())
                                                                     : bitmap_from_uri(result.detail()));
            return;
        }
        // deferred: stream results decode via an InMemoryRandomAccessStream + BitmapImage.SetSource;
        // font results raster via a CanvasImageSource — the mirror above keeps the load observable.
    }

    // Clear the loaded image: mirrors cleared + inner Image.Source = null and Collapsed
    // (ButtonExtensions.UpdateImageSource's null branch / ImageViewExtensions.Clear).
    void image_button_handler::clear_source_native(image_button_platform& platform)
    {
        platform.source_kind.clear();
        platform.source_file.clear();
        platform.source_loaded = false;
        if (platform.image != nullptr)
        {
            set_inner_image_source(platform, muxmi::BitmapImage{nullptr});
        }
    }

    maui::graphics::size image_button_handler::get_desired_size(double width_constraint,
                                                                double height_constraint) const
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
        // DesiredSize on the Button (the inner Image's opened natural size flows up through the
        // Button's own MeasureOverride), with the AdjustForExplicitSize clamp fed from the virtual
        // view's explicit width()/height() like every windows partial.
        const double explicit_width = virtual_view() != nullptr ? virtual_view()->width() : dimension::unset;
        const double explicit_height = virtual_view() != nullptr ? virtual_view()->height() : dimension::unset;
        return wnative::measure_native(platform->native, width_constraint, height_constraint, explicit_width,
                                       explicit_height);
    }

    void image_button_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // XAML-less: no native layout to apply
        }
        // The shared Canvas recipe: Canvas.SetLeft/SetTop + explicit Width/Height pin the Button to
        // the frame (ViewHandlerExtensions.Windows.cs PlatformArrangeHandler on the Canvas layout
        // model).
        wnative::arrange_native(platform->native, frame);
    }
} // namespace maui::core
