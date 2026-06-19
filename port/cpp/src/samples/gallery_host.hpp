#pragma once
// gallery_host — the runnable demo-gallery host shared by the macOS + iOS gallery mains.
//
// The 11 curated demo pages (src/samples/pages/*.hpp) each OWN a self-contained element tree and expose
// page() returning their root page (a content_page, except tabbed_flyout's flyout_page). They are
// orphaned on their own — nothing builds or runs them. This header bridges that gap two ways:
//
//   - gallery_app<Page> is a maui::controls::application subclass (the use_maui_app<TApp> shape, like the
//     sample_app in maui_app_sample.mm): it OWNS a Page member + a window, hosts the page in the window
//     (window.set_content(page.page())), and overrides create_window() to return that window. The mains
//     boot it through the maui_app_builder, attach the page's handlers (page.attach_handlers), attach the
//     window handler, open_window, then measure/arrange + show the native window.
//
//   - MAUI_GALLERY_PAGES(X) single-sources the page list (name string ⇄ page type). The mains expand it
//     against the MAUI_SAMPLE_PAGE env var to pick which gallery_app<PageType> to boot, so a new page is
//     added in exactly one place. window.set_content and i_view::measure/arrange are reached through the
//     page's element / i_view faces, so the same template deduces for both content_page and flyout_page
//     roots.

#include "maui/controls/application.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/i_window.hpp"

#include "pages/chrome_page.hpp"
#include "pages/containers_page.hpp"
#include "pages/formatted_text_page.hpp"
#include "pages/input_controls_page.hpp"
#include "pages/items_page.hpp"
#include "pages/pickers_page.hpp"
#include "pages/shapes_page.hpp"
#include "pages/swipe_refresh_page.hpp"
#include "pages/tabbed_flyout_page.hpp"
#include "pages/value_controls_page.hpp"
#include "pages/web_view_page.hpp"
// --- batch 5 (UserInterface + Core feature demos) ---
#include "pages/animation_page.hpp"
#include "pages/behaviors_page.hpp"
#include "pages/brushes_page.hpp"
#include "pages/fonts_page.hpp"
#include "pages/gestures_page.hpp"
#include "pages/styles_page.hpp"
#include "pages/transformations_page.hpp"
#include "pages/triggers_page.hpp"
#include "pages/visual_states_page.hpp"
// --- batch 4 (Tier-1/Tier-2 tail) ---
#include "pages/clipping_page.hpp"
#include "pages/custom_layout_page.hpp"
#include "pages/hybrid_web_view_page.hpp"
#include "pages/indicator_page.hpp"
#include "pages/layout_is_enabled_page.hpp"
#include "pages/relative_layout_page.hpp"
#include "pages/shapes_demo_page.hpp"
#include "pages/templated_view_page.hpp"
#include "pages/title_bar_page.hpp"
// --- batch 3 (Pages/Layouts) ---
#include "pages/absolute_layout_page.hpp"
#include "pages/content_view_page.hpp"
#include "pages/flex_layout_page.hpp"
#include "pages/grid_page.hpp"
#include "pages/horizontal_stack_layout_page.hpp"
#include "pages/scroll_view_page.hpp"
#include "pages/stack_layout_page.hpp"
#include "pages/vertical_stack_layout_page.hpp"
#include "pages/z_index_page.hpp"
// --- Tier-1 per-control sample pages (MAUI Controls.Sample/Pages/Controls) ---
#include "pages/button_page.hpp"
#include "pages/check_box_page.hpp"
#include "pages/editor_page.hpp"
#include "pages/entry_page.hpp"
#include "pages/image_page.hpp"
#include "pages/label_page.hpp"
#include "pages/search_bar_page.hpp"
#include "pages/slider_page.hpp"
#include "pages/switch_page.hpp"
// --- batch 2 (more Pages/Controls) ---
#include "pages/activity_indicator_page.hpp"
#include "pages/box_view_page.hpp"
#include "pages/date_picker_page.hpp"
#include "pages/image_button_page.hpp"
#include "pages/picker_page.hpp"
#include "pages/progress_bar_page.hpp"
#include "pages/refresh_view_page.hpp"
#include "pages/stepper_page.hpp"
#include "pages/time_picker_page.hpp"

// The curated demo set, single-sourced. X(name_literal, page_type). The mains map the MAUI_SAMPLE_PAGE
// env var onto these, and gallery READMEs/captures key off the same names.
#define MAUI_GALLERY_PAGES(X)                                                                                          \
    X("value_controls", value_controls_page)                                                                           \
    X("input_controls", input_controls_page)                                                                           \
    X("pickers", pickers_page)                                                                                         \
    X("formatted_text", formatted_text_page)                                                                           \
    X("items", items_page)                                                                                             \
    X("shapes", shapes_page)                                                                                           \
    X("containers", containers_page)                                                                                   \
    X("swipe_refresh", swipe_refresh_page)                                                                             \
    X("web_view", web_view_page)                                                                                       \
    X("chrome", chrome_page)                                                                                           \
    X("tabbed_flyout", tabbed_flyout_page)                                                                             \
    X("button", button_page)                                                                                           \
    X("label", label_page)                                                                                             \
    X("image", image_page) X("entry", entry_page) X("editor", editor_page) X("search_bar", search_bar_page)            \
        X("check_box", check_box_page) X("switch", switch_page) X("slider", slider_page) X("stepper", stepper_page) X( \
            "progress_bar", progress_bar_page) X("activity_indicator", activity_indicator_page)                        \
            X("box_view", box_view_page) X("date_picker", date_picker_page) X("time_picker", time_picker_page) X(      \
                "picker", picker_page) X("image_button", image_button_page) X("refresh_view", refresh_view_page)       \
                X("absolute_layout", absolute_layout_page) X("grid", grid_page) X("flex_layout", flex_layout_page) X(  \
                    "stack_layout", stack_layout_page) X("vertical_stack", vertical_stack_layout_page)                 \
                    X("horizontal_stack", horizontal_stack_layout_page) X("scroll_view", scroll_view_page) X(          \
                        "content_view", content_view_page) X("z_index", z_index_page) X("indicator", indicator_page)   \
                        X("shapes_demo", shapes_demo_page) X("title_bar", title_bar_page) X("clipping", clipping_page) \
                            X("templated_view", templated_view_page) X("layout_is_enabled", layout_is_enabled_page)    \
                                X("custom_layout", custom_layout_page) X("hybrid_web_view", hybrid_web_view_page)      \
                                    X("relative_layout", relative_layout_page) X("brushes", brushes_page)              \
                                        X("transformations", transformations_page) X("gestures", gestures_page)        \
                                            X("animation", animation_page) X("styles", styles_page)                    \
                                                X("triggers", triggers_page) X("behaviors", behaviors_page)            \
                                                    X("visual_states", visual_states_page) X("fonts", fonts_page)

namespace maui::samples
{
    // The application subclass the builder mints (use_maui_app<gallery_app<Page>>). It OWNS the demo page
    // and the window, and hosts the page in the window (the C# Application.CreateWindow shape).
    template <class Page> class gallery_app final : public maui::controls::application
    {
    public:
        gallery_app()
        {
            window_.set_title("MAUI C++ — gallery");
            window_.set_content(page_.page()); // the page root is an element (content_page / flyout_page)
        }

        [[nodiscard]] maui::core::i_window* create_window() override
        {
            return &window_;
        }

        // The owned demo page (for the main's page_.attach_handlers(maui_app)).
        [[nodiscard]] Page& page_member()
        {
            return page_;
        }
        // The owned window (for the main's window-handler attach + native show).
        [[nodiscard]] maui::controls::window& win()
        {
            return window_;
        }

    private:
        // The page is declared BEFORE the window: the window holds a non-owning back-pointer to the page
        // (set_content), so the page must outlive the window (destroyed in reverse declaration order).
        Page page_;
        maui::controls::window window_;
    };
} // namespace maui::samples
