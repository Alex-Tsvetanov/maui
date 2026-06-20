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
// --- batch 18 (final tail: CV selection/empty edge + iOS specifics) ---
#include "pages/empty_view_load_simulate_page.hpp"
#include "pages/empty_view_null_page.hpp"
#include "pages/empty_view_rtl_page.hpp"
#include "pages/filter_selection_page.hpp"
#include "pages/ios_first_responder_page.hpp"
#include "pages/ios_pan_gesture_page.hpp"
#include "pages/ios_swipe_transition_page.hpp"
#include "pages/preselected_item_page.hpp"
#include "pages/selection_synchronization_page.hpp"
// --- batch 17 (CV tail + pan gesture) ---
#include "pages/cv_visual_states_page.hpp"
#include "pages/footer_only_string_page.hpp"
#include "pages/grouping_plus_selection_page.hpp"
#include "pages/header_footer_grid_horizontal_page.hpp"
#include "pages/items_updating_scroll_mode_page.hpp"
#include "pages/measure_first_strategy_page.hpp"
#include "pages/pan_gesture_events_page.hpp"
#include "pages/scroll_mode_test_page.hpp"
#include "pages/switch_grouping_page.hpp"
// --- batch 16 (PlatformSpecifics/iOS) ---
#include "pages/ios_blur_effect_page.hpp"
#include "pages/ios_date_picker_page.hpp"
#include "pages/ios_entry_page.hpp"
#include "pages/ios_picker_page.hpp"
#include "pages/ios_safe_area_page.hpp"
#include "pages/ios_scroll_view_page.hpp"
#include "pages/ios_search_bar_page.hpp"
#include "pages/ios_slider_update_on_tap_page.hpp"
#include "pages/ios_time_picker_page.hpp"
// --- batch 15 (CollectionView/Carousel tail) ---
#include "pages/carousel_page.hpp"
#include "pages/empty_view_selector_page.hpp"
#include "pages/empty_view_swap_page.hpp"
#include "pages/empty_view_view_page.hpp"
#include "pages/grouping_no_templates_page.hpp"
#include "pages/nested_collection_page.hpp"
#include "pages/scroll_to_group_page.hpp"
#include "pages/staggered_layout_page.hpp"
#include "pages/varied_size_selector_page.hpp"
// --- batch 14 (more CollectionView galleries) ---
#include "pages/empty_view_template_page.hpp"
#include "pages/grid_grouping_page.hpp"
#include "pages/header_footer_grid_page.hpp"
#include "pages/header_footer_template_page.hpp"
#include "pages/header_footer_view_page.hpp"
#include "pages/multiple_bound_selection_page.hpp"
#include "pages/preselected_items_page.hpp"
#include "pages/selection_command_param_page.hpp"
#include "pages/some_empty_groups_page.hpp"
// --- batch 13 (SwipeView galleries) ---
#include "pages/basic_swipe_page.hpp"
#include "pages/custom_size_swipe_page.hpp"
#include "pages/custom_swipe_item_view_page.hpp"
#include "pages/swipe_gesture_page.hpp"
#include "pages/swipe_item_position_page.hpp"
#include "pages/swipe_item_size_page.hpp"
#include "pages/swipe_threshold_page.hpp"
#include "pages/swipe_view_margin_page.hpp"
#include "pages/swipe_view_shadow_page.hpp"
// --- batch 12 (RadioButton + Shadow galleries) ---
#include "pages/invalidate_shadow_host_page.hpp"
#include "pages/radio_button_content_page.hpp"
#include "pages/radio_button_group_binding_page.hpp"
#include "pages/radio_button_group_gallery_page.hpp"
#include "pages/radio_button_group_page.hpp"
#include "pages/radio_content_properties_page.hpp"
#include "pages/radio_template_from_style_page.hpp"
#include "pages/scattered_radio_button_page.hpp"
#include "pages/shadow_playground_page.hpp"
// --- layout alignment (LayoutOptions Start/Center/End/Fill) ---
#include "pages/alignment_page.hpp"
// --- controls showcase (maui-compare ControlsStack reproduction) ---
#include "pages/controls_stack_page.hpp"
// --- batch 11 (BorderGalleries) ---
#include "pages/border_alignment_page.hpp"
#include "pages/border_clip_playground_page.hpp"
#include "pages/border_layout_page.hpp"
#include "pages/border_playground_page.hpp"
#include "pages/border_resize_content_page.hpp"
#include "pages/border_stroke_page.hpp"
#include "pages/border_styles_page.hpp"
#include "pages/borderless_page.hpp"
#include "pages/radio_button_border_page.hpp"
// --- batch 10 (ShapesGalleries — transforms/clip/mutation) ---
#include "pages/auto_size_shapes_page.hpp"
#include "pages/clip_corner_radius_page.hpp"
#include "pages/clip_gallery_page.hpp"
#include "pages/clip_views_page.hpp"
#include "pages/invalidate_brush_page.hpp"
#include "pages/path_transform_string_page.hpp"
#include "pages/shape_app_theme_page.hpp"
#include "pages/transform_playground_page.hpp"
#include "pages/update_path_data_page.hpp"
// --- batch 9 (CollectionView galleries) ---
#include "pages/adaptive_collection_page.hpp"
#include "pages/basic_grouping_page.hpp"
#include "pages/chat_example_page.hpp"
#include "pages/data_template_selector_page.hpp"
#include "pages/empty_view_page.hpp"
#include "pages/filter_collection_page.hpp"
#include "pages/header_footer_page.hpp"
#include "pages/selection_mode_page.hpp"
#include "pages/single_bound_selection_page.hpp"
// --- batch 8 (ShapesGalleries) ---
#include "pages/composition_gallery_page.hpp"
#include "pages/ellipse_gallery_page.hpp"
#include "pages/line_gallery_page.hpp"
#include "pages/line_join_gallery_page.hpp"
#include "pages/path_aspect_gallery_page.hpp"
#include "pages/path_gallery_page.hpp"
#include "pages/polygon_gallery_page.hpp"
#include "pages/polyline_gallery_page.hpp"
#include "pages/rectangle_gallery_page.hpp"
// --- batch 7 (Core tail + HitTesting) ---
#include "pages/application_control_page.hpp"
#include "pages/clip_page.hpp"
#include "pages/context_flyout_page.hpp"
#include "pages/drag_drop_page.hpp"
#include "pages/hit_testing_page.hpp"
#include "pages/menu_bar_page.hpp"
#include "pages/modal_page.hpp"
#include "pages/navigation_gallery_page.hpp"
#include "pages/pointer_gesture_page.hpp"
// --- batch 6 (Core feature demos) ---
#include "pages/alerts_page.hpp"
#include "pages/app_theme_binding_page.hpp"
#include "pages/device_page.hpp"
#include "pages/dispatcher_page.hpp"
#include "pages/effects_page.hpp"
#include "pages/focus_page.hpp"
#include "pages/input_transparent_page.hpp"
#include "pages/semantics_page.hpp"
#include "pages/toolbar_page.hpp"
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
    X("image", image_page)                                                                                             \
    X("entry", entry_page)                                                                                             \
    X("editor", editor_page)                                                                                           \
    X("search_bar", search_bar_page)                                                                                   \
    X("check_box", check_box_page)                                                                                     \
    X("switch", switch_page)                                                                                           \
    X("slider", slider_page)                                                                                           \
    X("stepper", stepper_page)                                                                                         \
    X("progress_bar", progress_bar_page)                                                                               \
    X("activity_indicator", activity_indicator_page)                                                                   \
    X("box_view", box_view_page)                                                                                       \
    X("date_picker", date_picker_page)                                                                                 \
    X("time_picker", time_picker_page)                                                                                 \
    X("picker", picker_page)                                                                                           \
    X("image_button", image_button_page)                                                                               \
    X("refresh_view", refresh_view_page)                                                                               \
    X("absolute_layout", absolute_layout_page)                                                                         \
    X("grid", grid_page)                                                                                               \
    X("flex_layout", flex_layout_page)                                                                                 \
    X("stack_layout", stack_layout_page)                                                                               \
    X("vertical_stack", vertical_stack_layout_page)                                                                    \
    X("horizontal_stack", horizontal_stack_layout_page)                                                                \
    X("scroll_view", scroll_view_page)                                                                                 \
    X("content_view", content_view_page)                                                                               \
    X("z_index", z_index_page)                                                                                         \
    X("indicator", indicator_page)                                                                                     \
    X("shapes_demo", shapes_demo_page)                                                                                 \
    X("title_bar", title_bar_page)                                                                                     \
    X("clipping", clipping_page)                                                                                       \
    X("templated_view", templated_view_page)                                                                           \
    X("layout_is_enabled", layout_is_enabled_page)                                                                     \
    X("custom_layout", custom_layout_page)                                                                             \
    X("hybrid_web_view", hybrid_web_view_page)                                                                         \
    X("relative_layout", relative_layout_page)                                                                         \
    X("brushes", brushes_page)                                                                                         \
    X("transformations", transformations_page)                                                                         \
    X("gestures", gestures_page)                                                                                       \
    X("animation", animation_page)                                                                                     \
    X("styles", styles_page)                                                                                           \
    X("triggers", triggers_page)                                                                                       \
    X("behaviors", behaviors_page)                                                                                     \
    X("visual_states", visual_states_page)                                                                             \
    X("fonts", fonts_page)                                                                                             \
    X("alerts", alerts_page)                                                                                           \
    X("semantics", semantics_page)                                                                                     \
    X("focus", focus_page)                                                                                             \
    X("dispatcher", dispatcher_page)                                                                                   \
    X("device", device_page)                                                                                           \
    X("app_theme_binding", app_theme_binding_page)                                                                     \
    X("toolbar", toolbar_page)                                                                                         \
    X("effects", effects_page)                                                                                         \
    X("input_transparent", input_transparent_page)                                                                     \
    X("clip", clip_page)                                                                                               \
    X("context_flyout", context_flyout_page)                                                                           \
    X("menu_bar", menu_bar_page)                                                                                       \
    X("navigation_gallery", navigation_gallery_page)                                                                   \
    X("modal", modal_page)                                                                                             \
    X("application_control", application_control_page)                                                                 \
    X("pointer_gesture", pointer_gesture_page)                                                                         \
    X("drag_drop", drag_drop_page)                                                                                     \
    X("hit_testing", hit_testing_page)                                                                                 \
    X("ellipse_gallery", ellipse_gallery_page)                                                                         \
    X("rectangle_gallery", rectangle_gallery_page)                                                                     \
    X("line_gallery", line_gallery_page)                                                                               \
    X("polygon_gallery", polygon_gallery_page)                                                                         \
    X("polyline_gallery", polyline_gallery_page)                                                                       \
    X("path_gallery", path_gallery_page)                                                                               \
    X("line_join_gallery", line_join_gallery_page)                                                                     \
    X("path_aspect_gallery", path_aspect_gallery_page)                                                                 \
    X("composition_gallery", composition_gallery_page)                                                                 \
    X("filter_collection", filter_collection_page)                                                                     \
    X("basic_grouping", basic_grouping_page)                                                                           \
    X("selection_mode", selection_mode_page)                                                                           \
    X("header_footer", header_footer_page)                                                                             \
    X("empty_view", empty_view_page)                                                                                   \
    X("data_template_selector", data_template_selector_page)                                                           \
    X("adaptive_collection", adaptive_collection_page)                                                                 \
    X("single_bound_selection", single_bound_selection_page)                                                           \
    X("chat_example", chat_example_page)                                                                               \
    X("transform_playground", transform_playground_page)                                                               \
    X("path_transform_string", path_transform_string_page)                                                             \
    X("shape_app_theme", shape_app_theme_page)                                                                         \
    X("clip_gallery", clip_gallery_page)                                                                               \
    X("clip_views", clip_views_page)                                                                                   \
    X("clip_corner_radius", clip_corner_radius_page)                                                                   \
    X("auto_size_shapes", auto_size_shapes_page)                                                                       \
    X("invalidate_brush", invalidate_brush_page)                                                                       \
    X("update_path_data", update_path_data_page)                                                                       \
    X("border_styles", border_styles_page)                                                                             \
    X("border_stroke", border_stroke_page)                                                                             \
    X("border_playground", border_playground_page)                                                                     \
    X("border_layout", border_layout_page)                                                                             \
    X("alignment", alignment_page)                                                                                     \
    X("controls_stack", controls_stack_page)                                                                           \
    X("border_alignment", border_alignment_page)                                                                       \
    X("border_clip_playground", border_clip_playground_page)                                                           \
    X("borderless", borderless_page)                                                                                   \
    X("border_resize_content", border_resize_content_page)                                                             \
    X("radio_button_border", radio_button_border_page)                                                                 \
    X("radio_button_group", radio_button_group_page)                                                                   \
    X("radio_button_group_binding", radio_button_group_binding_page)                                                   \
    X("scattered_radio_button", scattered_radio_button_page)                                                           \
    X("radio_button_content", radio_button_content_page)                                                               \
    X("radio_content_properties", radio_content_properties_page)                                                       \
    X("radio_template_from_style", radio_template_from_style_page)                                                     \
    X("shadow_playground", shadow_playground_page)                                                                     \
    X("invalidate_shadow_host", invalidate_shadow_host_page)                                                           \
    X("radio_button_group_gallery", radio_button_group_gallery_page)                                                   \
    X("basic_swipe", basic_swipe_page)                                                                                 \
    X("swipe_item_position", swipe_item_position_page)                                                                 \
    X("swipe_view_shadow", swipe_view_shadow_page)                                                                     \
    X("custom_swipe_item_view", custom_swipe_item_view_page)                                                           \
    X("swipe_item_size", swipe_item_size_page)                                                                         \
    X("swipe_view_margin", swipe_view_margin_page)                                                                     \
    X("custom_size_swipe", custom_size_swipe_page)                                                                     \
    X("swipe_gesture", swipe_gesture_page)                                                                             \
    X("swipe_threshold", swipe_threshold_page)                                                                         \
    X("header_footer_grid", header_footer_grid_page)                                                                   \
    X("header_footer_template", header_footer_template_page)                                                           \
    X("header_footer_view", header_footer_view_page)                                                                   \
    X("multiple_bound_selection", multiple_bound_selection_page)                                                       \
    X("selection_command_param", selection_command_param_page)                                                         \
    X("preselected_items", preselected_items_page)                                                                     \
    X("grid_grouping", grid_grouping_page)                                                                             \
    X("some_empty_groups", some_empty_groups_page)                                                                     \
    X("empty_view_template", empty_view_template_page)                                                                 \
    X("staggered_layout", staggered_layout_page)                                                                       \
    X("nested_collection", nested_collection_page)                                                                     \
    X("varied_size_selector", varied_size_selector_page)                                                               \
    X("scroll_to_group", scroll_to_group_page)                                                                         \
    X("carousel_page", carousel_page)                                                                                  \
    X("empty_view_swap", empty_view_swap_page)                                                                         \
    X("empty_view_view", empty_view_view_page)                                                                         \
    X("empty_view_selector", empty_view_selector_page)                                                                 \
    X("grouping_no_templates", grouping_no_templates_page)                                                             \
    X("ios_entry", ios_entry_page)                                                                                     \
    X("ios_date_picker", ios_date_picker_page)                                                                         \
    X("ios_picker", ios_picker_page)                                                                                   \
    X("ios_slider_update_on_tap", ios_slider_update_on_tap_page)                                                       \
    X("ios_scroll_view", ios_scroll_view_page)                                                                         \
    X("ios_search_bar", ios_search_bar_page)                                                                           \
    X("ios_time_picker", ios_time_picker_page)                                                                         \
    X("ios_safe_area", ios_safe_area_page)                                                                             \
    X("ios_blur_effect", ios_blur_effect_page)                                                                         \
    X("cv_visual_states", cv_visual_states_page)                                                                       \
    X("switch_grouping", switch_grouping_page)                                                                         \
    X("grouping_plus_selection", grouping_plus_selection_page)                                                         \
    X("items_updating_scroll_mode", items_updating_scroll_mode_page)                                                   \
    X("measure_first_strategy", measure_first_strategy_page)                                                           \
    X("scroll_mode_test", scroll_mode_test_page)                                                                       \
    X("footer_only_string", footer_only_string_page)                                                                   \
    X("header_footer_grid_horizontal", header_footer_grid_horizontal_page)                                             \
    X("pan_gesture_events", pan_gesture_events_page)                                                                   \
    X("filter_selection", filter_selection_page)                                                                       \
    X("selection_synchronization", selection_synchronization_page)                                                     \
    X("preselected_item", preselected_item_page)                                                                       \
    X("empty_view_null", empty_view_null_page)                                                                         \
    X("empty_view_load_simulate", empty_view_load_simulate_page)                                                       \
    X("empty_view_rtl", empty_view_rtl_page)                                                                           \
    X("ios_first_responder", ios_first_responder_page)                                                                 \
    X("ios_pan_gesture", ios_pan_gesture_page)                                                                         \
    X("ios_swipe_transition", ios_swipe_transition_page)

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
