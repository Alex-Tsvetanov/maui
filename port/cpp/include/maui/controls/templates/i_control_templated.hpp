#pragma once
// maui::controls::i_control_templated  <=  Microsoft.Maui.Controls.IControlTemplated (internal)
//
// The contract a templated control (templated_view / templated_page, later content_view/content_page)
// gives the ControlTemplate machinery (template_utilities): the template property, the internal
// logical-children store, the template root, and the change/apply hooks.
//
// Ownership: internal_children OWN their elements (shared_ptr) — a control template MINTS its content
// subtree, so the templated control is the only natural owner (PROFILE §8); template_root() is a
// non-owning borrow into that store (C#'s TemplateRoot is likewise just a reference to one
// InternalChildren entry).
//
// templated_content() is a port adaptation: C#'s ContentPresenter binds to the templated parent's
// IContentView.Content (the DEVELOPER content — null on a plain TemplatedView/TemplatedPage, the
// Content property on ContentView/ContentPage). The port's i_content_view::content() carries
// PresentedContent (what the handler hosts — the template root when templated), so the presenter
// source needs its own accessor; content-hosting subclasses override it to return their content.

#include <memory>
#include <vector>

namespace maui::controls
{
    class control_template;
    class element;

    class i_control_templated
    {
    public:
        virtual ~i_control_templated() = default;

        // IControlTemplated.ControlTemplate (get; the set is each control's bindable property).
        [[nodiscard]] virtual const std::shared_ptr<maui::controls::control_template>& control_template() const = 0;

        // IControlTemplated.InternalChildren — the owning logical-children store.
        [[nodiscard]] virtual const std::vector<std::shared_ptr<element>>& internal_children() const = 0;
        // IControlTemplated.AddLogicalChild — append (if absent) + attach to the logical tree.
        virtual void add_logical_child(std::shared_ptr<element> child) = 0;
        // IControlTemplated.RemoveAt — detach + remove the child at index (clearing template_root when
        // it is the one removed — the C# OnChildRemoved → TemplateUtilities.OnChildRemoved hook).
        virtual bool remove_at(int index) = 0;

        // IControlTemplated.TemplateRoot — the applied template's root (borrowed; null when none).
        [[nodiscard]] virtual element* template_root() const = 0;
        virtual void set_template_root(element* value) = 0;

        // C# IContentView.Content for the ContentPresenter pull (see header comment). Null here.
        // Returned SHARED: the presenter co-owns what it presents (the C# GC reference), so content
        // outliving or predeceasing its presenter cannot dangle.
        [[nodiscard]] virtual std::shared_ptr<element> templated_content() const
        {
            return nullptr;
        }

        // IControlTemplated.OnControlTemplateChanged / OnApplyTemplate — subclass hooks invoked by
        // template_utilities in the C# order (after the new root is attached).
        virtual void on_control_template_changed(maui::controls::control_template* old_value,
                                                 maui::controls::control_template* new_value) = 0;
        virtual void on_apply_template() = 0;

    protected:
        i_control_templated() = default;
        i_control_templated(const i_control_templated&) = default;
        i_control_templated(i_control_templated&&) = default;
        i_control_templated& operator=(const i_control_templated&) = default;
        i_control_templated& operator=(i_control_templated&&) = default;
    };
} // namespace maui::controls
