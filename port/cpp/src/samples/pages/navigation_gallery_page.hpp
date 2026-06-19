#pragma once
// maui::samples::navigation_gallery_page — ports NavigationGallery.xaml (+ .xaml.cs).
//
// The C# NavigationGallery is a BasePage whose buttons drive the AMBIENT Navigation stack of the
// NavigationPage hosting it (Navigation.PushAsync / PopAsync / PopToRootAsync / InsertPageBefore /
// RemovePage), with a "Page Count" label echoing how many gallery instances exist. The gallery host
// here mounts a single content_page (page()), so there is no live visual navigation host to push into.
// We therefore exercise the SAME INavigation surface on a PAGE-OWNED navigation_page (the stack), and
// echo the resulting stack DEPTH + the current/title list into a readout label — the code-first,
// headless-safe analog of the live push/pop the C# sample performs on its ambient stack.
//
// Mapped from NavigationGallery.xaml.cs:
//   - "Push Page"           → nav_.push(new page)           (Navigation.PushAsync)
//   - "Pop Page"            → nav_.pop()                    (Navigation.PopAsync)
//   - "Insert Page Before…" → nav_.insert_page_before(new, current)  (Navigation.InsertPageBefore)
//   - "Remove Page Before…" → nav_.remove_page(stack[count-2])       (Navigation.RemovePage)
//   - "Pop To Root"         → nav_.pop_to_root()            (Navigation.PopToRootAsync)
//   - "Toggle Secondary Toolbar Item" → add/remove two secondary toolbar_items on the current page
//     (the C# ToggleSecondaryToolbarItem branch — a page chrome edit, fully modeled here).
// The C# SwapRoot / ToggleNavigationBar / ToggleBackButton branches lean on a LIVE IStackNavigationView
// host (RequestNavigation) / the NavigationPage.HasNavigationBar & HasBackButton attached properties,
// which need a real navigation host + the platform-specific attached-property store; those are noted as
// best-effort omissions below (the stack edits + toolbar toggle cover the demonstrated INavigation set).
//
// The page OWNS its whole element tree (the sample_app pattern): the readout, the buttons, the stack,
// the gallery content_page (page()), AND the page-owned navigation_page + the pool of content_pages it
// pushes/pops (the nav stack is NON-owning, so the pages must outlive it — they are members here).
// It is backend-agnostic; attach_handlers attaches every VIEW bottom-up via gallery_attach.hpp and
// re-hosts the tree built in the ctor. The navigation_page is NOT mounted in the visual tree (it is the
// page-owned stack under test), so it is intentionally excluded from the attach/re-host walk.

#include <array>
#include <cstdio>
#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/navigation_page.hpp"
#include "maui/controls/toolbar_item.hpp"
#include "maui/controls/toolbar_item_order.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class navigation_gallery_page
    {
    public:
        navigation_gallery_page()
        {
            page_.set_title("Navigation gallery");
            stack_.set_spacing(8);

            // The page-owned navigation stack under test, seeded with the first pooled page as its root
            // (C# NavigationPage(root)). Each pooled page carries a "PAGE NUMBER n" title — the C# sample's
            // per-instance Title — so the readout can list the live stack's titles.
            for (std::size_t i = 0; i < pool_.size(); ++i)
            {
                pool_[i].set_title("PAGE NUMBER " + std::to_string(i + 1));
            }
            // nav_ holds the root pool_[0]; next_ is the next free page to push.
            // (construct nav_ with the root — see the member init below.)

            // ---- buttons mirroring the C# Clicked handlers ----
            push_button_.set_text("Push Page");
            push_button_.clicked.connect([this] { push_page(); });

            pop_button_.set_text("Pop Page");
            pop_button_.clicked.connect([this] {
                nav_.pop();
                refresh();
            });

            insert_button_.set_text("Insert Page Before Current");
            insert_button_.clicked.connect([this] { insert_page(); });

            remove_button_.set_text("Remove Page Before Current");
            remove_button_.clicked.connect([this] { remove_page_before_current(); });

            pop_to_root_button_.set_text("Pop To Root");
            pop_to_root_button_.clicked.connect([this] {
                nav_.pop_to_root();
                refresh();
            });

            toggle_toolbar_button_.set_text("Toggle Secondary Toolbar Item");
            toggle_toolbar_button_.clicked.connect([this] { toggle_secondary_toolbar_item(); });

            readout_.set_text("");

            stack_.add(readout_);
            stack_.add(push_button_);
            stack_.add(pop_button_);
            stack_.add(insert_button_);
            stack_.add(remove_button_);
            stack_.add(pop_to_root_button_);
            stack_.add(toggle_toolbar_button_);
            page_.set_content(stack_);

            refresh();
            // note: the C# SwapRoot / ToggleNavigationBar / ToggleBackButton branches require a live
            // IStackNavigationView host (RequestNavigation) and the NavigationPage.HasNavigationBar /
            // HasBackButton attached-property store, which need a real navigation host — omitted here
            // (the push/pop/insert/remove/pop-to-root + secondary-toolbar-toggle set is fully exercised).
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED VIEW bottom-up (the readout + buttons, then the stack, then the
        // page), then re-host the tree built in the ctor. The page-owned navigation_page + its pooled
        // content_pages are the STACK UNDER TEST (not mounted in this page's visual tree), so they are
        // deliberately excluded — like chrome_page excludes its non-view menu items (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, readout_, "readout_");
            gallery_attach_one(app, push_button_, "push_button_");
            gallery_attach_one(app, pop_button_, "pop_button_");
            gallery_attach_one(app, insert_button_, "insert_button_");
            gallery_attach_one(app, remove_button_, "remove_button_");
            gallery_attach_one(app, pop_to_root_button_, "pop_to_root_button_");
            gallery_attach_one(app, toggle_toolbar_button_, "toggle_toolbar_button_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(stack_);
            gallery_rehost_content(page_);
        }

        // ---- owned controls, exposed for the hosting main's bottom-up attachment + tests ----
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }
        [[nodiscard]] maui::controls::navigation_page& nav()
        {
            return nav_;
        }
        [[nodiscard]] maui::controls::button& push_button()
        {
            return push_button_;
        }
        [[nodiscard]] maui::controls::button& pop_button()
        {
            return pop_button_;
        }
        [[nodiscard]] maui::controls::button& insert_button()
        {
            return insert_button_;
        }
        [[nodiscard]] maui::controls::button& remove_button()
        {
            return remove_button_;
        }
        [[nodiscard]] maui::controls::button& pop_to_root_button()
        {
            return pop_to_root_button_;
        }
        [[nodiscard]] maui::controls::button& toggle_toolbar_button()
        {
            return toggle_toolbar_button_;
        }

    private:
        // C# PushPage → Navigation.PushAsync(new NavigationGallery()): push the next pooled page (if any).
        void push_page()
        {
            if (next_ < pool_.size())
            {
                nav_.push(pool_[next_]);
                ++next_;
            }
            refresh();
        }

        // C# InsertPage → Navigation.InsertPageBefore(new, NavigationStack.Last()): insert the next pooled
        // page immediately before the current top page.
        void insert_page()
        {
            if (next_ < pool_.size() && nav_.current_page() != nullptr)
            {
                nav_.insert_page_before(pool_[next_], *nav_.current_page());
                ++next_;
            }
            refresh();
        }

        // C# RemovePage → if (NavigationStack.Count >= 2) Navigation.RemovePage(stack[Count - 2]).
        void remove_page_before_current()
        {
            const auto& live = nav_.navigation_stack();
            if (live.size() >= 2)
            {
                nav_.remove_page(*live[live.size() - 2]);
            }
            refresh();
        }

        // C# ToggleSecondaryToolbarItem: if the current page has no secondary toolbar items, add "One" +
        // "Two" (Secondary); else remove every secondary item. Operates on the current top page's chrome.
        void toggle_secondary_toolbar_item()
        {
            maui::controls::content_page* const current = nav_.current_page();
            if (current == nullptr)
            {
                return;
            }
            auto& items = current->toolbar_items();
            bool has_secondary = false;
            for (std::size_t i = 0; i < items.count(); ++i)
            {
                if (items.at(i)->order() == maui::controls::toolbar_item_order::secondary)
                {
                    has_secondary = true;
                    break;
                }
            }
            if (!has_secondary)
            {
                one_item_.set_text("One");
                one_item_.set_order(maui::controls::toolbar_item_order::secondary);
                two_item_.set_text("Two");
                two_item_.set_order(maui::controls::toolbar_item_order::secondary);
                items.add(one_item_);
                items.add(two_item_);
            }
            else
            {
                // Remove every secondary item (back to front so indices stay valid).
                for (std::size_t i = items.count(); i > 0; --i)
                {
                    maui::controls::toolbar_item* const item = items.at(i - 1);
                    if (item->order() == maui::controls::toolbar_item_order::secondary)
                    {
                        items.remove(*item);
                    }
                }
            }
            refresh();
        }

        // Echo the live navigation stack's depth + titles into the readout (the C# "Page Count" label,
        // generalized to show the realized stack). Lists titles bottom→top with the current page marked.
        void refresh()
        {
            const auto& live = nav_.navigation_stack();
            std::string text = "Stack depth: " + std::to_string(live.size());
            if (!live.empty())
            {
                text += "  |  top: ";
                text += std::string(live.back()->title());
            }
            maui::controls::content_page* const current = nav_.current_page();
            const std::size_t secondary = current == nullptr ? 0 : count_secondary(current->toolbar_items());
            text += "  |  secondary toolbar items: " + std::to_string(secondary);
            readout_.set_text(text);
        }

        static std::size_t count_secondary(maui::controls::menu_element_list<maui::controls::toolbar_item>& items)
        {
            std::size_t n = 0;
            for (std::size_t i = 0; i < items.count(); ++i)
            {
                if (items.at(i)->order() == maui::controls::toolbar_item_order::secondary)
                {
                    ++n;
                }
            }
            return n;
        }

        // ---- the gallery's own visual tree ----
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label readout_;
        maui::controls::button push_button_;
        maui::controls::button pop_button_;
        maui::controls::button insert_button_;
        maui::controls::button remove_button_;
        maui::controls::button pop_to_root_button_;
        maui::controls::button toggle_toolbar_button_;

        // ---- the page-owned navigation stack under test + its pooled pages (NON-owning stack, so the
        // pages are members and outlive it) ----
        std::array<maui::controls::content_page, 5> pool_{};
        maui::controls::navigation_page nav_{pool_[0]}; // root = pool_[0]
        std::size_t next_ = 1;                          // next free pooled page to push/insert
        // The two secondary toolbar items the toggle adds/removes (owned; non-owning chrome list).
        maui::controls::toolbar_item one_item_;
        maui::controls::toolbar_item two_item_;
    };
} // namespace maui::samples
