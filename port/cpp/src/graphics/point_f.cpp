// maui::graphics::point_f  <=  Microsoft.Maui.Graphics.PointF
#include "maui/graphics/point_f.hpp"

#include <cmath>
#include <format>
#include <string>
#include <string_view>

#include "maui/graphics/point.hpp"
#include "maui/graphics/size_f.hpp"

#include "detail/parse_util.hpp"

namespace maui::graphics
{
    const point_f point_f::zero{};

    point_f::point_f(float x_, float y_) : x(x_), y(y_)
    {
    }
    point_f::point_f(const size_f &sz) : x(sz.width), y(sz.height)
    {
    }

    point_f point_f::offset(float dx, float dy) const
    {
        return {x + dx, y + dy};
    }
    point_f point_f::round() const
    {
        return {std::nearbyint(x), std::nearbyint(y)};
    }
    bool point_f::is_empty() const
    {
        return x == 0 && y == 0;
    }
    float point_f::distance(const point_f &o) const
    {
        float const dx = x - o.x;
        float const dy = y - o.y;
        return std::sqrt((dx * dx) + (dy * dy));
    }
    bool point_f::equals(const point_f &o, float epsilon) const
    {
        return std::abs(o.x - x) < epsilon && std::abs(o.y - y) < epsilon;
    }
    std::string point_f::to_string() const
    {
        return std::format("{{X={} Y={}}}", x, y);
    }

    bool point_f::try_parse(std::string_view value, point_f &out)
    {
        std::string_view a;
        std::string_view b;
        float px = 0;
        float py = 0;
        if (detail::ps_split2(value, a, b) && detail::ps_parse_num(a, px) && detail::ps_parse_num(b, py))
        {
            out = point_f(px, py);
            return true;
        }
        out = point_f{};
        return false;
    }

    point_f::operator size_f() const
    {
        return {x, y};
    }
    point_f::operator point() const
    {
        return {x, y};
    }

    bool operator==(const point_f &a, const point_f &b)
    {
        return a.x == b.x && a.y == b.y;
    }
    bool operator!=(const point_f &a, const point_f &b)
    {
        return !(a == b);
    }
    point_f operator+(const point_f &pt, const size_f &sz)
    {
        return {pt.x + sz.width, pt.y + sz.height};
    }
    size_f operator-(const point_f &a, const point_f &b)
    {
        return {a.x - b.x, a.y - b.y};
    }
    point_f operator-(const point_f &pt, const size_f &sz)
    {
        return {pt.x - sz.width, pt.y - sz.height};
    }
}
