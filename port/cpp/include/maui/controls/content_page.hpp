#pragma once
// maui::controls::content_page  <=  Microsoft.Maui.Controls.ContentPage
//
// A page that hosts a single child view (its Content) within a Padding, plus a Title. The minimal
// content-hosting control: it owns no visual of its own — it measures/arranges the one content child
// inside the padding and lets its handler host that child on a native panel. Ported from ContentPage.cs
// (+ Page.cs for Title/Padding); behavior derived from LayoutExtensions.MeasureContent/ArrangeContent.
//
// Same API shape as the other controls: bare-noun interface getters (content()/padding()) + method
// accessors, each backed by a private property<T> whose change flows through view::on_property_changed
// to the handler — EXCEPT the content child, which is a NON-OWNING raw pointer (the caller owns the
// content's lifetime, PROFILE §8) rather than a property<T>. Setting it notifies the handler through the
// "set_content" command (mirroring how the layout control routes child changes through its command
// mapper), so the native host re-parents the content's subview.
//
// Title lives on the control only (Page.Title in C#); the i_content_view contract does not carry it
// (kept minimal — a page title is not part of the cross-platform content-hosting surface).
//
// Page LIFECYCLE (M4d): content_page also serves as the minimal `Page` for navigation — it carries the
// Appearing/Disappearing lifecycle (Page.cs SendAppearing/SendDisappearing + the Appearing/Disappearing
// events). The pair is idempotent via a has_appeared_ flag: send_disappearing returns early unless the
// page has appeared; send_appearing sets the flag and fires (a second send_appearing is a no-op). A
// navigation_page drives these as it swaps the current page on push/pop. The C# window-hierarchy guard
// in SendAppearing (only appear once attached to a window) is dropped at this layer — there is no window
// lifecycle yet, so the navigation host (or the test) drives appearing/disappearing directly.

#include <functional>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/element.hpp"
#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/i_content_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/property.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::controls
{
    class content_page : public view<maui::core::i_content_view>
    {
    public:
        // Declare the style TargetType so an implicit / class style targeting `content_page` matches it.
        content_page()
        {
            this->set_style_target_type<content_page>();
        }

        // Shared bindable-property descriptors (one instance per type, like ContentPage/Page.*Property).
        static const maui::core::bindable_property<maui::core::thickness>& padding_property();
        static const maui::core::bindable_property<std::string>& title_property();

        // ---- page lifecycle events (Page.Appearing / Page.Disappearing) ----
        // Fired by send_appearing()/send_disappearing(); carry no args (C# EventHandler with EventArgs.Empty).
        maui::core::event<> appearing;
        maui::core::event<> disappearing;

        // C# Page.SendAppearing: idempotent — sets has_appeared and fires `appearing` once; a second call
        // while already appeared is a no-op (matching _hasAppeared early-out). VIRTUAL (the C#
        // OnAppearing/OnDisappearing template-method hook collapsed onto the send): a composite page
        // (flyout_page) overrides to propagate the lifecycle into its panes first, then calls the base.
        virtual void send_appearing()
        {
            if (has_appeared_)
            {
                return;
            }
            has_appeared_ = true;
            appearing.raise();
        }

        // C# Page.SendDisappearing: idempotent — returns early unless the page has appeared, then clears
        // has_appeared and fires `disappearing`. Virtual like send_appearing (see above).
        virtual void send_disappearing()
        {
            if (!has_appeared_)
            {
                return;
            }
            has_appeared_ = false;
            disappearing.raise();
        }

        // C# Page.HasAppeared (exposed here so a navigation host can mirror the appeared-state guard).
        [[nodiscard]] bool has_appeared() const
        {
            return has_appeared_;
        }

        // ---- i_content_view (Content is a non-owning child pointer; the caller owns its lifetime) ----
        [[nodiscard]] maui::core::i_view* content() const override
        {
            return content_;
        }
        // Set (or replace) the content child. Notifies the handler so the native host re-parents it.
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
            // Detach the old content / attach the new one from this page's logical tree, so the content
            // inherits (or loses) this page's BindingContext + Window (Element.OnChildRemoved/OnChildAdded).
            if (auto* old_child = dynamic_cast<element*>(content_))
            {
                detach_logical_child(*old_child);
            }
            content_ = value;
            if (auto* new_child = dynamic_cast<element*>(content_))
            {
                attach_logical_child(*new_child);
            }
            notify_content_changed();
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

        // ---- Title (control-only; Page.Title) ----
        [[nodiscard]] std::string_view title() const
        {
            return title_.get();
        }
        void set_title(std::string value)
        {
            title_.set(std::move(value));
        }

        // ---- layout pass: the content view computes its OWN geometry by measuring/arranging the single
        // content within the padding (C# LayoutExtensions.MeasureContent / ArrangeContent), unlike a leaf
        // control which delegates measure/arrange to its handler. arrange additionally sizes the native
        // host panel to its bounds (C# ContentPage.ArrangeOverride/CrossPlatformArrange: Frame = bounds;
        // Handler.PlatformArrange) before placing the content.
        maui::graphics::size measure(double width_constraint, double height_constraint) override;
        maui::graphics::size arrange(const maui::graphics::rect& bounds) override;

    protected:
        // The single content child is this page's one logical child, so BindingContext + Window inherit
        // down to it. content_ is the i_view contract; cross-cast to the element base every control shares.
        void for_each_logical_child(const std::function<void(element&)>& visit) const override
        {
            if (auto* child = dynamic_cast<element*>(content_))
            {
                visit(*child);
            }
        }

    private:
        // Tell the handler (if attached) to re-host the new content on the native panel. The handler
        // reads the current content from the virtual view, so the bare command carries no payload.
        void notify_content_changed()
        {
            if (const auto& element_handler = handler())
            {
                element_handler->invoke("set_content");
            }
        }

        maui::core::i_view* content_ = nullptr; // NON-owning: the caller owns the content's lifetime
        maui::core::property<maui::core::thickness> padding_{*this, padding_property()};
        maui::core::property<std::string> title_{*this, title_property()};
        bool has_appeared_ = false; // Page._hasAppeared — gates the idempotent appearing/disappearing pair
    };
} // namespace maui::controls
