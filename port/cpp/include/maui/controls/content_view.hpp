#pragma once
// maui::controls::content_view  <=  Microsoft.Maui.Controls.ContentView
//
// An element that contains a single child element — the simple (non-page) single-content control,
// commonly the base for custom composite views. Ported from src/Controls/src/Core/ContentView/
// ContentView.cs: it derives TemplatedView (the merged templates base, templates/templated_view.hpp),
// adds the Content bindable surface routed through TemplateUtilities.OnContentChanged, and overrides
// the BindingContext inheritance so the content ALWAYS receives the control's context (unlike the
// bare TemplatedView, whose template subtree binds to the templated parent instead):
//   - SetChildInheritedBindingContext → plain inheritance for every logical child,
//   - OnBindingContextChanged / OnControlTemplateChanged → push the context into Content directly
//     (the content belongs to the OUTER data context even while presented inside a template scope).
//
// Content OWNERSHIP: shared_ptr<element> — unlike the page-level content hosts (content_page /
// border / scroll_view, whose content is a non-owning i_view*), the templated machinery co-owns what
// it presents (i_control_templated::templated_content() and content_presenter both traffic in
// shared_ptr, the C# GC-reference analog), so the developer content uses the same currency.
//
// i_content_view::content() carries C# IContentView.PresentedContent — the template root when a
// ControlTemplate is applied, else the developer Content (ContentView.cs: PresentedContent =>
// TemplateRoot ?? Content). The handler is the SAME content_page_handler the port ports from C#'s
// ContentViewHandler (registered in content_view.cpp) — content_page and content_view are siblings
// over one handler, exactly as both C# controls resolve to ContentViewHandler.
//
// measure/arrange are inherited from templated_view (MeasureContent/ArrangeContent over the
// presented content within the padding — the C# ContentView inherits the same via TemplatedView's
// ICrossPlatformLayout).

#include <memory>
#include <utility>

#include "maui/controls/element.hpp"
#include "maui/controls/templates/control_template.hpp"
#include "maui/controls/templates/template_utilities.hpp"
#include "maui/controls/templates/templated_view.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/i_safe_area_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/property.hpp"
#include "maui/core/safe_area_edges.hpp"
#include "maui/core/safe_area_regions.hpp"
#include "maui/core/thickness.hpp"

namespace maui::controls
{
    class content_view : public templated_view, public maui::core::i_safe_area_view2
    {
    public:
        content_view()
        {
            this->set_style_target_type<content_view>();
        }
        // Out-of-line (content_view.cpp) so that TU — which also carries the MAUI_REGISTER_HANDLER
        // registrar — is the vtable's home and gets linked whenever the control is used (an otherwise
        // header-only control would let the static-library linker drop the registration object).
        ~content_view() override;
        content_view(const content_view&) = delete;
        content_view(content_view&&) = delete;
        content_view& operator=(const content_view&) = delete;
        content_view& operator=(content_view&&) = delete;

        // ---- Content (C# ContentView.ContentProperty, changed → TemplateUtilities.OnContentChanged) ----
        [[nodiscard]] const std::shared_ptr<element>& content_child() const
        {
            return content_;
        }
        void set_content(std::shared_ptr<element> value)
        {
            if (content_ == value)
            {
                return;
            }
            content_ = std::move(value);
            // Untemplated: the content becomes THE logical child; templated: it inherits this control's
            // context directly and the owned presenters re-present it (TemplateUtilities.OnContentChanged).
            template_utilities::on_content_changed(*this, content_);
            // Tell the handler to re-host the (possibly changed) presented content — the C# mapper's
            // "Content" entry runs off the property change; the port routes runtime content changes
            // through the same "set_content" command its sibling content hosts use.
            if (const auto& element_handler = handler())
            {
                element_handler->invoke("set_content");
            }
        }

        // ---- i_control_templated: the developer content the content_presenter pulls ----
        [[nodiscard]] std::shared_ptr<element> templated_content() const override
        {
            return content_;
        }

        // ---- i_content_view: PresentedContent = TemplateRoot ?? Content (ContentView.cs) ----
        [[nodiscard]] maui::core::i_view* content() const override
        {
            if (auto* presented = templated_view::content())
            {
                return presented; // the applied template's root
            }
            return dynamic_cast<maui::core::i_view*>(content_.get());
        }

        // C# ContentView.OnControlTemplateChanged: re-push the context into Content (the template swap
        // re-parents the logical tree, but the content keeps binding to the outer data context).
        void on_control_template_changed(maui::controls::control_template* old_value,
                                         maui::controls::control_template* new_value) override
        {
            templated_view::on_control_template_changed(old_value, new_value);
            if (content_ != nullptr)
            {
                content_->set_inherited_binding_context(raw_binding_context());
            }
        }

        // maui::controls::content_view::safe_area_edges_property <=
        // Microsoft.Maui.Controls.ContentView.SafeAreaEdgesProperty
        static const maui::core::bindable_property<maui::core::safe_area_edges>& safe_area_edges_property();

        // ---- SafeAreaEdges (control-only; ContentView.SafeAreaEdges) ----
        [[nodiscard]] maui::core::safe_area_edges safe_area_edges() const
        {
            return safe_area_edges_.get();
        }
        void set_safe_area_edges(maui::core::safe_area_edges value)
        {
            safe_area_edges_.set(value);
        }

        void set_safe_area_insets(const maui::core::thickness& value) override;
        [[nodiscard]] maui::core::safe_area_regions get_safe_area_regions_for_edge(int edge) const override;

    protected:
        // C# ContentView.SetChildInheritedBindingContext: ALWAYS propagate (TemplatedView suppresses
        // inheritance while templated; ContentView restores it so Content binds to the data context).
        void set_child_inherited_binding_context(
            element& child, const maui::core::bindable_object::binding_context_box& context) override
        {
            child.set_inherited_binding_context(context);
        }

        // C# ContentView.OnBindingContextChanged: push the new context into Content directly.
        void on_binding_context_changed() override
        {
            templated_view::on_binding_context_changed();
            if (content_ != nullptr)
            {
                content_->set_inherited_binding_context(raw_binding_context());
            }
        }

    private:
        std::shared_ptr<element> content_; // ContentView.Content (co-owned; see header comment)
        maui::core::property<maui::core::safe_area_edges> safe_area_edges_{*this, safe_area_edges_property()};
    };
} // namespace maui::controls
