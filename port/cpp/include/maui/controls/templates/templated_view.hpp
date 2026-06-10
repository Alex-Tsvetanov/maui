#pragma once
// maui::controls::templated_view  <=  Microsoft.Maui.Controls.TemplatedView
//
// A view that displays content from a ControlTemplate — the base for content_view (M-later). Setting
// control_template runs template_utilities::on_control_template_changed (the bindable property's
// change callback, like C#'s ControlTemplateProperty), which mints the template content and parents
// it as THE internal logical child; template_root() then names it.
//
// i_content_view::content() carries C# PresentedContent (the template root as a view) — the port's
// i_content_view collapses Content/PresentedContent into the single accessor the handler/measure seam
// reads; the C# IContentView.Content (developer content, null here) lives on
// i_control_templated::templated_content() for the content_presenter pull.
//
// BindingContext inheritance follows TemplatedView.SetChildInheritedBindingContext: while a
// ControlTemplate is set, logical children (the template subtree) do NOT inherit this view's context —
// template content binds to the TEMPLATED PARENT, not to the data context.
//
// measure/arrange mirror the C# CrossPlatformMeasure/Arrange (MeasureContent/ArrangeContent over the
// presented content within the padding) — the same shape as content_page. Out of scope (documented):
// the legacy Compatibility.Layout surface, IsClippedToBounds and CascadeInputTransparent.

#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "maui/controls/element.hpp"
#include "maui/controls/templates/control_template.hpp"
#include "maui/controls/templates/i_control_templated.hpp"
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
    class templated_view : public view<maui::core::i_content_view>, public i_control_templated
    {
    public:
        templated_view()
        {
            this->set_style_target_type<templated_view>();
        }
        // Deterministic teardown (PROFILE §8): drop the template — the un-apply pipeline clears the
        // presenters and DETACHES every internal child, so a child kept alive by an external owner
        // re-resolves its template bindings to "out of scope" and drops its subscriptions into this
        // (dying) templated parent instead of dangling. Then detach any manually-added remainder.
        ~templated_view() override
        {
            set_control_template(nullptr);
            while (!internal_children_.empty())
            {
                // Qualified on purpose: in a destructor the dispatch is at this class level anyway.
                templated_view::remove_at(static_cast<int>(internal_children_.size()) - 1);
            }
        }
        templated_view(const templated_view&) = delete;
        templated_view(templated_view&&) = delete;
        templated_view& operator=(const templated_view&) = delete;
        templated_view& operator=(templated_view&&) = delete;

        // Shared bindable-property descriptors (TemplatedView.ControlTemplateProperty — its change
        // callback IS the template application — and the Padding store).
        static const maui::core::bindable_property<std::shared_ptr<maui::controls::control_template>>&
        control_template_property();
        static const maui::core::bindable_property<maui::core::thickness>& padding_property();

        // ---- ControlTemplate (bindable; setting it applies/replaces the template) ----
        [[nodiscard]] const std::shared_ptr<maui::controls::control_template>& control_template() const override
        {
            return control_template_.get();
        }
        void set_control_template(std::shared_ptr<maui::controls::control_template> value)
        {
            control_template_.set(std::move(value));
        }

        // ---- i_control_templated ----
        [[nodiscard]] const std::vector<std::shared_ptr<element>>& internal_children() const override
        {
            return internal_children_;
        }
        void add_logical_child(std::shared_ptr<element> child) override;
        bool remove_at(int index) override;
        [[nodiscard]] element* template_root() const override
        {
            return template_root_;
        }
        void set_template_root(element* value) override
        {
            template_root_ = value;
        }
        // TemplatedView.OnControlTemplateChanged / OnApplyTemplate — virtual no-op hooks for
        // subclasses (invoked by template_utilities in the C# order).
        void on_control_template_changed(maui::controls::control_template* old_value,
                                         maui::controls::control_template* new_value) override
        {
            (void)old_value;
            (void)new_value;
        }
        void on_apply_template() override
        {
        }

        // ---- i_content_view (content = C# PresentedContent; see header comment) ----
        [[nodiscard]] maui::core::i_view* content() const override
        {
            return dynamic_cast<maui::core::i_view*>(template_root_);
        }
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
        // TemplatedView.SetChildInheritedBindingContext: only inherit while untemplated.
        void set_child_inherited_binding_context(
            element& child, const maui::core::bindable_object::binding_context_box& context) override
        {
            if (control_template() == nullptr)
            {
                element::set_child_inherited_binding_context(child, context);
            }
        }

        void for_each_logical_child(const std::function<void(element&)>& visit) const override
        {
            for (const auto& child : internal_children_)
            {
                visit(*child);
            }
        }

    private:
        std::vector<std::shared_ptr<element>> internal_children_; // OWNING (the template mints content)
        element* template_root_ = nullptr;                        // borrowed from internal_children_
        maui::core::property<std::shared_ptr<maui::controls::control_template>> control_template_{
            *this, control_template_property()};
        maui::core::property<maui::core::thickness> padding_{*this, padding_property()};
    };
} // namespace maui::controls
