#pragma once
// maui::controls::tool_tip_properties  <=  Microsoft.Maui.Controls.ToolTipProperties
//
// The attached ToolTip accessors: SetText/GetText store the tooltip text ON the view (the analog of
// the C# attached BindableProperty's per-instance store — view<>::set_tool_tip_text), whose setter
// also runs C#'s OnToolTipPropertyChanged work (handler.UpdateValue("tool_tip") → the shared
// view_mapper materializes NSView.toolTip on AppKit; plain iOS is a documented no-op, C# parity).
// Ported from src/Controls/src/Core/ToolTipProperties.cs. Header-only: both accessors are templates
// (standing in for C#'s any-BindableObject signature; only views materialize the value).

#include <optional>
#include <string>
#include <utility>

namespace maui::controls
{
    class tool_tip_properties
    {
    public:
        tool_tip_properties() = delete; // statics only, like the C# class

        // C# ToolTipProperties.SetText(bindable, value).
        template <class TView> static void set_text(TView& view, std::string text)
        {
            view.set_tool_tip_text(std::move(text));
        }

        // C# ToolTipProperties.GetText — nullopt when never set (the IsSet(TextProperty) probe
        // GetToolTip applies; C# returns null Content then).
        template <class TView> [[nodiscard]] static std::optional<std::string> get_text(const TView& view)
        {
            return view.tool_tip();
        }
    };
} // namespace maui::controls
