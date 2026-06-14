#pragma once
// maui::core::i_flex_layout  <=  Microsoft.Maui.IFlexLayout
//
// A flexbox-like layout that lays out children in optionally wrappable rows or columns. Ported from
// src/Core/src/Core/IFlexLayout.cs (IFlexLayout : ILayout). Exposes the container-level flex knobs
// (direction / justify / align-content / align-items / position / wrap), the per-child flex attached
// values (order / grow / shrink / align-self / basis), the engine entry points (layout(width,height) +
// get_flex_frame), all consumed by the flex_layout_manager.

#include "maui/core/i_layout.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/layouts/flex_basis.hpp"
#include "maui/layouts/flex_enums.hpp"

namespace maui::core
{
    class i_flex_layout : public i_layout
    {
    public:
        // ---- container-level flex properties ----
        [[nodiscard]] virtual maui::layouts::flex_direction direction() const = 0;
        [[nodiscard]] virtual maui::layouts::flex_justify justify_content() const = 0;
        [[nodiscard]] virtual maui::layouts::flex_align_content align_content() const = 0;
        [[nodiscard]] virtual maui::layouts::flex_align_items align_items() const = 0;
        [[nodiscard]] virtual maui::layouts::flex_position position() const = 0;
        [[nodiscard]] virtual maui::layouts::flex_wrap wrap() const = 0;

        // ---- per-child flex attached properties ----
        [[nodiscard]] virtual int get_order(const i_view& view) const = 0;
        [[nodiscard]] virtual float get_grow(const i_view& view) const = 0;
        [[nodiscard]] virtual float get_shrink(const i_view& view) const = 0;
        [[nodiscard]] virtual maui::layouts::flex_align_self get_align_self(const i_view& view) const = 0;
        [[nodiscard]] virtual maui::layouts::flex_basis get_basis(const i_view& view) const = 0;

        // ---- engine entry points (driven by the flex_layout_manager) ----
        // The arranged flex frame of a child after the most recent layout(...) call.
        [[nodiscard]] virtual maui::graphics::rect get_flex_frame(const i_view& view) const = 0;
        // Run the flex algorithm over the children for the given available area.
        virtual void layout(double width, double height) = 0;
    };
} // namespace maui::core
