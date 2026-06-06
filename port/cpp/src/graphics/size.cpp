// maui::graphics::size  <=  Microsoft.Maui.Graphics.Size
#include "maui/graphics/size.hpp"

#include <format>
#include <string>
#include <string_view>

#include "maui/graphics/point.hpp"
#include "maui/graphics/size_f.hpp"

#include "detail/parse_util.hpp"

namespace maui::graphics
{
    const size size::zero{};

    size::size(double size_) : width(size_), height(size_)
    {
    }
    size::size(double width_, double height_) : width(width_), height(height_)
    {
    }

    bool size::is_zero() const
    {
        return width == 0 && height == 0;
    }
    bool size::equals(const size &o) const
    {
        return width == o.width && height == o.height;
    }
    std::string size::to_string() const
    {
        return std::format("{{Width={} Height={}}}", width, height);
    }

    bool size::try_parse(std::string_view value, size &out)
    {
        std::string_view a;
        std::string_view b;
        double w = 0;
        double h = 0;
        if (detail::ps_split2(value, a, b) && detail::ps_parse_num(a, w) && detail::ps_parse_num(b, h))
        {
            out = size(w, h);
            return true;
        }
        out = size{};
        return false;
    }

    size::operator point() const
    {
        return {width, height};
    }
    size::operator size_f() const
    {
        return {static_cast<float>(width), static_cast<float>(height)};
    }

    bool operator==(const size &a, const size &b)
    {
        return a.width == b.width && a.height == b.height;
    }
    bool operator!=(const size &a, const size &b)
    {
        return !(a == b);
    }
    size operator+(const size &a, const size &b)
    {
        return {a.width + b.width, a.height + b.height};
    }
    size operator-(const size &a, const size &b)
    {
        return {a.width - b.width, a.height - b.height};
    }
    size operator*(const size &s, double v)
    {
        return {s.width * v, s.height * v};
    }
    size operator/(const size &s, double v)
    {
        return {s.width / v, s.height / v};
    }
}
