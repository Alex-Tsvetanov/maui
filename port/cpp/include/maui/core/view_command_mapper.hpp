#pragma once
// maui::core::view_command_mapper  <=  Microsoft.Maui.Handlers.ViewHandler.ViewCommandMapper (the generic
// CommandMapper<IView, IViewHandler> shared by every view handler).
//
// The shared command mapper for the fundamental IView commands — Focus and Unfocus (ViewHandler.cs:
// ViewCommandMapper { [nameof(IView.Focus)] = MapFocus, [nameof(IView.Unfocus)] = MapUnfocus }). Every
// concrete view handler CHAINS this as the fallback of its own command mapper (see
// entry_handler::command_mapper), so a Focus/Unfocus command resolves here when the control's own table
// has no override, exactly as C#'s per-control CommandMapper chains ViewHandler.ViewCommandMapper.
//
// MapFocus (ViewHandler.MapFocus): read the focus_request payload, ask the native view to take first
// responder (view_focus_ops::focus_native_view via the handler's native view), record the realized result
// on the request, and reflect it onto the virtual view's IsFocused — the port's analog of C#'s native
// focus callback writing IView.IsFocused after BecomeFirstResponder.
// MapUnfocus (ViewHandler.MapUnfocus): resign the native first responder, then clear IsFocused.

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"

namespace maui::core
{
    // The shared generic-IView command mapper (Meyers singleton — one table, like view_mapper()).
    command_mapper<i_view, i_view_handler>& view_command_mapper();
} // namespace maui::core
