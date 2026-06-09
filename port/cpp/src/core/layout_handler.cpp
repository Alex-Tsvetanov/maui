// layout_handler — cross-platform part: the shared mapper + command tables + ctor (LayoutHandler.cs).
// The platform recipe (create + the add/remove/… subview wiring) lives in the per-backend partial.

#include "maui/core/layout_handler.hpp"

#include <any>

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_layout.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"

namespace maui::core
{
    // The layout's own property mapper, chained after the shared view_mapper (C# LayoutHandler.Mapper =
    // new(ViewHandler.ViewMapper) { [ClipsToBounds] = MapClipsToBounds }). The generic IView properties
    // (Visibility/Opacity/IsEnabled/AutomationId/...) map through the chained view_mapper; the layout's own
    // ClipsToBounds maps here to the panel's clip flag.
    property_mapper<i_layout, layout_handler>& layout_handler::mapper()
    {
        static property_mapper<i_layout, layout_handler> table = [] {
            property_mapper<i_layout, layout_handler> mapped{
                {"clips_to_bounds", &layout_handler::map_clips_to_bounds},
            };
            mapped.set_chained({&view_mapper()});
            return mapped;
        }();
        return table;
    }

    // ILayout.ClipsToBounds → the panel's clip flag (C# LayoutHandler chains ViewMapper whose
    // MapClipsToBounds sets PlatformView.ClipsToBounds; the port routes through layout_platform).
    void layout_handler::map_clips_to_bounds(layout_handler& handler, i_layout& layout)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->update_clips_to_bounds(layout.clips_to_bounds());
        }
    }

    // The child-management commands (C# LayoutHandler.CommandMapper): each forwards a layout_handler_update
    // (or a bare view, for update_z_index) payload to the typed i_layout_handler methods. The type must be
    // qualified inside the body: the method name `command_mapper` shadows the template.
    maui::core::command_mapper<i_layout, layout_handler>& layout_handler::command_mapper()
    {
        static maui::core::command_mapper<i_layout, layout_handler> table{
            {"add", &layout_handler::map_add},       {"remove", &layout_handler::map_remove},
            {"clear", &layout_handler::map_clear},   {"insert", &layout_handler::map_insert},
            {"update", &layout_handler::map_update}, {"update_z_index", &layout_handler::map_update_z_index},
        };
        return table;
    }

    layout_handler::layout_handler() : view_handler(&mapper(), &command_mapper())
    {
    }

    // ---- command map functions: unwrap the std::any payload and call the typed methods ----
    // (C# MapAdd/MapRemove/… : `if (arg is LayoutHandlerUpdate args) handler.Add(args.View);`.)

    void layout_handler::map_add(layout_handler& handler, i_layout& /*layout*/, const std::any& args)
    {
        if (const auto* update = std::any_cast<layout_handler_update>(&args);
            update != nullptr && update->view != nullptr)
        {
            handler.add(*update->view);
        }
    }

    void layout_handler::map_remove(layout_handler& handler, i_layout& /*layout*/, const std::any& args)
    {
        if (const auto* update = std::any_cast<layout_handler_update>(&args);
            update != nullptr && update->view != nullptr)
        {
            handler.remove(*update->view);
        }
    }

    void layout_handler::map_clear(layout_handler& handler, i_layout& /*layout*/, const std::any& /*args*/)
    {
        handler.clear();
    }

    void layout_handler::map_insert(layout_handler& handler, i_layout& /*layout*/, const std::any& args)
    {
        if (const auto* update = std::any_cast<layout_handler_update>(&args);
            update != nullptr && update->view != nullptr)
        {
            handler.insert(update->index, *update->view);
        }
    }

    void layout_handler::map_update(layout_handler& handler, i_layout& /*layout*/, const std::any& args)
    {
        if (const auto* update = std::any_cast<layout_handler_update>(&args);
            update != nullptr && update->view != nullptr)
        {
            handler.update(update->index, *update->view);
        }
    }

    // C# MapUpdateZIndex casts `arg is IView`, so the z-index command carries the bare child view (what
    // VisualElement.MapZIndex passes). The port accepts BOTH: a raw i_view* (from view<>'s z-index change)
    // and a layout_handler_update (the uniform layout-command payload), so either dispatch path works.
    void layout_handler::map_update_z_index(layout_handler& handler, i_layout& /*layout*/, const std::any& args)
    {
        if (const auto* view = std::any_cast<i_view*>(&args); view != nullptr && *view != nullptr)
        {
            handler.update_z_index(**view);
        }
        else if (const auto* update = std::any_cast<layout_handler_update>(&args);
                 update != nullptr && update->view != nullptr)
        {
            handler.update_z_index(*update->view);
        }
    }
} // namespace maui::core
