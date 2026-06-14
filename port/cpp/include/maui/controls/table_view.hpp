#pragma once
// maui::controls::table_view  <=  Microsoft.Maui.Controls.TableView
//
// A view that displays a scrollable list of cells grouped into sections, driven by a table_root. Ported
// from src/Controls/src/Core/TableView/TableView.cs (+ its nested TableSectionModel).
//
// Surface: Root (the table_root; defaults to an empty root), RowHeight (bindable, default -1),
// HasUnevenRows (bindable, default false), Intent (table_intent, default data, INPC on change),
// Model (the table_section_model, read by the handler), and ModelChanged.
//
// Behavior ported from TableView.cs:
//   - Root setter: unhook the old root, swap in the new one (or a fresh empty root), inherit the table's
//     binding context, PARENT every cell to the table (cell.Parent = this), hook the new root's
//     SectionCollectionChanged + Title-change, then OnModelChanged,
//   - OnSectionCollectionChanged: parent the newly-added cells + OnModelChanged,
//   - a section Title change re-runs OnModelChanged,
//   - OnBindingContextChanged: flow the context into the Root,
//   - OnModelChanged: re-parent every cell + raise ModelChanged.
// table_view implements i_cell_container so a hosted cell's render_height reads HasUnevenRows + RowHeight.
//
// DEVIATION (documented): C#'s constructor sets VerticalOptions = HorizontalOptions = FillAndExpand; the
// port has no LayoutOptions surface yet (no `view<>` analog), so that line has no port analog (noted in
// the TestConstructor port — the layout-options assertions are skipped).

#include <memory>
#include <string>
#include <vector>

#include "maui/controls/cells/cell.hpp"
#include "maui/controls/i_table_view.hpp"
#include "maui/controls/observable_collection.hpp"
#include "maui/controls/table_intent.hpp"
#include "maui/controls/table_model.hpp"
#include "maui/controls/table_root.hpp"
#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"

namespace maui::controls
{
    class table_view : public view<i_table_view>, public i_cell_container
    {
    public:
        table_view();
        explicit table_view(std::shared_ptr<table_root> root);
        ~table_view() override;
        table_view(const table_view&) = delete;
        table_view(table_view&&) = delete;
        table_view& operator=(const table_view&) = delete;
        table_view& operator=(table_view&&) = delete;

        // Shared bindable-property descriptors (TableView.RowHeightProperty / HasUnevenRowsProperty).
        static const maui::core::bindable_property<int>& row_height_property();
        static const maui::core::bindable_property<bool>& has_uneven_rows_property();

        // TableView.ModelChanged.
        maui::core::event<> model_changed;

        // ---- Root (TableView.Root) ----
        [[nodiscard]] const std::shared_ptr<table_root>& root() const
        {
            return root_;
        }
        void set_root(std::shared_ptr<table_root> value);

        // ---- RowHeight / HasUnevenRows / Intent (i_table_view + the developer surface) ----
        [[nodiscard]] int row_height() const override
        {
            return row_height_.get();
        }
        void set_row_height(int value)
        {
            row_height_.set(value);
        }
        [[nodiscard]] bool has_uneven_rows() const override
        {
            return has_uneven_rows_.get();
        }
        void set_has_uneven_rows(bool value)
        {
            has_uneven_rows_.set(value);
        }
        [[nodiscard]] table_intent intent() const override
        {
            return intent_;
        }
        void set_intent(table_intent value);

        // ---- i_table_view ----
        [[nodiscard]] table_model* model() const override
        {
            return model_.get();
        }

    protected:
        // TableView.OnBindingContextChanged: flow the context into the Root.
        void on_binding_context_changed() override;
        // Visit the Root's cells as logical children (so the binding context propagates to the cells the
        // table parents). The Root itself isn't an element, so the table flattens to its cells.
        void for_each_logical_child(const std::function<void(element&)>& visit) const override;
        // RowHeight / HasUnevenRows changes re-run their mappers (handled by view<>::on_property_changed)
        // and refresh every cell's RenderHeight notification (Cell listens for RowHeight on the parent).
        void on_property_changed(std::string_view name) override;

    private:
        // Non-virtual cell visitation over the Root's sections (the implementation for_each_logical_child
        // and parent_all_cells both call). Non-virtual so the constructor can use it without the
        // virtual-call-during-construction hazard.
        void visit_cells(const std::function<void(element&)>& visit) const;
        // TableView.OnModelChanged: re-parent every cell + raise ModelChanged.
        void on_model_changed();
        // Parent every cell in the root to this table (cell.Parent = this) + inherit the table's context.
        void parent_all_cells();
        // Parent the cells newly added to a section (TableView.OnSectionCollectionChanged).
        void parent_added_cells(const collection_changed_args<std::shared_ptr<cell>>& args);
        // Hook / unhook the root's section_collection_changed + property_changed (title) events.
        void hook_root();
        void unhook_root();

        std::shared_ptr<table_root> root_;         // TableView.Root (owned; never null)
        std::unique_ptr<table_model> model_;       // TableView.Model — the table_section_model over root_
        table_intent intent_ = table_intent::data; // TableView._intent (default Data)

        maui::core::property<int> row_height_{*this, row_height_property()};
        maui::core::property<bool> has_uneven_rows_{*this, has_uneven_rows_property()};

        // The root subscriptions (after root_/model_ so they tear down first — §8).
        maui::core::scoped_connection section_changed_token_;
        maui::core::scoped_connection root_property_token_;
    };
} // namespace maui::controls
