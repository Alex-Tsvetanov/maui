// layout_handler — cross-platform part: the shared mapper + command tables + ctor (LayoutHandler.cs).
// The platform recipe (create + the add/remove/… subview wiring) lives in the per-backend partial.

#include "maui/core/layout_handler.hpp"

#include <any>

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_layout.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"

namespace maui::core
{
    // The layout's own property mapper. M4 first cut: the layout hosts children and computes its own
    // geometry, so there are no own visual properties to push yet (C#'s ILayout.Background /
    // ClipsToBounds + the shared ViewMapper arrive with the ViewMapper retrofit). Kept as an explicit
    // empty table so the recipe (and chaining onto the future ViewMapper) is in place.
    property_mapper<i_layout, layout_handler>& layout_handler::mapper()
    {
        static property_mapper<i_layout, layout_handler> table{};
        return table;
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

    void layout_handler::map_update_z_index(layout_handler& handler, i_layout& /*layout*/, const std::any& args)
    {
        if (const auto* update = std::any_cast<layout_handler_update>(&args);
            update != nullptr && update->view != nullptr)
        {
            handler.update_z_index(*update->view);
        }
    }
} // namespace maui::core
