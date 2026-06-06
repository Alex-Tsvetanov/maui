#pragma once
// Shared GoogleTest support for the maui::graphics ports: value printers + helpers that
// reproduce the two xUnit Assert.Equal float overloads MAUI's tests use.
#include <gtest/gtest.h>

#include <ostream>

#include "maui/graphics/color.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/point_f.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/rect_f.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/size_f.hpp"

namespace maui::graphics
{
    inline std::ostream &operator<<(std::ostream &os, const color &c)
    {
        return os << c.to_string();
    }
    inline std::ostream &operator<<(std::ostream &os, const point_f &p)
    {
        return os << p.to_string();
    }
    inline std::ostream &operator<<(std::ostream &os, const point &p)
    {
        return os << p.to_string();
    }
    inline std::ostream &operator<<(std::ostream &os, const size_f &s)
    {
        return os << s.to_string();
    }
    inline std::ostream &operator<<(std::ostream &os, const size &s)
    {
        return os << s.to_string();
    }
    inline std::ostream &operator<<(std::ostream &os, const rect_f &r)
    {
        return os << r.to_string();
    }
    inline std::ostream &operator<<(std::ostream &os, const rect &r)
    {
        return os << r.to_string();
    }
    inline std::ostream &operator<<(std::ostream &os, const path_f &p)
    {
        return os << "path_f{ops=" << p.operation_count() << ", pts=" << p.count() << "}";
    }
} // namespace maui::graphics

namespace maui_test
{
    // xUnit Assert.Equal(double, double, int precision) compares values rounded to `precision`
    // decimal places; we approximate with a half-ULP-at-that-decimal tolerance.
    constexpr double prec_tol(int precision)
    {
        double tol = 0.5;
        for (int i = 0; i < precision; ++i)
        {
            tol *= 0.1;
        }
        return tol;
    }
} // namespace maui_test

// Macros expand at the call site so GoogleTest reports the right line on failure.
#define MAUI_EXPECT_PREC(expected, actual, precision)                                                                  \
    EXPECT_NEAR(static_cast<double>(expected), static_cast<double>(actual), ::maui_test::prec_tol(precision))
// xUnit Assert.Equal(float, float, float tolerance). Some MAUI color tests pass a large
// tolerance (2f/3f) making the check loose; we reproduce it faithfully (the exact-equality
// and integer-precision assertions are what actually pin the behavior).
#define MAUI_EXPECT_TOL(expected, actual, tolerance)                                                                   \
    EXPECT_NEAR(static_cast<float>(expected), static_cast<float>(actual), static_cast<float>(tolerance))
