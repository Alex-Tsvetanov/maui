#pragma once
// maui::controls::stack_layout  <=  Microsoft.Maui.Controls.StackLayout
//
// A layout control that positions its children in a single line whose direction can be flipped at
// runtime via the bindable Orientation (default Vertical). It is the generic sibling of the
// fixed-orientation vertical_stack_layout / horizontal_stack_layout; prefer those when the orientation
// is known at construction. Like them it is a layout<> over i_stack_layout (children + padding from the
// base; this adds the bindable Spacing inherited from StackBase plus Orientation), and it supplies the
// orientation-dispatching stack_layout_manager (the M3 measure/arrange algorithm that selects the
// vertical vs horizontal manager per pass). Ported from src/Controls/src/Core/Layout/StackLayout.cs
// (Orientation) + StackBase.cs (Spacing).

#include <memory>

#include "maui/controls/layout.hpp"
#include "maui/controls/stack_layout_manager.hpp"
#include "maui/controls/stack_orientation.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/i_stack_layout.hpp"
#include "maui/core/property.hpp"
#include "maui/core/thickness.hpp"
#include "maui/layouts/i_layout_manager.hpp"

namespace maui::controls
{
    class stack_layout : public layout<maui::core::i_stack_layout>
    {
    public:
        stack_layout() : layout(padding_property())
        {
            this->set_style_target_type<stack_layout>(); // implicit / class style match
        }

        // Shared bindable-property descriptors (one instance per type, like StackLayout.*Property /
        // StackBase.SpacingProperty).
        static const maui::core::bindable_property<stack_orientation>& orientation_property();
        static const maui::core::bindable_property<double>& spacing_property();
        static const maui::core::bindable_property<maui::core::thickness>& padding_property();

        // ---- StackLayout.Orientation (default Vertical) ----
        [[nodiscard]] stack_orientation orientation() const
        {
            return orientation_.get();
        }
        void set_orientation(stack_orientation value)
        {
            orientation_.set(value);
        }

        // ---- i_stack_layout ----
        [[nodiscard]] double spacing() const override
        {
            return spacing_.get();
        }
        void set_spacing(double value)
        {
            spacing_.set(value);
        }

    protected:
        [[nodiscard]] std::unique_ptr<maui::layouts::i_layout_manager> create_layout_manager() override
        {
            return std::make_unique<stack_layout_manager>(*this);
        }

    private:
        maui::core::property<stack_orientation> orientation_{*this, orientation_property()};
        maui::core::property<double> spacing_{*this, spacing_property()};
    };
} // namespace maui::controls
