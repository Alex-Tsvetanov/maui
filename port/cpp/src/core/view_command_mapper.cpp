// Implementation of the shared view_command_mapper — the generic-IView CommandMapper (Focus / Unfocus)
// every view handler chains. See view_command_mapper.hpp for the design + the C# source mapping.

#include "maui/core/view_command_mapper.hpp"

#include <any>

#include "maui/core/command_mapper.hpp"
#include "maui/core/focus_request.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/view_focus_ops.hpp"

namespace maui::core
{
    namespace
    {
        // ViewHandler.MapFocus: `if (args is not FocusRequest request) return;` then
        // `((PlatformView)handler.PlatformView)?.Focus(request)`. The native focus's bool result is
        // recorded on the request AND reflected onto the virtual view's IsFocused (the C# native focus
        // callback writing IView.IsFocused), which is what fires the control's Focused event +
        // ChangeVisualState through set_is_focused's funnel.
        void map_focus(i_view_handler& handler, i_view& view, const std::any& args)
        {
            const auto* request = std::any_cast<focus_request>(&args);
            if (request == nullptr)
            {
                return;
            }
            const bool focused = focus_native_view(handler.native_view());
            request->try_set_result(focused);
            view.set_is_focused(focused);
        }

        // ViewHandler.MapUnfocus: `((PlatformView)handler.PlatformView)?.Unfocus(view)` — resign first
        // responder, then clear IsFocused (the native resign callback's analog).
        void map_unfocus(i_view_handler& handler, i_view& view, const std::any& /*args*/)
        {
            unfocus_native_view(handler.native_view());
            view.set_is_focused(false);
        }
    } // namespace

    command_mapper<i_view, i_view_handler>& view_command_mapper()
    {
        static command_mapper<i_view, i_view_handler> table{
            {"focus",
             [](i_view_handler& handler, i_view& view, const std::any& args) { map_focus(handler, view, args); }},
            {"unfocus",
             [](i_view_handler& handler, i_view& view, const std::any& args) { map_unfocus(handler, view, args); }},
        };
        return table;
    }
} // namespace maui::core
