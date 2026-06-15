// The Brush ⇄ Paint bridge implementations (the C# Brush implicit operators) + Paint.IsNullOrEmpty.
// Header: brushes/brush_paint_bridge.hpp.

#include "maui/controls/brushes/brush_paint_bridge.hpp"

#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "maui/controls/brushes/brush.hpp"
#include "maui/controls/brushes/gradient_brush.hpp"
#include "maui/controls/brushes/gradient_stop.hpp"
#include "maui/controls/brushes/image_brush.hpp"
#include "maui/controls/brushes/linear_gradient_brush.hpp"
#include "maui/controls/brushes/radial_gradient_brush.hpp"
#include "maui/controls/brushes/solid_color_brush.hpp"
#include "maui/core/image_source_paint.hpp"
#include "maui/graphics/gradient_paint.hpp"
#include "maui/graphics/gradient_stop.hpp"
#include "maui/graphics/linear_gradient_paint.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/radial_gradient_paint.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::controls
{
    namespace
    {
        // The brush's stops → graphics gradient_stops, SKIPPING null entries and null-color stops (the
        // value-type-color deviation, documented in the header): a stop carrying C#'s null Color has no
        // value-type representation, so it is dropped. An all-null-color gradient becomes zero stops.
        std::vector<maui::graphics::gradient_stop> to_graphics_stops(const gradient_brush& brush)
        {
            std::vector<maui::graphics::gradient_stop> result;
            for (const auto& stop : brush.gradient_stops().items())
            {
                if (stop && stop->color().has_value())
                {
                    result.emplace_back(stop->offset(), *stop->color());
                }
            }
            return result;
        }
    } // namespace

    std::shared_ptr<maui::graphics::paint> to_paint(const brush& source)
    {
        // C# implicit operator Paint(Brush): per kind. solid → solid_paint; linear/radial → the gradient
        // paint with stops + geometry; image → image_source_paint (borrowing the brush's source). Any other
        // (the empty base brush) → null.
        if (const auto* solid = dynamic_cast<const solid_color_brush*>(&source))
        {
            // C#: new SolidPaint { Color = solidColorBrush.Color }. A null color leaves the paint's color at
            // its value-type default (solid_paint's default color).
            if (const auto& c = solid->color(); c.has_value())
            {
                return std::make_shared<maui::graphics::solid_paint>(*c);
            }
            return std::make_shared<maui::graphics::solid_paint>();
        }
        if (const auto* linear = dynamic_cast<const linear_gradient_brush*>(&source))
        {
            auto paint = std::make_shared<maui::graphics::linear_gradient_paint>();
            paint->set_gradient_stops(to_graphics_stops(*linear));
            paint->set_start_point(linear->start_point());
            paint->set_end_point(linear->end_point());
            return paint;
        }
        if (const auto* radial = dynamic_cast<const radial_gradient_brush*>(&source))
        {
            auto paint = std::make_shared<maui::graphics::radial_gradient_paint>();
            paint->set_gradient_stops(to_graphics_stops(*radial));
            paint->set_center(radial->center());
            paint->set_radius(radial->radius());
            return paint;
        }
        if (const auto* image = dynamic_cast<const image_brush*>(&source))
        {
            // C#: new ImageSourcePaint { ImageSource = imageBrush.ImageSource }. image_source_paint borrows
            // the source (the brush owns its lifetime).
            return std::make_shared<maui::core::image_source_paint>(image->image_source().get());
        }
        return nullptr;
    }

    std::shared_ptr<maui::graphics::paint> to_paint(const std::shared_ptr<brush>& source)
    {
        if (!source)
        {
            return nullptr;
        }
        return to_paint(*source);
    }

    std::shared_ptr<brush> to_brush(const maui::graphics::paint& source)
    {
        // C# implicit operator Brush(Paint): the reverse mapping.
        if (const auto* solid = dynamic_cast<const maui::graphics::solid_paint*>(&source))
        {
            return std::make_shared<solid_color_brush>(solid->color());
        }
        if (const auto* linear = dynamic_cast<const maui::graphics::linear_gradient_paint*>(&source))
        {
            std::vector<std::shared_ptr<gradient_stop>> stops;
            for (const auto& gs : linear->gradient_stops())
            {
                stops.push_back(std::make_shared<gradient_stop>(gs.color(), gs.offset()));
            }
            return std::make_shared<linear_gradient_brush>(std::move(stops), linear->start_point(),
                                                           linear->end_point());
        }
        if (const auto* radial = dynamic_cast<const maui::graphics::radial_gradient_paint*>(&source))
        {
            std::vector<std::shared_ptr<gradient_stop>> stops;
            for (const auto& gs : radial->gradient_stops())
            {
                stops.push_back(std::make_shared<gradient_stop>(gs.color(), gs.offset()));
            }
            return std::make_shared<radial_gradient_brush>(std::move(stops), radial->center(), radial->radius());
        }
        if (dynamic_cast<const maui::core::image_source_paint*>(&source) != nullptr)
        {
            // C#: new ImageBrush { ImageSource = imageSourcePaint.ImageSource }. image_source_paint borrows
            // its source raw; there is no owning shared handle to adopt here, so the round-trip from a paint
            // produces an empty image_brush (the paint→brush direction is not exercised for image fills —
            // documented). The owning brush→paint direction carries the source faithfully.
            return std::make_shared<image_brush>();
        }
        return nullptr;
    }

    std::shared_ptr<brush> to_brush(const maui::graphics::paint* source)
    {
        if (source == nullptr)
        {
            return nullptr;
        }
        return to_brush(*source);
    }

    bool paint_is_null_or_empty(const maui::graphics::paint* paint)
    {
        // PaintExtensions.IsNullOrEmpty, per kind (see header for the gradient value-type-color note).
        if (paint == nullptr)
        {
            return true;
        }
        if (const auto* solid = dynamic_cast<const maui::graphics::solid_paint*>(paint))
        {
            // C#: solidPaint.Color is null. The bridge maps a null brush color to solid_paint's default
            // (opaque black); the port has no null marker on a value-type color, so a solid_paint is treated
            // as non-empty (it always has a usable color). DEVIATION (documented): a solid brush built from a
            // null color is reported non-empty by this paint check — but Brush.is_empty() (the controls-level
            // surface used by VisualElement.IView.Background) still reports it empty via the nullable color.
            (void)solid;
            return false;
        }
        if (const auto* gradient = dynamic_cast<const maui::graphics::gradient_paint*>(paint))
        {
            return gradient->gradient_stops().empty();
        }
        if (const auto* image = dynamic_cast<const maui::core::image_source_paint*>(paint))
        {
            return image->image_source() == nullptr;
        }
        return false;
    }

    bool brush_is_null_or_empty_as_paint(const brush* source)
    {
        // `Paint p = brush; p.IsNullOrEmpty()` over the brush's nullable data (PaintExtensions.IsNullOrEmpty
        // per kind), so the null-color gradient cases match C# (the value-type graphics::paint cannot carry a
        // null color, hence this brush-level computation).
        if (source == nullptr)
        {
            return true;
        }
        if (const auto* solid = dynamic_cast<const solid_color_brush*>(source))
        {
            return !solid->color().has_value(); // C#: solidPaint.Color is null
        }
        if (const auto* gradient = dynamic_cast<const gradient_brush*>(source))
        {
            // C#: no stops OR StartColor/EndColor is null. The port's StartColor/EndColor map to the
            // lowest/highest-offset stop colors; if EVERY stop has a null color (or there are none), both are
            // null → empty. A single non-null-color stop makes it non-empty (matches the "Red, null, Blue"
            // oracle, where the bridge keeps Red/Blue).
            bool any_color = false;
            for (const auto& stop : gradient->gradient_stops().items())
            {
                if (stop && stop->color().has_value())
                {
                    any_color = true;
                    break;
                }
            }
            return !any_color;
        }
        if (const auto* image = dynamic_cast<const image_brush*>(source))
        {
            return image->image_source() == nullptr;
        }
        return false;
    }
} // namespace maui::controls
