// maui::controls::shapes::shape — out-of-line definitions: the shared descriptors (Shape.*Property
// defaults), PathForBounds + TransformPathForBounds (the aspect fitting over matrix3x2), the
// path-computation fallbacks, and MeasureOverride. See shape.hpp.

#include "maui/controls/shapes/shape.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

#include "maui/core/bindable_property.hpp"
#include "maui/core/path_aspect.hpp"
#include "maui/graphics/line_cap.hpp"
#include "maui/graphics/line_join.hpp"
#include "maui/graphics/matrix3x2.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/rect_f.hpp"
#include "maui/graphics/size.hpp"

namespace maui::controls::shapes
{
    const maui::core::bindable_property<std::shared_ptr<maui::graphics::paint>>& shape::fill_property()
    {
        // C# Shape.FillProperty default: null.
        static const maui::core::bindable_property<std::shared_ptr<maui::graphics::paint>> descriptor{"fill"};
        return descriptor;
    }

    const maui::core::bindable_property<std::shared_ptr<maui::graphics::paint>>& shape::stroke_property()
    {
        // C# Shape.StrokeProperty default: null.
        static const maui::core::bindable_property<std::shared_ptr<maui::graphics::paint>> descriptor{"stroke"};
        return descriptor;
    }

    const maui::core::bindable_property<double>& shape::stroke_thickness_property()
    {
        // C# Shape.StrokeThicknessProperty default: 1.0.
        static const maui::core::bindable_property<double> descriptor{"stroke_thickness", 1.0};
        return descriptor;
    }

    const maui::core::bindable_property<std::vector<double>>& shape::stroke_dash_array_property()
    {
        // C# Shape.StrokeDashArrayProperty defaultValueCreator: an empty DoubleCollection.
        static const maui::core::bindable_property<std::vector<double>> descriptor{"stroke_dash_array"};
        return descriptor;
    }

    const maui::core::bindable_property<double>& shape::stroke_dash_offset_property()
    {
        // C# Shape.StrokeDashOffsetProperty default: 0.0.
        static const maui::core::bindable_property<double> descriptor{"stroke_dash_offset", 0.0};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::line_cap>& shape::stroke_line_cap_property()
    {
        // C# Shape.StrokeLineCapProperty default: PenLineCap.Flat (butt).
        static const maui::core::bindable_property<maui::graphics::line_cap> descriptor{"stroke_line_cap",
                                                                                        maui::graphics::line_cap::butt};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::line_join>& shape::stroke_line_join_property()
    {
        // C# Shape.StrokeLineJoinProperty default: PenLineJoin.Miter.
        static const maui::core::bindable_property<maui::graphics::line_join> descriptor{
            "stroke_line_join", maui::graphics::line_join::miter};
        return descriptor;
    }

    const maui::core::bindable_property<double>& shape::stroke_miter_limit_property()
    {
        // C# Shape.StrokeMiterLimitProperty default: 10.0.
        static const maui::core::bindable_property<double> descriptor{"stroke_miter_limit", 10.0};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::path_aspect>& shape::aspect_property()
    {
        // C# Shape.AspectProperty default: Stretch.None (→ path_aspect::none; the header collapse).
        static const maui::core::bindable_property<maui::core::path_aspect> descriptor{"aspect",
                                                                                       maui::core::path_aspect::none};
        return descriptor;
    }

    // C# Shape's explicit IShape.PathForBounds: capture the fallback extents, build the path, fit it.
    maui::graphics::path_f shape::path_for_bounds(const maui::graphics::rect& bounds) const
    {
        fallback_height_ = bounds.height;
        fallback_width_ = bounds.width;

        maui::graphics::path_f path = get_path();
        transform_path_for_bounds(path, bounds);
        return path;
    }

    // C# Shape.TransformPathForBounds (the platform branch — the port always renders).
    void shape::transform_path_for_bounds(maui::graphics::path_f& path, const maui::graphics::rect& view_bounds) const
    {
        // "not using GetPath().Bounds since GetBoundsByFlattening(0.001) returns incorrect results
        // for curves" — the C# comment; flatness 1 like the source.
        const maui::graphics::rect_f path_bounds = path.get_bounds_by_flattening(1);

        maui::graphics::rect bounds = view_bounds;
        const double thickness = stroke_thickness();
        bounds.x += thickness / 2;
        bounds.y += thickness / 2;
        bounds.width -= thickness;
        bounds.height -= thickness;

        maui::graphics::matrix3x2 transform = maui::graphics::matrix3x2::identity();

        if (aspect() == maui::core::path_aspect::none)
        {
            float translate_x = 0;
            float translate_y = 0;

            // shift right/left to fit within the view bounds
            if (bounds.left() > path_bounds.left())
            {
                translate_x = static_cast<float>(bounds.left() - path_bounds.left());
            }
            else if (path_bounds.right() > bounds.right())
            {
                translate_x = static_cast<float>(bounds.right() - path_bounds.right());
            }

            // shift down/up to fit within the view bounds
            if (bounds.top() > path_bounds.top())
            {
                translate_y = static_cast<float>(bounds.top() - path_bounds.top());
            }
            else if (path_bounds.bottom() > bounds.bottom())
            {
                translate_y = static_cast<float>(bounds.bottom() - path_bounds.bottom());
            }

            if (translate_x != 0 || translate_y != 0)
            {
                transform = maui::graphics::matrix3x2::create_translation(translate_x, translate_y);
            }
        }
        else
        {
            const auto calculated_width = static_cast<float>(bounds.width / path_bounds.width);
            const auto calculated_height = static_cast<float>(bounds.height / path_bounds.height);

            const float width_scale =
                std::isnan(calculated_width) || std::isinf(calculated_width) ? 0 : calculated_width;
            const float height_scale =
                std::isnan(calculated_height) || std::isinf(calculated_height) ? 0 : calculated_height;

            switch (aspect())
            {
                case maui::core::path_aspect::stretch: // C# Stretch.Fill
                    transform = transform * maui::graphics::matrix3x2::create_scale(width_scale, height_scale);
                    transform = transform * maui::graphics::matrix3x2::create_translation(
                                                static_cast<float>(bounds.left() - (width_scale * path_bounds.left())),
                                                static_cast<float>(bounds.top() - (height_scale * path_bounds.top())));
                    break;

                case maui::core::path_aspect::aspect_fit: // C# Stretch.Uniform
                {
                    const float min_scale = std::min(width_scale, height_scale);
                    transform = transform * maui::graphics::matrix3x2::create_scale(min_scale, min_scale);
                    transform =
                        transform * maui::graphics::matrix3x2::create_translation(
                                        static_cast<float>(bounds.left() - (min_scale * path_bounds.left()) +
                                                           ((bounds.width - (min_scale * path_bounds.width)) / 2)),
                                        static_cast<float>(bounds.top() - (min_scale * path_bounds.top()) +
                                                           ((bounds.height - (min_scale * path_bounds.height)) / 2)));
                    break;
                }

                case maui::core::path_aspect::aspect_fill: // C# Stretch.UniformToFill
                {
                    const float max_scale = std::max(width_scale, height_scale);
                    transform = transform * maui::graphics::matrix3x2::create_scale(max_scale, max_scale);
                    transform = transform * maui::graphics::matrix3x2::create_translation(
                                                static_cast<float>(bounds.left() - (max_scale * path_bounds.left())),
                                                static_cast<float>(bounds.top() - (max_scale * path_bounds.top())));
                    break;
                }

                case maui::core::path_aspect::none:
                case maui::core::path_aspect::center:
                default:
                    break;
            }
        }

        if (transform != maui::graphics::matrix3x2::identity())
        {
            path.transform(transform);
        }
    }

    double shape::width_for_path_computation() const
    {
        // C#: Width == -1 (never arranged) → the PathForBounds fallback (header deviation note:
        // the port's unarranged frame extent is 0).
        const double width = frame().width;
        return width <= 0 ? fallback_width_ : width;
    }

    double shape::height_for_path_computation() const
    {
        const double height = frame().height;
        return height <= 0 ? fallback_height_ : height;
    }

    // C# Shape.MeasureOverride: the handler reports nothing (shape_view_handler), so the flattened
    // path bounds + the aspect scaling produce the content size; the standard size-request resolve
    // (the view<>::measure tail) then applies WidthRequest/Minimum*/Maximum*.
    maui::graphics::size shape::measure(double width_constraint, double height_constraint)
    {
        const maui::graphics::rect_f path_bounds = get_path().get_bounds_by_flattening(1);

        maui::graphics::size result{path_bounds.width, path_bounds.height};

        const double thickness = stroke_thickness();
        width_constraint -= thickness;
        height_constraint -= thickness;

        double scale_x = width_constraint / result.width;
        double scale_y = height_constraint / result.height;
        scale_x = std::isnan(scale_x) || std::isinf(scale_x) ? 0 : scale_x;
        scale_y = std::isnan(scale_y) || std::isinf(scale_y) ? 0 : scale_y;

        switch (aspect())
        {
            case maui::core::path_aspect::none:
                result.height += path_bounds.y;
                result.width += path_bounds.x;
                break;

            case maui::core::path_aspect::stretch: // C# Stretch.Fill — HeightRequest wins over the constraint
                if (!std::isinf(height_constraint) || height() > 0)
                {
                    result.height = height() < 0 ? height_constraint : height();
                }
                if (!std::isinf(width_constraint) || width() > 0)
                {
                    result.width = width() < 0 ? width_constraint : width();
                }
                break;

            case maui::core::path_aspect::aspect_fit: // C# Stretch.Uniform
            {
                const double min_scale = std::min(scale_x, scale_y);
                if (!std::isinf(min_scale))
                {
                    result.height *= min_scale;
                    result.width *= min_scale;
                }
                break;
            }

            case maui::core::path_aspect::aspect_fill: // C# Stretch.UniformToFill
            {
                const double max_scale = std::max(scale_x, scale_y);
                if (max_scale != 0)
                {
                    result.height *= max_scale;
                    result.width *= max_scale;
                }
                break;
            }

            case maui::core::path_aspect::center:
            default:
                break;
        }

        result.height += thickness;
        result.width += thickness;
        if (adds_margin_to_measure())
        {
            // C#: Line/Path/Polyline add the margin back; the port's margin is always zero today
            // (view<>::margin returns {}), so this is shape-preserving only.
            result.height += margin().vertical_thickness();
            result.width += margin().horizontal_thickness();
        }

        desired_size_ = {resolve_size_request(result.width, width(), minimum_width(), maximum_width()),
                         resolve_size_request(result.height, height(), minimum_height(), maximum_height())};
        return desired_size_;
    }
} // namespace maui::controls::shapes
