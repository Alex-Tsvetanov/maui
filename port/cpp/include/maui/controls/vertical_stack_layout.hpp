#pragma once
// maui::controls::vertical_stack_layout  <=  Microsoft.Maui.Controls.VerticalStackLayout
//
// A layout control that stacks its children top-to-bottom with uniform spacing. It is a layout<> over
// i_stack_layout: the children + padding come from the base, this adds the bindable Spacing and supplies
// the vertical stack manager (the M3 measure/arrange algorithm). Ported from VerticalStackLayout.cs.

#include <memory>

#include "maui/controls/layout.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/i_stack_layout.hpp"
#include "maui/core/property.hpp"
#include "maui/core/thickness.hpp"
#include "maui/layouts/i_layout_manager.hpp"
#include "maui/layouts/vertical_stack_layout_manager.hpp"

namespace maui::controls
{
    class vertical_stack_layout : public layout<maui::core::i_stack_layout>
    {
    public:
        vertical_stack_layout() : layout(padding_property())
        {
            this->set_style_target_type<vertical_stack_layout>(); // implicit / class style match
        }

        // Shared bindable-property descriptors (one instance per type, like VerticalStackLayout.*Property).
        static const maui::core::bindable_property<double>& spacing_property();
        static const maui::core::bindable_property<maui::core::thickness>& padding_property();

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
            return std::make_unique<maui::layouts::vertical_stack_layout_manager>(*this);
        }

    private:
        maui::core::property<double> spacing_{*this, spacing_property()};
    };
} // namespace maui::controls
