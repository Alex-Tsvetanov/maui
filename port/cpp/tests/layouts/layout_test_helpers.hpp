#pragma once
// Test doubles for the layout-manager tests — the C++ equivalent of the C# NSubstitute mocks in
// LayoutTestHelpers / StackLayoutManagerTests. A mock_view records its measure/arrange calls and
// returns a configured size; a mock_stack is a configurable i_stack_layout over a child list. Both
// reuse maui::controls::view<ViewInterface> for the i_view boilerplate (and to dodge the i_view
// diamond), overriding only the bits the tests drive.

#include <cstddef>
#include <memory>
#include <vector>

#include "maui/controls/view.hpp"
#include "maui/core/dimension.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/i_grid_column_definition.hpp"
#include "maui/core/i_grid_layout.hpp"
#include "maui/core/i_grid_row_definition.hpp"
#include "maui/core/i_stack_layout.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::layouts::testing
{
    // A child view whose Measure returns a fixed size and which records its measure/arrange calls.
    class mock_view : public maui::controls::view<maui::core::i_view>
    {
    public:
        void configure(maui::graphics::size size)
        {
            measure_result = size;
            desired_size_ = size; // DesiredSize is available pre-measure too (matching the C# mock)
        }

        [[nodiscard]] maui::graphics::size measure(double width_constraint, double height_constraint) override
        {
            ++measure_count;
            last_measure_width = width_constraint;
            last_measure_height = height_constraint;
            desired_size_ = measure_result;
            return measure_result;
        }

        maui::graphics::size arrange(const maui::graphics::rect& bounds) override
        {
            ++arrange_count;
            last_arrange = bounds;
            frame_ = bounds;
            return {bounds.width, bounds.height};
        }

        [[nodiscard]] maui::core::visibility visibility() const override
        {
            return visibility_value;
        }

        maui::graphics::size measure_result;
        maui::core::visibility visibility_value = maui::core::visibility::visible;
        int measure_count = 0;
        int arrange_count = 0;
        double last_measure_width = 0;
        double last_measure_height = 0;
        maui::graphics::rect last_arrange;
    };

    // A configurable stack layout over a list of child views (non-owning; the fixture owns the views).
    class mock_stack : public maui::controls::view<maui::core::i_stack_layout>
    {
    public:
        // i_container
        [[nodiscard]] int count() const override
        {
            return static_cast<int>(children.size());
        }
        [[nodiscard]] maui::core::i_view& at(int index) const override
        {
            return *children[static_cast<std::size_t>(index)];
        }
        void add(maui::core::i_view& child) override
        {
            children.push_back(&child);
        }
        void insert(int index, maui::core::i_view& child) override
        {
            children.insert(children.begin() + index, &child);
        }
        void remove_at(int index) override
        {
            children.erase(children.begin() + index);
        }
        void clear() override
        {
            children.clear();
        }
        [[nodiscard]] int index_of(const maui::core::i_view& child) const override
        {
            for (std::size_t i = 0; i < children.size(); ++i)
            {
                if (children[i] == &child)
                {
                    return static_cast<int>(i);
                }
            }
            return -1;
        }

        // i_padding / i_layout / i_stack_layout
        [[nodiscard]] maui::core::thickness padding() const override
        {
            return padding_value;
        }
        [[nodiscard]] bool clips_to_bounds() const override
        {
            return clips_to_bounds_value;
        }
        [[nodiscard]] double spacing() const override
        {
            return spacing_value;
        }

        // Configurable view dimensions (override view<>'s frame-derived defaults).
        [[nodiscard]] double width() const override
        {
            return width_value;
        }
        [[nodiscard]] double height() const override
        {
            return height_value;
        }
        [[nodiscard]] double minimum_width() const override
        {
            return min_width_value;
        }
        [[nodiscard]] double maximum_width() const override
        {
            return max_width_value;
        }
        [[nodiscard]] double minimum_height() const override
        {
            return min_height_value;
        }
        [[nodiscard]] double maximum_height() const override
        {
            return max_height_value;
        }

        std::vector<maui::core::i_view*> children;
        maui::core::thickness padding_value;
        bool clips_to_bounds_value = false;
        double spacing_value = 0;
        double width_value = maui::core::dimension::unset;
        double height_value = maui::core::dimension::unset;
        double min_width_value = maui::core::dimension::minimum;
        double max_width_value = maui::core::dimension::maximum;
        double min_height_value = maui::core::dimension::minimum;
        double max_height_value = maui::core::dimension::maximum;
    };

    // Owns the child views + the stack, mirroring the C# BuildStack / CreateTestLayout helpers.
    class stack_fixture
    {
    public:
        mock_stack stack;

        mock_view& add_view(maui::graphics::size size)
        {
            auto view = std::make_unique<mock_view>();
            view->configure(size);
            mock_view& reference = *view;
            stack.add(reference);
            owned_.push_back(std::move(view));
            return reference;
        }

        void build_stack(int view_count, double view_width, double view_height)
        {
            for (int n = 0; n < view_count; ++n)
            {
                add_view({view_width, view_height});
            }
        }

    private:
        std::vector<std::unique_ptr<mock_view>> owned_;
    };

    // ---- grid test doubles ----

    class mock_grid_row_definition : public maui::core::i_grid_row_definition
    {
    public:
        explicit mock_grid_row_definition(maui::core::grid_length height) : height_(height)
        {
        }
        [[nodiscard]] maui::core::grid_length height() const override
        {
            return height_;
        }

    private:
        maui::core::grid_length height_;
    };

    class mock_grid_column_definition : public maui::core::i_grid_column_definition
    {
    public:
        explicit mock_grid_column_definition(maui::core::grid_length width) : width_(width)
        {
        }
        [[nodiscard]] maui::core::grid_length width() const override
        {
            return width_;
        }

    private:
        maui::core::grid_length width_;
    };

    class mock_grid : public maui::controls::view<maui::core::i_grid_layout>
    {
    public:
        struct cell_info
        {
            int row = 0;
            int column = 0;
            int row_span = 1;
            int column_span = 1;
        };

        // i_container
        [[nodiscard]] int count() const override
        {
            return static_cast<int>(children.size());
        }
        [[nodiscard]] maui::core::i_view& at(int index) const override
        {
            return *children[static_cast<std::size_t>(index)];
        }
        void add(maui::core::i_view& child) override
        {
            children.push_back(&child);
            child_info.emplace_back();
        }
        void insert(int index, maui::core::i_view& child) override
        {
            children.insert(children.begin() + index, &child);
            child_info.insert(child_info.begin() + index, cell_info{});
        }
        void remove_at(int index) override
        {
            children.erase(children.begin() + index);
            child_info.erase(child_info.begin() + index);
        }
        void clear() override
        {
            children.clear();
            child_info.clear();
        }
        [[nodiscard]] int index_of(const maui::core::i_view& child) const override
        {
            for (std::size_t i = 0; i < children.size(); ++i)
            {
                if (children[i] == &child)
                {
                    return static_cast<int>(i);
                }
            }
            return -1;
        }

        // i_padding / i_layout
        [[nodiscard]] maui::core::thickness padding() const override
        {
            return padding_value;
        }
        [[nodiscard]] bool clips_to_bounds() const override
        {
            return clips_to_bounds_value;
        }

        // i_grid_layout
        [[nodiscard]] int row_definition_count() const override
        {
            return static_cast<int>(rows.size());
        }
        [[nodiscard]] const maui::core::i_grid_row_definition& row_definition_at(int index) const override
        {
            return rows[static_cast<std::size_t>(index)];
        }
        [[nodiscard]] int column_definition_count() const override
        {
            return static_cast<int>(columns.size());
        }
        [[nodiscard]] const maui::core::i_grid_column_definition& column_definition_at(int index) const override
        {
            return columns[static_cast<std::size_t>(index)];
        }
        [[nodiscard]] double row_spacing() const override
        {
            return row_spacing_value;
        }
        [[nodiscard]] double column_spacing() const override
        {
            return column_spacing_value;
        }
        [[nodiscard]] int get_row(const maui::core::i_view& view) const override
        {
            return info_for(view).row;
        }
        [[nodiscard]] int get_row_span(const maui::core::i_view& view) const override
        {
            return info_for(view).row_span;
        }
        [[nodiscard]] int get_column(const maui::core::i_view& view) const override
        {
            return info_for(view).column;
        }
        [[nodiscard]] int get_column_span(const maui::core::i_view& view) const override
        {
            return info_for(view).column_span;
        }

        // Configurable grid dimensions (override view<>'s frame-derived defaults).
        [[nodiscard]] double width() const override
        {
            return width_value;
        }
        [[nodiscard]] double height() const override
        {
            return height_value;
        }
        [[nodiscard]] double minimum_width() const override
        {
            return min_width_value;
        }
        [[nodiscard]] double maximum_width() const override
        {
            return max_width_value;
        }
        [[nodiscard]] double minimum_height() const override
        {
            return min_height_value;
        }
        [[nodiscard]] double maximum_height() const override
        {
            return max_height_value;
        }

        std::vector<maui::core::i_view*> children;
        std::vector<cell_info> child_info; // parallel to children
        std::vector<mock_grid_row_definition> rows;
        std::vector<mock_grid_column_definition> columns;
        maui::core::thickness padding_value;
        bool clips_to_bounds_value = false;
        double row_spacing_value = 0;
        double column_spacing_value = 0;
        double width_value = maui::core::dimension::unset;
        double height_value = maui::core::dimension::unset;
        double min_width_value = maui::core::dimension::minimum;
        double max_width_value = maui::core::dimension::maximum;
        double min_height_value = maui::core::dimension::minimum;
        double max_height_value = maui::core::dimension::maximum;

    private:
        [[nodiscard]] const cell_info& info_for(const maui::core::i_view& view) const
        {
            for (std::size_t i = 0; i < children.size(); ++i)
            {
                if (children[i] == &view)
                {
                    return child_info[i];
                }
            }
            static const cell_info fallback;
            return fallback;
        }
    };

    // Owns the grid's children, mirroring the C# grid test setup.
    class grid_fixture
    {
    public:
        mock_grid grid;

        void add_row(maui::core::grid_length height)
        {
            grid.rows.emplace_back(height);
        }
        void add_column(maui::core::grid_length width)
        {
            grid.columns.emplace_back(width);
        }

        mock_view& add_view(maui::graphics::size size, int row, int column, int row_span = 1, int column_span = 1)
        {
            auto view = std::make_unique<mock_view>();
            view->configure(size);
            mock_view& reference = *view;
            grid.children.push_back(&reference);
            grid.child_info.push_back({row, column, row_span, column_span});
            owned_.push_back(std::move(view));
            return reference;
        }

    private:
        std::vector<std::unique_ptr<mock_view>> owned_;
    };
} // namespace maui::layouts::testing
