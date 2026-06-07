// navigation_page_handler — cross-platform part: the shared mapper + command tables + ctor + the
// "request_navigation" command dispatch (NavigationViewHandler.cs). The platform recipe (create the
// container + the host_current subview swap) lives in the per-backend partial.

#include "maui/core/navigation_page_handler.hpp"

#include <any>
#include <vector>

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_stack_navigation.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/navigation_request.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"

namespace maui::core
{
    // The navigation view's property mapper. This cut has no navigation-specific PROPERTY maps (bar /
    // title / back button are deferred), so it is just the shared view_mapper chained through, so the
    // generic IView properties (Visibility/Opacity/IsEnabled/AutomationId/transform/FlowDirection) reach
    // the container. (C#'s NavigationViewHandler.Mapper likewise mostly forwards to ViewHandler.ViewMapper
    // for the cross-platform host.)
    property_mapper<i_view, navigation_page_handler>& navigation_page_handler::mapper()
    {
        static property_mapper<i_view, navigation_page_handler> table{
            view_mapper(),
            {},
        };
        return table;
    }

    // The navigation command (C# NavigationViewHandler routes RequestNavigation as a command): re-host the
    // new top-most page. The type must be qualified inside the body: the method name `command_mapper`
    // shadows the template.
    maui::core::command_mapper<i_view, navigation_page_handler>& navigation_page_handler::command_mapper()
    {
        static maui::core::command_mapper<i_view, navigation_page_handler> table{
            {"request_navigation", &navigation_page_handler::map_request_navigation},
        };
        return table;
    }

    navigation_page_handler::navigation_page_handler() : view_handler(&mapper(), &command_mapper())
    {
    }

    // C# MapRequestNavigation: read the request's top-most page, re-host it as the container's content,
    // then report completion back to the view (IStackNavigation.NavigationFinished) — synchronous here,
    // standing in for C#'s async completion once the native push/pop animation finishes.
    void navigation_page_handler::map_request_navigation(navigation_page_handler& handler, i_view& view,
                                                         const std::any& args)
    {
        const auto* request = std::any_cast<navigation_request>(&args);
        if (request == nullptr)
        {
            return;
        }
        // The new current page is the top-most (last) in the request's stack, or null for an empty stack.
        i_view* const top = request->stack.empty() ? nullptr : request->stack.back();
        handler.host_current(top);

        // C# IStackNavigation.NavigationFinished(newStack): the transition is complete (the NSView swap is
        // synchronous), so acknowledge it on the view. The realized native stack is the request's stack.
        if (auto* navigation = dynamic_cast<i_stack_navigation*>(&view))
        {
            navigation->navigation_finished(request->stack);
        }
    }
} // namespace maui::core
