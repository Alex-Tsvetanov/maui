#pragma once
// windows_native.hpp — shared helpers for the Windows (WinUI 3) platform partials.
//
// Ownership (PROFILE §8, the CFRetain/CFRelease twin): a platform struct's `void* native` slot holds
// exactly ONE strong WinRT reference, stored by detaching the ABI pointer. borrow<T>() re-wraps it as
// a live projection object (copy_from_abi AddRefs — the borrow is an independent strong ref that drops
// at scope exit), and release() reattaches + drops the stored ref in the platform dtor. No winrt type
// ever appears in a shared header — `native` stays void* there; these helpers localize the ABI dance
// to the windows/*.cpp TUs.
//
// Conversions: the small maui::graphics -> WinUI value bridges every handler needs (color, thickness,
// corner radius, UTF-8 text). Mirrors src/platform/apple/apple_conversions.hpp in role.

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>

#include <winrt/Microsoft.UI.Xaml.Automation.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.h>
#include <winrt/base.h>

#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::platform::win
{
    // ---- ownership -------------------------------------------------------------------------------
    // Store one strong ref into a platform struct's void* native slot (replacing nothing — call
    // release() first when re-storing).
    template <class T> void* store(T const& object)
    {
        T strong = object;               // AddRef — the copy is the slot's own strong ref…
        return winrt::detach_abi(strong); // …stolen into the raw slot (detach_abi nulls the source)
    }

    // Borrow the stored object back as a live projection value (AddRefs; safe to copy/hold locally).
    template <class T> [[nodiscard]] T borrow(void* native)
    {
        T object{nullptr};
        if (native != nullptr)
        {
            winrt::copy_from_abi(object, native);
        }
        return object;
    }

    // Drop the stored strong ref (idempotent; nulls the slot).
    inline void release(void*& native)
    {
        if (native != nullptr)
        {
            winrt::Windows::Foundation::IUnknown object{nullptr};
            winrt::attach_abi(object, native); // adopts the stored ref; dropped at scope exit
            native = nullptr;
        }
    }

    // Borrow the stored object as a DIFFERENT projected type than it was stored as. copy_from_abi alone
    // must only re-wrap the exact stored default interface (it does no QueryInterface), so any cross-type
    // access goes IInspectable (every WinRT interface derives it) → try_as<T>. Used to reach ANOTHER
    // handler's native (a child stored as TextBlock/Button/Canvas borrowed as FrameworkElement/UIElement).
    // Empty on a null slot or a failed QI.
    template <class T> [[nodiscard]] T borrow_as(void* native)
    {
        winrt::Windows::Foundation::IInspectable object{nullptr};
        if (native != nullptr)
        {
            winrt::copy_from_abi(object, native);
        }
        return object != nullptr ? object.try_as<T>() : T{nullptr};
    }

    // ---- conversions -----------------------------------------------------------------------------
    [[nodiscard]] inline winrt::Windows::UI::Color to_ui_color(const maui::graphics::color& value)
    {
        return winrt::Windows::UI::Color{
            .A = static_cast<uint8_t>(value.alpha * 255.0F + 0.5F),
            .R = static_cast<uint8_t>(value.red * 255.0F + 0.5F),
            .G = static_cast<uint8_t>(value.green * 255.0F + 0.5F),
            .B = static_cast<uint8_t>(value.blue * 255.0F + 0.5F),
        };
    }

    [[nodiscard]] inline winrt::Microsoft::UI::Xaml::Media::SolidColorBrush to_brush(
        const maui::graphics::color& value)
    {
        return winrt::Microsoft::UI::Xaml::Media::SolidColorBrush{to_ui_color(value)};
    }

    [[nodiscard]] inline winrt::Microsoft::UI::Xaml::Thickness to_thickness(const maui::core::thickness& value)
    {
        return winrt::Microsoft::UI::Xaml::Thickness{
            .Left = value.left, .Top = value.top, .Right = value.right, .Bottom = value.bottom};
    }

    [[nodiscard]] inline winrt::hstring to_hstring_utf8(std::string_view value)
    {
        return winrt::to_hstring(value); // interprets the input as UTF-8
    }

    [[nodiscard]] inline std::string to_utf8(winrt::hstring const& value)
    {
        return winrt::to_string(value);
    }

    // CharacterSpacingExtensions.ToEm: Convert.ToInt32(pt * 0.0624f * 1000) — Pt → 1/1000 em, the unit
    // TextBlock.CharacterSpacing / Control.CharacterSpacing speak.
    [[nodiscard]] inline std::int32_t to_em(double pt)
    {
        return static_cast<std::int32_t>(std::lround(pt * static_cast<double>(0.0624F) * 1000.0));
    }

    // ---- measure / arrange (the shared per-view seam bodies) --------------------------------------
    // ViewHandlerExtensions.Windows.cs GetDesiredSizeFromHandler: negative constraints → Size.Zero;
    // AdjustForExplicitSize widens each constraint to max(constraint, explicit). C# reads the explicit
    // size back off platformView.Width/Height (which MapWidth/MapHeight had pushed from the virtual
    // view); the port's platform_arrange writes the ARRANGED FRAME into those slots (the Canvas layout
    // model below), so the caller passes the virtual view's explicit width()/height() instead — the
    // same value C#'s MapWidth would have pushed (NaN = unset). The arranged-frame Width/Height are
    // reset to Auto before measuring so the previous arrange pass cannot pin DesiredSize; the arrange
    // that follows in the same synchronous layout pass re-writes them.
    [[nodiscard]] inline maui::graphics::size measure_native(void* native, double width_constraint,
                                                             double height_constraint, double explicit_width,
                                                             double explicit_height)
    {
        auto element = borrow_as<winrt::Microsoft::UI::Xaml::FrameworkElement>(native);
        if (element == nullptr || width_constraint < 0 || height_constraint < 0)
        {
            return {0, 0};
        }
        if (!std::isnan(explicit_width))
        {
            width_constraint = (std::max)(width_constraint, explicit_width); // (parens: windows.h max macro)
        }
        if (!std::isnan(explicit_height))
        {
            height_constraint = (std::max)(height_constraint, explicit_height);
        }
        element.Width(std::numeric_limits<double>::quiet_NaN());  // Auto — drop the arranged-frame pin
        element.Height(std::numeric_limits<double>::quiet_NaN());
        const auto to_measure = [](double constraint) {
            // An unconstrained (infinite / NaN) double becomes the float infinity XAML's Measure expects.
            return std::isfinite(constraint) ? static_cast<float>(constraint)
                                             : std::numeric_limits<float>::infinity();
        };
        element.Measure(
            winrt::Windows::Foundation::Size{to_measure(width_constraint), to_measure(height_constraint)});
        const winrt::Windows::Foundation::Size desired = element.DesiredSize();
        return {static_cast<double>(desired.Width), static_cast<double>(desired.Height)};
    }

    // ViewHandler.PlatformArrange on the Canvas layout model (the apple flipped_container twin): the
    // C++ side owns measure/arrange and children are absolutely positioned — Canvas.SetLeft/SetTop
    // place the element and the explicit Width/Height pin it to the frame (a Canvas arranges each
    // child at its desired size, which the explicit size forces to the frame). Mirrors
    // PlatformArrangeHandler's negative-size guard.
    inline void arrange_native(void* native, const maui::graphics::rect& frame)
    {
        auto element = borrow_as<winrt::Microsoft::UI::Xaml::FrameworkElement>(native);
        if (element == nullptr || frame.width < 0 || frame.height < 0)
        {
            return;
        }
        winrt::Microsoft::UI::Xaml::Controls::Canvas::SetLeft(element, frame.x);
        winrt::Microsoft::UI::Xaml::Controls::Canvas::SetTop(element, frame.y);
        element.Width(frame.width);
        element.Height(frame.height);
    }

    // ---- generic-IView pushes (Platform/Windows/ViewExtensions.cs) --------------------------------
    // UpdateVisibility: Visible → restore Opacity + Visible; Hidden → Opacity 0 + Visible (WinUI has no
    // "invisible but still occupies space" state, so C# fakes Hidden through opacity); Collapsed →
    // Collapsed. `mirrored_opacity` is the platform struct's alpha mirror (C# re-reads view.Opacity).
    inline void apply_visibility(void* native, maui::core::visibility value, double mirrored_opacity)
    {
        auto element = borrow_as<winrt::Microsoft::UI::Xaml::FrameworkElement>(native);
        if (element == nullptr)
        {
            return;
        }
        switch (value)
        {
            case maui::core::visibility::visible:
                element.Opacity(mirrored_opacity);
                element.Visibility(winrt::Microsoft::UI::Xaml::Visibility::Visible);
                break;
            case maui::core::visibility::hidden:
                element.Opacity(0.0);
                element.Visibility(winrt::Microsoft::UI::Xaml::Visibility::Visible);
                break;
            case maui::core::visibility::collapsed:
                element.Opacity(mirrored_opacity);
                element.Visibility(winrt::Microsoft::UI::Xaml::Visibility::Collapsed);
                break;
        }
    }

    // UpdateOpacity: platformView.Opacity = opacity. The Hidden state pins opacity 0 (C#'s
    // `view.Visibility == Hidden ? 0 : view.Opacity`), so callers gate on their hidden mirror and let
    // apply_visibility restore the value when the view becomes visible again.
    inline void apply_opacity(void* native, double value)
    {
        if (auto element = borrow_as<winrt::Microsoft::UI::Xaml::FrameworkElement>(native))
        {
            element.Opacity(value);
        }
    }

    // UpdateAutomationId: AutomationProperties.SetAutomationId(platformView, view.AutomationId).
    inline void apply_automation_id(void* native, std::string_view value)
    {
        if (auto element = borrow_as<winrt::Microsoft::UI::Xaml::FrameworkElement>(native))
        {
            winrt::Microsoft::UI::Xaml::Automation::AutomationProperties::SetAutomationId(element,
                                                                                          to_hstring_utf8(value));
        }
    }
} // namespace maui::platform::win
