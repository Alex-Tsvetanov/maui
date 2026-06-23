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
#include "maui/controls/menu_bar_item.hpp"
#include "maui/controls/menu_element_list.hpp"
#include "maui/controls/toolbar_item.hpp"
#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/i_content_view.hpp"
#include "maui/core/i_ios_page_specifics.hpp" // --- platform configuration (W2-24) ---
#include "maui/core/i_safe_area_view.hpp"     // --- platform configuration (W2-24) ---
#include "maui/core/i_view.hpp"
#include "maui/core/property.hpp"
#include "maui/core/safe_area_edges.hpp" // --- per-control SafeAreaEdges (U20) ---
#include "maui/core/safe_area_regions.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::controls
{
    class content_page : public view<maui::core::i_content_view>,
                         public maui::core::i_ios_page_specifics, // --- platform configuration (W2-24) ---
                         public maui::core::i_safe_area_view,     // (the iOS status-bar / safe-area faces
                         public maui::core::i_safe_area_view2     //  the native host consults; Page.cs)
    {
    public:
        // Declare the style TargetType so an implicit / class style targeting `content_page` matches it.
        // C# ContentPage() hooks `NavigatedTo += (_, _) => UpdateHideSoftInputOnTapped()`; the port's
        // nearest navigation-lifecycle signal is `appearing` (fired once the page is shown — has_appeared()
        // is the HasNavigatedTo gate the manager re-checks), so the update fires on each appear.
        content_page()
        {
            this->set_style_target_type<content_page>();
            appearing.connect([this] { update_hide_soft_input_on_tapped(); });
        }

        // Shared bindable-property descriptors (one instance per type, like ContentPage/Page.*Property).
        static const maui::core::bindable_property<maui::core::thickness>& padding_property();
        static const maui::core::bindable_property<std::string>& title_property();
        // C# ContentPage.HideSoftInputOnTappedProperty (bool, default false): when true, tapping the page
        // outside text-input controls dismisses the soft keyboard (iOS-only behavior — see
        // hide_soft_input_on_tapped_manager). A change routes through on_property_changed →
        // handler->update_value("hide_soft_input_on_tapped") → the handler's map_hide_soft_input_on_tapped.
        static const maui::core::bindable_property<bool>& hide_soft_input_on_tapped_property();
        // C# SafeAreaElement.SafeAreaEdgesProperty (ContentPage.SafeAreaEdgesProperty): the per-edge
        // safe-area knob. Static metadata default is SafeAreaEdges.Default, but the per-element
        // default-value creator returns SafeAreaEdges.None (edge-to-edge) — every page is None by default.
        // A change routes through on_property_changed -> handler->update_value("safe_area_edges") ->
        // content_page_handler::map_safe_area_edges (re-layout).
        static const maui::core::bindable_property<maui::core::safe_area_edges>& safe_area_edges_property();

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

        // ---- HideSoftInputOnTapped (control-only; ContentPage.HideSoftInputOnTapped) ----
        // A change flows through on_property_changed → handler->update_value("hide_soft_input_on_tapped")
        // (content_page_handler::map_hide_soft_input_on_tapped → manager.update_page(*this)).
        [[nodiscard]] bool hide_soft_input_on_tapped() const
        {
            return hide_soft_input_on_tapped_.get();
        }
        void set_hide_soft_input_on_tapped(bool value)
        {
            hide_soft_input_on_tapped_.set(value);
        }

        // ---- SafeAreaEdges (control-only; ContentPage.SafeAreaEdges) ----
        // The per-edge safe-area knob. A change flows through on_property_changed →
        // handler->update_value("safe_area_edges") (content_page_handler::map_safe_area_edges → re-layout).
        [[nodiscard]] maui::core::safe_area_edges safe_area_edges() const
        {
            return safe_area_edges_.get();
        }
        void set_safe_area_edges(maui::core::safe_area_edges value)
        {
            safe_area_edges_.set(value);
        }

        // ---- chrome (W1-11): the per-page chrome item collections (Page.ToolbarItems /
        // Page.MenuBarItems). Items added here are parented to this page (Page's collection-changed
        // parenting), surface through the window chrome via the toolbar/menu-bar trackers, and are
        // NON-owning (the caller owns each item's lifetime, PROFILE §8). ----
        [[nodiscard]] menu_element_list<toolbar_item>& toolbar_items()
        {
            return toolbar_items_;
        }
        [[nodiscard]] menu_element_list<menu_bar_item>& menu_bar_items()
        {
            return menu_bar_items_;
        }

        // ---- layout pass: the content view computes its OWN geometry by measuring/arranging the single
        // content within the padding (C# LayoutExtensions.MeasureContent / ArrangeContent), unlike a leaf
        // control which delegates measure/arrange to its handler. arrange additionally sizes the native
        // host panel to its bounds (C# ContentPage.ArrangeOverride/CrossPlatformArrange: Frame = bounds;
        // Handler.PlatformArrange) before placing the content.
        maui::graphics::size measure(double width_constraint, double height_constraint) override;
        maui::graphics::size arrange(const maui::graphics::rect& bounds) override;

        // --- platform configuration (W2-24): the iOSSpecific Page faces ----------------------------------
        // C# Page.cs's explicit IiOSPageSpecifics implementation (each getter carries the oracle's
        // parent-redirect quirk: when the logical parent is a page that has the HOME-INDICATOR knob set —
        // yes, all three getters probe that one key — the parent's value wins). Defined in content_page.cpp.
        [[nodiscard]] bool is_home_indicator_auto_hidden() const override;
        [[nodiscard]] int prefers_status_bar_hidden_mode() const override;
        [[nodiscard]] int preferred_status_bar_update_animation_mode() const override;

        // C# Page.cs: ISafeAreaView.IgnoreSafeArea => !On<iOS>().UsingSafeArea();
        // ISafeAreaView2.SafeAreaInsets set => On<iOS>().SetSafeAreaInsets(value).
        [[nodiscard]] bool ignore_safe_area() const override;
        void set_safe_area_insets(const maui::core::thickness& value) override;

        // C# ContentPage.ISafeAreaView2.GetSafeAreaRegionsForEdge: when SafeAreaEdgesProperty IS set, the
        // edge's region from the property; otherwise the legacy IgnoreSafeArea fallback
        // (ignore → None, else Container). The iOS host (MauiView.AdjustForSafeArea) consults this per edge.
        [[nodiscard]] maui::core::safe_area_regions get_safe_area_regions_for_edge(int edge) const override;

    private:
        // Padding + (when UseSafeArea) the realized safe-area insets — the MauiView.AdjustForSafeArea
        // analog folded into the measure/arrange inset (content_page.cpp).
        [[nodiscard]] maui::core::thickness layout_inset() const;
        // --- end platform configuration (W2-24) -----------------------------------------------------------

    protected:
        // The content child plus the chrome items are this page's logical children, so BindingContext +
        // Window inherit down to them (C# Page propagates into ToolbarItems/MenuBarItems too). content_
        // is the i_view contract; cross-cast to the element base every control shares.
        void for_each_logical_child(const std::function<void(element&)>& visit) const override
        {
            if (auto* child = dynamic_cast<element*>(content_))
            {
                visit(*child);
            }
            for (toolbar_item* const item : toolbar_items_.items())
            {
                visit(*item);
            }
            for (menu_bar_item* const item : menu_bar_items_.items())
            {
                visit(*item);
            }
        }

        // Generic mount (app_host): re-host the page content on the now-attached handler (the same
        // "set_content" command notify_content_changed fires when the content changes with a handler present).
        void mount_into_handler() override
        {
            notify_content_changed();
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

        // C# ContentPage.UpdateHideSoftInputOnTapped: re-run the HideSoftInputOnTapped mapping so the
        // manager re-evaluates this page (the appearing hook + the bindable change both funnel here).
        // Defined out-of-line (content_page.cpp) to keep the handler type out of this header.
        void update_hide_soft_input_on_tapped();

        maui::core::i_view* content_ = nullptr; // NON-owning: the caller owns the content's lifetime
        maui::core::property<maui::core::thickness> padding_{*this, padding_property()};
        maui::core::property<std::string> title_{*this, title_property()};
        // ContentPage.HideSoftInputOnTapped (default false); a change routes through on_property_changed →
        // handler->update_value("hide_soft_input_on_tapped").
        maui::core::property<bool> hide_soft_input_on_tapped_{*this, hide_soft_input_on_tapped_property()};
        // ContentPage.SafeAreaEdges (per-element default SafeAreaEdges.None via the descriptor's
        // default-value creator); a change routes through on_property_changed → update_value("safe_area_edges").
        maui::core::property<maui::core::safe_area_edges> safe_area_edges_{*this, safe_area_edges_property()};
        // chrome (W1-11): the page's chrome item collections — items parent to this page on add and
        // un-parent on remove (Page's collection-changed parenting).
        menu_element_list<toolbar_item> toolbar_items_{[this](toolbar_item& item) { attach_logical_child(item); },
                                                       [](toolbar_item& item) { detach_logical_child(item); }};
        menu_element_list<menu_bar_item> menu_bar_items_{[this](menu_bar_item& item) { attach_logical_child(item); },
                                                         [](menu_bar_item& item) { detach_logical_child(item); }};
        bool has_appeared_ = false; // Page._hasAppeared — gates the idempotent appearing/disappearing pair
    };
} // namespace maui::controls
