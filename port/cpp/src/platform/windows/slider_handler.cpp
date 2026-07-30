// slider_handler — WinUI 3 platform recipe: a real Microsoft.UI.Xaml.Controls.Slider, the same native
// type SliderHandler.Windows.cs creates. Ported from SliderHandler.Windows.cs + SliderExtensions.cs
// (Windows).
//
// THREE DOCUMENTED SIMPLIFICATIONS against C#, each narrowed on purpose rather than left vague:
//
//  1. C# creates a MauiSlider — a Slider subclass adding a ThumbImageSource dependency property and an
//     OnApplyTemplate override that swaps the Thumb's Style for "MauiSliderImageThumbStyle" (an
//     app-resource XAML style). The port has no mechanism yet for a custom XAML-templated control type
//     or for merging an app resource dictionary on this backend, so this slice creates a plain
//     winrt::Microsoft::UI::Xaml::Controls::Slider — everything Minimum/Maximum/Value/track/thumb-color
//     does is identical on the plain type; only the ThumbImageSource swap needs MauiSlider.
//  2. ThumbImageSource (apply_thumb_image / clear_thumb_image below) therefore stays MIRROR-ONLY, exactly
//     like button_handler.cpp's image-content primitives: the real recipe needs the MauiSlider style
//     swap above PLUS GetFirstDescendant<Thumb>() (a visual-tree walk this backend has no helper for
//     yet) PLUS decoding the loaded image into a native WinRT ImageSource (a conversion this backend's
//     image pipeline does not produce yet). map_thumb_image_source still runs end to end through the
//     shared image_source_loader.
//  3. map_update_on_tap mirrors the flag only: MAUI's real Windows SliderHandler has NO UpdateOnTap
//     remap at all (Slider.Mapper.cs's ReplaceMapping("UpdateOnTap", ...) targets iOS exclusively) — the
//     port's mapper table registers the "ios.Slider.UpdateOnTap" key for every backend (see
//     src/core/slider_handler.cpp), so this backend must still define the function, matching headless.
//  4. update_background does NOT delegate to the shared winui_visual_ops::apply_background every other
//     control's override uses. SliderHandler.cs's cross-platform Mapper carries a WINDOWS-ONLY remap —
//     `#if WINDOWS [nameof(ISlider.Background)] = MapBackgroundColor` — that replaces the generic
//     Background push with SliderExtensions.UpdateBackgroundColor's resource-key-override recipe (the
//     same shape as the track/thumb colors below): a plain Control.Background set does not survive the
//     control template's per-visual-state rebind. This is the one real, oracle-confirmed divergence from
//     the generic five-override set button/label use — not a simplification, a faithful port of a
//     Slider-specific remap that exists in the C# source.

#include "maui/core/slider_handler.hpp"

#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Input.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <memory>
#include <span>
#include <string>
#include <string_view>

#include "maui/core/bindable_object.hpp"
#include "maui/core/i_ios_slider_specifics.hpp"
#include "maui/core/i_slider.hpp"
#include "maui/core/image_source_loader.hpp"
#include "maui/core/image_source_result.hpp"
#include "maui/core/view_chrome_ops.hpp"
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
    using slider_control = winui::Controls::Slider;

    slider_control as_slider(void* native)
    {
        return maui::platform::windows::ref<winui::UIElement>(native).as<slider_control>();
    }

    // "Was this property explicitly set?" — see the twin in button_handler.cpp for why this must not be a
    // value comparison ([[cpp-unset-color-sentinel-collision]]).
    bool is_set(const maui::core::i_view& view, std::string_view property)
    {
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        return bindable != nullptr && bindable->is_property_set(property);
    }

    // SliderExtensions' resource-key sets — copied verbatim from the oracle, not invented.
    constexpr std::array<std::wstring_view, 4> k_minimum_track_color_keys{
        L"SliderTrackValueFill", L"SliderTrackValueFillPointerOver", L"SliderTrackValueFillPressed",
        L"SliderTrackValueFillDisabled"};
    constexpr std::array<std::wstring_view, 4> k_maximum_track_color_keys{
        L"SliderTrackFill", L"SliderTrackFillPointerOver", L"SliderTrackFillPressed", L"SliderTrackFillDisabled"};
    constexpr std::array<std::wstring_view, 4> k_thumb_color_keys{
        L"SliderThumbBackground", L"SliderThumbBackgroundPointerOver", L"SliderThumbBackgroundPressed",
        L"SliderThumbBackgroundDisabled"};
    // SliderExtensions.cs BackgroundColorResourceKeys — see update_background's file-top note 4: unlike
    // every other control here, Slider's cross-platform Mapper (SliderHandler.cs) REMAPS Background to
    // this resource-key recipe on Windows only (`#if WINDOWS`), instead of the generic ViewHandler push.
    constexpr std::array<std::wstring_view, 4> k_background_color_keys{
        L"SliderContainerBackground", L"SliderContainerBackgroundPointerOver", L"SliderContainerBackgroundPressed",
        L"SliderContainerBackgroundDisabled"};

    void set_resources(const slider_control& slider, std::span<const std::wstring_view> keys,
                       const winui::Media::Brush& brush)
    {
        for (const auto& key : keys)
        {
            slider.Resources().Insert(winrt::box_value(winrt::hstring{key}), brush);
        }
    }

    void remove_resources(const slider_control& slider, std::span<const std::wstring_view> keys)
    {
        for (const auto& key : keys)
        {
            slider.Resources().Remove(winrt::box_value(winrt::hstring{key}));
        }
    }

    // FrameworkElementExtensions.RefreshThemeResources: flip RequestedTheme away and back so the control
    // template re-resolves the resources just overridden.
    void refresh_theme_resources(const winui::FrameworkElement& element)
    {
        const auto previous = element.RequestedTheme();
        element.RequestedTheme(element.ActualTheme() == winui::ElementTheme::Dark ? winui::ElementTheme::Light
                                                                                  : winui::ElementTheme::Dark);
        element.RequestedTheme(previous);
    }

    // SliderExtensions.UpdateIncrement: reads the DIFFERENCE OFF THE VIRTUAL VIEW (not the native control),
    // exactly as the oracle does — `slider.Maximum - slider.Minimum`, both ISlider properties.
    void update_increment(const slider_control& native_slider, maui::core::i_slider& view)
    {
        const double difference = view.maximum() - view.minimum();
        double stepping = 1.0;
        // "Setting SmallChange to 0 would throw an ArgumentException."
        if (difference != 0)
        {
            stepping = std::min(difference / 1000.0, 1.0);
        }
        native_slider.StepFrequency(stepping);
    }

    // The two pointer delegates, kept alive for as long as they are subscribed — same rationale as
    // button_handler.cpp's pointer_sink (RemoveHandler matches on the delegate OBJECT, not a token).
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
        // Unhook everything on_connect_handler registered. Called from on_disconnect_handler AND from
        // ~slider_platform: the handlers capture a slider_platform*, so if the struct is destroyed while
        // still subscribed the next pointer/value event fires into freed memory. Every revoke below is a
        // safe no-op on a token/delegate that was never registered (WinRT event revocation ignores an
        // unknown token), so this runs unconditionally regardless of whether Loaded ever fired.
        void detach_native_events(maui::core::slider_platform& platform)
        {
            if (platform.native != nullptr)
            {
                const slider_control slider = as_slider(platform.native);
                slider.Loaded(winrt::event_token{platform.loaded_token});
                slider.ValueChanged(winrt::event_token{platform.value_changed_token});
                if (platform.pointer_events != nullptr)
                {
                    auto* sink = static_cast<pointer_sink*>(platform.pointer_events);
                    slider.RemoveHandler(winui::UIElement::PointerPressedEvent(), winrt::box_value(sink->pressed));
                    slider.RemoveHandler(winui::UIElement::PointerReleasedEvent(), winrt::box_value(sink->released));
                    slider.RemoveHandler(winui::UIElement::PointerCanceledEvent(), winrt::box_value(sink->released));
                }
            }
            platform.loaded_token = 0;
            platform.value_changed_token = 0;
            delete static_cast<pointer_sink*>(platform.pointer_events);
            platform.pointer_events = nullptr;
        }
    } // namespace

    slider_platform::~slider_platform()
    {
        detach_native_events(*this);
        if (native != nullptr)
        {
            maui::platform::windows::drop<winui::UIElement>(native);
        }
    }

    std::unique_ptr<slider_platform> slider_handler::create_platform_view()
    {
        auto platform = std::make_unique<slider_platform>();
        slider_control slider;
        slider.IsThumbToolTipEnabled(false);
        platform->native = maui::platform::windows::take<winui::UIElement>(slider);
        return platform;
    }

    void slider_handler::on_connect_handler(slider_platform& platform)
    {
        // The cross-platform half first — identical to the headless partial's contract, so a test (and
        // the DevFlow tap driver on the backends that have it) can raise these without a real pointer.
        platform.on_value_changed = [this] {
            auto* view = virtual_view();
            auto* platform_view = typed_platform_view();
            if (view != nullptr && platform_view != nullptr)
            {
                view->set_value(platform_view->value);
            }
        };
        platform.on_drag_started = [this] {
            if (auto* view = virtual_view())
            {
                view->send_drag_started();
            }
        };
        platform.on_drag_completed = [this] {
            if (auto* view = virtual_view())
            {
                view->send_drag_completed();
            }
        };
        if (platform.native == nullptr)
        {
            return;
        }
        auto* self = &platform;
        const slider_control slider = as_slider(platform.native);

        // ValueChanged is subscribed from Loaded, NOT here — see the header's loaded_token/
        // value_changed_token comment for why (SliderHandler.Windows.cs OnPlatformViewLoaded). Also
        // where C# captures GetFirstDescendant<Thumb>()'s size for the ThumbImageSource reset path; that
        // capture is NOT ported (see the file-top note 2), so this closure only does the subscription.
        platform.loaded_token =
            slider
                .Loaded([self](const winrt::Windows::Foundation::IInspectable&, const winui::RoutedEventArgs&) {
                    if (self->native == nullptr)
                    {
                        return;
                    }
                    const slider_control native_slider = as_slider(self->native);
                    self->value_changed_token =
                        native_slider
                            .ValueChanged(
                                [self](const winrt::Windows::Foundation::IInspectable&,
                                       const winui::Controls::Primitives::RangeBaseValueChangedEventArgs& args) {
                                    self->value = args.NewValue();
                                    if (self->on_value_changed)
                                    {
                                        self->on_value_changed();
                                    }
                                })
                            .value;
                })
                .value;

        // The POINTER events, exactly like button_handler.cpp: RangeBase's control template marks
        // PointerPressed/PointerReleased handled, so a plain subscription is never invoked and
        // send_drag_started/send_drag_completed would silently never fire. C#'s ConnectHandler uses
        // AddHandler(..., handledEventsToo: true) for exactly this reason, and so does this. The SAME
        // released handler is installed for BOTH PointerReleased and PointerCanceled, matching C#'s
        // reuse of _pointerReleasedHandler for both events.
        auto sink = std::make_unique<pointer_sink>();
        sink->pressed = winui::Input::PointerEventHandler(
            [self](const winrt::Windows::Foundation::IInspectable&, const winui::Input::PointerRoutedEventArgs&) {
                if (self->on_drag_started)
                {
                    self->on_drag_started();
                }
            });
        sink->released = winui::Input::PointerEventHandler(
            [self](const winrt::Windows::Foundation::IInspectable&, const winui::Input::PointerRoutedEventArgs&) {
                if (self->on_drag_completed)
                {
                    self->on_drag_completed();
                }
            });
        slider.AddHandler(winui::UIElement::PointerPressedEvent(), winrt::box_value(sink->pressed), true);
        slider.AddHandler(winui::UIElement::PointerReleasedEvent(), winrt::box_value(sink->released), true);
        slider.AddHandler(winui::UIElement::PointerCanceledEvent(), winrt::box_value(sink->released), true);
        platform.pointer_events = sink.release();
    }

    void slider_handler::on_disconnect_handler(slider_platform& platform)
    {
        detach_native_events(platform);
        platform.on_value_changed = nullptr;
        platform.on_drag_started = nullptr;
        platform.on_drag_completed = nullptr;
    }

    void slider_handler::map_minimum(slider_handler& handler, i_slider& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->minimum = view.minimum();
        const slider_control slider = as_slider(platform->native);
        slider.Minimum(platform->minimum);
        update_increment(slider, view);
    }

    void slider_handler::map_maximum(slider_handler& handler, i_slider& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->maximum = view.maximum();
        const slider_control slider = as_slider(platform->native);
        slider.Maximum(platform->maximum);
        update_increment(slider, view);
    }

    void slider_handler::map_value(slider_handler& handler, i_slider& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->value = view.value();
        // SliderExtensions.UpdateValue: write only when it differs (prevents the native ValueChanged
        // echo from looping).
        const slider_control slider = as_slider(platform->native);
        if (slider.Value() != platform->value)
        {
            slider.Value(platform->value);
        }
    }

    void slider_handler::map_minimum_track_color(slider_handler& handler, i_slider& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->minimum_track_color = view.minimum_track_color();
        const slider_control slider = as_slider(platform->native);
        // C#: a null MinimumTrackColor REMOVES the resource override so the theme brush returns.
        if (!is_set(view, "minimum_track_color"))
        {
            remove_resources(slider, k_minimum_track_color_keys);
        }
        else
        {
            const winui::Media::SolidColorBrush brush{
                maui::platform::windows::to_ui_color(platform->minimum_track_color)};
            set_resources(slider, k_minimum_track_color_keys, brush);
        }
        refresh_theme_resources(slider);
    }

    void slider_handler::map_maximum_track_color(slider_handler& handler, i_slider& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->maximum_track_color = view.maximum_track_color();
        const slider_control slider = as_slider(platform->native);
        if (!is_set(view, "maximum_track_color"))
        {
            remove_resources(slider, k_maximum_track_color_keys);
        }
        else
        {
            const winui::Media::SolidColorBrush brush{
                maui::platform::windows::to_ui_color(platform->maximum_track_color)};
            set_resources(slider, k_maximum_track_color_keys, brush);
        }
        refresh_theme_resources(slider);
    }

    void slider_handler::map_thumb_color(slider_handler& handler, i_slider& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->thumb_color = view.thumb_color();
        const slider_control slider = as_slider(platform->native);
        if (!is_set(view, "thumb_color"))
        {
            remove_resources(slider, k_thumb_color_keys);
        }
        else
        {
            const winui::Media::SolidColorBrush brush{maui::platform::windows::to_ui_color(platform->thumb_color)};
            set_resources(slider, k_thumb_color_keys, brush);
        }
        refresh_theme_resources(slider);
    }

    // Per-backend loader wiring: no real fetch infrastructure exists for this backend's thumb-image path
    // yet (see the file-top note 2), so the loader stays on its synchronous defaults, same as headless.
    void slider_handler::configure_thumb_loader(image_source_loader& /*loader*/)
    {
    }

    // MIRROR-ONLY (file-top note 2): the real Windows recipe needs the MauiSlider Thumb-style swap, which
    // this backend does not have. `view` is unused here for the same reason apple/headless's is — the
    // WinUI recipe would tint via the Thumb's Tag, not by re-reading the color, once that swap exists.
    void slider_handler::apply_thumb_image(slider_platform& platform, i_slider& /*view*/,
                                           const image_source_result& /*result*/)
    {
        platform.thumb_image_set = true;
    }

    void slider_handler::clear_thumb_image(slider_platform& platform, i_slider& view)
    {
        platform.thumb_image_set = false;
        platform.thumb_color = view.thumb_color();
    }

    // Slider.MapUpdateOnTap: MAUI's real Windows backend has NO UpdateOnTap remap at all (file-top note
    // 3) — mirror the resolved flag only, matching headless, since there is no native gesture to
    // install/remove on this platform.
    void slider_handler::map_update_on_tap(slider_handler& handler, i_slider& view)
    {
        auto* platform = handler.typed_platform_view();
        const auto* specifics = dynamic_cast<const i_ios_slider_specifics*>(&view);
        if (platform == nullptr || specifics == nullptr)
        {
            return;
        }
        platform->update_on_tap = specifics->update_on_tap();
    }

    maui::graphics::size slider_handler::get_desired_size(double width_constraint, double height_constraint) const
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
        const slider_control slider = as_slider(platform->native);
        // ARRANGE/EXPLICIT-SIZE FIX (generalised from image_button_handler.cpp, commit a2444f94ba): pin
        // Width/Height to the view's own explicit request instead of clearing to NaN unconditionally,
        // then only WIDEN the incoming constraint at measure time — see the oracle at
        // ViewHandlerExtensions.Windows.cs:56-74 GetDesiredSizeFromHandler + :91-105 AdjustForExplicitSize,
        // and image_button_handler.cpp's get_desired_size for the full writeup. platform_arrange's OWN
        // stamp (below) is UNTOUCHED.
        const auto* view = virtual_view();
        const double explicit_width = (view != nullptr) ? view->width() : std::numeric_limits<double>::quiet_NaN();
        const double explicit_height = (view != nullptr) ? view->height() : std::numeric_limits<double>::quiet_NaN();
        slider.Width(explicit_width);
        slider.Height(explicit_height);
        const double adjusted_width_constraint =
            std::isnan(explicit_width) ? width_constraint : std::max(width_constraint, explicit_width);
        const double adjusted_height_constraint =
            std::isnan(explicit_height) ? height_constraint : std::max(height_constraint, explicit_height);
        slider.Measure(
            winrt::Windows::Foundation::Size{maui::platform::windows::measure_constraint(adjusted_width_constraint),
                                             maui::platform::windows::measure_constraint(adjusted_height_constraint)});
        const auto desired = slider.DesiredSize();
        return {desired.Width, desired.Height};
    }

    void slider_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        // PlatformArrangeHandler's guard, WIDENED to non-finite — see button_handler.cpp's
        // platform_arrange for why (an unrecoverable stowed exception, 0xC000027B, otherwise).
        if (!std::isfinite(frame.x) || !std::isfinite(frame.y) || !std::isfinite(frame.width) ||
            !std::isfinite(frame.height) || frame.width < 0 || frame.height < 0)
        {
            return;
        }
        const slider_control slider = as_slider(platform->native);
        winui::Controls::Canvas::SetLeft(slider, frame.x);
        winui::Controls::Canvas::SetTop(slider, frame.y);
        slider.Width(frame.width);
        slider.Height(frame.height);
        // Clip is bounds-dependent (view_chrome_ops.cpp's apply_native_clip reads the just-set Width/
        // Height back); map_clip's own push (view_mapper.cpp) always runs before the first arrange, so
        // this re-invoke is what actually installs the clip once the slider has a real size.
        if (const auto* view = virtual_view(); view != nullptr)
        {
            apply_native_clip(platform->native, view->clip());
        }
    }

    // ---- generic-IView property pushes (view_platform_base overrides) ---------------------------
    // The first four delegate to the shared winui_visual_ops free functions, same as button/label; see
    // that header for why they are free functions taking the void* slot. update_background below is the
    // one exception — see its own comment and file-top note 4.
    void slider_platform::update_visibility(maui::core::visibility value)
    {
        maui::platform::windows::apply_visibility(native, value);
    }

    void slider_platform::update_opacity(double value)
    {
        maui::platform::windows::apply_opacity(native, value);
    }

    void slider_platform::update_is_enabled(bool value)
    {
        maui::platform::windows::apply_is_enabled(native, value);
    }

    void slider_platform::update_automation_id(std::string_view value)
    {
        maui::platform::windows::apply_automation_id(native, value);
    }

    void slider_platform::update_background(const maui::graphics::paint* value)
    {
        // NOT the generic apply_background push — see the file-top note 4. SliderExtensions.cs
        // UpdateBackgroundColor: a null paint removes the resource override (the theme brush returns); a
        // set paint overrides all four visual-state keys via the same brush translation apply_background
        // uses (exported as maui::platform::windows::brush_for for exactly this reuse).
        if (native == nullptr)
        {
            return;
        }
        const slider_control slider = as_slider(native);
        if (value == nullptr)
        {
            remove_resources(slider, k_background_color_keys);
        }
        else
        {
            set_resources(slider, k_background_color_keys, maui::platform::windows::brush_for(*value));
        }
        refresh_theme_resources(slider);
    }
} // namespace maui::core
