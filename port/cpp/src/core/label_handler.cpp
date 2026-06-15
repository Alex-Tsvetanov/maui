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
    // first). Maps the visible native text properties plus character_spacing (the kerning attribute),
    // vertical_text_alignment (custom cell on AppKit / MauiLabel draw-rect on UIKit) and text_decorations
    // (underline/strikethrough on the attributed text — LabelHandler.Mapper's TextDecorations entry).
    // line_height (LabelHandler.Mapper LineHeight → UpdateLineHeight, a paragraph-style multiple) and
    // padding (LabelHandler.Mapper Padding → UpdatePadding, the native TextInsets / cell inset) round out
    // the LabelHandler.Mapper coverage. line_break_mode + max_lines (LabelHandler.Mapper LineBreakMode /
    // MaxLines → SetLineBreakMode) drive the UILabel wrap mode + numberOfLines (both map fns delegate to a
    // shared platform refresh, matching MapLineBreakMode/MapMaxLines in Label.iOS.cs).
    property_mapper<i_label, label_handler>& label_handler::mapper()
    {
        static property_mapper<i_label, label_handler> table{
            view_mapper(),
            {
                {"text", &label_handler::map_text},
                {"text_color", &label_handler::map_text_color},
                {"font", &label_handler::map_font},
                {"horizontal_text_alignment", &label_handler::map_horizontal_text_alignment},
                {"vertical_text_alignment", &label_handler::map_vertical_text_alignment},
                {"character_spacing", &label_handler::map_character_spacing},
                {"text_decorations", &label_handler::map_text_decorations},
                {"line_height", &label_handler::map_line_height},
                {"padding", &label_handler::map_padding},
                {"formatted_text", &label_handler::map_formatted_text},
                {"line_break_mode", &label_handler::map_line_break_mode},
                {"max_lines", &label_handler::map_max_lines},
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
