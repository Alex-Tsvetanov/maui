#pragma once
// maui::controls::templated_page  <=  Microsoft.Maui.Controls.TemplatedPage
//
// A page that displays content from a ControlTemplate — the C# base of ContentPage. The template
// machinery is the same as templated_view's (C# duplicates the IControlTemplated implementation
// across the two roots exactly the same way; the port mirrors that rather than inventing a shared
// base the original does not have). PAGE-NESS DEVIATION (documented): C# TemplatedPage derives Page;
// the port has no separate page base yet — the page bits (Title, Appearing/Disappearing) live on the
// existing content_page (M4d), and unifying them under a page base is deferred with content_page's
// re-basing onto this type. This type carries exactly TemplatedPage.cs's own surface.

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
    class templated_page : public view<maui::core::i_content_view>, public i_control_templated
    {
    public:
        templated_page()
        {
            this->set_style_target_type<templated_page>();
        }

        // Shared bindable-property descriptors (TemplatedPage.ControlTemplateProperty — its change
        // callback IS the template application — and the Padding store, from the Page base in C#).
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
        // TemplatedPage.OnControlTemplateChanged / OnApplyTemplate — virtual no-op subclass hooks.
        void on_control_template_changed(maui::controls::control_template* old_value,
                                         maui::controls::control_template* new_value) override
        {
            (void)old_value;
            (void)new_value;
        }
        void on_apply_template() override
        {
        }

        // ---- i_content_view (content = C# PresentedContent — the template root as a view) ----
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

        // ---- layout pass over the presented content (the same shape as templated_view) ----
        maui::graphics::size measure(double width_constraint, double height_constraint) override;
        maui::graphics::size arrange(const maui::graphics::rect& bounds) override;

    protected:
        // TemplatedPage.SetChildInheritedBindingContext: only inherit while untemplated.
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
