#pragma once
// web_view_page — a self-contained demo page for the W1-08 web_view vertical: a web_view loading a
// STATIC html_web_view_source (no network), back/forward/reload buttons over the handler-pushed
// CanGoBack/CanGoForward read-onlys, an "Eval 1+1" button driving the EvaluateJavaScriptAsync round
// trip into a label, and a status label fed by the navigating/navigated events (the C# gallery-page
// convention, code-first).
//
// The page OWNS its whole element tree (the value_controls_page pattern). It is backend-agnostic — a
// sample main attaches handlers bottom-up via the hosting layer and hosts page() in a window; the
// headless/apple/ios test trees exercise the same controls directly.
//
// Interactions demonstrated:
//   - "Page A"/"Page B" buttons swap the html source (a second load makes CanGoBack flip true),
//   - back/forward/reload call web_view::go_back/go_forward/reload,
//   - navigated updates the status label with the event kind + url,
//   - "Eval 1+1" runs eval_js and writes the result (or "<null>") into the result label.

#include <cstdio>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/html_web_view_source.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/controls/web_navigation_event_args.hpp"
#include "maui/controls/web_view.hpp"
#include "maui/core/web_navigation_event.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class web_view_page
    {
    public:
        web_view_page()
        {
            page_.set_title("Web view");
            stack_.set_spacing(8);

            status_.set_text("No navigation yet");
            result_.set_text("Eval result: <none>");

            browser_.navigated.connect([this](const maui::controls::web_navigated_event_args& args) {
                status_.set_text(std::string(kind_name(args.navigation_event)) + " -> " + args.url);
            });

            load_a_.set_text("Page A");
            load_a_.clicked.connect([this] { load_page("Page A", "https://demo.test/a"); });
            load_b_.set_text("Page B");
            load_b_.clicked.connect([this] { load_page("Page B", "https://demo.test/b"); });

            back_.set_text("Back");
            back_.clicked.connect([this] { browser_.go_back(); });
            forward_.set_text("Forward");
            forward_.clicked.connect([this] { browser_.go_forward(); });
            reload_.set_text("Reload");
            reload_.clicked.connect([this] { browser_.reload(); });

            eval_.set_text("Eval 1+1");
            eval_.clicked.connect([this] {
                browser_.eval_js("1+1", [this](const std::optional<std::string>& value) {
                    result_.set_text("Eval result: " + (value ? *value : std::string("<null>")));
                });
            });

            stack_.add(browser_);
            stack_.add(status_);
            stack_.add(result_);
            stack_.add(load_a_);
            stack_.add(load_b_);
            stack_.add(back_);
            stack_.add(forward_);
            stack_.add(reload_);
            stack_.add(eval_);
            page_.set_content(stack_);

            load_page("Welcome", "https://demo.test/welcome");
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (the stack's children, then the stack, then the
        // page), then re-host the tree built in the ctor (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, status_, "status_");
            gallery_attach_one(app, result_, "result_");
            gallery_attach_one(app, load_a_, "load_a_");
            gallery_attach_one(app, load_b_, "load_b_");
            gallery_attach_one(app, back_, "back_");
            gallery_attach_one(app, forward_, "forward_");
            gallery_attach_one(app, reload_, "reload_");
            gallery_attach_one(app, eval_, "eval_");
            gallery_attach_one(app, browser_, "browser_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(stack_);
            gallery_rehost_content(page_);
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::web_view& browser()
        {
            return browser_;
        }
        [[nodiscard]] maui::controls::label& status()
        {
            return status_;
        }
        [[nodiscard]] maui::controls::label& result()
        {
            return result_;
        }
        [[nodiscard]] maui::controls::button& load_a_button()
        {
            return load_a_;
        }
        [[nodiscard]] maui::controls::button& load_b_button()
        {
            return load_b_;
        }
        [[nodiscard]] maui::controls::button& back_button()
        {
            return back_;
        }
        [[nodiscard]] maui::controls::button& forward_button()
        {
            return forward_;
        }
        [[nodiscard]] maui::controls::button& reload_button()
        {
            return reload_;
        }
        [[nodiscard]] maui::controls::button& eval_button()
        {
            return eval_;
        }

    private:
        static std::string_view kind_name(maui::core::web_navigation_event kind)
        {
            switch (kind)
            {
                case maui::core::web_navigation_event::back:
                    return "back";
                case maui::core::web_navigation_event::forward:
                    return "forward";
                case maui::core::web_navigation_event::refresh:
                    return "refresh";
                case maui::core::web_navigation_event::new_page:
                    break;
            }
            return "new_page";
        }

        void load_page(const std::string& heading, std::string base_url)
        {
            browser_.set_source(std::make_shared<maui::controls::html_web_view_source>(
                "<html><body><h1>" + heading + "</h1><p>Served from a static HtmlWebViewSource.</p></body></html>",
                std::move(base_url)));
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::web_view browser_;
        maui::controls::label status_;
        maui::controls::label result_;
        maui::controls::button load_a_;
        maui::controls::button load_b_;
        maui::controls::button back_;
        maui::controls::button forward_;
        maui::controls::button reload_;
        maui::controls::button eval_;
    };
} // namespace maui::samples
