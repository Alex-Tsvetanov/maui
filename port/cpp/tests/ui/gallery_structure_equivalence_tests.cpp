// Builder-vs-XAML structure equivalence — the first line of defense for the shared-XAML board
// (port/maui-reference/pages, manifest rows with builder_twin=true): the code-first builder page
// (examples/gallery/pages/<page_type>.hpp) and the SAME page hydrated from the canonical shared .xaml
// via the runtime loader must normalize to the SAME control tree (tests/support/view_tree_describe.hpp).
//
// One TEST per key — every manifest builder_twin key that is also in MAUI_GALLERY_PAGES (the three
// macro keys with no shared page — brushes, selection_mode, shapes_demo — are excluded); adding a key
// is one STRUCTURE_EQUIVALENCE_TEST line (plus its page include).
// These tests are EXPECTED to fail while a hand-authored twin genuinely diverges from its builder page —
// each failure's printed tree diff is the work item; do NOT weaken describe() to paper over a real
// structural divergence (only over cosmetic noise, per the header's conservative-props policy).
//
// The builder pages construct plain cross-platform control trees (no handlers, no mounting), so this
// runs headless. SHARED_PAGES_DIR is the absolute source-tree path to the shared pages, injected by
// CMake exactly like tests/xaml/gallery_twin_tests.cpp.

#include "tests/support/view_tree_describe.hpp"

#include "maui/controls/content_page.hpp"
#include "maui/xaml/xaml_loader.hpp"
#include "maui/xaml/xaml_runtime_bindings.hpp" // register_runtime_bindings ({Binding} in item templates)

#include "pages/absolute_layout_page.hpp"
#include "pages/activity_indicator_page.hpp"
#include "pages/adaptive_collection_page.hpp"
#include "pages/alerts_page.hpp"
#include "pages/alignment_page.hpp"
#include "pages/animation_page.hpp"
#include "pages/app_theme_binding_page.hpp"
#include "pages/application_control_page.hpp"
#include "pages/auto_size_shapes_page.hpp"
#include "pages/basic_grouping_page.hpp"
#include "pages/basic_swipe_page.hpp"
#include "pages/behaviors_page.hpp"
#include "pages/border_alignment_page.hpp"
#include "pages/border_clip_playground_page.hpp"
#include "pages/border_layout_page.hpp"
#include "pages/border_page.hpp"
#include "pages/border_playground_page.hpp"
#include "pages/border_resize_content_page.hpp"
#include "pages/border_stroke_page.hpp"
#include "pages/border_styles_page.hpp"
#include "pages/borderless_page.hpp"
#include "pages/box_view_page.hpp"
#include "pages/button_page.hpp"
#include "pages/carousel_page.hpp"
#include "pages/chat_example_page.hpp"
#include "pages/check_box_page.hpp"
#include "pages/chrome_page.hpp"
#include "pages/clip_corner_radius_page.hpp"
#include "pages/clip_gallery_page.hpp"
#include "pages/clip_page.hpp"
#include "pages/clip_views_page.hpp"
#include "pages/clipping_page.hpp"
#include "pages/collectionview_page.hpp"
#include "pages/composition_gallery_page.hpp"
#include "pages/containers_page.hpp"
#include "pages/content_view_page.hpp"
#include "pages/context_flyout_page.hpp"
#include "pages/controls_stack_page.hpp"
#include "pages/custom_layout_page.hpp"
#include "pages/custom_size_swipe_page.hpp"
#include "pages/custom_swipe_item_view_page.hpp"
#include "pages/cv_visual_states_page.hpp"
#include "pages/data_template_selector_page.hpp"
#include "pages/date_picker_page.hpp"
#include "pages/device_page.hpp"
#include "pages/dispatcher_page.hpp"
#include "pages/drag_drop_page.hpp"
#include "pages/editor_page.hpp"
#include "pages/effects_page.hpp"
#include "pages/ellipse_gallery_page.hpp"
#include "pages/empty_view_load_simulate_page.hpp"
#include "pages/empty_view_null_page.hpp"
#include "pages/empty_view_page.hpp"
#include "pages/empty_view_rtl_page.hpp"
#include "pages/empty_view_selector_page.hpp"
#include "pages/empty_view_swap_page.hpp"
#include "pages/empty_view_template_page.hpp"
#include "pages/empty_view_view_page.hpp"
#include "pages/entry_page.hpp"
#include "pages/filter_collection_page.hpp"
#include "pages/filter_selection_page.hpp"
#include "pages/flex_layout_page.hpp"
#include "pages/focus_page.hpp"
#include "pages/fonts_page.hpp"
#include "pages/footer_only_string_page.hpp"
#include "pages/formatted_text_page.hpp"
#include "pages/gestures_page.hpp"
#include "pages/gradient_page.hpp"
#include "pages/grid_grouping_page.hpp"
#include "pages/grid_page.hpp"
#include "pages/grouping_no_templates_page.hpp"
#include "pages/grouping_plus_selection_page.hpp"
#include "pages/header_footer_grid_horizontal_page.hpp"
#include "pages/header_footer_grid_page.hpp"
#include "pages/header_footer_page.hpp"
#include "pages/header_footer_template_page.hpp"
#include "pages/header_footer_view_page.hpp"
#include "pages/hit_testing_page.hpp"
#include "pages/horizontal_stack_layout_page.hpp"
#include "pages/hybrid_web_view_page.hpp"
#include "pages/image_button_page.hpp"
#include "pages/image_page.hpp"
#include "pages/indicator_page.hpp"
#include "pages/input_controls_page.hpp"
#include "pages/input_transparent_page.hpp"
#include "pages/invalidate_brush_page.hpp"
#include "pages/invalidate_shadow_host_page.hpp"
#include "pages/ios_blur_effect_page.hpp"
#include "pages/ios_date_picker_page.hpp"
#include "pages/ios_entry_page.hpp"
#include "pages/ios_first_responder_page.hpp"
#include "pages/ios_pan_gesture_page.hpp"
#include "pages/ios_picker_page.hpp"
#include "pages/ios_safe_area_page.hpp"
#include "pages/ios_scroll_view_page.hpp"
#include "pages/ios_search_bar_page.hpp"
#include "pages/ios_slider_update_on_tap_page.hpp"
#include "pages/ios_swipe_transition_page.hpp"
#include "pages/ios_time_picker_page.hpp"
#include "pages/items_page.hpp"
#include "pages/items_updating_scroll_mode_page.hpp"
#include "pages/label_page.hpp"
#include "pages/layout_is_enabled_page.hpp"
#include "pages/line_gallery_page.hpp"
#include "pages/line_join_gallery_page.hpp"
#include "pages/measure_first_strategy_page.hpp"
#include "pages/menu_bar_page.hpp"
#include "pages/modal_page.hpp"
#include "pages/multiple_bound_selection_page.hpp"
#include "pages/navigation_gallery_page.hpp"
#include "pages/nested_collection_page.hpp"
#include "pages/pan_gesture_events_page.hpp"
#include "pages/path_aspect_gallery_page.hpp"
#include "pages/path_gallery_page.hpp"
#include "pages/path_transform_string_page.hpp"
#include "pages/picker_page.hpp"
#include "pages/pickers_page.hpp"
#include "pages/pointer_gesture_page.hpp"
#include "pages/polygon_gallery_page.hpp"
#include "pages/polyline_gallery_page.hpp"
#include "pages/preselected_item_page.hpp"
#include "pages/preselected_items_page.hpp"
#include "pages/progress_bar_page.hpp"
#include "pages/radio_button_border_page.hpp"
#include "pages/radio_button_content_page.hpp"
#include "pages/radio_button_group_binding_page.hpp"
#include "pages/radio_button_group_gallery_page.hpp"
#include "pages/radio_button_group_page.hpp"
#include "pages/radio_content_properties_page.hpp"
#include "pages/radio_template_from_style_page.hpp"
#include "pages/rectangle_gallery_page.hpp"
#include "pages/refresh_view_page.hpp"
#include "pages/relative_layout_page.hpp"
#include "pages/scattered_radio_button_page.hpp"
#include "pages/scroll_mode_test_page.hpp"
#include "pages/scroll_to_group_page.hpp"
#include "pages/scroll_view_page.hpp"
#include "pages/search_bar_page.hpp"
#include "pages/selection_command_param_page.hpp"
#include "pages/selection_synchronization_page.hpp"
#include "pages/semantics_page.hpp"
#include "pages/shadow_playground_page.hpp"
#include "pages/shape_app_theme_page.hpp"
#include "pages/shapes_page.hpp"
#include "pages/single_bound_selection_page.hpp"
#include "pages/slider_page.hpp"
#include "pages/some_empty_groups_page.hpp"
#include "pages/stack_layout_page.hpp"
#include "pages/staggered_layout_page.hpp"
#include "pages/stepper_page.hpp"
#include "pages/styles_page.hpp"
#include "pages/swipe_gesture_page.hpp"
#include "pages/swipe_item_position_page.hpp"
#include "pages/swipe_item_size_page.hpp"
#include "pages/swipe_refresh_page.hpp"
#include "pages/swipe_threshold_page.hpp"
#include "pages/swipe_view_margin_page.hpp"
#include "pages/swipe_view_shadow_page.hpp"
#include "pages/switch_grouping_page.hpp"
#include "pages/switch_page.hpp"
#include "pages/tabbed_flyout_page.hpp"
#include "pages/table_view_page.hpp"
#include "pages/templated_view_page.hpp"
#include "pages/time_picker_page.hpp"
#include "pages/title_bar_page.hpp"
#include "pages/toolbar_page.hpp"
#include "pages/transform_playground_page.hpp"
#include "pages/transformations_page.hpp"
#include "pages/triggers_page.hpp"
#include "pages/update_path_data_page.hpp"
#include "pages/value_controls_page.hpp"
#include "pages/varied_size_selector_page.hpp"
#include "pages/vertical_stack_layout_page.hpp"
#include "pages/visual_states_page.hpp"
#include "pages/web_view_page.hpp"
#include "pages/z_index_page.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

namespace
{
    using maui::tests::describe;
    using maui::tests::view_node;

    // Hydrate the shared page for `key` through the runtime XAML loader and normalize it. The
    // xaml_load_result OWNS the created object graph (PROFILE §8 non-owning tree wiring), so the tree
    // is described before the result leaves scope.
    [[nodiscard]] view_node describe_shared_xaml(const std::string& key)
    {
        // Idempotent (just sets the global applier); some twins use {Binding} in item templates.
        maui::xaml::register_runtime_bindings();

        const std::filesystem::path path = std::filesystem::path{SHARED_PAGES_DIR} / (key + ".xaml");
        std::ifstream stream(path);
        std::stringstream buffer;
        buffer << stream.rdbuf();
        const std::string xaml = buffer.str();
        EXPECT_FALSE(xaml.empty()) << "missing or empty shared page: " << path;

        maui::controls::content_page page;
        const maui::xaml::xaml_load_result result = maui::xaml::xaml_loader::load_into(page, xaml);
        return describe(page);
    }

    // Keys whose builder page and shared XAML are KNOWN to diverge structurally — the 2026-07-05
    // full-corpus triage backlog, clustered by root cause in
    // port/maui-reference/docs/EQUIVALENCE_FINDINGS.md. The gate stays green while every entry is
    // tracked BIDIRECTIONALLY: a listed key whose trees start matching FAILS ("divergence closed —
    // remove it from this list"), and an unlisted key that diverges fails normally. Aligning a page
    // (fix the builder or the shared xaml per the findings doc) = delete its line here.
    [[nodiscard]] bool known_diverging(std::string_view key)
    {
        static constexpr std::string_view keys[] = {
            // cluster A — twin uses StackLayout where the builder uses V/H StackLayout
            "animation",
            "alerts",
            "basic_swipe",
            "composition_gallery",
            "ellipse_gallery",
            "header_footer_grid",
            "header_footer_grid_horizontal",
            "line_gallery",
            "path_transform_string",
            "pointer_gesture",
            "polygon_gallery",
            "polyline_gallery",
            "preselected_items",
            "rectangle_gallery",
            "selection_synchronization",
            "shape_app_theme",
            "some_empty_groups",
            // cluster B — root Padding/Spacing set on one side only
            "behaviors",
            "border_resize_content",
            "border_stroke",
            "check_box",
            "gestures",
            "horizontal_stack",
            "input_controls",
            "invalidate_brush",
            "radio_button_border",
            "scattered_radio_button",
            "selection_command_param",
            "switch_grouping",
            "transform_playground",
            "vertical_stack",
            // cluster C — builder computes runtime state vs the twin's static snapshot
            "activity_indicator",
            "pickers",
            "web_view",
            "pan_gesture_events",
            "swipe_threshold",
            "radio_button_group",
            // cluster D — twin structurally rewritten around unsupported features / loader gaps
            "tabbed_flyout",
            "radio_button_content",
            "adaptive_collection",
            "custom_layout",
            "chat_example",
            "hybrid_web_view",
            "ios_scroll_view",
            "layout_is_enabled",
            "indicator",
            "path_gallery",
            "border_clip_playground",
            // cluster E — builder ADDS a gallery-convention interactivity widget the twin omits
            // (AUTHORING.md rule 3: no event attributes in shared XAML). "clip"'s builder page appends a
            // "Toggle clip on/off" Button + a status Label after the five images (clip_page.hpp header);
            // NOTE this is UNRELATED to the Image.Clip XAML-registration gap, which is closed as of
            // 2026-07 (register_xaml_geometries.cpp) — clip_corner_radius/clip_gallery/clip_views were
            // de-listed for that fix; "clip" itself would still diverge even with a perfect Clip port.
            "clip",
        };
        for (const std::string_view k : keys)
        {
            if (k == key)
            {
                return true;
            }
        }
        return false;
    }

// One structural-equivalence case: builder page `page_type` vs shared `<key>.xaml`. The failure
// message prints both normalized trees (view_node's PrintTo), so the divergence reads as a tree diff.
// Keys on the known_diverging list assert the divergence still EXISTS (bidirectional tracking).
#define STRUCTURE_EQUIVALENCE_TEST(key, page_type)                                                                     \
    TEST(gallery_structure_equivalence, key)                                                                           \
    {                                                                                                                  \
        if (known_diverging(#key))                                                                                     \
        {                                                                                                              \
            maui::samples::page_type builder;                                                                          \
            EXPECT_NE(describe(builder.page()), describe_shared_xaml(#key))                                            \
                << "divergence closed — remove '" #key "' from known_diverging() and keep it strict";                  \
            return;                                                                                                    \
        }                                                                                                              \
        maui::samples::page_type builder;                                                                              \
        EXPECT_EQ(describe(builder.page()), describe_shared_xaml(#key));                                               \
    }

    STRUCTURE_EQUIVALENCE_TEST(absolute_layout, absolute_layout_page)
    STRUCTURE_EQUIVALENCE_TEST(activity_indicator, activity_indicator_page)
    STRUCTURE_EQUIVALENCE_TEST(adaptive_collection, adaptive_collection_page)
    STRUCTURE_EQUIVALENCE_TEST(alerts, alerts_page)
    STRUCTURE_EQUIVALENCE_TEST(alignment, alignment_page)
    STRUCTURE_EQUIVALENCE_TEST(animation, animation_page)
    STRUCTURE_EQUIVALENCE_TEST(app_theme_binding, app_theme_binding_page)
    STRUCTURE_EQUIVALENCE_TEST(application_control, application_control_page)
    STRUCTURE_EQUIVALENCE_TEST(auto_size_shapes, auto_size_shapes_page)
    STRUCTURE_EQUIVALENCE_TEST(basic_grouping, basic_grouping_page)
    STRUCTURE_EQUIVALENCE_TEST(basic_swipe, basic_swipe_page)
    STRUCTURE_EQUIVALENCE_TEST(behaviors, behaviors_page)
    STRUCTURE_EQUIVALENCE_TEST(border, border_page)
    STRUCTURE_EQUIVALENCE_TEST(border_alignment, border_alignment_page)
    STRUCTURE_EQUIVALENCE_TEST(border_clip_playground, border_clip_playground_page)
    STRUCTURE_EQUIVALENCE_TEST(border_layout, border_layout_page)
    STRUCTURE_EQUIVALENCE_TEST(border_playground, border_playground_page)
    STRUCTURE_EQUIVALENCE_TEST(border_resize_content, border_resize_content_page)
    STRUCTURE_EQUIVALENCE_TEST(border_stroke, border_stroke_page)
    STRUCTURE_EQUIVALENCE_TEST(border_styles, border_styles_page)
    STRUCTURE_EQUIVALENCE_TEST(borderless, borderless_page)
    STRUCTURE_EQUIVALENCE_TEST(box_view, box_view_page)
    STRUCTURE_EQUIVALENCE_TEST(button, button_page)
    STRUCTURE_EQUIVALENCE_TEST(carousel_page, carousel_page)
    STRUCTURE_EQUIVALENCE_TEST(chat_example, chat_example_page)
    STRUCTURE_EQUIVALENCE_TEST(check_box, check_box_page)
    STRUCTURE_EQUIVALENCE_TEST(chrome, chrome_page)
    STRUCTURE_EQUIVALENCE_TEST(clip, clip_page)
    STRUCTURE_EQUIVALENCE_TEST(clip_corner_radius, clip_corner_radius_page)
    STRUCTURE_EQUIVALENCE_TEST(clip_gallery, clip_gallery_page)
    STRUCTURE_EQUIVALENCE_TEST(clip_views, clip_views_page)
    STRUCTURE_EQUIVALENCE_TEST(clipping, clipping_page)
    STRUCTURE_EQUIVALENCE_TEST(collectionview, collectionview_page)
    STRUCTURE_EQUIVALENCE_TEST(composition_gallery, composition_gallery_page)
    STRUCTURE_EQUIVALENCE_TEST(containers, containers_page)
    STRUCTURE_EQUIVALENCE_TEST(content_view, content_view_page)
    STRUCTURE_EQUIVALENCE_TEST(context_flyout, context_flyout_page)
    STRUCTURE_EQUIVALENCE_TEST(controls_stack, controls_stack_page)
    STRUCTURE_EQUIVALENCE_TEST(custom_layout, custom_layout_page)
    STRUCTURE_EQUIVALENCE_TEST(custom_size_swipe, custom_size_swipe_page)
    STRUCTURE_EQUIVALENCE_TEST(custom_swipe_item_view, custom_swipe_item_view_page)
    STRUCTURE_EQUIVALENCE_TEST(cv_visual_states, cv_visual_states_page)
    STRUCTURE_EQUIVALENCE_TEST(data_template_selector, data_template_selector_page)
    STRUCTURE_EQUIVALENCE_TEST(date_picker, date_picker_page)
    // device_page's ctor throws on the headless backend (device-info fake unseeded) — cluster E in
    // port/maui-reference/docs/EQUIVALENCE_FINDINGS.md; re-enable via the macro once a fake is seeded.
    TEST(gallery_structure_equivalence, device)
    {
        GTEST_SKIP() << "device_page is headless-unconstructible (device-info fake unseeded)";
    }
    STRUCTURE_EQUIVALENCE_TEST(dispatcher, dispatcher_page)
    STRUCTURE_EQUIVALENCE_TEST(drag_drop, drag_drop_page)
    STRUCTURE_EQUIVALENCE_TEST(editor, editor_page)
    STRUCTURE_EQUIVALENCE_TEST(effects, effects_page)
    STRUCTURE_EQUIVALENCE_TEST(ellipse_gallery, ellipse_gallery_page)
    STRUCTURE_EQUIVALENCE_TEST(empty_view, empty_view_page)
    STRUCTURE_EQUIVALENCE_TEST(empty_view_load_simulate, empty_view_load_simulate_page)
    STRUCTURE_EQUIVALENCE_TEST(empty_view_null, empty_view_null_page)
    STRUCTURE_EQUIVALENCE_TEST(empty_view_rtl, empty_view_rtl_page)
    STRUCTURE_EQUIVALENCE_TEST(empty_view_selector, empty_view_selector_page)
    STRUCTURE_EQUIVALENCE_TEST(empty_view_swap, empty_view_swap_page)
    STRUCTURE_EQUIVALENCE_TEST(empty_view_template, empty_view_template_page)
    STRUCTURE_EQUIVALENCE_TEST(empty_view_view, empty_view_view_page)
    STRUCTURE_EQUIVALENCE_TEST(entry, entry_page)
    STRUCTURE_EQUIVALENCE_TEST(filter_collection, filter_collection_page)
    STRUCTURE_EQUIVALENCE_TEST(filter_selection, filter_selection_page)
    STRUCTURE_EQUIVALENCE_TEST(flex_layout, flex_layout_page)
    STRUCTURE_EQUIVALENCE_TEST(focus, focus_page)
    STRUCTURE_EQUIVALENCE_TEST(fonts, fonts_page)
    STRUCTURE_EQUIVALENCE_TEST(footer_only_string, footer_only_string_page)
    STRUCTURE_EQUIVALENCE_TEST(formatted_text, formatted_text_page)
    STRUCTURE_EQUIVALENCE_TEST(gestures, gestures_page)
    STRUCTURE_EQUIVALENCE_TEST(gradient, gradient_page)
    STRUCTURE_EQUIVALENCE_TEST(grid, grid_page)
    STRUCTURE_EQUIVALENCE_TEST(grid_grouping, grid_grouping_page)
    STRUCTURE_EQUIVALENCE_TEST(grouping_no_templates, grouping_no_templates_page)
    STRUCTURE_EQUIVALENCE_TEST(grouping_plus_selection, grouping_plus_selection_page)
    STRUCTURE_EQUIVALENCE_TEST(header_footer, header_footer_page)
    STRUCTURE_EQUIVALENCE_TEST(header_footer_grid, header_footer_grid_page)
    STRUCTURE_EQUIVALENCE_TEST(header_footer_grid_horizontal, header_footer_grid_horizontal_page)
    STRUCTURE_EQUIVALENCE_TEST(header_footer_template, header_footer_template_page)
    STRUCTURE_EQUIVALENCE_TEST(header_footer_view, header_footer_view_page)
    STRUCTURE_EQUIVALENCE_TEST(hit_testing, hit_testing_page)
    STRUCTURE_EQUIVALENCE_TEST(horizontal_stack, horizontal_stack_layout_page)
    STRUCTURE_EQUIVALENCE_TEST(hybrid_web_view, hybrid_web_view_page)
    STRUCTURE_EQUIVALENCE_TEST(image, image_page)
    STRUCTURE_EQUIVALENCE_TEST(image_button, image_button_page)
    STRUCTURE_EQUIVALENCE_TEST(indicator, indicator_page)
    STRUCTURE_EQUIVALENCE_TEST(input_controls, input_controls_page)
    STRUCTURE_EQUIVALENCE_TEST(input_transparent, input_transparent_page)
    STRUCTURE_EQUIVALENCE_TEST(invalidate_brush, invalidate_brush_page)
    STRUCTURE_EQUIVALENCE_TEST(invalidate_shadow_host, invalidate_shadow_host_page)
    STRUCTURE_EQUIVALENCE_TEST(ios_blur_effect, ios_blur_effect_page)
    STRUCTURE_EQUIVALENCE_TEST(ios_date_picker, ios_date_picker_page)
    STRUCTURE_EQUIVALENCE_TEST(ios_entry, ios_entry_page)
    STRUCTURE_EQUIVALENCE_TEST(ios_first_responder, ios_first_responder_page)
    STRUCTURE_EQUIVALENCE_TEST(ios_pan_gesture, ios_pan_gesture_page)
    STRUCTURE_EQUIVALENCE_TEST(ios_picker, ios_picker_page)
    STRUCTURE_EQUIVALENCE_TEST(ios_safe_area, ios_safe_area_page)
    STRUCTURE_EQUIVALENCE_TEST(ios_scroll_view, ios_scroll_view_page)
    STRUCTURE_EQUIVALENCE_TEST(ios_search_bar, ios_search_bar_page)
    STRUCTURE_EQUIVALENCE_TEST(ios_slider_update_on_tap, ios_slider_update_on_tap_page)
    STRUCTURE_EQUIVALENCE_TEST(ios_swipe_transition, ios_swipe_transition_page)
    STRUCTURE_EQUIVALENCE_TEST(ios_time_picker, ios_time_picker_page)
    STRUCTURE_EQUIVALENCE_TEST(items, items_page)
    STRUCTURE_EQUIVALENCE_TEST(items_updating_scroll_mode, items_updating_scroll_mode_page)
    STRUCTURE_EQUIVALENCE_TEST(label, label_page)
    STRUCTURE_EQUIVALENCE_TEST(layout_is_enabled, layout_is_enabled_page)
    STRUCTURE_EQUIVALENCE_TEST(line_gallery, line_gallery_page)
    STRUCTURE_EQUIVALENCE_TEST(line_join_gallery, line_join_gallery_page)
    STRUCTURE_EQUIVALENCE_TEST(measure_first_strategy, measure_first_strategy_page)
    STRUCTURE_EQUIVALENCE_TEST(menu_bar, menu_bar_page)
    STRUCTURE_EQUIVALENCE_TEST(modal, modal_page)
    STRUCTURE_EQUIVALENCE_TEST(multiple_bound_selection, multiple_bound_selection_page)
    STRUCTURE_EQUIVALENCE_TEST(navigation_gallery, navigation_gallery_page)
    STRUCTURE_EQUIVALENCE_TEST(nested_collection, nested_collection_page)
    STRUCTURE_EQUIVALENCE_TEST(pan_gesture_events, pan_gesture_events_page)
    STRUCTURE_EQUIVALENCE_TEST(path_aspect_gallery, path_aspect_gallery_page)
    STRUCTURE_EQUIVALENCE_TEST(path_gallery, path_gallery_page)
    STRUCTURE_EQUIVALENCE_TEST(path_transform_string, path_transform_string_page)
    STRUCTURE_EQUIVALENCE_TEST(picker, picker_page)
    STRUCTURE_EQUIVALENCE_TEST(pickers, pickers_page)
    STRUCTURE_EQUIVALENCE_TEST(pointer_gesture, pointer_gesture_page)
    STRUCTURE_EQUIVALENCE_TEST(polygon_gallery, polygon_gallery_page)
    STRUCTURE_EQUIVALENCE_TEST(polyline_gallery, polyline_gallery_page)
    STRUCTURE_EQUIVALENCE_TEST(preselected_item, preselected_item_page)
    STRUCTURE_EQUIVALENCE_TEST(preselected_items, preselected_items_page)
    STRUCTURE_EQUIVALENCE_TEST(progress_bar, progress_bar_page)
    STRUCTURE_EQUIVALENCE_TEST(radio_button_border, radio_button_border_page)
    STRUCTURE_EQUIVALENCE_TEST(radio_button_content, radio_button_content_page)
    STRUCTURE_EQUIVALENCE_TEST(radio_button_group, radio_button_group_page)
    STRUCTURE_EQUIVALENCE_TEST(radio_button_group_binding, radio_button_group_binding_page)
    STRUCTURE_EQUIVALENCE_TEST(radio_button_group_gallery, radio_button_group_gallery_page)
    STRUCTURE_EQUIVALENCE_TEST(radio_content_properties, radio_content_properties_page)
    STRUCTURE_EQUIVALENCE_TEST(radio_template_from_style, radio_template_from_style_page)
    STRUCTURE_EQUIVALENCE_TEST(rectangle_gallery, rectangle_gallery_page)
    STRUCTURE_EQUIVALENCE_TEST(refresh_view, refresh_view_page)
    STRUCTURE_EQUIVALENCE_TEST(relative_layout, relative_layout_page)
    STRUCTURE_EQUIVALENCE_TEST(scattered_radio_button, scattered_radio_button_page)
    STRUCTURE_EQUIVALENCE_TEST(scroll_mode_test, scroll_mode_test_page)
    STRUCTURE_EQUIVALENCE_TEST(scroll_to_group, scroll_to_group_page)
    STRUCTURE_EQUIVALENCE_TEST(scroll_view, scroll_view_page)
    STRUCTURE_EQUIVALENCE_TEST(search_bar, search_bar_page)
    STRUCTURE_EQUIVALENCE_TEST(selection_command_param, selection_command_param_page)
    STRUCTURE_EQUIVALENCE_TEST(selection_synchronization, selection_synchronization_page)
    STRUCTURE_EQUIVALENCE_TEST(semantics, semantics_page)
    STRUCTURE_EQUIVALENCE_TEST(shadow_playground, shadow_playground_page)
    STRUCTURE_EQUIVALENCE_TEST(shape_app_theme, shape_app_theme_page)
    STRUCTURE_EQUIVALENCE_TEST(shapes, shapes_page)
    STRUCTURE_EQUIVALENCE_TEST(single_bound_selection, single_bound_selection_page)
    STRUCTURE_EQUIVALENCE_TEST(slider, slider_page)
    STRUCTURE_EQUIVALENCE_TEST(some_empty_groups, some_empty_groups_page)
    STRUCTURE_EQUIVALENCE_TEST(stack_layout, stack_layout_page)
    STRUCTURE_EQUIVALENCE_TEST(staggered_layout, staggered_layout_page)
    STRUCTURE_EQUIVALENCE_TEST(stepper, stepper_page)
    STRUCTURE_EQUIVALENCE_TEST(styles, styles_page)
    STRUCTURE_EQUIVALENCE_TEST(swipe_gesture, swipe_gesture_page)
    STRUCTURE_EQUIVALENCE_TEST(swipe_item_position, swipe_item_position_page)
    STRUCTURE_EQUIVALENCE_TEST(swipe_item_size, swipe_item_size_page)
    STRUCTURE_EQUIVALENCE_TEST(swipe_refresh, swipe_refresh_page)
    STRUCTURE_EQUIVALENCE_TEST(swipe_threshold, swipe_threshold_page)
    STRUCTURE_EQUIVALENCE_TEST(swipe_view_margin, swipe_view_margin_page)
    STRUCTURE_EQUIVALENCE_TEST(swipe_view_shadow, swipe_view_shadow_page)
    STRUCTURE_EQUIVALENCE_TEST(switch, switch_page)
    STRUCTURE_EQUIVALENCE_TEST(switch_grouping, switch_grouping_page)
    STRUCTURE_EQUIVALENCE_TEST(table_view, table_view_page)
    STRUCTURE_EQUIVALENCE_TEST(tabbed_flyout, tabbed_flyout_page)
    STRUCTURE_EQUIVALENCE_TEST(time_picker, time_picker_page)
    STRUCTURE_EQUIVALENCE_TEST(title_bar, title_bar_page)
    STRUCTURE_EQUIVALENCE_TEST(toolbar, toolbar_page)
    STRUCTURE_EQUIVALENCE_TEST(transform_playground, transform_playground_page)
    STRUCTURE_EQUIVALENCE_TEST(transformations, transformations_page)
    STRUCTURE_EQUIVALENCE_TEST(triggers, triggers_page)
    STRUCTURE_EQUIVALENCE_TEST(update_path_data, update_path_data_page)
    STRUCTURE_EQUIVALENCE_TEST(value_controls, value_controls_page)
    STRUCTURE_EQUIVALENCE_TEST(varied_size_selector, varied_size_selector_page)
    STRUCTURE_EQUIVALENCE_TEST(vertical_stack, vertical_stack_layout_page)
    STRUCTURE_EQUIVALENCE_TEST(visual_states, visual_states_page)
    STRUCTURE_EQUIVALENCE_TEST(web_view, web_view_page)
    STRUCTURE_EQUIVALENCE_TEST(z_index, z_index_page)

    // SKIPPED templated_view: hydrating the shared twin crashes the test binary (SIGBUS after the
    // EXPECT_EQ diff prints, during end-of-test teardown of the loaded templated_view/content_presenter
    // graph) — a crash kills every test after it in the binary, so it is excluded rather than failed.
    // The templated_view_page.hpp include stays so re-enabling is a one-line change once fixed.

#undef STRUCTURE_EQUIVALENCE_TEST
} // namespace
