#pragma once
// maui::core::i_range  <=  Microsoft.Maui.IRange
//
// Provides functionality to select a value from a range of values — the surface shared by slider and
// stepper. Ported from src/Core/src/Core/IRange.cs. C#'s IRange derives IView; here it is a bare mixin
// (the port convention for secondary contracts — i_padding/i_text_input do the same) so a control's
// view-interface (i_slider/i_stepper : i_view, i_range) keeps a single i_view subobject.
//
// Value is MUTABLE in C# (the native control writes the user's new value back through it). The setter
// keeps the C# property shape (set_value), and the concrete controls implement it over their bindable
// Value — clamping/coercion included — exactly as Slider/Stepper implement IRange.Value implicitly.

namespace maui::core
{
    class i_range
    {
    public:
        virtual ~i_range() = default;

        // Gets the minimum selectable value.
        [[nodiscard]] virtual double minimum() const = 0;
        // Gets the maximum selectable value.
        [[nodiscard]] virtual double maximum() const = 0;
        // Gets or sets the current value.
        [[nodiscard]] virtual double value() const = 0;
        virtual void set_value(double value) = 0;

    protected:
        i_range() = default;
        i_range(const i_range&) = default;
        i_range(i_range&&) = default;
        i_range& operator=(const i_range&) = default;
        i_range& operator=(i_range&&) = default;
    };
} // namespace maui::core
