// check_box_handler â€” Windows (WinUI 3) platform partial: a REAL Microsoft.UI.Xaml.Controls.CheckBox.
// The windows twin of src/platform/apple/check_box_handler.mm (NSButton checkbox) / the android JNI
// partial, and the real-native sibling of the headless mirror partial
// (src/platform/headless/check_box_handler.cpp). IsChecked maps onto CheckBox.IsChecked, and the native
// Checked/Unchecked events flow back through on_checked_changed into i_check_box::send_is_checked.
//
// Ported DIRECTLY from CheckBoxHandler.Windows.cs (CreatePlatformView's AdjustCheckBoxForNoText +
// ConnectHandler/DisconnectHandler/OnChecked) + Platform/Windows/CheckBoxExtensions.cs
// (UpdateIsChecked/UpdateForeground) + ViewExtensions.cs (the generic-IView pushes).
//
// DOCUMENTED DEVIATIONS (each an infrastructure gap of this first cut, not a behavior guess):
//   - UpdateForeground's per-state resource keys (the sixteen CheckBoxCheckBackgroundFillChecked* /
//     CheckBoxCheckBackgroundStroke* keys + RefreshThemeResources) are deferred with the port's
//     resource-dictionary seam: the port pushes the DIRECT Foreground dependency property (the glyph/
//     text brush half of C#'s body â€” `platformCheckBox.Foreground = tintBrush`), and the null branch's
//     `Foreground = null` + RemoveKeys maps to ClearValue (restoring the theme default), exactly like
//     the button partial's color maps.
//   - AdjustCheckBoxForNoText's Loaded-time root-grid margin fix-up ((CheckBoxHeight - CheckBoxSize)/2,
//     read from Application.Current.Resources) is deferred: it needs a Loaded hook + the application
//     resource lookup. The synchronous half IS ported: MinWidth/MinHeight = 0 and Padding = 0.
//   - C#'s PreventGestureBubbling => true is a Controls-layer gesture-recognizer knob with no port
//     surface yet.
//
// XAML-less degradation (the windows twin of the android VM-less fallback): the windows preset also
// runs the cross-platform suite on the host where NO XAML runtime exists; create_platform_view catches
// the construction failure and keeps native null, while the headless mirrors are ALWAYS maintained and
// on_checked_changed stays an invokable C++ callback (the cross-platform suite drives it).

#include "maui/core/check_box_handler.hpp"

#include <memory>
#include <string_view>

#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h> // ToggleButton/RangeBase: the projected base carries IsChecked/Minimum/Maximum/Value
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h> // ResourceDictionary IMap Insert
#include <winrt/Windows.Foundation.h>
#include <winrt/base.h>

#include <initializer_list>

#include "maui/core/dimension.hpp"
#include "maui/core/i_check_box.hpp"
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
    namespace wnative = maui::platform::win;

    // CheckBoxExtensions._tintColorResourceKeys: the sixteen per-state fill/stroke brushes the CheckBox
    // template binds via {ThemeResource}. Overriding them on the control's OWN resources tints the box
    // (checked fill + unchecked/checked/indeterminate stroke) without the global resource seam — C#'s
    // SetValueForAllKey / RemoveKeys, minus RefreshThemeResources (a local entry resolves at template apply).
    constexpr std::wstring_view k_tint_keys[] = {
        L"CheckBoxCheckBackgroundFillChecked", L"CheckBoxCheckBackgroundFillCheckedPointerOver",
        L"CheckBoxCheckBackgroundFillCheckedPressed", L"CheckBoxCheckBackgroundFillCheckedDisabled",
        L"CheckBoxCheckBackgroundStrokeUnchecked", L"CheckBoxCheckBackgroundStrokeUncheckedPointerOver",
        L"CheckBoxCheckBackgroundStrokeUncheckedPressed", L"CheckBoxCheckBackgroundStrokeUncheckedDisabled",
        L"CheckBoxCheckBackgroundStrokeChecked", L"CheckBoxCheckBackgroundStrokeCheckedPointerOver",
        L"CheckBoxCheckBackgroundStrokeCheckedPressed", L"CheckBoxCheckBackgroundStrokeCheckedDisabled",
        L"CheckBoxCheckBackgroundStrokeIndeterminate", L"CheckBoxCheckBackgroundStrokeIndeterminatePointerOver",
        L"CheckBoxCheckBackgroundStrokeIndeterminatePressed", L"CheckBoxCheckBackgroundStrokeIndeterminateDisabled"};

    // Set (or, when brush is null, remove) the tint keys on the control's own ResourceDictionary.
    void put_tint_brushes(const muxc::Control& control, const winrt::Microsoft::UI::Xaml::Media::Brush& brush)
    {
        auto res = control.Resources();
        for (const auto key : k_tint_keys)
        {
            const auto boxed = winrt::box_value(winrt::hstring{key});
            if (res.HasKey(boxed))
            {
                res.Remove(boxed);
            }
            if (brush != nullptr)
            {
                res.Insert(boxed, brush);
            }
        }
    }

    [[nodiscard]] muxc::CheckBox check_box_of(const maui::core::check_box_platform& platform)
    {
        return wnative::borrow<muxc::CheckBox>(platform.native);
    }
} // namespace

namespace maui::core
{
    // Releases the one strong ref pinning the CheckBox (the wnative shape of the pimpl-owned-native
    // doctrine; the apple twin CFReleases its NSButton here). The Checked/Unchecked tokens are normally
    // revoked in on_disconnect_handler.
    check_box_platform::~check_box_platform()
    {
        wnative::release(native);
    }

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each
    // calls the base body FIRST â€” the headless mirrors must stay live for the XAML-less cross-platform
    // suite (header note) â€” then pushes to the real CheckBox when one exists.

    void check_box_platform::update_visibility(maui::core::visibility value)
    {
        view_platform_base::update_visibility(value);
        // ViewExtensions.UpdateVisibility (Windows): Hidden rides Opacity 0, Collapsed collapses.
        wnative::apply_visibility(native, value, alpha);
    }

    void check_box_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        // ViewExtensions.UpdateOpacity: the Hidden state pins opacity 0 (apply_visibility restores it).
        if (!hidden)
        {
            wnative::apply_opacity(native, value);
        }
    }

    void check_box_platform::update_is_enabled(bool value)
    {
        view_platform_base::update_is_enabled(value);
        // ViewExtensions.UpdateIsEnabled â†’ ControlExtensions.UpdateIsEnabled: Control.IsEnabled (the
        // CheckBox IS a Control).
        if (auto check_box = check_box_of(*this))
        {
            check_box.IsEnabled(value);
        }
    }

    void check_box_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        // ViewExtensions.UpdateAutomationId: AutomationProperties.SetAutomationId.
        wnative::apply_automation_id(native, value);
    }

    std::unique_ptr<check_box_platform> check_box_handler::create_platform_view()
    {
        auto platform = std::make_unique<check_box_platform>();
        try
        {
            // CheckBoxHandler.Windows.CreatePlatformView: new CheckBox() +
            // AdjustCheckBoxForNoText(checkBox): MinWidth = 0, MinHeight = 0, Padding = 0 (the control
            // never shows content text, so the label reservations are dropped). The Loaded-time root-grid
            // margin centering is deferred (header deviations).
            const muxc::CheckBox check_box;
            check_box.MinWidth(0.0);
            check_box.MinHeight(0.0);
            check_box.Padding(mux::Thickness{0.0, 0.0, 0.0, 0.0});
            platform->native = wnative::store(check_box); // released in ~check_box_platform
        }
        catch (const winrt::hresult_error&)
        {
            platform->native = nullptr; // XAML-less degradation (header note)
        }
        return platform;
    }

    void check_box_handler::on_connect_handler(check_box_platform& platform)
    {
        // CheckBoxHandler.Windows.OnChecked: `VirtualView.IsChecked = platformView.IsChecked == true`.
        // The differ guard mirrors the headless twin (the write-back is skipped when nothing changed â€”
        // the virtualâ†’native map's echo dies here). The callback stays wired even XAML-less so the
        // cross-platform suite can drive it.
        platform.on_checked_changed = [this] {
            auto* view = virtual_view();
            auto* platform_view = typed_platform_view();
            if (view != nullptr && platform_view != nullptr && view->is_checked() != platform_view->is_checked)
            {
                view->send_is_checked(platform_view->is_checked);
            }
        };
        auto check_box = check_box_of(platform);
        if (check_box == nullptr)
        {
            return;
        }
        // ConnectHandler: platformView.Checked += OnChecked; platformView.Unchecked += OnChecked â€”
        // BOTH events share one body. The lambda mirrors the CONTROL's nullable IsChecked into the
        // platform mirror with C#'s `== true` collapse (null/indeterminate â†’ false), then invokes the
        // write-back callback. The peer is the platform struct (stable until disconnect revokes these).
        auto* peer = &platform;
        const auto on_native_checked = [peer](const winrt::Windows::Foundation::IInspectable& sender,
                                              const mux::RoutedEventArgs&) {
            if (const auto native_box = sender.try_as<muxc::CheckBox>())
            {
                const auto state = native_box.IsChecked(); // IReference<bool> â€” C#'s bool?
                peer->is_checked = state != nullptr && state.Value();
            }
            if (peer->on_checked_changed)
            {
                peer->on_checked_changed();
            }
        };
        platform.checked_token = check_box.Checked(on_native_checked).value;
        platform.unchecked_token = check_box.Unchecked(on_native_checked).value;
    }

    void check_box_handler::on_disconnect_handler(check_box_platform& platform)
    {
        // DisconnectHandler: Checked -= OnChecked; Unchecked -= OnChecked. The C++ callback is cleared
        // like the headless twin.
        platform.on_checked_changed = nullptr;
        if (auto check_box = check_box_of(platform))
        {
            if (platform.checked_token != 0)
            {
                check_box.Checked(winrt::event_token{platform.checked_token});
            }
            if (platform.unchecked_token != 0)
            {
                check_box.Unchecked(winrt::event_token{platform.unchecked_token});
            }
        }
        platform.checked_token = 0;
        platform.unchecked_token = 0;
    }

    void check_box_handler::map_is_checked(check_box_handler& handler, i_check_box& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->is_checked = view.is_checked(); // headless mirror first (XAML-less suite)
        // CheckBoxExtensions.UpdateIsChecked: platformCheckBox.IsChecked = check.IsChecked (the
        // nullable IsChecked takes the plain bool boxed as IReference<bool>).
        if (auto check_box = check_box_of(*platform))
        {
            check_box.IsChecked(winrt::Windows::Foundation::IReference<bool>{view.is_checked()});
        }
    }

    void check_box_handler::map_foreground(check_box_handler& handler, i_check_box& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->foreground = view.foreground(); // headless mirror first (a NON-owning borrow)
        auto check_box = check_box_of(*platform);
        if (check_box == nullptr)
        {
            return;
        }
        // CheckBoxExtensions.UpdateForeground: tintBrush = check.Foreground?.ToPlatform(); null â†’
        // RemoveKeys(_tintColorResourceKeys) + Foreground = null; value â†’ SetValueForAllKey +
        // Foreground = tintBrush; then RefreshThemeResources. The port pushes the DIRECT Foreground
        // (deferred: the sixteen per-state CheckBoxCheckBackground* resource keys â€” header), with the
        // null branch mapped to ClearValue (the theme default restore).
        const auto* foreground = view.foreground();
        if (foreground == nullptr)
        {
            put_tint_brushes(check_box, nullptr); // RemoveKeys(_tintColorResourceKeys)
            check_box.ClearValue(muxc::Control::ForegroundProperty());
            return;
        }
        if (const auto* solid = dynamic_cast<const maui::graphics::solid_paint*>(foreground))
        {
            const auto brush = wnative::to_brush(solid->color());
            put_tint_brushes(check_box, brush); // SetValueForAllKey → the box's checked fill + strokes tint
            check_box.Foreground(brush);         // C#'s direct Foreground = tintBrush (the glyph/text half)
            return;
        }
        // deferred: gradient foreground paints (Paint.ToPlatform) â€” the mirror above keeps the borrow
        // observable.
    }

    maui::graphics::size check_box_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return {0, 0};
        }
        if (platform->native == nullptr)
        {
            // XAML-less degradation: the headless partial's placeholder square (the iOS MinimumSize
            // 44x44), so the backend-agnostic size-request suites see consistent numbers. (C#'s Windows
            // handler has no GetDesiredSize override â€” the native measure below is the whole recipe.)
            return {44.0, 44.0};
        }
        // ViewHandlerExtensions.Windows.cs GetDesiredSizeFromHandler: FrameworkElement.Measure +
        // DesiredSize, with the AdjustForExplicitSize clamp fed from the virtual view's explicit
        // width()/height() (see measure_native).
        const double explicit_width = virtual_view() != nullptr ? virtual_view()->width() : dimension::unset;
        const double explicit_height = virtual_view() != nullptr ? virtual_view()->height() : dimension::unset;
        return wnative::measure_native(platform->native, width_constraint, height_constraint, explicit_width,
                                       explicit_height);
    }

    void check_box_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // XAML-less: no native layout to apply
        }
        // The shared Canvas recipe: Canvas.SetLeft/SetTop + explicit Width/Height pin the CheckBox to
        // the frame (ViewHandlerExtensions.Windows.cs PlatformArrangeHandler on the Canvas model).
        wnative::arrange_native(platform->native, frame);
    }
} // namespace maui::core
