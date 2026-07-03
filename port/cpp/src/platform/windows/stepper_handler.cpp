// stepper_handler â€” Windows (WinUI 3) platform partial: the MauiStepper recipe ported STRUCTURALLY.
// C#'s platform view is MauiStepper (Platform/Windows/MauiStepper.cs â€” a templated Control whose
// MauiStepperStyle.xaml template is a two-star-column Grid hosting a "Minus" Button (Content "-") and a
// "Plus" Button (Content "+")); the port has no XAML template infrastructure, so `native` IS that
// template: a Microsoft.UI.Xaml.Controls.Grid with two star columns and the two Buttons as direct
// children (each pinned by its own strong ref on the platform struct). The windows sibling of the
// android twin (a LinearLayout + two Buttons) and of the headless mirror partial
// (src/platform/headless/stepper_handler.cpp).
//
// Ported DIRECTLY from StepperHandler.Windows.cs + Platform/Windows/{MauiStepper.cs (Increment/
// Maximum/Minimum/Value + UpdateEnabled/UpdateValue/OnMinusClicked/OnPlusClicked/OnApplyTemplate's
// button automation ids + ButtonBackground), StepperExtensions.cs (UpdateMinimum/UpdateMaximum/
// UpdateInterval/UpdateValue/UpdateBackground), Styles/MauiStepperStyle.xaml (the Grid template)} +
// ViewExtensions.cs (the generic-IView pushes).
//
// DOCUMENTED DEVIATIONS (each an infrastructure gap of this first cut, not a behavior guess):
//   - MauiStepper is a Control resolved through DefaultStyleKey + MauiStepperStyle.xaml; the port
//     builds the template's exact visual (Grid, two star columns, "-"/"+" Buttons, HorizontalAlignment
//     Left) in code â€” no ControlTemplate seam exists. IsEnabled therefore lands on the two Buttons
//     (C#'s Control.IsEnabled cascade would have disabled the same template children).
//   - MauiStepper's boundary "pseudo-disable" (PseudoDisable/PsuedoEnable â€” VisualState surgery that
//     dims a button WITHOUT IsEnabled, C#'s click-radius-overlap hack) is ported as REAL
//     Button.IsEnabled: the enabled predicate is C#'s exact UpdateEnabled logic (plus dies when
//     value + increment > Maximum, minus when value - increment < Minimum), only the visual mechanism
//     differs â€” the VisualStateManager surgery needs template internals the port does not carry.
//   - The mirrors are authoritative for Minimum/Maximum/Increment/Value (like the android twin): the
//     Grid has no value properties, so MauiStepper's private fields map onto the platform struct's
//     headless mirrors and the buttons are what the native side updates.
//
// XAML-less degradation (the windows twin of the android VM-less fallback): the windows preset also
// runs the cross-platform suite on the host where NO XAML runtime exists; create_platform_view catches
// the construction failure and keeps native/buttons null, while the headless mirrors are ALWAYS
// maintained and on_value_changed/on_minus/on_plus stay invokable C++ callbacks (the cross-platform
// suite drives them) â€” so that suite observes exactly the headless partial's behavior.

#include "maui/core/stepper_handler.hpp"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

#include <winrt/Windows.Foundation.Collections.h> // UIElementCollection Append (IVector)
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h> // ButtonBase Click lives on the projected base
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/base.h>

#include "maui/core/dimension.hpp"
#include "maui/core/flow_direction.hpp"
#include "maui/core/i_stepper.hpp"
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

    // MauiStepper.UpdateEnabled (Windows â€” read, not guessed; NOTE this differs from the android
    // StepperHandlerManager predicate, which is increment-blind):
    //   plus  is usable while value + increment <= Maximum;
    //   minus is usable while value - increment >= Minimum;
    // and BOTH die when the stepper itself is disabled (C#: the MauiStepper Control.IsEnabled cascade;
    // here the buttons carry it â€” header deviations). Re-run on every Increment/Maximum/Minimum/Value
    // write (each MauiStepper property setter calls UpdateEnabled) and on IsEnabled. Scalar form so
    // BOTH the view-driven mappers and update_is_enabled (mirror-only scope) can call it.
    void update_buttons(const maui::core::stepper_platform& platform, bool is_enabled, double value, double increment,
                        double minimum, double maximum)
    {
        if (auto minus = wnative::borrow<muxc::Button>(platform.down_button))
        {
            minus.IsEnabled(is_enabled && value - increment >= minimum);
        }
        if (auto plus = wnative::borrow<muxc::Button>(platform.up_button))
        {
            plus.IsEnabled(is_enabled && value + increment <= maximum);
        }
    }

    // The view-driven overload (the stepper-property mappers): read the live i_stepper.
    void update_buttons(const maui::core::stepper_platform& platform, const maui::core::i_stepper& view)
    {
        update_buttons(platform, view.is_enabled(), view.value(), view.interval(), view.minimum(), view.maximum());
    }

    // MauiStepperStyle.xaml's button factory: a stock Button with the template's Content glyph, placed
    // in its Grid column. Returns an empty Button on construction failure (caller degrades).
    [[nodiscard]] muxc::Button create_stepper_button(const char* text, std::int32_t column)
    {
        const muxc::Button button;
        button.Content(winrt::box_value(wnative::to_hstring_utf8(text)));
        muxc::Grid::SetColumn(button, column);
        return button;
    }
} // namespace

namespace maui::core
{
    // Releases the strong refs pinning the Grid panel AND its two child Buttons (the wnative shape of
    // the pimpl-owned-native doctrine; the android twin deletes its three JNI global refs here). The
    // Click tokens are normally revoked in on_disconnect_handler.
    stepper_platform::~stepper_platform()
    {
        wnative::release(down_button);
        wnative::release(up_button);
        wnative::release(native);
    }

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each
    // calls the base body FIRST â€” the headless mirrors must stay live for the XAML-less cross-platform
    // suite (header note) â€” then pushes to the real panel/buttons when they exist.

    void stepper_platform::update_visibility(maui::core::visibility value)
    {
        view_platform_base::update_visibility(value);
        // ViewExtensions.UpdateVisibility (Windows): Hidden rides Opacity 0, Collapsed collapses.
        wnative::apply_visibility(native, value, alpha);
    }

    void stepper_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        // ViewExtensions.UpdateOpacity: the Hidden state pins opacity 0 (apply_visibility restores it).
        if (!hidden)
        {
            wnative::apply_opacity(native, value);
        }
    }

    void stepper_platform::update_is_enabled(bool value)
    {
        view_platform_base::update_is_enabled(value);
        // ViewExtensions.UpdateIsEnabled â†’ Control.IsEnabled: C#'s MauiStepper is a Control, so the
        // cascade disables its template buttons; the port's Grid is a Panel (no IsEnabled), so the push
        // lands directly on the two Buttons, re-derived through MauiStepper.UpdateEnabled's predicate.
        // Only the headless mirrors are in scope here (IsEnabled never crosses as an i_stepper), and
        // they are the values the four stepper mappers keep current.
        update_buttons(*this, value, this->value, this->increment, this->minimum, this->maximum);
    }

    void stepper_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        // ViewExtensions.UpdateAutomationId on the panel, PLUS MauiStepper.OnApplyTemplate's derived
        // button ids: AutomationProperties.SetAutomationId(_minus, id + "Minus") / (_plus, id + "Plus").
        wnative::apply_automation_id(native, value);
        const std::string base_id{value};
        wnative::apply_automation_id(down_button, base_id + "Minus");
        wnative::apply_automation_id(up_button, base_id + "Plus");
    }

    void stepper_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        // StepperHandler.Windows.MapBackground â†’ StepperExtensions.UpdateBackground: a null background
        // RETURNS (no clear â€” C#'s exact body); a value lands on MauiStepper.ButtonBackground, which
        // UpdateButtonBackground pushes onto BOTH buttons' Background.
        if (value == nullptr)
        {
            return;
        }
        if (const auto* solid = dynamic_cast<const maui::graphics::solid_paint*>(value))
        {
            // Borrow BEFORE constructing the brush: XAML-less (null native/buttons) even the
            // SolidColorBrush activation throws, and this push must degrade to the base mirror like
            // every other windows partial (their bodies null-check the borrowed native first).
            auto minus = wnative::borrow<muxc::Button>(down_button);
            auto plus = wnative::borrow<muxc::Button>(up_button);
            if (minus == nullptr && plus == nullptr)
            {
                return;
            }
            const auto brush = wnative::to_brush(solid->color());
            if (minus != nullptr)
            {
                minus.Background(brush);
            }
            if (plus != nullptr)
            {
                plus.Background(brush);
            }
            return;
        }
        // deferred: gradient / image-source paints (Paint.ToPlatform) â€” the base mirror above keeps
        // the borrow observable.
    }

    std::unique_ptr<stepper_platform> stepper_handler::create_platform_view()
    {
        auto platform = std::make_unique<stepper_platform>();
        try
        {
            // StepperHandler.Windows.CreatePlatformView: new MauiStepper() â€” whose MauiStepperStyle.xaml
            // template is built here in code (header deviations): a Grid, HorizontalAlignment Left, two
            // star ColumnDefinitions, Button "Minus" (Content "-") in column 0 and Button "Plus"
            // (Content "+") in column 1.
            const muxc::Grid panel;
            panel.HorizontalAlignment(mux::HorizontalAlignment::Left);
            // AUTO columns (not the template's Star): the port's Canvas model pins the Grid to the full
            // arranged frame Width, and Star columns would then split that whole width and spread the
            // - / + buttons to opposite ends. Auto columns keep each column at its button's natural width,
            // left-packed and adjacent, so the pair stays grouped on the left exactly as MAUI renders it
            // (MAUI keeps the MauiStepper Grid at its Left-aligned desired width instead — same visual).
            const muxc::ColumnDefinition minus_column;
            minus_column.Width(mux::GridLength{0.0, mux::GridUnitType::Auto});
            panel.ColumnDefinitions().Append(minus_column);
            const muxc::ColumnDefinition plus_column;
            plus_column.Width(mux::GridLength{0.0, mux::GridUnitType::Auto});
            panel.ColumnDefinitions().Append(plus_column);
            const muxc::Button minus = create_stepper_button("-", 0);
            const muxc::Button plus = create_stepper_button("+", 1);
            panel.Children().Append(minus);
            panel.Children().Append(plus);
            // Pinned separately (released in ~stepper_platform): update_buttons + the Click channel
            // address the buttons directly, like the android twin's global refs.
            platform->down_button = wnative::store(minus);
            platform->up_button = wnative::store(plus);
            platform->native = wnative::store(panel);
        }
        catch (const winrt::hresult_error&)
        {
            wnative::release(platform->down_button);
            wnative::release(platform->up_button);
            platform->native = nullptr; // XAML-less degradation (header note)
        }
        return platform;
    }

    void stepper_handler::on_connect_handler(stepper_platform& platform)
    {
        // StepperHandler.Windows.OnValueChanged: write the native value back to the virtual view (the
        // headless twin's exact body; C#'s differ guard collapses in Value's setter path). The
        // callbacks stay wired even XAML-less so the cross-platform suite can drive them.
        platform.on_value_changed = [this] {
            auto* view = virtual_view();
            auto* platform_view = typed_platform_view();
            if (view != nullptr && platform_view != nullptr)
            {
                view->set_value(platform_view->value);
            }
        };
        // MauiStepper.OnMinusClicked / OnPlusClicked â†’ UpdateValue(âˆ“Increment): newValue = Value Â± delta
        // clamped Max(min)-then-Min(max) (C#'s exact order â€” Minimum wins when the range is inverted),
        // then the Value setter runs UpdateEnabled and fires ValueChanged â†’ the write-back above. The
        // mirrors are the native value store (header deviations).
        platform.on_minus = [this] {
            auto* platform_view = typed_platform_view();
            if (platform_view == nullptr)
            {
                return;
            }
            double new_value = platform_view->value - platform_view->increment;
            new_value = std::max(new_value, platform_view->minimum);
            new_value = std::min(new_value, platform_view->maximum);
            platform_view->value = new_value;
            update_buttons(*platform_view, platform_view->enabled, new_value, platform_view->increment,
                           platform_view->minimum, platform_view->maximum);
            if (platform_view->on_value_changed)
            {
                platform_view->on_value_changed();
            }
        };
        platform.on_plus = [this] {
            auto* platform_view = typed_platform_view();
            if (platform_view == nullptr)
            {
                return;
            }
            double new_value = platform_view->value + platform_view->increment;
            new_value = std::max(new_value, platform_view->minimum);
            new_value = std::min(new_value, platform_view->maximum);
            platform_view->value = new_value;
            update_buttons(*platform_view, platform_view->enabled, new_value, platform_view->increment,
                           platform_view->minimum, platform_view->maximum);
            if (platform_view->on_value_changed)
            {
                platform_view->on_value_changed();
            }
        };
        // MauiStepper.OnApplyTemplate: _minus.Click += OnMinusClicked; _plus.Click += OnPlusClicked.
        // The native events route through the platform callbacks (the peer is the platform struct,
        // whose heap address is stable until disconnect revokes these handlers).
        auto* peer = &platform;
        if (auto minus = wnative::borrow<muxc::Button>(platform.down_button))
        {
            const winrt::event_token token =
                minus.Click([peer](const winrt::Windows::Foundation::IInspectable&, const mux::RoutedEventArgs&) {
                    if (peer->on_minus)
                    {
                        peer->on_minus();
                    }
                });
            platform.minus_click_token = token.value;
        }
        if (auto plus = wnative::borrow<muxc::Button>(platform.up_button))
        {
            const winrt::event_token token =
                plus.Click([peer](const winrt::Windows::Foundation::IInspectable&, const mux::RoutedEventArgs&) {
                    if (peer->on_plus)
                    {
                        peer->on_plus();
                    }
                });
            platform.plus_click_token = token.value;
        }
    }

    void stepper_handler::on_disconnect_handler(stepper_platform& platform)
    {
        // DisconnectHandler: ValueChanged -= OnValueChanged (the port's Click tokens are the native
        // half of that channel â€” revoked here). The C++ callbacks are cleared like the headless twin.
        platform.on_value_changed = nullptr;
        platform.on_minus = nullptr;
        platform.on_plus = nullptr;
        if (auto minus = wnative::borrow<muxc::Button>(platform.down_button))
        {
            if (platform.minus_click_token != 0)
            {
                minus.Click(winrt::event_token{platform.minus_click_token});
            }
        }
        if (auto plus = wnative::borrow<muxc::Button>(platform.up_button))
        {
            if (platform.plus_click_token != 0)
            {
                plus.Click(winrt::event_token{platform.plus_click_token});
            }
        }
        platform.minus_click_token = 0;
        platform.plus_click_token = 0;
    }

    void stepper_handler::map_increment(stepper_handler& handler, i_stepper& view)
    {
        // StepperExtensions.UpdateInterval: MauiStepper.Increment = stepper.Interval, whose setter
        // re-runs UpdateEnabled. The mirror keeps the headless twin's positive-only gate (the shared
        // suite's contract); the buttons' enabled state is then re-derived.
        if (auto* platform = handler.typed_platform_view())
        {
            if (view.interval() > 0)
            {
                platform->increment = view.interval(); // headless mirror first (XAML-less suite)
            }
            update_buttons(*platform, view);
        }
    }

    void stepper_handler::map_minimum(stepper_handler& handler, i_stepper& view)
    {
        // StepperExtensions.UpdateMinimum: MauiStepper.Minimum = stepper.Minimum â†’ UpdateEnabled.
        if (auto* platform = handler.typed_platform_view())
        {
            platform->minimum = view.minimum(); // headless mirror first (XAML-less suite)
            update_buttons(*platform, view);
        }
    }

    void stepper_handler::map_maximum(stepper_handler& handler, i_stepper& view)
    {
        // StepperExtensions.UpdateMaximum: MauiStepper.Maximum = stepper.Maximum â†’ UpdateEnabled.
        if (auto* platform = handler.typed_platform_view())
        {
            platform->maximum = view.maximum(); // headless mirror first (XAML-less suite)
            update_buttons(*platform, view);
        }
    }

    void stepper_handler::map_value(stepper_handler& handler, i_stepper& view)
    {
        // StepperExtensions.UpdateValue: MauiStepper.Value = stepper.Value â†’ UpdateEnabled +
        // ValueChanged (whose write-back the differ guard kills â€” the echo dies here). The mirror keeps
        // the headless twin's body (refresh the minimum mirror first so a stale higher minimum does not
        // skew the comparison, then write when the value differs), then the buttons re-derive.
        if (auto* platform = handler.typed_platform_view())
        {
            if (platform->minimum != view.minimum())
            {
                platform->minimum = view.minimum();
            }
            if (platform->value != view.value())
            {
                platform->value = view.value();
            }
            update_buttons(*platform, view);
        }
    }

    void stepper_handler::map_flow_direction(stepper_handler& handler, i_stepper& view)
    {
        // StepperHandler.MapFlowDirection's base part: record the RESOLVED direction (the MatchParent â†’
        // parent-IView fallback) and push it as the generic Windows recipe (ViewExtensions
        // .UpdateFlowDirection): LeftToRight/RightToLeft set FrameworkElement.FlowDirection, an
        // unresolved MatchParent ClearValues back to the inherited default. (The iOS-26 subview
        // re-application is a UIKit concern; XAML FlowDirection inherits to the buttons natively.)
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const maui::core::flow_direction resolved = resolved_flow_direction(view);
        platform->resolved_flow_direction = resolved; // headless mirror first (XAML-less suite)
        auto panel = wnative::borrow_as<mux::FrameworkElement>(platform->native);
        if (panel == nullptr)
        {
            return;
        }
        switch (resolved)
        {
            case maui::core::flow_direction::left_to_right:
                panel.FlowDirection(mux::FlowDirection::LeftToRight);
                break;
            case maui::core::flow_direction::right_to_left:
                panel.FlowDirection(mux::FlowDirection::RightToLeft);
                break;
            case maui::core::flow_direction::match_parent:
                panel.ClearValue(mux::FrameworkElement::FlowDirectionProperty());
                break;
        }
    }

    maui::graphics::size stepper_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return {0, 0};
        }
        if (platform->native == nullptr)
        {
            // XAML-less degradation: the headless partial's UIStepper-shaped placeholder (94x32), so
            // the backend-agnostic size-request suites see consistent numbers (the android twin's shape).
            return {94.0, 32.0};
        }
        // ViewHandlerExtensions.Windows.cs GetDesiredSizeFromHandler: the Grid measures its two Buttons
        // (star columns behave as Auto under an unconstrained measure â€” the buttons keep their natural
        // widths, like the template's Left-aligned Grid), with the AdjustForExplicitSize clamp fed from
        // the virtual view's explicit width()/height() (see measure_native).
        const double explicit_width = virtual_view() != nullptr ? virtual_view()->width() : dimension::unset;
        const double explicit_height = virtual_view() != nullptr ? virtual_view()->height() : dimension::unset;
        return wnative::measure_native(platform->native, width_constraint, height_constraint, explicit_width,
                                       explicit_height);
    }

    void stepper_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // XAML-less: no native layout to apply
        }
        // The shared Canvas recipe: Canvas.SetLeft/SetTop + explicit Width/Height pin the Grid to the
        // frame; the two star columns split it between the buttons (MauiStepperStyle.xaml's template
        // geometry under ViewHandlerExtensions.Windows.cs PlatformArrangeHandler).
        wnative::arrange_native(platform->native, frame);
    }
} // namespace maui::core
