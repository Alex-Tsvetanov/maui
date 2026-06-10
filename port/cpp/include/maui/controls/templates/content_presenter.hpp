#pragma once
// maui::controls::content_presenter  <=  Microsoft.Maui.Controls.ContentPresenter
//
// The placeholder inside a control template where the templated parent's developer content appears.
// Its constructor registers the TemplatedParent.Content pull as a template_binding (the C# ctor's
// `SetBinding(ContentProperty, (IContentView v) => v.Content, source: RelativeBindingSource
// .TemplatedParent)`): whenever the presenter enters/leaves a template scope, it re-presents (or
// drops) the templated parent's templated_content(); later Content changes on the templated parent
// are pushed by template_utilities::on_content_changed (the binding's change propagation in C#).
//
// The presented content is held NON-owning (element*) — it is the developer's content, owned outside
// the template scope (the templated control / the developer, PROFILE §8), unlike the template-minted
// internal children a templated control owns.
//
// SetChildInheritedBindingContext is a no-op (the C# comment: "we never want to use the standard
// inheritance mechanism, we will get this set by our parent") — the presented content's BindingContext
// is pushed DIRECTLY by the templated control, never by the presenter (which lives inside the
// template scope and has no data context of its own). C#'s ParentOverride redirection collapses into
// that push model. clear() (C# internal Clear) is what the template-replacement BFS calls.

#include <functional>
#include <memory>

#include "maui/controls/element.hpp"
#include "maui/controls/view.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/i_content_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/property.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::controls
{
    class content_presenter : public view<maui::core::i_content_view>
    {
    public:
        // Registers the TemplatedParent.Content pull (see header comment).
        content_presenter();

        // Shared bindable-property descriptor (the Padding store; ContentProperty's role is carried
        // by set_content + the pull binding — see header comment).
        static const maui::core::bindable_property<maui::core::thickness>& padding_property();

        // ---- Content (ContentPresenter.Content; NON-owning — see header comment) ----
        [[nodiscard]] maui::core::i_view* content() const override
        {
            return dynamic_cast<maui::core::i_view*>(content_);
        }
        [[nodiscard]] element* content_element() const
        {
            return content_;
        }
        // The OnContentChanged transition: detach the old content from the logical tree, attach the
        // new one (window/resources flow through the presenter; the BindingContext does NOT — the
        // templated control pushes it into the content directly).
        void set_content(element* value);

        // ContentPresenter.Clear — Content = null (called by the template-replacement BFS).
        void clear()
        {
            set_content(nullptr);
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

        // ---- layout pass over the presented content (C# MeasureContent/ArrangeContent) ----
        maui::graphics::size measure(double width_constraint, double height_constraint) override;
        maui::graphics::size arrange(const maui::graphics::rect& bounds) override;

    protected:
        // "We never want to use the standard inheritance mechanism" (ContentPresenter.cs).
        void set_child_inherited_binding_context(
            element& child, const maui::core::bindable_object::binding_context_box& context) override
        {
            (void)child;
            (void)context;
        }

        void for_each_logical_child(const std::function<void(element&)>& visit) const override
        {
            if (content_ != nullptr)
            {
                visit(*content_);
            }
        }

    private:
        element* content_ = nullptr; // NON-owning: the developer/templated control owns the content
        maui::core::property<maui::core::thickness> padding_{*this, padding_property()};
    };
} // namespace maui::controls
