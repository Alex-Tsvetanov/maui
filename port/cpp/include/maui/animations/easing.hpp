#pragma once
// maui::animations::easing  <=  Microsoft.Maui.Easing
//
// Functions that modify values non-linearly, generally used for animations: applied to inputs in
// [0,1], a well-behaved easing returns 0 at 0 and 1 at 1. Ported from src/Core/src/Easing.cs (the
// class lives in namespace Microsoft.Maui in C#; the port files it with the rest of the animation
// engine under maui::animations per the W1-14 unit layout). The eleven standard easings are exposed
// as Meyer-singleton accessors (the C# static readonly fields); default_easing() is C#'s
// Easing.Default (=> CubicInOut). The formulas keep the C# float literals (e.g. the 1.70158f spring
// tension, the 2.75f bounce divisor) so the curves are numerically identical to the oracle.
//
// The wrapped callable is held in a std::function (NOT the port's move_only_function): an easing is
// a shared, copyable value — animations store their easing by value, and the same standard easing is
// handed to many animations at once. C#'s implicit Func<double,double> -> Easing conversion maps to
// the (intentionally implicit-friendly) single-argument constructor taking any callable.

#include <functional>

namespace maui::animations
{
    class easing
    {
    public:
        using ease_function = std::function<double(double)>;

        // C# Easing(Func<double, double>) — throws (std::invalid_argument here, ArgumentNullException
        // in C#) when the callable is empty.
        explicit easing(ease_function easing_func);

        // C# Easing.Ease(v): apply the easing function to v (expected in [0,1]).
        [[nodiscard]] double ease(double v) const;

        // ---- the standard easings (C#'s static readonly fields) ----
        [[nodiscard]] static const easing& linear();       // x => x
        [[nodiscard]] static const easing& sin_out();      // smoothly decelerates
        [[nodiscard]] static const easing& sin_in();       // smoothly accelerates
        [[nodiscard]] static const easing& sin_in_out();   // accelerates in, decelerates out
        [[nodiscard]] static const easing& cubic_in();     // starts slowly, accelerates
        [[nodiscard]] static const easing& cubic_out();    // starts quickly, decelerates
        [[nodiscard]] static const easing& cubic_in_out(); // accelerate + decelerate (the natural pick)
        [[nodiscard]] static const easing& bounce_out();   // leaps to final value, bounces 3x, settles
        [[nodiscard]] static const easing& bounce_in();    // bounces toward the final value
        [[nodiscard]] static const easing& spring_in();    // moves away, then leaps toward final value
        [[nodiscard]] static const easing& spring_out();   // overshoots, then returns
        // C# Easing.Default => CubicInOut ("default" is reserved in C++, hence the suffix).
        [[nodiscard]] static const easing& default_easing();

    private:
        ease_function easing_func_;
    };
} // namespace maui::animations
