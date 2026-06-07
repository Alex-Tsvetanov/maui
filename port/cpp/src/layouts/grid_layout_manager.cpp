// grid_layout_manager — the Grid measure/arrange algorithm. A faithful port of
// src/Core/src/Layouts/GridLayoutManager.cs (incl. its nested GridStructure / Definition / Cell / Span).
// Absolute / Auto / Star sizing, row+column spans, spacing, padding, min/max, and arrange-time star
// expansion for Fill / explicit-size grids.

#include "maui/layouts/grid_layout_manager.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <unordered_map>
#include <vector>

#include "maui/core/dimension.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/grid_unit_type.hpp"
#include "maui/core/i_grid_layout.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/layout_alignment.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/layouts/layout_manager.hpp"

namespace
{
    constexpr double nan_value = std::numeric_limits<double>::quiet_NaN();
    constexpr double inf_value = std::numeric_limits<double>::infinity();

    // The combination of GridLength kinds across the rows/columns a cell spans (a flag set, stored as a
    // plain byte so OR-ing two kinds together is just arithmetic — an enum class would make the combined
    // values out-of-range).
    namespace length_flag
    {
        constexpr std::uint8_t none = 0;
        constexpr std::uint8_t absolute = 1;
        constexpr std::uint8_t automatic = 2;
        constexpr std::uint8_t star = 4;
    } // namespace length_flag

    bool has_flag(std::uint8_t value, std::uint8_t flag)
    {
        return (value & flag) == flag;
    }
    std::uint8_t to_length_flag(maui::core::grid_unit_type unit)
    {
        switch (unit)
        {
            case maui::core::grid_unit_type::absolute:
                return length_flag::absolute;
            case maui::core::grid_unit_type::star:
                return length_flag::star;
            case maui::core::grid_unit_type::automatic:
                return length_flag::automatic;
        }
        return length_flag::none;
    }

    // One row or column: its GridLength plus the resolved Size and (for star defs) MinimumSize.
    class definition
    {
    public:
        explicit definition(maui::core::grid_length length) : length_(length)
        {
            if (length.is_absolute())
            {
                set_size(length.value());
            }
        }

        [[nodiscard]] double size() const
        {
            return size_;
        }
        // Setting Size also pins MinimumSize for non-star defs (their min == size).
        void set_size(double value)
        {
            size_ = value;
            if (!is_star())
            {
                minimum_size_ = value;
            }
        }
        [[nodiscard]] double minimum_size() const
        {
            return minimum_size_;
        }
        void set_minimum_size(double value)
        {
            minimum_size_ = value;
        }
        void update(double size)
        {
            if (size > size_)
            {
                set_size(size);
            }
        }

        [[nodiscard]] bool is_auto() const
        {
            return length_.is_auto();
        }
        [[nodiscard]] bool is_star() const
        {
            return length_.is_star();
        }
        [[nodiscard]] bool is_absolute() const
        {
            return length_.is_absolute();
        }
        [[nodiscard]] const maui::core::grid_length& length() const
        {
            return length_;
        }

    private:
        maui::core::grid_length length_;
        double size_ = 0;
        double minimum_size_ = 0;
    };

    struct cell
    {
        int view_index = 0;
        int row = 0;
        int column = 0;
        int row_span = 0;
        int column_span = 0;
        std::uint8_t column_grid_length_type = length_flag::none;
        std::uint8_t row_grid_length_type = length_flag::none;
        double measure_width = nan_value;
        double measure_height = nan_value;
        bool needs_second_pass = false;

        [[nodiscard]] bool is_column_span_auto() const
        {
            return has_flag(column_grid_length_type, length_flag::automatic);
        }
        [[nodiscard]] bool is_row_span_auto() const
        {
            return has_flag(row_grid_length_type, length_flag::automatic);
        }
        [[nodiscard]] bool is_column_span_star() const
        {
            return has_flag(column_grid_length_type, length_flag::star);
        }
        [[nodiscard]] bool is_row_span_star() const
        {
            return has_flag(row_grid_length_type, length_flag::star);
        }
    };

    struct span_key
    {
        int start = 0;
        int length = 0;
        bool is_column = false;
        bool operator==(const span_key&) const = default;
    };
    struct span_key_hash
    {
        std::size_t operator()(const span_key& key) const noexcept
        {
            const std::size_t h = (static_cast<std::size_t>(key.start) * 397U) ^ static_cast<std::size_t>(key.length);
            return (h * 397U) ^ static_cast<std::size_t>(key.is_column ? 1 : 0);
        }
    };
    struct span
    {
        int start = 0;
        int length = 0;
        bool is_column = false;
        double requested = 0;
        [[nodiscard]] span_key key() const
        {
            return {.start = start, .length = length, .is_column = is_column};
        }
    };

    // C# LayoutExtensions.AdjustForFill.
    maui::graphics::size adjust_for_fill(maui::graphics::size size, const maui::graphics::rect& bounds,
                                         const maui::core::i_view& view)
    {
        if (view.horizontal_layout_alignment() == maui::core::layout_alignment::fill)
        {
            size.width = std::max(bounds.width, size.width);
        }
        if (view.vertical_layout_alignment() == maui::core::layout_alignment::fill)
        {
            size.height = std::max(bounds.height, size.height);
        }
        return size;
    }

    bool any_auto(const std::vector<definition>& definitions)
    {
        return std::ranges::any_of(definitions, [](const definition& current) { return current.is_auto(); });
    }

    double count_stars(const std::vector<definition>& definitions)
    {
        double star_count = 0.0;
        for (const definition& current : definitions)
        {
            if (current.is_star())
            {
                star_count += current.length().value();
            }
        }
        return star_count;
    }
} // namespace

namespace maui::layouts
{
    class grid_layout_manager::grid_structure
    {
    public:
        grid_structure(maui::core::i_grid_layout& grid, double width_constraint, double height_constraint)
            : grid_(&grid), explicit_grid_height_(grid.height()), explicit_grid_width_(grid.width()),
              grid_width_constraint_(maui::core::dimension::is_explicit_set(grid.width()) ? grid.width()
                                                                                          : width_constraint),
              grid_height_constraint_(maui::core::dimension::is_explicit_set(grid.height()) ? grid.height()
                                                                                            : height_constraint),
              grid_max_height_(grid.maximum_height()), grid_min_height_(grid.minimum_height()),
              grid_max_width_(grid.maximum_width()), grid_min_width_(grid.minimum_width()), padding_(grid.padding()),
              column_spacing_(grid.column_spacing()), row_spacing_(grid.row_spacing()), rows_(make_rows(grid)),
              columns_(make_columns(grid)), row_star_count_(count_stars(rows_)),
              column_star_count_(count_stars(columns_)),
              is_star_height_precomputable_(!std::isinf(grid_height_constraint_) && !any_auto(rows_)),
              is_star_width_precomputable_(!std::isinf(grid_width_constraint_) && !any_auto(columns_)),
              children_to_lay_out_(make_children(grid)), cells_(children_to_lay_out_.size())
        {
            initialize_cells();
            measure_cells();
        }

        [[nodiscard]] double measured_grid_width() const
        {
            double width = maui::core::dimension::is_explicit_set(explicit_grid_width_) ? explicit_grid_width_
                                                                                        : grid_minimum_width();
            if (grid_max_width_ >= 0 && width > grid_max_width_)
            {
                width = grid_max_width_;
            }
            if (grid_min_width_ >= 0 && width < grid_min_width_)
            {
                width = grid_min_width_;
            }
            return width;
        }

        [[nodiscard]] double measured_grid_height() const
        {
            double height = maui::core::dimension::is_explicit_set(explicit_grid_height_) ? explicit_grid_height_
                                                                                          : grid_minimum_height();
            if (grid_max_height_ >= 0 && height > grid_max_height_)
            {
                height = grid_max_height_;
            }
            if (grid_min_height_ >= 0 && height < grid_min_height_)
            {
                height = grid_min_height_;
            }
            return height;
        }

        void prepare_for_arrange(maui::graphics::size target_size)
        {
            minimize_stars(rows_);
            minimize_stars(columns_);

            const bool expand_star_rows =
                row_star_count_ > 0 && (maui::core::dimension::is_explicit_set(explicit_grid_height_) ||
                                        grid_->vertical_layout_alignment() == maui::core::layout_alignment::fill);
            const bool expand_star_columns =
                column_star_count_ > 0 && (maui::core::dimension::is_explicit_set(explicit_grid_width_) ||
                                           grid_->horizontal_layout_alignment() == maui::core::layout_alignment::fill);

            if (expand_star_rows)
            {
                const bool limit = !std::isinf(grid_height_constraint_);
                expand_star_definitions(rows_, target_size.height - padding_.vertical_thickness(),
                                        grid_minimum_height() - padding_.vertical_thickness(), row_spacing_,
                                        row_star_count_, limit);
            }
            if (expand_star_columns)
            {
                const bool limit = !std::isinf(grid_width_constraint_);
                expand_star_definitions(columns_, target_size.width - padding_.horizontal_thickness(),
                                        grid_minimum_width() - padding_.horizontal_thickness(), column_spacing_,
                                        column_star_count_, limit);
            }
        }

        [[nodiscard]] maui::graphics::rect get_cell_bounds_for(const maui::core::i_view& view, double x_offset,
                                                               double y_offset) const
        {
            const int first_column = std::clamp(grid_->get_column(view), 0, column_count() - 1);
            const int column_span = std::clamp(grid_->get_column_span(view), 1, column_count() - first_column);
            const int last_column = first_column + column_span;

            const int first_row = std::clamp(grid_->get_row(view), 0, row_count() - 1);
            const int row_span = std::clamp(grid_->get_row_span(view), 1, row_count() - first_row);
            const int last_row = first_row + row_span;

            const double top = top_edge_of_row(first_row);
            const double left = left_edge_of_column(first_column);

            double width = 0;
            double height = 0;
            for (int n = first_column; n < last_column; n++)
            {
                width += columns_[static_cast<std::size_t>(n)].size();
            }
            for (int n = first_row; n < last_row; n++)
            {
                height += rows_[static_cast<std::size_t>(n)].size();
            }
            width += (column_span - 1) * column_spacing_;
            height += (row_span - 1) * row_spacing_;

            return {left + x_offset, top + y_offset, width, height};
        }

    private:
        [[nodiscard]] int row_count() const
        {
            return static_cast<int>(rows_.size());
        }
        [[nodiscard]] int column_count() const
        {
            return static_cast<int>(columns_.size());
        }

        static std::vector<definition> make_rows(const maui::core::i_grid_layout& grid)
        {
            std::vector<definition> rows;
            const int count = grid.row_definition_count();
            if (count == 0)
            {
                rows.emplace_back(maui::core::grid_length::star()); // implied single star row
                return rows;
            }
            rows.reserve(static_cast<std::size_t>(count));
            for (int n = 0; n < count; n++)
            {
                rows.emplace_back(grid.row_definition_at(n).height());
            }
            return rows;
        }

        static std::vector<definition> make_columns(const maui::core::i_grid_layout& grid)
        {
            std::vector<definition> columns;
            const int count = grid.column_definition_count();
            if (count == 0)
            {
                columns.emplace_back(maui::core::grid_length::star()); // implied single star column
                return columns;
            }
            columns.reserve(static_cast<std::size_t>(count));
            for (int n = 0; n < count; n++)
            {
                columns.emplace_back(grid.column_definition_at(n).width());
            }
            return columns;
        }

        static std::vector<maui::core::i_view*> make_children(const maui::core::i_grid_layout& grid)
        {
            std::vector<maui::core::i_view*> children;
            const int count = grid.count();
            for (int n = 0; n < count; n++)
            {
                maui::core::i_view& child = grid.at(n);
                if (child.visibility() != maui::core::visibility::collapsed)
                {
                    children.push_back(&child);
                }
            }
            return children;
        }

        void initialize_cells()
        {
            for (std::size_t n = 0; n < children_to_lay_out_.size(); n++)
            {
                const maui::core::i_view& view = *children_to_lay_out_[n];

                const int column = std::clamp(grid_->get_column(view), 0, column_count() - 1);
                const int column_span = std::clamp(grid_->get_column_span(view), 1, column_count() - column);
                std::uint8_t column_type = length_flag::none;
                for (int index = column; index < column + column_span; index++)
                {
                    column_type |= to_length_flag(columns_[static_cast<std::size_t>(index)].length().unit_type());
                }

                const int row = std::clamp(grid_->get_row(view), 0, row_count() - 1);
                const int row_span = std::clamp(grid_->get_row_span(view), 1, row_count() - row);
                std::uint8_t row_type = length_flag::none;
                for (int index = row; index < row + row_span; index++)
                {
                    row_type |= to_length_flag(rows_[static_cast<std::size_t>(index)].length().unit_type());
                }

                cell& current = cells_[n];
                current = cell{.view_index = static_cast<int>(n),
                               .row = row,
                               .column = column,
                               .row_span = row_span,
                               .column_span = column_span,
                               .column_grid_length_type = column_type,
                               .row_grid_length_type = row_type};

                determine_cell_measure_width(current);
                determine_cell_measure_height(current);
            }

            if (is_star_width_precomputable_)
            {
                resolve_star_columns(grid_width_constraint_);
            }
            if (is_star_height_precomputable_)
            {
                resolve_star_rows(grid_height_constraint_);
            }
        }

        [[nodiscard]] double grid_height() const
        {
            return sum_definitions(rows_, row_spacing_, false) + padding_.vertical_thickness();
        }
        [[nodiscard]] double grid_width() const
        {
            return sum_definitions(columns_, column_spacing_, false) + padding_.horizontal_thickness();
        }
        [[nodiscard]] double grid_minimum_height() const
        {
            return sum_definitions(rows_, row_spacing_, true) + padding_.vertical_thickness();
        }
        [[nodiscard]] double grid_minimum_width() const
        {
            return sum_definitions(columns_, column_spacing_, true) + padding_.horizontal_thickness();
        }

        static double sum_definitions(const std::vector<definition>& definitions, double spacing, bool minimize)
        {
            double sum = 0;
            for (std::size_t n = 0; n < definitions.size(); n++)
            {
                sum += minimize ? definitions[n].minimum_size() : definitions[n].size();
                if (n > 0)
                {
                    sum += spacing;
                }
            }
            return sum;
        }

        void measure_cells()
        {
            first_measure_pass();

            if (!is_star_width_precomputable_)
            {
                resolve_star_columns(grid_width_constraint_);
            }
            if (!is_star_height_precomputable_)
            {
                resolve_star_rows(grid_height_constraint_);
            }

            second_measure_pass();
            resolve_spans();
            minimize_stars_for_measurement();
        }

        maui::graphics::size measure_cell(const cell& current, double width, double height)
        {
            return children_to_lay_out_[static_cast<std::size_t>(current.view_index)]->measure(width, height);
        }

        void first_measure_pass()
        {
            for (cell& current : cells_)
            {
                const bool treat_height_as_auto = treat_cell_height_as_auto(current);
                const bool treat_width_as_auto = treat_cell_width_as_auto(current);

                if (std::isnan(current.measure_height) || std::isnan(current.measure_width))
                {
                    current.needs_second_pass = true;
                    continue;
                }

                const maui::graphics::size measured =
                    measure_cell(current, current.measure_width, current.measure_height);

                if (treat_width_as_auto)
                {
                    if (current.column_span == 1)
                    {
                        columns_[static_cast<std::size_t>(current.column)].update(measured.width);
                    }
                    else
                    {
                        track_span({.start = current.column,
                                    .length = current.column_span,
                                    .is_column = true,
                                    .requested = measured.width});
                    }
                }
                if (treat_height_as_auto)
                {
                    if (current.row_span == 1)
                    {
                        rows_[static_cast<std::size_t>(current.row)].update(measured.height);
                    }
                    else
                    {
                        track_span({.start = current.row,
                                    .length = current.row_span,
                                    .is_column = false,
                                    .requested = measured.height});
                    }
                }
            }
        }

        void second_measure_pass()
        {
            for (const cell& current : cells_)
            {
                if (!current.needs_second_pass)
                {
                    continue;
                }

                double width = 0;
                double height = 0;

                if (std::isinf(current.measure_height))
                {
                    height = inf_value;
                }
                else
                {
                    for (int n = current.row; n < current.row + current.row_span; n++)
                    {
                        height += rows_[static_cast<std::size_t>(n)].size();
                    }
                    height += row_spacing_ * (current.row_span > 0 ? current.row_span - 1 : 0);
                }

                if (std::isinf(current.measure_width))
                {
                    width = inf_value;
                }
                else
                {
                    for (int n = current.column; n < current.column + current.column_span; n++)
                    {
                        width += columns_[static_cast<std::size_t>(n)].size();
                    }
                    width += column_spacing_ * (current.column_span > 0 ? current.column_span - 1 : 0);
                }

                if (width == 0 || height == 0)
                {
                    continue;
                }

                const maui::graphics::size measured = measure_cell(current, width, height);

                if (current.is_column_span_star() && current.column_span > 1)
                {
                    track_span({.start = current.column,
                                .length = current.column_span,
                                .is_column = true,
                                .requested = measured.width});
                }
                else if (current.column_span == 1 && treat_cell_width_as_auto(current))
                {
                    columns_[static_cast<std::size_t>(current.column)].update(measured.width);
                }

                if (current.is_row_span_star() && current.row_span > 1)
                {
                    track_span({.start = current.row,
                                .length = current.row_span,
                                .is_column = false,
                                .requested = measured.height});
                }
                else if (current.row_span == 1 && treat_cell_height_as_auto(current))
                {
                    rows_[static_cast<std::size_t>(current.row)].update(measured.height);
                }
            }
        }

        void track_span(const span& candidate)
        {
            const auto existing = span_index_.find(candidate.key());
            if (existing != span_index_.end())
            {
                if (candidate.requested > spans_[existing->second].requested)
                {
                    spans_[existing->second] = candidate;
                }
            }
            else
            {
                span_index_[candidate.key()] = spans_.size();
                spans_.push_back(candidate);
            }
        }

        void resolve_spans()
        {
            for (const span& current : spans_)
            {
                if (current.is_column)
                {
                    resolve_span(columns_, current.start, current.length, column_spacing_, current.requested);
                }
                else
                {
                    resolve_span(rows_, current.start, current.length, row_spacing_, current.requested);
                }
            }
        }

        static void resolve_span(std::vector<definition>& definitions, int start, int length, double spacing,
                                 double requested_size)
        {
            double current_size = 0;
            const int end = start + length;
            for (int n = start; n < end; n++)
            {
                current_size += definitions[static_cast<std::size_t>(n)].size();
                if (n > start)
                {
                    current_size += spacing;
                }
            }

            if (requested_size <= current_size)
            {
                return;
            }

            const double required = requested_size - current_size;

            int auto_count = 0;
            for (int n = start; n < end; n++)
            {
                if (definitions[static_cast<std::size_t>(n)].is_auto())
                {
                    auto_count += 1;
                }
                else if (definitions[static_cast<std::size_t>(n)].is_star())
                {
                    return; // a star in the span -> the auto parts don't grow
                }
            }

            const double distribution = required / auto_count;
            for (int n = start; n < end; n++)
            {
                definition& current = definitions[static_cast<std::size_t>(n)];
                if (current.is_auto())
                {
                    current.set_size(current.size() + distribution);
                }
            }
        }

        [[nodiscard]] double left_edge_of_column(int column) const
        {
            double left = padding_.left;
            for (int n = 0; n < column; n++)
            {
                left += columns_[static_cast<std::size_t>(n)].size();
                left += column_spacing_;
            }
            return left;
        }
        [[nodiscard]] double top_edge_of_row(int row) const
        {
            double top = padding_.top;
            for (int n = 0; n < row; n++)
            {
                top += rows_[static_cast<std::size_t>(n)].size();
                top += row_spacing_;
            }
            return top;
        }

        void resolve_stars(std::vector<definition>& definitions, double available_space,
                           bool (*cell_check)(const cell&), double (*dimension)(const maui::graphics::size&),
                           double star_count)
        {
            if (available_space <= 0)
            {
                return;
            }

            double star_size = 0;
            if (std::isinf(available_space))
            {
                // Infinite space -> a star means "as big as the content in it" (empty -> 0).
                for (const cell& current : cells_)
                {
                    if (cell_check(current))
                    {
                        star_size = std::max(
                            star_size,
                            dimension(
                                children_to_lay_out_[static_cast<std::size_t>(current.view_index)]->desired_size()));
                    }
                }
            }
            else
            {
                star_size = available_space / star_count;
            }

            for (definition& current : definitions)
            {
                if (current.is_star())
                {
                    current.set_size(star_size * current.length().value());
                }
            }
        }

        void resolve_star_columns(double width_constraint)
        {
            if (column_star_count_ == 0)
            {
                return;
            }
            const double available_space = width_constraint - grid_width();
            resolve_stars(
                columns_, available_space, [](const cell& current) { return current.is_column_span_star(); },
                [](const maui::graphics::size& size) { return size.width; }, column_star_count_);

            for (cell& current : cells_)
            {
                if (std::isnan(current.measure_width))
                {
                    update_known_measure_width(current);
                }
            }
        }

        void resolve_star_rows(double height_constraint)
        {
            if (row_star_count_ == 0)
            {
                return;
            }
            const double available_space = height_constraint - grid_height();
            resolve_stars(
                rows_, available_space, [](const cell& current) { return current.is_row_span_star(); },
                [](const maui::graphics::size& size) { return size.height; }, row_star_count_);

            for (cell& current : cells_)
            {
                if (std::isnan(current.measure_height))
                {
                    update_known_measure_height(current);
                }
            }
        }

        void minimize_stars_for_measurement()
        {
            minimize_star_rows();
            minimize_star_columns();
        }

        void minimize_star_rows()
        {
            for (const cell& current : cells_)
            {
                if (!current.is_row_span_star())
                {
                    continue;
                }
                const double required =
                    std::min(grid_height_constraint_,
                             children_to_lay_out_[static_cast<std::size_t>(current.view_index)]->desired_size().height);
                determine_minimum_star_sizes_in_span(required, rows_, current.row, current.row + current.row_span);
            }
        }

        void minimize_star_columns()
        {
            for (const cell& current : cells_)
            {
                if (!current.is_column_span_star())
                {
                    continue;
                }
                const double required =
                    std::min(grid_width_constraint_,
                             children_to_lay_out_[static_cast<std::size_t>(current.view_index)]->desired_size().width);
                determine_minimum_star_sizes_in_span(required, columns_, current.column,
                                                     current.column + current.column_span);
            }
        }

        static void determine_minimum_star_sizes_in_span(double space_needed, std::vector<definition>& definitions,
                                                         int start, int end)
        {
            for (int n = start; n < end; n++)
            {
                const definition& current = definitions[static_cast<std::size_t>(n)];
                if (current.is_absolute() || current.is_auto())
                {
                    space_needed -= current.size();
                }
            }

            double space_available = 0;
            int stars_in_span = 0;
            for (int n = start; n < end; n++)
            {
                const definition& current = definitions[static_cast<std::size_t>(n)];
                if (current.is_star())
                {
                    stars_in_span += 1;
                    space_available += current.minimum_size();
                }
            }

            if (space_available < space_needed)
            {
                const double to_add = (space_needed - space_available) / stars_in_span;
                for (int n = start; n < end; n++)
                {
                    definition& current = definitions[static_cast<std::size_t>(n)];
                    if (current.is_star())
                    {
                        current.set_minimum_size(std::min(current.minimum_size() + to_add, current.size()));
                    }
                }
            }
        }

        static void minimize_stars(std::vector<definition>& definitions)
        {
            for (definition& current : definitions)
            {
                if (current.is_star())
                {
                    current.set_size(current.minimum_size());
                }
            }
        }

        static void expand_star_definitions(std::vector<definition>& definitions, double target_size,
                                            double current_size, double spacing, double star_count, bool limit_sizes)
        {
            const double star_size = compute_star_size_for_target(target_size, definitions, spacing, star_count);
            if (limit_sizes)
            {
                ensure_size_limit(definitions, star_size);
            }
            expand_stars(target_size, current_size, definitions, star_size);
        }

        static void ensure_size_limit(std::vector<definition>& definitions, double star_size)
        {
            for (definition& current : definitions)
            {
                if (!current.is_star())
                {
                    continue;
                }
                const double max_size = star_size * current.length().value();
                current.set_size(std::min(max_size, current.size()));
                current.set_minimum_size(std::min(max_size, current.minimum_size()));
            }
        }

        static double compute_star_size_for_target(double target_size, const std::vector<definition>& definitions,
                                                   double spacing, double star_count)
        {
            double sum = sum_definitions(definitions, spacing, true);
            for (const definition& current : definitions)
            {
                if (current.is_star())
                {
                    sum -= current.minimum_size();
                }
            }
            return (target_size - sum) / star_count;
        }

        static void expand_stars(double target_size, double current_size, std::vector<definition>& definitions,
                                 double target_star_size)
        {
            const double available_space = target_size - current_size;
            if (available_space <= 0)
            {
                return;
            }

            double max_current_star_size = 0.0;
            for (const definition& current : definitions)
            {
                if (current.is_star())
                {
                    max_current_star_size =
                        std::max(max_current_star_size, current.minimum_size() / current.length().value());
                }
            }

            if (max_current_star_size <= target_star_size)
            {
                for (definition& current : definitions)
                {
                    if (current.is_star())
                    {
                        current.set_size(target_star_size * current.length().value());
                    }
                }
                return;
            }

            double total_diff = 0;
            for (const definition& current : definitions)
            {
                if (current.is_star())
                {
                    const double full_target = target_star_size * current.length().value();
                    if (current.minimum_size() < full_target)
                    {
                        total_diff += full_target - current.minimum_size();
                    }
                }
            }

            for (definition& current : definitions)
            {
                if (current.is_star())
                {
                    const double full_target = target_star_size * current.length().value();
                    if (current.minimum_size() < full_target)
                    {
                        const double scale = (full_target - current.minimum_size()) / total_diff;
                        current.set_size(current.minimum_size() + (scale * available_space));
                    }
                }
            }
        }

        [[nodiscard]] bool treat_cell_width_as_auto(const cell& current) const
        {
            if (current.is_column_span_star())
            {
                return std::isinf(grid_width_constraint_);
            }
            return current.is_column_span_auto();
        }

        [[nodiscard]] bool treat_cell_height_as_auto(const cell& current) const
        {
            if (current.is_row_span_star())
            {
                return std::isinf(grid_height_constraint_);
            }
            return current.is_row_span_auto();
        }

        void update_known_measure_width(cell& current)
        {
            double measure_width = 0;
            for (int column = current.column; column < current.column + current.column_span; column++)
            {
                measure_width += columns_[static_cast<std::size_t>(column)].size();
                if (column > current.column)
                {
                    measure_width += column_spacing_;
                }
            }
            current.measure_width = measure_width;
        }

        void update_known_measure_height(cell& current)
        {
            double measure_height = 0;
            for (int row = current.row; row < current.row + current.row_span; row++)
            {
                measure_height += rows_[static_cast<std::size_t>(row)].size();
                if (row > current.row)
                {
                    measure_height += row_spacing_;
                }
            }
            current.measure_height = measure_height;
        }

        void determine_cell_measure_width(cell& current)
        {
            if (current.column_grid_length_type == length_flag::absolute)
            {
                update_known_measure_width(current);
            }
            else if (treat_cell_width_as_auto(current))
            {
                current.measure_width = inf_value;
            }
        }

        void determine_cell_measure_height(cell& current)
        {
            if (current.row_grid_length_type == length_flag::absolute)
            {
                update_known_measure_height(current);
            }
            else if (treat_cell_height_as_auto(current))
            {
                current.measure_height = inf_value;
            }
        }

        maui::core::i_grid_layout* grid_;
        double explicit_grid_height_;
        double explicit_grid_width_;
        double grid_width_constraint_;
        double grid_height_constraint_;
        double grid_max_height_;
        double grid_min_height_;
        double grid_max_width_;
        double grid_min_width_;
        maui::core::thickness padding_;
        double column_spacing_;
        double row_spacing_;
        std::vector<definition> rows_;
        std::vector<definition> columns_;
        double row_star_count_;
        double column_star_count_;
        bool is_star_height_precomputable_;
        bool is_star_width_precomputable_;
        std::vector<maui::core::i_view*> children_to_lay_out_;
        std::vector<cell> cells_;
        std::vector<span> spans_;
        std::unordered_map<span_key, std::size_t, span_key_hash> span_index_;
    };

    grid_layout_manager::grid_layout_manager(maui::core::i_grid_layout& grid) : layout_manager(grid), grid_(&grid)
    {
    }

    grid_layout_manager::~grid_layout_manager() = default;

    maui::graphics::size grid_layout_manager::measure(double width_constraint, double height_constraint)
    {
        structure_ = std::make_unique<grid_structure>(*grid_, width_constraint, height_constraint);
        return {structure_->measured_grid_width(), structure_->measured_grid_height()};
    }

    maui::graphics::size grid_layout_manager::arrange_children(const maui::graphics::rect& bounds)
    {
        if (!structure_)
        {
            structure_ = std::make_unique<grid_structure>(*grid_, bounds.width, bounds.height);
        }

        structure_->prepare_for_arrange({bounds.width, bounds.height});

        for (int n = 0; n < grid_->count(); n++)
        {
            maui::core::i_view& view = grid_->at(n);
            if (view.visibility() == maui::core::visibility::collapsed)
            {
                continue;
            }
            view.arrange(structure_->get_cell_bounds_for(view, bounds.left(), bounds.top()));
        }

        const maui::graphics::size actual{structure_->measured_grid_width(), structure_->measured_grid_height()};
        return adjust_for_fill(actual, bounds, *grid_);
    }
} // namespace maui::layouts
