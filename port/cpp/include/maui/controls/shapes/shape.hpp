#pragma once
// maui::controls::shapes::shape  <=  Microsoft.Maui.Controls.Shapes.Shape
//
// The base of the shape CONTROLS (rectangle/ellipse/line/polyline/polygon/path): a View that is both
// an IShapeView (rendered by shape_view_handler through the canvas stack) and an IShape (usable as
// any view's clip). Ported from Shape.cs: the Fill/Stroke paints, StrokeThickness/DashArray/Offset,
// LineCap/LineJoin/MiterLimit, Aspect, the abstract GetPath(), PathForBounds (fallback size capture
// + TransformPathForBounds — the aspect fitting math over matrix3x2), and MeasureOverride (the
// path-bounds-driven sizing).
//
// PORT COLLAPSES (documented, not stubbed — the border precedents):
//   - PenLineCap/PenLineJoin are exposed as maui::graphics::line_cap/line_join (Flat = butt).
//   - The Controls Stretch enum collapses onto maui::core::path_aspect — the exact value mapping
//     C#'s IShapeView.Aspect performs (None→none, Fill→stretch, Uniform→aspect_fit,
//     UniformToFill→aspect_fill); the port stores the mapped value directly.
//   - StrokeDashArray is a plain std::vector<double> property; stroke_dash_pattern() materializes
//     the float[] per read, exactly like Shape.StrokeDashPattern.
//   - The WeakBrushChangedProxy resubscription on Fill/Stroke (re-render when the brush object
//     mutates internally) and the IVersionedShape version counter are not modeled — set a new
//     shared_ptr to retrigger the mapper (the geometry.hpp invalidation rule).
//
// detail::shape_contract: C++ forbids a member function sharing its class name, so the
// i_shape_view::shape() override lives on this thin seat the control derives (the structural sibling
// of the i_button send_* naming precedent); the ctor points it at the control's own i_shape face.
//
// width/height_for_path_computation: C# falls back to the PathForBounds bounds while Width/Height
// are still -1 (never arranged); the port's arranged frame defaults to 0x0, so a non-positive frame
// extent selects the fallback (deviation note).

#include <memory>
#include <utility>
#include <vector>

#include "maui/controls/brushes/brush.hpp" // X1: Shape.Fill/Stroke accept a Brush (bridged to paint)
#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/i_shape_view.hpp"
#include "maui/core/path_aspect.hpp"
#include "maui/core/property.hpp"
#include "maui/graphics/i_shape.hpp"
#include "maui/graphics/line_cap.hpp"
#include "maui/graphics/line_join.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::controls::shapes
{
    namespace detail
    {
        // The seat for the i_shape_view::shape() override (header note).
        class shape_contract : public maui::controls::view<maui::core::i_shape_view>
        {
        public:
            [[nodiscard]] maui::graphics::i_shape* shape() const final
            {
                return shape_self_;
            }

        protected:
            maui::graphics::i_shape* shape_self_ = nullptr; // the control's own i_shape face
        };
    } // namespace detail

    class shape : public detail::shape_contract, public maui::graphics::i_shape
    {
    public:
        // Shared bindable-property descriptors (one instance for the whole family, like Shape.*Property).
        static const maui::core::bindable_property<std::shared_ptr<maui::graphics::paint>>& fill_property();
        static const maui::core::bindable_property<std::shared_ptr<maui::graphics::paint>>& stroke_property();
        static const maui::core::bindable_property<double>& stroke_thickness_property();
        static const maui::core::bindable_property<std::vector<double>>& stroke_dash_array_property();
        static const maui::core::bindable_property<double>& stroke_dash_offset_property();
        static const maui::core::bindable_property<maui::graphics::line_cap>& stroke_line_cap_property();
        static const maui::core::bindable_property<maui::graphics::line_join>& stroke_line_join_property();
        static const maui::core::bindable_property<double>& stroke_miter_limit_property();
        static const maui::core::bindable_property<maui::core::path_aspect>& aspect_property();

        // C# Shape.GetPath() — the shape's own geometry (before aspect fitting).
        [[nodiscard]] virtual maui::graphics::path_f get_path() const = 0;

        // ---- Fill / Stroke (owned paints; i_shape_view returns the raw borrows) ----
        [[nodiscard]] maui::graphics::paint* fill() const override
        {
            return fill_.get().get();
        }
        void set_fill(std::shared_ptr<maui::graphics::paint> value)
        {
            fill_brush_.reset(); // a raw-paint fill clears any brush previously set
            fill_.set(std::move(value));
        }
        [[nodiscard]] maui::graphics::paint* stroke() const override
        {
            return stroke_.get().get();
        }
        void set_stroke(std::shared_ptr<maui::graphics::paint> value)
        {
            stroke_brush_.reset();
            stroke_.set(std::move(value));
        }

        // X1 — Shape.Fill / Shape.Stroke as Brushes (the C# bindable surface). The shape owns the brush for
        // BindingContext inheritance and bridges it to the graphics::paint the shape_view_handler renders
        // (Brush's implicit operator Paint). Named set_fill_brush / set_stroke_brush (not overloads) so the
        // existing set_fill(nullptr)/(paint) calls stay unambiguous — non-breaking: the paint-typed setters
        // above are untouched.
        void set_fill_brush(std::shared_ptr<brush> value)
        {
            fill_brush_ = std::move(value);
            if (fill_brush_)
            {
                fill_brush_->set_inherited_binding_context(this->raw_binding_context());
            }
            fill_.set(maui::controls::to_paint(fill_brush_));
        }
        void set_stroke_brush(std::shared_ptr<brush> value)
        {
            stroke_brush_ = std::move(value);
            if (stroke_brush_)
            {
                stroke_brush_->set_inherited_binding_context(this->raw_binding_context());
            }
            stroke_.set(maui::controls::to_paint(stroke_brush_));
        }
        [[nodiscard]] const std::shared_ptr<brush>& fill_brush() const
        {
            return fill_brush_;
        }
        [[nodiscard]] const std::shared_ptr<brush>& stroke_brush() const
        {
            return stroke_brush_;
        }

        // ---- the stroke detail surface (i_stroke) ----
        [[nodiscard]] double stroke_thickness() const override
        {
            return stroke_thickness_.get();
        }
        void set_stroke_thickness(double value)
        {
            stroke_thickness_.set(value);
        }
        [[nodiscard]] const std::vector<double>& stroke_dash_array() const
        {
            return stroke_dash_array_.get();
        }
        void set_stroke_dash_array(std::vector<double> value)
        {
            stroke_dash_array_.set(std::move(value));
        }
        // C# Shape.StrokeDashPattern: the float[] materialized from the dash array on every read.
        [[nodiscard]] std::vector<float> stroke_dash_pattern() const override
        {
            const std::vector<double>& dashes = stroke_dash_array_.get();
            return {dashes.begin(), dashes.end()};
        }
        [[nodiscard]] double stroke_dash_offset_value() const
        {
            return stroke_dash_offset_.get();
        }
        [[nodiscard]] float stroke_dash_offset() const override
        {
            return static_cast<float>(stroke_dash_offset_.get());
        }
        void set_stroke_dash_offset(double value)
        {
            stroke_dash_offset_.set(value);
        }
        [[nodiscard]] maui::graphics::line_cap stroke_line_cap() const override
        {
            return stroke_line_cap_.get();
        }
        void set_stroke_line_cap(maui::graphics::line_cap value)
        {
            stroke_line_cap_.set(value);
        }
        [[nodiscard]] maui::graphics::line_join stroke_line_join() const override
        {
            return stroke_line_join_.get();
        }
        void set_stroke_line_join(maui::graphics::line_join value)
        {
            stroke_line_join_.set(value);
        }
        [[nodiscard]] double stroke_miter_limit_value() const
        {
            return stroke_miter_limit_.get();
        }
        [[nodiscard]] float stroke_miter_limit() const override
        {
            return static_cast<float>(stroke_miter_limit_.get());
        }
        void set_stroke_miter_limit(double value)
        {
            stroke_miter_limit_.set(value);
        }

        // ---- Aspect (the Stretch → path_aspect collapse; header note) ----
        [[nodiscard]] maui::core::path_aspect aspect() const override
        {
            return aspect_.get();
        }
        void set_aspect(maui::core::path_aspect value)
        {
            aspect_.set(value);
        }

        // ---- i_shape (C# Shape's explicit IShape.PathForBounds) ----
        [[nodiscard]] maui::graphics::path_f path_for_bounds(const maui::graphics::rect& bounds) const override;

        // path_for_bounds -> transform_path_for_bounds deflates by THIS shape's own stroke_thickness()
        // (shape.cpp:110-114), which is C# Shape.TransformPathForBounds (Shape.cs:312-323) itself — so the
        // border handlers' maui::core::shape_self_inset must NOT deflate again on top of it. See the base
        // declaration in graphics/i_shape.hpp for the measurement that pinned this.
        [[nodiscard]] bool applies_own_stroke_inset() const override
        {
            return true;
        }

        // C# Shape.MeasureOverride: size from the flattened path bounds + the aspect scaling + the
        // stroke thickness, then the standard size-request resolve (the view<>::measure tail).
        maui::graphics::size measure(double width_constraint, double height_constraint) override;

    protected:
        shape()
        {
            shape_self_ = this;
        }

        // X1 — after the base (view<>) propagation (which handles the Background brush + gesture
        // recognizers), flow this shape's (possibly inherited) context into the Fill/Stroke brushes too, so
        // a brush's gradient stops inherit it. The brushes are not logical children (their Parent stays
        // null), matching the VisualElement.Background treatment.
        void on_binding_context_changed() override
        {
            maui::controls::view<maui::core::i_shape_view>::on_binding_context_changed();
            const auto& context = this->raw_binding_context();
            if (fill_brush_)
            {
                fill_brush_->set_inherited_binding_context(context);
            }
            if (stroke_brush_)
            {
                stroke_brush_->set_inherited_binding_context(context);
            }
        }

        // C# Shape.TransformPathForBounds — fit the path into the (stroke-inset) view bounds.
        void transform_path_for_bounds(maui::graphics::path_f& path, const maui::graphics::rect& view_bounds) const;

        // C# Shape.WidthForPathComputation/HeightForPathComputation (virtual — RoundRectangle
        // overrides in C#; the deviation note in the header covers the frame-vs-Width difference).
        [[nodiscard]] virtual double width_for_path_computation() const;
        [[nodiscard]] virtual double height_for_path_computation() const;

        // Whether MeasureOverride adds the margin back (C#: `this is Line or Path or Polyline`; the
        // port's margin is always zero today, so this only preserves the seam shape).
        [[nodiscard]] virtual bool adds_margin_to_measure() const
        {
            return false;
        }

    private:
        maui::core::property<std::shared_ptr<maui::graphics::paint>> fill_{*this, fill_property()};
        maui::core::property<std::shared_ptr<maui::graphics::paint>> stroke_{*this, stroke_property()};
        // X1 — the developer-facing Fill/Stroke brushes (when set via the brush overloads). Owned by the
        // shape so they inherit its BindingContext; the bridged paints live in fill_/stroke_ for the handler.
        std::shared_ptr<brush> fill_brush_;
        std::shared_ptr<brush> stroke_brush_;
        maui::core::property<double> stroke_thickness_{*this, stroke_thickness_property()};
        maui::core::property<std::vector<double>> stroke_dash_array_{*this, stroke_dash_array_property()};
        maui::core::property<double> stroke_dash_offset_{*this, stroke_dash_offset_property()};
        maui::core::property<maui::graphics::line_cap> stroke_line_cap_{*this, stroke_line_cap_property()};
        maui::core::property<maui::graphics::line_join> stroke_line_join_{*this, stroke_line_join_property()};
        maui::core::property<double> stroke_miter_limit_{*this, stroke_miter_limit_property()};
        maui::core::property<maui::core::path_aspect> aspect_{*this, aspect_property()};

        // C# Shape._fallbackWidth/_fallbackHeight — captured by PathForBounds (a const seam in the
        // port, hence mutable; the C# member writes happen inside the same call).
        mutable double fallback_width_ = 0;
        mutable double fallback_height_ = 0;
    };
} // namespace maui::controls::shapes
