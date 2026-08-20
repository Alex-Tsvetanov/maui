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

            // Match the twin's <WebView HeightRequest="240"/> — without it the WebView collapses to its
            // content height, pulling the status/eval labels + buttons ~240px up out of parity.
            browser_.set_height_request(240);
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
                // <meta color-scheme> opts this WebView out of MAUI's DayNight dark-darkening (parity
                // rule 5 (PORT-MUST-EXPRESS-IT), fix-both-sides): white in both themes, matching the shared web_view.xaml twin.
                // <!DOCTYPE html> selects standards mode — MAUI's welcome.html has it, and without it the
                // WebView renders in quirks mode (different default font/box metrics → a ~1.5% parity diff).
                //
                // The explicit `html{background:#fff}` is load-bearing on WINDOWS ONLY, and the meta tag
                // alone is not enough there. Measured on the board (2026-08-06, reproduced on a repeat
                // capture): in DARK this cell renders black text on a #121212 canvas, while the shared twin
                // — same handler, same markup, but navigated as a URL (Source="welcome.html") — renders the
                // same document on opaque white. Only the source KIND differs: an html source reaches
                // WebView2 through NavigateToString, whose document brings no opaque canvas of its own, so
                // the host's dark base paints through while `color-scheme: light` still resolves the TEXT
                // black. Light scored SSIM 1.0000 throughout, because a white document over a white app
                // background hides exactly this. Declaring the background makes the document opaque
                // regardless of what is beneath it. This also falsifies web_view_handler.cpp:611-613's
                // "zero render risk" note on skipping Profile.PreferredColorScheme — the meta tag does NOT
                // pin the canvas "either way" under NavigateToString.
                "<!DOCTYPE html><html><head><meta name=\"color-scheme\" content=\"light\">"
                "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
                "<style>html{background:#fff}</style></head>"
                "<body><h1>" +
                    heading + "</h1><p>Served from a static HtmlWebViewSource.</p></body></html>",
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
