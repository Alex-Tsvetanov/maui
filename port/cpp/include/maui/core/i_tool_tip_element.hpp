#pragma once
// maui::core::i_tool_tip_element  <=  Microsoft.Maui.IToolTipElement
//
// Marks a view as carrying a tooltip. The shared view_mapper's map_tool_tip cross-casts the i_view to
// this (exactly how C#'s ViewHandler.MapToolTip type-checks `view is IToolTipElement`) and pushes the
// text to the native view (NSView.toolTip on AppKit; a documented no-op on plain iOS — C# materializes
// tooltips on desktop/Catalyst only). Ported from src/Core/src/Core/IToolTipElement.cs; C#'s ToolTip
// wrapper (Content + Element) collapses to the optional text — the only payload the natives consume.

#include <optional>
#include <string>

namespace maui::core
{
    class i_tool_tip_element
    {
    public:
        virtual ~i_tool_tip_element() = default;

        // The tooltip text, or nullopt when never set (C# ToolTipProperties.GetToolTip returns null
        // unless the attached Text property IsSet).
        [[nodiscard]] virtual std::optional<std::string> tool_tip() const = 0;

    protected:
        i_tool_tip_element() = default;
        i_tool_tip_element(const i_tool_tip_element&) = default;
        i_tool_tip_element(i_tool_tip_element&&) = default;
        i_tool_tip_element& operator=(const i_tool_tip_element&) = default;
        i_tool_tip_element& operator=(i_tool_tip_element&&) = default;
    };
} // namespace maui::core
