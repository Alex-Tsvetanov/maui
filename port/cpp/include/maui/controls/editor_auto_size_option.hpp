#pragma once
// maui::controls::editor_auto_size_option  <=  Microsoft.Maui.Controls.EditorAutoSizeOption
//
// Whether an editor changes size to accommodate input as the user enters it. Ported from
// src/Controls/src/Core/EditorAutoSizeOption.cs. Default is disabled (Editor.AutoSizeProperty).

#include <cstdint>

namespace maui::controls
{
    enum class editor_auto_size_option : std::uint8_t
    {
        disabled = 0,
        text_changes = 1,
    };
} // namespace maui::controls
