#pragma once
// maui::controls::shell  <=  Microsoft.Maui.Controls.Shell (model/ownership/navigation surface)
//
// The developer-facing shell: an OWNED tree of shell_items (each owning shell_sections owning
// shell_contents), the CurrentItem selection chain, URI navigation (go_to_async over the
// shell_navigation_manager), the Navigating/Navigated events, and the flyout MODEL state
// (FlyoutBehavior / FlyoutIsPresented). Ported from Shell.cs.
//
// THIS UNIT IS HEADLESS-ONLY: there is NO shell handler / native chrome (flyout drawer, tab bars,
// toolbars are wave 4) — shell derives view<i_view> so the handler seam can attach later, but
// nothing maps its properties yet. Out of scope (documented, STATUS.md): the modal stack, the
// flyout content/header/footer/templates, ShellToolbar/SearchHandler/MenuShellItem, appearance
// observers, the back-button behavior surface, and Shell.Current (no window-attachment this unit).
//
// Implicit-conversion seams: C# lets you add a ShellSection / ShellContent / page wherever a
// ShellItem is expected (implicit-wrapping operators). Per PROFILE (no implicit conversions) those
// are explicit add_item / set_current_item OVERLOADS that wrap through the same
// create_from_shell_section / create_from_shell_content / adopt factories.

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "maui/controls/shell/flyout_behavior.hpp"
#include "maui/controls/shell/flyout_header_behavior.hpp"
#include "maui/controls/shell/shell_appearance.hpp"
#include "maui/controls/shell/shell_item.hpp"
#include "maui/controls/shell/shell_navigated_event_args.hpp"
#include "maui/controls/shell/shell_navigating_event_args.hpp"
#include "maui/controls/shell/shell_navigation_manager.hpp"
#include "maui/controls/shell/shell_navigation_source.hpp"
#include "maui/controls/shell/shell_navigation_state.hpp"
#include "maui/controls/shell/shell_route_parameters.hpp"
#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/property.hpp"

namespace maui::controls
{
    class content_page;
    class search_handler;
    class data_template;

    class shell : public view<maui::core::i_view>
    {
    public:
        shell();
        ~shell() override;
        shell(const shell&) = delete;
        shell(shell&&) = delete;
        shell& operator=(const shell&) = delete;
        shell& operator=(shell&&) = delete;

        // Shared bindable-property descriptors (Shell.FlyoutBehaviorProperty — plain instead of
        // attached here — and Shell.FlyoutIsPresentedProperty).
        static const maui::core::bindable_property<flyout_behavior>& flyout_behavior_property();
        static const maui::core::bindable_property<bool>& flyout_is_presented_property();
        // Shell.FlyoutHeaderBehaviorProperty (BindingMode.OneTime, default FlyoutHeaderBehavior.Default).
        static const maui::core::bindable_property<flyout_header_behavior>& flyout_header_behavior_property();

        // ---- Items (Shell.Items — owned children) ----
        [[nodiscard]] const std::vector<std::shared_ptr<shell_item>>& items() const
        {
            return items_;
        }
        void add_item(std::shared_ptr<shell_item> item);
        // The C# implicit-wrapping adds: a section / content / bare page lands wrapped in IMPL_ nodes.
        std::shared_ptr<shell_item> add_item(std::shared_ptr<shell_section> section);
        std::shared_ptr<shell_item> add_item(std::shared_ptr<shell_content> content);
        std::shared_ptr<shell_item> add_item(content_page& page);
        void remove_item(const shell_item& item);
        // IShellController.GetItems — the visible items, point-in-time.
        [[nodiscard]] std::vector<shell_item*> visible_items() const;

        // ---- CurrentItem / the selection chain ----
        [[nodiscard]] shell_item* current_item() const
        {
            return current_item_;
        }
        // Shell.CurrentItem setter (OnCurrentItemChanging adds a missing item to Items first).
        void set_current_item(const std::shared_ptr<shell_item>& item);
        // The implicit-conversion setters: select an (already- or newly-wrapped) section / content.
        void set_current_item(const std::shared_ptr<shell_section>& section);
        void set_current_item(const std::shared_ptr<shell_content>& content);
        void set_current_item(content_page& page);

        [[nodiscard]] shell_section* current_section() const; // Shell.CurrentSection
        [[nodiscard]] shell_content* current_content() const; // Shell.CurrentContent
        // Shell.CurrentPage — the presented page of the current section (no modal overlay here).
        [[nodiscard]] content_page* current_page() const;

        // ---- CurrentState (read-only bindable in C#) ----
        [[nodiscard]] const shell_navigation_state* current_state() const
        {
            return current_state_ ? &*current_state_ : nullptr;
        }

        // ---- navigation ----
        // Shell.GoToAsync(state[, animate][, parameters]) — synchronous-with-suspension (see the
        // navigation manager header): returns once the navigation completed, was cancelled, or was
        // suspended behind a deferral.
        void go_to_async(const shell_navigation_state& state, std::optional<bool> animate = std::nullopt,
                         std::optional<shell_route_parameters> parameters = std::nullopt);

        // The Shell.Navigation facade (NavigationProxy → ShellSection.NavigationImpl): the stack of
        // the current section (slot 0 is the nullptr root marker, like C#'s NavigationStack[0]).
        [[nodiscard]] const std::vector<content_page*>& navigation_stack() const;
        void navigation_push(content_page& page, std::optional<bool> animated = std::nullopt);
        void navigation_pop(std::optional<bool> animated = std::nullopt);
        void navigation_pop_to_root(std::optional<bool> animated = std::nullopt);
        void navigation_remove_page(content_page& page);
        void navigation_insert_page_before(content_page& page, content_page& before);

        // ---- events ----
        maui::core::event<shell_navigating_event_args&> navigating;
        maui::core::event<const shell_navigated_event_args&> navigated;
        maui::core::event<> structure_changed;    // IShellController.StructureChanged
        maui::core::event<> flyout_items_changed; // IShellController.FlyoutItemsChanged

        void send_structure_changed()
        {
            structure_changed.raise();
        }
        void send_flyout_items_changed()
        {
            flyout_items_changed.raise();
        }

        // ---- flyout model state ----
        [[nodiscard]] flyout_behavior get_flyout_behavior() const
        {
            return flyout_behavior_.get();
        }
        void set_flyout_behavior(flyout_behavior value)
        {
            flyout_behavior_.set(value);
        }
        [[nodiscard]] bool flyout_is_presented() const
        {
            return flyout_is_presented_.get();
        }
        void set_flyout_is_presented(bool value)
        {
            flyout_is_presented_.set(value);
        }
        // Shell.GetEffectiveFlyoutBehavior, collapsed to the shell's own value (the C# walk consults
        // the attached property up the current page's chain — attached storage is out of scope).
        [[nodiscard]] flyout_behavior effective_flyout_behavior() const
        {
            return flyout_behavior_.get();
        }

        // ---- flyout header / footer (Shell.FlyoutHeader/FlyoutFooter + their *Template props) ----
        // Shell.FlyoutHeader / FlyoutFooter (C# `object`, BindingMode.OneTime). In C# these accept any
        // object but the renderer uses only `value as View` (ShellTemplatedViewManager.OnViewDataChanged
        // only applies a header view when no Template is set). The port models the View directly as a
        // shared_ptr<i_view> (the resolved chrome content); a Template, when set, overrides it. Setting
        // either re-resolves _flyoutHeaderView/_flyoutFooterView and notifies the handler so the chrome
        // re-materializes (C# OnFlyoutHeaderChanged → Handler.UpdateValue path via the renderer's
        // PropertyChanged subscription).
        void set_flyout_header(std::shared_ptr<maui::core::i_view> value);
        void set_flyout_footer(std::shared_ptr<maui::core::i_view> value);
        [[nodiscard]] maui::core::i_view* flyout_header() const
        {
            return flyout_header_.get();
        }
        [[nodiscard]] maui::core::i_view* flyout_footer() const
        {
            return flyout_footer_.get();
        }
        // Shell.FlyoutHeaderTemplate / FlyoutFooterTemplate (DataTemplate, BindingMode.OneTime). When set,
        // the resolved header/footer view is Template.CreateContent() (cast to View) instead of the raw
        // FlyoutHeader/Footer (ShellTemplatedViewManager.OnViewTemplateChanged). Setting it re-resolves.
        void set_flyout_header_template(std::shared_ptr<data_template> value);
        void set_flyout_footer_template(std::shared_ptr<data_template> value);
        [[nodiscard]] const std::shared_ptr<data_template>& flyout_header_template() const
        {
            return flyout_header_template_;
        }
        [[nodiscard]] const std::shared_ptr<data_template>& flyout_footer_template() const
        {
            return flyout_footer_template_;
        }
        // The RESOLVED header/footer view (C# Shell.FlyoutHeaderView / FlyoutFooterView): the
        // Template.CreateContent() result when a Template is set, else the raw FlyoutHeader/Footer view.
        // null when nothing resolves. The chrome (the handler rebuild) reads these.
        [[nodiscard]] maui::core::i_view* flyout_header_view() const
        {
            return flyout_header_view_.get();
        }
        [[nodiscard]] maui::core::i_view* flyout_footer_view() const
        {
            return flyout_footer_view_.get();
        }

        // Shell.FlyoutHeaderBehavior — how the header behaves on flyout scroll.
        [[nodiscard]] flyout_header_behavior get_flyout_header_behavior() const
        {
            return flyout_header_behavior_.get();
        }
        void set_flyout_header_behavior(flyout_header_behavior value)
        {
            flyout_header_behavior_.set(value);
        }

        // ---- routes (Shell.Route/RouteHost/RouteScheme) ----
        [[nodiscard]] std::string route() const;
        [[nodiscard]] const std::string& route_host() const
        {
            return route_host_;
        }
        [[nodiscard]] const std::string& route_scheme() const
        {
            return route_scheme_;
        }

        [[nodiscard]] shell_navigation_manager& navigation_manager()
        {
            return navigation_manager_;
        }

        // ---- Shell.SearchHandler (attached property) ----
        // Shell.SetSearchHandler / GetSearchHandler — the search surface installed onto a page (the common
        // case) or the shell itself. Ported from Shell.SearchHandlerProperty (an attached BindableProperty,
        // BindingMode.OneTime). The port has no central attached-property store, so (exactly like
        // routing's element-route side map) the handler is held in a process-wide side map keyed by the
        // TARGET bindable_object pointer; the C# OnSearchHandlerPropertyChanged inherited-binding-context
        // wiring is reproduced — setting a handler flows the target's binding context into it; clearing
        // detaches it. The chrome resolves get_search_handler(current_page) (falling back to the shell).
        // The target object's destructor must call remove_search_handler (lifetime hygiene); content_page
        // does so in its dtor, like base_shell_item does for routes.
        static void set_search_handler(maui::core::bindable_object& target, std::shared_ptr<search_handler> handler);
        [[nodiscard]] static search_handler* get_search_handler(const maui::core::bindable_object& target);
        // The owning handle (so the chrome can keep it alive while installed), or null when unset.
        [[nodiscard]] static std::shared_ptr<search_handler> get_search_handler_shared(
            const maui::core::bindable_object& target);
        static void remove_search_handler(const maui::core::bindable_object& target);

        // ---- Shell appearance attached properties (Shell.<Color>Property, all attached) ----
        // The ten chrome-color attached properties (Shell.BackgroundColor / ForegroundColor / TitleColor /
        // DisabledColor / UnselectedColor + the five TabBar* counterparts) plus FlyoutWidth / FlyoutHeight.
        // In C# these are BindableProperty.CreateAttached settable on ANY element of the shell tree (the
        // page, a ShellContent, a ShellSection, a ShellItem, or the Shell). The port has no central
        // attached-property store, so — exactly like routing's element-route side map and Shell.SearchHandler
        // — each element's set values live in a process-wide side map keyed by the element pointer (with a
        // weak liveness token so a dead entry self-prunes; see shell.cpp). The resolve walk
        // (get_appearance_for_pivot) ingests these up the hierarchy.
        //
        // The per-element set values (the appearance attached-property bag for one element). Each color /
        // double is std::nullopt when that element never set it (the C# attached-property "unset" / IsSet
        // false). Public so shell_appearance::ingest can read a pivot's bag through values_of.
        struct appearance_values
        {
            std::optional<maui::graphics::color> background_color;
            std::optional<maui::graphics::color> disabled_color;
            std::optional<maui::graphics::color> foreground_color;
            std::optional<maui::graphics::color> tab_bar_background_color;
            std::optional<maui::graphics::color> tab_bar_disabled_color;
            std::optional<maui::graphics::color> tab_bar_foreground_color;
            std::optional<maui::graphics::color> tab_bar_title_color;
            std::optional<maui::graphics::color> tab_bar_unselected_color;
            std::optional<maui::graphics::color> title_color;
            std::optional<maui::graphics::color> unselected_color;
            std::optional<double> flyout_width;
            std::optional<double> flyout_height;
            [[nodiscard]] bool any_set() const;
        };

        static void set_background_color(element& target, maui::graphics::color value);
        static void set_disabled_color(element& target, maui::graphics::color value);
        static void set_foreground_color(element& target, maui::graphics::color value);
        static void set_tab_bar_background_color(element& target, maui::graphics::color value);
        static void set_tab_bar_disabled_color(element& target, maui::graphics::color value);
        static void set_tab_bar_foreground_color(element& target, maui::graphics::color value);
        static void set_tab_bar_title_color(element& target, maui::graphics::color value);
        static void set_tab_bar_unselected_color(element& target, maui::graphics::color value);
        static void set_title_color(element& target, maui::graphics::color value);
        static void set_unselected_color(element& target, maui::graphics::color value);
        static void set_flyout_width(element& target, double value);
        static void set_flyout_height(element& target, double value);

        [[nodiscard]] static std::optional<maui::graphics::color> get_background_color(const element& target);
        [[nodiscard]] static std::optional<maui::graphics::color> get_disabled_color(const element& target);
        [[nodiscard]] static std::optional<maui::graphics::color> get_foreground_color(const element& target);
        [[nodiscard]] static std::optional<maui::graphics::color> get_tab_bar_background_color(const element& target);
        [[nodiscard]] static std::optional<maui::graphics::color> get_tab_bar_disabled_color(const element& target);
        [[nodiscard]] static std::optional<maui::graphics::color> get_tab_bar_foreground_color(const element& target);
        [[nodiscard]] static std::optional<maui::graphics::color> get_tab_bar_title_color(const element& target);
        [[nodiscard]] static std::optional<maui::graphics::color> get_tab_bar_unselected_color(const element& target);
        [[nodiscard]] static std::optional<maui::graphics::color> get_title_color(const element& target);
        [[nodiscard]] static std::optional<maui::graphics::color> get_unselected_color(const element& target);
        [[nodiscard]] static std::optional<double> get_flyout_width(const element& target);
        [[nodiscard]] static std::optional<double> get_flyout_height(const element& target);

        // The set values bag for `target` (an empty bag when the element set nothing). The reference is a
        // by-value copy (the side map may rehash). shell_appearance::ingest reads it.
        [[nodiscard]] static appearance_values values_of(const element& target);
        // Side-map hygiene (the routing::remove_route precedent): drop an element's appearance bag. The weak
        // token self-prunes a dead entry on access, so this is only an eager cleanup — optional to call.
        static void remove_appearance_values(const element& target);

        // Shell.GetAppearanceForPivot: the effective appearance for `pivot`. Walk DOWN from the pivot to the
        // current page (WalkToPage), then back UP to the root shell, ingesting each level's set attached
        // values (lowest level wins per slot). Returns nullopt when nothing in the line set any value
        // (the C# null return → the chrome keeps its system defaults). See shell.cpp for the exact walk.
        // STATIC: the C# method is an instance member but reads nothing off the shell — the walk runs entirely
        // from the pivot up its own parent chain — so it is a pure function of `pivot` (callers resolve the
        // shell only to find the pivot/current page; the resolution itself doesn't need it).
        [[nodiscard]] static std::optional<shell_appearance> get_appearance_for_pivot(element& pivot);

        // ---- IShellController ----
        // ProposeNavigation: route the UI-initiated change through Navigating (cancel/defer).
        bool propose_navigation(shell_navigation_source source, shell_item* item, shell_section* section,
                                shell_content* content, const std::vector<content_page*>* stack, bool can_cancel);
        // UpdateCurrentState: recompose CurrentState; on change fire (or accumulate) Navigated.
        void update_current_state(shell_navigation_source source);
        // OnFlyoutItemSelected(Async): navigate to a flyout element (item/section/content).
        void on_flyout_item_selected(base_shell_item& element);

    protected:
        // The C# OnNavigating/OnNavigated virtual hooks (TestShell overrides them).
        virtual void on_navigating(shell_navigating_event_args& args)
        {
            (void)args;
        }
        virtual void on_navigated(const shell_navigated_event_args& args)
        {
            (void)args;
        }

        // The items are the shell's logical children (BindingContext inheritance).
        void for_each_logical_child(const std::function<void(element&)>& visit) const override;

    private:
        friend class shell_navigation_manager; // drives set_current_item_core during go_to

        void on_flyout_item_selected(base_shell_item& element, bool platform_initiated);
        // The shared CurrentItem write (OnCurrentItemChanging + OnCurrentItemChanged).
        void set_current_item_core(const std::shared_ptr<shell_item>& item);
        // The owning handle of a borrowed item pointer (null when the item is not in items()).
        [[nodiscard]] std::shared_ptr<shell_item> owner_of(const shell_item* item) const;
        // Shell.Initialize's SetCurrentItem: ensure a valid current item after an items change.
        void auto_select_current_item();
        // BaseShellItem.IsChecked maintenance across the whole tree (Shell.UpdateChecked).
        void update_checked();

        // Re-resolve _flyoutHeaderView from FlyoutHeader / FlyoutHeaderTemplate (ShellTemplatedViewManager:
        // Template wins, else the raw header view), then notify the handler ("flyout_header"). Same for the
        // footer ("flyout_footer"). Adds/removes the resolved view as a logical child for context inheritance.
        void resolve_flyout_header_view();
        void resolve_flyout_footer_view();

        std::vector<std::shared_ptr<shell_item>> items_; // OWNED children
        shell_item* current_item_ = nullptr;             // borrowed view of one items_ entry
        std::optional<shell_navigation_state> current_state_;
        maui::core::property<flyout_behavior> flyout_behavior_{*this, flyout_behavior_property()};
        maui::core::property<bool> flyout_is_presented_{*this, flyout_is_presented_property()};
        maui::core::property<flyout_header_behavior> flyout_header_behavior_{*this, flyout_header_behavior_property()};
        // The raw FlyoutHeader / FlyoutFooter views (the `object as View` slot) and their DataTemplates;
        // flyout_header_view_ / flyout_footer_view_ are the RESOLVED views (Template result or raw view) the
        // chrome reads. The views are OWNED here (the C# tree keeps them alive as logical children).
        std::shared_ptr<maui::core::i_view> flyout_header_;
        std::shared_ptr<maui::core::i_view> flyout_footer_;
        std::shared_ptr<data_template> flyout_header_template_;
        std::shared_ptr<data_template> flyout_footer_template_;
        std::shared_ptr<maui::core::i_view> flyout_header_view_;
        std::shared_ptr<maui::core::i_view> flyout_footer_view_;
        std::string route_host_ = "shell";
        std::string route_scheme_ = "app";
        // The manager is declared AFTER the state it reads and BEFORE the forwarding tokens, so the
        // tokens disconnect before the manager (their publisher) dies (§8).
        shell_navigation_manager navigation_manager_{*this};
        maui::core::scoped_connection navigating_token_;
        maui::core::scoped_connection navigated_token_;
    };
} // namespace maui::controls
