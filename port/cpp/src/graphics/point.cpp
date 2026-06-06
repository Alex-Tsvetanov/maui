// maui::graphics::point  <=  Microsoft.Maui.Graphics.Point
#include "maui/graphics/point.hpp"

#include <cmath>
#include <format>
#include <string>
#include <string_view>

#include "maui/graphics/point_f.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/size_f.hpp"

#include "detail/parse_util.hpp"

namespace maui::graphics
{
    const point point::zero{};

    point::point(double x_, double y_) : x(x_), y(y_)
    {
    }
    point::point(const size &sz) : x(sz.width), y(sz.height)
    {
    }
    point::point(const size_f &sz) : x(sz.width), y(sz.height)
    {
    }

    point point::offset(double dx, double dy) const
    {
        return {x + dx, y + dy};
    }
    point point::round() const
    {
        return {std::nearbyint(x), std::nearbyint(y)};
    }
    bool point::is_empty() const
    {
        return x == 0 && y == 0;
    }
    double point::distance(const point &o) const
    {
        double const dx = x - o.x;
        double const dy = y - o.y;
        return std::sqrt((dx * dx) + (dy * dy));
    }
    bool point::equals(const point &o, double epsilon) const
    {
        return std::abs(o.x - x) < epsilon && std::abs(o.y - y) < epsilon;
    }
    std::string point::to_string() const
    {
        return std::format("{{X={} Y={}}}", x, y);
    }

    bool point::try_parse(std::string_view value, point &out)
    {
        std::string_view a;
        std::string_view b;
        double px = 0;
        double py = 0;
        if (detail::ps_split2(value, a, b) && detail::ps_parse_num(a, px) && detail::ps_parse_num(b, py))
        {
            out = point(px, py);
            return true;
        }
        out = point{};
        return false;
    }

    point::operator size() const
    {
        return {x, y};
    }
    point::operator point_f() const
    {
        return {static_cast<float>(x), static_cast<float>(y)};
    }

    bool operator==(const point &a, const point &b)
    {
        return a.x == b.x && a.y == b.y;
    }
    bool operator!=(const point &a, const point &b)
    {
        return !(a == b);
    }
    point operator+(const point &pt, const size_f &sz)
    {
        return {pt.x + sz.width, pt.y + sz.height};
    }
    size operator-(const point &a, const point &b)
    {
        return {a.x - b.x, a.y - b.y};
    }
    point operator-(const point &pt, const size_f &sz)
    {
        return {pt.x - sz.width, pt.y - sz.height};
    }
}
