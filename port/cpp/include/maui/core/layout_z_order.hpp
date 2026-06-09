#pragma once
// maui::core::order_by_z_index / get_layout_handler_index  <=  Microsoft.Maui.Handlers.LayoutExtensions
//
// The two z-index ordering helpers the layout handlers use to keep the native subview list front-to-back
// by each child's ZIndex (ports of the internal LayoutExtensions in
// src/Core/src/Handlers/Layout/LayoutExtensions.cs):
//   - order_by_z_index(layout) returns the children in ascending ZIndex order, STABLE (ties keep their
//     logical/add order) — C#'s `layout.OrderBy(v => v.ZIndex)` (LINQ OrderBy is a stable sort).
//   - get_layout_handler_index(layout, view) returns the subview index `view` should occupy in that
//     z-ordering: the count of siblings that sort strictly before it (lower ZIndex, or equal ZIndex added
//     earlier), or -1 if `view` is not a child — the faithful C# GetLayoutHandlerIndex algorithm.
//
// These are free functions (the C# extension methods) over the i_layout contract; the layout managers
// arrange in this order and layout_handler reorders the panel's subviews to it.

#include <vector>

namespace maui::core
{
    class i_layout;
    class i_view;

    // The layout's children in ascending z-index order (stable on ties — add order preserved). Non-owning
    // borrows: the returned pointers reference the layout's children, valid until the children mutate.
    [[nodiscard]] std::vector<i_view*> order_by_z_index(const i_layout& layout);

    // The subview index `view` should occupy in the z-ordering of `layout` (C# GetLayoutHandlerIndex):
    // -1 if not a child, else the number of siblings that sort before it.
    [[nodiscard]] int get_layout_handler_index(const i_layout& layout, const i_view& view);
} // namespace maui::core
