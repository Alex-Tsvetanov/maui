// table_view_handler — headless platform recipe: a row-realization simulator standing in for the native
// UITableView / NSTableView. reload() realizes one row per [section,row] through the model (recording a
// realize, or a reuse when a same-type row view is free in the pool), and simulate_select(section,row)
// records the selection + routes through the model's RowSelected (which taps the cell) — the headless
// twin of the UITableView didSelectRow / NSTableView selection notification. The ios/apple .mm partials
// are the real native twins.

#include "maui/controls/table_view_handler.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <utility>

#include "maui/controls/cells/cell.hpp"
#include "maui/controls/i_table_view.hpp"
#include "maui/controls/table_model.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::controls
{
    // Headless has no native view in the `native` slot, so destruction is trivial.
    table_view_platform::table_view_platform() = default;
    table_view_platform::~table_view_platform() = default;

    std::unique_ptr<table_view_platform> table_view_handler::create_platform_view()
    {
        return std::make_unique<table_view_platform>();
    }

    void table_view_handler::on_connect_handler(table_view_platform& /*platform*/)
    {
        // The renderer realizes its rows on connect (TableViewRenderer.SetupVisualElement → ReloadData).
        reload();
    }

    void table_view_handler::on_disconnect_handler(table_view_platform& platform) // static (see header)
    {
        platform.realized.clear();
        platform.recycle_pool.clear();
        platform.selected_path.reset();
        platform.section_headers.clear();
    }

    void table_view_handler::reload()
    {
        realize_all_rows();
    }

    void table_view_handler::realize_all_rows()
    {
        auto* platform = typed_platform_view();
        auto* view = virtual_view();
        if (platform == nullptr || view == nullptr)
        {
            return;
        }
        const table_model* const data_model = view->model();
        if (data_model == nullptr)
        {
            return;
        }

        // Recycle every currently-realized row back into the pool (the renderer's PrepareForReuse on a
        // reload), then realize the model fresh — a same-type pooled row counts as a reuse.
        for (auto& row : platform->realized)
        {
            platform->recycle_pool.emplace_back(row.reuse_id, row.source);
        }
        platform->realized.clear();
        platform->selected_path.reset();
        platform->section_headers.clear();

        const int section_count = data_model->get_section_count();
        for (int section = 0; section < section_count; ++section)
        {
            const int row_count = data_model->get_row_count(section);
            // Record the section header/group row for each non-empty section (the renderer's
            // titleForHeaderInSection; the apple twin renders an explicit header row).
            if (row_count > 0)
            {
                platform->section_headers.push_back(
                    {.section = section, .title = data_model->get_section_title(section)});
            }
            for (int row = 0; row < row_count; ++row)
            {
                const std::shared_ptr<cell> source = data_model->get_cell(section, row);
                if (source == nullptr)
                {
                    continue;
                }
                const std::string reuse_id = reuse_identifier(*source);

                // Dequeue a same-type row view from the pool (cell reuse), else realize a fresh one.
                const auto pooled = std::ranges::find_if(
                    platform->recycle_pool, [&reuse_id](const auto& slot) { return slot.first == reuse_id; });
                const bool reused = pooled != platform->recycle_pool.end();
                if (reused)
                {
                    platform->recycle_pool.erase(pooled);
                }

                const table_row_path path{.section = section, .row = row};
                realized_row realized;
                realized.path = path;
                realized.reuse_id = reuse_id;
                realized.text = display_text(*source);
                realized.source = source;
                describe_cell(realized, *source); // the per-cell-type content (switch on / entry text / image / detail)
                platform->realized.push_back(std::move(realized));

                platform->events.push_back(
                    {.kind = reused ? table_row_event_kind::reused : table_row_event_kind::realized,
                     .path = path,
                     .reuse_id = reuse_id});
            }
        }
    }

    void table_view_handler::simulate_select(int section, int row)
    {
        auto* platform = typed_platform_view();
        auto* view = virtual_view();
        if (platform == nullptr || view == nullptr)
        {
            return;
        }
        table_model* const data_model = view->model();
        if (data_model == nullptr)
        {
            return;
        }
        const table_row_path path{.section = section, .row = row};
        platform->selected_path = path;
        platform->events.push_back({.kind = table_row_event_kind::selected, .path = path, .reuse_id = {}});
        // UITableView didSelectRow / NSTableView selection → TableModel.RowSelected (taps the cell).
        data_model->row_selected(section, row);
    }

    void table_view_handler::map_row_height(table_view_handler& handler, i_table_view& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->row_height = view.row_height();
        }
    }

    void table_view_handler::map_has_uneven_rows(table_view_handler& handler, i_table_view& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->has_uneven_rows = view.has_uneven_rows();
        }
    }

    maui::graphics::size table_view_handler::get_desired_size(double width_constraint,
                                                              double /*height_constraint*/) const
    {
        // Headless placeholder metric: a 40x40 minimum like the C# TableView.OnMeasure minimum size.
        const double width = (width_constraint > 0 && width_constraint < 40.0) ? 40.0 : width_constraint;
        return {width > 0 ? width : 40.0, 40.0};
    }

    void table_view_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native layout to apply.
    }
} // namespace maui::controls
