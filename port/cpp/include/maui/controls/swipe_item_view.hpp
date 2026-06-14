#pragma once
// maui::controls::swipe_item_view  <=  Microsoft.Maui.Controls.SwipeItemView
//
// A swipe item that displays custom content. Ported from src/Controls/src/Core/SwipeView/
// SwipeItemView.cs (SwipeItemView : ContentView, Controls.ISwipeItem, Maui.ISwipeItemView): it hosts a
// single Content (like content_page) and is activated as a swipe item via OnInvoked(), which (in C#)
// executes the Command then raises Invoked. The port has no ICommand (the command channel IS the
// `invoked` event, per the W1-11 collapse), so OnInvoked() simply raises `invoked` — the observable
// effect a test asserts.
//
// Content is a non-owning child (the caller owns its lifetime, PROFILE §8), parented as this view's
// logical child so BindingContext inherits down — the same recipe content_page uses. AutomationId is
// carried here (C# Element.AutomationId) for native item identification — behaviorally inert.

#include <functional>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/element.hpp"
#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/i_swipe_item_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/property.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::controls
{
    class swipe_item_view : public view<maui::core::i_swipe_item_view>
    {
    public:
        swipe_item_view()
        {
            this->set_style_target_type<swipe_item_view>();
        }

        // Shared bindable-property descriptor (the content view's Padding).
        static const maui::core::bindable_property<maui::core::thickness>& padding_property();

        // C# SwipeItemView.Invoked — raised by on_invoked().
        maui::core::event<> invoked;

        // ---- i_content_view: Content (non-owning child pointer; the caller owns its lifetime) ----
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

        // ---- AutomationId override (the i_swipe_item face) ----
        // view<> already carries an automation_id bindable; expose it as the i_swipe_item getter so the
        // single value backs both the IView automation id and the ISwipeItem.AutomationId.
        [[nodiscard]] std::string_view automation_id() const override
        {
            return view<maui::core::i_swipe_item_view>::automation_id();
        }

        // ---- i_swipe_item: OnInvoked ----
        // C# SwipeItemView.OnInvoked: execute the command (collapsed to the `invoked` event), then raise
        // Invoked. With no ICommand, the single observable effect is the Invoked event.
        void on_invoked() override
        {
            invoked.raise();
        }

        // ---- layout pass: MeasureContent / ArrangeContent within the padding (content_page recipe) ----
        maui::graphics::size measure(double width_constraint, double height_constraint) override;
        maui::graphics::size arrange(const maui::graphics::rect& bounds) override;

    protected:
        void for_each_logical_child(const std::function<void(element&)>& visit) const override
        {
            if (auto* child = dynamic_cast<element*>(content_))
            {
                visit(*child);
            }
        }

    private:
        maui::core::i_view* content_ = nullptr; // NON-owning: the caller owns the content's lifetime
        maui::core::property<maui::core::thickness> padding_{*this, padding_property()};
    };
} // namespace maui::controls
