#pragma once
// maui::core::i_view  <=  Microsoft.Maui.IView
//
// The visual-element contract: a view's layout, appearance, and interaction surface. Ported from
// src/Core/src/Core/IView.cs (IView : IElement, ITransform). An abstract class (PROFILE §11).
//
// M1 scope (per the maintainer): the FULL member surface is declared, but the heavy sub-objects are
// only forward-declared — paint (background), semantics, i_shadow (shadow), i_shape (clip) gain real
// definitions at M3/M4, and the typed view-handler accessor (C#'s `new IViewHandler Handler`) is
// added with the handler layer (#22); for now the inherited i_element::handler() suffices. Getters
// whose name collides with their return type (visibility, flow_direction, semantics) qualify the
// return type to keep the type visible.

#include <string_view>

#include "maui/core/flow_direction.hpp"
#include "maui/core/i_element.hpp"
#include "maui/core/i_transform.hpp"
#include "maui/core/layout_alignment.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::graphics
{
    class paint;   // Background (M4)
    class i_shape; // Clip (M4)
} // namespace maui::graphics

namespace maui::core
{
    class semantics; // accessibility metadata (M3/M4)
    class i_shadow;  // Shadow (M4)

    class i_view : public i_element, public i_transform
    {
    public:
        // ---- identity / layout direction ----
        [[nodiscard]] virtual std::string_view automation_id() const = 0;
        [[nodiscard]] virtual maui::core::flow_direction flow_direction() const = 0;
        [[nodiscard]] virtual layout_alignment horizontal_layout_alignment() const = 0;
        [[nodiscard]] virtual layout_alignment vertical_layout_alignment() const = 0;

        // ---- appearance (heavy sub-objects forward-declared; null until M3/M4) ----
        [[nodiscard]] virtual maui::core::semantics* semantics() const = 0;
        [[nodiscard]] virtual maui::graphics::i_shape* clip() const = 0;
        [[nodiscard]] virtual maui::core::i_shadow* shadow() const = 0;
        [[nodiscard]] virtual maui::graphics::paint* background() const = 0;
        [[nodiscard]] virtual maui::core::visibility visibility() const = 0;
        [[nodiscard]] virtual double opacity() const = 0;

        // ---- state ----
        [[nodiscard]] virtual bool is_enabled() const = 0;
        [[nodiscard]] virtual bool is_focused() const = 0;
        virtual void set_is_focused(bool value) = 0;
        [[nodiscard]] virtual bool input_transparent() const = 0;

        // ---- geometry ----
        [[nodiscard]] virtual maui::graphics::rect frame() const = 0;
        virtual void set_frame(maui::graphics::rect value) = 0;
        [[nodiscard]] virtual double width() const = 0;
        [[nodiscard]] virtual double minimum_width() const = 0;
        [[nodiscard]] virtual double maximum_width() const = 0;
        [[nodiscard]] virtual double height() const = 0;
        [[nodiscard]] virtual double minimum_height() const = 0;
        [[nodiscard]] virtual double maximum_height() const = 0;
        [[nodiscard]] virtual thickness margin() const = 0;
        [[nodiscard]] virtual maui::graphics::size desired_size() const = 0;
        [[nodiscard]] virtual int z_index() const = 0;

        // ---- layout pass ----
        virtual maui::graphics::size arrange(const maui::graphics::rect& bounds) = 0;
        virtual maui::graphics::size measure(double width_constraint, double height_constraint) = 0;
        virtual void invalidate_measure() = 0;
        virtual void invalidate_arrange() = 0;

        // ---- focus ----
        virtual bool focus() = 0;
        virtual void unfocus() = 0;
    };
} // namespace maui::core
