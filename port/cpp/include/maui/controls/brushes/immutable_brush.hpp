#pragma once
// maui::controls::immutable_brush  <=  Microsoft.Maui.Controls.ImmutableBrush (internal)
//
// The concrete type behind the named Brush statics (Brush.Red, SolidColorBrush.Green …). Ported from
// src/Controls/src/Core/ImmutableBrush.cs: a SolidColorBrush whose Color is fixed at construction (the
// setter is a no-op) and which refuses to be parented — OnParentChangingCore throws InvalidOperationException
// ("Parent cannot be set on this Brush.").
//
// C# uses a read-only BindableProperty key for Color; the port models the immutability by overriding the
// virtual setter to a no-op and seeding the value once in the ctor (set_color_internal). The parent refusal
// rides element::on_logical_parent_changing (the C# OnParentChangingCore seam).
//
// Internal to the brush family (the named statics return solid_color_brush& backed by these singletons).
// Out-of-line definitions live in brush.cpp (alongside the statics that mint them).

#include <optional>
#include <utility>

#include "maui/controls/brushes/solid_color_brush.hpp"
#include "maui/controls/element.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    class immutable_brush final : public solid_color_brush
    {
    public:
        explicit immutable_brush(maui::graphics::color color)
        {
            // Seed the fixed color through the protected internal setter (the public setter is a no-op).
            set_color_internal(std::optional<maui::graphics::color>{color});
        }

        // C# ImmutableBrush.Color setter is a no-op (the value is read-only). The getter is inherited.
        void set_color(std::optional<maui::graphics::color> value) override
        {
            (void)value;
        }

        // C# ImmutableBrush.OnParentChangingCore — refuses any parent (a shared system brush must stay
        // unparented). Throws on a non-null parent; a null transition (detach) is allowed.
        void on_logical_parent_changing(element* new_parent) override;
    };
} // namespace maui::controls
