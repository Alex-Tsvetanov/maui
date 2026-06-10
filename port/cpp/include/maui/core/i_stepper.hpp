#pragma once
// maui::core::i_stepper  <=  Microsoft.Maui.IStepper
//
// The virtual-view contract for the minus/plus numeric stepper: an IView + IRange plus the step
// amount. Ported from src/Core/src/Core/IStepper.cs (IStepper : IView, IRange). C#'s Interval is the
// Core-layer name for the Controls-layer Increment (Stepper's `IStepper.Interval => Increment`).

#include "maui/core/i_range.hpp"
#include "maui/core/i_view.hpp"

namespace maui::core
{
    class i_stepper : public i_view, public i_range
    {
    public:
        // Gets the amount by which Value is increased or decreased (Stepper.Increment).
        [[nodiscard]] virtual double interval() const = 0;
    };
} // namespace maui::core
