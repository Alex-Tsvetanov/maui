#pragma once
// maui::controls::shell_content  <=  Microsoft.Maui.Controls.ShellContent
//
// One page slot inside a shell_section tab: either a live Content page (set eagerly) or a
// ContentTemplate that creates the page LAZILY on first display (IShellContentController
// .GetOrCreateContent). Ported from ShellContent.cs.
//
// Port shape notes:
//   - content() is a NON-owning content_page* (the caller owns an eagerly-set page, PROFILE §8);
//     a TEMPLATE-created page is owned by this shell_content (the C# ContentCache keeps it alive).
//   - adopt(page) is the explicit factory replacing C#'s `implicit operator ShellContent(
//     TemplatedPage)` (no implicit conversions per PROFILE): it wraps the page in a new
//     shell_content with the IMPL_ route and a live Title sync (the C# Title binding; the port's
//     content_page has no Icon, so the Icon/FlyoutIcon bindings have no source and are dropped).
//   - query parameters: apply_query_attributes stores the dictionary and forwards it to the created
//     page when the page (or its later creation) implements i_query_attributable. The C#
//     [QueryProperty] reflection path and the BindingContext fan-out are not ported (i_query_attributable
//     is the port's single delivery channel — see i_query_attributable.hpp).
//   - MenuItems, EvaluateDisconnect (DI-created page teardown) and the Window-property hooks are out
//     of scope (no DI page activation, no native chrome this unit; STATUS.md).

#include <memory>
#include <stdexcept>

#include "maui/controls/shell/base_shell_item.hpp"
#include "maui/controls/shell/shell_route_parameters.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/core/event.hpp"

namespace maui::controls
{
    class content_page;
    class shell_section;

    class shell_content final : public base_shell_item
    {
    public:
        shell_content() = default;
        ~shell_content() override;
        shell_content(const shell_content&) = delete;
        shell_content(shell_content&&) = delete;
        shell_content& operator=(const shell_content&) = delete;
        shell_content& operator=(shell_content&&) = delete;

        // ---- Content (eager) ----
        [[nodiscard]] content_page* content() const
        {
            return content_;
        }
        // OnContentChanged: with no template the page becomes the displayed cache immediately; the
        // parent shell_item is notified (SendStructureChanged).
        void set_content(content_page* value);

        // ---- ContentTemplate (lazy) ----
        [[nodiscard]] const std::shared_ptr<data_template>& content_template() const
        {
            return content_template_;
        }
        void set_content_template(std::shared_ptr<data_template> value)
        {
            content_template_ = std::move(value);
        }

        // IShellContentController.Page — the displayed page if it exists (never creates).
        [[nodiscard]] content_page* page() const
        {
            return content_cache_;
        }

        // IShellContentController.GetOrCreateContent: the eager Content, or the template-created page
        // (cached). Throws std::runtime_error when neither yields a page (C# InvalidOperationException
        // "No Content found"). The C# TabbedPage/FlyoutPage/NavigationPage rejections are structural
        // here (content is typed content_page*).
        [[nodiscard]] content_page* get_or_create_content();

        // The explicit `implicit operator ShellContent(TemplatedPage)`: an already-wrapped page
        // returns its existing wrapper; otherwise a fresh shell_content adopts the page (IMPL_ route +
        // live Title sync).
        [[nodiscard]] static std::shared_ptr<shell_content> adopt(content_page& page);

        // ---- query parameters (ShellContent.ApplyQueryAttributes + the attached QueryAttributes) ----
        void apply_query_attributes(const shell_route_parameters& query) override;
        [[nodiscard]] const shell_route_parameters& query_attributes() const
        {
            return query_attributes_;
        }
        // C# IsSet(QueryAttributesProperty) — were attributes ever propagated to this content?
        [[nodiscard]] bool has_query_attributes() const
        {
            return query_attributes_set_;
        }

        // ShellContent.IsVisibleContent: this is the current item of a visible section (the port has
        // no modal stack, so the C# modal guard collapses away).
        [[nodiscard]] bool is_visible_content() const;

        // ShellContent.SendAppearing only fires once a page exists; both fan the lifecycle into it.
        void send_appearing() override;
        void send_disappearing() override;

        // BaseShellItem.OnAppearing(Action): when this content has not appeared but its section's
        // nav stack carries pushed pages, the action is delegated to the TOP nav-stack page (whose
        // own OnAppearing runs it now if that page has appeared, else queues on its Appearing). This
        // is the Navigation.NavigationStack branch the base on_appearing lacks.
        void on_appearing(std::function<void()> action) override;

    protected:
        // The displayed page is this node's logical child (BindingContext inheritance).
        void for_each_logical_child(const std::function<void(element&)>& visit) const override;

    private:
        // The ContentCache property setter: swap the logical child + tell the section to re-resolve
        // its displayed page.
        void set_content_cache(content_page* value);

        [[nodiscard]] shell_section* parent_section() const;

        content_page* content_ = nullptr;          // NON-owning (caller-owned eager content)
        content_page* content_cache_ = nullptr;    // the displayed page (C# _contentCache)
        std::shared_ptr<content_page> owned_page_; // owns a TEMPLATE-created page only
        std::shared_ptr<data_template> content_template_;
        shell_route_parameters query_attributes_;           // the attached QueryAttributesProperty store
        bool query_attributes_set_ = false;                 // C# IsSet(QueryAttributesProperty)
        maui::core::scoped_connection adopted_title_token_; // adopt()'s live Title sync
    };
} // namespace maui::controls
