// label_handler — cross-platform part: the shared mapper table + ctor (LabelHandler.cs). The platform
// recipe (create/map/measure) lives in the per-backend partial.

#include "maui/core/label_handler.hpp"

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_label.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"

namespace maui::core
{
    // Keyed on i_label, which exposes Text + the i_text_style appearance + i_text_alignment directly, so
    // no chained text mapper is needed (cf. button). Chained onto the shared view_mapper so the generic
    // IView properties (Visibility/Opacity/IsEnabled/AutomationId) map first (keys() walks the chain
    // first). M4 first cut maps the visible NSTextField properties; character_spacing / line_height /
    // vertical alignment / padding / decorations are present on the control but their native mapping is
    // deferred (documented in STATUS).
    property_mapper<i_label, label_handler>& label_handler::mapper()
    {
        static property_mapper<i_label, label_handler> table{
            view_mapper(),
            {
                {"text", &label_handler::map_text},
                {"text_color", &label_handler::map_text_color},
                {"font", &label_handler::map_font},
                {"horizontal_text_alignment", &label_handler::map_horizontal_text_alignment},
            },
        };
        return table;
    }

    // The type must be qualified inside the body: the method name `command_mapper` shadows the template.
    maui::core::command_mapper<i_label, label_handler>& label_handler::command_mapper()
    {
        static maui::core::command_mapper<i_label, label_handler> table{};
        return table;
    }

    label_handler::label_handler() : view_handler(&mapper(), &command_mapper())
    {
    }
} // namespace maui::core
