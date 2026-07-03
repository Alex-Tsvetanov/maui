// switch_handler — Windows (WinUI 3) platform partial: a REAL Microsoft.UI.Xaml.Controls.ToggleSwitch.
// The windows twin of src/platform/apple/switch_handler.mm (NSSwitch) / the android JNI partial, and
// the real-native sibling of the headless mirror partial (src/platform/headless/switch_handler.cpp).
// IsOn maps onto ToggleSwitch.IsOn, and the native Toggled event flows back through on_value_changed
// into i_switch::set_is_on (guarded against echo, exactly like C#'s OnToggled).
//
// Ported DIRECTLY from SwitchHandler.Windows.cs + Platform/Windows/SwitchExtensions.cs
// (UpdateIsToggled/UpdateTrackColor/UpdateThumbColor/UpdateMinWidth) + ViewExtensions.cs
// (UpdateVisibility/UpdateOpacity/UpdateIsEnabled/UpdateAutomationId).
//
// DOCUMENTED DEVIATIONS (each an infrastructure gap of this first cut, not a behavior guess):
//   - TrackColor / ThumbColor are MIRROR-ONLY (// deferred at the maps): C#'s UpdateTrackColor writes
//     the ToggleSwitchFillOn*/ToggleSwitchFillOff* resource keys (per toggle state) and UpdateThumbColor
//     the ToggleSwitchKnobFill* keys, both followed by RefreshThemeResources — the per-state resource-
//     dictionary theming needs the port's resource-dictionary seam (the same deferral as the button's
//     ButtonBackground* keys).
//   - C#'s OnLoaded template surgery (zeroing the SwitchAreaGrid root grid's spacing column — the
//     On/Off content column gap) is deferred: it needs a Loaded hook + a visual-tree descendant walk.
//     The cheap width half of the same fix IS ported: create_platform_view sets MinWidth = 0, the exact
//     effect of MapSwitchMinimumWidth → UpdateMinWidth's NaN (unset MinimumWidth) branch — dropping
//     WinUI's default 154 MinWidth reserved for the OnContent/OffContent labels MAUI never shows.
//   - The switch_platform `container` slot (the iOS >101pt accessibility wrapper) stays null on this
//     backend, like android — no windows on_setup_container hook is declared, so the shared
//     container_view map records the no-op exactly as headless.
//
// XAML-less degradation (the windows twin of the android VM-less fallback): the windows preset also
// runs the cross-platform suite on the host where NO XAML runtime exists; create_platform_view catches
// the construction failure and keeps native null, while the headless mirrors are ALWAYS maintained and
// on_value_changed stays an invokable C++ callback (the cross-platform suite drives it) — so that
// suite observes exactly the headless partial's behavior.

#include "maui/core/switch_handler.hpp"

#include <memory>
#include <string_view>

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h> // ResourceDictionary IMap Insert
#include <winrt/Windows.Foundation.h>
#include <winrt/base.h>

#include <initializer_list>

#include "maui/core/bindable_object.hpp"

#include "maui/core/dimension.hpp"
#include "maui/core/i_switch.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "windows_native.hpp"

namespace
{
    namespace mux = winrt::Microsoft::UI::Xaml;
    namespace muxc = winrt::Microsoft::UI::Xaml::Controls;
    namespace wnative = maui::platform::win;

    [[nodiscard]] muxc::ToggleSwitch switch_of(const maui::core::switch_platform& platform)
    {
        return wnative::borrow<muxc::ToggleSwitch>(platform.native);
    }

    // Override a control's per-state theme-resource brushes on its OWN ResourceDictionary — the
    // {ThemeResource} the ToggleSwitch template binds resolves the control's local entry before the
    // framework default, so this tints the fill/knob template parts without the global resource seam or a
    // re-template (C#'s SwitchExtensions SetValueForAllKey on these exact keys). is_set false → remove.
    void put_state_brushes(const muxc::Control& control, std::initializer_list<std::wstring_view> keys,
                           bool is_set, maui::graphics::color color)
    {
        auto res = control.Resources();
        const auto brush = is_set ? wnative::to_brush(color) : nullptr;
        for (const auto key : keys)
        {
            const auto boxed = winrt::box_value(winrt::hstring{key});
            if (res.HasKey(boxed))
            {
                res.Remove(boxed);
            }
            if (is_set)
            {
                res.Insert(boxed, brush);
            }
        }
    }

    [[nodiscard]] bool is_set(const maui::core::i_switch& view, std::string_view name)
    {
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        return bindable != nullptr && bindable->is_property_set(name);
    }
} // namespace

namespace maui::core
{
    // Releases the one strong ref pinning the ToggleSwitch (the wnative shape of the pimpl-owned-native
    // doctrine; the apple twin CFReleases its NSSwitch here). The Toggled token is normally revoked in
    // on_disconnect_handler; `container` stays null on this backend (header note).
    switch_platform::~switch_platform()
    {
        wnative::release(native);
    }

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each
    // calls the base body FIRST — the headless mirrors must stay live for the XAML-less cross-platform
    // suite (header note) — then pushes to the real ToggleSwitch when one exists.

    void switch_platform::update_visibility(maui::core::visibility value)
    {
        view_platform_base::update_visibility(value);
        // ViewExtensions.UpdateVisibility (Windows): Hidden rides Opacity 0, Collapsed collapses.
        wnative::apply_visibility(native, value, alpha);
    }

    void switch_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        // ViewExtensions.UpdateOpacity: the Hidden state pins opacity 0 (apply_visibility restores it).
        if (!hidden)
        {
            wnative::apply_opacity(native, value);
        }
    }

    void switch_platform::update_is_enabled(bool value)
    {
        view_platform_base::update_is_enabled(value);
        // ViewExtensions.UpdateIsEnabled → ControlExtensions.UpdateIsEnabled: Control.IsEnabled (the
        // ToggleSwitch IS a Control).
        if (auto toggle = switch_of(*this))
        {
            toggle.IsEnabled(value);
        }
    }

    void switch_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        // ViewExtensions.UpdateAutomationId: AutomationProperties.SetAutomationId.
        wnative::apply_automation_id(native, value);
    }

    std::unique_ptr<switch_platform> switch_handler::create_platform_view()
    {
        auto platform = std::make_unique<switch_platform>();
        try
        {
            // SwitchHandler.Windows.CreatePlatformView: new ToggleSwitch { OffContent = null,
            // OnContent = null } (MAUI does not support the On/Off content labels).
            const muxc::ToggleSwitch toggle;
            toggle.OnContent(nullptr);
            toggle.OffContent(nullptr);
            // MapSwitchMinimumWidth → SwitchExtensions.UpdateMinWidth (the NaN / unset-MinimumWidth
            // branch): MinWidth = 0 — override WinUI's default 154 that reserves label space the
            // hidden OnContent/OffContent never uses. (An explicit view MinimumWidth push rides the
            // generic width pipeline; the OnLoaded spacing-column zeroing is deferred — header.)
            toggle.MinWidth(0.0);
            platform->native = wnative::store(toggle); // released in ~switch_platform
        }
        catch (const winrt::hresult_error&)
        {
            platform->native = nullptr; // XAML-less degradation (header note)
        }
        return platform;
    }

    void switch_handler::on_connect_handler(switch_platform& platform)
    {
        // SwitchHandler.Windows.OnToggled: write the native state back only when it differs (C#:
        // `if (VirtualView is null || PlatformView is null || VirtualView.IsOn == PlatformView.IsOn)
        // return;`) — the guard prevents the virtual→native map from echoing back into a second set.
        // The callback stays wired even XAML-less so the cross-platform suite can drive it (the
        // headless twin's exact body).
        platform.on_value_changed = [this] {
            auto* view = virtual_view();
            auto* platform_view = typed_platform_view();
            if (view != nullptr && platform_view != nullptr && view->is_on() != platform_view->is_on)
            {
                view->set_is_on(platform_view->is_on);
            }
        };
        auto toggle = switch_of(platform);
        if (toggle == nullptr)
        {
            return;
        }
        // ConnectHandler: platformView.Toggled += OnToggled. The native event routes through the
        // platform callback (the peer is the platform struct, whose heap address is stable until
        // disconnect revokes this handler): the lambda mirrors the CONTROL's IsOn into the platform
        // mirror (`is_on` doubles as the native state, the headless convention), then invokes the
        // write-back callback.
        auto* peer = &platform;
        const winrt::event_token token =
            toggle.Toggled([peer](const winrt::Windows::Foundation::IInspectable& sender, const mux::RoutedEventArgs&) {
                if (const auto native_switch = sender.try_as<muxc::ToggleSwitch>())
                {
                    peer->is_on = native_switch.IsOn();
                }
                if (peer->on_value_changed)
                {
                    peer->on_value_changed();
                }
            });
        platform.toggled_token = token.value;
        // deferred: platformView.Loaded += OnLoaded (the SwitchAreaGrid spacing-column zeroing — the
        // template descendant walk; header deviations).
    }

    void switch_handler::on_disconnect_handler(switch_platform& platform)
    {
        // DisconnectHandler: platformView.Toggled -= OnToggled (the Loaded unhook is deferred with the
        // Loaded wiring). The C++ callback is cleared like the headless twin.
        platform.on_value_changed = nullptr;
        if (auto toggle = switch_of(platform))
        {
            if (platform.toggled_token != 0)
            {
                toggle.Toggled(winrt::event_token{platform.toggled_token});
            }
        }
        platform.toggled_token = 0;
    }

    void switch_handler::map_is_on(switch_handler& handler, i_switch& view)
    {
        // C# MapIsOn re-runs the TrackColor mapper first (the headless twin's shape — the effective
        // track color depends on the toggle state; on Windows too, UpdateTrackColor branches on
        // view.IsOn between the On and Off fill keys), then the native state is pushed.
        handler.update_value("track_color");
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->is_on = view.is_on(); // headless mirror first (XAML-less suite)
        // SwitchExtensions.UpdateIsToggled: toggleSwitch.IsOn = view?.IsOn ?? false.
        if (auto toggle = switch_of(*platform))
        {
            toggle.IsOn(view.is_on());
        }
    }

    void switch_handler::map_track_color(switch_handler& handler, i_switch& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->track_color = view.track_color(); // headless mirror (XAML-less suite)
        // SwitchExtensions.UpdateTrackColor: the effective track color (OnColor when toggled on, else
        // OffColor) tints the ToggleSwitchFillOn*/Off* keys. Override them on the switch's own resources
        // (both On and Off so the visible state is right regardless of toggle). Set when on/off color is set.
        if (auto toggle = switch_of(*platform))
        {
            put_state_brushes(toggle,
                              {L"ToggleSwitchFillOn", L"ToggleSwitchFillOnPointerOver",
                               L"ToggleSwitchFillOnPressed", L"ToggleSwitchFillOnDisabled", L"ToggleSwitchFillOff",
                               L"ToggleSwitchFillOffPointerOver", L"ToggleSwitchFillOffPressed",
                               L"ToggleSwitchFillOffDisabled"},
                              is_set(view, "on_color") || is_set(view, "off_color"), view.track_color());
        }
    }

    void switch_handler::map_thumb_color(switch_handler& handler, i_switch& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->thumb_color = view.thumb_color(); // headless mirror (XAML-less suite)
        // SwitchExtensions.UpdateThumbColor: the ToggleSwitchKnobFillOn*/Off* keys (the knob).
        if (auto toggle = switch_of(*platform))
        {
            put_state_brushes(toggle,
                              {L"ToggleSwitchKnobFillOn", L"ToggleSwitchKnobFillOnPointerOver",
                               L"ToggleSwitchKnobFillOnPressed", L"ToggleSwitchKnobFillOnDisabled",
                               L"ToggleSwitchKnobFillOff", L"ToggleSwitchKnobFillOffPointerOver",
                               L"ToggleSwitchKnobFillOffPressed", L"ToggleSwitchKnobFillOffDisabled"},
                              is_set(view, "thumb_color"), view.thumb_color());
        }
    }

    maui::graphics::size switch_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return {0, 0};
        }
        if (platform->native == nullptr)
        {
            // XAML-less degradation: the headless partial's UISwitch-shaped placeholder (51x31), so
            // the backend-agnostic size-request suites see consistent numbers (the android twin's shape).
            return {51.0, 31.0};
        }
        // ViewHandlerExtensions.Windows.cs GetDesiredSizeFromHandler: FrameworkElement.Measure +
        // DesiredSize, with the AdjustForExplicitSize clamp fed from the virtual view's explicit
        // width()/height() (see measure_native).
        const double explicit_width = virtual_view() != nullptr ? virtual_view()->width() : dimension::unset;
        const double explicit_height = virtual_view() != nullptr ? virtual_view()->height() : dimension::unset;
        return wnative::measure_native(platform->native, width_constraint, height_constraint, explicit_width,
                                       explicit_height);
    }

    void switch_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // XAML-less: no native layout to apply
        }
        // The shared Canvas recipe: Canvas.SetLeft/SetTop + explicit Width/Height pin the ToggleSwitch
        // to the frame (ViewHandlerExtensions.Windows.cs PlatformArrangeHandler on the Canvas model).
        wnative::arrange_native(platform->native, frame);
    }
} // namespace maui::core
