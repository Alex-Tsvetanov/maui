#pragma once
// maui::core::view_mapper  <=  Microsoft.Maui.Handlers.ViewHandler.ViewMapper (the generic-IView
// PropertyMapper<IView, IViewHandler> shared by every view handler).
//
// The shared property mapper for the fundamental IView properties. Every concrete view handler CHAINS
// this as the base of its own mapper (see button_handler::mapper / label_handler::mapper), so the
// generic IView properties map before — and in the chained position ahead of — the control-specific
// ones, exactly as C#'s ButtonHandler.Mapper chains ViewHandler.ViewMapper.
//
// This M4c cut maps the four fundamental IView properties — Visibility, Opacity, IsEnabled,
// AutomationId — PLUS the render transform (the ten ITransform scalars: translation_x/translation_y/
// scale/scale_x/scale_y/rotation/rotation_x/rotation_y/anchor_x/anchor_y — ten keys, all routing to one
// map_transform that rebuilds the whole spec) and FlowDirection. Each map_* reaches the platform view's
// view_platform_base face via i_view_handler::platform_base() (null when the handler's platform view
// does not derive view_platform_base — then the map is a no-op) and calls the matching update_*. The
// remaining ViewMapper set (Width/Height/Background/Clip/Shadow/Semantics/…) is still deferred — those
// need value types not yet ported or are layout-driven (see STATUS.md). C#'s IsConnectingHandler()
// default-skip optimization is not ported here (a perf gate, behavior-preserving to omit).
//
// The key strings ("visibility"/"opacity"/"is_enabled"/"automation_id"/the ten transform names/
// "flow_direction") MUST match the bindable property names the control raises (see controls/view.cpp)
// so update_value() finds the right map.

#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/property_mapper.hpp"

namespace maui::core
{
    // The shared generic-IView mapper (Meyers singleton — one table, like each handler's mapper()).
    property_mapper<i_view, i_view_handler>& view_mapper();
} // namespace maui::core
