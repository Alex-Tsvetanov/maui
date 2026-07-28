#pragma once
// winui_visual_ops — the generic-IView property pushes for the WinUI 3 backend: the Windows twin of
// src/platform/ios/ios_visual_ops.hpp. Ported from src/Core/src/Platform/Windows/ViewExtensions.cs and
// BrushExtensions.cs (the `UpdateBackground` / `UpdateVisibility` / `UpdateOpacity` / `UpdateIsEnabled` /
// `UpdateAutomationId` extension methods MAUI's ViewHandler mapper calls).
//
// These exist as FREE FUNCTIONS taking a UIElement, not as methods on a platform struct, because every
// control's platform struct needs the same four or five pushes and they differ only in which native type
// is in the void* slot. Each function does the `try_as` itself, so a control whose native element does
// not expose a property (a TextBlock has no Background and no IsEnabled - it is a FrameworkElement, not
// a Control) degrades to a no-op instead of failing to compile or throwing.

#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>

#include <string_view>

#include "maui/core/visibility.hpp"

namespace maui::graphics
{
    class paint;
} // namespace maui::graphics

namespace maui::platform::windows
{
    // The `native` void* slot of a *_platform struct (a boxed winrt::Microsoft::UI::Xaml::UIElement).
    // Every function below no-ops on a null slot: a mapper can run before create_platform_view on a
    // handler that was never connected, and the Apple twins guard the same way.

    // ViewHandler.MapVisibility -> UIElement.Visibility. MAUI's Windows backend maps Hidden and Collapsed
    // BOTH to Visibility.Collapsed (unlike iOS, where Hidden keeps its layout slot) - XAML has no
    // "invisible but still measured" state, and the port's layout has already run by this point anyway.
    void apply_visibility(void* slot, maui::core::visibility value);

    void apply_opacity(void* slot, double value);

    // Control.IsEnabled. A no-op for a native element that is not a Control (TextBlock, Canvas, Border):
    // C# reaches those through a container, which this first slice does not have.
    void apply_is_enabled(void* slot, bool value);

    // AutomationProperties.SetAutomationId - an attached property, so it works on any UIElement.
    void apply_automation_id(void* slot, std::string_view value);

    // ViewHandler.MapBackground -> the native Background brush. Solid, linear-gradient and
    // radial-gradient paints are translated; a null paint CLEARS the local value so the theme brush
    // returns (C# does the same, and setting Transparent instead would suppress a themed control's own
    // chrome - which is exactly the bug the first Windows capture showed on buttons).
    void apply_background(void* slot, const maui::graphics::paint* value);
} // namespace maui::platform::windows
