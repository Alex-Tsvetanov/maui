// maui::core::shadow — out-of-line definitions. See shadow.hpp. The concrete IShadow with the Shadow.cs
// defaults (radius 10, opacity 1, black paint, zero offset).

#include "maui/core/shadow.hpp"

#include <memory>

#include "maui/graphics/color.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::core
{
    shadow::shadow() : paint_(std::make_shared<maui::graphics::solid_paint>(maui::graphics::color{}))
    {
    }

    double shadow::radius() const
    {
        return radius_;
    }

    double shadow::opacity() const
    {
        return opacity_;
    }

    maui::graphics::paint* shadow::paint() const
    {
        return paint_.get();
    }

    maui::graphics::point shadow::offset() const
    {
        return offset_;
    }

    void shadow::set_radius(double value)
    {
        radius_ = value;
    }

    void shadow::set_opacity(double value)
    {
        opacity_ = value;
    }

    void shadow::set_color(maui::graphics::color value)
    {
        // Setting the color replaces the colorizing brush with a solid paint (mirrors assigning Brush).
        paint_ = std::make_shared<maui::graphics::solid_paint>(value);
    }

    void shadow::set_offset(maui::graphics::point value)
    {
        offset_ = value;
    }
} // namespace maui::core
