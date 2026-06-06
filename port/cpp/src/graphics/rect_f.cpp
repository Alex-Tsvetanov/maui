// maui::graphics::rect_f  <=  Microsoft.Maui.Graphics.RectF
#include "maui/graphics/rect_f.hpp"

#include <algorithm>
#include <cmath>
#include <format>
#include <string>
#include <string_view>

#include "maui/graphics/point_f.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size_f.hpp"

#include "detail/parse_util.hpp"

namespace maui::graphics
{
    const rect_f rect_f::zero{};

    rect_f::rect_f(float x_, float y_, float w, float h) : x(x_), y(y_), width(w), height(h)
    {
    }
    rect_f::rect_f(const point_f &loc, const size_f &sz) : x(loc.x), y(loc.y), width(sz.width), height(sz.height)
    {
    }

    rect_f rect_f::from_ltrb(float left_, float top_, float right_, float bottom_)
    {
        return {left_, top_, right_ - left_, bottom_ - top_};
    }

    bool rect_f::equals(const rect_f &o) const
    {
        return x == o.x && y == o.y && width == o.width && height == o.height;
    }

    float rect_f::left() const
    {
        return x;
    }
    float rect_f::top() const
    {
        return y;
    }
    float rect_f::right() const
    {
        return x + width;
    }
    float rect_f::bottom() const
    {
        return y + height;
    }
    void rect_f::set_left(float v)
    {
        x = v;
    }
    void rect_f::set_top(float v)
    {
        y = v;
    }
    void rect_f::set_right(float v)
    {
        width = v - x;
    }
    void rect_f::set_bottom(float v)
    {
        height = v - y;
    }

    bool rect_f::is_empty() const
    {
        return width <= 0 || height <= 0;
    }

    size_f rect_f::size() const
    {
        return {width, height};
    }
    void rect_f::set_size(const size_f &v)
    {
        width = v.width;
        height = v.height;
    }
    point_f rect_f::location() const
    {
        return {x, y};
    }
    void rect_f::set_location(const point_f &v)
    {
        x = v.x;
        y = v.y;
    }
    point_f rect_f::center() const
    {
        return {x + (width / 2), y + (height / 2)};
    }

    bool rect_f::contains(const rect_f &r) const
    {
        return x <= r.x && right() >= r.right() && y <= r.y && bottom() >= r.bottom();
    }
    bool rect_f::contains(const point_f &pt) const
    {
        return contains(pt.x, pt.y);
    }
    bool rect_f::contains(float px, float py) const
    {
        return px >= left() && px < right() && py >= top() && py < bottom();
    }
    bool rect_f::intersects_with(const rect_f &r) const
    {
        // Mirrors RectF.IntersectsWith: !(any separating-axis test). Kept as the explicit negation
        // (not DeMorgan-distributed) so the NaN semantics match the C# original exactly.
        bool const separated = left() >= r.right() || right() <= r.left() || top() >= r.bottom() || bottom() <= r.top();
        return !separated;
    }

    rect_f rect_f::union_with(const rect_f &r) const
    {
        return union_with(*this, r);
    }
    rect_f rect_f::union_with(const rect_f &r1, const rect_f &r2)
    {
        return from_ltrb(std::min(r1.left(), r2.left()), std::min(r1.top(), r2.top()), std::max(r1.right(), r2.right()),
                         std::max(r1.bottom(), r2.bottom()));
    }
    rect_f rect_f::intersect(const rect_f &r) const
    {
        return intersect(*this, r);
    }
    rect_f rect_f::intersect(const rect_f &r1, const rect_f &r2)
    {
        float const ix = std::max(r1.x, r2.x);
        float const iy = std::max(r1.y, r2.y);
        float const iw = std::min(r1.right(), r2.right()) - ix;
        float const ih = std::min(r1.bottom(), r2.bottom()) - iy;
        if (iw < 0 || ih < 0)
        {
            return zero;
        }
        return {ix, iy, iw, ih};
    }

    rect_f rect_f::inflate(const size_f &sz) const
    {
        return inflate(sz.width, sz.height);
    }
    rect_f rect_f::inflate(float w, float h) const
    {
        return {x - w, y - h, width + (w * 2), height + (h * 2)};
    }
    rect_f rect_f::offset(float dx, float dy) const
    {
        return {x + dx, y + dy, width, height};
    }
    rect_f rect_f::offset(const point_f &dr) const
    {
        return offset(dr.x, dr.y);
    }
    rect_f rect_f::round() const
    {
        return {std::nearbyint(x), std::nearbyint(y), std::nearbyint(width), std::nearbyint(height)};
    }

    std::string rect_f::to_string() const
    {
        return std::format("{{X={} Y={} Width={} Height={}}}", x, y, width, height);
    }

    bool rect_f::try_parse(std::string_view value, rect_f &out)
    {
        std::string_view a;
        std::string_view b;
        std::string_view c;
        std::string_view d;
        float px = 0;
        float py = 0;
        float pw = 0;
        float ph = 0;
        if (detail::ps_split4(value, a, b, c, d) && detail::ps_parse_num(a, px) && detail::ps_parse_num(b, py) &&
            detail::ps_parse_num(c, pw) && detail::ps_parse_num(d, ph))
        {
            out = rect_f(px, py, pw, ph);
            return true;
        }
        out = rect_f{};
        return false;
    }

    rect_f::operator rect() const
    {
        return {x, y, width, height};
    }

    bool operator==(const rect_f &a, const rect_f &b)
    {
        return a.equals(b);
    }
    bool operator!=(const rect_f &a, const rect_f &b)
    {
        return !a.equals(b);
    }
} // namespace maui::graphics
