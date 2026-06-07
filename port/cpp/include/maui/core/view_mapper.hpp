#pragma once
// maui::core::view_mapper  <=  Microsoft.Maui.Handlers.ViewHandler.ViewMapper (the generic-IView
// PropertyMapper<IView, IViewHandler> shared by every view handler).
//
// The shared property mapper for the fundamental IView properties. Every concrete view handler CHAINS
// this as the base of its own mapper (see button_handler::mapper / label_handler::mapper), so the
// generic IView properties map before — and in the chained position ahead of — the control-specific
// ones, exactly as C#'s ButtonHandler.Mapper chains ViewHandler.ViewMapper.
//
// This M4b cut maps EXACTLY four properties — Visibility, Opacity, IsEnabled, AutomationId. Each map_*
// reaches the platform view's view_platform_base face via i_view_handler::platform_base() (null when
// the handler's platform view does not derive view_platform_base — then the map is a no-op) and calls
// the matching update_*. The wider ViewMapper set (Width/Height/Background/transforms/Clip/Shadow/
// FlowDirection/…) is deferred (see STATUS.md). C#'s IsConnectingHandler() default-skip optimization is
// not ported here (it is a perf gate, behavior-preserving to omit for this subset).
//
// The key strings ("visibility"/"opacity"/"is_enabled"/"automation_id") MUST match the bindable
// property names the control raises (see controls/view.cpp) so update_value() finds the right map.

#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/property_mapper.hpp"

namespace maui::core
{
    // The shared generic-IView mapper (Meyers singleton — one table, like each handler's mapper()).
    property_mapper<i_view, i_view_handler>& view_mapper();
} // namespace maui::core
