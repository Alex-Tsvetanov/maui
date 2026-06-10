#pragma once
// maui::controls::template_utilities  <=  Microsoft.Maui.Controls.TemplateUtilities (internal)
//
// The shared ControlTemplate machinery the templated controls call into:
//   - on_control_template_changed: the ControlTemplateProperty change handler — un-bind every
//     content_presenter in the OLD template's scope (the BFS that stops at nested templated scopes),
//     drop all internal children, create + attach the new template content (throwing
//     std::runtime_error, the C# NotSupportedException, when it is not a View), then fire the
//     OnControlTemplateChanged / TemplateRoot / OnApplyTemplate sequence in the C# order.
//   - on_content_changed: the Content change handler a content-hosting subclass (content_view /
//     content_page, or a test double) routes its Content set through — untemplated: the content
//     becomes THE logical child (all others removed); templated: the content inherits the control's
//     BindingContext directly and every content_presenter bound to this control re-presents it (the
//     C# presenter binding push, walked explicitly here).
//
// GetTemplateChild (TemplateRoot?.FindByName) is DEFERRED with the element-side name-scope slot
// (xaml/name_scope.hpp documents the placement deviation) — the code-first port has no per-element
// name registry yet; its C# tests are XAML-driven.
//
// A friend of element: the walks need the protected logical-children visitation (the C# class reads
// Element.LogicalChildrenInternal, likewise internal).

#include <memory>

namespace maui::controls
{
    class control_template;
    class element;

    class template_utilities
    {
    public:
        template_utilities() = delete;

        // TemplateUtilities.OnControlTemplateChanged — `self` must implement i_control_templated (its
        // control_template() already returns the NEW value; `old_value` is the replaced one).
        static void on_control_template_changed(element& self, control_template* old_value);

        // TemplateUtilities.OnContentChanged — `self` must implement i_control_templated and have
        // already stored `new_content` (templated_content() returns it).
        static void on_content_changed(element& self, const std::shared_ptr<element>& new_content);

    private:
        // The presenter push: re-present `owner`'s templated_content() on every content_presenter in
        // its logical subtree whose templated parent resolves to `owner` (members so the element
        // friendship covers the child walk).
        static void refresh_owned_presenters(element& owner);
        // Drop every internal child (the C# "remove all remnants" while-loop).
        static void remove_all_internal_children(element& self);
    };
} // namespace maui::controls
