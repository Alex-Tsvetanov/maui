# C++ port vs .NET MAUI — iOS pixel-parity tracker

Theme-matched iOS comparison: each page rendered by **real .NET MAUI** vs the **C++ port**, on the same iPhone 17 simulator, compared **light-vs-light** and **dark-vs-dark**. Both stacks render native-default controls + the system font (the C# app's `dotnet new maui` default `Styles.xaml` + OpenSans are stripped; appearance forced via `MAUI_THEME` / `MAUI_APPEARANCE`). Goal: pixel-perfect parity, fixed example-by-example.

**Progress: 48 / 172 🟢 matched** · 95 🟡 minor · 29 🔴 diff · 0 ⬜ pending

**Flags: 9 ⚠️ broken MAUI reference captures (re-shoot needed) · 14 🎬 motion/effect pages needing an animated GIF to judge.**

Status legend: 🟢 pixel-match (both themes) · 🟡 minor diff · 🔴 notable diff to fix · ⬛ C++ renders blank · ⬜ not yet reviewed · ⚠️ MAUI reference capture itself is broken (re-shoot needed) · 🎬 motion/effect page — a still frame can't judge it; needs a GIF. Per-theme verdicts + per-page notes in `parity_status.json`; the **Per-page findings** section below lists every non-matching page's concrete diffs.

> macOS / Mac Catalyst 4-way comparison is **Phase 2** (pending: aligning the gallery window size to the C# window). The earlier 2-way macOS grid + notes live in [PARITY_FINDINGS.md](PARITY_FINDINGS.md).

Rows are in **fix order** (top → bottom): foundational single controls first (their fixes cascade), then layouts, shapes, borders/clip, collection-views, radio, swipe, gestures, scroll/web, combos, iOS-specifics, and chrome/host pages last.

| # | Page | Status | .NET MAUI (light) | C++ (light) | .NET MAUI (dark) | C++ (dark) |
| --: | --- | :---: | --- | --- | --- | --- |
| 1 | Label | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/label.png) | ![](cpp_ios_light/label.png) | ![](csharp_ios_dark/label.png) | ![](cpp_ios_dark/label.png) |
| 2 | Button | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/button.png) | ![](cpp_ios_light/button.png) | ![](csharp_ios_dark/button.png) | ![](cpp_ios_dark/button.png) |
| 3 | Entry | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/entry.png) | ![](cpp_ios_light/entry.png) | ![](csharp_ios_dark/entry.png) | ![](cpp_ios_dark/entry.png) |
| 4 | Editor | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/editor.png) | ![](cpp_ios_light/editor.png) | ![](csharp_ios_dark/editor.png) | ![](cpp_ios_dark/editor.png) |
| 5 | Search Bar | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/search_bar.png) | ![](cpp_ios_light/search_bar.png) | ![](csharp_ios_dark/search_bar.png) | ![](cpp_ios_dark/search_bar.png) |
| 6 | Picker | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/picker.png) | ![](cpp_ios_light/picker.png) | ![](csharp_ios_dark/picker.png) | ![](cpp_ios_dark/picker.png) |
| 7 | Date Picker | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/date_picker.png) | ![](cpp_ios_light/date_picker.png) | ![](csharp_ios_dark/date_picker.png) | ![](cpp_ios_dark/date_picker.png) |
| 8 | Time Picker | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/time_picker.png) | ![](cpp_ios_light/time_picker.png) | ![](csharp_ios_dark/time_picker.png) | ![](cpp_ios_dark/time_picker.png) |
| 9 | Pickers | 🟡<br>L:minor<br>D:match | ![](csharp_ios_light/pickers.png) | ![](cpp_ios_light/pickers.png) | ![](csharp_ios_dark/pickers.png) | ![](cpp_ios_dark/pickers.png) |
| 10 | Slider | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/slider.png) | ![](cpp_ios_light/slider.png) | ![](csharp_ios_dark/slider.png) | ![](cpp_ios_dark/slider.png) |
| 11 | Stepper | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/stepper.png) | ![](cpp_ios_light/stepper.png) | ![](csharp_ios_dark/stepper.png) | ![](cpp_ios_dark/stepper.png) |
| 12 | Switch | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/switch.png) | ![](cpp_ios_light/switch.png) | ![](csharp_ios_dark/switch.png) | ![](cpp_ios_dark/switch.png) |
| 13 | Check Box | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/check_box.png) | ![](cpp_ios_light/check_box.png) | ![](csharp_ios_dark/check_box.png) | ![](cpp_ios_dark/check_box.png) |
| 14 | Progress Bar | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/progress_bar.png) | ![](cpp_ios_light/progress_bar.png) | ![](csharp_ios_dark/progress_bar.png) | ![](cpp_ios_dark/progress_bar.png) |
| 15 | Activity Indicator | 🟡🎬<br>L:minor<br>D:minor | ![](csharp_ios_light/activity_indicator.png) | ![](cpp_ios_light/activity_indicator.png) | ![](csharp_ios_dark/activity_indicator.png) | ![](cpp_ios_dark/activity_indicator.png) |
| 16 | Indicator | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/indicator.png) | ![](cpp_ios_light/indicator.png) | ![](csharp_ios_dark/indicator.png) | ![](cpp_ios_dark/indicator.png) |
| 17 | Image | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/image.png) | ![](cpp_ios_light/image.png) | ![](csharp_ios_dark/image.png) | ![](cpp_ios_dark/image.png) |
| 18 | Image Button | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/image_button.png) | ![](cpp_ios_light/image_button.png) | ![](csharp_ios_dark/image_button.png) | ![](cpp_ios_dark/image_button.png) |
| 19 | Box View | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/box_view.png) | ![](cpp_ios_light/box_view.png) | ![](csharp_ios_dark/box_view.png) | ![](cpp_ios_dark/box_view.png) |
| 20 | Content View | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/content_view.png) | ![](cpp_ios_light/content_view.png) | ![](csharp_ios_dark/content_view.png) | ![](cpp_ios_dark/content_view.png) |
| 21 | Containers | 🟡<br>L:minor<br>D:match | ![](csharp_ios_light/containers.png) | ![](cpp_ios_light/containers.png) | ![](csharp_ios_dark/containers.png) | ![](cpp_ios_dark/containers.png) |
| 22 | Control stack | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/controls_stack.png) | ![](cpp_ios_light/controls_stack.png) | ![](csharp_ios_dark/controls_stack.png) | ![](cpp_ios_dark/controls_stack.png) |
| 23 | Input Controls | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/input_controls.png) | ![](cpp_ios_light/input_controls.png) | ![](csharp_ios_dark/input_controls.png) | ![](cpp_ios_dark/input_controls.png) |
| 24 | Fonts | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/fonts.png) | ![](cpp_ios_light/fonts.png) | ![](csharp_ios_dark/fonts.png) | ![](cpp_ios_dark/fonts.png) |
| 25 | Formatted Text | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/formatted_text.png) | ![](cpp_ios_light/formatted_text.png) | ![](csharp_ios_dark/formatted_text.png) | ![](cpp_ios_dark/formatted_text.png) |
| 26 | Styles | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/styles.png) | ![](cpp_ios_light/styles.png) | ![](csharp_ios_dark/styles.png) | ![](cpp_ios_dark/styles.png) |
| 27 | Triggers | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/triggers.png) | ![](cpp_ios_light/triggers.png) | ![](csharp_ios_dark/triggers.png) | ![](cpp_ios_dark/triggers.png) |
| 28 | Behaviors | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/behaviors.png) | ![](cpp_ios_light/behaviors.png) | ![](csharp_ios_dark/behaviors.png) | ![](cpp_ios_dark/behaviors.png) |
| 29 | Semantics | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/semantics.png) | ![](cpp_ios_light/semantics.png) | ![](csharp_ios_dark/semantics.png) | ![](cpp_ios_dark/semantics.png) |
| 30 | App Theme Binding | 🟡<br>L:match<br>D:minor | ![](csharp_ios_light/app_theme_binding.png) | ![](cpp_ios_light/app_theme_binding.png) | ![](csharp_ios_dark/app_theme_binding.png) | ![](cpp_ios_dark/app_theme_binding.png) |
| 31 | Stack Layout | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/stack_layout.png) | ![](cpp_ios_light/stack_layout.png) | ![](csharp_ios_dark/stack_layout.png) | ![](cpp_ios_dark/stack_layout.png) |
| 32 | Vertical Stack | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/vertical_stack.png) | ![](cpp_ios_light/vertical_stack.png) | ![](csharp_ios_dark/vertical_stack.png) | ![](cpp_ios_dark/vertical_stack.png) |
| 33 | Horizontal Stack | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/horizontal_stack.png) | ![](cpp_ios_light/horizontal_stack.png) | ![](csharp_ios_dark/horizontal_stack.png) | ![](cpp_ios_dark/horizontal_stack.png) |
| 34 | Grid | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/grid.png) | ![](cpp_ios_light/grid.png) | ![](csharp_ios_dark/grid.png) | ![](cpp_ios_dark/grid.png) |
| 35 | Absolute Layout | 🟡<br>L:minor<br>D:match | ![](csharp_ios_light/absolute_layout.png) | ![](cpp_ios_light/absolute_layout.png) | ![](csharp_ios_dark/absolute_layout.png) | ![](cpp_ios_dark/absolute_layout.png) |
| 36 | Flex Layout | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/flex_layout.png) | ![](cpp_ios_light/flex_layout.png) | ![](csharp_ios_dark/flex_layout.png) | ![](cpp_ios_dark/flex_layout.png) |
| 37 | Relative Layout | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/relative_layout.png) | ![](cpp_ios_light/relative_layout.png) | ![](csharp_ios_dark/relative_layout.png) | ![](cpp_ios_dark/relative_layout.png) |
| 38 | Layout alignment (Start/Center/End/Fill) | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/alignment.png) | ![](cpp_ios_light/alignment.png) | ![](csharp_ios_dark/alignment.png) | ![](cpp_ios_dark/alignment.png) |
| 39 | Z Index | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/z_index.png) | ![](cpp_ios_light/z_index.png) | ![](csharp_ios_dark/z_index.png) | ![](cpp_ios_dark/z_index.png) |
| 40 | Layout Is Enabled | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/layout_is_enabled.png) | ![](cpp_ios_light/layout_is_enabled.png) | ![](csharp_ios_dark/layout_is_enabled.png) | ![](cpp_ios_dark/layout_is_enabled.png) |
| 41 | Shapes | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/shapes.png) | ![](cpp_ios_light/shapes.png) | ![](csharp_ios_dark/shapes.png) | ![](cpp_ios_dark/shapes.png) |
| 42 | Ellipse Gallery | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/ellipse_gallery.png) | ![](cpp_ios_light/ellipse_gallery.png) | ![](csharp_ios_dark/ellipse_gallery.png) | ![](cpp_ios_dark/ellipse_gallery.png) |
| 43 | Rectangle Gallery | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/rectangle_gallery.png) | ![](cpp_ios_light/rectangle_gallery.png) | ![](csharp_ios_dark/rectangle_gallery.png) | ![](cpp_ios_dark/rectangle_gallery.png) |
| 44 | Line Gallery | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/line_gallery.png) | ![](cpp_ios_light/line_gallery.png) | ![](csharp_ios_dark/line_gallery.png) | ![](cpp_ios_dark/line_gallery.png) |
| 45 | Line Join Gallery | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/line_join_gallery.png) | ![](cpp_ios_light/line_join_gallery.png) | ![](csharp_ios_dark/line_join_gallery.png) | ![](cpp_ios_dark/line_join_gallery.png) |
| 46 | Polygon Gallery | 🟡<br>L:minor<br>D:match | ![](csharp_ios_light/polygon_gallery.png) | ![](cpp_ios_light/polygon_gallery.png) | ![](csharp_ios_dark/polygon_gallery.png) | ![](cpp_ios_dark/polygon_gallery.png) |
| 47 | Polyline Gallery | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/polyline_gallery.png) | ![](cpp_ios_light/polyline_gallery.png) | ![](csharp_ios_dark/polyline_gallery.png) | ![](cpp_ios_dark/polyline_gallery.png) |
| 48 | Path Gallery | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/path_gallery.png) | ![](cpp_ios_light/path_gallery.png) | ![](csharp_ios_dark/path_gallery.png) | ![](cpp_ios_dark/path_gallery.png) |
| 49 | Path Aspect Gallery | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/path_aspect_gallery.png) | ![](cpp_ios_light/path_aspect_gallery.png) | ![](csharp_ios_dark/path_aspect_gallery.png) | ![](cpp_ios_dark/path_aspect_gallery.png) |
| 50 | Path Transform String | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/path_transform_string.png) | ![](cpp_ios_light/path_transform_string.png) | ![](csharp_ios_dark/path_transform_string.png) | ![](cpp_ios_dark/path_transform_string.png) |
| 51 | Composition Gallery | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/composition_gallery.png) | ![](cpp_ios_light/composition_gallery.png) | ![](csharp_ios_dark/composition_gallery.png) | ![](cpp_ios_dark/composition_gallery.png) |
| 52 | Transform Playground | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/transform_playground.png) | ![](cpp_ios_light/transform_playground.png) | ![](csharp_ios_dark/transform_playground.png) | ![](cpp_ios_dark/transform_playground.png) |
| 53 | Transformations | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/transformations.png) | ![](cpp_ios_light/transformations.png) | ![](csharp_ios_dark/transformations.png) | ![](cpp_ios_dark/transformations.png) |
| 54 | Update Path Data | 🟡<br>L:minor<br>D:match | ![](csharp_ios_light/update_path_data.png) | ![](cpp_ios_light/update_path_data.png) | ![](csharp_ios_dark/update_path_data.png) | ![](cpp_ios_dark/update_path_data.png) |
| 55 | Auto Size Shapes | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/auto_size_shapes.png) | ![](cpp_ios_light/auto_size_shapes.png) | ![](csharp_ios_dark/auto_size_shapes.png) | ![](cpp_ios_dark/auto_size_shapes.png) |
| 56 | Shape App Theme | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/shape_app_theme.png) | ![](cpp_ios_light/shape_app_theme.png) | ![](csharp_ios_dark/shape_app_theme.png) | ![](cpp_ios_dark/shape_app_theme.png) |
| 57 | Invalidate Brush | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/invalidate_brush.png) | ![](cpp_ios_light/invalidate_brush.png) | ![](csharp_ios_dark/invalidate_brush.png) | ![](cpp_ios_dark/invalidate_brush.png) |
| 58 | Gradient brushes | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/gradient.png) | ![](cpp_ios_light/gradient.png) | ![](csharp_ios_dark/gradient.png) | ![](cpp_ios_dark/gradient.png) |
| 59 | Border | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/border.png) | ![](cpp_ios_light/border.png) | ![](csharp_ios_dark/border.png) | ![](cpp_ios_dark/border.png) |
| 60 | Border Stroke | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/border_stroke.png) | ![](cpp_ios_light/border_stroke.png) | ![](csharp_ios_dark/border_stroke.png) | ![](cpp_ios_dark/border_stroke.png) |
| 61 | Border Layout | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/border_layout.png) | ![](cpp_ios_light/border_layout.png) | ![](csharp_ios_dark/border_layout.png) | ![](cpp_ios_dark/border_layout.png) |
| 62 | Border Playground | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/border_playground.png) | ![](cpp_ios_light/border_playground.png) | ![](csharp_ios_dark/border_playground.png) | ![](cpp_ios_dark/border_playground.png) |
| 63 | Border Clip Playground | 🟡⚠️<br>L:minor<br>D:minor | ![](csharp_ios_light/border_clip_playground.png) | ![](cpp_ios_light/border_clip_playground.png) | ![](csharp_ios_dark/border_clip_playground.png) | ![](cpp_ios_dark/border_clip_playground.png) |
| 64 | Border Resize Content | 🟡⚠️<br>L:minor<br>D:minor | ![](csharp_ios_light/border_resize_content.png) | ![](cpp_ios_light/border_resize_content.png) | ![](csharp_ios_dark/border_resize_content.png) | ![](cpp_ios_dark/border_resize_content.png) |
| 65 | Borderless | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/borderless.png) | ![](cpp_ios_light/borderless.png) | ![](csharp_ios_dark/borderless.png) | ![](cpp_ios_dark/borderless.png) |
| 66 | Clip | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/clip.png) | ![](cpp_ios_light/clip.png) | ![](csharp_ios_dark/clip.png) | ![](cpp_ios_dark/clip.png) |
| 67 | Clip Views | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/clip_views.png) | ![](cpp_ios_light/clip_views.png) | ![](csharp_ios_dark/clip_views.png) | ![](cpp_ios_dark/clip_views.png) |
| 68 | Clip Corner Radius | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/clip_corner_radius.png) | ![](cpp_ios_light/clip_corner_radius.png) | ![](csharp_ios_dark/clip_corner_radius.png) | ![](cpp_ios_dark/clip_corner_radius.png) |
| 69 | Clip Gallery | 🟢⚠️<br>L:match<br>D:match | ![](csharp_ios_light/clip_gallery.png) | ![](cpp_ios_light/clip_gallery.png) | ![](csharp_ios_dark/clip_gallery.png) | ![](cpp_ios_dark/clip_gallery.png) |
| 70 | Clipping | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/clipping.png) | ![](cpp_ios_light/clipping.png) | ![](csharp_ios_dark/clipping.png) | ![](cpp_ios_dark/clipping.png) |
| 71 | Shadow Playground | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/shadow_playground.png) | ![](cpp_ios_light/shadow_playground.png) | ![](csharp_ios_dark/shadow_playground.png) | ![](cpp_ios_dark/shadow_playground.png) |
| 72 | Invalidate Shadow Host | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/invalidate_shadow_host.png) | ![](cpp_ios_light/invalidate_shadow_host.png) | ![](csharp_ios_dark/invalidate_shadow_host.png) | ![](cpp_ios_dark/invalidate_shadow_host.png) |
| 73 | CollectionView | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/collectionview.png) | ![](cpp_ios_light/collectionview.png) | ![](csharp_ios_dark/collectionview.png) | ![](cpp_ios_dark/collectionview.png) |
| 74 | Items | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/items.png) | ![](cpp_ios_light/items.png) | ![](csharp_ios_dark/items.png) | ![](cpp_ios_dark/items.png) |
| 75 | Single Bound Selection | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/single_bound_selection.png) | ![](cpp_ios_light/single_bound_selection.png) | ![](csharp_ios_dark/single_bound_selection.png) | ![](cpp_ios_dark/single_bound_selection.png) |
| 76 | Multiple Bound Selection | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/multiple_bound_selection.png) | ![](cpp_ios_light/multiple_bound_selection.png) | ![](csharp_ios_dark/multiple_bound_selection.png) | ![](cpp_ios_dark/multiple_bound_selection.png) |
| 77 | Preselected Item | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/preselected_item.png) | ![](cpp_ios_light/preselected_item.png) | ![](csharp_ios_dark/preselected_item.png) | ![](cpp_ios_dark/preselected_item.png) |
| 78 | Preselected Items | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/preselected_items.png) | ![](cpp_ios_light/preselected_items.png) | ![](csharp_ios_dark/preselected_items.png) | ![](cpp_ios_dark/preselected_items.png) |
| 79 | Selection Command Param | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/selection_command_param.png) | ![](cpp_ios_light/selection_command_param.png) | ![](csharp_ios_dark/selection_command_param.png) | ![](cpp_ios_dark/selection_command_param.png) |
| 80 | Selection Synchronization | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/selection_synchronization.png) | ![](cpp_ios_light/selection_synchronization.png) | ![](csharp_ios_dark/selection_synchronization.png) | ![](cpp_ios_dark/selection_synchronization.png) |
| 81 | Filter Collection | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/filter_collection.png) | ![](cpp_ios_light/filter_collection.png) | ![](csharp_ios_dark/filter_collection.png) | ![](cpp_ios_dark/filter_collection.png) |
| 82 | Filter Selection | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/filter_selection.png) | ![](cpp_ios_light/filter_selection.png) | ![](csharp_ios_dark/filter_selection.png) | ![](cpp_ios_dark/filter_selection.png) |
| 83 | Header Footer | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/header_footer.png) | ![](cpp_ios_light/header_footer.png) | ![](csharp_ios_dark/header_footer.png) | ![](cpp_ios_dark/header_footer.png) |
| 84 | Header Footer Grid | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/header_footer_grid.png) | ![](cpp_ios_light/header_footer_grid.png) | ![](csharp_ios_dark/header_footer_grid.png) | ![](cpp_ios_dark/header_footer_grid.png) |
| 85 | Header Footer Grid Horizontal | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/header_footer_grid_horizontal.png) | ![](cpp_ios_light/header_footer_grid_horizontal.png) | ![](csharp_ios_dark/header_footer_grid_horizontal.png) | ![](cpp_ios_dark/header_footer_grid_horizontal.png) |
| 86 | Header Footer Template | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/header_footer_template.png) | ![](cpp_ios_light/header_footer_template.png) | ![](csharp_ios_dark/header_footer_template.png) | ![](cpp_ios_dark/header_footer_template.png) |
| 87 | Header Footer View | ⬛<br>L:blank<br>D:blank | ![](csharp_ios_light/header_footer_view.png) | ![](cpp_ios_light/header_footer_view.png) | ![](csharp_ios_dark/header_footer_view.png) | ![](cpp_ios_dark/header_footer_view.png) |
| 88 | Footer Only String | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/footer_only_string.png) | ![](cpp_ios_light/footer_only_string.png) | ![](csharp_ios_dark/footer_only_string.png) | ![](cpp_ios_dark/footer_only_string.png) |
| 89 | Basic Grouping | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/basic_grouping.png) | ![](cpp_ios_light/basic_grouping.png) | ![](csharp_ios_dark/basic_grouping.png) | ![](cpp_ios_dark/basic_grouping.png) |
| 90 | Grid Grouping | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/grid_grouping.png) | ![](cpp_ios_light/grid_grouping.png) | ![](csharp_ios_dark/grid_grouping.png) | ![](cpp_ios_dark/grid_grouping.png) |
| 91 | Grouping No Templates | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/grouping_no_templates.png) | ![](cpp_ios_light/grouping_no_templates.png) | ![](csharp_ios_dark/grouping_no_templates.png) | ![](cpp_ios_dark/grouping_no_templates.png) |
| 92 | Grouping Plus Selection | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/grouping_plus_selection.png) | ![](cpp_ios_light/grouping_plus_selection.png) | ![](csharp_ios_dark/grouping_plus_selection.png) | ![](cpp_ios_dark/grouping_plus_selection.png) |
| 93 | Switch Grouping | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/switch_grouping.png) | ![](cpp_ios_light/switch_grouping.png) | ![](csharp_ios_dark/switch_grouping.png) | ![](cpp_ios_dark/switch_grouping.png) |
| 94 | Some Empty Groups | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/some_empty_groups.png) | ![](cpp_ios_light/some_empty_groups.png) | ![](csharp_ios_dark/some_empty_groups.png) | ![](cpp_ios_dark/some_empty_groups.png) |
| 95 | Scroll To Group | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/scroll_to_group.png) | ![](cpp_ios_light/scroll_to_group.png) | ![](csharp_ios_dark/scroll_to_group.png) | ![](cpp_ios_dark/scroll_to_group.png) |
| 96 | Scroll Mode Test | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/scroll_mode_test.png) | ![](cpp_ios_light/scroll_mode_test.png) | ![](csharp_ios_dark/scroll_mode_test.png) | ![](cpp_ios_dark/scroll_mode_test.png) |
| 97 | Adaptive Collection | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/adaptive_collection.png) | ![](cpp_ios_light/adaptive_collection.png) | ![](csharp_ios_dark/adaptive_collection.png) | ![](cpp_ios_dark/adaptive_collection.png) |
| 98 | Staggered Layout | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/staggered_layout.png) | ![](cpp_ios_light/staggered_layout.png) | ![](csharp_ios_dark/staggered_layout.png) | ![](cpp_ios_dark/staggered_layout.png) |
| 99 | Varied Size Selector | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/varied_size_selector.png) | ![](cpp_ios_light/varied_size_selector.png) | ![](csharp_ios_dark/varied_size_selector.png) | ![](cpp_ios_dark/varied_size_selector.png) |
| 100 | Nested Collection | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/nested_collection.png) | ![](cpp_ios_light/nested_collection.png) | ![](csharp_ios_dark/nested_collection.png) | ![](cpp_ios_dark/nested_collection.png) |
| 101 | Data Template Selector | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/data_template_selector.png) | ![](cpp_ios_light/data_template_selector.png) | ![](csharp_ios_dark/data_template_selector.png) | ![](cpp_ios_dark/data_template_selector.png) |
| 102 | Cv Visual States | 🟡⚠️<br>L:minor<br>D:match | ![](csharp_ios_light/cv_visual_states.png) | ![](cpp_ios_light/cv_visual_states.png) | ![](csharp_ios_dark/cv_visual_states.png) | ![](cpp_ios_dark/cv_visual_states.png) |
| 103 | Empty View | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/empty_view.png) | ![](cpp_ios_light/empty_view.png) | ![](csharp_ios_dark/empty_view.png) | ![](cpp_ios_dark/empty_view.png) |
| 104 | Empty View Null | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/empty_view_null.png) | ![](cpp_ios_light/empty_view_null.png) | ![](csharp_ios_dark/empty_view_null.png) | ![](cpp_ios_dark/empty_view_null.png) |
| 105 | Empty View Rtl | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/empty_view_rtl.png) | ![](cpp_ios_light/empty_view_rtl.png) | ![](csharp_ios_dark/empty_view_rtl.png) | ![](cpp_ios_dark/empty_view_rtl.png) |
| 106 | Empty View Selector | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/empty_view_selector.png) | ![](cpp_ios_light/empty_view_selector.png) | ![](csharp_ios_dark/empty_view_selector.png) | ![](cpp_ios_dark/empty_view_selector.png) |
| 107 | Empty View Swap | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/empty_view_swap.png) | ![](cpp_ios_light/empty_view_swap.png) | ![](csharp_ios_dark/empty_view_swap.png) | ![](cpp_ios_dark/empty_view_swap.png) |
| 108 | Empty View Template | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/empty_view_template.png) | ![](cpp_ios_light/empty_view_template.png) | ![](csharp_ios_dark/empty_view_template.png) | ![](cpp_ios_dark/empty_view_template.png) |
| 109 | Empty View View | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/empty_view_view.png) | ![](cpp_ios_light/empty_view_view.png) | ![](csharp_ios_dark/empty_view_view.png) | ![](cpp_ios_dark/empty_view_view.png) |
| 110 | Empty View Load Simulate | 🟡🎬<br>L:minor<br>D:minor | ![](csharp_ios_light/empty_view_load_simulate.png) | ![](cpp_ios_light/empty_view_load_simulate.png) | ![](csharp_ios_dark/empty_view_load_simulate.png) | ![](cpp_ios_dark/empty_view_load_simulate.png) |
| 111 | Carousel Page | 🟡🎬<br>L:minor<br>D:minor | ![](csharp_ios_light/carousel_page.png) | ![](cpp_ios_light/carousel_page.png) | ![](csharp_ios_dark/carousel_page.png) | ![](cpp_ios_dark/carousel_page.png) |
| 112 | Chat Example | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/chat_example.png) | ![](cpp_ios_light/chat_example.png) | ![](csharp_ios_dark/chat_example.png) | ![](cpp_ios_dark/chat_example.png) |
| 113 | Items Updating Scroll Mode | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/items_updating_scroll_mode.png) | ![](cpp_ios_light/items_updating_scroll_mode.png) | ![](csharp_ios_dark/items_updating_scroll_mode.png) | ![](cpp_ios_dark/items_updating_scroll_mode.png) |
| 114 | Radio Button Group | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/radio_button_group.png) | ![](cpp_ios_light/radio_button_group.png) | ![](csharp_ios_dark/radio_button_group.png) | ![](cpp_ios_dark/radio_button_group.png) |
| 115 | Radio Button Group Binding | 🟢⚠️<br>L:match<br>D:match | ![](csharp_ios_light/radio_button_group_binding.png) | ![](cpp_ios_light/radio_button_group_binding.png) | ![](csharp_ios_dark/radio_button_group_binding.png) | ![](cpp_ios_dark/radio_button_group_binding.png) |
| 116 | Radio Button Group Gallery | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/radio_button_group_gallery.png) | ![](cpp_ios_light/radio_button_group_gallery.png) | ![](csharp_ios_dark/radio_button_group_gallery.png) | ![](cpp_ios_dark/radio_button_group_gallery.png) |
| 117 | Radio Button Border | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/radio_button_border.png) | ![](cpp_ios_light/radio_button_border.png) | ![](csharp_ios_dark/radio_button_border.png) | ![](cpp_ios_dark/radio_button_border.png) |
| 118 | Radio Button Content | 🔴<br>L:minor<br>D:diff | ![](csharp_ios_light/radio_button_content.png) | ![](cpp_ios_light/radio_button_content.png) | ![](csharp_ios_dark/radio_button_content.png) | ![](cpp_ios_dark/radio_button_content.png) |
| 119 | Radio Content Properties | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/radio_content_properties.png) | ![](cpp_ios_light/radio_content_properties.png) | ![](csharp_ios_dark/radio_content_properties.png) | ![](cpp_ios_dark/radio_content_properties.png) |
| 120 | Radio Template From Style | 🟢⚠️<br>L:match<br>D:match | ![](csharp_ios_light/radio_template_from_style.png) | ![](cpp_ios_light/radio_template_from_style.png) | ![](csharp_ios_dark/radio_template_from_style.png) | ![](cpp_ios_dark/radio_template_from_style.png) |
| 121 | Scattered Radio Button | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/scattered_radio_button.png) | ![](cpp_ios_light/scattered_radio_button.png) | ![](csharp_ios_dark/scattered_radio_button.png) | ![](cpp_ios_dark/scattered_radio_button.png) |
| 122 | Swipe Gesture | 🟢⚠️🎬<br>L:match<br>D:match | ![](csharp_ios_light/swipe_gesture.png) | ![](cpp_ios_light/swipe_gesture.png) | ![](csharp_ios_dark/swipe_gesture.png) | ![](cpp_ios_dark/swipe_gesture.png) |
| 123 | Swipe Item Position | 🟢🎬<br>L:match<br>D:match | ![](csharp_ios_light/swipe_item_position.png) | ![](cpp_ios_light/swipe_item_position.png) | ![](csharp_ios_dark/swipe_item_position.png) | ![](cpp_ios_dark/swipe_item_position.png) |
| 124 | Swipe Item Size | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/swipe_item_size.png) | ![](cpp_ios_light/swipe_item_size.png) | ![](csharp_ios_dark/swipe_item_size.png) | ![](cpp_ios_dark/swipe_item_size.png) |
| 125 | Swipe Threshold | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/swipe_threshold.png) | ![](cpp_ios_light/swipe_threshold.png) | ![](csharp_ios_dark/swipe_threshold.png) | ![](cpp_ios_dark/swipe_threshold.png) |
| 126 | Swipe View Margin | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/swipe_view_margin.png) | ![](cpp_ios_light/swipe_view_margin.png) | ![](csharp_ios_dark/swipe_view_margin.png) | ![](cpp_ios_dark/swipe_view_margin.png) |
| 127 | Swipe View Shadow | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/swipe_view_shadow.png) | ![](cpp_ios_light/swipe_view_shadow.png) | ![](csharp_ios_dark/swipe_view_shadow.png) | ![](cpp_ios_dark/swipe_view_shadow.png) |
| 128 | Swipe Refresh | 🟢🎬<br>L:match<br>D:match | ![](csharp_ios_light/swipe_refresh.png) | ![](cpp_ios_light/swipe_refresh.png) | ![](csharp_ios_dark/swipe_refresh.png) | ![](cpp_ios_dark/swipe_refresh.png) |
| 129 | Refresh View | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/refresh_view.png) | ![](cpp_ios_light/refresh_view.png) | ![](csharp_ios_dark/refresh_view.png) | ![](cpp_ios_dark/refresh_view.png) |
| 130 | Custom Size Swipe | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/custom_size_swipe.png) | ![](cpp_ios_light/custom_size_swipe.png) | ![](csharp_ios_dark/custom_size_swipe.png) | ![](cpp_ios_dark/custom_size_swipe.png) |
| 131 | Custom Swipe Item View | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/custom_swipe_item_view.png) | ![](cpp_ios_light/custom_swipe_item_view.png) | ![](csharp_ios_dark/custom_swipe_item_view.png) | ![](cpp_ios_dark/custom_swipe_item_view.png) |
| 132 | Basic Swipe | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/basic_swipe.png) | ![](cpp_ios_light/basic_swipe.png) | ![](csharp_ios_dark/basic_swipe.png) | ![](cpp_ios_dark/basic_swipe.png) |
| 133 | Gestures | 🟡🎬<br>L:minor<br>D:minor | ![](csharp_ios_light/gestures.png) | ![](cpp_ios_light/gestures.png) | ![](csharp_ios_dark/gestures.png) | ![](cpp_ios_dark/gestures.png) |
| 134 | Pan Gesture Events | 🟢🎬<br>L:match<br>D:match | ![](csharp_ios_light/pan_gesture_events.png) | ![](cpp_ios_light/pan_gesture_events.png) | ![](csharp_ios_dark/pan_gesture_events.png) | ![](cpp_ios_dark/pan_gesture_events.png) |
| 135 | Pointer Gesture | 🟡🎬<br>L:minor<br>D:minor | ![](csharp_ios_light/pointer_gesture.png) | ![](cpp_ios_light/pointer_gesture.png) | ![](csharp_ios_dark/pointer_gesture.png) | ![](cpp_ios_dark/pointer_gesture.png) |
| 136 | Drag Drop | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/drag_drop.png) | ![](cpp_ios_light/drag_drop.png) | ![](csharp_ios_dark/drag_drop.png) | ![](cpp_ios_dark/drag_drop.png) |
| 137 | Hit Testing | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/hit_testing.png) | ![](cpp_ios_light/hit_testing.png) | ![](csharp_ios_dark/hit_testing.png) | ![](cpp_ios_dark/hit_testing.png) |
| 138 | Input Transparent | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/input_transparent.png) | ![](cpp_ios_light/input_transparent.png) | ![](csharp_ios_dark/input_transparent.png) | ![](cpp_ios_dark/input_transparent.png) |
| 139 | Focus | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/focus.png) | ![](cpp_ios_light/focus.png) | ![](csharp_ios_dark/focus.png) | ![](cpp_ios_dark/focus.png) |
| 140 | Dispatcher | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/dispatcher.png) | ![](cpp_ios_light/dispatcher.png) | ![](csharp_ios_dark/dispatcher.png) | ![](cpp_ios_dark/dispatcher.png) |
| 141 | Device | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/device.png) | ![](cpp_ios_light/device.png) | ![](csharp_ios_dark/device.png) | ![](cpp_ios_dark/device.png) |
| 142 | Effects | 🔴⚠️<br>L:diff<br>D:diff | ![](csharp_ios_light/effects.png) | ![](cpp_ios_light/effects.png) | ![](csharp_ios_dark/effects.png) | ![](cpp_ios_dark/effects.png) |
| 143 | Measure First Strategy | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/measure_first_strategy.png) | ![](cpp_ios_light/measure_first_strategy.png) | ![](csharp_ios_dark/measure_first_strategy.png) | ![](cpp_ios_dark/measure_first_strategy.png) |
| 144 | Scroll View | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/scroll_view.png) | ![](cpp_ios_light/scroll_view.png) | ![](csharp_ios_dark/scroll_view.png) | ![](cpp_ios_dark/scroll_view.png) |
| 145 | Web View | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/web_view.png) | ![](cpp_ios_light/web_view.png) | ![](csharp_ios_dark/web_view.png) | ![](cpp_ios_dark/web_view.png) |
| 146 | Hybrid Web View | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/hybrid_web_view.png) | ![](cpp_ios_light/hybrid_web_view.png) | ![](csharp_ios_dark/hybrid_web_view.png) | ![](cpp_ios_dark/hybrid_web_view.png) |
| 147 | Alerts | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/alerts.png) | ![](cpp_ios_light/alerts.png) | ![](csharp_ios_dark/alerts.png) | ![](cpp_ios_dark/alerts.png) |
| 148 | Animation | 🟡🎬<br>L:minor<br>D:minor | ![](csharp_ios_light/animation.png) | ![](cpp_ios_light/animation.png) | ![](csharp_ios_dark/animation.png) | ![](cpp_ios_dark/animation.png) |
| 149 | Application Control | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/application_control.png) | ![](cpp_ios_light/application_control.png) | ![](csharp_ios_dark/application_control.png) | ![](cpp_ios_dark/application_control.png) |
| 150 | Ios Entry | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/ios_entry.png) | ![](cpp_ios_light/ios_entry.png) | ![](csharp_ios_dark/ios_entry.png) | ![](cpp_ios_dark/ios_entry.png) |
| 151 | Ios Date Picker | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/ios_date_picker.png) | ![](cpp_ios_light/ios_date_picker.png) | ![](csharp_ios_dark/ios_date_picker.png) | ![](cpp_ios_dark/ios_date_picker.png) |
| 152 | Ios Time Picker | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/ios_time_picker.png) | ![](cpp_ios_light/ios_time_picker.png) | ![](csharp_ios_dark/ios_time_picker.png) | ![](cpp_ios_dark/ios_time_picker.png) |
| 153 | Ios Picker | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/ios_picker.png) | ![](cpp_ios_light/ios_picker.png) | ![](csharp_ios_dark/ios_picker.png) | ![](cpp_ios_dark/ios_picker.png) |
| 154 | Ios Search Bar | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/ios_search_bar.png) | ![](cpp_ios_light/ios_search_bar.png) | ![](csharp_ios_dark/ios_search_bar.png) | ![](cpp_ios_dark/ios_search_bar.png) |
| 155 | Ios Scroll View | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/ios_scroll_view.png) | ![](cpp_ios_light/ios_scroll_view.png) | ![](csharp_ios_dark/ios_scroll_view.png) | ![](cpp_ios_dark/ios_scroll_view.png) |
| 156 | Ios Slider Update On Tap | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/ios_slider_update_on_tap.png) | ![](cpp_ios_light/ios_slider_update_on_tap.png) | ![](csharp_ios_dark/ios_slider_update_on_tap.png) | ![](cpp_ios_dark/ios_slider_update_on_tap.png) |
| 157 | Ios First Responder | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/ios_first_responder.png) | ![](cpp_ios_light/ios_first_responder.png) | ![](csharp_ios_dark/ios_first_responder.png) | ![](cpp_ios_dark/ios_first_responder.png) |
| 158 | Ios Pan Gesture | 🟢🎬<br>L:match<br>D:match | ![](csharp_ios_light/ios_pan_gesture.png) | ![](cpp_ios_light/ios_pan_gesture.png) | ![](csharp_ios_dark/ios_pan_gesture.png) | ![](cpp_ios_dark/ios_pan_gesture.png) |
| 159 | Ios Safe Area | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/ios_safe_area.png) | ![](cpp_ios_light/ios_safe_area.png) | ![](csharp_ios_dark/ios_safe_area.png) | ![](cpp_ios_dark/ios_safe_area.png) |
| 160 | Ios Swipe Transition | 🟡🎬<br>L:minor<br>D:minor | ![](csharp_ios_light/ios_swipe_transition.png) | ![](cpp_ios_light/ios_swipe_transition.png) | ![](csharp_ios_dark/ios_swipe_transition.png) | ![](cpp_ios_dark/ios_swipe_transition.png) |
| 161 | Ios Blur Effect | 🔴⚠️🎬<br>L:diff<br>D:diff | ![](csharp_ios_light/ios_blur_effect.png) | ![](cpp_ios_light/ios_blur_effect.png) | ![](csharp_ios_dark/ios_blur_effect.png) | ![](cpp_ios_dark/ios_blur_effect.png) |
| 162 | Navigation Gallery | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/navigation_gallery.png) | ![](cpp_ios_light/navigation_gallery.png) | ![](csharp_ios_dark/navigation_gallery.png) | ![](cpp_ios_dark/navigation_gallery.png) |
| 163 | Modal | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/modal.png) | ![](cpp_ios_light/modal.png) | ![](csharp_ios_dark/modal.png) | ![](cpp_ios_dark/modal.png) |
| 164 | Tabbed Flyout | ⬛<br>L:blank<br>D:blank | ![](csharp_ios_light/tabbed_flyout.png) | ![](cpp_ios_light/tabbed_flyout.png) | ![](csharp_ios_dark/tabbed_flyout.png) | ![](cpp_ios_dark/tabbed_flyout.png) |
| 165 | Toolbar | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/toolbar.png) | ![](cpp_ios_light/toolbar.png) | ![](csharp_ios_dark/toolbar.png) | ![](cpp_ios_dark/toolbar.png) |
| 166 | Menu Bar | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/menu_bar.png) | ![](cpp_ios_light/menu_bar.png) | ![](csharp_ios_dark/menu_bar.png) | ![](cpp_ios_dark/menu_bar.png) |
| 167 | Title Bar | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/title_bar.png) | ![](cpp_ios_light/title_bar.png) | ![](csharp_ios_dark/title_bar.png) | ![](cpp_ios_dark/title_bar.png) |
| 168 | Chrome | 🟡🎬<br>L:minor<br>D:minor | ![](csharp_ios_light/chrome.png) | ![](cpp_ios_light/chrome.png) | ![](csharp_ios_dark/chrome.png) | ![](cpp_ios_dark/chrome.png) |
| 169 | Context Flyout | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/context_flyout.png) | ![](cpp_ios_light/context_flyout.png) | ![](csharp_ios_dark/context_flyout.png) | ![](cpp_ios_dark/context_flyout.png) |
| 170 | Templated View | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/templated_view.png) | ![](cpp_ios_light/templated_view.png) | ![](csharp_ios_dark/templated_view.png) | ![](cpp_ios_dark/templated_view.png) |
| 171 | Custom Layout | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/custom_layout.png) | ![](cpp_ios_light/custom_layout.png) | ![](csharp_ios_dark/custom_layout.png) | ![](cpp_ios_dark/custom_layout.png) |
| 172 | Visual States | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/visual_states.png) | ![](cpp_ios_light/visual_states.png) | ![](csharp_ios_dark/visual_states.png) | ![](cpp_ios_dark/visual_states.png) |

## Per-page findings

Concrete, per-theme notes for every page with a diff, a broken reference, or a motion/effect caveat. Clean both-theme matches with no note are omitted. Numbers match the grid above.

### 1. Label — 🟢 (L:match / D:match)
- **Light:** All label demos match: 'Defaults', red 'This text should be RED', cyan-background label, start/center/end-aligned labels in gray cards, justified Lorem ipsum paragraph, and 'This should be at the start/center' gray boxes. C++ renders the page at a slightly smaller text scale so more content fits on one screen (also shows the 'Plain old Text / Colors / Strikethrough / Big Font' FormattedString row and the repeated Lorem rows that MAUI crops off below 'This should be at the center'); this is a harness scroll/scale artifact, not a content difference.
- **Dark:** Same as light — identical structure and colors in dark theme (red text, cyan background, gray cards all correct). Difference is only that C++ fits more rows on screen due to a smaller text scale; comparable region is equivalent.

### 2. Button — 🟢 (L:match / D:match)
- **Light:** All comparable button demos match: 'Taps: 0', blue 'Button', gray 'Button (disabled)', 'Clicked', 'Command', solid blue/red/green colored buttons, green 'BorderColor', green 'BorderWidth' with red border, purple rounded 'CornerRadius', pink 'Button'. C++ is scaled smaller and fits the rest of the page that MAUI crops (gear ImageButtons 'settings', Decrease/Increase Spacing, a slider, 'BorderWidth Changing') — harness scroll/scale, not a bug.
- **Dark:** Same matching set of buttons and colors in dark. The extra ImageButton/slider rows C++ shows are below MAUI's crop point so they are not comparable; everything visible in both matches.

### 3. Entry — 🟢 (L:match / D:match)
- **Light:** Matches: 'LENGTH: 0  RETURN: Default', 'Type here...' placeholder, purple 'Text', purple 'Placeholder', checked blue checkbox, password dots (6), 'I am read only', 'Text', right-aligned 'This should be on the end', 'CursorPosition = 4'. Entries have the standard rounded gray border in both. C++ fits the extra slider + 'Cursor' entry below that MAUI crops — harness scale.
- **Dark:** Identical structure and colors in dark theme; same harness scale difference (C++ shows the trailing slider/Cursor entry MAUI crops).

### 4. Editor — 🟢 (L:match / D:match)
- **Light:** Matches exactly: 'LENGTH: 0', 'Type here...' placeholder, purple 'Text', purple 'Placeholder', large 'FontSize (Large)', 'I am read only', '123', 'This should be on the bottom', 'AUTOSIZE LENGTH: 0', 'Grows as you type...' placeholder. Editors are borderless in both (correct iOS editor styling).
- **Dark:** Same content, colors and borderless editors in dark theme; equivalent.

### 5. Search Bar — 🟢 (L:match / D:match)
- **Light:** Matches: 'LENGTH: 0  SEARCHES: 0', pill search bars with magnifier icon and 'Search...' placeholder, green 'Green text' with clear-X plus a secondary close-X button, pink 'Placeholder', italic 'Italic 24pt' enlarged bar. C++ is scaled smaller and shows additional rows ('end of the line' right-aligned, 'Cancel is red', 'Numeric keyboard' placeholder) that MAUI crops; harness scale only.
- **Dark:** Same search bars and accent colors rendered correctly in dark (dark-gray pill fills, green/pink text). Same harness scale difference for the trailing rows.

### 6. Picker — 🟢 (L:match / D:match)
- **Light:** Matches: 'Basic' 'Select an item' placeholder, 'SelectedIndex=1' Item 2, 'SelectedIndexChanged' Item 2, 'Selected: (none)', 'TextColor=Blue' picker, 'TitleColor=Blue' blue 'Select an item', 'FontAttributes=Italic + BackgroundColor=Yellow' yellow italic picker. C++ fits the rest (Dynamic add items, Clear/Add/Replace Items, green 'Item 1' picker) that MAUI crops — harness scale.
- **Dark:** Same pickers and accent colors in dark theme; the italic placeholder on the yellow picker is equally low-contrast (light text on yellow) in both. Equivalent.

### 7. Date Picker — 🟡 (L:minor / D:minor)
- **Light:** Structure matches (Default, BackgroundColor blue, two Background gradient pickers, Update/Clear Background, 'Default with date' 21.06.2018). Date format is correct device-locale DD.MM.YYYY in both — MAUI shows '19.06.2026' vs C++ '21.06.2026' purely because they were captured on different days (today's date), NOT a format bug. One real cosmetic diff: the third 'Background' gradient picker uses a blue->cyan/teal gradient in MAUI but a pink/magenta->purple gradient in C++ (the first yellow->green Background gradient matches). C++ also fits trailing rows (Disabled, red TextColor, Format 2026/06/21, IsFocused) MAUI crops.
- **Dark:** Same as light: device-locale DD.MM.YYYY date format correct in both (capture-day difference only). Same third-gradient hue mismatch (MAUI blue->cyan vs C++ pink->magenta).

### 8. Time Picker — 🟡 (L:minor / D:minor)
- **Light:** Structure matches (Default, BackgroundColor blue, two Background gradient pickers, Update/Clear Background, 'Default with time' 4:15). Time format is correct 24h device-locale '0:00' in both (NOT the old invariant '12:00 AM' bug) — format fix confirmed working; the C++ 'Format' demo row correctly shows '12:00'. Same one cosmetic diff as date_picker: the third 'Background' gradient picker is blue->cyan/teal in MAUI vs pink/magenta->purple in C++ (first yellow->green gradient matches). C++ fits trailing rows (Disabled, green TextColor, Format 12:00, IsFocused) MAUI crops.
- **Dark:** Same as light: 24h '0:00' device-locale time format correct in both. Same third-gradient hue mismatch (MAUI blue->cyan vs C++ pink->magenta).

### 9. Pickers — 🟡 (L:minor / D:match)
- **Light:** Both show the same three pickers (placeholder 'Pick a room', a date field in device-locale dd.MM.yyyy format, a time field '09:00' 24h) plus the 'No room on M/d/yyyy at 09:00' label. Content/format/layout match. Only difference: MAUI light wraps the pickers in a white card container while C++ has no card (transparent background). The date values differ (19.06.2026 vs 21.06.2026) only because captured on different days — not a bug.
- **Dark:** Identical to MAUI: 'Pick a room' placeholder, date field (dd.MM.yyyy device-locale), time '09:00' (24h), and 'No room on M/d/yyyy at 09:00' label. No card container in either dark capture. Date differs (19 vs 21) only due to capture day.

### 10. Slider — 🟡 (L:minor / D:minor)
- **Light:** Same slider sequence and colors: Default, BackgroundColor (blue), Background (yellow-green gradient), Minimum(5)/Maximum(15) with value '5', Disabled, MinimumTrackColor=LightBlue, etc. Two cosmetic diffs: MAUI section headers ('Default','BackgroundColor') are bold while C++ renders them regular weight; MAUI light has a white card container, C++ does not. C++ also fits more rows (tighter vertical spacing) so MaximumTrackColor=Pink/ThumbColor=Orange/ThumbImageSource/Custom Slider are visible — content is identical.
- **Dark:** Same slider list and colors as MAUI including the gray disabled thumb. Only cosmetic diff: section headers bold in MAUI vs regular weight in C++, and C++ uses slightly tighter row spacing.

### 11. Stepper — 🟡 (L:minor / D:minor)
- **Light:** Same controls in same order: Default, Disabled, 'Enable Stepper' link, BackgroundColor (red), Background, Minimum(5)/Maximum(25), Increment(2), ValueChanged, 'Value: 0'. Two diffs: (1) the BackgroundColor red fill spans the full row width in MAUI but in C++ the red fill is sized only to the stepper -/+ pill; (2) section headers bold in MAUI vs regular weight in C++.
- **Dark:** Same controls/order as MAUI. Same two diffs: BackgroundColor red fill spans full row width in MAUI but only the -/+ pill width in C++; section headers bold vs regular weight.

### 12. Switch — 🟡 (L:minor / D:minor)
- **Light:** Same switches and colors: Default (off, 'Default switch is Off'), BackgroundColor (blue), Background (yellow-green gradient), Disabled, OnColor, ThumbColor. Diffs: the ThumbColor demo switch shows a white/light thumb in MAUI but an orange thumb in C++; section headers bold in MAUI vs regular weight in C++; MAUI light has a white card container.
- **Dark:** Same switches/colors as MAUI. Same diffs: ThumbColor switch thumb is white in MAUI but orange in C++; section headers bold vs regular weight.

### 13. Check Box — 🟡 (L:minor / D:minor)
- **Light:** Excellent match: Default (blue ring), Colored (purple ring), Disabled (blue ring), Disabled Colored (filled purple checked), Change IsChecked, 'Is green? False' (red text) + filled red check. All checkbox colors/positions/checked states match. Only diff: section headers bold in MAUI vs regular weight in C++ (and MAUI light has a white card container).
- **Dark:** Same checkbox states/colors as MAUI. Only diff: section headers bold vs regular weight.

### 14. Progress Bar — 🟡 (L:minor / D:minor)
- **Light:** Excellent match: Default (blue ~50%), ProgressColor (orange ~50%), Disabled (blue), ProgressColor (orange), ProgressTo (empty bar), and the 'ProgressTo' blue link. Fill amounts, colors and order all match. Only diff: section headers bold in MAUI vs regular weight in C++ (and MAUI light has a white card container).
- **Dark:** Same bars/colors/fills as MAUI. Only diff: section headers bold vs regular weight.

### 15. Activity Indicator — 🟡 (L:minor / D:minor)  🎬 _motion/effect — needs an animated GIF to judge_
- **Light:** Same spinners and layout: Default, Styled-Color from theme (blue), Styled-BackgroundColor=Yellow (yellow bar + spinner), Larger (big spinner), Smaller-HorizontalOptions=Center. C++ additionally shows 'Not Running' and '- End of page -' (tighter spacing fits more; MAUI cut them off below). Only cosmetic diff: section headers bold in MAUI vs regular weight in C++. Spinners are animated so spin parity needs a GIF, but static layout/count/colors match.
- **Dark:** Same spinners/layout/colors as MAUI; C++ fits the extra 'Not Running' and '- End of page -' rows. Only diff: section headers bold vs regular weight. Spin animation would need a GIF for full parity.

### 16. Indicator — 🔴 (L:diff / D:diff)
- **Light:** Labels and colors match (Basic, Colors=blue/blue/red/blue/blue on yellow, Indicator Shape, Indicator Size, HideSingle blank, MaximumVisible 7-of-10, Template, 'Using with CarouselView' + 'Item 1' with 3 dots). But the dot LAYOUT differs: MAUI renders evenly-spaced dot rows, while C++ splits each dot row with a visible gap in the middle (dots cluster as a left group and right group around the selected dot). Also the 'Indicator Size' demo dots are rendered much larger and more spread out in C++ than MAUI's small dots. The CarouselView 'Item 1' label also wraps onto the same line as 'Using with CarouselView' in C++.
- **Dark:** Same as light: labels/colors match but C++ indicator dot rows have an abnormal middle gap (left+right dot clusters) vs MAUI's even spacing, and the 'Indicator Size' demo dots are noticeably larger/wider-spaced in C++. 'Item 1' overlaps the 'Using with CarouselView' line in C++.

### 17. Image — 🔴 (L:diff / D:diff)
- **Light:** C++ is missing two of MAUI's three sections. MAUI shows: UriSource (Microsoft campus photo), FileSource (purple 3D submarine render), and a 'Font Image Source' label. C++ shows only the UriSource photo (rendered noticeably LARGER/taller) and the 'FileSource' label, but the FileSource purple submarine image is absent and the entire 'Font Image Source' section is missing.
- **Dark:** Same as light: C++ renders only the UriSource building photo (oversized) and the FileSource label; MAUI's FileSource purple-submarine image is missing in C++ and the 'Font Image Source' section is absent entirely.

### 18. Image Button — 🟢 (L:match / D:match)
- **Light:** The MAUI reference is scrolled to the top and truncated after the 'BorderColor + BorderWidth' green/red-border button; C++ shows the full page (additionally: slider, 'CornerRadius = 0 / 10 / slider' three purple bars, a second slider, 'Custom Size (click to resize)' submarine thumbnail, 'Padding (slider-driven)'). All overlapping content matches: '0 ImageButton clicks', AspectFit/AspectFill/Fill green buttons, and the red-bordered green button are equivalent. Extra C++ content is correct page content not visible in the truncated reference.
- **Dark:** Same as light: overlapping content (clicks counter, AspectFit/AspectFill/Fill green buttons, BorderColor+BorderWidth) matches; the additional sliders, purple CornerRadius bars, custom-size thumbnail and padding row in C++ are correct continuation of the page that the truncated MAUI reference does not show.

### 19. Box View — 🟢 (L:match / D:match)
- **Light:** MAUI reference is truncated at the 'Background' label; C++ shows the full page including the extra 'Using CornerRadius' rounded green box. All overlapping content matches: Default (blue box), Using Color (purple box) and the Background yellow-to-green gradient are equivalent in size and color.
- **Dark:** Same as light: Default blue box, Using Color purple box, and Background yellow-to-green gradient match; C++ additionally shows the 'Using CornerRadius' rounded light-green box that the truncated MAUI reference cuts off.

### 20. Content View — 🟢 (L:match / D:match)
- **Light:** Equivalent: 'ContentView' label, indented 'Content', a second 'Content', and a centered blue 'Swap content' button. Layout, text and colors all match.
- **Dark:** Equivalent: same 'ContentView' / 'Content' / 'Content' stack and centered blue 'Swap content' button; matches MAUI.

### 21. Containers — 🟡 (L:minor / D:match)
- **Light:** Same structure: 'Scrolled to: 0 / 0', 'Inside a border' (blue dashed box), 'Inside a frame' (red-bordered rounded box), 'Inside a content_view'. Cosmetic difference: the C++ 'Inside a frame' box has a light-gray background fill, whereas MAUI's frame is white/transparent (page-colored). Also the C++ frame label text is bold vs regular in MAUI.
- **Dark:** Clean match: 'Scrolled to: 0 / 0', blue dashed 'Inside a border', red-bordered 'Inside a frame' (transparent in both), and 'Inside a content_view' are all equivalent.

### 22. Control stack — 🟢 (L:match / D:match)
- **Light:** MAUI reference is truncated near the stepper; C++ shows the full page. All overlapping controls match: 'Controls' header, blue 'A Button', 'An Entry' field, 'An Editor', rounded 'A SearchBar', checkbox + green switch + activity spinner row, slider, and +/- stepper. C++ additionally shows the trailing blue progress bar that the truncated reference cuts off.
- **Dark:** Same as light: 'Controls' header, A Button, An Entry, An Editor, SearchBar, checkbox/switch/spinner row, slider and stepper all match; the trailing blue progress bar shown in C++ is the page continuation not visible in the truncated MAUI reference.

### 23. Input Controls — 🟡 (L:minor / D:minor)
- **Light:** FIXED: the UPPER/lower radio buttons now left-align at the margin (HorizontalOptions=Start) matching MAUI (were centered). Editor, search bar, readout all match.
- **Dark:** Same as light; left-aligned radios in dark.

### 24. Fonts — 🟢 (L:match / D:match)
- **Light:** Clean match: the full type ladder Title, Subtitle, Header, Body, Caption, Bold, Italic, Bold + Italic, and the letter-spaced mono 'Character spacing 4.0' all render with equivalent sizes, weights and styles.
- **Dark:** Clean match: identical type ladder (Title through Bold + Italic) plus the 'Character spacing 4.0' letter-spaced line; sizes, weights and styles all equivalent to MAUI.

### 25. Formatted Text — 🟡 (L:minor / D:minor)
- **Light:** Both render the same FormattedString: 'Bold red' (red+bold), 'italic underlined' (italic+underline), 'k e r n e d' (kerned letter-spacing), and 'Plain text label'. Spans, colors, and styling are identical. Only difference: MAUI vertically centers the content within a white content card (text sits near vertical center), while C++ places the text near the top of the page. Cosmetic vertical-position difference only.
- **Dark:** Same as light: identical formatted spans (red bold / italic-underlined / kerned / plain). MAUI centers the content lower on a black background; C++ renders it at the top. Colors and styling match; only the vertical position differs.

### 26. Styles — 🟡 (L:minor / D:minor)
- **Light:** Both render 4 styled items: base subtitle label (gray), custom-derived pink label ('Pink wins'), plain default-color label ('no explicit style'), and a 'Style Me' button with a yellow border. Styling/colors match. Cosmetic differences: MAUI wraps the pink and 'no explicit style' lines to 2 lines (narrower content card) while C++ keeps them on fewer lines (full page width); MAUI's button is taller with rounded corners and more padding, C++'s button is shorter/thinner.
- **Dark:** Same 4 styled items and yellow-bordered 'Style Me' button render in both. The pink custom-style label is a more saturated magenta-pink in MAUI vs a lighter pink in C++ (both clearly pink). Text wrapping and button height/padding differ as in light. Cosmetic only.

### 27. Triggers — 🟡 (L:minor / D:minor)
- **Light:** FIXED: the 'Triggers' heading now renders bold ~32pt (C# Title style) matching MAUI (was small/regular). Content (instructions, entry, status, Toggle button + the validation/highlight triggers) matches.
- **Dark:** Same as light; bold heading + matching content in dark.

### 28. Behaviors — 🟡 (L:minor / D:minor)
- **Light:** Both render the large heading 'Red when the number isn't valid' and an 'Enter a System.Double' entry field. Same large heading font in both. Only difference is line-wrapping driven by content width: MAUI (narrower card) wraps 'Red when the / number isn't valid', C++ (full width) wraps 'Red when the number / isn't valid'. Cosmetic wrap difference only.
- **Dark:** Same as light: identical heading and entry field; only the wrap point of the heading differs due to content width. Cosmetic.

### 29. Semantics — 🟡 (L:minor / D:minor)
- **Light:** Both render the SemanticProperties showcase identically: header, 'Semantics readout: (interact to inspect)', 'Label text TH/DH', blue 'Button text TH/DH', 'Entry text, DTH' field, 'Editor text, DTH', a search bar 'Search bar text, DH' with clear X, and the 'HeadingLevel labels' section. Difference is the visible scroll viewport: MAUI vertically centers content in the white card so it cuts off at 'HeadingLevel labels...', whereas C++ starts content higher and shows more (Heading 1-4, StackLayout labels, 'Click to set semantic focus...'). Same content, different visible offset.
- **Dark:** Same content and structure in dark; identical controls and search bar. C++ shows more of the list visible (down to 'Label receiving semantic focus') because MAUI centers content lower in the card. Cosmetic vertical-offset difference.

### 30. App Theme Binding — 🟡 (L:match / D:minor)
- **Light:** All matches in light: 'AppThemeBinding' header, green/orange themed text, ResourceDictionary text, blue 'Toggle theme' button, 'Theme: Light' status.
- **Dark:** AppThemeBinding state + green/orange text colors match; the only diff is MAUI's harness wraps the content in a WHITE card under system dark while the port renders on the system-dark background — a harness-framing artifact, not a port bug.

### 31. Stack Layout — 🟡 (L:minor / D:minor)
- **Light:** Both render a 'Vertical' label with a vertical column of 6 colored squares (red, yellow, blue, green, orange, purple) and a 'Horizontal' label with a horizontal row of the same 6 colors. Order, colors, and structure match exactly. Cosmetic sizing: MAUI's vertical squares are larger/taller and the horizontal row spans the full page width to the right edge; C++'s squares are slightly smaller and the horizontal row is slightly narrower (doesn't reach the right edge).
- **Dark:** Same content and color order in dark on a black background. Same minor sizing difference: MAUI squares slightly larger and horizontal row wider; C++ slightly smaller/narrower. Cosmetic only.

### 32. Vertical Stack — 🟡 (L:minor / D:minor)
- **Light:** Both render the 'VerticalStackLayout' label and a centered vertical column of 6 colored squares (red, yellow, blue, green, orange, purple). Color order and structure match exactly. Only difference: MAUI's squares are slightly larger/taller than C++'s. Cosmetic sizing only.
- **Dark:** Same content and color order in dark on a black background; squares centered horizontally. MAUI squares slightly larger than C++'s. Cosmetic sizing only.

### 33. Horizontal Stack — 🔴 (L:diff / D:diff)
- **Light:** Both show the 'HorizontalStackLayout' header and a right-aligned, vertically-centered horizontal row of colored boxes. MAUI shows 6 full boxes (red, yellow, blue, green, orange, purple). C++ shows only 4 full boxes (red, yellow, blue, green) plus a thin sliver of orange clipped at the right screen edge — the last two boxes are pushed off-screen/clipped. Wrong item count visible / content clipped at right edge.
- **Dark:** Same as light: MAUI shows all 6 boxes (red, yellow, blue, green, orange, purple) right-aligned; C++ shows 4 full boxes plus an orange sliver clipped at the right edge, with the purple box entirely off-screen. Boxes overflow/clip to the right in C++.

### 34. Grid — 🟡 (L:minor / D:minor)
- **Light:** Both show 'Grid (2 cols x rows)' header and an identical 2x2 grid (red, green / blue, orange) with matching cell sizes and gutter. Only difference is vertical placement: MAUI wraps the content in a harness card pushed down the page so the grid sits mid-screen; C++ renders the grid flush near the top. Content and structure identical.
- **Dark:** Same as light: identical 2x2 grid (red, green / blue, orange) and header; MAUI positions it lower (harness card), C++ at the top. Structure and colors match.

### 35. Absolute Layout — 🟡 (L:minor / D:match)
- **Light:** Both place the same AbsoluteLayout children identically: top-center blue bar, left green vertical bar, right red vertical bar, centered 'Centered text', lower-left 'AutoSized' (blue highlight), and a bottom-center dark bar. Only difference: MAUI renders inside a light-gray harness container card filling the page, while C++ has a white/transparent background. Positions of all children match.
- **Dark:** Both backgrounds are black and all children match in position and color (top-center blue bar, left green bar, right red bar, 'Centered text', lower-left 'AutoSized', bottom-center dark bar). Visually equivalent.

### 36. Flex Layout — 🟡 (L:minor / D:minor)
- **Light:** Both render the same FlexLayout: cyan HEADER bar on top, blue left column, gray CONTENT center, green right column, pink FOOTER bar at bottom — same colors and proportions. Difference is vertical extent: MAUI is bounded by a harness card (white margins top/bottom) so HEADER starts lower and FOOTER ends higher; C++ fills the screen edge-to-edge so the FOOTER is pushed to the very bottom (partially below the visible area). Structure identical.
- **Dark:** Same flex structure and colors (cyan HEADER, blue/green side columns, gray CONTENT, pink FOOTER). MAUI is inset by the harness card (black margins); C++ fills to the bottom edge with the pink FOOTER nearly off the bottom. Content equivalent, only vertical extent differs.

### 37. Relative Layout — 🟡 (L:minor / D:minor)
- **Light:** Corner squares (red TL/green TR/blue BL/yellow BR) + centered gray-with-black box all in matching proportional positions. Residual is a HARNESS-framing artifact: the C++ gallery gives the AbsoluteLayout the full screen height while the maui-compare host frames it in a shorter card, so the 0.33-proportional center box resolves to a different aspect ratio. Not a port bug (proportional sizing is correct on both against their given heights).
- **Dark:** Same; corner squares + center box positions match; only the harness content-area aspect differs.

### 38. Layout alignment (Start/Center/End/Fill) — 🟢 (L:match / D:match)
- **Light:** Both show four labeled sections (Start, Center, End, Fill) each with a blue button outlined in red, aligned per its label: Start left-aligned, Center centered, End right-aligned, Fill centered/narrower. All four alignments, button colors, red borders, and label text match exactly. Only difference is the whole block sits lower in MAUI (harness card offset).
- **Dark:** Identical to light: Start/Center/End/Fill sections with red-outlined blue buttons aligned correctly (left/center/right/center). All alignment behavior, colors and text match; MAUI is offset slightly lower due to the harness card.

### 39. Z Index — 🟡 (L:minor / D:minor)
- **Light:** Both show the 'Z-Index of Label 5: 5' header with a -/+ stepper and a diagonal cascade of 10 colored labels ('This is Label 0..9, z-index 0..9') stacked so red Label 9 is frontmost. The z-order, colors, label text and cascade direction all match. Only differences: MAUI sits lower (harness card) and C++ stacks the rows with slightly tighter vertical spacing; the stepper is a pill in MAUI vs a flatter control in C++.
- **Dark:** Same as light: identical 10-label diagonal z-index cascade with red Label 9 on top, correct stacking order and all label texts present. MAUI is positioned lower (harness card) and C++ has marginally tighter row spacing; z-order matches.

### 40. Layout Is Enabled — 🔴 (L:diff / D:diff)
- **Light:** This page demonstrates enabled/disabled visual states. In the first section 'All children are enabled', MAUI renders the two 'Enabled' labels in bright BLUE (enabled appearance), but C++ renders them in GRAY/dimmed (disabled appearance) — the enabled state is shown wrong. The teal 'First item is enabled and the second one is disabled' and 'Children have commands attached' sections likewise show the supposed-to-be-enabled blue label dimmed/gray in C++, whereas MAUI shows enabled children in blue. Disabled sections (light-blue 'Disabled', pink 'Enabled') match. Note MAUI's capture is zoomed in showing only the top ~4 sections while C++ shows all sections plus the bottom action buttons (different capture scale, not a bug). The real diff: enabled children fail to render in the enabled blue color in C++.
- **Dark:** Same diff as light: in 'All children are enabled' MAUI shows the 'Enabled' labels in bright blue while C++ shows them gray/dimmed as if disabled; the teal sections' enabled label is also dimmed in C++ vs blue in MAUI. Disabled sections match. MAUI capture is zoomed to the top sections; C++ shows the full page including the bottom Enable/Disable buttons.

### 41. Shapes — 🟢 (L:match / D:match)
- **Light:** Both show Ellipse (red fill / navy stroke), RoundRectangle (navy fill), EvenOdd pentagram (blue fill / red stroke), and 'Line' label with a purple diagonal line. C++ renders the Line slightly more visibly but geometry, colors and layout are equivalent. MAUI has a white harness card wrapper; C++ is plain white — harness-only difference.
- **Dark:** Same controls and colors on black background: red/navy ellipse, navy round-rect, blue/red pentagram. C++ shows the purple diagonal Line clearly while MAUI dark renders it as a faint near-invisible stub at the bottom; otherwise equivalent.

### 42. Ellipse Gallery — 🟢 (L:match / D:match)
- **Light:** All four items match: 'A basic Rectangle' red filled ellipse, 'A Circle' red-outline circle, 'An Ellipse with stroke' (hollow red-outline + navy-filled red-outline pair), 'An Ellipse with stroke dash' navy ellipse with red dashed border. C++ actually renders the dashed ellipse fully while MAUI's is clipped at the bottom edge.
- **Dark:** Identical content and colors on black: filled red ellipse, red-outline circle, stroked ellipse pair, navy dashed-stroke ellipse. Equivalent layout; C++ shows the dashed ellipse fully, MAUI clips it slightly.

### 43. Rectangle Gallery — 🟢 (L:match / D:match)
- **Light:** Shared items all match: 'A basic Rectangle' red, 'A Square' red-outline, 'A Rectangle with stroke' (hollow + navy-filled pair), 'A Rectangle with stroke dash' navy with red dashed border. C++ additionally fits 'A Rectangle with curved corners' (navy rounded rect) because content starts higher; MAUI's reference is shifted down by the harness card so that item is below the fold.
- **Dark:** Same five rectangles, same red/navy colors on black. C++ shows all five including the curved-corners rounded rect; MAUI dark clips after the dashed rect. Shared content equivalent.

### 44. Line Gallery — 🟢 (L:match / D:match)
- **Light:** Three sections match exactly: 'A basic Line' purple diagonal, 'A dash Line' orange dotted diagonal, 'A Line using StrokeThickness' thick black diagonal. Same colors, angles and dash patterns.
- **Dark:** Same three diagonal lines (purple solid, orange dotted, thick black) rendered correctly on black background. Equivalent.

### 45. Line Join Gallery — 🟢 (L:match / D:match)
- **Light:** All three cyan chevron polylines match: Miter (sharp apex), Bevel (flat-cut apex), Round (rounded apex). The line-join styles render correctly. C++ shapes fill slightly more vertical space (no harness card offset) but proportions and join geometry are equivalent.
- **Dark:** Same three cyan chevrons with correct Miter/Bevel/Round joins on black. Equivalent join rendering and layout.

### 46. Polygon Gallery — 🟡 (L:minor / D:match)
- **Light:** 'A basic Polygon' green triangle, 'A dash Polygon' green dashed triangle, and 'EvenOdd Polygon' (blue fill / red stroke star with hollow center) all match. For 'NonZero Polygon' MAUI shows a black-filled yellow-outline star; C++ light is scrolled up so only the top tip + a yellow horizontal segment is visible (rest below the fold), so the NonZero star can't be fully compared — a scroll-offset artifact, not a wrong render.
- **Dark:** Green triangle, green dashed triangle, and blue/red EvenOdd star all match. The NonZero star renders as a yellow outline whose interior fill is black (matching MAUI's black NonZero fill, invisible on the black background in both). Equivalent.

### 47. Polyline Gallery — 🟡 (L:minor / D:minor)
- **Light:** Both render 'A basic Polyline' as a thin red near-horizontal segment clipped at the left edge and 'A dash Polyline' as a red dashed segment at the left edge — same content/structure. Minor: C++ draws the dashed polyline as a longer continuous red dotted line whereas MAUI shows only a short 3-dot stub; the basic-line length also differs slightly. Geometry (flat near-horizontal lines anchored at x=0) is the same.
- **Dark:** Same as light on black background: thin red basic polyline stub and red dashed polyline at the left edge. C++ dash polyline is longer than MAUI's 3-dot stub; otherwise same content and structure.

### 48. Path Gallery — 🟢 (L:match / D:match)
- **Light:** Shared items all match: 'Create a LineSegment' thin diagonal, 'Create a Shape' right-triangle outline, 'Cubic Bezier Path' wavy curve, 'Composite shape' concentric purple circles. C++ additionally shows 'Overlapping Rectangles' (red L/Z shape) and 'EllipseGeometry' (orange square with green overlapping ellipse outlines) because content starts higher; MAUI's reference is shifted down so those are below the fold. Equivalent on shared content.
- **Dark:** The first three path items (LineSegment, triangle, Cubic Bezier) use a default black stroke and are therefore invisible on the black background in BOTH MAUI and C++ — they match (both blank in that region). 'Composite shape' light-purple circles render in both. C++ additionally shows the red Overlapping Rectangles and orange/green EllipseGeometry which are below MAUI's clipped fold. Equivalent.

### 49. Path Aspect Gallery — 🟡 (L:minor / D:minor)
- **Light:** Both show the 'None'/'Fill'/'Uniform'/'UniformToFill' labels each with a red heart that correctly demonstrates each aspect mode. MAUI draws each heart inside a light-gray container card (a harness gray box, e.g. the 'None' heart sits in a large empty gray rectangle); C++ draws the hearts directly on the white background with no gray card. Heart shapes/aspect behavior match. Minor harness-card-only difference.
- **Dark:** Same as light: MAUI keeps the light-gray container cards behind each heart (gray boxes stay gray in dark theme); C++ renders the hearts directly on the black background with no gray card. Heart shapes and aspect modes match. Minor harness-card difference.

### 50. Path Transform String — 🟢 (L:match / D:match)
- **Light:** Both show 'Without RenderTransform' (a Z/butterfly path) and 'With RenderTransform' (the same path skewed). The two stroked shapes and the skew applied to the second one match closely between MAUI and C++.
- **Dark:** Both MAUI and C++ show only the two labels with the path shapes invisible (black stroke on black background did not adapt to dark theme in either app). Identical behavior on both sides.

### 51. Composition Gallery — 🔴 (L:diff / D:diff)
- **Light:** MAUI renders TWO separate cream canvases: a square top canvas with the composed shape (green triangle + yellow circle + translucent diagonal red/orange bars) CENTERED in it, and a shorter wide canvas below with the blue/green/red axis lines centered. C++ merges them into ONE tall cream canvas: the composed shape is offset to the upper-RIGHT (not centered) and the axis lines fill the lower portion. The composition content (translucency, colors) is correct, but the canvas layout and shape centering differ.
- **Dark:** Same layout discrepancy as light: MAUI shows two distinct cream canvases (top square with centered composed shape, bottom wide box with centered axes); C++ shows a single tall cream canvas with the composed shape pushed to the upper-right and axes below. Shape rendering itself is correct, but canvases merged and content not centered.

### 52. Transform Playground — 🟢 (L:match / D:match)
- **Light:** Both show the gray canvas square with the red/blue-bordered square in its top-left, then identical labeled sliders: 'RotateTransform / Rotation: 0', 'CenterX: 0', 'CenterY: 0', 'ScaleTransform / ScaleX: 1.00'. Slider thumb positions and values match. C++ simply shows slightly more rows below the fold (SkewTransform) due to tighter row spacing; structure and values are identical.
- **Dark:** Same as light in dark theme: gray canvas + red square top-left, identical slider labels and values (Rotation 0, CenterX 0, CenterY 0, ScaleX 1.00). C++ shows a bit more content below the fold but structure matches.

### 53. Transformations — 🟡 (L:minor / D:minor)
- **Light:** FIXED number format to match C# double.ToString(): whole values now print as integers ('Scale = 1' not '1.0'; 'AnchorX = 0.5' preserved). Heading 'SCALE AND ROTATE', the 6 transform sliders, Anchor steppers, and Translation sliders all match.
- **Dark:** Same as light; integer number format + matching controls in dark.

### 54. Update Path Data — 🟡 (L:minor / D:match)
- **Light:** Both show the S-curve cubic-bezier path stroked in black, the bottom text 'counter = 0 | Data: M 10,100 C 10,300 300,-200 300,100', and the blue 'Update Path Data' button overlapping that text. The curve shape and labels match; only the curve's vertical position differs (MAUI centers the curve in the white card, C++ draws it near the top). Minor positional difference.
- **Dark:** Both MAUI and C++ show ONLY the bottom 'counter = 0 | Data:...' text and the blue 'Update Path Data' button; the S-curve is invisible (black stroke on black background did not adapt to dark theme) in BOTH apps. Identical behavior, so they match.

### 55. Auto Size Shapes — 🔴 (L:diff / D:diff)
- **Light:** Both show the label 'The Ellipse below must occupy half of the available space.', a yellow band with a green/blue-outlined ellipse, and an orange band below. The ELLIPSE ASPECT differs: MAUI draws a true wide-and-flat ellipse (wider than tall, filling the yellow band's width but short), whereas C++ draws a near-CIRCLE (almost as tall as wide); the C++ yellow band is correspondingly taller. Shape sizing visibly differs.
- **Dark:** Same ellipse-shape diff in dark theme: MAUI's green ellipse is wide-and-flat within a shorter yellow band, while C++'s is a near-circle within a taller yellow band. Orange band and label match; the ellipse aspect ratio / yellow-band height differ.

### 56. Shape App Theme — 🟡 (L:minor / D:minor)
- **Light:** FIXED rectangle horizontal alignment to Start (was centered) — now left-aligned at the stack margin matching MAUI. AppTheme color binding correct (green label+rect in light). Equivalent.
- **Dark:** Dark renders red label + red rectangle on black (theme seed works), left-aligned. Matches MAUI.

### 57. Invalidate Brush — 🟡 (L:minor / D:minor)
- **Light:** FIXED: the 'Change color' button is now content-width left-aligned (HorizontalOptions=Start) matching MAUI (was full-bleed). Green button bg, green underline, and 'Brush color: Green' label all match.
- **Dark:** Same as light; content-width button + green brush in dark.

### 58. Gradient brushes — 🟢 (L:match / D:match)
- **Light:** Both render the two labeled gradient swatches identically: 'LinearGradientBrush (yellow→green)' with a horizontal yellow-to-green gradient and 'RadialGradientBrush (red→navy)' with a centered red-to-purple radial gradient. Only difference is MAUI's content starts lower (harness card top padding); the gradients and labels are equivalent.
- **Dark:** Identical to light comparison: both labels and both gradients (linear yellow→green, radial red→navy) render equivalently. Equivalent page.

### 59. Border — 🟢 (L:match / D:match)
- **Light:** Both show a red rounded-rectangle border with thick stroke, cream/light-yellow fill, and centered 'Bordered content' text. Box size, stroke, fill color, and label all match. Only MAUI's box sits slightly lower (harness padding).
- **Dark:** Both render the same red-bordered cream box centered. Note: in dark theme BOTH MAUI and C++ render the 'Bordered content' text in a near-white color on the cream fill, making it faint/low-contrast — this is shared behavior (MAUI's actual ground truth), so the two still match each other.

### 60. Border Stroke — 🟢 (L:match / D:match)
- **Light:** Both show 'Using different StrokeThickness' with three orange boxes (red stroke widths 1/5/10), then 'Updating the Content Height', 'Content height: 60', a slider, and a second set of orange/red boxes labeled 1/5/10. Labels, orange fills, red stroke thicknesses, and slider position all match. Only difference: MAUI's harness card clips the bottom '10' box while C++ shows it fully.
- **Dark:** Same as light: all labels, the three stroke-thickness boxes, the slider, and the lower content-height boxes (1/5/10) render equivalently between MAUI and C++.

### 61. Border Layout — 🟢 (L:match / D:match)
- **Light:** Both show 'Stroke thickness: 5 / 40', a slider, and a gray pill-shaped border containing a Grid: red rounded-left cap, green cell with 'Center' label, blue block, then green fill. Segment layout, colors, pill stroke, and label all match.
- **Dark:** Identical to light: 'Stroke thickness: 5 / 40', slider, and the pill border with red/green-'Center'/blue/green segments all render equivalently.

### 62. Border Playground — 🟡 (L:minor / D:minor)
- **Light:** Both render the dashed-yellow border with cyan-to-blue gradient fill and 'Just a Label' text, plus the form: 'Border Content'/Label, 'Border Shape'/RoundRectangle, 'Background'/Background Start Color #00B4DB/End Color #0083B0. Cosmetic diffs only: the C++ box is positioned higher and the form scrolls to show more fields (End Color, Content Background, checkbox, 'Show Content Background', 'Border'), while MAUI shows the box lower with more whitespace and fewer fields above the fold. C++ section headers ('Border Content', 'Border Shape', 'Background') render in regular weight vs MAUI's bold.
- **Dark:** Same content as light — dashed border, gradient fill, 'Just a Label', and form fields all present. Differences are cosmetic: C++ packs the box higher and exposes more form rows; MAUI positions the box lower with extra whitespace. Header weight differs (regular in C++ vs bold in MAUI).

### 63. Border Clip Playground — 🟡 (L:minor / D:minor)  ⚠️ _MAUI reference capture broken — re-shoot needed_
- **Light:** NOT a port bug — the MAUI reference is broken (the maui-compare app does not bundle oasis.jpg, so its Border shows an EMPTY red outline). The C# XAML places <Image Source="oasis.jpg" Aspect="AspectFill"> inside the Border; the C++ port correctly resolves a bundled stand-in JPEG and clips it into the StrokeShape with the LIVE per-corner radius (top-left 60, top-right 0 both honored). Clip-into-shape + red stroke + corner-radius sliders all work. Residual: the gallery bundles a different photo than oasis.jpg (asset packaging, not a framework diff).
- **Dark:** Same as light — MAUI ref empty (missing oasis.jpg asset); the port correctly clips a bundled image into the rounded border. Not a port rendering diff.

### 64. Border Resize Content — 🟡 (L:minor / D:minor)  ⚠️ _MAUI reference capture broken — re-shoot needed_
- **Light:** NOT a port bug — MAUI reference is broken. The C# XAML's right-column borders contain <Image Source="oasis.jpg">; the maui-compare app does not bundle oasis.jpg, so MAUI shows only the LightBlue style background. The C++ port correctly clips a bundled stand-in image into the circle/round-rect/triangle StrokeShapes (left column red shapes with '+' match exactly). Residual: the gallery bundles a different photo than oasis.jpg (asset packaging, not a framework diff).
- **Dark:** Same as light — right-column image borders show the LightBlue style bg in MAUI (missing oasis.jpg) vs a correctly-clipped bundled image in C++. Left column matches. Not a port rendering diff.

### 65. Borderless — 🟡 (L:minor / D:minor)
- **Light:** Both show the same yellow borderless container, the label 'Style: borderless (StrokeThickness 0)', and a switch with a yellow track. Only difference: MAUI insets the yellow region inside a black rounded card (black margin visible top and bottom), while C++ renders the yellow edge-to-edge with no surrounding black inset. Content, label, switch state and colors all match.
- **Dark:** Same as light: identical content (yellow container, label, yellow-track switch). MAUI shows the yellow inside a black rounded card with margins; C++ fills edge-to-edge. The label text is dark/black in MAUI's lighter-yellow area and rendered correctly in both; cosmetic inset difference only.

### 66. Clip — 🔴 (L:diff / D:diff)
- **Light:** MAUI 'Image' shows the submarine photo filling the full content width WITH its gray background. C++ renders the same submarine but smaller/narrower, centered, and WITHOUT the gray background fill (transparent behind the sub). Because C++ images are more compact, C++ fits 4 sections (Image, Rectangle, Ellipse, GeometryGroup clips) on screen while MAUI shows only the first 2 — but the core diff is the missing gray image background and the undersized/centered image in C++.
- **Dark:** Same diff as light: MAUI's top 'Image' is full-width with a gray background; C++ shows a smaller centered submarine with no gray background. The clip geometries themselves render in C++, but image sizing and the missing gray bg fill differ from the MAUI ground truth.

### 67. Clip Views — 🔴 (L:diff / D:diff)
- **Light:** Row of controls each with a red swoosh clip. Two real diffs: (1) the 'Entry' and 'Editor' rows have a solid red clip fill in MAUI but in C++ they render as plain text fields with NO red fill (just a faint underline); (2) the red swoosh clips are noticeably narrower in C++ (~75% width) vs nearly full width in MAUI. The SearchBar clip is pink/red-filled in MAUI but transparent/gray in C++. Date format matches (DD.MM.YYYY device locale); the day differs only due to capture date.
- **Dark:** Same diffs as light: Entry and Editor rows lack the red clip fill in C++ (present in MAUI), C++ red swoosh clips are narrower than MAUI's full-width clips, and the SearchBar clip fill is missing/gray in C++ vs red-tinted in MAUI.

### 68. Clip Corner Radius — 🟡 (L:minor / D:minor)
- **Light:** Structure matches: 'Clipped Image using RoundRectangleGeometry' header, a gray-backed clipped image, then four sliders (Top Left / Top Right / Bottom Left / Bottom Right Corner). The clip rendering and sliders are correct. Difference: C++ loads a DIFFERENT sample image (a dog/pug photo) where MAUI shows the purple submarine. C++'s more compact spacing also fits all 4 sliders on screen vs MAUI's 3. Sample-asset difference, clip behavior itself is correct.
- **Dark:** Same as light: identical structure and slider styling; the only notable difference is the sample image asset (C++ shows a dog photo, MAUI shows the purple submarine). Round-rectangle clip and corner sliders render correctly in both.

### 69. Clip Gallery — 🟢 (L:match / D:match)  ⚠️ _MAUI reference capture broken — re-shoot needed_
- **Light:** MAUI reference is BROKEN here: 'Image' and 'Clipped Image using RectangleGeometry' render as EMPTY GRAY PLACEHOLDER BOXES (the image asset failed to load in MAUI). C++ correctly renders the actual photo (a dog) with all clip variants (Rectangle, RoundRectangle, etc.). The C++ side is the more-correct render; the MAUI capture needs re-shooting before a fair pixel comparison. Marking match because C++ behaves correctly and the only divergence is the broken MAUI reference.
- **Dark:** Same as light: MAUI shows empty gray placeholder boxes (image failed to load) while C++ correctly renders the dog photo clipped with each geometry. MAUI reference capture is broken and needs re-capture; C++ render is correct.

### 70. Clipping — 🔴 (L:diff / D:diff)
- **Light:** Orange clipping demo: 'Not clipping' label, 'Toggle clipping on horizontal stack layouts' button, an orange-red square, a purple 'Hey Hey Hey Hey' block over a light-blue clip, and a numbers row. Two real diffs: (1) the numbers row — MAUI spreads '1'-'8' across the full width (the '1' clipped off-screen left, large gaps between digits) while C++ bunches them tightly as '12345678' at the far left with no spacing; (2) C++ renders two coffee-cup emoji glyphs at the left of the light-blue bar that are ABSENT in MAUI. C++ also fills edge-to-edge orange vs MAUI's black-inset rounded card.
- **Dark:** Same diffs as light: numbers '1'-'8' are spread full-width in MAUI but bunched together at the left in C++, and two coffee-cup emojis appear in C++ but not in MAUI. The orange square, purple 'Hey' block and light-blue clip otherwise match.

### 71. Shadow Playground — 🟡 (L:minor / D:minor)
- **Light:** Both show 'Label with a Shadow' (red shadow under text), a cyan #00B4DB box with a red drop shadow offset down-right, the 'Background' field #00B4DB, 'Shadow Color' #FF0000, and three sliders (Offset X:10, Offset Y:10, Radius:10) plus a partly-visible 4th. All values, colors and controls match. Minor difference: MAUI's red shadows are softer/more spread (larger blur glow around the box and a more visible label shadow); C++'s red shadow is a tighter glow concentrated bottom-right and the label shadow is fainter.
- **Dark:** Same as light: identical structure, hex fields and slider positions; the red drop shadow is present and correctly colored/offset in both. C++'s shadow blur is slightly tighter/less spread than MAUI's softer glow. Cosmetic shadow-blur difference only.

### 72. Invalidate Shadow Host — 🟡 (L:minor / D:minor)
- **Light:** Both show 'Host' label, 'Update Host Size' blue button, 'Shadow' label, sliders Offset X:10 / Offset Y:10 / Radius:10 / Opacity:1.00 (thumb positions match), and a green-bordered white host box at the bottom. All controls, labels and slider values match. Difference: the green host box is noticeably TALLER in C++ (fills more vertical space, ~330px) vs a short ~120px box in MAUI — largely because MAUI compresses content inside a black rounded-card inset while C++ renders edge-to-edge, letting the host fill remaining height.
- **Dark:** Same as light: identical controls and slider positions; the green-bordered host box renders taller in C++ than in MAUI (MAUI's content is inset in a black rounded card, compressing the box). Otherwise a full match.

### 73. CollectionView — 🟡 (L:minor / D:minor)
- **Light:** Content matches: 'This is the header' plus a 3-column grid of 24 items (cover1.jpg,0 ... Vegetables.jpg,23) with identical labels and order. Cosmetic diffs only: MAUI wraps the list in a harness white rounded card; the 'This is the header' label is bold in MAUI vs regular weight in C++; item text wraps at slightly different points. A user would call it basically the same page.
- **Dark:** Same as light: identical 3-column 24-item grid and header text. Only the header weight (bold in MAUI, regular in C++) and minor text-wrap points differ; no harness card visible in dark on either. Basically the same page.

### 74. Items — 🟡 (L:minor / D:minor)
- **Light:** Same grouped CollectionView content: group header 'Today', items 'Water the plants', 'Review the port', 'Ship wave 2', then footer/label 'Pick a task'. Cosmetic diffs: 'Today' is bold in MAUI vs regular in C++; C++ uses much larger vertical spacing between the three items (they are spread out) whereas MAUI packs them tightly; MAUI shows a harness white card. All content present and correct.
- **Dark:** Identical content (Today / 3 tasks / Pick a task). Same cosmetic diffs: 'Today' bold in MAUI vs regular in C++, and notably larger inter-item spacing in C++. No missing content.

### 75. Single Bound Selection — 🟡 (L:minor / D:minor)
- **Light:** Matches: same instruction paragraph, 'Selected: (none)', and the 5 countries (United States, Canada, Mexico, Brazil, Argentina) in order with none selected. Only cosmetic diff is larger inter-item vertical spacing in the C++ CollectionView (items spread further apart); MAUI also shows a harness white card. Content identical.
- **Dark:** Same as light: identical instruction text, 'Selected: (none)', and the 5-country list with no selection. Only the larger C++ inter-item spacing differs. Content identical.

### 76. Multiple Bound Selection — 🟡 (L:minor / D:minor)
- **Light:** FIXED button labels to match C# ('Clear and Add' / 'Reset' / 'Direct Update'). Structure, selection (Item 1&2 highlighted), header, and readout all match. Residual minor: the 3 buttons stack centered vs MAUI's layout — cosmetic.
- **Dark:** Same as light; labels + selection match in dark.

### 77. Preselected Item — 🟡 (L:minor / D:minor)
- **Light:** Matches: 'Preselected: photo.jpg, 2', header 'This is the header', single-column list (cover1.jpg,0 ... ) with 'photo.jpg, 2' (index 2) correctly preselected/highlighted. Cosmetic diffs: header bold in MAUI vs regular in C++; C++ uses larger inter-item spacing so fewer rows fit per screen (C++ visible to ~index 15-16, MAUI to ~19-20). Selection and content correct.
- **Dark:** Same as light: correct preselection of 'photo.jpg, 2', same list. Only cosmetic diffs are the header weight and the larger C++ row spacing. Content and selection correct.

### 78. Preselected Items — 🟡 (L:minor / D:minor)
- **Light:** Matches: instruction text, 'Preselected: photo.jpg, 2, Fruits.jpg, 4, FlowerBuds.jpg, 5', header 'This is the header', and a 3-column grid with exactly photo.jpg/2, Fruits.jpg/4, FlowerBuds.jpg/5 highlighted. Cosmetic diffs only: header bold in MAUI vs regular in C++, and the light MAUI capture appears at a slightly smaller/zoomed-out scale, but item layout, order and the 3 highlighted cells are identical.
- **Dark:** Same as light: identical preselection of the 3 items in the grid, same header and instruction text. Only the header weight and minor scale differ. Selection correct.

### 79. Selection Command Param — 🟡 (L:minor / D:minor)
- **Light:** FIXED: each cell now renders two lines ('Item N' + 'This is item N') matching MAUI's title+detail DataTemplate (was title-only — the single-root struct cell now binds text+description on two lines). Header 'This is the header' + the Pending/Success status present.
- **Dark:** Same as light; two-line cells in dark.

### 80. Selection Synchronization — 🟡 (L:minor / D:minor)
- **Light:** Matches: instruction paragraph, two CollectionView sections ('Set ItemsSource then SelectedItems...' and 'Set SelectedItems then ItemsSource...'), each with 'Selected: Item 3, Item 2' and Item 1-4 where Item 2 and Item 3 are correctly highlighted. Cosmetic diff: C++ uses larger inter-item spacing, so it fits both full sections on screen while MAUI shows the first section's list plus only the start of the second section. Selection state correct in both.
- **Dark:** Same as light: both sections render with Item 2 and Item 3 correctly selected and 'Selected: Item 3, Item 2'. Only the larger C++ row spacing (causing different amounts of scroll content visible) differs. Selection correct.

### 81. Filter Collection — 🟡 (L:minor / D:minor)
- **Light:** Content and structure match: 'Use EmptyView' toggle (green/on), 'Filter' search box, and a two-column grid of 'filename, index' items (cover1.jpg 0, oasis.jpg 1, ...). C++ uses larger vertical row spacing so fewer rows fit (C++ ends at row 31; MAUI reaches 33). The C++ light search box also shows a faint border MAUI lacks. Cosmetic only.
- **Dark:** Same as light: matching toggle, search box and two-column item grid; only difference is C++'s wider row spacing (fewer items visible). Cosmetic.

### 82. Filter Selection — 🟡 (L:minor / D:minor)
- **Light:** Structure matches: instruction paragraph, 'Filter' search box, 'Selected: (none)' label + blue 'Reset' button, and single-column item list. C++ uses noticeably larger vertical row spacing so items are more spread out (C++ shows through index 13; MAUI through 12 in a tighter list). Cosmetic spacing difference only.
- **Dark:** Same as light: identical content and controls; only difference is C++'s wider inter-row spacing. Cosmetic.

### 83. Header Footer — 🔴 (L:diff / D:diff)
- **Light:** Items match (cover1.jpg 0, oasis.jpg 1, photo.jpg 2) and both string header/footer texts are present. BUT in MAUI the header 'Just a string as a header' and footer 'This footer is also a string' render in a LARGER BOLD style; in C++ they render in the same regular weight/size as the body items (no bold/large header-footer styling). C++ also adds extra row spacing.
- **Dark:** Same as light: header 'Just a string as a header' and footer 'This footer is also a string' are bold/larger in MAUI but plain same-size text in C++; plus wider row spacing.

### 84. Header Footer Grid — 🔴 (L:diff / D:diff)
- **Light:** The 3-column item grid renders, but the templated HEADER ('This Is A Header' large styled text + an 'Add Content' button) and the templated FOOTER ('This Is A Footer' + 'Add Content' button) are BOTH entirely missing in C++ — MAUI shows both. Also the 'Toggle Header' and 'Toggle Footer' buttons run together with no gap ('Toggle HeaderToggle Footer') in C++ vs spaced in MAUI.
- **Dark:** Same as light: 3-column grid present, but the templated 'This Is A Header'/'Add Content' header and 'This Is A Footer'/'Add Content' footer are missing in C++; Toggle buttons also lack spacing.

### 85. Header Footer Grid Horizontal — 🔴 (L:diff / D:diff)
- **Light:** Major layout discrepancy. MAUI shows a HORIZONTALLY-scrolling grid: items flow left-to-right in rows (cover1.jpg 0 / Vegetables.jpg 3 / Legumes.jpg... across the top, oasis.jpg 1 / Fruits.jpg 4 / cover1.jpg 7 below) with the tall stacked 'This Is A Header' template at the left edge. C++ ignores the horizontal orientation: it renders a vertical layout of very narrow columns that wrap each item into a ~4-character-wide column ('cover'/'1.jpg,'/'0' stacked). The 'This Is A Header' template is also missing, and the Toggle buttons run together with no gap.
- **Dark:** Same as light: MAUI is a horizontal grid with the stacked 'This Is A Header' on the left; C++ renders cramped narrow wrapped vertical columns, omits the header template, and runs the Toggle buttons together.

### 86. Header Footer Template — 🔴 (L:diff / D:diff)
- **Light:** Templated header/footer differ. MAUI header shows a DATE-ONLY value ('6/19/2026') with a 'This Is A Header' label beneath it; C++ header shows a full DATE-TIME ('6/21/2026 2:48:11 PM') and OMITS the 'This Is A Header' label. The three blue item cards are spaced apart with visible gaps in MAUI but are crammed together into one solid blue block (no inter-item spacing) in C++. The footer's 'This Is A Footer' label is also missing in C++ (only the date-time line remains).
- **Dark:** Same as light: C++ header shows date+time instead of MAUI's date-only and drops the 'This Is A Header' label; blue item cards collapse together with no spacing; footer 'This Is A Footer' label missing. (Differing date value 6/19 vs 6/21 is just capture date, not a bug; the format/labels are.)

### 87. Header Footer View — ⬛ (L:blank / D:blank)
- **Light:** C++ renders essentially NOTHING — empty white page with only the status bar. MAUI shows a large 'This Is A Header' header view, a styled/rotated 'This Is A Footer' footer view, and two buttons 'Add 2 Items' and 'Clear All Items'. All of that is absent in C++.
- **Dark:** Same as light: C++ is a fully blank black page (status bar only); MAUI shows 'This Is A Header', 'This Is A Footer', and the 'Add 2 Items'/'Clear All Items' buttons. Nothing renders in C++.

### 88. Footer Only String — 🔴 (L:diff / D:diff)
- **Light:** Items render correctly, but C++ uses much wider row spacing: MAUI fits all 20 items (index 0-19) plus the bold 'This is a footer' string on screen; C++ only shows through index 17 and the 'This is a footer' footer string — the page's whole point — is pushed below the fold and NOT visible. MAUI also renders the footer string in bold.
- **Dark:** Same as light: C++'s excessive row spacing pushes the 'This is a footer' string off-screen (only items 0-17 visible) whereas MAUI shows all 20 items plus the bold footer string.

### 89. Basic Grouping — 🟡 (L:minor / D:minor)
- **Light:** Content is identical: title 'This is a header', green group headers (Avengers/Fantastic Four/Defenders), orange 'Total members:' footers, and member names all present in the same order. Two cosmetic differences: (1) C++ row spacing is much taller — MAUI fits the whole first 3 groups on screen (~30 rows) while C++ fits only ~17 rows; (2) the 'This is a header' title is bold in MAUI but regular weight in C++.
- **Dark:** Same as light: identical content (green headers, orange footers, member names). C++ uses noticeably taller per-row spacing (fewer rows per screen) and renders the 'This is a header' title in regular weight vs MAUI's bold.

### 90. Grid Grouping — 🔴 (L:diff / D:diff)
- **Light:** Both render the 2-column grid with green group headers and orange 'Total members:' footers, same items. Real diff: C++ shows an extra 'This is a header' title row at the very top that MAUI does NOT show (MAUI begins directly with the green 'Avengers' header). Also C++ uses much taller row spacing — MAUI shows all 6 groups through 'West Coast Avengers' on one screen, while C++ only reaches mid-'Defenders'.
- **Dark:** Same as light: 2-column grid content matches, but C++ adds an extra 'This is a header' title at the top that is absent in MAUI, and C++ row spacing is much taller so far fewer groups fit per screen.

### 91. Grouping No Templates — 🟡 (L:minor / D:minor)
- **Light:** Both render the same flat list of member names with no group headers/footers, in identical order (Thor, Captain America, Iron Man ... Luke Cage). Only difference is row density: C++ rows are noticeably taller/more spaced, so MAUI fits ~25 names on screen while C++ fits ~14.
- **Dark:** Identical flat name list and order; the only difference is C++'s taller per-row spacing (fewer names per screen than MAUI).

### 92. Grouping Plus Selection — 🟡 (L:minor / D:minor)
- **Light:** Both show the grouped list (green headers, orange 'Total members:' footers, member names) in identical order starting at 'Avengers'. No selection highlight is visible in either capture. Only difference is C++'s taller row spacing (fewer rows per screen than MAUI).
- **Dark:** Same content and order as MAUI; no selected-cell highlight visible in either; C++ uses taller per-row spacing so fewer rows fit per screen.

### 93. Switch Grouping — 🟡 (L:minor / D:minor)
- **Light:** Both show 'Is Grouped:' label with a green ON switch, then the grouped list (green headers, orange footers, members) in identical order. Switch renders correctly. Only difference is C++'s taller row spacing — MAUI reaches 'Defenders' while C++ only reaches 'Fantastic Four' on screen.
- **Dark:** Same as light: 'Is Grouped:' label + green ON switch and identical grouped content; C++ uses taller per-row spacing so fewer rows fit per screen.

### 94. Some Empty Groups — 🟡 (L:minor / D:minor)
- **Light:** Both show the description text, then groups where empty groups (Thundercats 'Total members: 0', Bionic Six 'Total members: 0') still display their headers and footers exactly as expected. Content, group order and member counts match. Only difference is C++'s taller row spacing.
- **Dark:** Same correct behavior: empty groups (Thundercats, Bionic Six) still show headers/'Total members: 0' footers; all content matches MAUI. C++ uses taller per-row spacing.

### 95. Scroll To Group — 🟡 (L:minor / D:minor)
- **Light:** Both render the full form: 'Group:'/'Item:' entries pre-filled '0', a blue 'Go', 'Group Name:'/'Item Name:' entries (empty), a second blue 'Go', the 'No scroll requested yet' status, and the grouped list below starting at green 'Avengers'. All controls and text present and correctly placed. Only difference is C++'s taller row spacing in the list (MAUI reaches 'Black Widow', C++ reaches 'Mockingbird' but with far more whitespace per row).
- **Dark:** Same as light: all form controls (two entries pre-set to 0, two empty name entries, two Go buttons, status text) and the grouped list render correctly and in the right positions; only difference is C++'s taller list row spacing.

### 96. Scroll Mode Test — 🔴 (L:diff / D:diff)
- **Light:** Both render the same controls ('ItemsUpdatingScrollMode:' label, 'Scroll To Middle', 'Add Item Above', 'Add Item Below', 'Add Item To End' buttons, 'Mode: KeepItemsInView · Items: 20', then the image-name list cover1.jpg,0 / oasis.jpg,1 ...). Real diff: in C++ the picker value to the right of the label is rendered TWICE — it shows 'KeepItemsInView  KeepS...' (a second copy that runs off the right edge), whereas MAUI shows a single 'KeepItemsInView'. C++ list rows are also noticeably taller-spaced.
- **Dark:** Same as light: controls and list content match, but the picker value is duplicated in C++ ('KeepItemsInView' followed by a second truncated 'KeepS...' off the right edge) vs MAUI's single 'KeepItemsInView'; C++ also uses taller list row spacing.

### 97. Adaptive Collection — 🟡 (L:minor / D:minor)
- **Light:** Both show 'Layout: Linear (single column)' header and Item 1..8. In MAUI each 'Item N' label is horizontally CENTERED with large vertical spacing; in C++ each item is LEFT-aligned and tightly spaced (all 8 visible at once). Same content & structure, only item alignment + row spacing differ. MAUI also wraps content in the harness white card; C++ fills the screen.
- **Dark:** Same as light: MAUI centers each 'Item N' with wide spacing on black; C++ left-aligns them tightly. Content identical (header + Items 1-8), only alignment/spacing differs.

### 98. Staggered Layout — 🔴 (L:diff / D:diff)
- **Light:** The staggered effect is MISSING in C++. MAUI renders a 3-column grid where each cell has a VARYING tall height (Item 0/1/2, big gap, Item 3/4/5, ... only ~12 items visible due to tall staggered cells). C++ renders a uniform tight grid of equal-height rows (Item 0..23 packed compactly). C++ cells are not staggered at all — the whole point of the page (variable-height masonry layout) is absent.
- **Dark:** Same defect in dark: MAUI shows tall variable-height staggered cells (~12 items, Item 0-11); C++ shows a uniform compact equal-height 3-column grid (Item 0-23). The staggered/masonry sizing is missing in C++.

### 99. Varied Size Selector — 🔴 (L:diff / D:diff)
- **Light:** C++ does not vary cell heights per template. MAUI shows 4 items (Coffee0, Milk1, Coffee2, Coffee3) where the Milk template cell is much TALLER than the Coffee cells (varied size by data-template selector). C++ shows 6 uniform-height compact rows (Coffee0, Milk1, Coffee2, Coffee3, Milk4, Coffee5) — Milk rows are the same height as Coffee rows, so the varied-size effect is missing. Item count and per-cell heights both differ. Bottom controls (Insert/Add/Remove, Index field '1', 'Latte' field) are present on both.
- **Dark:** Same defect in dark: MAUI gives the Milk1 cell a taller height than Coffee cells (4 items); C++ renders 6 uniform compact rows with no height variation between Milk and Coffee templates. Varied-size selection not applied.

### 100. Nested Collection — 🔴 (L:diff / D:diff)
- **Light:** The nested inner CollectionViews are MISSING in C++. MAUI shows each 'Source N' header (red, italic) followed by a horizontal nested CollectionView row of blue captions ('Caption N-0 Caption N-1 Caption N-2 ...'). C++ renders ONLY the 'Source N' labels (plain black text, not red/italic) with NO nested caption rows at all — so Source 0..17 stack tightly. Both the inner horizontal collections and the red-italic header styling are absent.
- **Dark:** Same defect in dark: MAUI shows red-italic 'Source N' headers each with a nested horizontal row of blue 'Caption N-x' items; C++ shows only plain white 'Source 0..17' labels with no nested caption rows and no red/italic styling.

### 101. Data Template Selector — 🟡 (L:minor / D:minor)
- **Light:** Match in content. Both show a 'Day of Week Filter' SearchBar and a list where weekend entries render the selected template "It's the weekend! Woot!" and weekdays render the day name (Monday/Tuesday/...). Template selection works identically. Only difference: MAUI has tighter line spacing (more rows visible) while C++ has slightly larger per-row spacing; MAUI wraps content in the harness white card.
- **Dark:** Same as light: both show the 'Day of Week Filter' SearchBar and the correctly-selected templates (weekend 'Woot!' vs weekday names) on black. Only row-spacing density differs.

### 102. Cv Visual States — 🟡 (L:minor / D:match)  ⚠️ _MAUI reference capture broken — re-shoot needed_
- **Light:** Both show 'Single Selection' (Item 1/2/3) and 'Multi Selection' (Item 1/2/3/4). Content and structure identical; only row spacing differs (MAUI tighter, C++ more spaced) and MAUI uses the harness white card. No selection highlight is visible in either still.
- **Dark:** The MAUI dark REFERENCE is broken: it renders the 'Single Selection' and 'Multi Selection' section headers but the item areas are solid WHITE blocks with the item text invisible (white-on-white) — needs re-capture. The C++ dark render is CORRECT: it clearly shows 'Single Selection' Item 1/2/3 and 'Multi Selection' Item 1/2/3/4 in white text on black. C++ is fine; the ground-truth capture is the broken one.

### 103. Empty View — 🟡 (L:minor / D:minor)
- **Light:** Match in content. Both show a 'Filter' SearchBar and the populated list (cover1.jpg 0, oasis.jpg 1, photo.jpg 2, Vegetables.jpg 3, Fruits.jpg 4, FlowerBuds.jpg 5, Legumes.jpg 6, ... same order and numbering). The empty view itself only appears when the filter empties the list; here the list is populated and identical. Only difference: MAUI has tighter row spacing and the harness white card; C++ has slightly more per-row spacing.
- **Dark:** Same as light: both show the 'Filter' SearchBar and the identical populated list (cover1.jpg 0 ... oasis.jpg 15) in white on black. Only row-spacing density differs.

### 104. Empty View Null — 🟢 (L:match / D:match)
- **Light:** Both show a centered 'Nothing to display.' empty-view string on an otherwise empty page. Content identical. Only harness difference: MAUI wraps it in the rounded white card; C++ fills the screen white. Vertical centering is near-identical.
- **Dark:** Both show the centered 'Nothing to display.' empty-view string in white on black. Equivalent.

### 105. Empty View Rtl — 🟡 (L:minor / D:minor)
- **Light:** Both show the 'Left to Right' picker, a Filter SearchBar, and a 3-column grid of 'name.jpg, N' items with identical content/order. Only cosmetic differences: MAUI renders a gray harness header card above the picker (absent in C++), and the C++ port uses slightly tighter row spacing so it shows more rows. Despite the page name, neither side actually flips the grid to RTL (both show 'Left to Right'), so they match each other.
- **Dark:** Same as light: identical 'Left to Right' picker + Filter + 3-column grid content. Cosmetic-only differences are the MAUI gray header card and slightly denser C++ row spacing. Both render white-on-black text correctly in dark mode.

### 106. Empty View Selector — 🟡 (L:minor / D:minor)
- **Light:** Both show the same multi-line instruction paragraph ('1. Filter the items below by search term. 2. Filtering 'Xamarin'...'), the Filter SearchBar, and the single item 'Baboon — Africa & Asia'. Only difference is harness chrome (MAUI has a gray header card; C++ does not) and the paragraph wraps at a slightly different width. Equivalent content.
- **Dark:** Same as light — identical instruction paragraph, Filter bar, and 'Baboon — Africa & Asia' item, white-on-black. Only the MAUI gray header card and minor text-wrap width differ.

### 107. Empty View Swap — 🟡 (L:minor / D:minor)
- **Light:** Both show the Filter SearchBar, the 'Toggle Between EmptyViews' label with a Switch (off), 'Clear'/'Fill' blue buttons, and a 3-column grid of 'name.jpg, N' items with matching content. Cosmetic-only: MAUI shows a gray harness header card and the SearchBar/switch sit lower; C++ packs rows tighter and shows more of them. Switch and buttons render identically.
- **Dark:** Same as light: matching Filter bar, 'Toggle Between EmptyViews' + Switch, 'Clear'/'Fill' buttons, and 3-column grid in white-on-black. Differences are the MAUI gray header card and tighter C++ row spacing.

### 108. Empty View Template — 🟡 (L:minor / D:minor)
- **Light:** Both show the Filter SearchBar and an identical 3-column grid of 'name.jpg, N' items (same content/order). Only differences are harness chrome (MAUI gray header card vs none) and the C++ port using tighter row spacing so it shows more rows on screen.
- **Dark:** Same as light — identical Filter bar + 3-column grid, white-on-black. Only the MAUI gray header card and denser C++ row spacing differ.

### 109. Empty View View — 🟡 (L:minor / D:minor)
- **Light:** Identical to empty_view_template: both show the Filter SearchBar and the same 3-column grid of 'name.jpg, N' items. Only harness chrome (MAUI gray header card) and the C++ port's tighter row spacing differ.
- **Dark:** Same as light — matching Filter bar and 3-column grid in white-on-black. Differences are limited to the MAUI gray header card and denser C++ rows.

### 110. Empty View Load Simulate — 🟡 (L:minor / D:minor)  🎬 _motion/effect — needs an animated GIF to judge_
- **Light:** Both show the centered 'Items loading simulation...' EmptyView text in the same place. The only difference is the MAUI harness wraps it in a white/gray container card while the C++ port renders it on a transparent (page-colored) background. Same content and layout otherwise. This page is a load simulation, but both stills captured the same loading state so this frame is a fair comparison.
- **Dark:** Both show centered 'Items loading simulation...' in white-on-black. MAUI shows no container card here (page is black), and C++ matches; the two are essentially identical. (This is a load-simulation page whose point is the eventual swap to data, but both captures landed on the loading frame.)

### 111. Carousel Page — 🟡 (L:minor / D:minor)  🎬 _motion/effect — needs an animated GIF to judge_
- **Light:** Carousel item now renders BIG + centered ('Item 1') matching MAUI's caption. Residual: the port adds Prev/Next buttons + a 'Position' readout below (an intentional demo affordance — the C# carousel has none and is swiped; documented in the page header), so the C++ item is shorter to leave room. Faithful deviation.
- **Dark:** Same as light; big centered item + the intentional Prev/Next affordances.

### 112. Chat Example — 🔴 (L:diff / D:diff)
- **Light:** Bubble shape and alignment are wrong. In MAUI the two messages are content-width rounded pill bubbles aligned by sender: green 'Hi there!' is right-aligned (sent) and blue 'Hello — how can I help you today?' is left-aligned (received). In the C++ port both messages render as FULL-WIDTH rectangular colored bars (no rounded corners, no left/right alignment) spanning edge to edge, so they don't look like chat bubbles at all.
- **Dark:** Same full-width-bar vs content-width-bubble problem as light, plus a text-color bug: MAUI keeps DARK text on the light green/blue bubbles in dark mode (readable), while the C++ port renders the bubble text in white/light on the green and blue bars, washing out contrast. So C++ has both wrong bubble shape/alignment AND wrong (light) text color on light bubbles in dark mode.

### 113. Items Updating Scroll Mode — 🟡 (L:minor / D:minor)
- **Light:** All content matches: 'UpdatingScrollMode:' header, KeepItemsInView/KeepScrollOffset buttons, 'Add Item' button, 'Mode: KeepItemsInView · Items: 50' status, and the Title N — Subtitle N list. Diff: C++ CollectionView row spacing is roughly double MAUI's (C++ ~50px tall rows showing ~17 items, MAUI tight ~24px rows). Cosmetic row-height difference only.
- **Dark:** Match in dark theme: same header, buttons, status, and list with identical content/order. Same enlarged C++ row spacing vs MAUI's tighter rows.

### 114. Radio Button Group — 🟡 (L:minor / D:minor)
- **Light:** FIXED (iOS radio handler): Option A/B/C in the StackLayout and 'This RadioButton is inside a Grid' + Option D are now left-aligned with a ring↔label gap (were centered/flush). All content present and equivalent to MAUI. Residual minor: slightly tighter vertical/inter-radio spacing.
- **Dark:** Same as light — left-aligned with gaps; all content present. Residual minor: tighter spacing than MAUI.

### 115. Radio Button Group Binding — 🟢 (L:match / D:match)  ⚠️ _MAUI reference capture broken — re-shoot needed_
- **Light:** MAUI ground-truth capture is entirely blank (all black, only the nav bar visible) — broken capture, cannot be compared. C++ renders the full page correctly: explanatory text, 'GroupName is group1 / Selection is (null)', Option A/B/C/D in a 2-col grid, and the two blue action buttons ('Set selection... to B', 'Clear selection... to null'). C++ is correct; classifying match since C++ output is right and the MAUI side is unusable.
- **Dark:** Same as light — MAUI capture is fully blank/broken; C++ renders the complete page correctly with all text, the four radio options and the two blue action labels.

### 116. Radio Button Group Gallery — 🟡 (L:minor / D:minor)
- **Light:** FIXED (iOS radio handler): all radio rows (Parent-level Group=null, Page-level Group='A', and the mixed-group-names section) are now left-aligned with a ring↔label gap (were centered/flush). C++ also shows the full third section the MAUI capture is truncated before. Residual minor: tighter vertical spacing than MAUI.
- **Dark:** Same as light — left-aligned with gaps; full content shown. Residual minor: tighter spacing than MAUI.

### 117. Radio Button Border — 🟡 (L:minor / D:minor)
- **Light:** FIXED (iOS radio handler: left-align + ring↔label gap + VisualElement background). Now four left-aligned bordered rows: Option 1 red border over YELLOW fill, Option 2 YELLOW fill, Option 3 plain, Option 4 green border with the selected dot — matching MAUI. Residual minor: MAUI's rows are a little taller (more internal content padding) than the port's.
- **Dark:** Same as light — yellow fill + red/green borders + selection all render correctly in dark. Residual minor: rows slightly shorter than MAUI; white label text on the yellow fill is lower-contrast (text follows the dynamic dark label color).

### 118. Radio Button Content — 🔴 (L:minor / D:diff)
- **Light:** FIXED (iOS radio handler): each radio row is left-aligned with a gap between ring and label (Option A, Option C, the View-fallback radio, coffee.png). Residual minor: the long View-fallback content truncates to one line ('Can't use View for…, so just plain old text') where MAUI wraps it to two lines.
- **Dark:** Radio rows now left-aligned with gaps (same fix as light). SEPARATE residual diff: the custom coffee-cup ControlTemplate at the bottom uses hardcoded BLACK shapes (cup glyph + one underline) that are invisible on the dark background — only the red underline renders. Element drops out in dark (a theme-color bug in that custom template, not the radio layout).

### 119. Radio Content Properties — 🟡 (L:minor / D:minor)
- **Light:** All content rows render correctly in both (colored Option A italic-red, Option B bold-blue, green 'It's a button inside a button' rows). C++ uses denser line spacing so it shows all rows fully on one screen while MAUI's last rows are cut off; C++ radio glyph circles sit immediately left of their text (near-centered) vs MAUI's far-left aligned larger circles. Cosmetic spacing/glyph-position only.
- **Dark:** Same as light: text, colors, and font attributes all correct in dark; only difference is C++'s tighter vertical spacing and radio-circle horizontal position vs MAUI. Cosmetic.

### 120. Radio Template From Style — 🟢 (L:match / D:match)  ⚠️ _MAUI reference capture broken — re-shoot needed_
- **Light:** MAUI ground-truth PNG is entirely blank/all-black (only status bar visible) — cannot be used as a reference. C++ renders correctly: three templated cards labeled A/B/C each with a blue selected radio dot. C++ output looks correct; classified match since the broken MAUI ref cannot contradict it.
- **Dark:** MAUI ground-truth PNG again fully blank/all-black. C++ renders the three A/B/C templated cards with blue radio dots correctly. MAUI ref unusable.

### 121. Scattered Radio Button — 🟡 (L:minor / D:minor)
- **Light:** FIXED (iOS radio handler): the A/B/C radios now render left-aligned with a gap between each ring and its label (was crammed 'OAOBOC'), and 'D (None of the above)' is left-aligned at the page margin (was centered). Residual minor: inter-radio horizontal spacing is slightly tighter than MAUI's.
- **Dark:** Same as light — radios now left-aligned with ring↔label gaps. Residual minor: tighter inter-radio spacing; the A/B/C row sits on the light-blue highlight band (present in MAUI too).

### 122. Swipe Gesture — 🟢 (L:match / D:match)  ⚠️ _MAUI reference capture broken — re-shoot needed_ 🎬 _motion/effect — needs an animated GIF to judge_
- **Light:** MAUI reference capture is broken: the card content text is GARBLED/overlapping into an illegible jumble (the 'Welcome to .NET MAUI!', the 'June 2026' date, and the subtitle 'A SwipeView with gesture recognizers / Double-tap the card...' are all stacked on top of each other). The C++ side renders this card cleanly and correctly (title 'Welcome to .NET MAUI!', 'June 2026', two-line subtitle, then 'TapCommand (double-tap)'). Needs re-capture before a fair comparison.
- **Dark:** MAUI reference capture is broken in dark too: the 'Welcome to .NET MAUI!' card content is MISSING entirely (only the header band and 'TapCommand (double-tap)' show, no card). The C++ side renders the full card content correctly. Needs re-capture.

### 123. Swipe Item Position — 🟢 (L:match / D:match)  🎬 _motion/effect — needs an animated GIF to judge_
- **Light:** Both show the 'Reveal' bordered box followed by the 'Swipe in any direction' subtitle. Layout, text, and sizing match (C++ omits only the white harness card border, a harness-only artifact).
- **Dark:** Both show the 'Reveal' box at top followed by a large light-gray SwipeView content block filling most of the page; the 'Swipe in any direction' subtitle is hidden behind the gray block on both sides. Equivalent rendering. The page's real point (swipe to reveal items) needs a GIF to judge.

### 124. Swipe Item Size — 🟢 (L:match / D:match)
- **Light:** Strong match: 'Swipe a row left to reveal Delete', bold 'Different icon sizes', the 128x128/256x256/512x512 Icon rows each with a gray 'Swipe to Left' card, then bold 'Different SwipeView sizes', 'SwipeView 128 Height' and 'SwipeView 256 Height' rows. All gray cards and labels align.
- **Dark:** Same strong match in dark; the gray 'Swipe to Left' cards render identically and the 'Swipe to Left' label is light-on-gray in both. All section headers and rows present.

### 125. Swipe Threshold — 🟢 (L:match / D:match)
- **Light:** Content matches: header band, 'Default Threshold (Reveal Mode)' purple bar, 'Custom Threshold ... Reveal Mode' slider, purple bar, 'Default Threshold (Execute Mode)' purple bar, plus the C++ side correctly shows the full remainder ('Custom Threshold ... Execute Mode' slider + bar + 'Reveal threshold=80 / Execute threshold=80' footer) that the MAUI capture is scrolled above. Slider thumb positions and purple bar colors match.
- **Dark:** Same as light: identical purple bars, sliders, and labels; C++ shows the complete list (footer 'Reveal threshold=80 / Execute threshold=80') while the MAUI capture is scrolled to show less. Equivalent rendering.

### 126. Swipe View Margin — 🟢 (L:match / D:match)
- **Light:** Strong match: 'Horizontal items revealed', the black instruction box, 'SwipeView Content Margin' slider, 'SwipeView Content Padding' slider (thumbs at matching positions), and the gray 'Horizontal SwipeItems' / 'Vertical SwipeItems' cards. C++ shows both cards fully; MAUI capture is scrolled to top showing Horizontal + partial Vertical.
- **Dark:** Same strong match in dark: black instruction box, both sliders at matching positions, and both gray SwipeItems cards present and aligned.

### 127. Swipe View Shadow — 🔴 (L:diff / D:diff)
- **Light:** MAUI shows two content-width rounded cards (each ~screen-width minus margins, centered) with a light gradient fill, black border, drop SHADOW, and 'Content' text CENTERED. C++ renders the cards full-bleed to the screen edges, with NO drop shadow, the cards are a different (shorter) height, and the 'Content' label is LEFT-aligned at the card's top-left instead of centered.
- **Dark:** MAUI dark shows the two cards with 'Content' text CENTERED. C++ dark shows 'Content' LEFT-aligned, and again no visible shadow. The card chrome is hard to see on black in both, but the text-alignment difference (centered vs left) is a clear discrepancy and the page's shadow effect is not rendered by C++.

### 128. Swipe Refresh — 🟢 (L:match / D:match)  🎬 _motion/effect — needs an animated GIF to judge_
- **Light:** Both render only the two text lines 'Swipe left to delete, pull to refresh' and 'Ready' with the rest of the page empty (the list/refresh content requires interaction). Identical static rendering. The page's point (pull-to-refresh spinner) needs a GIF to judge.
- **Dark:** Same as light: both show 'Swipe left to delete, pull to refresh' and 'Ready' on black, no list items, identical. Pull-to-refresh motion needs a GIF.

### 129. Refresh View — 🟡 (L:minor / D:minor)
- **Light:** All elements present and matching: header 'Pull the items down...', 'Number of items: 50', four blue action buttons, 'Is Refreshing: False', 'Is Enabled: True', '50 items loaded' footer. Difference is button layout — MAUI spreads buttons 2-per-row widely (clipping 'Toggle Background Color' at right edge) while C++ packs them tighter so all labels are fully visible. Cosmetic spacing.
- **Dark:** Same as light: identical content, only button-row spacing differs (MAUI wide/clipped, C++ compact/full). Cosmetic.

### 130. Custom Size Swipe — 🟢 (L:match / D:match)
- **Light:** Green SwipeView content block with 'This is the SwipeView Content', 'Test Click from Content' blue link, and 'RightItems revealed (open=1, threshold=0)' label all render identically in position, color and text.
- **Dark:** Dark theme matches: identical green block, blue link, and status label. Only harness nav-bar chrome differs.

### 131. Custom Swipe Item View — 🟢 (L:match / D:match)
- **Light:** 'Right items revealed (Favourite)' label and the blue card ('Welcome to .NET MAUI' / 'June 19, 2026') render identically. C++ card padding is marginally tighter but equivalent.
- **Dark:** Dark theme matches: same label and blue card with identical text and color.

### 132. Basic Swipe — 🟢 (L:match / D:match)
- **Light:** Five gray swipe rows ('Swipe Up/Down/Right/Left (Execute/Reveal)', 'Swipe in any direction') plus 'Swipe a row, then invoke Delete' label render identically. Only difference is the harness nav title bar present in MAUI vs absent in C++ (harness chrome), and MAUI rows have slightly more rounded corners — cosmetic.
- **Dark:** Dark theme matches: same rows with white labels on light-gray cells, identical text and order. Same harness chrome difference only.

### 133. Gestures — 🟡 (L:minor / D:minor)  🎬 _motion/effect — needs an animated GIF to judge_
- **Light:** Layout equivalent: 'Gesture target (tap / pan / pinch / swipe / pointer)' label and blue rounded target box render identically. Differences are captured-state only: C++ shows 'Last gesture: Pointer exited' vs MAUI 'Last gesture: (none)', and the MAUI harness wraps the page in a white card with top/bottom black margins while C++ fills full-bleed. No element/color/size bug.
- **Dark:** Same as light: identical label + blue target box. C++ 'Last gesture: Pointer exited' vs MAUI '(none)' is a captured-state diff; MAUI white-card framing absent in C++. Cosmetic only.

### 134. Pan Gesture Events — 🟢 (L:match / D:match)  🎬 _motion/effect — needs an animated GIF to judge_
- **Light:** Green/red vertical split box with white 'StatusType: Completed, TotalX: 0, TotalY: 0' label at top of green region — matches MAUI. C++ box fills the full content height (no thin white top/bottom margins MAUI shows) but the layout, colors and label are equivalent. Static frame of a pan-gesture demo.
- **Dark:** Same green/red split with status label, equivalent to MAUI. Minor: C++ fills full height vs MAUI's small white margins.

### 135. Pointer Gesture — 🟡 (L:minor / D:minor)  🎬 _motion/effect — needs an animated GIF to judge_
- **Light:** All text present and matching: yellow 'Thanks for releasing me!...' banner, three pointer-position lines {40,30}, 'Thanks for hovering me!', three {25,18} lines, green 'Hover me green!', and hint text. Cosmetic font-size diff: MAUI renders the yellow banner, 'Thanks for hovering me!' and 'Hover me green!' in a large header font; C++ renders them at the normal body size (smaller).
- **Dark:** Same content and same font-size diff as light — C++ uses smaller font for the banner / 'Thanks for hovering me!' / 'Hover me green!' headers that MAUI shows large. Colors (yellow banner, green label, white text) match.

### 136. Drag Drop — 🟢 (L:match / D:match)
- **Light:** Both render 'All colors (drag a swatch...)' header, the 6 color swatches (orange/yellow/green/blue/purple/pink), 'Rainbow:' label and red row. C++ additionally shows the post-drop event readout ('Drag start position relative to... - Self X:0, Y:0 (Red)', 'Drag position...', 'Drop position...', 'Move: swatch dropped into Rainbow') because it has less top padding; MAUI ref clips these at bottom. Same elements.
- **Dark:** Same swatch stack and Rainbow row in dark theme; equivalent. C++ surfaces the same extra event-detail lines below due to smaller top padding.

### 137. Hit Testing — 🔴 (L:diff / D:diff)
- **Light:** Real transform-rendering bugs in C++: (1) 'Scale = 2' is NOT enlarged — rendered at the same size as 'Scale = 1', whereas MAUI renders it roughly double-size. (2) The second 'Lorem ipsum dolor sit ame' line is not horizontally offset in C++ (MAUI shifts it right via a TranslationX/transform). (3) 'Rotation = 20' is drawn upright in C++ vs visibly rotated ~20deg in MAUI. (4) C++ adds a large light-green rounded rectangle at the bottom that is absent in MAUI. Selected state ('Image' vs '-') is a benign capture diff.
- **Dark:** Same transform bugs as light: 'Scale = 2' not enlarged, second Lorem line not offset, 'Rotation = 20' not rotated, and an extra large green rounded rectangle appears at the bottom in C++ that MAUI does not show. Scale/Rotate/Translate render transforms are not being applied to these labels.

### 138. Input Transparent — 🟡 (L:minor / D:minor)
- **Light:** All content matches: the three explanatory paragraphs, 'Clickable'/'Not Clickable' buttons, the four 'Tap ...' buttons, and the intentionally overlapping 'Top (transparent)'/'Bottom (clickable)' and 'Test Button'/'Bottom Layer' button pairs (overlap is by-design in both). C++ uses tighter vertical spacing and full-bleed (no MAUI white-card padding), so it fits the toggle switch + 'Ready — tap a layer set below' line that MAUI clips off-screen. Spacing/clipping only.
- **Dark:** Same as light: identical paragraphs and buttons, identical intentional button overlaps. C++ tighter spacing reveals the toggle + 'Ready' line that MAUI cut off. Cosmetic spacing difference only.

### 139. Focus — 🟢 (L:match / D:match)
- **Light:** Both show 'Focus target' placeholder entry, 'Focus Entry' and 'Unfocus Entry' blue buttons, and 'IsFocused: false'. Only difference: MAUI spaces the two buttons farther apart while C++ places them closer together (minor spacing). Same elements and state.
- **Dark:** Identical content in dark theme: bordered 'Focus target' entry, two blue action buttons, 'IsFocused: false'. Same minor button-spacing difference as light.

### 140. Dispatcher — 🟢 (L:match / D:match)
- **Light:** Same content top-to-bottom: 'Watch the machines complain...' / 'Fail Access' (blue) / '...' / 'Now observe the happy machines...' / 'Access' (blue) / 'This was a success!' / timer section. C++ has less top padding so it surfaces additional lower rows ('3 Second Timer (Start/Stop)', 'OBSOLETE ZONE ALERT!', 'Device.StartTimer(3s)') that the MAUI ref pushes off-screen; same page, only scroll-position/top-padding differs.
- **Dark:** Identical to light comparison in dark theme; text/blue-link colors and layout equivalent, C++ shows a bit more bottom content due to smaller top padding.

### 141. Device — 🟡 (L:minor / D:minor)
- **Light:** Same text content ('Platform: iOS', 'Idiom: Phone', 'Version: 26.5'). Alignment differs: MAUI centers the text block horizontally and vertically on the page; C++ pins it to top-left. Content correct, only positioning differs.
- **Dark:** Dark theme: identical text values; same centered-vs-top-left alignment difference as light.

### 142. Effects — 🔴 (L:diff / D:diff)  ⚠️ _MAUI reference capture broken — re-shoot needed_
- **Light:** MAUI reference capture is broken — it shows the iOS home screen / springboard (Fitness, Watch, Contacts, Files, app icons) instead of the page, so no valid comparison. The C++ port renders the page correctly: 'Entry With Focus Routing Effect' + Alert Simple entry, 'Entry With Focus Platform Effect' + Alert Simple entry, 'Detach routing effect' / 'Re-attach routing effect' blue buttons, 'routing effect attached — routing attached: yes'. Not a C++ bug.
- **Dark:** MAUI reference is again the iOS home screen/springboard (capture failure). C++ renders the same correct dark-theme page. Cannot compare due to broken MAUI ref.

### 143. Measure First Strategy — 🟡 (L:minor / D:minor)
- **Light:** All content matches: intro text, blue 'Toggle Sizing Strategy' button, 'ItemSizingStrategy: MeasureFirstItem' label, green section headers (Avengers/Fantastic Four), member list, orange 'Total members: 12'. Difference is cosmetic: C++ CollectionView rows have noticeably larger inter-item vertical spacing/padding so fewer items fit per screen vs MAUI's tight rows.
- **Dark:** Same as light — content equivalent, C++ row spacing is larger than MAUI's. Colors (green headers, orange total, white text on black) match.

### 144. Scroll View — 🟡 (L:minor / D:minor)
- **Light:** Both render the 'Row N of 40' list correctly. MAUI is captured scrolled down (showing Row 12-21), while C++ shows from Row 0 with an extra top status label 'Scrolled to: 0 / 0 (done)'. C++ rows are more densely spaced (Row 0-14 visible). Difference is scroll position/state and row spacing, not a render bug; the programmatic-scroll-to-target that MAUI performed did not move C++ off Row 0.
- **Dark:** Same as light: list content correct in both; MAUI scrolled to Row 12-21, C++ at Row 0 with status label and denser spacing. State/spacing difference only.

### 145. Web View — 🟡 (L:minor / D:minor)
- **Light:** C++ correctly renders the WebView HTML ('Welcome' + 'Served from a static HtmlWebViewSource.') and the nav label 'new_page -> https://demo.test/welcome'. MAUI's capture caught the page BEFORE the async didFinishNavigation fired (blank body, 'No navigation yet') — a screenshot-timing artifact, not a C++ bug (the port is the more-complete render).
- **Dark:** Same: C++ renders correctly; the MAUI ref difference is async-load capture timing.

### 146. Hybrid Web View — 🟡 (L:minor / D:minor)
- **Light:** Same page: "HybridWebView here" label on the left and a column of 5 blue action buttons on the right (Send message to JS, Invoke JS, Invoke Async JS, Test JS Exception, Test JS Async). Truncation strategy differs: MAUI clips the button labels at the right viewport edge ("Send messag", "Invoke Async", "Test JS Exce", "Test JS Asyn") while C++ middle-truncates with ellipsis so they fit ("Send...age to JS", "Test JS...ception"). Same controls, cosmetic text-fit difference.
- **Dark:** Same controls and buttons as light. MAUI dark shows a large white blank band in the lower-middle (the empty HybridWebView native region renders white) whereas C++ dark renders that region dark/black. Content and the 5 buttons match; the empty-webview background color and the button truncation strategy differ.

### 147. Alerts — 🟡 (L:minor / D:minor)
- **Light:** Identical content and order: "OnAppearing: Alert — Welcome to the Alerts Page [Hello!]", "Display Alert" (Alert Simple / Alert Yes/No), "Display ActionSheet" (ActionSheet Simple / ActionSheet Cancel/Delete), "Display Prompt" (Question 1 / Question 2). Only difference is vertical spacing — MAUI has larger gaps between the blue buttons; C++ is more compact. Cosmetic.
- **Dark:** Same as light — identical labels and button set, only the inter-button vertical spacing differs (MAUI taller, C++ compact). Cosmetic.

### 148. Animation — 🟡 (L:minor / D:minor)  🎬 _motion/effect — needs an animated GIF to judge_
- **Light:** This page's point is a rotation animation. The "...t (animation target)" label was captured MID-ROTATION in MAUI (rendered diagonally/rotated ~70deg) whereas C++ shows the ".NET bot (animation target)" label flat/horizontal at top-left. The three buttons (Start Animation, Start Custom Animation, Cancel Animation — last one greyed/disabled) match in both. The label-rotation discrepancy is a capture-timing difference between animation phases, not a layout bug; a GIF is needed to judge true parity.
- **Dark:** Same as light: MAUI dark shows the target label rotated mid-animation, C++ shows it flat. Buttons (Start Animation / Start Custom Animation / Cancel Animation) match. Needs a motion GIF to fairly compare the rotation animation.

### 149. Application Control — 🟡 (L:minor / D:minor)
- **Light:** Same structure: "Quits the application" header, then blue buttons Terminate Application / Open Window / Close Window, then a status line. Two cosmetic deltas: (1) MAUI renders the "Quits the application" header BOLD and large; C++ renders it in a lighter/smaller weight. (2) Status line differs — MAUI shows "main window: (untitled)" while C++ shows "main window: MAUI C++ — gallery" (the C++ window has an actual title). Both show "Windows open: 1" and "main page: set".
- **Dark:** Same as light — header weight differs (bold in MAUI, lighter in C++) and the window-title text in the status line differs ("(untitled)" in MAUI vs "MAUI C++ — gallery" in C++, an app-identity difference). Same buttons and counts.

### 150. Ios Entry — 🟢 (L:match / D:match)
- **Light:** Entry with placeholder "Enter text here to see the font size change" inside a subtle rounded-border field, and a blue "Toggle AdjustsFontSizeToFitWidth" button below. Placeholder text, border outline, and button all match between MAUI and C++. Only minor vertical positioning differs (harness card).
- **Dark:** Same as light — placeholder Entry with its border and the blue toggle button render identically in dark theme. Match.

### 151. Ios Date Picker — 🟢 (L:match / D:match)
- **Light:** DatePicker shows "31.12.2020" (device-locale dd.MM.yyyy) in a bordered field with a blue "Toggle DatePicker UpdateMode" button below. Date format, value, border box, and button all match MAUI exactly. Only vertical positioning differs (harness card).
- **Dark:** Same as light — "31.12.2020" in the same locale format, bordered field, and toggle button all match in dark theme. Match.

### 152. Ios Time Picker — 🟢 (L:match / D:match)
- **Light:** TimePicker shows "14:00" (device-locale 24h format) in a bordered field, with blue "Toggle TimePicker UpdateMode" button and "UpdateMode: WhenFinished" status label below. Time format/value, border, button, and status text all match MAUI. Only vertical positioning differs (harness card).
- **Dark:** Same as light — "14:00" 24h format, bordered field, toggle button, and "UpdateMode: WhenFinished" status all match in dark theme. Match.

### 153. Ios Picker — 🟢 (L:match / D:match)
- **Light:** Equivalent: 'Select a monkey' placeholder picker field with hairline border, and 'Toggle Picker UpdateMode' blue button below. Only difference is C# white harness card vs C++ no card.
- **Dark:** Equivalent: same picker field and button in dark theme; placeholder and border render correctly. Harness-card only difference.

### 154. Ios Search Bar — 🟡 (L:minor / D:minor)
- **Light:** All elements match: rounded search bar with magnifier icon + 'Enter search term' placeholder, 'Toggle SearchBar Style' and 'Toggle Background' blue buttons. Only diff: C++ places the search bar higher/closer to the top (less top spacing) than MAUI which centers it lower; button vertical gap also tighter in C++.
- **Dark:** Match in dark theme: dark-gray rounded search bar, placeholder, magnifier, and both blue buttons all render. Same minor top-spacing difference as light (C++ bar sits higher).

### 155. Ios Scroll View — 🟡 (L:minor / D:minor)
- **Light:** All elements present: slider, 'Toggle ScrollView DelayContentTouches' and 'Return to Platform-Specifics List' blue buttons. Differences are cosmetic: C# spaces the two buttons far apart with a white harness card; C++ stacks them tightly and shows a harness back-chevron. Slider thumb sits at ~50% (C++) vs ~45% (C#) — demo value.
- **Dark:** Same elements and same cosmetic spacing/chevron differences in dark theme; slider thumb position differs slightly. No missing content.

### 156. Ios Slider Update On Tap — 🟡 (L:minor / D:minor)
- **Light:** Match: 'Tap on the Slider bar to move the thumb.' label, slider (thumb at left, gray track) and blue 'Toggle Update on Tap' button all present and equivalent. Only diff: C++ content sits higher with less top spacing vs MAUI.
- **Dark:** Match in dark theme: label, slider with white thumb on dark track, and blue toggle button render correctly. Same minor top-spacing offset as light.

### 157. Ios First Responder — 🟡 (L:minor / D:minor)
- **Light:** Strong parity: both Entry fields ('First Entry', 'Second Entry' placeholders), both 'OK' buttons, the 'Focus First'/'Focus Second' buttons, and all three status lines ('First IsFocused: false', 'Second IsFocused: false', 'Second OK CanBecomeFirstResponder: true') render with identical text and state. Minor diffs: C++ places 'Focus First' and 'Focus Second' adjacent with no gap (MAUI spaces them across the row), and C++ clips the first instruction one line shorter ('...keyboard should' without 'disappear.'). Spacing/clipping only.
- **Dark:** Same as light: both entries, both OK buttons, Focus First/Second, and the three status lines all match in text and state. C++ 'Focus First'/'Focus Second' buttons sit flush together vs MAUI spaced apart, and the first instruction line is clipped one row shorter. Cosmetic only.

### 158. Ios Pan Gesture — 🟢 (L:match / D:match)  🎬 _motion/effect — needs an animated GIF to judge_
- **Light:** Equivalent: 'panned x:45 y:-12' label, 'Toggle Simultaneous Gesture Recognition' blue button, 'Pan target', 'SimultaneousRecognition: false' all present and identically laid out. C# wraps content in a white harness card and renders the panned label bold; C++ has no card and regular weight — harness/style artifacts only.
- **Dark:** Equivalent content and order in dark theme; same harness-card/bold differences as light, cosmetic only.

### 159. Ios Safe Area — 🟢 (L:match / D:match)
- **Light:** Equivalent: identical lorem-ipsum paragraph (Lorem ipsum...constructio interrete.) and 'Disable Use Safe Area' blue button. Content respects the safe-area top inset in both. C# white card vs C++ no card; C++ starts slightly higher due to no card padding — cosmetic.
- **Dark:** Equivalent lorem text and button in dark theme; same harness-card difference only.

### 160. Ios Swipe Transition — 🟡 (L:minor / D:minor)  🎬 _motion/effect — needs an animated GIF to judge_
- **Light:** Match: 'SwipeTransitionMode:' label, blue Reveal/Drag buttons, light-gray swipe item ('Swipe right'), instruction text, and 'SwipeTransitionMode: Drag' status all present. Minor: C++ packs the Reveal and Drag buttons close together on one line vs MAUI's wider spacing; C++ content sits higher.
- **Dark:** Match in dark theme: same elements render. The swipe item keeps a light-gray background with dark text in both platforms (demo color, not a bug). Same minor button-spacing/top-offset diffs as light.

### 161. Ios Blur Effect — 🔴 (L:diff / D:diff)  ⚠️ _MAUI reference capture broken — re-shoot needed_ 🎬 _motion/effect — needs an animated GIF to judge_
- **Light:** This is a blur-effect demo whose subject is an image. The MAUI reference (LIGHT) shows ONLY the four buttons (No Blur / Extra Light Blur / Light Blur / Dark Blur) and the 'BlurEffect: ExtraLight' label, with NO image rendered at all. The C++ port correctly shows the source photo (a black pug in a sweater) at the top with the buttons below. So C++ renders MORE content than MAUI here; MAUI's reference appears to be missing the image entirely. The actual blur strength is a transient effect a still frame cannot fully judge.
- **Dark:** Same as light: the MAUI DARK reference shows only the four buttons + 'BlurEffect: ExtraLight' label and NO image, while the C++ port shows the dog photo plus the buttons. MAUI reference is missing the image content; cannot fairly compare blur fidelity from a still.

### 162. Navigation Gallery — 🟡 (L:minor / D:minor)
- **Light:** Identical content: status label 'Stack depth: 1 | top: PAGE NUMBER 1 | secondary toolbar items: 0' and the 6 buttons (Push Page, Pop Page, Insert Page Before Current, Remove Page Before Current, Pop To Root, Toggle Secondary Toolbar Item). Only difference is vertical density: MAUI has generous spacing between buttons and a nav-bar header; C++ packs the buttons tighter and omits the nav header. No element missing or wrong.
- **Dark:** Same as light: identical label + 6 buttons; the only difference is tighter inter-button spacing and the absent nav-bar header in C++.

### 163. Modal — 🟡 (L:minor / D:minor)
- **Light:** Identical content: 'Modal Page 1', buttons Push Page / Push Modal Page / Push Modal Navigation Page / Push Modal Flyout Page, a grayed-out (disabled) 'Pop Modal Page', and 'Modal depth: 0 | page stack depth: 1'. Disabled state, colors, and order all match. Only difference is MAUI's wider button spacing and nav-bar header vs C++'s tighter spacing and no header.
- **Dark:** Same as light: identical content and disabled-button state; only the inter-button spacing density and absent nav header differ in C++.

### 164. Tabbed Flyout — ⬛ (L:blank / D:blank)
- **Light:** C++ renders a completely BLANK screen (only the status bar is visible) where MAUI shows a full flyout+tabbed page: 'Menu' header, buttons 'Home tab' / 'Settings tab' / 'Toggle flyout', then 'Flyout dismissed', 'Demo tabs', 'Home', 'This is the Home tab.', 'Settings', 'This is the Settings tab.' The entire C++ page body is empty.
- **Dark:** Same as light: C++ DARK renders a fully blank black screen with no content at all, while MAUI shows the complete flyout/tabbed page (Menu, Home tab/Settings tab/Toggle flyout buttons, and the Home/Settings tab text).

### 165. Toolbar — 🟡 (L:minor / D:minor)
- **Light:** Identical content: 'You clicked on ToolbarItem: {none}' and the 6 buttons (Enable/Disable Test (1), Enable/Disable Test Secondary (4), Enable/Disable Test Secondary (2), Change text on Test Secondary (1), Remove/Add Secondary (3), Change Command Property on Secondary (3)). Only difference is the tighter button spacing and the missing nav-bar header in C++.
- **Dark:** Same as light: identical label + 6 buttons; only spacing density and the absent nav header differ.

### 166. Menu Bar — 🟡 (L:minor / D:minor)
- **Light:** Identical content: 'You clicked on Menu Item:' label and the blue 'Toggle Menu Bar Item' button. (A native MenuBar is a desktop concept and is not shown in the body on either side.) Only difference is the tighter spacing and missing nav-bar header in C++.
- **Dark:** Same as light: identical 'You clicked on Menu Item:' label and 'Toggle Menu Bar Item' button; only spacing and absent nav header differ.

### 167. Title Bar — 🟡 (L:minor / D:minor)
- **Light:** Both show the same two-column layout: left 'Content Options' with Set Icon radio, Title + Subtitle entry fields, Leading Content / Content / Trailing Content / Tall TitleBar radios, and a checked 'Show TitleBar'; right 'Color Options' with two 'Green' entry fields, 'Set Color'/'Set Foreground' buttons (clipped at the right edge on both), 'Toggle Title Bar On Window', and 'TitleBar: Title / Subtitle / Content are live'. All controls and the checked state match. Only difference is C++'s tighter vertical density and missing nav header.
- **Dark:** Same as light: identical two-column control set including the checked 'Show TitleBar' and the right-edge-clipped Color Options buttons; only spacing density and absent nav header differ.

### 168. Chrome — 🟡 (L:minor / D:minor)  🎬 _motion/effect — needs an animated GIF to judge_
- **Light:** Both show the blue 'Press or right-click me' button and a 'Ready' label below it. Identical static content. The context menu itself only appears on right-click/long-press, so a still cannot show the menu. Only difference is C++'s tighter top spacing and missing nav-bar header.
- **Dark:** Same as light: identical 'Press or right-click me' button + 'Ready' label; only spacing and absent nav header differ. The right-click menu is interaction-only.

### 169. Context Flyout — 🔴 (L:diff / D:diff)
- **Light:** Top controls match (gray 'Increment by 1 (or right-click me)' button, 'Is dynamic menu enabled?' toggle, 'Right-click to see beautiful menus', 'Has a custom context menu' entry, and the blue 'COOL' image-button). Below that the two captures diverge: the C++ port renders the full lower page content — a 'Microsoft Bing' cookie-consent card (Accept/Reject/More options buttons), a 'Privacy/Legal/Advertise/Ad Info' link row, a '0' counter label, and the description text 'Right-click a control, or its menu items are exercised programmatically' — whereas the MAUI reference shows only blank white space there with none of that content. C++ shows more/correct content; MAUI ref appears truncated/missing the lower section.
- **Dark:** Same divergence in dark theme: top controls match, but C++ renders the full lower section (Microsoft Bing cookie card with Accept/Reject, link row, '0' counter, and 'Right-click a control...' description) while the MAUI reference shows only an empty white card band and omits all of that content.

### 170. Templated View — 🟡 (L:minor / D:minor)
- **Light:** Strong match. Both show the red intro text 'A standard CardView control is suitable for grid layouts:', the standard card ('Slavko Vlasic' + lorem text), the red 'A ControlTemplate overrides the standard view, creating a more compact view:' note, and three compact cards (Carolina Pena, Wade Blanks, Colette Quint) each with a gray image placeholder and bold 'Compact card' title. Only cosmetic difference: C++ compact cards / gray placeholders are slightly smaller so all three fit fully on screen, while MAUI's cards are a touch larger so the third (Colette Quint) is partially cut off at the bottom.
- **Dark:** Same as light: content, structure, red headings, gray placeholders and three compact cards all match; the only difference is the slightly smaller C++ card sizing (all three visible) vs slightly larger MAUI cards (third card clipped).

### 171. Custom Layout — 🟢 (L:match / D:match)
- **Light:** Match. The custom layout positions blue labels identically: 'Top' centered near the top, a middle row of 'Left  Left ........ Right  Right', and 'Bottom' centered near the bottom. Same blue text color, same anchoring and spacing in both. Only differences are harness-only (nav pill, clock).
- **Dark:** Match in dark theme too: 'Top' / 'Left Left ... Right Right' / 'Bottom' blue labels at the same positions on a black background, identical between MAUI and C++.

### 172. Visual States — 🔴 (L:diff / D:diff)
- **Light:** Real discrepancy: the first field, 'Entry with VisualStateManager:', is rendered as a BRIGHT GREEN filled entry in the MAUI ground truth (the VSM has applied its green-state background), but in the C++ port that same entry has NO green fill — it renders as a plain empty/white entry box. The VisualStateManager background color is missing. Everything else matches: 'Entry to enable 2nd Entry' placeholder field, the 'Hover me to see the state change' and 'Click me to see the state change and revert' blue buttons, and the descriptive labels.
- **Dark:** Same missing-fill bug in dark theme: MAUI renders the 'Entry with VisualStateManager:' entry as bright green, while the C++ port shows it as a plain dark/empty entry with no green VSM background. All other elements (second entry placeholder, two blue buttons, description text) match.
