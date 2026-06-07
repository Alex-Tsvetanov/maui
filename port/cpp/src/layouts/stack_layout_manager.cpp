// stack_layout_manager — shared spacing computation. See stack_layout_manager.hpp.

#include "maui/layouts/stack_layout_manager.hpp"

namespace maui::layouts
{
    double stack_layout_manager::measure_spacing(double spacing, int child_count)
    {
        return child_count > 1 ? (child_count - 1) * spacing : 0;
    }
} // namespace maui::layouts
