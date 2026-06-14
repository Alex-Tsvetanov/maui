#pragma once
// maui::controls::table_intent  <=  Microsoft.Maui.Controls.TableIntent
//
// The visual intent of a table_view (drives platform rendering). Ported from
// src/Controls/src/Core/TableView/TableIntent.cs.

#include <cstdint>

namespace maui::controls
{
    enum class table_intent : std::uint8_t
    {
        menu = 0, // a menu
        settings, // application settings
        form,     // a data-entry form
        data,     // tabular data (the table_view default)
    };
} // namespace maui::controls
