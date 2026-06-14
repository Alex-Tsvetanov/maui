#pragma once
// maui::controls::i_table_view  <=  Microsoft.Maui.Controls.ITableViewController (the handler-facing
// surface of TableView)
//
// The virtual-view contract the table_view_handler reads to realize the table. Lives in the controls
// layer (like i_items_view for the collection_view_handler) because it traffics in the controls-layer
// table_model. Derives maui::core::i_view so the shared view_mapper still chains for the generic IView
// properties.

#include "maui/controls/table_intent.hpp"
#include "maui/controls/table_model.hpp"
#include "maui/core/i_view.hpp"

namespace maui::controls
{
    class i_table_view : public maui::core::i_view
    {
    public:
        // The current data model (ITableViewController.Model). Borrowed — the table_view owns it. Never
        // null once the table_view is constructed.
        [[nodiscard]] virtual table_model* model() const = 0;
        // TableView.RowHeight (-1 = platform default) / HasUnevenRows / Intent.
        [[nodiscard]] virtual int row_height() const = 0;
        [[nodiscard]] virtual bool has_uneven_rows() const = 0;
        [[nodiscard]] virtual table_intent intent() const = 0;

    protected:
        i_table_view() = default;
        i_table_view(const i_table_view&) = default;
        i_table_view(i_table_view&&) = default;
        i_table_view& operator=(const i_table_view&) = default;
        i_table_view& operator=(i_table_view&&) = default;
    };
} // namespace maui::controls
