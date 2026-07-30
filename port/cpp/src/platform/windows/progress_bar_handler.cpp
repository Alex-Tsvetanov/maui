// progress_bar_handler — WinUI 3 platform recipe: a real Microsoft.UI.Xaml.Controls.ProgressBar, the
// same native type ProgressBarHandler.Windows.cs creates (`ViewHandler<IProgress, ProgressBar>` — a
// plain RangeBase-derived control, no MauiProgressBar subclass; unlike Slider/Picker/DatePicker there is
// no XAML-template swap or Windows-only Mapper remap here at all). Ported from
// ProgressBarHandler.Windows.cs (cross-platform mapper in ProgressBarHandler.cs) + Platform/Windows/
// ProgressBarExtensions.cs.
//
// TWO THINGS THE BRIEF FOR THIS PORT GUESSED AT THAT THE ORACLE ANSWERS DIRECTLY (read ProgressBarExtensions.cs
// in full — it is 19 lines, no resource-key machinery at all):
//
//  1. Progress VALUE RANGE: CreatePlatformView is exactly `new() { Minimum = 0, Maximum = 1 }` — Progress
//     (an IProgress double in 0..1) maps DIRECTLY onto RangeBase.Value with NO scaling to 0..100. Ported
//     as-is below (bar.Minimum(0); bar.Maximum(1);).
//  2. ProgressColor: UpdateProgressColor is a BARE Foreground push — `if (progressColor != null)
//     platformProgressBar.Foreground = progressColor.ToPlatform();` — NOT the per-visual-state
//     k_*_keys/set_resources/refresh_theme_resources recipe every other Windows handler here
//     (slider/picker/date_picker/check_box/radio_button) needs for ITS colors. There is no "else" branch
//     either: an unset/cleared ProgressColor leaves whatever Foreground is already there (the control's
//     own theme default), it is never explicitly cleared back. This port mirrors that exactly — is_set()
//     below gates the push, but there is deliberately no companion ClearValue/remove_resources call.
//     (Presumably ProgressBar's default control template binds its fill Brush straight to Foreground,
//     unlike Slider's four separate per-visual-state track/thumb brushes — but that is inference, not
//     something this file needs to rely on; the oracle's simplicity is copied either way.)
//
// FLOW DIRECTION: ProgressBarHandler.cs's Mapper only remaps `[nameof(IView.FlowDirection)] =
// MapFlowDirection` under `#if __IOS__ || MACCATALYST` — real MAUI has NO Windows FlowDirection remap
// for ProgressBar. This port's SHARED cross-platform mapper table (src/core/progress_bar_handler.cpp)
// wires the "flow_direction" key to map_flow_direction UNCONDITIONALLY for every backend, though, so this
// backend must still define the symbol (a link-time requirement, not a behavior requirement) — it mirrors
// resolved_flow_direction() into the observable field only, matching headless, since no Windows
// *_platform in this backend has a real update_flow_direction native push yet (view_platform_base's
// no-op mirror body is the current state for EVERY Windows handler, not a gap specific to this control).
//
// ARRANGE WIDTH: no Windows-only Width/Height Mapper remap exists for ProgressBar either (unlike
// ActivityIndicator's documented shrink-wrap pair), so real MAUI uses the generic MapWidth/MapHeight
// (a plain propagate-through of the — usually NaN — WidthRequest) and relies on ITS real layout panel to
// stretch the bar across its row. A horizontal ProgressBar is stretch-by-default there (the same
// Control-base default as Slider/ComboBox/TextBox/Button — a fixed-circle ProgressRing is the outlier,
// not the rule), so this file follows the slider_handler.cpp / image_button_handler.cpp ARRANGE/
// EXPLICIT-SIZE FIX pattern (pin Width/Height to the explicit request, widened at measure time) rather
// than activity_indicator_handler.cpp's shrink-wrap-only-when-explicit pattern.

#include "maui/core/progress_bar_handler.hpp"

#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <string_view>

#include "maui/core/bindable_object.hpp"
#include "maui/core/i_progress.hpp"
#include "maui/core/view_chrome_ops.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "winui_interop.hpp"
#include "winui_visual_ops.hpp"

namespace
{
    // Named `winui`, NOT `xaml` — see slider_handler.cpp's / picker_handler.cpp's identical note (the
    // port's own maui::xaml XAML-loader namespace would shadow a file-scope `xaml` alias inside
    // namespace maui::*).
    namespace winui = winrt::Microsoft::UI::Xaml;
    using progress_bar_control = winui::Controls::ProgressBar;

    progress_bar_control as_progress_bar(void* native)
    {
        return maui::platform::windows::ref<winui::UIElement>(native).as<progress_bar_control>();
    }

    // "Was this property explicitly set?" — see slider_handler.cpp/activity_indicator_handler.cpp for why
    // this must not be a value comparison ([[cpp-unset-color-sentinel-collision]]): the port's Color is a
    // non-nullable value type whose default-constructed value is opaque BLACK, so it cannot stand in for
    // C#'s `progressColor != null`.
    bool is_set(const maui::core::i_view& view, std::string_view property)
    {
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        return bindable != nullptr && bindable->is_property_set(property);
    }
} // namespace

namespace maui::core
{
    progress_bar_platform::~progress_bar_platform()
    {
        if (native != nullptr)
        {
            maui::platform::windows::drop<winui::UIElement>(native);
        }
    }

    std::unique_ptr<progress_bar_platform> progress_bar_handler::create_platform_view()
    {
        auto platform = std::make_unique<progress_bar_platform>();
        // ProgressBarHandler.Windows.cs: `new() { Minimum = 0, Maximum = 1 }`.
        progress_bar_control bar;
        bar.Minimum(0);
        bar.Maximum(1);
        platform->native = maui::platform::windows::take<winui::UIElement>(bar);
        return platform;
    }

    void progress_bar_handler::map_progress(progress_bar_handler& handler, i_progress& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->progress = view.progress();
        // ProgressBarExtensions.UpdateProgress: `Value = progress.Progress` — direct, no scaling.
        as_progress_bar(platform->native).Value(platform->progress);
    }

    void progress_bar_handler::map_progress_color(progress_bar_handler& handler, i_progress& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->progress_color = view.progress_color();
        // ProgressBarExtensions.UpdateProgressColor: `if (progressColor != null) Foreground = ...` — no
        // clearing branch at all when unset (see file-top note 2), unlike Slider/DatePicker's resource-key
        // overrides. An unset ProgressColor leaves whatever Foreground is already there.
        if (!is_set(view, "progress_color"))
        {
            return;
        }
        const progress_bar_control bar = as_progress_bar(platform->native);
        bar.Foreground(winui::Media::SolidColorBrush{maui::platform::windows::to_ui_color(platform->progress_color)});
    }

    void progress_bar_handler::map_flow_direction(progress_bar_handler& handler, i_progress& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            // See file-top FLOW DIRECTION note: real MAUI has no Windows remap for this at all; this
            // backend still must define the symbol because the port's shared mapper table wires it
            // unconditionally. Observable-mirror only, matching headless — no native push exists for any
            // Windows handler's flow direction yet.
            platform->resolved_flow_direction = resolved_flow_direction(view);
        }
    }

    maui::graphics::size progress_bar_handler::get_desired_size(double width_constraint, double height_constraint) const
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
        const progress_bar_control bar = as_progress_bar(platform->native);
        // ARRANGE/EXPLICIT-SIZE FIX (see file-top ARRANGE WIDTH note + slider_handler.cpp's identical
        // block + image_button_handler.cpp, commit a2444f94ba): pin Width/Height to the view's own
        // explicit request instead of clearing to NaN unconditionally, then only WIDEN the incoming
        // constraint at measure time — ViewHandlerExtensions.Windows.cs:56-74 GetDesiredSizeFromHandler +
        // :91-105 AdjustForExplicitSize. platform_arrange's OWN stamp (below) is UNTOUCHED.
        const auto* view = virtual_view();
        const double explicit_width = (view != nullptr) ? view->width() : std::numeric_limits<double>::quiet_NaN();
        const double explicit_height = (view != nullptr) ? view->height() : std::numeric_limits<double>::quiet_NaN();
        bar.Width(explicit_width);
        bar.Height(explicit_height);
        const double adjusted_width_constraint =
            std::isnan(explicit_width) ? width_constraint : std::max(width_constraint, explicit_width);
        const double adjusted_height_constraint =
            std::isnan(explicit_height) ? height_constraint : std::max(height_constraint, explicit_height);
        bar.Measure(
            winrt::Windows::Foundation::Size{maui::platform::windows::measure_constraint(adjusted_width_constraint),
                                             maui::platform::windows::measure_constraint(adjusted_height_constraint)});
        const auto desired = bar.DesiredSize();
        return {desired.Width, desired.Height};
    }

    void progress_bar_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        // PlatformArrangeHandler's guard, WIDENED to non-finite — see button_handler.cpp/slider_handler.cpp's
        // platform_arrange for why (an unrecoverable stowed exception, 0xC000027B, otherwise).
        if (!std::isfinite(frame.x) || !std::isfinite(frame.y) || !std::isfinite(frame.width) ||
            !std::isfinite(frame.height) || frame.width < 0 || frame.height < 0)
        {
            return;
        }
        const progress_bar_control bar = as_progress_bar(platform->native);
        winui::Controls::Canvas::SetLeft(bar, frame.x);
        winui::Controls::Canvas::SetTop(bar, frame.y);
        bar.Width(frame.width);
        bar.Height(frame.height);
        // Clip is bounds-dependent (view_chrome_ops.cpp's apply_native_clip reads the just-set Width/
        // Height back); map_clip's own push (view_mapper.cpp) always runs before the first arrange, so
        // this re-invoke is what actually installs the clip once the bar has a real size.
        if (const auto* view = virtual_view(); view != nullptr)
        {
            apply_native_clip(platform->native, view->clip());
        }
    }

    // ---- generic-IView property pushes (view_platform_base overrides) ---------------------------
    // Delegated to the shared winui_visual_ops free functions, same as slider/activity_indicator; see
    // that header for why they are free functions taking the void* slot. ProgressBar IS a Control
    // (RangeBase : Control), so IsEnabled/Background reach it directly with no wrapper needed. No
    // Windows-only Background remap exists for ProgressBar (unlike Slider's — see SliderHandler.cs's
    // `#if WINDOWS` note in slider_handler.cpp), so this is the plain generic push.
    void progress_bar_platform::update_visibility(maui::core::visibility value)
    {
        maui::platform::windows::apply_visibility(native, value);
    }

    void progress_bar_platform::update_opacity(double value)
    {
        maui::platform::windows::apply_opacity(native, value);
    }

    void progress_bar_platform::update_is_enabled(bool value)
    {
        maui::platform::windows::apply_is_enabled(native, value);
    }

    void progress_bar_platform::update_automation_id(std::string_view value)
    {
        maui::platform::windows::apply_automation_id(native, value);
    }

    void progress_bar_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::windows::apply_background(native, value);
    }
} // namespace maui::core
