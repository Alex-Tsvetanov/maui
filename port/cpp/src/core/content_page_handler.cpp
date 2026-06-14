// content_page_handler — cross-platform part: the shared mapper + command tables + ctor
// (ContentViewHandler.cs). The platform recipe (create + the set_content subview re-host) lives in the
// per-backend partial.

#include "maui/core/content_page_handler.hpp"

#include <any>

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_content_view.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"

namespace maui::core
{
    // The content view's property mapper. Mirrors C#'s ContentViewHandler.Mapper: the "content" entry
    // (C# MapContent) re-hosts the content on connect (the initial update_properties) AND on a property-
    // path content change. Chained onto the shared view_mapper so the generic IView properties
    // (Visibility/Opacity/IsEnabled/AutomationId) map first (keys() walks the chain first). ILayout-style
    // Background is deferred (this cut hosts a single child + computes its own geometry).
    property_mapper<i_content_view, content_page_handler>& content_page_handler::mapper()
    {
        static property_mapper<i_content_view, content_page_handler> table{
            view_mapper(),
            {
                {"content", &content_page_handler::map_content},
                // --- platform configuration (W2-24): the iOSSpecific Page knob nudges (see the hpp note).
                {"ios.Page.PrefersStatusBarHidden", &content_page_handler::map_prefers_status_bar_hidden},
                {"ios.Page.PrefersHomeIndicatorAutoHidden", &content_page_handler::map_home_indicator_auto_hidden},
            },
        };
        return table;
    }

    // The content-management command (cf. C# ContentViewHandler's MapContent, routed as a command here):
    // re-host the content subview. The type must be qualified inside the body: the method name
    // `command_mapper` shadows the template.
    maui::core::command_mapper<i_content_view, content_page_handler>& content_page_handler::command_mapper()
    {
        static maui::core::command_mapper<i_content_view, content_page_handler> table{
            {"set_content", &content_page_handler::map_set_content},
        };
        return table;
    }

    content_page_handler::content_page_handler() : view_handler(&mapper(), &command_mapper())
    {
    }

    // C# MapContent → UpdateContent (the property path): re-host the content. Reads the new content from
    // the virtual view, so it runs identically on connect and on a property-path content change.
    void content_page_handler::map_content(content_page_handler& handler, i_content_view& /*view*/)
    {
        handler.set_content();
    }

    // The "set_content" command path: identical re-host, invoked by the control on a runtime content
    // change. Funnels to the same set_content() as map_content.
    void content_page_handler::map_set_content(content_page_handler& handler, i_content_view& /*view*/,
                                               const std::any& /*args*/)
    {
        handler.set_content();
    }
} // namespace maui::core
