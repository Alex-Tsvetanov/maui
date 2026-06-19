#pragma once
// maui::samples::content_view_page — ports ContentViewPage.xaml (+ ContentViewPage.xaml.cs), code-first.
//
// The MAUI page shows the content_view in its two roles over a padded, scrolled stack:
//   - a ContentView whose .Content is a Label bound to the page's BindingContext.Text ("Content"),
//   - a ContentView whose .ControlTemplate wraps a Label (the templated presentation path).
// (The CardView rows in the XAML are sample chrome, not the control under test, so they are omitted.)
//
// This port keeps the control under test — content_view — and demonstrates its single-content surface
// plus the runtime content SWAP that the C# Text-binding stands in for: the page replaces the hosted
// label and the wrapper re-presents it (TemplateUtilities.OnContentChanged → the "set_content"
// command). content_view co-owns its content via shared_ptr<element> (unlike the non-owning page-level
// hosts), so the swapped-in labels are owned here as shared_ptrs.
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic: a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same wiring directly.
//
// Interactions demonstrated:
//   - the wrapper (content_view) hosts live content inside its padding,
//   - swapping the wrapper's content at runtime re-presents the new label (the binding-update stand-in),
//   - padding on the wrapper (ContentView padding) frames the presented content.

#include <memory>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/content_view.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/thickness.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class content_view_page
    {
    public:
        content_view_page()
        {
            page_.set_title("ContentView");
            stack_.set_spacing(12);

            // Section headers (the XAML "Default" / "BindingContext" Headline labels, distilled).
            default_header_.set_text("ContentView");
            swap_header_.set_text("Content");

            // The wrapper (content_view) — the simple single-content host. Padding frames the content
            // (ContentView padding, inherited from the templated/layout base). content_view co-owns
            // its content via shared_ptr<element>, so the presented label is a shared_ptr here.
            wrapped_text_ = std::make_shared<maui::controls::label>();
            wrapped_text_->set_text("Content");
            wrapper_.set_padding(maui::core::thickness(12));
            wrapper_.set_content(wrapped_text_);

            // The "Swap content" button — tapping it re-presents the wrapper's content (the C# XAML's
            // Button bound to a swap command). Mirrors ContentViewPage's interactive control.
            swap_button_.set_text("Swap content");
            swap_button_.clicked.connect([this] { show_alternate_content(); });

            stack_.add(default_header_);
            stack_.add(wrapper_);
            stack_.add(swap_header_);
            stack_.add(swap_button_);

            scroller_.set_content(stack_);
            page_.set_content(scroller_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Swap the wrapper's hosted content at runtime — the ContentView re-presents the new label
        // (TemplateUtilities.OnContentChanged → "set_content"). Stands in for the C# Text-binding update.
        void show_alternate_content()
        {
            alternate_text_ = std::make_shared<maui::controls::label>();
            alternate_text_->set_text("Swapped content");
            wrapper_.set_content(alternate_text_);
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (the inner content before the wrapper, then the
        // stack, the scroll_view, and the page last), then re-host the tree built in the ctor.
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, default_header_, "default_header_");
            gallery_attach_one(app, *wrapped_text_, "wrapped_text_");
            if (alternate_text_)
            {
                gallery_attach_one(app, *alternate_text_, "alternate_text_");
            }
            gallery_attach_one(app, wrapper_, "wrapper_");
            gallery_attach_one(app, swap_header_, "swap_header_");
            gallery_attach_one(app, swap_button_, "swap_button_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, scroller_, "scroller_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_content(wrapper_);  // content_view hosts its presented label
            gallery_rehost_layout(stack_);     // stack hosts the headers + the wrapper
            gallery_rehost_content(scroller_); // scroll_view hosts the stack
            gallery_rehost_content(page_);     // page hosts the scroll_view
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::content_view& wrapper()
        {
            return wrapper_;
        }
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] const std::shared_ptr<maui::controls::label>& wrapped_text()
        {
            return wrapped_text_;
        }

    private:
        maui::controls::content_page page_;
        maui::controls::scroll_view scroller_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label default_header_;
        maui::controls::label swap_header_;
        maui::controls::button swap_button_;
        maui::controls::content_view wrapper_;
        std::shared_ptr<maui::controls::label> wrapped_text_;   // content_view co-owns its content
        std::shared_ptr<maui::controls::label> alternate_text_; // the swapped-in content (created on demand)
    };
} // namespace maui::samples
