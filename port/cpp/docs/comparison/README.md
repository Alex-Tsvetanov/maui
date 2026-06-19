# C++ port vs .NET MAUI — visual parity comparison

Each page rendered by **real shipped .NET MAUI** (left) vs the **C++ port** (right), on **iOS** and **macOS / Mac Catalyst**. Compare layout, geometry, and control rendering. The .NET MAUI captures use the OS dark appearance on Mac Catalyst; the C++ gallery renders in a light window. C++ macOS captures are post the AppKit top-left-origin flip fix, so both stacks render top-down.

**172 pages** captured against real .NET MAUI. iOS is the faithful reference platform for both stacks.

### iOS

| Page | .NET MAUI (iOS) | C++ port (iOS) |
| --- | --- | --- |
| Absolute Layout | ![cs](csharp_ios/absolute_layout.png) | ![cpp](../examples/absolute_layout/ios.png) |
| Activity Indicator | ![cs](csharp_ios/activity_indicator.png) | ![cpp](../examples/activity_indicator/ios.png) |
| Adaptive Collection | ![cs](csharp_ios/adaptive_collection.png) | ![cpp](../examples/adaptive_collection/ios.png) |
| Alerts | ![cs](csharp_ios/alerts.png) | ![cpp](../examples/alerts/ios.png) |
| Layout alignment (Start/Center/End/Fill) | ![cs](csharp_ios/alignment.png) | ![cpp](../examples/border_alignment/ios.png) |
| Animation | ![cs](csharp_ios/animation.png) | ![cpp](../examples/animation/ios.png) |
| App Theme Binding | ![cs](csharp_ios/app_theme_binding.png) | ![cpp](../examples/app_theme_binding/ios.png) |
| Application Control | ![cs](csharp_ios/application_control.png) | ![cpp](../examples/application_control/ios.png) |
| Auto Size Shapes | ![cs](csharp_ios/auto_size_shapes.png) | ![cpp](../examples/auto_size_shapes/ios.png) |
| Basic Grouping | ![cs](csharp_ios/basic_grouping.png) | ![cpp](../examples/basic_grouping/ios.png) |
| Basic Swipe | ![cs](csharp_ios/basic_swipe.png) | ![cpp](../examples/basic_swipe/ios.png) |
| Behaviors | ![cs](csharp_ios/behaviors.png) | ![cpp](../examples/behaviors/ios.png) |
| Border | ![cs](csharp_ios/border.png) | ![cpp](../examples/border_styles/ios.png) |
| Border Clip Playground | ![cs](csharp_ios/border_clip_playground.png) | ![cpp](../examples/border_clip_playground/ios.png) |
| Border Layout | ![cs](csharp_ios/border_layout.png) | ![cpp](../examples/border_layout/ios.png) |
| Border Playground | ![cs](csharp_ios/border_playground.png) | ![cpp](../examples/border_playground/ios.png) |
| Border Resize Content | ![cs](csharp_ios/border_resize_content.png) | ![cpp](../examples/border_resize_content/ios.png) |
| Border Stroke | ![cs](csharp_ios/border_stroke.png) | ![cpp](../examples/border_stroke/ios.png) |
| Borderless | ![cs](csharp_ios/borderless.png) | ![cpp](../examples/borderless/ios.png) |
| Box View | ![cs](csharp_ios/box_view.png) | ![cpp](../examples/box_view/ios.png) |
| Button | ![cs](csharp_ios/button.png) | ![cpp](../examples/button/ios.png) |
| Carousel Page | ![cs](csharp_ios/carousel_page.png) | — |
| Chat Example | ![cs](csharp_ios/chat_example.png) | ![cpp](../examples/chat_example/ios.png) |
| Check Box | ![cs](csharp_ios/check_box.png) | ![cpp](../examples/check_box/ios.png) |
| Chrome | ![cs](csharp_ios/chrome.png) | ![cpp](../examples/chrome/ios.png) |
| Clip | ![cs](csharp_ios/clip.png) | ![cpp](../examples/clip/ios.png) |
| Clip Corner Radius | ![cs](csharp_ios/clip_corner_radius.png) | ![cpp](../examples/clip_corner_radius/ios.png) |
| Clip Gallery | ![cs](csharp_ios/clip_gallery.png) | ![cpp](../examples/clip_gallery/ios.png) |
| Clip Views | ![cs](csharp_ios/clip_views.png) | ![cpp](../examples/clip_views/ios.png) |
| Clipping | ![cs](csharp_ios/clipping.png) | ![cpp](../examples/clipping/ios.png) |
| CollectionView | ![cs](csharp_ios/collectionview.png) | ![cpp](../examples/selection_mode/ios.png) |
| Composition Gallery | ![cs](csharp_ios/composition_gallery.png) | ![cpp](../examples/composition_gallery/ios.png) |
| Containers | ![cs](csharp_ios/containers.png) | ![cpp](../examples/containers/ios.png) |
| Content View | ![cs](csharp_ios/content_view.png) | ![cpp](../examples/content_view/ios.png) |
| Context Flyout | ![cs](csharp_ios/context_flyout.png) | ![cpp](../examples/context_flyout/ios.png) |
| Control stack | ![cs](csharp_ios/controls_stack.png) | ![cpp](../examples/value_controls/ios.png) |
| Custom Layout | ![cs](csharp_ios/custom_layout.png) | ![cpp](../examples/custom_layout/ios.png) |
| Custom Size Swipe | ![cs](csharp_ios/custom_size_swipe.png) | ![cpp](../examples/custom_size_swipe/ios.png) |
| Custom Swipe Item View | ![cs](csharp_ios/custom_swipe_item_view.png) | ![cpp](../examples/custom_swipe_item_view/ios.png) |
| Cv Visual States | ![cs](csharp_ios/cv_visual_states.png) | ![cpp](../examples/cv_visual_states/ios.png) |
| Data Template Selector | ![cs](csharp_ios/data_template_selector.png) | ![cpp](../examples/data_template_selector/ios.png) |
| Date Picker | ![cs](csharp_ios/date_picker.png) | ![cpp](../examples/date_picker/ios.png) |
| Device | ![cs](csharp_ios/device.png) | ![cpp](../examples/device/ios.png) |
| Dispatcher | ![cs](csharp_ios/dispatcher.png) | ![cpp](../examples/dispatcher/ios.png) |
| Drag Drop | ![cs](csharp_ios/drag_drop.png) | ![cpp](../examples/drag_drop/ios.png) |
| Editor | ![cs](csharp_ios/editor.png) | ![cpp](../examples/editor/ios.png) |
| Effects | ![cs](csharp_ios/effects.png) | ![cpp](../examples/effects/ios.png) |
| Ellipse Gallery | ![cs](csharp_ios/ellipse_gallery.png) | ![cpp](../examples/ellipse_gallery/ios.png) |
| Empty View | ![cs](csharp_ios/empty_view.png) | ![cpp](../examples/empty_view/ios.png) |
| Empty View Load Simulate | ![cs](csharp_ios/empty_view_load_simulate.png) | ![cpp](../examples/empty_view_load_simulate/ios.png) |
| Empty View Null | ![cs](csharp_ios/empty_view_null.png) | ![cpp](../examples/empty_view_null/ios.png) |
| Empty View Rtl | ![cs](csharp_ios/empty_view_rtl.png) | ![cpp](../examples/empty_view_rtl/ios.png) |
| Empty View Selector | ![cs](csharp_ios/empty_view_selector.png) | ![cpp](../examples/empty_view_selector/ios.png) |
| Empty View Swap | ![cs](csharp_ios/empty_view_swap.png) | ![cpp](../examples/empty_view_swap/ios.png) |
| Empty View Template | ![cs](csharp_ios/empty_view_template.png) | ![cpp](../examples/empty_view_template/ios.png) |
| Empty View View | ![cs](csharp_ios/empty_view_view.png) | ![cpp](../examples/empty_view_view/ios.png) |
| Entry | ![cs](csharp_ios/entry.png) | ![cpp](../examples/entry/ios.png) |
| Filter Collection | ![cs](csharp_ios/filter_collection.png) | ![cpp](../examples/filter_collection/ios.png) |
| Filter Selection | ![cs](csharp_ios/filter_selection.png) | ![cpp](../examples/filter_selection/ios.png) |
| Flex Layout | ![cs](csharp_ios/flex_layout.png) | ![cpp](../examples/flex_layout/ios.png) |
| Focus | ![cs](csharp_ios/focus.png) | ![cpp](../examples/focus/ios.png) |
| Fonts | ![cs](csharp_ios/fonts.png) | ![cpp](../examples/fonts/ios.png) |
| Footer Only String | ![cs](csharp_ios/footer_only_string.png) | ![cpp](../examples/footer_only_string/ios.png) |
| Formatted Text | ![cs](csharp_ios/formatted_text.png) | ![cpp](../examples/formatted_text/ios.png) |
| Gestures | ![cs](csharp_ios/gestures.png) | ![cpp](../examples/gestures/ios.png) |
| Gradient brushes | ![cs](csharp_ios/gradient.png) | ![cpp](../examples/brushes/ios.png) |
| Grid | ![cs](csharp_ios/grid.png) | ![cpp](../examples/grid/ios.png) |
| Grid Grouping | ![cs](csharp_ios/grid_grouping.png) | ![cpp](../examples/grid_grouping/ios.png) |
| Grouping No Templates | ![cs](csharp_ios/grouping_no_templates.png) | ![cpp](../examples/grouping_no_templates/ios.png) |
| Grouping Plus Selection | ![cs](csharp_ios/grouping_plus_selection.png) | ![cpp](../examples/grouping_plus_selection/ios.png) |
| Header Footer | ![cs](csharp_ios/header_footer.png) | ![cpp](../examples/header_footer/ios.png) |
| Header Footer Grid | ![cs](csharp_ios/header_footer_grid.png) | ![cpp](../examples/header_footer_grid/ios.png) |
| Header Footer Grid Horizontal | ![cs](csharp_ios/header_footer_grid_horizontal.png) | ![cpp](../examples/header_footer_grid_horizontal/ios.png) |
| Header Footer Template | ![cs](csharp_ios/header_footer_template.png) | ![cpp](../examples/header_footer_template/ios.png) |
| Header Footer View | ![cs](csharp_ios/header_footer_view.png) | ![cpp](../examples/header_footer_view/ios.png) |
| Hit Testing | ![cs](csharp_ios/hit_testing.png) | ![cpp](../examples/hit_testing/ios.png) |
| Horizontal Stack | ![cs](csharp_ios/horizontal_stack.png) | ![cpp](../examples/horizontal_stack/ios.png) |
| Hybrid Web View | ![cs](csharp_ios/hybrid_web_view.png) | ![cpp](../examples/hybrid_web_view/ios.png) |
| Image | ![cs](csharp_ios/image.png) | ![cpp](../examples/image/ios.png) |
| Image Button | ![cs](csharp_ios/image_button.png) | ![cpp](../examples/image_button/ios.png) |
| Indicator | ![cs](csharp_ios/indicator.png) | ![cpp](../examples/indicator/ios.png) |
| Input Controls | ![cs](csharp_ios/input_controls.png) | ![cpp](../examples/input_controls/ios.png) |
| Input Transparent | ![cs](csharp_ios/input_transparent.png) | ![cpp](../examples/input_transparent/ios.png) |
| Invalidate Brush | ![cs](csharp_ios/invalidate_brush.png) | ![cpp](../examples/invalidate_brush/ios.png) |
| Invalidate Shadow Host | ![cs](csharp_ios/invalidate_shadow_host.png) | ![cpp](../examples/invalidate_shadow_host/ios.png) |
| Ios Blur Effect | ![cs](csharp_ios/ios_blur_effect.png) | ![cpp](../examples/ios_blur_effect/ios.png) |
| Ios Date Picker | ![cs](csharp_ios/ios_date_picker.png) | ![cpp](../examples/ios_date_picker/ios.png) |
| Ios Entry | ![cs](csharp_ios/ios_entry.png) | ![cpp](../examples/ios_entry/ios.png) |
| Ios First Responder | ![cs](csharp_ios/ios_first_responder.png) | ![cpp](../examples/ios_first_responder/ios.png) |
| Ios Pan Gesture | ![cs](csharp_ios/ios_pan_gesture.png) | ![cpp](../examples/ios_pan_gesture/ios.png) |
| Ios Picker | ![cs](csharp_ios/ios_picker.png) | ![cpp](../examples/ios_picker/ios.png) |
| Ios Safe Area | ![cs](csharp_ios/ios_safe_area.png) | ![cpp](../examples/ios_safe_area/ios.png) |
| Ios Scroll View | ![cs](csharp_ios/ios_scroll_view.png) | ![cpp](../examples/ios_scroll_view/ios.png) |
| Ios Search Bar | ![cs](csharp_ios/ios_search_bar.png) | ![cpp](../examples/ios_search_bar/ios.png) |
| Ios Slider Update On Tap | ![cs](csharp_ios/ios_slider_update_on_tap.png) | ![cpp](../examples/ios_slider_update_on_tap/ios.png) |
| Ios Swipe Transition | ![cs](csharp_ios/ios_swipe_transition.png) | ![cpp](../examples/ios_swipe_transition/ios.png) |
| Ios Time Picker | ![cs](csharp_ios/ios_time_picker.png) | ![cpp](../examples/ios_time_picker/ios.png) |
| Items | ![cs](csharp_ios/items.png) | ![cpp](../examples/items/ios.png) |
| Items Updating Scroll Mode | ![cs](csharp_ios/items_updating_scroll_mode.png) | ![cpp](../examples/items_updating_scroll_mode/ios.png) |
| Label | ![cs](csharp_ios/label.png) | ![cpp](../examples/label/ios.png) |
| Layout Is Enabled | ![cs](csharp_ios/layout_is_enabled.png) | ![cpp](../examples/layout_is_enabled/ios.png) |
| Line Gallery | ![cs](csharp_ios/line_gallery.png) | ![cpp](../examples/line_gallery/ios.png) |
| Line Join Gallery | ![cs](csharp_ios/line_join_gallery.png) | ![cpp](../examples/line_join_gallery/ios.png) |
| Measure First Strategy | ![cs](csharp_ios/measure_first_strategy.png) | ![cpp](../examples/measure_first_strategy/ios.png) |
| Menu Bar | ![cs](csharp_ios/menu_bar.png) | ![cpp](../examples/menu_bar/ios.png) |
| Modal | ![cs](csharp_ios/modal.png) | ![cpp](../examples/modal/ios.png) |
| Multiple Bound Selection | ![cs](csharp_ios/multiple_bound_selection.png) | ![cpp](../examples/multiple_bound_selection/ios.png) |
| Navigation Gallery | ![cs](csharp_ios/navigation_gallery.png) | ![cpp](../examples/navigation_gallery/ios.png) |
| Nested Collection | ![cs](csharp_ios/nested_collection.png) | ![cpp](../examples/nested_collection/ios.png) |
| Pan Gesture Events | ![cs](csharp_ios/pan_gesture_events.png) | ![cpp](../examples/pan_gesture_events/ios.png) |
| Path Aspect Gallery | ![cs](csharp_ios/path_aspect_gallery.png) | ![cpp](../examples/path_aspect_gallery/ios.png) |
| Path Gallery | ![cs](csharp_ios/path_gallery.png) | ![cpp](../examples/path_gallery/ios.png) |
| Path Transform String | ![cs](csharp_ios/path_transform_string.png) | ![cpp](../examples/path_transform_string/ios.png) |
| Picker | ![cs](csharp_ios/picker.png) | ![cpp](../examples/picker/ios.png) |
| Pickers | ![cs](csharp_ios/pickers.png) | ![cpp](../examples/pickers/ios.png) |
| Pointer Gesture | ![cs](csharp_ios/pointer_gesture.png) | ![cpp](../examples/pointer_gesture/ios.png) |
| Polygon Gallery | ![cs](csharp_ios/polygon_gallery.png) | ![cpp](../examples/polygon_gallery/ios.png) |
| Polyline Gallery | ![cs](csharp_ios/polyline_gallery.png) | ![cpp](../examples/polyline_gallery/ios.png) |
| Preselected Item | ![cs](csharp_ios/preselected_item.png) | ![cpp](../examples/preselected_item/ios.png) |
| Preselected Items | ![cs](csharp_ios/preselected_items.png) | ![cpp](../examples/preselected_items/ios.png) |
| Progress Bar | ![cs](csharp_ios/progress_bar.png) | ![cpp](../examples/progress_bar/ios.png) |
| Radio Button Border | ![cs](csharp_ios/radio_button_border.png) | ![cpp](../examples/radio_button_border/ios.png) |
| Radio Button Content | ![cs](csharp_ios/radio_button_content.png) | ![cpp](../examples/radio_button_content/ios.png) |
| Radio Button Group | ![cs](csharp_ios/radio_button_group.png) | ![cpp](../examples/radio_button_group/ios.png) |
| Radio Button Group Binding | ![cs](csharp_ios/radio_button_group_binding.png) | ![cpp](../examples/radio_button_group_binding/ios.png) |
| Radio Button Group Gallery | ![cs](csharp_ios/radio_button_group_gallery.png) | ![cpp](../examples/radio_button_group_gallery/ios.png) |
| Radio Content Properties | ![cs](csharp_ios/radio_content_properties.png) | ![cpp](../examples/radio_content_properties/ios.png) |
| Radio Template From Style | ![cs](csharp_ios/radio_template_from_style.png) | ![cpp](../examples/radio_template_from_style/ios.png) |
| Rectangle Gallery | ![cs](csharp_ios/rectangle_gallery.png) | ![cpp](../examples/rectangle_gallery/ios.png) |
| Refresh View | ![cs](csharp_ios/refresh_view.png) | ![cpp](../examples/refresh_view/ios.png) |
| Relative Layout | ![cs](csharp_ios/relative_layout.png) | ![cpp](../examples/relative_layout/ios.png) |
| Scattered Radio Button | ![cs](csharp_ios/scattered_radio_button.png) | ![cpp](../examples/scattered_radio_button/ios.png) |
| Scroll Mode Test | ![cs](csharp_ios/scroll_mode_test.png) | ![cpp](../examples/scroll_mode_test/ios.png) |
| Scroll To Group | ![cs](csharp_ios/scroll_to_group.png) | ![cpp](../examples/scroll_to_group/ios.png) |
| Scroll View | ![cs](csharp_ios/scroll_view.png) | ![cpp](../examples/scroll_view/ios.png) |
| Search Bar | ![cs](csharp_ios/search_bar.png) | ![cpp](../examples/search_bar/ios.png) |
| Selection Command Param | ![cs](csharp_ios/selection_command_param.png) | ![cpp](../examples/selection_command_param/ios.png) |
| Selection Synchronization | ![cs](csharp_ios/selection_synchronization.png) | ![cpp](../examples/selection_synchronization/ios.png) |
| Semantics | ![cs](csharp_ios/semantics.png) | ![cpp](../examples/semantics/ios.png) |
| Shadow Playground | ![cs](csharp_ios/shadow_playground.png) | ![cpp](../examples/shadow_playground/ios.png) |
| Shape App Theme | ![cs](csharp_ios/shape_app_theme.png) | ![cpp](../examples/shape_app_theme/ios.png) |
| Shapes | ![cs](csharp_ios/shapes.png) | ![cpp](../examples/shapes_demo/ios.png) |
| Single Bound Selection | ![cs](csharp_ios/single_bound_selection.png) | ![cpp](../examples/single_bound_selection/ios.png) |
| Slider | ![cs](csharp_ios/slider.png) | ![cpp](../examples/slider/ios.png) |
| Some Empty Groups | ![cs](csharp_ios/some_empty_groups.png) | ![cpp](../examples/some_empty_groups/ios.png) |
| Stack Layout | ![cs](csharp_ios/stack_layout.png) | ![cpp](../examples/stack_layout/ios.png) |
| Staggered Layout | ![cs](csharp_ios/staggered_layout.png) | ![cpp](../examples/staggered_layout/ios.png) |
| Stepper | ![cs](csharp_ios/stepper.png) | ![cpp](../examples/stepper/ios.png) |
| Styles | ![cs](csharp_ios/styles.png) | ![cpp](../examples/styles/ios.png) |
| Swipe Gesture | ![cs](csharp_ios/swipe_gesture.png) | ![cpp](../examples/swipe_gesture/ios.png) |
| Swipe Item Position | ![cs](csharp_ios/swipe_item_position.png) | ![cpp](../examples/swipe_item_position/ios.png) |
| Swipe Item Size | ![cs](csharp_ios/swipe_item_size.png) | ![cpp](../examples/swipe_item_size/ios.png) |
| Swipe Refresh | ![cs](csharp_ios/swipe_refresh.png) | ![cpp](../examples/swipe_refresh/ios.png) |
| Swipe Threshold | ![cs](csharp_ios/swipe_threshold.png) | ![cpp](../examples/swipe_threshold/ios.png) |
| Swipe View Margin | ![cs](csharp_ios/swipe_view_margin.png) | ![cpp](../examples/swipe_view_margin/ios.png) |
| Swipe View Shadow | ![cs](csharp_ios/swipe_view_shadow.png) | ![cpp](../examples/swipe_view_shadow/ios.png) |
| Switch | ![cs](csharp_ios/switch.png) | ![cpp](../examples/switch/ios.png) |
| Switch Grouping | ![cs](csharp_ios/switch_grouping.png) | ![cpp](../examples/switch_grouping/ios.png) |
| Tabbed Flyout | ![cs](csharp_ios/tabbed_flyout.png) | ![cpp](../examples/tabbed_flyout/ios.png) |
| Templated View | ![cs](csharp_ios/templated_view.png) | ![cpp](../examples/templated_view/ios.png) |
| Time Picker | ![cs](csharp_ios/time_picker.png) | ![cpp](../examples/time_picker/ios.png) |
| Title Bar | ![cs](csharp_ios/title_bar.png) | ![cpp](../examples/title_bar/ios.png) |
| Toolbar | ![cs](csharp_ios/toolbar.png) | ![cpp](../examples/toolbar/ios.png) |
| Transform Playground | ![cs](csharp_ios/transform_playground.png) | ![cpp](../examples/transform_playground/ios.png) |
| Transformations | ![cs](csharp_ios/transformations.png) | ![cpp](../examples/transformations/ios.png) |
| Triggers | ![cs](csharp_ios/triggers.png) | ![cpp](../examples/triggers/ios.png) |
| Update Path Data | ![cs](csharp_ios/update_path_data.png) | ![cpp](../examples/update_path_data/ios.png) |
| Varied Size Selector | ![cs](csharp_ios/varied_size_selector.png) | ![cpp](../examples/varied_size_selector/ios.png) |
| Vertical Stack | ![cs](csharp_ios/vertical_stack.png) | ![cpp](../examples/vertical_stack/ios.png) |
| Visual States | ![cs](csharp_ios/visual_states.png) | ![cpp](../examples/visual_states/ios.png) |
| Web View | ![cs](csharp_ios/web_view.png) | ![cpp](../examples/web_view/ios.png) |
| Z Index | ![cs](csharp_ios/z_index.png) | ![cpp](../examples/z_index/ios.png) |

### macOS / Mac Catalyst

| Page | .NET MAUI (Mac Catalyst) | C++ port (macOS) |
| --- | --- | --- |
| Absolute Layout | ![cs](csharp_maccatalyst/absolute_layout.png) | ![cpp](../examples/absolute_layout/macos.png) |
| Activity Indicator | ![cs](csharp_maccatalyst/activity_indicator.png) | ![cpp](../examples/activity_indicator/macos.png) |
| Adaptive Collection | ![cs](csharp_maccatalyst/adaptive_collection.png) | ![cpp](../examples/adaptive_collection/macos.png) |
| Alerts | ![cs](csharp_maccatalyst/alerts.png) | ![cpp](../examples/alerts/macos.png) |
| Layout alignment (Start/Center/End/Fill) | ![cs](csharp_maccatalyst/alignment.png) | ![cpp](../examples/border_alignment/macos.png) |
| Animation | ![cs](csharp_maccatalyst/animation.png) | ![cpp](../examples/animation/macos.png) |
| App Theme Binding | ![cs](csharp_maccatalyst/app_theme_binding.png) | ![cpp](../examples/app_theme_binding/macos.png) |
| Application Control | ![cs](csharp_maccatalyst/application_control.png) | ![cpp](../examples/application_control/macos.png) |
| Auto Size Shapes | ![cs](csharp_maccatalyst/auto_size_shapes.png) | ![cpp](../examples/auto_size_shapes/macos.png) |
| Basic Grouping | ![cs](csharp_maccatalyst/basic_grouping.png) | ![cpp](../examples/basic_grouping/macos.png) |
| Basic Swipe | ![cs](csharp_maccatalyst/basic_swipe.png) | ![cpp](../examples/basic_swipe/macos.png) |
| Behaviors | ![cs](csharp_maccatalyst/behaviors.png) | ![cpp](../examples/behaviors/macos.png) |
| Border | ![cs](csharp_maccatalyst/border.png) | ![cpp](../examples/border_styles/macos.png) |
| Border Clip Playground | ![cs](csharp_maccatalyst/border_clip_playground.png) | ![cpp](../examples/border_clip_playground/macos.png) |
| Border Layout | ![cs](csharp_maccatalyst/border_layout.png) | ![cpp](../examples/border_layout/macos.png) |
| Border Playground | ![cs](csharp_maccatalyst/border_playground.png) | ![cpp](../examples/border_playground/macos.png) |
| Border Resize Content | ![cs](csharp_maccatalyst/border_resize_content.png) | ![cpp](../examples/border_resize_content/macos.png) |
| Border Stroke | ![cs](csharp_maccatalyst/border_stroke.png) | ![cpp](../examples/border_stroke/macos.png) |
| Borderless | ![cs](csharp_maccatalyst/borderless.png) | ![cpp](../examples/borderless/macos.png) |
| Box View | ![cs](csharp_maccatalyst/box_view.png) | ![cpp](../examples/box_view/macos.png) |
| Button | ![cs](csharp_maccatalyst/button.png) | ![cpp](../examples/button/macos.png) |
| Carousel Page | ![cs](csharp_maccatalyst/carousel_page.png) | ![cpp](../examples/carousel_page/macos.png) |
| Chat Example | ![cs](csharp_maccatalyst/chat_example.png) | ![cpp](../examples/chat_example/macos.png) |
| Check Box | ![cs](csharp_maccatalyst/check_box.png) | ![cpp](../examples/check_box/macos.png) |
| Chrome | ![cs](csharp_maccatalyst/chrome.png) | ![cpp](../examples/chrome/macos.png) |
| Clip | ![cs](csharp_maccatalyst/clip.png) | ![cpp](../examples/clip/macos.png) |
| Clip Corner Radius | ![cs](csharp_maccatalyst/clip_corner_radius.png) | ![cpp](../examples/clip_corner_radius/macos.png) |
| Clip Gallery | ![cs](csharp_maccatalyst/clip_gallery.png) | ![cpp](../examples/clip_gallery/macos.png) |
| Clip Views | ![cs](csharp_maccatalyst/clip_views.png) | ![cpp](../examples/clip_views/macos.png) |
| Clipping | ![cs](csharp_maccatalyst/clipping.png) | ![cpp](../examples/clipping/macos.png) |
| CollectionView | ![cs](csharp_maccatalyst/collectionview.png) | ![cpp](../examples/selection_mode/macos.png) |
| Composition Gallery | ![cs](csharp_maccatalyst/composition_gallery.png) | ![cpp](../examples/composition_gallery/macos.png) |
| Containers | ![cs](csharp_maccatalyst/containers.png) | ![cpp](../examples/containers/macos.png) |
| Content View | ![cs](csharp_maccatalyst/content_view.png) | ![cpp](../examples/content_view/macos.png) |
| Context Flyout | ![cs](csharp_maccatalyst/context_flyout.png) | ![cpp](../examples/context_flyout/macos.png) |
| Control stack | ![cs](csharp_maccatalyst/controls_stack.png) | ![cpp](../examples/value_controls/macos.png) |
| Custom Layout | ![cs](csharp_maccatalyst/custom_layout.png) | ![cpp](../examples/custom_layout/macos.png) |
| Custom Size Swipe | ![cs](csharp_maccatalyst/custom_size_swipe.png) | ![cpp](../examples/custom_size_swipe/macos.png) |
| Custom Swipe Item View | ![cs](csharp_maccatalyst/custom_swipe_item_view.png) | ![cpp](../examples/custom_swipe_item_view/macos.png) |
| Cv Visual States | ![cs](csharp_maccatalyst/cv_visual_states.png) | ![cpp](../examples/cv_visual_states/macos.png) |
| Data Template Selector | ![cs](csharp_maccatalyst/data_template_selector.png) | ![cpp](../examples/data_template_selector/macos.png) |
| Date Picker | ![cs](csharp_maccatalyst/date_picker.png) | ![cpp](../examples/date_picker/macos.png) |
| Device | ![cs](csharp_maccatalyst/device.png) | ![cpp](../examples/device/macos.png) |
| Dispatcher | ![cs](csharp_maccatalyst/dispatcher.png) | ![cpp](../examples/dispatcher/macos.png) |
| Drag Drop | ![cs](csharp_maccatalyst/drag_drop.png) | ![cpp](../examples/drag_drop/macos.png) |
| Editor | ![cs](csharp_maccatalyst/editor.png) | ![cpp](../examples/editor/macos.png) |
| Effects | — | ![cpp](../examples/effects/macos.png) |
| Ellipse Gallery | ![cs](csharp_maccatalyst/ellipse_gallery.png) | ![cpp](../examples/ellipse_gallery/macos.png) |
| Empty View | ![cs](csharp_maccatalyst/empty_view.png) | ![cpp](../examples/empty_view/macos.png) |
| Empty View Load Simulate | ![cs](csharp_maccatalyst/empty_view_load_simulate.png) | ![cpp](../examples/empty_view_load_simulate/macos.png) |
| Empty View Null | ![cs](csharp_maccatalyst/empty_view_null.png) | ![cpp](../examples/empty_view_null/macos.png) |
| Empty View Rtl | ![cs](csharp_maccatalyst/empty_view_rtl.png) | ![cpp](../examples/empty_view_rtl/macos.png) |
| Empty View Selector | ![cs](csharp_maccatalyst/empty_view_selector.png) | ![cpp](../examples/empty_view_selector/macos.png) |
| Empty View Swap | ![cs](csharp_maccatalyst/empty_view_swap.png) | ![cpp](../examples/empty_view_swap/macos.png) |
| Empty View Template | ![cs](csharp_maccatalyst/empty_view_template.png) | ![cpp](../examples/empty_view_template/macos.png) |
| Empty View View | ![cs](csharp_maccatalyst/empty_view_view.png) | ![cpp](../examples/empty_view_view/macos.png) |
| Entry | ![cs](csharp_maccatalyst/entry.png) | ![cpp](../examples/entry/macos.png) |
| Filter Collection | ![cs](csharp_maccatalyst/filter_collection.png) | ![cpp](../examples/filter_collection/macos.png) |
| Filter Selection | ![cs](csharp_maccatalyst/filter_selection.png) | ![cpp](../examples/filter_selection/macos.png) |
| Flex Layout | ![cs](csharp_maccatalyst/flex_layout.png) | ![cpp](../examples/flex_layout/macos.png) |
| Focus | ![cs](csharp_maccatalyst/focus.png) | ![cpp](../examples/focus/macos.png) |
| Fonts | ![cs](csharp_maccatalyst/fonts.png) | ![cpp](../examples/fonts/macos.png) |
| Footer Only String | ![cs](csharp_maccatalyst/footer_only_string.png) | ![cpp](../examples/footer_only_string/macos.png) |
| Formatted Text | ![cs](csharp_maccatalyst/formatted_text.png) | ![cpp](../examples/formatted_text/macos.png) |
| Gestures | ![cs](csharp_maccatalyst/gestures.png) | ![cpp](../examples/gestures/macos.png) |
| Gradient brushes | ![cs](csharp_maccatalyst/gradient.png) | ![cpp](../examples/brushes/macos.png) |
| Grid | ![cs](csharp_maccatalyst/grid.png) | ![cpp](../examples/grid/macos.png) |
| Grid Grouping | ![cs](csharp_maccatalyst/grid_grouping.png) | ![cpp](../examples/grid_grouping/macos.png) |
| Grouping No Templates | ![cs](csharp_maccatalyst/grouping_no_templates.png) | ![cpp](../examples/grouping_no_templates/macos.png) |
| Grouping Plus Selection | ![cs](csharp_maccatalyst/grouping_plus_selection.png) | ![cpp](../examples/grouping_plus_selection/macos.png) |
| Header Footer | ![cs](csharp_maccatalyst/header_footer.png) | ![cpp](../examples/header_footer/macos.png) |
| Header Footer Grid | ![cs](csharp_maccatalyst/header_footer_grid.png) | ![cpp](../examples/header_footer_grid/macos.png) |
| Header Footer Grid Horizontal | ![cs](csharp_maccatalyst/header_footer_grid_horizontal.png) | ![cpp](../examples/header_footer_grid_horizontal/macos.png) |
| Header Footer Template | ![cs](csharp_maccatalyst/header_footer_template.png) | ![cpp](../examples/header_footer_template/macos.png) |
| Header Footer View | ![cs](csharp_maccatalyst/header_footer_view.png) | ![cpp](../examples/header_footer_view/macos.png) |
| Hit Testing | ![cs](csharp_maccatalyst/hit_testing.png) | ![cpp](../examples/hit_testing/macos.png) |
| Horizontal Stack | ![cs](csharp_maccatalyst/horizontal_stack.png) | ![cpp](../examples/horizontal_stack/macos.png) |
| Hybrid Web View | ![cs](csharp_maccatalyst/hybrid_web_view.png) | ![cpp](../examples/hybrid_web_view/macos.png) |
| Image | ![cs](csharp_maccatalyst/image.png) | ![cpp](../examples/image/macos.png) |
| Image Button | ![cs](csharp_maccatalyst/image_button.png) | ![cpp](../examples/image_button/macos.png) |
| Indicator | ![cs](csharp_maccatalyst/indicator.png) | ![cpp](../examples/indicator/macos.png) |
| Input Controls | ![cs](csharp_maccatalyst/input_controls.png) | ![cpp](../examples/input_controls/macos.png) |
| Input Transparent | ![cs](csharp_maccatalyst/input_transparent.png) | ![cpp](../examples/input_transparent/macos.png) |
| Invalidate Brush | ![cs](csharp_maccatalyst/invalidate_brush.png) | ![cpp](../examples/invalidate_brush/macos.png) |
| Invalidate Shadow Host | ![cs](csharp_maccatalyst/invalidate_shadow_host.png) | ![cpp](../examples/invalidate_shadow_host/macos.png) |
| Ios Blur Effect | ![cs](csharp_maccatalyst/ios_blur_effect.png) | ![cpp](../examples/ios_blur_effect/macos.png) |
| Ios Date Picker | ![cs](csharp_maccatalyst/ios_date_picker.png) | ![cpp](../examples/ios_date_picker/macos.png) |
| Ios Entry | ![cs](csharp_maccatalyst/ios_entry.png) | ![cpp](../examples/ios_entry/macos.png) |
| Ios First Responder | ![cs](csharp_maccatalyst/ios_first_responder.png) | ![cpp](../examples/ios_first_responder/macos.png) |
| Ios Pan Gesture | ![cs](csharp_maccatalyst/ios_pan_gesture.png) | ![cpp](../examples/ios_pan_gesture/macos.png) |
| Ios Picker | ![cs](csharp_maccatalyst/ios_picker.png) | ![cpp](../examples/ios_picker/macos.png) |
| Ios Safe Area | ![cs](csharp_maccatalyst/ios_safe_area.png) | ![cpp](../examples/ios_safe_area/macos.png) |
| Ios Scroll View | ![cs](csharp_maccatalyst/ios_scroll_view.png) | ![cpp](../examples/ios_scroll_view/macos.png) |
| Ios Search Bar | ![cs](csharp_maccatalyst/ios_search_bar.png) | ![cpp](../examples/ios_search_bar/macos.png) |
| Ios Slider Update On Tap | ![cs](csharp_maccatalyst/ios_slider_update_on_tap.png) | ![cpp](../examples/ios_slider_update_on_tap/macos.png) |
| Ios Swipe Transition | ![cs](csharp_maccatalyst/ios_swipe_transition.png) | ![cpp](../examples/ios_swipe_transition/macos.png) |
| Ios Time Picker | ![cs](csharp_maccatalyst/ios_time_picker.png) | ![cpp](../examples/ios_time_picker/macos.png) |
| Items | ![cs](csharp_maccatalyst/items.png) | ![cpp](../examples/items/macos.png) |
| Items Updating Scroll Mode | ![cs](csharp_maccatalyst/items_updating_scroll_mode.png) | ![cpp](../examples/items_updating_scroll_mode/macos.png) |
| Label | ![cs](csharp_maccatalyst/label.png) | ![cpp](../examples/label/macos.png) |
| Layout Is Enabled | ![cs](csharp_maccatalyst/layout_is_enabled.png) | ![cpp](../examples/layout_is_enabled/macos.png) |
| Line Gallery | ![cs](csharp_maccatalyst/line_gallery.png) | ![cpp](../examples/line_gallery/macos.png) |
| Line Join Gallery | ![cs](csharp_maccatalyst/line_join_gallery.png) | ![cpp](../examples/line_join_gallery/macos.png) |
| Measure First Strategy | ![cs](csharp_maccatalyst/measure_first_strategy.png) | ![cpp](../examples/measure_first_strategy/macos.png) |
| Menu Bar | ![cs](csharp_maccatalyst/menu_bar.png) | ![cpp](../examples/menu_bar/macos.png) |
| Modal | ![cs](csharp_maccatalyst/modal.png) | ![cpp](../examples/modal/macos.png) |
| Multiple Bound Selection | ![cs](csharp_maccatalyst/multiple_bound_selection.png) | ![cpp](../examples/multiple_bound_selection/macos.png) |
| Navigation Gallery | ![cs](csharp_maccatalyst/navigation_gallery.png) | ![cpp](../examples/navigation_gallery/macos.png) |
| Nested Collection | ![cs](csharp_maccatalyst/nested_collection.png) | ![cpp](../examples/nested_collection/macos.png) |
| Pan Gesture Events | ![cs](csharp_maccatalyst/pan_gesture_events.png) | ![cpp](../examples/pan_gesture_events/macos.png) |
| Path Aspect Gallery | ![cs](csharp_maccatalyst/path_aspect_gallery.png) | ![cpp](../examples/path_aspect_gallery/macos.png) |
| Path Gallery | ![cs](csharp_maccatalyst/path_gallery.png) | ![cpp](../examples/path_gallery/macos.png) |
| Path Transform String | ![cs](csharp_maccatalyst/path_transform_string.png) | ![cpp](../examples/path_transform_string/macos.png) |
| Picker | ![cs](csharp_maccatalyst/picker.png) | ![cpp](../examples/picker/macos.png) |
| Pickers | ![cs](csharp_maccatalyst/pickers.png) | ![cpp](../examples/pickers/macos.png) |
| Pointer Gesture | ![cs](csharp_maccatalyst/pointer_gesture.png) | ![cpp](../examples/pointer_gesture/macos.png) |
| Polygon Gallery | ![cs](csharp_maccatalyst/polygon_gallery.png) | ![cpp](../examples/polygon_gallery/macos.png) |
| Polyline Gallery | ![cs](csharp_maccatalyst/polyline_gallery.png) | ![cpp](../examples/polyline_gallery/macos.png) |
| Preselected Item | ![cs](csharp_maccatalyst/preselected_item.png) | ![cpp](../examples/preselected_item/macos.png) |
| Preselected Items | ![cs](csharp_maccatalyst/preselected_items.png) | ![cpp](../examples/preselected_items/macos.png) |
| Progress Bar | ![cs](csharp_maccatalyst/progress_bar.png) | ![cpp](../examples/progress_bar/macos.png) |
| Radio Button Border | ![cs](csharp_maccatalyst/radio_button_border.png) | ![cpp](../examples/radio_button_border/macos.png) |
| Radio Button Content | ![cs](csharp_maccatalyst/radio_button_content.png) | ![cpp](../examples/radio_button_content/macos.png) |
| Radio Button Group | ![cs](csharp_maccatalyst/radio_button_group.png) | ![cpp](../examples/radio_button_group/macos.png) |
| Radio Button Group Binding | ![cs](csharp_maccatalyst/radio_button_group_binding.png) | ![cpp](../examples/radio_button_group_binding/macos.png) |
| Radio Button Group Gallery | ![cs](csharp_maccatalyst/radio_button_group_gallery.png) | ![cpp](../examples/radio_button_group_gallery/macos.png) |
| Radio Content Properties | ![cs](csharp_maccatalyst/radio_content_properties.png) | ![cpp](../examples/radio_content_properties/macos.png) |
| Radio Template From Style | — | ![cpp](../examples/radio_template_from_style/macos.png) |
| Rectangle Gallery | ![cs](csharp_maccatalyst/rectangle_gallery.png) | ![cpp](../examples/rectangle_gallery/macos.png) |
| Refresh View | ![cs](csharp_maccatalyst/refresh_view.png) | ![cpp](../examples/refresh_view/macos.png) |
| Relative Layout | ![cs](csharp_maccatalyst/relative_layout.png) | ![cpp](../examples/relative_layout/macos.png) |
| Scattered Radio Button | ![cs](csharp_maccatalyst/scattered_radio_button.png) | ![cpp](../examples/scattered_radio_button/macos.png) |
| Scroll Mode Test | ![cs](csharp_maccatalyst/scroll_mode_test.png) | ![cpp](../examples/scroll_mode_test/macos.png) |
| Scroll To Group | ![cs](csharp_maccatalyst/scroll_to_group.png) | ![cpp](../examples/scroll_to_group/macos.png) |
| Scroll View | ![cs](csharp_maccatalyst/scroll_view.png) | ![cpp](../examples/scroll_view/macos.png) |
| Search Bar | ![cs](csharp_maccatalyst/search_bar.png) | ![cpp](../examples/search_bar/macos.png) |
| Selection Command Param | ![cs](csharp_maccatalyst/selection_command_param.png) | ![cpp](../examples/selection_command_param/macos.png) |
| Selection Synchronization | ![cs](csharp_maccatalyst/selection_synchronization.png) | ![cpp](../examples/selection_synchronization/macos.png) |
| Semantics | ![cs](csharp_maccatalyst/semantics.png) | ![cpp](../examples/semantics/macos.png) |
| Shadow Playground | ![cs](csharp_maccatalyst/shadow_playground.png) | ![cpp](../examples/shadow_playground/macos.png) |
| Shape App Theme | ![cs](csharp_maccatalyst/shape_app_theme.png) | ![cpp](../examples/shape_app_theme/macos.png) |
| Shapes | ![cs](csharp_maccatalyst/shapes.png) | ![cpp](../examples/shapes_demo/macos.png) |
| Single Bound Selection | ![cs](csharp_maccatalyst/single_bound_selection.png) | ![cpp](../examples/single_bound_selection/macos.png) |
| Slider | ![cs](csharp_maccatalyst/slider.png) | ![cpp](../examples/slider/macos.png) |
| Some Empty Groups | ![cs](csharp_maccatalyst/some_empty_groups.png) | ![cpp](../examples/some_empty_groups/macos.png) |
| Stack Layout | ![cs](csharp_maccatalyst/stack_layout.png) | ![cpp](../examples/stack_layout/macos.png) |
| Staggered Layout | ![cs](csharp_maccatalyst/staggered_layout.png) | ![cpp](../examples/staggered_layout/macos.png) |
| Stepper | ![cs](csharp_maccatalyst/stepper.png) | ![cpp](../examples/stepper/macos.png) |
| Styles | ![cs](csharp_maccatalyst/styles.png) | ![cpp](../examples/styles/macos.png) |
| Swipe Gesture | ![cs](csharp_maccatalyst/swipe_gesture.png) | ![cpp](../examples/swipe_gesture/macos.png) |
| Swipe Item Position | ![cs](csharp_maccatalyst/swipe_item_position.png) | ![cpp](../examples/swipe_item_position/macos.png) |
| Swipe Item Size | ![cs](csharp_maccatalyst/swipe_item_size.png) | ![cpp](../examples/swipe_item_size/macos.png) |
| Swipe Refresh | ![cs](csharp_maccatalyst/swipe_refresh.png) | ![cpp](../examples/swipe_refresh/macos.png) |
| Swipe Threshold | ![cs](csharp_maccatalyst/swipe_threshold.png) | ![cpp](../examples/swipe_threshold/macos.png) |
| Swipe View Margin | ![cs](csharp_maccatalyst/swipe_view_margin.png) | ![cpp](../examples/swipe_view_margin/macos.png) |
| Swipe View Shadow | ![cs](csharp_maccatalyst/swipe_view_shadow.png) | ![cpp](../examples/swipe_view_shadow/macos.png) |
| Switch | ![cs](csharp_maccatalyst/switch.png) | ![cpp](../examples/switch/macos.png) |
| Switch Grouping | ![cs](csharp_maccatalyst/switch_grouping.png) | ![cpp](../examples/switch_grouping/macos.png) |
| Tabbed Flyout | ![cs](csharp_maccatalyst/tabbed_flyout.png) | ![cpp](../examples/tabbed_flyout/macos.png) |
| Templated View | ![cs](csharp_maccatalyst/templated_view.png) | ![cpp](../examples/templated_view/macos.png) |
| Time Picker | ![cs](csharp_maccatalyst/time_picker.png) | ![cpp](../examples/time_picker/macos.png) |
| Title Bar | ![cs](csharp_maccatalyst/title_bar.png) | ![cpp](../examples/title_bar/macos.png) |
| Toolbar | ![cs](csharp_maccatalyst/toolbar.png) | ![cpp](../examples/toolbar/macos.png) |
| Transform Playground | ![cs](csharp_maccatalyst/transform_playground.png) | ![cpp](../examples/transform_playground/macos.png) |
| Transformations | ![cs](csharp_maccatalyst/transformations.png) | ![cpp](../examples/transformations/macos.png) |
| Triggers | ![cs](csharp_maccatalyst/triggers.png) | ![cpp](../examples/triggers/macos.png) |
| Update Path Data | ![cs](csharp_maccatalyst/update_path_data.png) | ![cpp](../examples/update_path_data/macos.png) |
| Varied Size Selector | ![cs](csharp_maccatalyst/varied_size_selector.png) | ![cpp](../examples/varied_size_selector/macos.png) |
| Vertical Stack | ![cs](csharp_maccatalyst/vertical_stack.png) | ![cpp](../examples/vertical_stack/macos.png) |
| Visual States | ![cs](csharp_maccatalyst/visual_states.png) | ![cpp](../examples/visual_states/macos.png) |
| Web View | ![cs](csharp_maccatalyst/web_view.png) | ![cpp](../examples/web_view/macos.png) |
| Z Index | ![cs](csharp_maccatalyst/z_index.png) | ![cpp](../examples/z_index/macos.png) |
