# MAUI C++ port — runnable example gallery

A code-first **C++23** port of .NET MAUI's `Controls.Sample` pages, each wired into the runnable demo gallery and captured on **macOS (AppKit)** + **iOS (UIKit)** with a screenshot, an iOS GIF, and a per-page README mapping it to its C# oracle. Behavior is derived from the read-only C# source, never invented; per-page deviations are documented honestly.

**Run any page** — macOS: `MAUI_SAMPLE_PAGE=<key> ./build/apple/maui_macos_gallery` · iOS: `SIMCTL_CHILD_MAUI_SAMPLE_PAGE=<key> xcrun simctl launch booted dev.maui-cpp.ios-gallery`.

**164 example pages.** Two genuine framework gaps the sample work surfaced were root-caused + fixed with full-gate framework changes + TDD tests: **CollectionView struct-typed templated-cell rendering** (porting C#'s `TemplatedCell.Bind` to the AppKit/iOS handlers) and **`View.HorizontalOptions`/`VerticalOptions`** (made settable + honored at `view::arrange`, the C# `ComputeFrame` path).

## Curated thematic demos (11)

[value_controls](value_controls/) · [input_controls](input_controls/) · [pickers](pickers/) · [formatted_text](formatted_text/) · [items](items/) · [shapes](shapes/) · [containers](containers/) · [swipe_refresh](swipe_refresh/) · [web_view](web_view/) · [chrome](chrome/) · [tabbed_flyout](tabbed_flyout/)

## Per-control pages (Pages/Controls) (22)

[button](button/) · [label](label/) · [image](image/) · [entry](entry/) · [editor](editor/) · [search_bar](search_bar/) · [check_box](check_box/) · [switch](switch/) · [slider](slider/) · [stepper](stepper/) · [progress_bar](progress_bar/) · [activity_indicator](activity_indicator/) · [box_view](box_view/) · [date_picker](date_picker/) · [time_picker](time_picker/) · [picker](picker/) · [image_button](image_button/) · [refresh_view](refresh_view/) · [indicator](indicator/) · [shapes_demo](shapes_demo/) · [title_bar](title_bar/) · [hybrid_web_view](hybrid_web_view/)

## Layouts (Pages/Layouts) (14)

[absolute_layout](absolute_layout/) · [grid](grid/) · [flex_layout](flex_layout/) · [stack_layout](stack_layout/) · [vertical_stack](vertical_stack/) · [horizontal_stack](horizontal_stack/) · [scroll_view](scroll_view/) · [content_view](content_view/) · [z_index](z_index/) · [clipping](clipping/) · [templated_view](templated_view/) · [layout_is_enabled](layout_is_enabled/) · [custom_layout](custom_layout/) · [relative_layout](relative_layout/)

## Feature subsystems (Core + UserInterface) (28)

[brushes](brushes/) · [transformations](transformations/) · [gestures](gestures/) · [animation](animation/) · [styles](styles/) · [triggers](triggers/) · [behaviors](behaviors/) · [visual_states](visual_states/) · [fonts](fonts/) · [alerts](alerts/) · [semantics](semantics/) · [focus](focus/) · [dispatcher](dispatcher/) · [device](device/) · [app_theme_binding](app_theme_binding/) · [toolbar](toolbar/) · [effects](effects/) · [input_transparent](input_transparent/) · [clip](clip/) · [context_flyout](context_flyout/) · [menu_bar](menu_bar/) · [navigation_gallery](navigation_gallery/) · [modal](modal/) · [application_control](application_control/) · [pointer_gesture](pointer_gesture/) · [drag_drop](drag_drop/) · [hit_testing](hit_testing/) · [pan_gesture_events](pan_gesture_events/)

## Shapes galleries (native CoreGraphics render) (18)

[ellipse_gallery](ellipse_gallery/) · [rectangle_gallery](rectangle_gallery/) · [line_gallery](line_gallery/) · [polygon_gallery](polygon_gallery/) · [polyline_gallery](polyline_gallery/) · [path_gallery](path_gallery/) · [line_join_gallery](line_join_gallery/) · [path_aspect_gallery](path_aspect_gallery/) · [composition_gallery](composition_gallery/) · [transform_playground](transform_playground/) · [path_transform_string](path_transform_string/) · [shape_app_theme](shape_app_theme/) · [clip_gallery](clip_gallery/) · [clip_views](clip_views/) · [clip_corner_radius](clip_corner_radius/) · [auto_size_shapes](auto_size_shapes/) · [invalidate_brush](invalidate_brush/) · [update_path_data](update_path_data/)

## Border galleries (9)

[border_styles](border_styles/) · [border_stroke](border_stroke/) · [border_playground](border_playground/) · [border_layout](border_layout/) · [border_alignment](border_alignment/) · [border_clip_playground](border_clip_playground/) · [borderless](borderless/) · [border_resize_content](border_resize_content/) · [radio_button_border](radio_button_border/)

## RadioButton + Shadow galleries (9)

[radio_button_group](radio_button_group/) · [radio_button_group_binding](radio_button_group_binding/) · [scattered_radio_button](scattered_radio_button/) · [radio_button_content](radio_button_content/) · [radio_content_properties](radio_content_properties/) · [radio_template_from_style](radio_template_from_style/) · [shadow_playground](shadow_playground/) · [invalidate_shadow_host](invalidate_shadow_host/) · [radio_button_group_gallery](radio_button_group_gallery/)

## SwipeView galleries (9)

[basic_swipe](basic_swipe/) · [swipe_item_position](swipe_item_position/) · [swipe_view_shadow](swipe_view_shadow/) · [custom_swipe_item_view](custom_swipe_item_view/) · [swipe_item_size](swipe_item_size/) · [swipe_view_margin](swipe_view_margin/) · [custom_size_swipe](custom_size_swipe/) · [swipe_gesture](swipe_gesture/) · [swipe_threshold](swipe_threshold/)

## CollectionView + CarouselView galleries (35)

[filter_collection](filter_collection/) · [basic_grouping](basic_grouping/) · [selection_mode](selection_mode/) · [header_footer](header_footer/) · [empty_view](empty_view/) · [data_template_selector](data_template_selector/) · [adaptive_collection](adaptive_collection/) · [single_bound_selection](single_bound_selection/) · [chat_example](chat_example/) · [header_footer_grid](header_footer_grid/) · [header_footer_template](header_footer_template/) · [header_footer_view](header_footer_view/) · [multiple_bound_selection](multiple_bound_selection/) · [selection_command_param](selection_command_param/) · [preselected_items](preselected_items/) · [grid_grouping](grid_grouping/) · [some_empty_groups](some_empty_groups/) · [empty_view_template](empty_view_template/) · [staggered_layout](staggered_layout/) · [nested_collection](nested_collection/) · [varied_size_selector](varied_size_selector/) · [scroll_to_group](scroll_to_group/) · [carousel](carousel/) · [empty_view_swap](empty_view_swap/) · [empty_view_view](empty_view_view/) · [empty_view_selector](empty_view_selector/) · [grouping_no_templates](grouping_no_templates/) · [cv_visual_states](cv_visual_states/) · [switch_grouping](switch_grouping/) · [grouping_plus_selection](grouping_plus_selection/) · [items_updating_scroll_mode](items_updating_scroll_mode/) · [measure_first_strategy](measure_first_strategy/) · [scroll_mode_test](scroll_mode_test/) · [footer_only_string](footer_only_string/) · [header_footer_grid_horizontal](header_footer_grid_horizontal/)

## PlatformSpecifics/iOS (platform-configuration) (9)

[ios_entry](ios_entry/) · [ios_date_picker](ios_date_picker/) · [ios_picker](ios_picker/) · [ios_slider_update_on_tap](ios_slider_update_on_tap/) · [ios_scroll_view](ios_scroll_view/) · [ios_search_bar](ios_search_bar/) · [ios_time_picker](ios_time_picker/) · [ios_safe_area](ios_safe_area/) · [ios_blur_effect](ios_blur_effect/)
