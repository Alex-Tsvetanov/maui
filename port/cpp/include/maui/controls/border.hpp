#pragma once
// maui::controls::border  <=  Microsoft.Maui.Controls.Border
//
// A container control that draws a border (and clips its content) around a single child. Ported from
// src/Controls/src/Core/Border/Border.cs (Border : View, IContentView, IBorderView, IPaddingElement):
// Content + Padding plus the stroke surface — Stroke (brush), StrokeThickness, StrokeShape,
// StrokeDashArray/Offset, StrokeLineCap/LineJoin, StrokeMiterLimit — with the C# defaults (thickness 1,
// shape Rectangle, cap Flat→butt, join Miter, miter limit 10, empty dash array).
//
// API shape: bare-noun i_border_view getters + method accessors over private property<T> engines —
// EXCEPT Content, which is a NON-OWNING raw pointer routed through the "set_content" command (the
// content_page recipe; the caller owns the content's lifetime, PROFILE §8). The stroke brush and shape
// are owned via shared_ptr (the i_view background/clip ownership rule).
//
// PORT COLLAPSES (documented, not stubbed):
//   - C# Border exposes PenLineCap/PenLineJoin and converts to Graphics' LineCap/LineJoin for IStroke;
//     the port exposes maui::graphics::line_cap/line_join directly (the same value sets; Flat = butt).
//   - StrokeDashArray is a plain std::vector<double> property; stroke_dash_pattern() materializes the
//     float[] on each read exactly as Border.StrokeDashPattern does. The DoubleCollection
//     INotifyCollectionChanged resubscription disappears (replace the vector to change the pattern).
//   - The WeakNotifyPropertyChangedProxy subscriptions on Stroke/StrokeShape (re-map when the brush or
//     shape object mutates INTERNALLY) are not modeled — set a new shared_ptr to retrigger the mapper.
//
// measure/arrange port Border.CrossPlatformMeasure/CrossPlatformArrange: measure insets the content by
// Padding + StrokeThickness; arrange insets the bounds by StrokeThickness and lets ArrangeContent apply
// the Padding within it. The protected ctor lets `frame` substitute its own Padding descriptor (default
// 20) while reusing the whole machinery — see frame.hpp.

#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "maui/controls/element.hpp"
#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/i_border_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/property.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/i_shape.hpp"
#include "maui/graphics/line_cap.hpp"
#include "maui/graphics/line_join.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::controls
{
    class border : public view<maui::core::i_border_view>
    {
    public:
        border() : border(padding_property())
        {
            this->set_style_target_type<border>();
        }

        // Shared bindable-property descriptors (one instance per type, like Border.*Property).
        static const maui::core::bindable_property<maui::core::thickness>& padding_property();
        static const maui::core::bindable_property<std::shared_ptr<maui::graphics::paint>>& stroke_property();
        static const maui::core::bindable_property<double>& stroke_thickness_property();
        static const maui::core::bindable_property<std::shared_ptr<maui::graphics::i_shape>>& stroke_shape_property();
        static const maui::core::bindable_property<std::vector<double>>& stroke_dash_array_property();
        static const maui::core::bindable_property<double>& stroke_dash_offset_property();
        static const maui::core::bindable_property<maui::graphics::line_cap>& stroke_line_cap_property();
        static const maui::core::bindable_property<maui::graphics::line_join>& stroke_line_join_property();
        static const maui::core::bindable_property<double>& stroke_miter_limit_property();

        // ---- Content (non-owning; Border.ContentChanged adds/removes the logical child) ----
        [[nodiscard]] maui::core::i_view* content() const override
        {
            return content_;
        }
        void set_content(maui::core::i_view& value)
        {
            set_content(&value);
        }
        void set_content(maui::core::i_view* value)
        {
            if (content_ == value)
            {
                return;
            }
            // Re-parent the logical child so the content inherits (or loses) this border's
            // BindingContext + Window (Border.ContentChanged → Remove/AddLogicalChild).
            if (auto* old_child = dynamic_cast<element*>(content_))
            {
                detach_logical_child(*old_child);
            }
            content_ = value;
            if (auto* new_child = dynamic_cast<element*>(content_))
            {
                attach_logical_child(*new_child);
            }
            if (const auto& element_handler = handler())
            {
                element_handler->invoke("set_content");
            }
        }

        // ---- i_padding ----
        [[nodiscard]] maui::core::thickness padding() const override
        {
            return padding_.get();
        }
        void set_padding(maui::core::thickness value)
        {
            padding_.set(value);
        }

        // ---- the stroke surface (i_border_stroke) ----
        [[nodiscard]] maui::graphics::paint* stroke() const override
        {
            return stroke_.get().get();
        }
        [[nodiscard]] const std::shared_ptr<maui::graphics::paint>& stroke_paint() const
        {
            return stroke_.get();
        }
        void set_stroke(std::shared_ptr<maui::graphics::paint> value)
        {
            stroke_.set(std::move(value));
        }

        [[nodiscard]] double stroke_thickness() const override
        {
            return stroke_thickness_.get();
        }
        void set_stroke_thickness(double value)
        {
            stroke_thickness_.set(value);
        }

        [[nodiscard]] maui::graphics::i_shape* shape() const override
        {
            return stroke_shape_.get().get();
        }
        [[nodiscard]] const std::shared_ptr<maui::graphics::i_shape>& stroke_shape() const
        {
            return stroke_shape_.get();
        }
        void set_stroke_shape(std::shared_ptr<maui::graphics::i_shape> value)
        {
            stroke_shape_.set(std::move(value));
        }

        [[nodiscard]] const std::vector<double>& stroke_dash_array() const
        {
            return stroke_dash_array_.get();
        }
        void set_stroke_dash_array(std::vector<double> value)
        {
            stroke_dash_array_.set(std::move(value));
        }
        // C# Border.StrokeDashPattern: the dash array materialized as float[] on each read.
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

        // ---- layout pass (Border.CrossPlatformMeasure/CrossPlatformArrange) ----
        maui::graphics::size measure(double width_constraint, double height_constraint) override;
        maui::graphics::size arrange(const maui::graphics::rect& bounds) override;

    protected:
        // The Padding store is descriptor-parameterized so `frame` can substitute its 20-default
        // descriptor (Frame.PaddingDefaultValueCreator) while reusing the border machinery.
        explicit border(const maui::core::bindable_property<maui::core::thickness>& padding_descriptor)
            : padding_{*this, padding_descriptor}
        {
        }

        // The single content child is this border's one logical child (BindingContext/Window inherit).
        void for_each_logical_child(const std::function<void(element&)>& visit) const override
        {
            if (auto* child = dynamic_cast<element*>(content_))
            {
                visit(*child);
            }
        }

    private:
        maui::core::i_view* content_ = nullptr; // NON-owning: the caller owns the content's lifetime
        maui::core::property<maui::core::thickness> padding_;
        maui::core::property<std::shared_ptr<maui::graphics::paint>> stroke_{*this, stroke_property()};
        maui::core::property<double> stroke_thickness_{*this, stroke_thickness_property()};
        maui::core::property<std::shared_ptr<maui::graphics::i_shape>> stroke_shape_{*this, stroke_shape_property()};
        maui::core::property<std::vector<double>> stroke_dash_array_{*this, stroke_dash_array_property()};
        maui::core::property<double> stroke_dash_offset_{*this, stroke_dash_offset_property()};
        maui::core::property<maui::graphics::line_cap> stroke_line_cap_{*this, stroke_line_cap_property()};
        maui::core::property<maui::graphics::line_join> stroke_line_join_{*this, stroke_line_join_property()};
        maui::core::property<double> stroke_miter_limit_{*this, stroke_miter_limit_property()};
    };
} // namespace maui::controls
