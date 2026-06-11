// maui::animations::easing — see include/maui/animations/easing.hpp. Formulas are verbatim from
// src/Core/src/Easing.cs, keeping the C# float literals so the curves match the oracle bit-for-bit.
#include "maui/animations/easing.hpp"

#include <cmath>
#include <numbers>
#include <stdexcept>
#include <utility>

namespace maui::animations
{
    namespace
    {
        constexpr double pi = std::numbers::pi;
        // The classic Penner spring tension, as the FLOAT literal C# uses (1.70158f).
        constexpr double spring_tension = 1.70158F;

        double bounce_out_curve(double p)
        {
            if (p < 1 / 2.75F)
            {
                return 7.5625F * p * p;
            }
            if (p < 2 / 2.75F)
            {
                p -= 1.5F / 2.75F;
                return (7.5625F * p * p) + .75F;
            }
            if (p < 2.5F / 2.75F)
            {
                p -= 2.25F / 2.75F;
                return (7.5625F * p * p) + .9375F;
            }
            p -= 2.625F / 2.75F;
            return (7.5625F * p * p) + .984375F;
        }
    } // namespace

    easing::easing(ease_function easing_func) : easing_func_(std::move(easing_func))
    {
        if (!easing_func_)
        {
            throw std::invalid_argument("easing: easing_func must not be empty");
        }
    }

    double easing::ease(double v) const
    {
        return easing_func_(v);
    }

    const easing& easing::linear()
    {
        static const easing instance{[](double x) { return x; }};
        return instance;
    }

    const easing& easing::sin_out()
    {
        static const easing instance{[](double x) { return std::sin(x * pi * 0.5F); }};
        return instance;
    }

    const easing& easing::sin_in()
    {
        static const easing instance{[](double x) { return 1.0F - std::cos(x * pi * 0.5F); }};
        return instance;
    }

    const easing& easing::sin_in_out()
    {
        static const easing instance{[](double x) { return (-std::cos(pi * x) / 2.0F) + 0.5F; }};
        return instance;
    }

    const easing& easing::cubic_in()
    {
        static const easing instance{[](double x) { return x * x * x; }};
        return instance;
    }

    const easing& easing::cubic_out()
    {
        static const easing instance{[](double x) { return std::pow(x - 1.0F, 3.0F) + 1.0F; }};
        return instance;
    }

    const easing& easing::cubic_in_out()
    {
        static const easing instance{[](double x) {
            return x < 0.5F ? std::pow(x * 2.0F, 3.0F) / 2.0F : (std::pow((x - 1) * 2.0F, 3.0F) + 2.0F) / 2.0F;
        }};
        return instance;
    }

    const easing& easing::bounce_out()
    {
        static const easing instance{[](double p) { return bounce_out_curve(p); }};
        return instance;
    }

    const easing& easing::bounce_in()
    {
        static const easing instance{[](double p) { return 1.0F - bounce_out_curve(1 - p); }};
        return instance;
    }

    const easing& easing::spring_in()
    {
        static const easing instance{[](double x) { return x * x * (((spring_tension + 1) * x) - spring_tension); }};
        return instance;
    }

    const easing& easing::spring_out()
    {
        static const easing instance{
            [](double x) { return ((x - 1) * (x - 1) * (((spring_tension + 1) * (x - 1)) + spring_tension)) + 1; }};
        return instance;
    }

    const easing& easing::default_easing()
    {
        return cubic_in_out();
    }
} // namespace maui::animations
