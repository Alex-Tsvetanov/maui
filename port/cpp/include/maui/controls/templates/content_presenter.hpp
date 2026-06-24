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
// The presented content is CO-OWNED (shared_ptr): in C# the presenter's Content property is a GC
// reference keeping the content alive while presented — the shared_ptr is the faithful translation
// (PROFILE §8), and it makes templated-parent teardown safe when the developer's owning reference is
// dropped first. No cycle: content never owns its presenter.
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

        // ---- Content (ContentPresenter.Content; co-owned — see header comment) ----
        [[nodiscard]] maui::core::i_view* content() const override
        {
            return dynamic_cast<maui::core::i_view*>(content_.get());
        }
        [[nodiscard]] element* content_element() const
        {
            return content_.get();
        }
        // The OnContentChanged transition: detach the old content from the logical tree, attach the
        // new one (window/resources flow through the presenter; the BindingContext does NOT — the
        // templated control pushes it into the content directly).
        void set_content(std::shared_ptr<element> value);

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

        // Generic mount hook (element::mount_into_handler): re-fire "set_content" so the presenter's
        // now-attached handler hosts the packed developer content as a native subview. The presenter's
        // content is packed in the templated parent's CONSTRUCTOR (before any handler exists), so the
        // set_content above found no handler and hosted nothing; the generic driver (app_host.hpp) calls
        // this post-order — after this presenter's handler AND its content's handler are attached — to
        // replay it. The exact re-host the old gallery_rehost_content(*presenter) hand-fired. A no-op when
        // no handler / no content (handler()->invoke is safe to call with no presented content).
        void mount_into_handler() override
        {
            if (const auto& element_handler = handler())
            {
                element_handler->invoke("set_content");
            }
        }

    private:
        std::shared_ptr<element> content_; // co-owned while presented (see header comment)
        maui::core::property<maui::core::thickness> padding_{*this, padding_property()};
    };
} // namespace maui::controls
