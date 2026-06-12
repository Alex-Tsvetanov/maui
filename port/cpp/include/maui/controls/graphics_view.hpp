#pragma once
// maui::controls::graphics_view  <=  Microsoft.Maui.Controls.GraphicsView (+ TouchEventArgs)
//
// A view drawn with canvas commands: the Drawable property + invalidate(), plus the seven
// interaction events the platform raises through the i_graphics_view inbound channel (the send_*
// methods — the i_button naming precedent). Ported from GraphicsView.cs.
//
// drawable: the control OWNS the drawable as a property<std::shared_ptr<i_drawable>> (a change flows
// through on_property_changed → handler->update_value("drawable") like every other property);
// i_graphics_view::drawable() returns the raw borrow. C#'s SetInheritedBindingContext propagation
// onto a BindableObject drawable is not modeled (the port's i_drawable is not a bindable —
// documented, not stubbed).
//
// The C# TouchEventArgs payload collapses onto the event signatures: the touch points (and the
// is-inside-bounds flag for end_interaction) ride the event args directly.

#include <memory>
#include <utility>
#include <vector>

#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/i_graphics_view.hpp"
#include "maui/core/property.hpp"
#include "maui/graphics/i_drawable.hpp"
#include "maui/graphics/point_f.hpp"

namespace maui::controls
{
    class graphics_view : public view<maui::core::i_graphics_view>
    {
    public:
        graphics_view()
        {
            this->set_style_target_type<graphics_view>();
        }

        // Shared bindable-property descriptor (GraphicsView.DrawableProperty).
        static const maui::core::bindable_property<std::shared_ptr<maui::graphics::i_drawable>>& drawable_property();

        // ---- i_graphics_view ----
        // Raw borrow into the owned shared_ptr (null when unset). The control retains ownership.
        [[nodiscard]] maui::graphics::i_drawable* drawable() const override
        {
            return drawable_.get().get();
        }

        // C# GraphicsView.Invalidate(): Handler?.Invoke(nameof(IGraphicsView.Invalidate)).
        void invalidate() override
        {
            if (const auto& element_handler = handler())
            {
                element_handler->invoke("invalidate");
            }
        }

        // ---- the inbound interaction channel (the platform view calls these; each raises its
        //      public event — the C# explicit IGraphicsView implementations) ----
        void send_start_hover_interaction(const std::vector<maui::graphics::point_f>& points) override
        {
            start_hover_interaction.raise(points);
        }
        void send_move_hover_interaction(const std::vector<maui::graphics::point_f>& points) override
        {
            move_hover_interaction.raise(points);
        }
        void send_end_hover_interaction() override
        {
            end_hover_interaction.raise();
        }
        void send_start_interaction(const std::vector<maui::graphics::point_f>& points) override
        {
            start_interaction.raise(points);
        }
        void send_drag_interaction(const std::vector<maui::graphics::point_f>& points) override
        {
            drag_interaction.raise(points);
        }
        void send_end_interaction(const std::vector<maui::graphics::point_f>& points, bool is_inside_bounds) override
        {
            end_interaction.raise(points, is_inside_bounds);
        }
        void send_cancel_interaction() override
        {
            cancel_interaction.raise();
        }

        // ---- public setter ----
        // The control takes ownership of the drawable. Passing a distinct instance fires the change.
        void set_drawable(std::shared_ptr<maui::graphics::i_drawable> value)
        {
            drawable_.set(std::move(value));
        }

        // ---- developer-facing events (C# GraphicsView's seven; TouchEventArgs collapsed) ----
        maui::core::event<std::vector<maui::graphics::point_f>> start_hover_interaction;
        maui::core::event<std::vector<maui::graphics::point_f>> move_hover_interaction;
        maui::core::event<> end_hover_interaction;
        maui::core::event<std::vector<maui::graphics::point_f>> start_interaction;
        maui::core::event<std::vector<maui::graphics::point_f>> drag_interaction;
        maui::core::event<std::vector<maui::graphics::point_f>, bool> end_interaction;
        maui::core::event<> cancel_interaction;

    private:
        maui::core::property<std::shared_ptr<maui::graphics::i_drawable>> drawable_{*this, drawable_property()};
    };
} // namespace maui::controls
