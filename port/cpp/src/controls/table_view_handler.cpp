// table_view_handler — cross-platform part: the shared mapper tables, the ctor, and the cell-text /
// reuse-id helpers every backend shares. The platform recipe (create/connect/disconnect/measure/arrange
// + reload/simulate_select/realize_all_rows) lives in the per-backend partial (headless .cpp / ios+apple
// .mm). Ported from TableViewRenderer.cs (cross-platform model + the per-cell text). See
// table_view_handler.hpp.

#include "maui/controls/table_view_handler.hpp"

#include <any>
#include <string>
#include <typeinfo>

#include "maui/controls/cells/cell.hpp"
#include "maui/controls/cells/entry_cell.hpp"
#include "maui/controls/cells/image_cell.hpp"
#include "maui/controls/cells/switch_cell.hpp"
#include "maui/controls/cells/text_cell.hpp"
#include "maui/controls/cells/view_cell.hpp"
#include "maui/controls/i_table_view.hpp"
#include "maui/core/command_mapper.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"

namespace maui::controls
{
    // TableViewRenderer.Mapper: RowHeight / HasUnevenRows over ViewHandler.ViewMapper (the chained shared
    // view_mapper supplies the generic IView keys). A model/root change reloads through the command map.
    maui::core::property_mapper<i_table_view, table_view_handler>& table_view_handler::mapper()
    {
        static maui::core::property_mapper<i_table_view, table_view_handler> table{
            maui::core::view_mapper(),
            {
                {"row_height", &table_view_handler::map_row_height},
                {"has_uneven_rows", &table_view_handler::map_has_uneven_rows},
            }};
        return table;
    }

    maui::core::command_mapper<i_table_view, table_view_handler>& table_view_handler::command_mapper()
    {
        static maui::core::command_mapper<i_table_view, table_view_handler> table{
            {"reload", &table_view_handler::map_reload},
        };
        return table;
    }

    table_view_handler::table_view_handler() : view_handler(&mapper(), &command_mapper())
    {
    }

    void table_view_handler::map_reload(table_view_handler& handler, i_table_view& /*view*/, const std::any& /*args*/)
    {
        handler.reload();
    }

    std::string table_view_handler::display_text(const cell& source)
    {
        // The renderer's GetCell primary text, per concrete cell type.
        if (const auto* text = dynamic_cast<const text_cell*>(&source))
        {
            return text->text(); // image_cell IS-A text_cell, so it falls here too
        }
        if (const auto* toggle = dynamic_cast<const switch_cell*>(&source))
        {
            return toggle->text();
        }
        if (const auto* entry = dynamic_cast<const entry_cell*>(&source))
        {
            return entry->label();
        }
        return {}; // view_cell hosts a content view, not text
    }

    std::string table_view_handler::reuse_identifier(const cell& source)
    {
        // The dequeue identifier: the cell's concrete type name (one reuse bucket per cell type, matching
        // the renderers keying reuse on the cell type). typeid name is stable per type within a build.
        return typeid(source).name();
    }

    // The per-cell-type GetCell dispatch (which native renderer C# would pick). image_cell IS-A text_cell,
    // so it is probed FIRST; the remaining checks are mutually exclusive concrete types.
    cell_content_kind table_view_handler::classify_cell(const cell& source)
    {
        if (dynamic_cast<const image_cell*>(&source) != nullptr)
        {
            return cell_content_kind::image;
        }
        if (dynamic_cast<const text_cell*>(&source) != nullptr)
        {
            return cell_content_kind::text;
        }
        if (dynamic_cast<const switch_cell*>(&source) != nullptr)
        {
            return cell_content_kind::toggle;
        }
        if (dynamic_cast<const entry_cell*>(&source) != nullptr)
        {
            return cell_content_kind::entry;
        }
        if (dynamic_cast<const view_cell*>(&source) != nullptr)
        {
            return cell_content_kind::view;
        }
        return cell_content_kind::none;
    }

    // The cross-platform half of the per-cell-type GetCell: fill the row's content kind + the observable
    // fields the embedded native control is bound to (SwitchCellRenderer's UISwitch.On, EntryCellRenderer's
    // TextField.Text/Placeholder, ImageCellRenderer's image presence, TextCellRenderer's detail). Both the
    // native datasource callbacks (to drive the real sub-control) and the headless mirror call this.
    void table_view_handler::describe_cell(realized_row& row, const cell& source)
    {
        row.content = classify_cell(source);
        switch (row.content)
        {
            case cell_content_kind::image: {
                const auto& img = dynamic_cast<const image_cell&>(source);
                row.detail = img.detail();
                row.has_image = img.image_source() != nullptr && !img.image_source()->is_empty();
                break;
            }
            case cell_content_kind::text: {
                row.detail = dynamic_cast<const text_cell&>(source).detail();
                break;
            }
            case cell_content_kind::toggle: {
                row.toggle_on = dynamic_cast<const switch_cell&>(source).on();
                break;
            }
            case cell_content_kind::entry: {
                const auto& entry = dynamic_cast<const entry_cell&>(source);
                row.entry_text = entry.text();
                row.entry_placeholder = entry.placeholder();
                break;
            }
            case cell_content_kind::view:
            case cell_content_kind::none:
                break;
        }
    }
} // namespace maui::controls
