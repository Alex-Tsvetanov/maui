// maui::graphics::size_f  <=  Microsoft.Maui.Graphics.SizeF
#include "maui/graphics/size_f.hpp"

#include <format>
#include <string>
#include <string_view>

#include "maui/graphics/point_f.hpp"
#include "maui/graphics/size.hpp"

#include "detail/parse_util.hpp"

namespace maui::graphics
{
    const size_f size_f::zero{};

    size_f::size_f(float size_) : width(size_), height(size_)
    {
    }
    size_f::size_f(float width_, float height_) : width(width_), height(height_)
    {
    }

    bool size_f::is_zero() const
    {
        return width == 0 && height == 0;
    }
    bool size_f::equals(const size_f &o) const
    {
        return width == o.width && height == o.height;
    }
    std::string size_f::to_string() const
    {
        return std::format("{{Width={} Height={}}}", width, height);
    }

    bool size_f::try_parse(std::string_view value, size_f &out)
    {
        std::string_view a;
        std::string_view b;
        double w = 0;
        double h = 0; // C# SizeF.TryParse parses with double.TryParse, then narrows
        if (detail::ps_split2(value, a, b) && detail::ps_parse_num(a, w) && detail::ps_parse_num(b, h))
        {
            out = size_f(static_cast<float>(w), static_cast<float>(h));
            return true;
        }
        out = size_f{};
        return false;
    }

    size_f::operator point_f() const
    {
        return {width, height};
    }
    size_f::operator size() const
    {
        return {width, height};
    }

    bool operator==(const size_f &a, const size_f &b)
    {
        return a.width == b.width && a.height == b.height;
    }
    bool operator!=(const size_f &a, const size_f &b)
    {
        return !(a == b);
    }
    size_f operator+(const size_f &a, const size_f &b)
    {
        return {a.width + b.width, a.height + b.height};
    }
    size_f operator-(const size_f &a, const size_f &b)
    {
        return {a.width - b.width, a.height - b.height};
    }
    size_f operator*(const size_f &s, float v)
    {
        return {s.width * v, s.height * v};
    }
    size_f operator/(const size_f &s, float v)
    {
        return {s.width / v, s.height / v};
    }
}
