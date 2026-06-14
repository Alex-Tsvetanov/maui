#pragma once
// maui::controls::table_model  <=  Microsoft.Maui.Controls.Internals.TableModel (ITableModel)
//
// The abstract data model a table_view's handler reads to realize sections + rows. Ported from
// src/Controls/src/Core/TableView/TableModel.cs. The table_view supplies a concrete
// table_section_model (table_view.hpp) backed by its table_root.
//
// DEVIATION (documented): C#'s TableModel.GetItem returns `object` (a cell OR an arbitrary data item,
// in which case GetCell wraps it in a TextCell). The port's table_view only ever hosts cells (its model
// is cell-backed), so the port's model is cell-typed: get_cell returns the cell at [section,row]
// directly, and the "wrap a non-cell item in a TextCell" branch has no analog. The row-selection seam
// (row_selected → item_selected event + the virtual on_row_selected the concrete model overrides to tap
// the cell) is faithfully ported.

#include <cstddef>
#include <memory>
#include <string>

#include "maui/controls/cells/cell.hpp"
#include "maui/core/event.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    class table_model
    {
    public:
        virtual ~table_model() = default;
        table_model(const table_model&) = delete;
        table_model(table_model&&) = delete;
        table_model& operator=(const table_model&) = delete;
        table_model& operator=(table_model&&) = delete;

        // The cell at [section, row] (TableModel.GetCell, cell-typed — see header note). Null if absent.
        [[nodiscard]] virtual std::shared_ptr<cell> get_cell(int section, int row) const = 0;
        // Row / section counts + the section header metrics (TableModel.GetRowCount / GetSectionCount /
        // GetSectionTitle / GetSectionTextColor).
        [[nodiscard]] virtual int get_row_count(int section) const = 0;
        [[nodiscard]] virtual int get_section_count() const = 0;
        [[nodiscard]] virtual std::string get_section_title(int section) const = 0;
        [[nodiscard]] virtual maui::graphics::color get_section_text_color(int section) const = 0;

        // TableModel.ItemSelected — raised by row_selected with the selected cell.
        maui::core::event<std::shared_ptr<cell>> item_selected;

        // TableModel.RowSelected(int, int) / RowSelected(object): raise item_selected then run the
        // overridable on_row_selected (the concrete model taps the cell).
        void row_selected(int section, int row)
        {
            row_selected(get_cell(section, row));
        }
        void row_selected(std::shared_ptr<cell> item)
        {
            item_selected.raise(item);
            on_row_selected(item);
        }

    protected:
        table_model() = default;

        // TableModel.OnRowSelected — default no-op; table_section_model overrides to tap the cell.
        virtual void on_row_selected(const std::shared_ptr<cell>& /*item*/)
        {
        }
    };
} // namespace maui::controls
