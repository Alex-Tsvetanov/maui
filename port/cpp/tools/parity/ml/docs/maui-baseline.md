# pixel_score --verify baseline (2026-10-13, box)
- Command: python3 tools/parity/lib/pixel_score.py --verify (in port/cpp). Exit code 1 (cells differ). comparison.json sha256 unchanged.
- Wall time: ~1119 s (18m39s), 172 pages, 1376 cells scored.
- MAUI vs C++ (pixel): green 666 / yellow 22 / red 0 (stored: 672/16/0)
- MAUI vs XAML (pixel_xaml): green 667 / yellow 21 / red 0 (stored: 671/16/1)
- 50 cells differ from stored; all are status flips, mostly green<->yellow (borderline near SSIM 0.98 / diff 1%). Only big one: selection_synchronization/ios/pixel_xaml red->green.
## All changes
  CHANGED activity_indicator/android/pixel: status yellow -> green
  CHANGED activity_indicator/android/pixel_xaml: status yellow -> green
  CHANGED basic_grouping/ios/pixel: status green -> yellow, motion None -> INVALID
  CHANGED basic_grouping/ios/pixel_xaml: status green -> yellow, motion None -> INVALID
  CHANGED basic_grouping/maccatalyst/pixel: status green -> yellow, motion None -> INVALID
  CHANGED basic_grouping/maccatalyst/pixel_xaml: status green -> yellow, motion None -> INVALID
  CHANGED basic_grouping/windows/pixel: status yellow -> green
  CHANGED basic_grouping/windows/pixel_xaml: status yellow -> green
  CHANGED box_view/android/pixel: status yellow -> green
  CHANGED box_view/android/pixel_xaml: status yellow -> green
  CHANGED carousel_page/ios/pixel: status yellow -> green
  CHANGED carousel_page/ios/pixel_xaml: status yellow -> green
  CHANGED carousel_page/maccatalyst/pixel: status green -> yellow
  CHANGED carousel_page/maccatalyst/pixel_xaml: status green -> yellow
  CHANGED carousel_page/android/pixel: status yellow -> green
  CHANGED carousel_page/android/pixel_xaml: status yellow -> green
  CHANGED check_box/android/pixel: status green -> yellow
  CHANGED check_box/android/pixel_xaml: status green -> yellow
  CHANGED entry/maccatalyst/pixel: status green -> yellow
  CHANGED entry/maccatalyst/pixel_xaml: status green -> yellow
  CHANGED ios_blur_effect/android/pixel: status green -> yellow
  CHANGED ios_blur_effect/android/pixel_xaml: status green -> yellow
  CHANGED ios_date_picker/ios/pixel: status yellow -> green
  CHANGED ios_date_picker/ios/pixel_xaml: status yellow -> green
  CHANGED radio_content_properties/maccatalyst/pixel: status green -> yellow
  CHANGED radio_content_properties/maccatalyst/pixel_xaml: status green -> yellow
  CHANGED scroll_to_group/android/pixel: status green -> yellow
  CHANGED scroll_to_group/android/pixel_xaml: status green -> yellow
  CHANGED scroll_view/ios/pixel_xaml: status yellow -> green
  CHANGED scroll_view/android/pixel: status yellow -> green
  CHANGED scroll_view/android/pixel_xaml: status yellow -> green
  CHANGED search_bar/windows/pixel: status green -> yellow
  CHANGED search_bar/windows/pixel_xaml: status green -> yellow
  CHANGED selection_synchronization/ios/pixel_xaml: status red -> green
  CHANGED selection_synchronization/maccatalyst/pixel: status green -> yellow
  CHANGED selection_synchronization/maccatalyst/pixel_xaml: status green -> yellow
  CHANGED selection_synchronization/android/pixel: status yellow -> green
  CHANGED selection_synchronization/android/pixel_xaml: status yellow -> green
  CHANGED semantics/windows/pixel: status green -> yellow
  CHANGED semantics/windows/pixel_xaml: status green -> yellow
  CHANGED stepper/android/pixel: status green -> yellow
  CHANGED stepper/android/pixel_xaml: status green -> yellow
  CHANGED swipe_refresh/ios/pixel: status yellow -> green
  CHANGED swipe_refresh/ios/pixel_xaml: status yellow -> green
  CHANGED swipe_refresh/maccatalyst/pixel: status green -> yellow
  CHANGED swipe_refresh/maccatalyst/pixel_xaml: status green -> yellow
  CHANGED swipe_refresh/windows/pixel: status green -> yellow
  CHANGED swipe_refresh/windows/pixel_xaml: status green -> yellow
  CHANGED switch/android/pixel: status green -> yellow
  CHANGED switch/android/pixel_xaml: status green -> yellow
## Full log
```
  [1/172] absolute_layout
  [2/172] activity_indicator
  [3/172] adaptive_collection
  [4/172] alerts
  [5/172] alignment
  [6/172] animation
  [7/172] app_theme_binding
  [8/172] application_control
  [9/172] auto_size_shapes
  [10/172] basic_grouping
  [11/172] basic_swipe
  [12/172] behaviors
  [13/172] border
  [14/172] border_clip_playground
  [15/172] border_layout
  [16/172] border_playground
  [17/172] border_resize_content
  [18/172] border_stroke
  [19/172] borderless
  [20/172] box_view
  [21/172] button
  [22/172] carousel_page
  [23/172] chat_example
  [24/172] check_box
  [25/172] chrome
  [26/172] clip
  [27/172] clip_corner_radius
  [28/172] clip_gallery
  [29/172] clip_views
  [30/172] clipping
  [31/172] collectionview
  [32/172] composition_gallery
  [33/172] containers
  [34/172] content_view
  [35/172] context_flyout
  [36/172] controls_stack
  [37/172] custom_layout
  [38/172] custom_size_swipe
  [39/172] custom_swipe_item_view
  [40/172] cv_visual_states
  [41/172] data_template_selector
  [42/172] date_picker
  [43/172] device
  [44/172] dispatcher
  [45/172] drag_drop
  [46/172] editor
  [47/172] effects
  [48/172] ellipse_gallery
  [49/172] empty_view
  [50/172] empty_view_load_simulate
  [51/172] empty_view_null
  [52/172] empty_view_rtl
  [53/172] empty_view_selector
  [54/172] empty_view_swap
  [55/172] empty_view_template
  [56/172] empty_view_view
  [57/172] entry
  [58/172] filter_collection
  [59/172] filter_selection
  [60/172] flex_layout
  [61/172] focus
  [62/172] fonts
  [63/172] footer_only_string
  [64/172] formatted_text
  [65/172] gestures
  [66/172] gradient
  [67/172] grid
  [68/172] grid_grouping
  [69/172] grouping_no_templates
  [70/172] grouping_plus_selection
  [71/172] header_footer
  [72/172] header_footer_grid
  [73/172] header_footer_grid_horizontal
  [74/172] header_footer_template
  [75/172] header_footer_view
  [76/172] hit_testing
  [77/172] horizontal_stack
  [78/172] hybrid_web_view
  [79/172] image
  [80/172] image_button
  [81/172] indicator
  [82/172] input_controls
  [83/172] input_transparent
  [84/172] invalidate_brush
  [85/172] invalidate_shadow_host
  [86/172] ios_blur_effect
  [87/172] ios_date_picker
  [88/172] ios_entry
  [89/172] ios_first_responder
  [90/172] ios_pan_gesture
  [91/172] ios_picker
  [92/172] ios_safe_area
  [93/172] ios_scroll_view
  [94/172] ios_search_bar
  [95/172] ios_slider_update_on_tap
  [96/172] ios_swipe_transition
  [97/172] ios_time_picker
  [98/172] items
  [99/172] items_updating_scroll_mode
  [100/172] label
  [101/172] layout_is_enabled
  [102/172] line_gallery
  [103/172] line_join_gallery
  [104/172] measure_first_strategy
  [105/172] menu_bar
  [106/172] modal
  [107/172] multiple_bound_selection
  [108/172] navigation_gallery
  [109/172] nested_collection
  [110/172] pan_gesture_events
  [111/172] path_aspect_gallery
  [112/172] path_gallery
  [113/172] path_transform_string
  [114/172] picker
  [115/172] pickers
  [116/172] pointer_gesture
  [117/172] polygon_gallery
  [118/172] polyline_gallery
  [119/172] preselected_item
  [120/172] preselected_items
  [121/172] progress_bar
  [122/172] radio_button_border
  [123/172] radio_button_content
  [124/172] radio_button_group
  [125/172] radio_button_group_binding
  [126/172] radio_button_group_gallery
  [127/172] radio_content_properties
  [128/172] radio_template_from_style
  [129/172] rectangle_gallery
  [130/172] refresh_view
  [131/172] relative_layout
  [132/172] scattered_radio_button
  [133/172] scroll_mode_test
  [134/172] scroll_to_group
  [135/172] scroll_view
  [136/172] search_bar
  [137/172] selection_command_param
  [138/172] selection_synchronization
  [139/172] semantics
  [140/172] shadow_playground
  [141/172] shape_app_theme
  [142/172] shapes
  [143/172] single_bound_selection
  [144/172] slider
  [145/172] some_empty_groups
  [146/172] stack_layout
  [147/172] staggered_layout
  [148/172] stepper
  [149/172] styles
  [150/172] swipe_gesture
  [151/172] swipe_item_position
  [152/172] swipe_item_size
  [153/172] swipe_refresh
  [154/172] swipe_threshold
  [155/172] swipe_view_margin
  [156/172] swipe_view_shadow
  [157/172] switch
  [158/172] switch_grouping
  [159/172] tabbed_flyout
  [160/172] templated_view
  [161/172] time_picker
  [162/172] title_bar
  [163/172] toolbar
  [164/172] transform_playground
  [165/172] transformations
  [166/172] triggers
  [167/172] update_path_data
  [168/172] varied_size_selector
  [169/172] vertical_stack
  [170/172] visual_states
  [171/172] web_view
  [172/172] z_index
  CHANGED activity_indicator/android/pixel: status yellow -> green
  CHANGED activity_indicator/android/pixel_xaml: status yellow -> green
  CHANGED basic_grouping/ios/pixel: status green -> yellow, motion None -> INVALID
  CHANGED basic_grouping/ios/pixel_xaml: status green -> yellow, motion None -> INVALID
  CHANGED basic_grouping/maccatalyst/pixel: status green -> yellow, motion None -> INVALID
  CHANGED basic_grouping/maccatalyst/pixel_xaml: status green -> yellow, motion None -> INVALID
  CHANGED basic_grouping/windows/pixel: status yellow -> green
  CHANGED basic_grouping/windows/pixel_xaml: status yellow -> green
  CHANGED box_view/android/pixel: status yellow -> green
  CHANGED box_view/android/pixel_xaml: status yellow -> green
  CHANGED carousel_page/ios/pixel: status yellow -> green
  CHANGED carousel_page/ios/pixel_xaml: status yellow -> green
  CHANGED carousel_page/maccatalyst/pixel: status green -> yellow
  CHANGED carousel_page/maccatalyst/pixel_xaml: status green -> yellow
  CHANGED carousel_page/android/pixel: status yellow -> green
  CHANGED carousel_page/android/pixel_xaml: status yellow -> green
  CHANGED check_box/android/pixel: status green -> yellow
  CHANGED check_box/android/pixel_xaml: status green -> yellow
  CHANGED entry/maccatalyst/pixel: status green -> yellow
  CHANGED entry/maccatalyst/pixel_xaml: status green -> yellow
  CHANGED ios_blur_effect/android/pixel: status green -> yellow
  CHANGED ios_blur_effect/android/pixel_xaml: status green -> yellow
  CHANGED ios_date_picker/ios/pixel: status yellow -> green
  CHANGED ios_date_picker/ios/pixel_xaml: status yellow -> green
  CHANGED radio_content_properties/maccatalyst/pixel: status green -> yellow
  CHANGED radio_content_properties/maccatalyst/pixel_xaml: status green -> yellow
  CHANGED scroll_to_group/android/pixel: status green -> yellow
  CHANGED scroll_to_group/android/pixel_xaml: status green -> yellow
  CHANGED scroll_view/ios/pixel_xaml: status yellow -> green
  CHANGED scroll_view/android/pixel: status yellow -> green
  CHANGED scroll_view/android/pixel_xaml: status yellow -> green
  CHANGED search_bar/windows/pixel: status green -> yellow
  CHANGED search_bar/windows/pixel_xaml: status green -> yellow
  CHANGED selection_synchronization/ios/pixel_xaml: status red -> green
  CHANGED selection_synchronization/maccatalyst/pixel: status green -> yellow
  CHANGED selection_synchronization/maccatalyst/pixel_xaml: status green -> yellow
  CHANGED selection_synchronization/android/pixel: status yellow -> green
  CHANGED selection_synchronization/android/pixel_xaml: status yellow -> green
  CHANGED semantics/windows/pixel: status green -> yellow
  CHANGED semantics/windows/pixel_xaml: status green -> yellow
  CHANGED stepper/android/pixel: status green -> yellow
  CHANGED stepper/android/pixel_xaml: status green -> yellow
  CHANGED swipe_refresh/ios/pixel: status yellow -> green
  CHANGED swipe_refresh/ios/pixel_xaml: status yellow -> green
  CHANGED swipe_refresh/maccatalyst/pixel: status green -> yellow
  CHANGED swipe_refresh/maccatalyst/pixel_xaml: status green -> yellow
  CHANGED swipe_refresh/windows/pixel: status green -> yellow
  CHANGED swipe_refresh/windows/pixel_xaml: status green -> yellow
  CHANGED switch/android/pixel: status green -> yellow
  CHANGED switch/android/pixel_xaml: status green -> yellow
--verify: 1376 scored, 50 cell(s) differ from /workspace/maui/port/cpp/docs/comparison/comparison.json (nothing written)
```
