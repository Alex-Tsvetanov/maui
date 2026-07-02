// slider_handler — Windows (WinUI 3) platform partial: a REAL Microsoft.UI.Xaml.Controls.Slider. The
// windows twin of src/platform/apple/slider_handler.mm (NSSlider) / the android JNI partial, and the
// real-native sibling of the headless mirror partial (src/platform/headless/slider_handler.cpp).
// Minimum/Maximum/Value map onto the RangeBase properties (with C#'s derived StepFrequency), the native
// ValueChanged flows back through on_value_changed into i_range::set_value, and the pointer press/
// release/cancel events stand in for DragStarted/DragCompleted exactly like SliderHandler.Windows.cs.
//
// Ported DIRECTLY from SliderHandler.Windows.cs + Platform/Windows/SliderExtensions.cs
// (UpdateIncrement/UpdateMinimum/UpdateMaximum/UpdateValue + the color/thumb maps) +
// ViewExtensions.cs (the generic-IView pushes).
//
// DOCUMENTED DEVIATIONS (each an infrastructure gap of this first cut, not a behavior guess):
//   - The control is a stock Slider, not the internal MauiSlider (Platform/Windows/MauiSlider.cs — a
//     Slider subclass adding a ThumbImageSource dependency property + a re-templated thumb): the image
//     fan-out has not reached this backend. CreatePlatformView's IsThumbToolTipEnabled = false IS
//     ported. The thumb-image primitives below are headless-mirror stubs (// deferred) for the same
//     reason, matching the android twin.
//   - MinimumTrackColor / MaximumTrackColor / ThumbColor are MIRROR-ONLY (// deferred at the maps):
//     C#'s UpdateColor writes the per-state resource keys (SliderTrackValueFill* / SliderTrackFill* /
//     SliderThumbBackground*) + RefreshThemeResources — the resource-dictionary seam deferral shared
//     with the button/switch partials (a Slider has no direct one-brush track/thumb property).
//   - C# wires ValueChanged inside OnPlatformViewLoaded (the Loaded event, which also captures the
//     default thumb size for the image swap); the port wires it directly in ConnectHandler — no Loaded
//     hook is needed because the thumb-size capture belongs to the deferred thumb-image pipeline.
//
// XAML-less degradation (the windows twin of the android VM-less fallback): the windows preset also
// runs the cross-platform suite on the host where NO XAML runtime exists; create_platform_view catches
// the construction failure and keeps native null, while the headless mirrors are ALWAYS maintained and
// on_value_changed/on_drag_started/on_drag_completed stay invokable C++ callbacks (the cross-platform
// suite drives them) — so that suite observes exactly the headless partial's behavior.

#include "maui/core/slider_handler.hpp"

#include <algorithm>
#include <memory>
#include <string_view>

#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Input.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/base.h>

#include "maui/core/dimension.hpp"
#include "maui/core/i_ios_slider_specifics.hpp"
#include "maui/core/i_slider.hpp"
#include "maui/core/image_source_loader.hpp"
#include "maui/core/image_source_result.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "windows_native.hpp"

namespace
{
    namespace mux = winrt::Microsoft::UI::Xaml;
    namespace muxc = winrt::Microsoft::UI::Xaml::Controls;
    namespace muxcp = winrt::Microsoft::UI::Xaml::Controls::Primitives;
    namespace muxi = winrt::Microsoft::UI::Xaml::Input;
    namespace wnative = maui::platform::win;

    [[nodiscard]] muxc::Slider slider_of(const maui::core::slider_platform& platform)
    {
        return wnative::borrow<muxc::Slider>(platform.native);
    }

    // SliderExtensions.UpdateIncrement (private — called by BOTH UpdateMinimum and UpdateMaximum):
    // StepFrequency = min((Maximum - Minimum) / 1000, 1), or 1 when the range is empty ("Setting the
    // Slider SmallChange property to 0 would throw an System.ArgumentException").
    void update_increment(const muxc::Slider& slider, const maui::core::i_slider& view)
    {
        const double difference = view.maximum() - view.minimum();
        double stepping = 1.0;
        if (difference != 0)
        {
            stepping = std::min(difference / 1000.0, 1.0);
        }
        slider.StepFrequency(stepping);
    }
} // namespace

namespace maui::core
{
    // Releases the one strong ref pinning the Slider + any event-wiring remainder (the wnative shape of
    // the pimpl-owned-native doctrine; the apple twin CFReleases its NSSlider here). The handler slots
    // are normally released in on_disconnect_handler — this is the defensive sweep.
    slider_platform::~slider_platform()
    {
        wnative::release(pointer_pressed_handler);
        wnative::release(pointer_released_handler);
        wnative::release(native);
    }

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each
    // calls the base body FIRST — the headless mirrors must stay live for the XAML-less cross-platform
    // suite (header note) — then pushes to the real Slider when one exists.

    void slider_platform::update_visibility(maui::core::visibility value)
    {
        view_platform_base::update_visibility(value);
        // ViewExtensions.UpdateVisibility (Windows): Hidden rides Opacity 0, Collapsed collapses.
        wnative::apply_visibility(native, value, alpha);
    }

    void slider_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        // ViewExtensions.UpdateOpacity: the Hidden state pins opacity 0 (apply_visibility restores it).
        if (!hidden)
        {
            wnative::apply_opacity(native, value);
        }
    }

    void slider_platform::update_is_enabled(bool value)
    {
        view_platform_base::update_is_enabled(value);
        // ViewExtensions.UpdateIsEnabled → ControlExtensions.UpdateIsEnabled: Control.IsEnabled (the
        // Slider IS a Control).
        if (auto slider = slider_of(*this))
        {
            slider.IsEnabled(value);
        }
    }

    void slider_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        // ViewExtensions.UpdateAutomationId: AutomationProperties.SetAutomationId.
        wnative::apply_automation_id(native, value);
    }

    std::unique_ptr<slider_platform> slider_handler::create_platform_view()
    {
        auto platform = std::make_unique<slider_platform>();
        try
        {
            // SliderHandler.Windows.CreatePlatformView: new MauiSlider { IsThumbToolTipEnabled = false }
            // — a stock Slider here (MauiSlider's ThumbImageSource re-template is deferred with the
            // image fan-out; header deviations).
            const muxc::Slider slider;
            slider.IsThumbToolTipEnabled(false);
            platform->native = wnative::store(slider); // released in ~slider_platform
        }
        catch (const winrt::hresult_error&)
        {
            platform->native = nullptr; // XAML-less degradation (header note)
        }
        return platform;
    }

    void slider_handler::on_connect_handler(slider_platform& platform)
    {
        // SliderHandler.Windows's event split: ValueChanged → Value write-back (OnPlatformValueChanged);
        // PointerPressed → DragStarted (OnPointerPressed); PointerReleased + PointerCanceled →
        // DragCompleted (OnPointerReleased). The callbacks stay wired even XAML-less so the
        // cross-platform suite can drive them (the headless twin's exact bodies).
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
        auto slider = slider_of(platform);
        if (slider == nullptr)
        {
            return;
        }
        // The native events route through the platform callbacks (the peer is the platform struct,
        // whose heap address is stable until disconnect revokes these handlers).
        auto* peer = &platform;
        // OnPlatformValueChanged: `if (VirtualView != null && VirtualView.Value != e.NewValue)
        // VirtualView.Value = e.NewValue` — the mirror takes e.NewValue (the platform `value` doubles
        // as the native thumb position, the headless convention) and on_value_changed writes it back;
        // the echo dies in map_value's differ guard, exactly like C#. Wired directly here rather than in
        // Loaded (header deviations).
        const winrt::event_token value_token = slider.ValueChanged(
            [peer](const winrt::Windows::Foundation::IInspectable&, const muxcp::RangeBaseValueChangedEventArgs& e) {
                peer->value = e.NewValue();
                if (peer->on_value_changed)
                {
                    peer->on_value_changed();
                }
            });
        platform.value_changed_token = value_token.value;
        // ConnectHandler: AddHandler(PointerPressedEvent/PointerReleasedEvent/PointerCanceledEvent,
        // handler, true) — handledEventsToo, because the Slider control class marks the pointer events
        // handled internally. The boxed delegates are retained so DisconnectHandler can RemoveHandler
        // the exact same instances (C# keeps _pointerPressedHandler/_pointerReleasedHandler); the
        // released handler serves BOTH PointerReleased and PointerCanceled, like C#.
        const muxi::PointerEventHandler pressed{
            [peer](const winrt::Windows::Foundation::IInspectable&, const muxi::PointerRoutedEventArgs&) {
                if (peer->on_drag_started)
                {
                    peer->on_drag_started();
                }
            }};
        const auto boxed_pressed = winrt::box_value(pressed);
        slider.AddHandler(mux::UIElement::PointerPressedEvent(), boxed_pressed, true);
        platform.pointer_pressed_handler = wnative::store(boxed_pressed);
        const muxi::PointerEventHandler released{
            [peer](const winrt::Windows::Foundation::IInspectable&, const muxi::PointerRoutedEventArgs&) {
                if (peer->on_drag_completed)
                {
                    peer->on_drag_completed();
                }
            }};
        const auto boxed_released = winrt::box_value(released);
        slider.AddHandler(mux::UIElement::PointerReleasedEvent(), boxed_released, true);
        slider.AddHandler(mux::UIElement::PointerCanceledEvent(), boxed_released, true);
        platform.pointer_released_handler = wnative::store(boxed_released);
    }

    void slider_handler::on_disconnect_handler(slider_platform& platform)
    {
        // DisconnectHandler: ValueChanged -= OnPlatformValueChanged; RemoveHandler(PointerPressed/
        // Released/CanceledEvent, handler); then drop the kept delegates (_thumbSize = null belongs to
        // the deferred thumb-image pipeline). The C++ callbacks are cleared like the headless twin.
        platform.on_value_changed = nullptr;
        platform.on_drag_started = nullptr;
        platform.on_drag_completed = nullptr;
        if (auto slider = slider_of(platform))
        {
            if (platform.value_changed_token != 0)
            {
                slider.ValueChanged(winrt::event_token{platform.value_changed_token});
            }
            if (platform.pointer_pressed_handler != nullptr)
            {
                const auto boxed =
                    wnative::borrow<winrt::Windows::Foundation::IInspectable>(platform.pointer_pressed_handler);
                slider.RemoveHandler(mux::UIElement::PointerPressedEvent(), boxed);
            }
            if (platform.pointer_released_handler != nullptr)
            {
                const auto boxed =
                    wnative::borrow<winrt::Windows::Foundation::IInspectable>(platform.pointer_released_handler);
                slider.RemoveHandler(mux::UIElement::PointerReleasedEvent(), boxed);
                slider.RemoveHandler(mux::UIElement::PointerCanceledEvent(), boxed);
            }
        }
        platform.value_changed_token = 0;
        wnative::release(platform.pointer_pressed_handler);
        wnative::release(platform.pointer_released_handler);
    }

    void slider_handler::map_minimum(slider_handler& handler, i_slider& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->minimum = view.minimum(); // headless mirror first (XAML-less suite)
        // SliderExtensions.UpdateMinimum: Minimum = slider.Minimum, then the derived StepFrequency.
        if (auto slider = slider_of(*platform))
        {
            slider.Minimum(view.minimum());
            update_increment(slider, view);
        }
    }

    void slider_handler::map_maximum(slider_handler& handler, i_slider& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->maximum = view.maximum(); // headless mirror first (XAML-less suite)
        // SliderExtensions.UpdateMaximum: Maximum = slider.Maximum, then the derived StepFrequency.
        if (auto slider = slider_of(*platform))
        {
            slider.Maximum(view.maximum());
            update_increment(slider, view);
        }
    }

    void slider_handler::map_value(slider_handler& handler, i_slider& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // SliderExtensions.UpdateValue: write only when it differs (prevents the native ValueChanged
        // echo from looping). Mirror first with the same guard (the headless twin's body); note the
        // mapper table pushes Minimum/Maximum BEFORE Value, so RangeBase's coercion cannot clamp a
        // value that is inside the (already-pushed) range.
        if (platform->value != view.value())
        {
            platform->value = view.value();
        }
        if (auto slider = slider_of(*platform))
        {
            if (slider.Value() != view.value())
            {
                slider.Value(view.value());
            }
        }
    }

    void slider_handler::map_minimum_track_color(slider_handler& handler, i_slider& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->minimum_track_color = view.minimum_track_color(); // headless mirror (XAML-less suite)
        }
        // deferred: SliderExtensions.UpdateMinimumTrackColor — SetValueForAllKey/RemoveKeys on the
        // SliderTrackValueFill* per-state resource keys + RefreshThemeResources; needs the port's
        // resource-dictionary seam (header deviations).
    }

    void slider_handler::map_maximum_track_color(slider_handler& handler, i_slider& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->maximum_track_color = view.maximum_track_color(); // headless mirror (XAML-less suite)
        }
        // deferred: SliderExtensions.UpdateMaximumTrackColor — the SliderTrackFill* per-state resource
        // keys; same resource-dictionary seam deferral.
    }

    void slider_handler::map_thumb_color(slider_handler& handler, i_slider& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->thumb_color = view.thumb_color(); // headless mirror (XAML-less suite)
        }
        // deferred: SliderExtensions.UpdateThumbColor — the SliderThumbBackground* per-state resource
        // keys; same resource-dictionary seam deferral.
    }

    // ---- per-backend thumb-image primitives (the cross-platform map_thumb_image_source routes here) --
    // deferred: the windows thumb image rides MauiSlider.ThumbImageSource (a re-templated thumb whose
    // ImageBrush the SliderExtensions.UpdateThumbImageSourceAsync BitmapImage pipeline fills) — the
    // stock-Slider cut has no image part and no decode pipeline on this backend yet (header
    // deviations). The primitives keep the shared headless mirrors so the cross-platform suite still
    // observes the load, exactly like the android twin.

    // deferred: leave the loader on its defaults (no BitmapImage seam yet).
    void slider_handler::configure_thumb_loader(image_source_loader& /*loader*/)
    {
    }

    // Headless-mirror body: a loaded result is recorded as "a thumb image is set" (the thumb tint is
    // conceptually cleared while an image shows — SliderExtensions swaps the thumb template).
    void slider_handler::apply_thumb_image(slider_platform& platform, i_slider& /*view*/,
                                           const image_source_result& /*result*/)
    {
        platform.thumb_image_set = true;
    }

    // Headless-mirror body: clearing the thumb image restores the thumb color mirror
    // (UpdateThumbImageSourceAsync's null branch → the default thumb + UpdateThumbColor re-applies).
    void slider_handler::clear_thumb_image(slider_platform& platform, i_slider& view)
    {
        platform.thumb_image_set = false;
        platform.thumb_color = view.thumb_color();
    }

    // Slider.MapUpdateOnTap is an iOS platform-specific (Slider.iOS.cs UpdateOnTap gesture); windows
    // records the resolved flag like the headless/android twins — the WinUI Slider already moves the
    // thumb on a track tap natively, so there is no gesture to install.
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
        if (platform == nullptr)
        {
            return {0, 0};
        }
        if (platform->native == nullptr)
        {
            // XAML-less degradation: the headless partial's nominal track placeholder (100x31), so the
            // backend-agnostic size-request suites see consistent numbers (the android twin's shape).
            return {100.0, 31.0};
        }
        // ViewHandlerExtensions.Windows.cs GetDesiredSizeFromHandler: FrameworkElement.Measure +
        // DesiredSize, with the AdjustForExplicitSize clamp fed from the virtual view's explicit
        // width()/height() (see measure_native).
        const double explicit_width = virtual_view() != nullptr ? virtual_view()->width() : dimension::unset;
        const double explicit_height = virtual_view() != nullptr ? virtual_view()->height() : dimension::unset;
        return wnative::measure_native(platform->native, width_constraint, height_constraint, explicit_width,
                                       explicit_height);
    }

    void slider_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // XAML-less: no native layout to apply
        }
        // The shared Canvas recipe: Canvas.SetLeft/SetTop + explicit Width/Height pin the Slider to the
        // frame (ViewHandlerExtensions.Windows.cs PlatformArrangeHandler on the Canvas model).
        wnative::arrange_native(platform->native, frame);
    }
} // namespace maui::core
