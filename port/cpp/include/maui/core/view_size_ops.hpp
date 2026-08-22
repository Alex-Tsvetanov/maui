#pragma once
// maui::core::view_size_ops  <=  Microsoft.Maui.Handlers.ViewHandler.MapMinimumWidth / MapMinimumHeight
// (src/Core/src/Handlers/View/ViewHandler.cs:52-55) -> the per-platform Update extensions.
//
// A free function the SHARED view_mapper calls with the handler's native view — the view_chrome_ops
// shape, for the same reason: the native view comes from i_view_handler::native_view(), so every control
// gets the behavior with NO per-control platform-struct override.
//
// WHY THIS IS A PER-PLATFORM PUSH AND NOT A CROSS-PLATFORM CLAMP. MAUI applies NO size request in the
// shared layer: LayoutExtensions.ComputeDesiredSize (src/Core/src/Layouts/LayoutExtensions.cs:11-31) adds
// the margin to the handler's GetDesiredSize and that is the whole method. Each platform then does its
// own thing with Minimum*:
//   - apple  applies it in the leaf desired-size path itself (ViewHandlerExtensions.iOS.cs:125-126,
//            ResolveConstraints(measured, Width, Minimum, Maximum), unconditionally), so its
//            Update extension is a no-op and the cross-platform clamp in view<>::measure covers it;
//   - android does NOT (ContextExtensions.CreateMeasureSpec, ContextExtensions.cs:418-434, reads
//            minimumSize only inside `if (IsExplicitSet(explicitSize))`). Its ONLY channel is the native
//            floor: ViewExtensions.cs:433-444 UpdateMinimumHeight/Width call
//            View.SetMinimumHeight/Width(ToPixels(ResolveMinimum(value))), which the widget's own
//            onMeasure honours via getSuggestedMinimumHeight/Width (TextView, Button, EditText, …) — and
//            which android.webkit.WebView deliberately does not, which is why MAUI renders a
//            <WebView MinimumHeightRequest="400" /> at its content height. view<>::measure's
//            measure_minimum_width/height stands the shared clamp down on that backend; THIS is the
//            channel that replaces it.
// Backends whose leaf measure already resolves the minimum keep the headless no-op definition.

namespace maui::core
{
    // Push the resolved Minimum*Request onto the native view. Values are in device-independent units and
    // may be Unset (Dimension.Unset / a negative sentinel) — the android definition runs them through the
    // ResolveMinimum(0-when-unset) rule, exactly like ViewExtensions. `native_view` may be null (headless
    // platform views, a VM-less android host) — then this is a no-op.
    void apply_native_minimum_size(void* native_view, double minimum_width, double minimum_height);
} // namespace maui::core
