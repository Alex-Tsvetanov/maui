#pragma once
// maui::core::view_focus_ops  <=  the per-platform native first-responder extensions behind
// Microsoft.Maui.Handlers.ViewHandler.MapFocus / MapUnfocus (ViewExtensions.Focus(FocusRequest) /
// ViewExtensions.Unfocus(IView) — BecomeFirstResponder / ResignFirstResponder on iOS, makeFirstResponder:
// on AppKit).
//
// Two free functions the shared view_command_mapper calls with the handler's native view — exactly C#'s
// shape, where MapFocus calls `((PlatformView)handler.PlatformView).Focus(request)`, an extension on the
// NATIVE view. Because the native view comes from i_view_handler::native_view(), every control gets the
// behavior uniformly with NO per-control platform-struct override. One definition per backend:
//   - headless (src/platform/headless/view_focus_ops.cpp): no native first responder, so focus always
//     "succeeds" (returns true) and unfocus is a no-op — the view's is_focused state (driven by the
//     command mapper) is the observable result;
//   - apple (src/platform/apple/view_focus_ops.mm): window makeFirstResponder: the NSView (returns
//     whether the window accepted it) / makeFirstResponder:nil to resign;
//   - ios (src/platform/ios/view_focus_ops.mm): UIView becomeFirstResponder (returns its result) /
//     resignFirstResponder.

namespace maui::core
{
    // Make the native view the first responder. Returns whether the native view actually took focus
    // (BecomeFirstResponder's bool / makeFirstResponder:'s acceptance). `native_view` may be null
    // (headless platform views) — headless returns true so the cross-platform focus state machine still
    // reflects the request.
    [[nodiscard]] bool focus_native_view(void* native_view);

    // Resign first responder on the native view. `native_view` may be null (headless) — then a no-op.
    void unfocus_native_view(void* native_view);
} // namespace maui::core
