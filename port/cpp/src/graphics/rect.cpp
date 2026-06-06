// maui::graphics::rect  <=  Microsoft.Maui.Graphics.Rect
#include "maui/graphics/rect.hpp"

#include <algorithm>
#include <cmath>
#include <format>
#include <string>
#include <string_view>

#include "maui/graphics/point.hpp"
#include "maui/graphics/rect_f.hpp"
#include "maui/graphics/size.hpp"

#include "detail/parse_util.hpp"

namespace maui::graphics
{
    const rect rect::zero{};

    rect::rect(double x_, double y_, double w, double h) : x(x_), y(y_), width(w), height(h)
    {
    }
    rect::rect(const point &loc, const ::maui::graphics::size &sz)
        : x(loc.x), y(loc.y), width(sz.width), height(sz.height)
    {
    }

    rect rect::from_ltrb(double left_, double top_, double right_, double bottom_)
    {
        return {left_, top_, right_ - left_, bottom_ - top_};
    }

    bool rect::equals(const rect &o) const
    {
        return x == o.x && y == o.y && width == o.width && height == o.height;
    }

    double rect::left() const
    {
        return x;
    }
    double rect::top() const
    {
        return y;
    }
    double rect::right() const
    {
        return x + width;
    }
    double rect::bottom() const
    {
        return y + height;
    }
    void rect::set_left(double v)
    {
        x = v;
    }
    void rect::set_top(double v)
    {
        y = v;
    }
    void rect::set_right(double v)
    {
        width = v - x;
    }
    void rect::set_bottom(double v)
    {
        height = v - y;
    }

    bool rect::is_empty() const
    {
        return width <= 0 || height <= 0;
    }

    ::maui::graphics::size rect::size() const
    {
        return {width, height};
    }
    void rect::set_size(const ::maui::graphics::size &v)
    {
        width = v.width;
        height = v.height;
    }
    point rect::location() const
    {
        return {x, y};
    }
    void rect::set_location(const point &v)
    {
        x = v.x;
        y = v.y;
    }
    point rect::center() const
    {
        return {x + (width / 2), y + (height / 2)};
    }

    bool rect::contains(const rect &r) const
    {
        return x <= r.x && right() >= r.right() && y <= r.y && bottom() >= r.bottom();
    }
    bool rect::contains(const point &pt) const
    {
        return contains(pt.x, pt.y);
    }
    bool rect::contains(double px, double py) const
    {
        return px >= left() && px < right() && py >= top() && py < bottom();
    }
    bool rect::intersects_with(const rect &r) const
    {
        // Mirrors Rect.IntersectsWith: !(any separating-axis test). Kept as the explicit negation
        // (not DeMorgan-distributed) so the NaN semantics match the C# original exactly.
        bool const separated = left() >= r.right() || right() <= r.left() || top() >= r.bottom() || bottom() <= r.top();
        return !separated;
    }

    rect rect::union_with(const rect &r) const
    {
        return union_with(*this, r);
    }
    rect rect::union_with(const rect &r1, const rect &r2)
    {
        return from_ltrb(std::min(r1.left(), r2.left()), std::min(r1.top(), r2.top()), std::max(r1.right(), r2.right()),
                         std::max(r1.bottom(), r2.bottom()));
    }
    rect rect::intersect(const rect &r) const
    {
        return intersect(*this, r);
    }
    rect rect::intersect(const rect &r1, const rect &r2)
    {
        double const ix = std::max(r1.x, r2.x);
        double const iy = std::max(r1.y, r2.y);
        double const iw = std::min(r1.right(), r2.right()) - ix;
        double const ih = std::min(r1.bottom(), r2.bottom()) - iy;
        if (iw < 0 || ih < 0)
        {
            return zero;
        }
        return {ix, iy, iw, ih};
    }

    rect rect::inflate(const ::maui::graphics::size &sz) const
    {
        return inflate(sz.width, sz.height);
    }
    rect rect::inflate(double w, double h) const
    {
        return {x - w, y - h, width + (w * 2), height + (h * 2)};
    }
    rect rect::offset(double dx, double dy) const
    {
        return {x + dx, y + dy, width, height};
    }
    rect rect::offset(const point &dr) const
    {
        return offset(dr.x, dr.y);
    }
    rect rect::round() const
    {
        return {std::nearbyint(x), std::nearbyint(y), std::nearbyint(width), std::nearbyint(height)};
    }

    std::string rect::to_string() const
    {
        return std::format("{{X={} Y={} Width={} Height={}}}", x, y, width, height);
    }

    bool rect::try_parse(std::string_view value, rect &out)
    {
        std::string_view a;
        std::string_view b;
        std::string_view c;
        std::string_view d;
        double px = 0;
        double py = 0;
        double pw = 0;
        double ph = 0;
        if (detail::ps_split4(value, a, b, c, d) && detail::ps_parse_num(a, px) && detail::ps_parse_num(b, py) &&
            detail::ps_parse_num(c, pw) && detail::ps_parse_num(d, ph))
        {
            out = rect(px, py, pw, ph);
            return true;
        }
        out = rect{};
        return false;
    }

    rect::operator rect_f() const
    {
        return {static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height)};
    }

    bool operator==(const rect &a, const rect &b)
    {
        return a.equals(b);
    }
    bool operator!=(const rect &a, const rect &b)
    {
        return !a.equals(b);
    }
} // namespace maui::graphics
