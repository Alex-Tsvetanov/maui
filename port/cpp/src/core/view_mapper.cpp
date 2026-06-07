// view_mapper — the shared generic-IView property mapper (ViewHandler.ViewMapper). Maps the four
// fundamental IView properties through i_view_handler::platform_base() onto the platform view's
// view_platform_base face. See view_mapper.hpp.

#include "maui/core/view_mapper.hpp"

#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_platform_base.hpp"

namespace maui::core
{
    namespace
    {
        // Each map_* mirrors ViewHandler.MapVisibility/MapOpacity/MapIsEnabled/MapAutomationId, which
        // call PlatformView.Update*; here the platform-view face is view_platform_base (null when the
        // handler's platform view does not derive it — then the map is a documented no-op).
        void map_visibility(i_view_handler& handler, i_view& view)
        {
            if (auto* base = handler.platform_base())
            {
                base->update_visibility(view.visibility());
            }
        }

        void map_opacity(i_view_handler& handler, i_view& view)
        {
            if (auto* base = handler.platform_base())
            {
                base->update_opacity(view.opacity());
            }
        }

        void map_is_enabled(i_view_handler& handler, i_view& view)
        {
            if (auto* base = handler.platform_base())
            {
                base->update_is_enabled(view.is_enabled());
            }
        }

        void map_automation_id(i_view_handler& handler, i_view& view)
        {
            if (auto* base = handler.platform_base())
            {
                base->update_automation_id(view.automation_id());
            }
        }
    } // namespace

    property_mapper<i_view, i_view_handler>& view_mapper()
    {
        static property_mapper<i_view, i_view_handler> table{
            {"visibility", &map_visibility},
            {"opacity", &map_opacity},
            {"is_enabled", &map_is_enabled},
            {"automation_id", &map_automation_id},
        };
        return table;
    }
} // namespace maui::core
