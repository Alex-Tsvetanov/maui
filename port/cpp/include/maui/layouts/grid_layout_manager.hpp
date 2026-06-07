#pragma once
// maui::layouts::grid_layout_manager  <=  Microsoft.Maui.Layouts.GridLayoutManager
//
// The row/column layout algorithm: Absolute, Auto (fit-content) and Star (proportional) sizing, with
// row/column spans, spacing, padding, and min/max. The heavy lifting lives in a private grid_structure
// (pimpl) cached between measure and arrange, exactly as the C# GridStructure is. Ported from
// src/Core/src/Layouts/GridLayoutManager.cs.

#include <memory>

#include "maui/core/i_grid_layout.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/layouts/layout_manager.hpp"

namespace maui::layouts
{
    class grid_layout_manager : public layout_manager
    {
    public:
        explicit grid_layout_manager(maui::core::i_grid_layout& grid);
        ~grid_layout_manager() override; // out-of-line: the pimpl grid_structure is incomplete here
        grid_layout_manager(const grid_layout_manager&) = delete;
        grid_layout_manager(grid_layout_manager&&) = delete;
        grid_layout_manager& operator=(const grid_layout_manager&) = delete;
        grid_layout_manager& operator=(grid_layout_manager&&) = delete;

        [[nodiscard]] maui::core::i_grid_layout& grid() const
        {
            return *grid_;
        }

        [[nodiscard]] maui::graphics::size measure(double width_constraint, double height_constraint) override;
        maui::graphics::size arrange_children(const maui::graphics::rect& bounds) override;

    private:
        class grid_structure; // defined in grid_layout_manager.cpp

        maui::core::i_grid_layout* grid_;
        std::unique_ptr<grid_structure> structure_;
    };
} // namespace maui::layouts
