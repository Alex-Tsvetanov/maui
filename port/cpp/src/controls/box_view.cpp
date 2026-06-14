// maui::controls::box_view — out-of-line definitions: the Color/CornerRadius descriptors, the Fill
// materialization, PathForBounds, the 40x40 OnMeasure, and the default-handler self-registration
// (the shared shape_view_handler). See box_view.hpp.

#include "maui/controls/box_view.hpp"

#include <memory>

#include "maui/core/bindable_property.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/shape_view_handler.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/corner_radius.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<maui::graphics::color>& box_view::color_property()
    {
        // C# ColorElement.ColorProperty default: null (the unset property<color> stands in).
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"color"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::corner_radius>& box_view::corner_radius_property()
    {
        // C# CornerElement.CornerRadiusProperty default: new CornerRadius().
        static const maui::core::bindable_property<maui::graphics::corner_radius> descriptor{"corner_radius"};
        return descriptor;
    }

    // C# IShapeView.Fill => Color?.AsPaint().
    maui::graphics::paint* box_view::fill() const
    {
        if (!color_.is_set())
        {
            return nullptr;
        }
        if (fill_cache_ == nullptr)
        {
            fill_cache_ = std::make_shared<maui::graphics::solid_paint>(color_.get());
        }
        else
        {
            fill_cache_->set_color(color_.get());
        }
        return fill_cache_.get();
    }

    // C# IShape.PathForBounds: AppendRoundedRectangle over the bounds with the four corner radii.
    maui::graphics::path_f box_view::path_for_bounds(const maui::graphics::rect& bounds) const
    {
        maui::graphics::path_f path;
        const maui::graphics::corner_radius radius = corner_radius();
        path.append_rounded_rectangle(static_cast<float>(bounds.x), static_cast<float>(bounds.y),
                                      static_cast<float>(bounds.width), static_cast<float>(bounds.height),
                                      static_cast<float>(radius.top_left), static_cast<float>(radius.top_right),
                                      static_cast<float>(radius.bottom_left), static_cast<float>(radius.bottom_right));
        return path;
    }

    // C# BoxView.OnMeasure: SizeRequest(Size(40, 40)) — resolved against the explicit requests like
    // every leaf measure (the view<>::measure tail).
    maui::graphics::size box_view::measure(double /*width_constraint*/, double /*height_constraint*/)
    {
        desired_size_ = {resolve_size_request(40, width(), minimum_width(), maximum_width()),
                         resolve_size_request(40, height(), minimum_height(), maximum_height())};
        return desired_size_;
    }
} // namespace maui::controls

// Self-register the shared shape handler for box_view (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::box_view, maui::core::shape_view_handler)
