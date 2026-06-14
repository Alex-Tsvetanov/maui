#pragma once
// maui::controls::base_shell_item  <=  Microsoft.Maui.Controls.BaseShellItem
//
// The base of every node in the shell item tree (shell_item / shell_section / shell_content):
// Title / Icon / FlyoutIcon / IsEnabled / IsVisible / FlyoutItemIsVisible / IsChecked as bindable
// properties, the Route accessor over the routing registry, and the Appearing/Disappearing
// lifecycle pair (idempotent via _hasAppearing, exactly like Page's). Ported from BaseShellItem.cs.
//
// Port shape notes:
//   - The tree is OWNED top-down with shared_ptr (PROFILE §8): a parent collection holds
//     shared_ptr children; the logical-parent back-reference is element's non-owning
//     logical_parent(). enable_shared_from_this lets the implicit-wrapping factories recover the
//     owner handle of an already-parented node (C# just casts Parent).
//   - Icon/FlyoutIcon are shared_ptr<i_image_source> like the image control; OnIconChanged's
//     "Icon also seeds FlyoutIcon unless FlyoutIcon was set" mirror is kept.
//   - on_appearing(action) is C#'s internal OnAppearing(Action): run now when appeared, else queue
//     until the next appearing (the navigated-event gate). The C# forwarding to the top of the
//     nav/modal stack is collapsed — the port's shell drives appearing through the same chain.
//   - The flyout default-template machinery (CreateDefaultFlyoutItemCell), IVisual/FlowDirection
//     controllers, and the Window attached property are out of scope (native flyout chrome is a
//     later wave; STATUS.md).

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/element.hpp"
#include "maui/controls/shell/routing.hpp"
#include "maui/controls/shell/shell_route_parameters.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/property.hpp"

namespace maui::controls
{
    class shell; // forward — find_parent_shell walks up to it

    class base_shell_item : public element, public std::enable_shared_from_this<base_shell_item>
    {
    public:
        base_shell_item() = default;
        // Drops this element's routing side-map entry (the port's lifetime hygiene; see routing.hpp).
        ~base_shell_item() override;
        base_shell_item(const base_shell_item&) = delete;
        base_shell_item(base_shell_item&&) = delete;
        base_shell_item& operator=(const base_shell_item&) = delete;
        base_shell_item& operator=(base_shell_item&&) = delete;

        // Shared bindable-property descriptors (one instance per type, like BaseShellItem.*Property).
        static const maui::core::bindable_property<std::string>& title_property();
        static const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>>& icon_property();
        static const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>>& flyout_icon_property();
        static const maui::core::bindable_property<bool>& is_enabled_descriptor();
        static const maui::core::bindable_property<bool>& is_checked_property();
        static const maui::core::bindable_property<bool>& is_visible_descriptor();
        static const maui::core::bindable_property<bool>& flyout_item_is_visible_property();

        // ---- Title / Icon / FlyoutIcon ----
        [[nodiscard]] std::string_view title() const
        {
            return title_.get();
        }
        void set_title(std::string value)
        {
            title_.set(std::move(value));
        }

        [[nodiscard]] std::shared_ptr<maui::core::i_image_source> icon() const
        {
            return icon_.get();
        }
        // OnIconChanged: a non-null Icon also seeds FlyoutIcon when FlyoutIcon was never set itself.
        void set_icon(std::shared_ptr<maui::core::i_image_source> value);

        [[nodiscard]] std::shared_ptr<maui::core::i_image_source> flyout_icon() const
        {
            return flyout_icon_.get();
        }
        void set_flyout_icon(std::shared_ptr<maui::core::i_image_source> value)
        {
            flyout_icon_.set(std::move(value));
        }

        // ---- IsEnabled / IsChecked / IsVisible / FlyoutItemIsVisible ----
        [[nodiscard]] bool is_enabled() const
        {
            return is_enabled_.get();
        }
        void set_is_enabled(bool value)
        {
            is_enabled_.set(value);
        }

        // Read-only to developers (BaseShellItem.IsChecked); the shell drives it (UpdateChecked).
        [[nodiscard]] bool is_checked() const
        {
            return is_checked_.get();
        }

        [[nodiscard]] bool is_visible() const
        {
            return is_visible_.get();
        }
        void set_is_visible(bool value)
        {
            is_visible_.set(value);
        }

        [[nodiscard]] bool flyout_item_is_visible() const
        {
            return flyout_item_is_visible_.get();
        }
        void set_flyout_item_is_visible(bool value)
        {
            flyout_item_is_visible_.set(value);
        }

        // ---- Route (over the routing registry; BaseShellItem.Route) ----
        [[nodiscard]] std::string route() const
        {
            return routing::get_route(*this);
        }
        // Validates sibling uniqueness for user-defined routes (Routing.ValidateForDuplicates) —
        // throws std::invalid_argument on a clash — then stores the route.
        void set_route(std::string value);

        // ---- Appearing / Disappearing (BaseShellItem's idempotent pair) ----
        maui::core::event<> appearing;
        maui::core::event<> disappearing;
        virtual void send_appearing();
        virtual void send_disappearing();
        [[nodiscard]] bool has_appeared() const
        {
            return has_appearing_;
        }
        // C# internal OnAppearing(Action): run now if appeared, else once on the next appearing.
        // VIRTUAL — shell_content overrides it to delegate to the section's nav-stack top page when the
        // content itself has not appeared but pushed pages are on the stack (BaseShellItem.OnAppearing's
        // Navigation.NavigationStack branch).
        virtual void on_appearing(std::function<void()> action);

        // ---- query parameters (BaseShellItem.ApplyQueryAttributes — overridden by shell_content) ----
        virtual void apply_query_attributes(const shell_route_parameters& query)
        {
            (void)query;
        }

        // BaseShellItem.IsPartOfVisibleTree: this node is among its parent's VISIBLE items.
        [[nodiscard]] bool is_part_of_visible_tree() const;

        // The nearest shell ancestor (FindParentOfType<Shell>), or null.
        [[nodiscard]] shell* find_parent_shell() const;

    protected:
        // The C# OnAppearing/OnDisappearing template hooks (called before the event fires).
        virtual void on_appearing_core()
        {
        }
        virtual void on_disappearing_core()
        {
        }

        // The shell sets is_checked (BaseShellItem.IsCheckedPropertyKey is internal-set in C# too).
        void set_is_checked(bool value)
        {
            is_checked_.set(value);
        }

        // The siblings used by the route-duplication validation: every base_shell_item child of this
        // node's logical parent (collected by the typed containers; empty when unparented).
        [[nodiscard]] std::vector<const maui::core::bindable_object*> route_validation_siblings() const;

        friend class shell; // UpdateChecked drives set_is_checked across the tree

    private:
        maui::core::property<std::string> title_{*this, title_property()};
        maui::core::property<std::shared_ptr<maui::core::i_image_source>> icon_{*this, icon_property()};
        maui::core::property<std::shared_ptr<maui::core::i_image_source>> flyout_icon_{*this, flyout_icon_property()};
        maui::core::property<bool> is_enabled_{*this, is_enabled_descriptor()};
        maui::core::property<bool> is_checked_{*this, is_checked_property()};
        maui::core::property<bool> is_visible_{*this, is_visible_descriptor()};
        maui::core::property<bool> flyout_item_is_visible_{*this, flyout_item_is_visible_property()};
        std::vector<std::function<void()>> pending_appearing_; // queued on_appearing actions
        bool has_appearing_ = false;                           // BaseShellItem._hasAppearing
    };
} // namespace maui::controls
