#pragma once
// maui::samples::progress_bar_page — ports ProgressBarPage.xaml (+ ProgressBarPage.xaml.cs).
//
// A self-contained, code-first demo page for maui::controls::progress_bar. Mirrors the EXACT shape of
// value_controls_page.hpp / button_page.hpp: the class OWNS its whole element tree as members,
// exposes page() and attach_handlers(maui_app) (bottom-up, leaves first -> layout -> page, via the
// shared gallery_attach helpers), and uses only cross-platform maui:: API so it stays headless-safe.
//
// What the MAUI page demonstrates (reproduced here so the demo visibly exercises the control):
//   - a Default progress bar at Progress=0.5,
//   - a ProgressColor=Orange bar at Progress=0.5 (twice in the XAML; the second pair repeats the same
//     two — kept here as one Default + one ProgressColor pair plus a Disabled bar),
//   - a Disabled bar (IsEnabled=false) at Progress=0.5,
//   - a ProgressTo bar driven by a button (OnProgressToClicked): in MAUI it animates to 1.0 over 1000ms;
//     here it sweeps to 1.0 directly (see note).
//
// note: progress_bar::ProgressTo(value, length, easing) is deferred in the port (documented in
// port/STATUS.md) — it needs the Animation/Easing subsystem the port does not have yet. The ProgressTo
// button therefore sets Progress=1.0 immediately (the end state of the MAUI animation) rather than
// animating the sweep. The Headline label styling has no resource-dictionary surface in the port, so
// the section headers are plain labels.

#include <memory>
#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/progress_bar.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class progress_bar_page
    {
    public:
        progress_bar_page()
        {
            page_.set_title("ProgressBar");
            stack_.set_spacing(6);
            stack_.set_padding(maui::core::thickness(12));

            // ---- Default (Progress=0.5) ----
            default_header_.set_text("Default");
            default_bar_.set_progress(0.5);

            // ---- ProgressColor=Orange (Progress=0.5) ----
            color_header_.set_text("ProgressColor");
            color_bar_.set_progress(0.5);
            color_bar_.set_progress_color(maui::graphics::colors::orange);

            // ---- Disabled (IsEnabled=false, Progress=0.5) ----
            disabled_header_.set_text("Disabled");
            disabled_bar_.set_is_enabled(false);
            disabled_bar_.set_progress(0.5);

            // ---- Second ProgressColor=Orange pair (the XAML repeats it) ----
            color2_header_.set_text("ProgressColor");
            color2_bar_.set_progress(0.5);
            color2_bar_.set_progress_color(maui::graphics::colors::orange);

            // ---- ProgressTo bar + button (OnProgressToClicked) ----
            progress_to_header_.set_text("ProgressTo");
            // note: ProgressTo(1.0, 1000, Easing.Linear) is deferred; the button jumps to the end state.
            progress_to_button_.set_text("ProgressTo");
            progress_to_button_.clicked.connect([this] { progress_to_bar_.set_progress(1.0); });

            // Section headers render bold @18pt — mirrors maui-compare ProgressBarPage.Headline().
            for (maui::controls::label* h : {&default_header_, &color_header_, &disabled_header_, &progress_to_header_})
            {
                h->set_font(maui::core::font::system_font_of_size(18.0, maui::core::font_weight::bold));
            }

            stack_.add(default_header_);
            stack_.add(default_bar_);
            stack_.add(color_header_);
            stack_.add(color_bar_);
            stack_.add(disabled_header_);
            stack_.add(disabled_bar_);
            stack_.add(color2_header_);
            stack_.add(color2_bar_);
            stack_.add(progress_to_header_);
            stack_.add(progress_to_bar_);
            stack_.add(progress_to_button_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (leaves first, the page last), then re-host the
        // tree built in the ctor (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, default_header_, "default_header_");
            gallery_attach_one(app, default_bar_, "default_bar_");
            gallery_attach_one(app, color_header_, "color_header_");
            gallery_attach_one(app, color_bar_, "color_bar_");
            gallery_attach_one(app, disabled_header_, "disabled_header_");
            gallery_attach_one(app, disabled_bar_, "disabled_bar_");
            gallery_attach_one(app, color2_header_, "color2_header_");
            gallery_attach_one(app, color2_bar_, "color2_bar_");
            gallery_attach_one(app, progress_to_header_, "progress_to_header_");
            gallery_attach_one(app, progress_to_bar_, "progress_to_bar_");
            gallery_attach_one(app, progress_to_button_, "progress_to_button_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(stack_);
            gallery_rehost_content(page_);
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment / tests.
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::progress_bar& default_bar()
        {
            return default_bar_;
        }
        [[nodiscard]] maui::controls::progress_bar& color_bar()
        {
            return color_bar_;
        }
        [[nodiscard]] maui::controls::progress_bar& progress_to_bar()
        {
            return progress_to_bar_;
        }
        [[nodiscard]] maui::controls::button& progress_to_button()
        {
            return progress_to_button_;
        }

    private:
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label default_header_;
        maui::controls::progress_bar default_bar_;
        maui::controls::label color_header_;
        maui::controls::progress_bar color_bar_;
        maui::controls::label disabled_header_;
        maui::controls::progress_bar disabled_bar_;
        maui::controls::label color2_header_;
        maui::controls::progress_bar color2_bar_;
        maui::controls::label progress_to_header_;
        maui::controls::progress_bar progress_to_bar_;
        maui::controls::button progress_to_button_;
    };
} // namespace maui::samples
