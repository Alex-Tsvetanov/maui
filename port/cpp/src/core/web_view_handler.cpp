// web_view_handler — cross-platform part: the shared mapper tables + ctor (WebViewHandler.cs). The
// platform recipe (create/connect/disconnect/map/measure) lives in the per-backend partial
// (src/platform/headless/web_view_handler.cpp; src/platform/apple_shared/web_view_handler.mm for BOTH
// Apple backends).

#include "maui/core/web_view_handler.hpp"

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_web_view.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"

namespace maui::core
{
    // WebViewHandler.Mapper: Source + UserAgent (cookies are out of scope, the Android/Windows client
    // keys are other platforms'). Chained onto the shared view_mapper so the generic IView properties map
    // first.
    property_mapper<i_web_view, web_view_handler>& web_view_handler::mapper()
    {
        static property_mapper<i_web_view, web_view_handler> table{
            view_mapper(),
            {
                {"source", &web_view_handler::map_source},
                {"user_agent", &web_view_handler::map_user_agent},
            },
        };
        return table;
    }

    // WebViewHandler.CommandMapper: GoBack / GoForward / Reload / Eval / EvaluateJavaScriptAsync.
    // The type must be qualified inside the body: the method name `command_mapper` shadows the template.
    maui::core::command_mapper<i_web_view, web_view_handler>& web_view_handler::command_mapper()
    {
        static maui::core::command_mapper<i_web_view, web_view_handler> table{
            {"go_back", &web_view_handler::map_go_back},
            {"go_forward", &web_view_handler::map_go_forward},
            {"reload", &web_view_handler::map_reload},
            {"eval", &web_view_handler::map_eval},
            {"evaluate_java_script", &web_view_handler::map_evaluate_java_script},
        };
        return table;
    }

    web_view_handler::web_view_handler() : view_handler(&mapper(), &command_mapper())
    {
    }
} // namespace maui::core
