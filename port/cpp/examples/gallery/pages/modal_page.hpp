#pragma once
// maui::samples::modal_page — ports ModalPage.xaml (+ .xaml.cs).
//
// The C# ModalPage is a BasePage with a "Modal Page n" label and five buttons that drive the AMBIENT
// modal/navigation stack (Navigation.PushAsync / PushModalAsync — with a plain page, a NavigationPage
// root, and a FlyoutPage — / PopModalAsync), with the "Pop Modal Page" button shown only while the modal
// stack is non-empty (OnAppearing: PopModal.IsVisible = Navigation.ModalStack.Count > 0). The gallery
// host mounts a single content_page (page()), so there is no live modal host to overlay into. We exercise
// the SAME modal-navigation surface on a PAGE-OWNED navigation_page (its SEPARATE modal_stack) and echo
// the resulting modal DEPTH into a readout — the code-first, headless-safe analog of the live PushModal/
// PopModal the C# sample performs on its ambient stack.
//
// Mapped from ModalPage.xaml.cs:
//   - "Push Page"                 → nav_.push(new page)         (Navigation.PushAsync — the page stack)
//   - "Push Modal Page"           → nav_.push_modal(new page)   (Navigation.PushModalAsync, plain page)
//   - "Push Modal Navigation Page"→ nav_.push_modal(new page)   (C# pushes a NavigationPage(modal) root;
//        the port's navigation_page modal stack holds content_pages, so we push the inner modal page and
//        note the NavigationPage-root wrapper as a best-effort simplification — same modal-depth effect.)
//   - "Push Modal Flyout Page"    → nav_.push_modal(new page)   (C# pushes a FlyoutPage modal; same note —
//        a content_page stand-in carries the modal, the FlyoutPage wrapper is omitted.)
//   - "Pop Modal Page"           → nav_.pop_modal()             (Navigation.PopModalAsync); the button is
//        ENABLED only while the modal stack is non-empty (the C# PopModal.IsVisible = ModalStack.Count>0).
//
// The page OWNS its whole element tree (the sample_app pattern): the readout, the five buttons, the grid-
// like vertical stack, the gallery content_page (page()), AND the page-owned navigation_page + the pool
// of content_pages it pushes/modals (the stacks are NON-owning, so the pages are members). It is backend-
// agnostic; the generic mount attaches every view's handler and hosts the ctor-built tree. The page-owned
// navigation_page + its pooled pages are the STACK UNDER TEST (not in this page's visual tree), so they
// are excluded from the attach/re-host walk. The C# Grid (RowDefinitions=Auto×6) is a plain vertical
// stack here — a single-column auto-row Grid lays out identically to a vertical stack (note below).

#include <array>
#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/navigation_page.hpp"
#include "maui/controls/vertical_stack_layout.hpp"

namespace maui::samples
{
    class modal_page
    {
    public:
        modal_page()
        {
            page_.set_title("Modal Page 0"); // C# Title = $"Modal Page {s_instanceCount++}"
            stack_.set_spacing(8);

            for (std::size_t i = 0; i < page_pool_.size(); ++i)
            {
                page_pool_[i].set_title("Page " + std::to_string(i + 1));
            }
            for (std::size_t i = 0; i < modal_pool_.size(); ++i)
            {
                modal_pool_[i].set_title("Modal Page " + std::to_string(i + 1));
            }
            // nav_ is constructed with page_pool_[0] as its (page-stack) root; the modal stack starts
            // empty. The page stack and the modal stack draw from SEPARATE pools — navigation_page's
            // "already on a stack" guard spans both, so a shared pool would make the second push no-op.

            number_label_.set_text("Modal Page 1"); // C# lblModalPageNumber.Text = $"Modal Page {count}"

            push_button_.set_text("Push Page");
            push_button_.clicked.connect([this] { push_page(); });

            push_modal_button_.set_text("Push Modal Page");
            push_modal_button_.clicked.connect([this] { push_modal(); });

            push_modal_nav_button_.set_text("Push Modal Navigation Page");
            push_modal_nav_button_.clicked.connect([this] { push_modal(); });
            // note: C# wraps the modal in a NavigationPage(modal) root; the port's modal stack holds
            // content_pages, so the inner modal page is pushed directly (same modal-depth effect).

            push_modal_flyout_button_.set_text("Push Modal Flyout Page");
            push_modal_flyout_button_.clicked.connect([this] { push_modal(); });
            // note: C# wraps the modal in a FlyoutPage; the FlyoutPage wrapper is omitted (a content_page
            // stand-in carries the modal) — best-effort, same modal-depth effect.

            pop_modal_button_.set_text("Pop Modal Page");
            pop_modal_button_.clicked.connect([this] {
                nav_.pop_modal();
                refresh();
            });

            readout_.set_text("");

            stack_.add(number_label_);
            stack_.add(push_button_);
            stack_.add(push_modal_button_);
            stack_.add(push_modal_nav_button_);
            stack_.add(push_modal_flyout_button_);
            stack_.add(pop_modal_button_);
            stack_.add(readout_);
            page_.set_content(stack_);

            refresh();
            // note: C# sets BackgroundColor = Purple/Pink per push — there is no set_background_color on
            // the headless view surface, so the cosmetic color alternation is omitted (not load-bearing).
            // The OnAppearing / OnNavigatingFrom / OnNavigatedTo window-title bookkeeping needs a live
            // Window host and is likewise omitted; PopModal visibility is modeled as button enablement.
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // ---- owned controls, exposed for the hosting main's bottom-up attachment + tests ----
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::label& number_label()
        {
            return number_label_;
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
        [[nodiscard]] maui::controls::button& push_modal_button()
        {
            return push_modal_button_;
        }
        [[nodiscard]] maui::controls::button& push_modal_nav_button()
        {
            return push_modal_nav_button_;
        }
        [[nodiscard]] maui::controls::button& push_modal_flyout_button()
        {
            return push_modal_flyout_button_;
        }
        [[nodiscard]] maui::controls::button& pop_modal_button()
        {
            return pop_modal_button_;
        }

    private:
        // C# PushClicked → Navigation.PushAsync(new ModalPage()): push the next pooled page onto the page
        // stack (distinct from the modal stack).
        void push_page()
        {
            if (next_page_ < page_pool_.size())
            {
                nav_.push(page_pool_[next_page_]);
                ++next_page_;
            }
            refresh();
        }

        // C# Push*ModalClicked → Navigation.PushModalAsync(new …): push the next pooled page onto the
        // SEPARATE modal stack.
        void push_modal()
        {
            if (next_modal_ < modal_pool_.size())
            {
                nav_.push_modal(modal_pool_[next_modal_]);
                ++next_modal_;
            }
            refresh();
        }

        // Echo the modal-stack depth into the readout, and enable "Pop Modal Page" only while the modal
        // stack is non-empty (the C# OnAppearing PopModal.IsVisible = ModalStack.Count > 0). Also shows
        // the page-stack depth for context.
        void refresh()
        {
            const std::size_t modal_depth = nav_.modal_stack().size();
            const std::size_t page_depth = nav_.navigation_stack().size();
            std::string text = "Modal depth: " + std::to_string(modal_depth);
            text += "  |  page stack depth: " + std::to_string(page_depth);
            if (modal_depth > 0)
            {
                text += "  |  top modal: ";
                text += std::string(nav_.modal_stack().back()->title());
            }
            readout_.set_text(text);
            pop_modal_button_.set_is_enabled(modal_depth > 0);
        }

        // ---- the gallery's own visual tree (the C# Auto-row Grid as a vertical stack — identical layout
        // for a single column of auto-height rows) ----
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label number_label_;
        maui::controls::button push_button_;
        maui::controls::button push_modal_button_;
        maui::controls::button push_modal_nav_button_;
        maui::controls::button push_modal_flyout_button_;
        maui::controls::button pop_modal_button_;
        maui::controls::label readout_;

        // ---- the page-owned navigation_page under test + its pooled pages (NON-owning stacks, so the
        // pages are members and outlive both the page stack and the modal stack). SEPARATE pools: the
        // "already on a stack" guard spans both stacks, so sharing would no-op the second push. ----
        std::array<maui::controls::content_page, 4> page_pool_{};  // page-stack root + push targets
        std::array<maui::controls::content_page, 4> modal_pool_{}; // modal-stack push targets
        maui::controls::navigation_page nav_{page_pool_[0]};       // page-stack root = page_pool_[0]
        std::size_t next_page_ = 1;                                // next free page for the page stack
        std::size_t next_modal_ = 0;                               // next free page for the modal stack
    };
} // namespace maui::samples
