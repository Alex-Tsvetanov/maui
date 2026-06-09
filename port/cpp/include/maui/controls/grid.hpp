#pragma once
// maui::controls::grid  <=  Microsoft.Maui.Controls.Grid
//
// A layout control that arranges its children in rows and columns. It is a layout<> over i_grid_layout:
// the children + padding come from the base, and this adds the row/column definitions, the bindable
// row/column spacing, and the Grid.Row/Column/RowSpan/ColumnSpan attached properties, then supplies the
// grid_layout_manager (the M3b measure/arrange algorithm). Ported from Grid.cs.
//
// Definitions: C# keeps Row/ColumnDefinitionCollection of bindable Row/ColumnDefinition objects; here the
// grid owns std::vector<row_definition>/<column_definition> (the concrete value types), exposed through
// add_*_definition + the i_grid_layout count/at accessors. C#'s Default*Definitions (a single Star) are a
// constraint-computation detail of the deferred VisualElement layout-pass, not part of the content — like
// C#, the grid starts with empty definition collections (the manager treats an empty axis as one implied
// star row/column).
//
// Attached cell info: C# stores Row/Column/Span as attached BindableProperties on BindableObject children
// and, for non-bindable (virtual) children, in an internal _viewInfo dictionary keyed on the view. The
// port has a single, uniform store: a std::unordered_map<const i_view*, cell_info> keyed on the child
// pointer (the map shape the task settled on — it avoids the index-sync a parallel vector would need on
// add/insert/remove). set_* validates exactly as the C# attached-property validators (row/column >= 0,
// span >= 1) and silently ignores an invalid value, as C#'s BindableProperty.validateValue does. get_*
// returns the stored cell_info or, for a child never positioned, the default (row 0, column 0, spans 1) —
// matching C#'s default(int)/1 attached-property defaults.

#include <memory>
#include <unordered_map>
#include <vector>

#include "maui/controls/column_definition.hpp"
#include "maui/controls/layout.hpp"
#include "maui/controls/row_definition.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/i_grid_column_definition.hpp"
#include "maui/core/i_grid_layout.hpp"
#include "maui/core/i_grid_row_definition.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/property.hpp"
#include "maui/core/thickness.hpp"
#include "maui/layouts/grid_layout_manager.hpp"
#include "maui/layouts/i_layout_manager.hpp"

namespace maui::controls
{
    class grid : public layout<maui::core::i_grid_layout>
    {
    public:
        grid() : layout(padding_property())
        {
            this->set_style_target_type<grid>(); // implicit / class style match
        }

        // The Grid.Row/Column/RowSpan/ColumnSpan attached values for one child (C#'s GridInfo). Defaults
        // match the C# attached-property defaults: row/column default(int)==0, spans default 1.
        struct cell_info
        {
            int row = 0;
            int column = 0;
            int row_span = 1;
            int column_span = 1;
        };

        // Shared bindable-property descriptors (one instance per type, like Grid.*SpacingProperty).
        static const maui::core::bindable_property<double>& row_spacing_property();
        static const maui::core::bindable_property<double>& column_spacing_property();
        static const maui::core::bindable_property<maui::core::thickness>& padding_property();

        // ---- row / column definitions (C# AddRowDefinition / RowDefinitions) ----
        void add_row_definition(maui::core::grid_length height)
        {
            row_definitions_.emplace_back(height);
        }
        void add_column_definition(maui::core::grid_length width)
        {
            column_definitions_.emplace_back(width);
        }
        [[nodiscard]] const std::vector<row_definition>& row_definitions() const
        {
            return row_definitions_;
        }
        [[nodiscard]] const std::vector<column_definition>& column_definitions() const
        {
            return column_definitions_;
        }

        // ---- i_grid_layout: definitions ----
        [[nodiscard]] int row_definition_count() const override
        {
            return static_cast<int>(row_definitions_.size());
        }
        [[nodiscard]] const maui::core::i_grid_row_definition& row_definition_at(int index) const override
        {
            return row_definitions_[static_cast<std::size_t>(index)];
        }
        [[nodiscard]] int column_definition_count() const override
        {
            return static_cast<int>(column_definitions_.size());
        }
        [[nodiscard]] const maui::core::i_grid_column_definition& column_definition_at(int index) const override
        {
            return column_definitions_[static_cast<std::size_t>(index)];
        }

        // ---- i_grid_layout: spacing (bindable; getters override, setters public) ----
        [[nodiscard]] double row_spacing() const override
        {
            return row_spacing_.get();
        }
        void set_row_spacing(double value)
        {
            row_spacing_.set(value);
        }
        [[nodiscard]] double column_spacing() const override
        {
            return column_spacing_.get();
        }
        void set_column_spacing(double value)
        {
            column_spacing_.set(value);
        }

        // ---- attached cell properties (C# Grid.GetRow/SetRow/… (IView overloads)) ----
        // get_* return the stored value or the default for an unpositioned child; set_* validate as the
        // C# attached-property validators do (row/column >= 0, span >= 1) and ignore an invalid value.
        [[nodiscard]] int get_row(const maui::core::i_view& view) const override
        {
            return info_for(view).row;
        }
        [[nodiscard]] int get_column(const maui::core::i_view& view) const override
        {
            return info_for(view).column;
        }
        [[nodiscard]] int get_row_span(const maui::core::i_view& view) const override
        {
            return info_for(view).row_span;
        }
        [[nodiscard]] int get_column_span(const maui::core::i_view& view) const override
        {
            return info_for(view).column_span;
        }
        void set_row(maui::core::i_view& view, int row)
        {
            if (row >= 0) // C# RowProperty validateValue: (int)value >= 0
            {
                view_info_[&view].row = row;
            }
        }
        void set_column(maui::core::i_view& view, int column)
        {
            if (column >= 0) // C# ColumnProperty validateValue: (int)value >= 0
            {
                view_info_[&view].column = column;
            }
        }
        void set_row_span(maui::core::i_view& view, int span)
        {
            if (span >= 1) // C# RowSpanProperty validateValue: (int)value >= 1
            {
                view_info_[&view].row_span = span;
            }
        }
        void set_column_span(maui::core::i_view& view, int span)
        {
            if (span >= 1) // C# ColumnSpanProperty validateValue: (int)value >= 1
            {
                view_info_[&view].column_span = span;
            }
        }

        // ---- i_container overrides: keep the cell store from leaking entries for departed children
        // (mirrors C# Grid.OnRemove/OnClear pruning _viewInfo). Add/insert need no entry — get_* falls
        // back to the default cell_info, exactly as an unset C# attached property reads its default.
        void remove_at(int index) override
        {
            const maui::core::i_view* const removed = &this->at(index);
            layout::remove_at(index);
            view_info_.erase(removed);
        }
        void clear() override
        {
            layout::clear();
            view_info_.clear();
        }

    protected:
        [[nodiscard]] std::unique_ptr<maui::layouts::i_layout_manager> create_layout_manager() override
        {
            return std::make_unique<maui::layouts::grid_layout_manager>(*this);
        }

    private:
        [[nodiscard]] cell_info info_for(const maui::core::i_view& view) const
        {
            const auto found = view_info_.find(&view);
            return found == view_info_.end() ? cell_info{} : found->second;
        }

        std::vector<row_definition> row_definitions_;
        std::vector<column_definition> column_definitions_;
        std::unordered_map<const maui::core::i_view*, cell_info> view_info_; // keyed on the child pointer
        maui::core::property<double> row_spacing_{*this, row_spacing_property()};
        maui::core::property<double> column_spacing_{*this, column_spacing_property()};
    };
} // namespace maui::controls
