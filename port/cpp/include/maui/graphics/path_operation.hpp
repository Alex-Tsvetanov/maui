#pragma once
// maui::graphics::path_operation  <=  Microsoft.Maui.Graphics.PathOperation
// The segment kinds a path_f is built from. Kept in its own header so path_f and path_builder can
// share it without pulling in the full path_f definition.

namespace maui::graphics
{
    enum class path_operation
    {
        move,
        line,
        quad,
        cubic,
        arc,
        close
    };
}
