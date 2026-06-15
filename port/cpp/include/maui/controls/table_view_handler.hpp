#pragma once
// maui::controls::table_view_handler  <=  Microsoft.Maui.Controls.Handlers.Compatibility.TableViewRenderer
//   (UITableView on iOS — TableViewRenderer/TableViewModelRenderer; NSTableView on macOS).
//
// The handler that hosts the native grouped table behind table_view. Lives in the controls layer (like
// collection_view_handler) because it reads the controls-layer table_model. Per-backend split:
//   - HEADLESS: a row-realization simulator — it realizes one row per [section,row] through the model
//     (recording realize/reuse/bind in `events`), tracks the selected path, and exposes simulate_select
//     so tests can drive the UITableView didSelectRow / NSTableView selection path without a device.
//   - iOS:  a real UITableView with a UITableViewDataSource/Delegate; rows realize through dequeue (cell
//     reuse by identifier), didSelectRowAtIndexPath routes back through the model (RowSelected).
//   - macOS: a real NSTableView (view-based) with a dataSource/delegate; row views realize through
//     makeViewWithIdentifier (reuse), the selection notification routes back through the model.
//
// The native row hosts a text label built from the cell (text_cell.Text / Detail; switch_cell.Text;
// entry_cell.Label; view_cell hosts its content view) — a faithful subset of the renderers' cell
// realization. The full per-cell-type native editor wiring (live UISwitch/UITextField inside the row)
// is realized where the C# renderer does it; see the .mm notes for the per-type coverage.

#include <any>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "maui/controls/cells/cell.hpp"
#include "maui/controls/i_table_view.hpp"
#include "maui/core/command_mapper.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::controls
{
    // A realized row's identity (which section + row it currently shows).
    struct table_row_path
    {
        int section = 0;
        int row = 0;
        friend bool operator==(const table_row_path&, const table_row_path&) = default;
    };

    // The realize / reuse / bind trail (the task's oracle record), mirroring the renderer's GetCell path.
    enum class table_row_event_kind : std::uint8_t
    {
        realized = 0, // a fresh native row view was created (no reuse-pool hit)
        reused,       // a pooled row view was dequeued + rebound
        selected,     // a row was selected (didSelectRow / selection-changed)
    };

    struct table_row_event
    {
        table_row_event_kind kind = table_row_event_kind::realized;
        table_row_path path{};
        std::string reuse_id; // the cell's reuse identifier (its type name)
    };

    // The native content a row hosts, per the C# per-cell-type renderers (GetCell): a SwitchCell builds a
    // UISwitch accessory, an EntryCell a UITextField, an ImageCell a text label + an image view, a TextCell
    // text + detail, a ViewCell an arbitrary content view. `none` = an unrecognized cell (text only).
    enum class cell_content_kind : std::uint8_t
    {
        text = 0, // text_cell (Text + Detail)
        toggle,   // switch_cell (a native switch accessory)
        entry,    // entry_cell (a native text field + label)
        image,    // image_cell (a native image view + text)
        view,     // view_cell (a hosted content view)
        none,     // an unknown cell type (no specific native content)
    };

    // One realized (visible) row.
    struct realized_row
    {
        table_row_path path{};
        std::string reuse_id;
        std::string text;             // the row's primary text (text_cell.Text / switch_cell.Text / entry_cell.Label)
        std::shared_ptr<cell> source; // the cell the row was realized from
        // The native content the row built (the per-cell-type GetCell path). The backends embed the real
        // sub-control (UISwitch/UITextField/UIImageView etc.) AND record it here so the suites assert which
        // native content was realized — headless mirrors the kind without a native tree.
        cell_content_kind content = cell_content_kind::text;
        // Per-content observable state the embedded native control was bound to (the tests assert these):
        bool toggle_on = false;        // switch_cell.On → the embedded switch's on-state
        std::string entry_text;        // entry_cell.Text → the embedded text field's text
        std::string entry_placeholder; // entry_cell.Placeholder → the field's placeholder
        std::string detail;            // text_cell/image_cell Detail → the row's detail label
        bool has_image = false;        // image_cell with a resolved ImageSource → an image view is populated
    };

    // A native section header/group row (the table's sections). iOS renders these as the grouped
    // UITableView's section headers (titleForHeaderInSection); apple, which has no native grouped table,
    // renders an explicit non-selectable header ROW before each section's rows. Recorded for the suites.
    struct section_header
    {
        int section = 0;
        std::string title;
    };

    struct table_view_platform : maui::core::view_platform_base
    {
        table_view_platform();
        ~table_view_platform() override; // backend-defined: releases the retained native table view
        table_view_platform(const table_view_platform&) = delete;
        table_view_platform(table_view_platform&&) = delete;
        table_view_platform& operator=(const table_view_platform&) = delete;
        table_view_platform& operator=(table_view_platform&&) = delete;

        // The native table view (headless: null; ios: UITableView; apple: NSTableView in a scroll view).
        void* native = nullptr;

        // ---- mapped mirrors ----
        int row_height = -1;
        bool has_uneven_rows = false;

        // ---- realization state + records (headless; the .mm twins drive the real table) ----
        std::vector<realized_row> realized; // visible rows in layout order
        // The reuse pool keyed by reuse_id (the cell's type name — the dequeue identifier).
        std::vector<std::pair<std::string, std::shared_ptr<cell>>> recycle_pool;
        std::vector<table_row_event> events; // the realize/reuse/select trail
        std::optional<table_row_path> selected_path;
        // The native section headers/group rows realized for each non-empty section (their titles).
        std::vector<section_header> section_headers;
    };

    class table_view_handler : public maui::core::view_handler<table_view_handler, i_table_view, table_view_platform>
    {
    public:
        table_view_handler();

        static maui::core::property_mapper<i_table_view, table_view_handler>& mapper();
        static maui::core::command_mapper<i_table_view, table_view_handler>& command_mapper();

        // Per-backend (headless .cpp / ios+apple .mm): mint the platform struct (+ the native table view).
        static std::unique_ptr<table_view_platform> create_platform_view();
        void on_connect_handler(table_view_platform& platform);
        static void on_disconnect_handler(table_view_platform& platform);

        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

        // ---- the simulator's user-side inbound channels (tests drive these) ----
        // Realize every row in the model (the renderer's initial GetCell pass / ReloadData). Recorded.
        void reload();
        // Select a row (the UITableView didSelectRow / NSTableView selection path): records the select +
        // routes through the model's RowSelected (which taps the cell).
        void simulate_select(int section, int row);

        // ---- mapper entries ----
        static void map_row_height(table_view_handler& handler, i_table_view& view);
        static void map_has_uneven_rows(table_view_handler& handler, i_table_view& view);
        // The MapModel / reload command (TableViewRenderer's model-change reload).
        static void map_reload(table_view_handler& handler, i_table_view& view, const std::any& args);

        // ---- shared realization helpers (cross-platform; called by the native datasource callbacks) ----
        // The display text for a cell (text_cell.Text / switch_cell.Text / entry_cell.Label, etc.).
        [[nodiscard]] static std::string display_text(const cell& source);
        // The reuse identifier for a cell (its concrete type name — the dequeue key).
        [[nodiscard]] static std::string reuse_identifier(const cell& source);
        // The native content kind a cell realizes (the per-cell-type GetCell dispatch).
        [[nodiscard]] static cell_content_kind classify_cell(const cell& source);
        // Fill the per-content fields of a realized_row from the cell (text/detail, the switch on-state, the
        // entry text/placeholder, the image presence) — the cross-platform half of GetCell that both the
        // native datasource callbacks (to drive the embedded controls) and the headless mirror share.
        static void describe_cell(realized_row& row, const cell& source);

    private:
        // Realize one row through the model (dequeue from the pool if a same-type cell is free, else a
        // fresh realize), recording the event. Shared by reload (headless) — the .mm twins realize through
        // the native dequeue path instead.
        void realize_all_rows();
    };
} // namespace maui::controls
