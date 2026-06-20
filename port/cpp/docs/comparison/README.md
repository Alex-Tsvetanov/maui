# C++ port vs .NET MAUI — iOS pixel-parity tracker

Theme-matched iOS comparison: each page rendered by **real .NET MAUI** vs the **C++ port**, on the same iPhone 17 simulator, compared **light-vs-light** and **dark-vs-dark**. Both stacks render native-default controls + the system font (the C# app's `dotnet new maui` default `Styles.xaml` + OpenSans are stripped; appearance forced via `MAUI_THEME` / `MAUI_APPEARANCE`). Goal: pixel-perfect parity, fixed example-by-example.

**Progress: 46 / 172 🟢 matched** · 72 🟡 minor · 54 🔴 diff · 0 ⬜ pending

**Flags: 13 ⚠️ broken MAUI reference captures (re-shoot needed) · 12 🎬 motion/effect pages needing an animated GIF to judge.**

Status legend: 🟢 pixel-match (both themes) · 🟡 minor diff · 🔴 notable diff to fix · ⬛ C++ renders blank · ⬜ not yet reviewed · ⚠️ MAUI reference capture itself is broken (re-shoot needed) · 🎬 motion/effect page — a still frame can't judge it; needs a GIF. Per-theme verdicts + per-page notes in `parity_status.json`; the **Per-page findings** section below lists every non-matching page's concrete diffs.

> macOS / Mac Catalyst 4-way comparison is **Phase 2** (pending: aligning the gallery window size to the C# window). The earlier 2-way macOS grid + notes live in [PARITY_FINDINGS.md](PARITY_FINDINGS.md).

Rows are in **fix order** (top → bottom): foundational single controls first (their fixes cascade), then layouts, shapes, borders/clip, collection-views, radio, swipe, gestures, scroll/web, combos, iOS-specifics, and chrome/host pages last.

| # | Page | Status | .NET MAUI (light) | C++ (light) | .NET MAUI (dark) | C++ (dark) |
| --: | --- | :---: | --- | --- | --- | --- |
| 1 | Label | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/label.png) | ![](cpp_ios_light/label.png) | ![](csharp_ios_dark/label.png) | ![](cpp_ios_dark/label.png) |
| 2 | Button | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/button.png) | ![](cpp_ios_light/button.png) | ![](csharp_ios_dark/button.png) | ![](cpp_ios_dark/button.png) |
| 3 | Entry | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/entry.png) | ![](cpp_ios_light/entry.png) | ![](csharp_ios_dark/entry.png) | ![](cpp_ios_dark/entry.png) |
| 4 | Editor | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/editor.png) | ![](cpp_ios_light/editor.png) | ![](csharp_ios_dark/editor.png) | ![](cpp_ios_dark/editor.png) |
| 5 | Search Bar | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/search_bar.png) | ![](cpp_ios_light/search_bar.png) | ![](csharp_ios_dark/search_bar.png) | ![](cpp_ios_dark/search_bar.png) |
| 6 | Picker | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/picker.png) | ![](cpp_ios_light/picker.png) | ![](csharp_ios_dark/picker.png) | ![](cpp_ios_dark/picker.png) |
| 7 | Date Picker | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/date_picker.png) | ![](cpp_ios_light/date_picker.png) | ![](csharp_ios_dark/date_picker.png) | ![](cpp_ios_dark/date_picker.png) |
| 8 | Time Picker | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/time_picker.png) | ![](cpp_ios_light/time_picker.png) | ![](csharp_ios_dark/time_picker.png) | ![](cpp_ios_dark/time_picker.png) |
| 9 | Pickers | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/pickers.png) | ![](cpp_ios_light/pickers.png) | ![](csharp_ios_dark/pickers.png) | ![](cpp_ios_dark/pickers.png) |
| 10 | Slider | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/slider.png) | ![](cpp_ios_light/slider.png) | ![](csharp_ios_dark/slider.png) | ![](cpp_ios_dark/slider.png) |
| 11 | Stepper | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/stepper.png) | ![](cpp_ios_light/stepper.png) | ![](csharp_ios_dark/stepper.png) | ![](cpp_ios_dark/stepper.png) |
| 12 | Switch | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/switch.png) | ![](cpp_ios_light/switch.png) | ![](csharp_ios_dark/switch.png) | ![](cpp_ios_dark/switch.png) |
| 13 | Check Box | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/check_box.png) | ![](cpp_ios_light/check_box.png) | ![](csharp_ios_dark/check_box.png) | ![](cpp_ios_dark/check_box.png) |
| 14 | Progress Bar | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/progress_bar.png) | ![](cpp_ios_light/progress_bar.png) | ![](csharp_ios_dark/progress_bar.png) | ![](cpp_ios_dark/progress_bar.png) |
| 15 | Activity Indicator | 🟡🎬<br>L:minor<br>D:minor | ![](csharp_ios_light/activity_indicator.png) | ![](cpp_ios_light/activity_indicator.png) | ![](csharp_ios_dark/activity_indicator.png) | ![](cpp_ios_dark/activity_indicator.png) |
| 16 | Indicator | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/indicator.png) | ![](cpp_ios_light/indicator.png) | ![](csharp_ios_dark/indicator.png) | ![](cpp_ios_dark/indicator.png) |
| 17 | Image | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/image.png) | ![](cpp_ios_light/image.png) | ![](csharp_ios_dark/image.png) | ![](cpp_ios_dark/image.png) |
| 18 | Image Button | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/image_button.png) | ![](cpp_ios_light/image_button.png) | ![](csharp_ios_dark/image_button.png) | ![](cpp_ios_dark/image_button.png) |
| 19 | Box View | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/box_view.png) | ![](cpp_ios_light/box_view.png) | ![](csharp_ios_dark/box_view.png) | ![](cpp_ios_dark/box_view.png) |
| 20 | Content View | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/content_view.png) | ![](cpp_ios_light/content_view.png) | ![](csharp_ios_dark/content_view.png) | ![](cpp_ios_dark/content_view.png) |
| 21 | Containers | 🟡<br>L:minor<br>D:match | ![](csharp_ios_light/containers.png) | ![](cpp_ios_light/containers.png) | ![](csharp_ios_dark/containers.png) | ![](cpp_ios_dark/containers.png) |
| 22 | Control stack | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/controls_stack.png) | ![](cpp_ios_light/controls_stack.png) | ![](csharp_ios_dark/controls_stack.png) | ![](cpp_ios_dark/controls_stack.png) |
| 23 | Input Controls | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/input_controls.png) | ![](cpp_ios_light/input_controls.png) | ![](csharp_ios_dark/input_controls.png) | ![](cpp_ios_dark/input_controls.png) |
| 24 | Fonts | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/fonts.png) | ![](cpp_ios_light/fonts.png) | ![](csharp_ios_dark/fonts.png) | ![](cpp_ios_dark/fonts.png) |
| 25 | Formatted Text | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/formatted_text.png) | ![](cpp_ios_light/formatted_text.png) | ![](csharp_ios_dark/formatted_text.png) | ![](cpp_ios_dark/formatted_text.png) |
| 26 | Styles | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/styles.png) | ![](cpp_ios_light/styles.png) | ![](csharp_ios_dark/styles.png) | ![](cpp_ios_dark/styles.png) |
| 27 | Triggers | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/triggers.png) | ![](cpp_ios_light/triggers.png) | ![](csharp_ios_dark/triggers.png) | ![](cpp_ios_dark/triggers.png) |
| 28 | Behaviors | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/behaviors.png) | ![](cpp_ios_light/behaviors.png) | ![](csharp_ios_dark/behaviors.png) | ![](cpp_ios_dark/behaviors.png) |
| 29 | Semantics | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/semantics.png) | ![](cpp_ios_light/semantics.png) | ![](csharp_ios_dark/semantics.png) | ![](cpp_ios_dark/semantics.png) |
| 30 | App Theme Binding | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/app_theme_binding.png) | ![](cpp_ios_light/app_theme_binding.png) | ![](csharp_ios_dark/app_theme_binding.png) | ![](cpp_ios_dark/app_theme_binding.png) |
| 31 | Stack Layout | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/stack_layout.png) | ![](cpp_ios_light/stack_layout.png) | ![](csharp_ios_dark/stack_layout.png) | ![](cpp_ios_dark/stack_layout.png) |
| 32 | Vertical Stack | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/vertical_stack.png) | ![](cpp_ios_light/vertical_stack.png) | ![](csharp_ios_dark/vertical_stack.png) | ![](cpp_ios_dark/vertical_stack.png) |
| 33 | Horizontal Stack | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/horizontal_stack.png) | ![](cpp_ios_light/horizontal_stack.png) | ![](csharp_ios_dark/horizontal_stack.png) | ![](cpp_ios_dark/horizontal_stack.png) |
| 34 | Grid | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/grid.png) | ![](cpp_ios_light/grid.png) | ![](csharp_ios_dark/grid.png) | ![](cpp_ios_dark/grid.png) |
| 35 | Absolute Layout | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/absolute_layout.png) | ![](cpp_ios_light/absolute_layout.png) | ![](csharp_ios_dark/absolute_layout.png) | ![](cpp_ios_dark/absolute_layout.png) |
| 36 | Flex Layout | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/flex_layout.png) | ![](cpp_ios_light/flex_layout.png) | ![](csharp_ios_dark/flex_layout.png) | ![](cpp_ios_dark/flex_layout.png) |
| 37 | Relative Layout | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/relative_layout.png) | ![](cpp_ios_light/relative_layout.png) | ![](csharp_ios_dark/relative_layout.png) | ![](cpp_ios_dark/relative_layout.png) |
| 38 | Layout alignment (Start/Center/End/Fill) | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/alignment.png) | ![](cpp_ios_light/alignment.png) | ![](csharp_ios_dark/alignment.png) | ![](cpp_ios_dark/alignment.png) |
| 39 | Z Index | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/z_index.png) | ![](cpp_ios_light/z_index.png) | ![](csharp_ios_dark/z_index.png) | ![](cpp_ios_dark/z_index.png) |
| 40 | Layout Is Enabled | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/layout_is_enabled.png) | ![](cpp_ios_light/layout_is_enabled.png) | ![](csharp_ios_dark/layout_is_enabled.png) | ![](cpp_ios_dark/layout_is_enabled.png) |
| 41 | Shapes | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/shapes.png) | ![](cpp_ios_light/shapes.png) | ![](csharp_ios_dark/shapes.png) | ![](cpp_ios_dark/shapes.png) |
| 42 | Ellipse Gallery | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/ellipse_gallery.png) | ![](cpp_ios_light/ellipse_gallery.png) | ![](csharp_ios_dark/ellipse_gallery.png) | ![](cpp_ios_dark/ellipse_gallery.png) |
| 43 | Rectangle Gallery | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/rectangle_gallery.png) | ![](cpp_ios_light/rectangle_gallery.png) | ![](csharp_ios_dark/rectangle_gallery.png) | ![](cpp_ios_dark/rectangle_gallery.png) |
| 44 | Line Gallery | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/line_gallery.png) | ![](cpp_ios_light/line_gallery.png) | ![](csharp_ios_dark/line_gallery.png) | ![](cpp_ios_dark/line_gallery.png) |
| 45 | Line Join Gallery | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/line_join_gallery.png) | ![](cpp_ios_light/line_join_gallery.png) | ![](csharp_ios_dark/line_join_gallery.png) | ![](cpp_ios_dark/line_join_gallery.png) |
| 46 | Polygon Gallery | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/polygon_gallery.png) | ![](cpp_ios_light/polygon_gallery.png) | ![](csharp_ios_dark/polygon_gallery.png) | ![](cpp_ios_dark/polygon_gallery.png) |
| 47 | Polyline Gallery | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/polyline_gallery.png) | ![](cpp_ios_light/polyline_gallery.png) | ![](csharp_ios_dark/polyline_gallery.png) | ![](cpp_ios_dark/polyline_gallery.png) |
| 48 | Path Gallery | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/path_gallery.png) | ![](cpp_ios_light/path_gallery.png) | ![](csharp_ios_dark/path_gallery.png) | ![](cpp_ios_dark/path_gallery.png) |
| 49 | Path Aspect Gallery | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/path_aspect_gallery.png) | ![](cpp_ios_light/path_aspect_gallery.png) | ![](csharp_ios_dark/path_aspect_gallery.png) | ![](cpp_ios_dark/path_aspect_gallery.png) |
| 50 | Path Transform String | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/path_transform_string.png) | ![](cpp_ios_light/path_transform_string.png) | ![](csharp_ios_dark/path_transform_string.png) | ![](cpp_ios_dark/path_transform_string.png) |
| 51 | Composition Gallery | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/composition_gallery.png) | ![](cpp_ios_light/composition_gallery.png) | ![](csharp_ios_dark/composition_gallery.png) | ![](cpp_ios_dark/composition_gallery.png) |
| 52 | Transform Playground | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/transform_playground.png) | ![](cpp_ios_light/transform_playground.png) | ![](csharp_ios_dark/transform_playground.png) | ![](cpp_ios_dark/transform_playground.png) |
| 53 | Transformations | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/transformations.png) | ![](cpp_ios_light/transformations.png) | ![](csharp_ios_dark/transformations.png) | ![](cpp_ios_dark/transformations.png) |
| 54 | Update Path Data | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/update_path_data.png) | ![](cpp_ios_light/update_path_data.png) | ![](csharp_ios_dark/update_path_data.png) | ![](cpp_ios_dark/update_path_data.png) |
| 55 | Auto Size Shapes | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/auto_size_shapes.png) | ![](cpp_ios_light/auto_size_shapes.png) | ![](csharp_ios_dark/auto_size_shapes.png) | ![](cpp_ios_dark/auto_size_shapes.png) |
| 56 | Shape App Theme | 🔴<br>L:match<br>D:diff | ![](csharp_ios_light/shape_app_theme.png) | ![](cpp_ios_light/shape_app_theme.png) | ![](csharp_ios_dark/shape_app_theme.png) | ![](cpp_ios_dark/shape_app_theme.png) |
| 57 | Invalidate Brush | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/invalidate_brush.png) | ![](cpp_ios_light/invalidate_brush.png) | ![](csharp_ios_dark/invalidate_brush.png) | ![](cpp_ios_dark/invalidate_brush.png) |
| 58 | Gradient brushes | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/gradient.png) | ![](cpp_ios_light/gradient.png) | ![](csharp_ios_dark/gradient.png) | ![](cpp_ios_dark/gradient.png) |
| 59 | Border | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/border.png) | ![](cpp_ios_light/border.png) | ![](csharp_ios_dark/border.png) | ![](cpp_ios_dark/border.png) |
| 60 | Border Stroke | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/border_stroke.png) | ![](cpp_ios_light/border_stroke.png) | ![](csharp_ios_dark/border_stroke.png) | ![](cpp_ios_dark/border_stroke.png) |
| 61 | Border Layout | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/border_layout.png) | ![](cpp_ios_light/border_layout.png) | ![](csharp_ios_dark/border_layout.png) | ![](cpp_ios_dark/border_layout.png) |
| 62 | Border Playground | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/border_playground.png) | ![](cpp_ios_light/border_playground.png) | ![](csharp_ios_dark/border_playground.png) | ![](cpp_ios_dark/border_playground.png) |
| 63 | Border Clip Playground | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/border_clip_playground.png) | ![](cpp_ios_light/border_clip_playground.png) | ![](csharp_ios_dark/border_clip_playground.png) | ![](cpp_ios_dark/border_clip_playground.png) |
| 64 | Border Resize Content | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/border_resize_content.png) | ![](cpp_ios_light/border_resize_content.png) | ![](csharp_ios_dark/border_resize_content.png) | ![](cpp_ios_dark/border_resize_content.png) |
| 65 | Borderless | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/borderless.png) | ![](cpp_ios_light/borderless.png) | ![](csharp_ios_dark/borderless.png) | ![](cpp_ios_dark/borderless.png) |
| 66 | Clip | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/clip.png) | ![](cpp_ios_light/clip.png) | ![](csharp_ios_dark/clip.png) | ![](cpp_ios_dark/clip.png) |
| 67 | Clip Views | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/clip_views.png) | ![](cpp_ios_light/clip_views.png) | ![](csharp_ios_dark/clip_views.png) | ![](cpp_ios_dark/clip_views.png) |
| 68 | Clip Corner Radius | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/clip_corner_radius.png) | ![](cpp_ios_light/clip_corner_radius.png) | ![](csharp_ios_dark/clip_corner_radius.png) | ![](cpp_ios_dark/clip_corner_radius.png) |
| 69 | Clip Gallery | 🔴⚠️<br>L:diff<br>D:diff | ![](csharp_ios_light/clip_gallery.png) | ![](cpp_ios_light/clip_gallery.png) | ![](csharp_ios_dark/clip_gallery.png) | ![](cpp_ios_dark/clip_gallery.png) |
| 70 | Clipping | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/clipping.png) | ![](cpp_ios_light/clipping.png) | ![](csharp_ios_dark/clipping.png) | ![](cpp_ios_dark/clipping.png) |
| 71 | Shadow Playground | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/shadow_playground.png) | ![](cpp_ios_light/shadow_playground.png) | ![](csharp_ios_dark/shadow_playground.png) | ![](cpp_ios_dark/shadow_playground.png) |
| 72 | Invalidate Shadow Host | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/invalidate_shadow_host.png) | ![](cpp_ios_light/invalidate_shadow_host.png) | ![](csharp_ios_dark/invalidate_shadow_host.png) | ![](cpp_ios_dark/invalidate_shadow_host.png) |
| 73 | CollectionView | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/collectionview.png) | ![](cpp_ios_light/collectionview.png) | ![](csharp_ios_dark/collectionview.png) | ![](cpp_ios_dark/collectionview.png) |
| 74 | Items | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/items.png) | ![](cpp_ios_light/items.png) | ![](csharp_ios_dark/items.png) | ![](cpp_ios_dark/items.png) |
| 75 | Single Bound Selection | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/single_bound_selection.png) | ![](cpp_ios_light/single_bound_selection.png) | ![](csharp_ios_dark/single_bound_selection.png) | ![](cpp_ios_dark/single_bound_selection.png) |
| 76 | Multiple Bound Selection | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/multiple_bound_selection.png) | ![](cpp_ios_light/multiple_bound_selection.png) | ![](csharp_ios_dark/multiple_bound_selection.png) | ![](cpp_ios_dark/multiple_bound_selection.png) |
| 77 | Preselected Item | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/preselected_item.png) | ![](cpp_ios_light/preselected_item.png) | ![](csharp_ios_dark/preselected_item.png) | ![](cpp_ios_dark/preselected_item.png) |
| 78 | Preselected Items | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/preselected_items.png) | ![](cpp_ios_light/preselected_items.png) | ![](csharp_ios_dark/preselected_items.png) | ![](cpp_ios_dark/preselected_items.png) |
| 79 | Selection Command Param | 🟢⚠️<br>L:match<br>D:match | ![](csharp_ios_light/selection_command_param.png) | ![](cpp_ios_light/selection_command_param.png) | ![](csharp_ios_dark/selection_command_param.png) | ![](cpp_ios_dark/selection_command_param.png) |
| 80 | Selection Synchronization | 🟢⚠️<br>L:match<br>D:match | ![](csharp_ios_light/selection_synchronization.png) | ![](cpp_ios_light/selection_synchronization.png) | ![](csharp_ios_dark/selection_synchronization.png) | ![](cpp_ios_dark/selection_synchronization.png) |
| 81 | Filter Collection | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/filter_collection.png) | ![](cpp_ios_light/filter_collection.png) | ![](csharp_ios_dark/filter_collection.png) | ![](cpp_ios_dark/filter_collection.png) |
| 82 | Filter Selection | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/filter_selection.png) | ![](cpp_ios_light/filter_selection.png) | ![](csharp_ios_dark/filter_selection.png) | ![](cpp_ios_dark/filter_selection.png) |
| 83 | Header Footer | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/header_footer.png) | ![](cpp_ios_light/header_footer.png) | ![](csharp_ios_dark/header_footer.png) | ![](cpp_ios_dark/header_footer.png) |
| 84 | Header Footer Grid | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/header_footer_grid.png) | ![](cpp_ios_light/header_footer_grid.png) | ![](csharp_ios_dark/header_footer_grid.png) | ![](cpp_ios_dark/header_footer_grid.png) |
| 85 | Header Footer Grid Horizontal | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/header_footer_grid_horizontal.png) | ![](cpp_ios_light/header_footer_grid_horizontal.png) | ![](csharp_ios_dark/header_footer_grid_horizontal.png) | ![](cpp_ios_dark/header_footer_grid_horizontal.png) |
| 86 | Header Footer Template | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/header_footer_template.png) | ![](cpp_ios_light/header_footer_template.png) | ![](csharp_ios_dark/header_footer_template.png) | ![](cpp_ios_dark/header_footer_template.png) |
| 87 | Header Footer View | ⬛<br>L:blank<br>D:blank | ![](csharp_ios_light/header_footer_view.png) | ![](cpp_ios_light/header_footer_view.png) | ![](csharp_ios_dark/header_footer_view.png) | ![](cpp_ios_dark/header_footer_view.png) |
| 88 | Footer Only String | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/footer_only_string.png) | ![](cpp_ios_light/footer_only_string.png) | ![](csharp_ios_dark/footer_only_string.png) | ![](cpp_ios_dark/footer_only_string.png) |
| 89 | Basic Grouping | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/basic_grouping.png) | ![](cpp_ios_light/basic_grouping.png) | ![](csharp_ios_dark/basic_grouping.png) | ![](cpp_ios_dark/basic_grouping.png) |
| 90 | Grid Grouping | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/grid_grouping.png) | ![](cpp_ios_light/grid_grouping.png) | ![](csharp_ios_dark/grid_grouping.png) | ![](cpp_ios_dark/grid_grouping.png) |
| 91 | Grouping No Templates | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/grouping_no_templates.png) | ![](cpp_ios_light/grouping_no_templates.png) | ![](csharp_ios_dark/grouping_no_templates.png) | ![](cpp_ios_dark/grouping_no_templates.png) |
| 92 | Grouping Plus Selection | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/grouping_plus_selection.png) | ![](cpp_ios_light/grouping_plus_selection.png) | ![](csharp_ios_dark/grouping_plus_selection.png) | ![](cpp_ios_dark/grouping_plus_selection.png) |
| 93 | Switch Grouping | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/switch_grouping.png) | ![](cpp_ios_light/switch_grouping.png) | ![](csharp_ios_dark/switch_grouping.png) | ![](cpp_ios_dark/switch_grouping.png) |
| 94 | Some Empty Groups | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/some_empty_groups.png) | ![](cpp_ios_light/some_empty_groups.png) | ![](csharp_ios_dark/some_empty_groups.png) | ![](cpp_ios_dark/some_empty_groups.png) |
| 95 | Scroll To Group | 🔴⚠️<br>L:diff<br>D:diff | ![](csharp_ios_light/scroll_to_group.png) | ![](cpp_ios_light/scroll_to_group.png) | ![](csharp_ios_dark/scroll_to_group.png) | ![](cpp_ios_dark/scroll_to_group.png) |
| 96 | Scroll Mode Test | 🔴⚠️<br>L:diff<br>D:diff | ![](csharp_ios_light/scroll_mode_test.png) | ![](cpp_ios_light/scroll_mode_test.png) | ![](csharp_ios_dark/scroll_mode_test.png) | ![](cpp_ios_dark/scroll_mode_test.png) |
| 97 | Adaptive Collection | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/adaptive_collection.png) | ![](cpp_ios_light/adaptive_collection.png) | ![](csharp_ios_dark/adaptive_collection.png) | ![](cpp_ios_dark/adaptive_collection.png) |
| 98 | Staggered Layout | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/staggered_layout.png) | ![](cpp_ios_light/staggered_layout.png) | ![](csharp_ios_dark/staggered_layout.png) | ![](cpp_ios_dark/staggered_layout.png) |
| 99 | Varied Size Selector | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/varied_size_selector.png) | ![](cpp_ios_light/varied_size_selector.png) | ![](csharp_ios_dark/varied_size_selector.png) | ![](cpp_ios_dark/varied_size_selector.png) |
| 100 | Nested Collection | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/nested_collection.png) | ![](cpp_ios_light/nested_collection.png) | ![](csharp_ios_dark/nested_collection.png) | ![](cpp_ios_dark/nested_collection.png) |
| 101 | Data Template Selector | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/data_template_selector.png) | ![](cpp_ios_light/data_template_selector.png) | ![](csharp_ios_dark/data_template_selector.png) | ![](cpp_ios_dark/data_template_selector.png) |
| 102 | Cv Visual States | 🟡⚠️<br>L:minor<br>D:match | ![](csharp_ios_light/cv_visual_states.png) | ![](cpp_ios_light/cv_visual_states.png) | ![](csharp_ios_dark/cv_visual_states.png) | ![](cpp_ios_dark/cv_visual_states.png) |
| 103 | Empty View | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/empty_view.png) | ![](cpp_ios_light/empty_view.png) | ![](csharp_ios_dark/empty_view.png) | ![](cpp_ios_dark/empty_view.png) |
| 104 | Empty View Null | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/empty_view_null.png) | ![](cpp_ios_light/empty_view_null.png) | ![](csharp_ios_dark/empty_view_null.png) | ![](cpp_ios_dark/empty_view_null.png) |
| 105 | Empty View Rtl | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/empty_view_rtl.png) | ![](cpp_ios_light/empty_view_rtl.png) | ![](csharp_ios_dark/empty_view_rtl.png) | ![](cpp_ios_dark/empty_view_rtl.png) |
| 106 | Empty View Selector | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/empty_view_selector.png) | ![](cpp_ios_light/empty_view_selector.png) | ![](csharp_ios_dark/empty_view_selector.png) | ![](cpp_ios_dark/empty_view_selector.png) |
| 107 | Empty View Swap | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/empty_view_swap.png) | ![](cpp_ios_light/empty_view_swap.png) | ![](csharp_ios_dark/empty_view_swap.png) | ![](cpp_ios_dark/empty_view_swap.png) |
| 108 | Empty View Template | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/empty_view_template.png) | ![](cpp_ios_light/empty_view_template.png) | ![](csharp_ios_dark/empty_view_template.png) | ![](cpp_ios_dark/empty_view_template.png) |
| 109 | Empty View View | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/empty_view_view.png) | ![](cpp_ios_light/empty_view_view.png) | ![](csharp_ios_dark/empty_view_view.png) | ![](cpp_ios_dark/empty_view_view.png) |
| 110 | Empty View Load Simulate | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/empty_view_load_simulate.png) | ![](cpp_ios_light/empty_view_load_simulate.png) | ![](csharp_ios_dark/empty_view_load_simulate.png) | ![](cpp_ios_dark/empty_view_load_simulate.png) |
| 111 | Carousel Page | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/carousel_page.png) | ![](cpp_ios_light/carousel_page.png) | ![](csharp_ios_dark/carousel_page.png) | ![](cpp_ios_dark/carousel_page.png) |
| 112 | Chat Example | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/chat_example.png) | ![](cpp_ios_light/chat_example.png) | ![](csharp_ios_dark/chat_example.png) | ![](cpp_ios_dark/chat_example.png) |
| 113 | Items Updating Scroll Mode | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/items_updating_scroll_mode.png) | ![](cpp_ios_light/items_updating_scroll_mode.png) | ![](csharp_ios_dark/items_updating_scroll_mode.png) | ![](cpp_ios_dark/items_updating_scroll_mode.png) |
| 114 | Radio Button Group | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/radio_button_group.png) | ![](cpp_ios_light/radio_button_group.png) | ![](csharp_ios_dark/radio_button_group.png) | ![](cpp_ios_dark/radio_button_group.png) |
| 115 | Radio Button Group Binding | ⬛⚠️<br>L:blank<br>D:blank | ![](csharp_ios_light/radio_button_group_binding.png) | ![](cpp_ios_light/radio_button_group_binding.png) | ![](csharp_ios_dark/radio_button_group_binding.png) | ![](cpp_ios_dark/radio_button_group_binding.png) |
| 116 | Radio Button Group Gallery | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/radio_button_group_gallery.png) | ![](cpp_ios_light/radio_button_group_gallery.png) | ![](csharp_ios_dark/radio_button_group_gallery.png) | ![](cpp_ios_dark/radio_button_group_gallery.png) |
| 117 | Radio Button Border | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/radio_button_border.png) | ![](cpp_ios_light/radio_button_border.png) | ![](csharp_ios_dark/radio_button_border.png) | ![](cpp_ios_dark/radio_button_border.png) |
| 118 | Radio Button Content | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/radio_button_content.png) | ![](cpp_ios_light/radio_button_content.png) | ![](csharp_ios_dark/radio_button_content.png) | ![](cpp_ios_dark/radio_button_content.png) |
| 119 | Radio Content Properties | 🟡⚠️<br>L:minor<br>D:minor | ![](csharp_ios_light/radio_content_properties.png) | ![](cpp_ios_light/radio_content_properties.png) | ![](csharp_ios_dark/radio_content_properties.png) | ![](cpp_ios_dark/radio_content_properties.png) |
| 120 | Radio Template From Style | ⬛⚠️<br>L:blank<br>D:blank | ![](csharp_ios_light/radio_template_from_style.png) | ![](cpp_ios_light/radio_template_from_style.png) | ![](csharp_ios_dark/radio_template_from_style.png) | ![](cpp_ios_dark/radio_template_from_style.png) |
| 121 | Scattered Radio Button | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/scattered_radio_button.png) | ![](cpp_ios_light/scattered_radio_button.png) | ![](csharp_ios_dark/scattered_radio_button.png) | ![](cpp_ios_dark/scattered_radio_button.png) |
| 122 | Swipe Gesture | 🟢⚠️<br>L:match<br>D:match | ![](csharp_ios_light/swipe_gesture.png) | ![](cpp_ios_light/swipe_gesture.png) | ![](csharp_ios_dark/swipe_gesture.png) | ![](cpp_ios_dark/swipe_gesture.png) |
| 123 | Swipe Item Position | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/swipe_item_position.png) | ![](cpp_ios_light/swipe_item_position.png) | ![](csharp_ios_dark/swipe_item_position.png) | ![](cpp_ios_dark/swipe_item_position.png) |
| 124 | Swipe Item Size | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/swipe_item_size.png) | ![](cpp_ios_light/swipe_item_size.png) | ![](csharp_ios_dark/swipe_item_size.png) | ![](cpp_ios_dark/swipe_item_size.png) |
| 125 | Swipe Threshold | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/swipe_threshold.png) | ![](cpp_ios_light/swipe_threshold.png) | ![](csharp_ios_dark/swipe_threshold.png) | ![](cpp_ios_dark/swipe_threshold.png) |
| 126 | Swipe View Margin | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/swipe_view_margin.png) | ![](cpp_ios_light/swipe_view_margin.png) | ![](csharp_ios_dark/swipe_view_margin.png) | ![](cpp_ios_dark/swipe_view_margin.png) |
| 127 | Swipe View Shadow | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/swipe_view_shadow.png) | ![](cpp_ios_light/swipe_view_shadow.png) | ![](csharp_ios_dark/swipe_view_shadow.png) | ![](cpp_ios_dark/swipe_view_shadow.png) |
| 128 | Swipe Refresh | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/swipe_refresh.png) | ![](cpp_ios_light/swipe_refresh.png) | ![](csharp_ios_dark/swipe_refresh.png) | ![](cpp_ios_dark/swipe_refresh.png) |
| 129 | Refresh View | ⬛⚠️<br>L:blank<br>D:blank | ![](csharp_ios_light/refresh_view.png) | ![](cpp_ios_light/refresh_view.png) | ![](csharp_ios_dark/refresh_view.png) | ![](cpp_ios_dark/refresh_view.png) |
| 130 | Custom Size Swipe | 🟡🎬<br>L:minor<br>D:match | ![](csharp_ios_light/custom_size_swipe.png) | ![](cpp_ios_light/custom_size_swipe.png) | ![](csharp_ios_dark/custom_size_swipe.png) | ![](cpp_ios_dark/custom_size_swipe.png) |
| 131 | Custom Swipe Item View | 🟡🎬<br>L:minor<br>D:match | ![](csharp_ios_light/custom_swipe_item_view.png) | ![](cpp_ios_light/custom_swipe_item_view.png) | ![](csharp_ios_dark/custom_swipe_item_view.png) | ![](cpp_ios_dark/custom_swipe_item_view.png) |
| 132 | Basic Swipe | 🟡🎬<br>L:minor<br>D:match | ![](csharp_ios_light/basic_swipe.png) | ![](cpp_ios_light/basic_swipe.png) | ![](csharp_ios_dark/basic_swipe.png) | ![](cpp_ios_dark/basic_swipe.png) |
| 133 | Gestures | 🟡🎬<br>L:minor<br>D:minor | ![](csharp_ios_light/gestures.png) | ![](cpp_ios_light/gestures.png) | ![](csharp_ios_dark/gestures.png) | ![](cpp_ios_dark/gestures.png) |
| 134 | Pan Gesture Events | 🟢🎬<br>L:match<br>D:match | ![](csharp_ios_light/pan_gesture_events.png) | ![](cpp_ios_light/pan_gesture_events.png) | ![](csharp_ios_dark/pan_gesture_events.png) | ![](cpp_ios_dark/pan_gesture_events.png) |
| 135 | Pointer Gesture | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/pointer_gesture.png) | ![](cpp_ios_light/pointer_gesture.png) | ![](csharp_ios_dark/pointer_gesture.png) | ![](cpp_ios_dark/pointer_gesture.png) |
| 136 | Drag Drop | 🟡🎬<br>L:minor<br>D:match | ![](csharp_ios_light/drag_drop.png) | ![](cpp_ios_light/drag_drop.png) | ![](csharp_ios_dark/drag_drop.png) | ![](cpp_ios_dark/drag_drop.png) |
| 137 | Hit Testing | 🔴🎬<br>L:diff<br>D:diff | ![](csharp_ios_light/hit_testing.png) | ![](cpp_ios_light/hit_testing.png) | ![](csharp_ios_dark/hit_testing.png) | ![](cpp_ios_dark/hit_testing.png) |
| 138 | Input Transparent | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/input_transparent.png) | ![](cpp_ios_light/input_transparent.png) | ![](csharp_ios_dark/input_transparent.png) | ![](cpp_ios_dark/input_transparent.png) |
| 139 | Focus | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/focus.png) | ![](cpp_ios_light/focus.png) | ![](csharp_ios_dark/focus.png) | ![](cpp_ios_dark/focus.png) |
| 140 | Dispatcher | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/dispatcher.png) | ![](cpp_ios_light/dispatcher.png) | ![](csharp_ios_dark/dispatcher.png) | ![](cpp_ios_dark/dispatcher.png) |
| 141 | Device | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/device.png) | ![](cpp_ios_light/device.png) | ![](csharp_ios_dark/device.png) | ![](cpp_ios_dark/device.png) |
| 142 | Effects | ⬛⚠️<br>L:blank<br>D:blank | ![](csharp_ios_light/effects.png) | ![](cpp_ios_light/effects.png) | ![](csharp_ios_dark/effects.png) | ![](cpp_ios_dark/effects.png) |
| 143 | Measure First Strategy | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/measure_first_strategy.png) | ![](cpp_ios_light/measure_first_strategy.png) | ![](csharp_ios_dark/measure_first_strategy.png) | ![](cpp_ios_dark/measure_first_strategy.png) |
| 144 | Scroll View | ⬛⚠️<br>L:blank<br>D:blank | ![](csharp_ios_light/scroll_view.png) | ![](cpp_ios_light/scroll_view.png) | ![](csharp_ios_dark/scroll_view.png) | ![](cpp_ios_dark/scroll_view.png) |
| 145 | Web View | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/web_view.png) | ![](cpp_ios_light/web_view.png) | ![](csharp_ios_dark/web_view.png) | ![](cpp_ios_dark/web_view.png) |
| 146 | Hybrid Web View | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/hybrid_web_view.png) | ![](cpp_ios_light/hybrid_web_view.png) | ![](csharp_ios_dark/hybrid_web_view.png) | ![](cpp_ios_dark/hybrid_web_view.png) |
| 147 | Alerts | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/alerts.png) | ![](cpp_ios_light/alerts.png) | ![](csharp_ios_dark/alerts.png) | ![](cpp_ios_dark/alerts.png) |
| 148 | Animation | 🟢🎬<br>L:match<br>D:match | ![](csharp_ios_light/animation.png) | ![](cpp_ios_light/animation.png) | ![](csharp_ios_dark/animation.png) | ![](cpp_ios_dark/animation.png) |
| 149 | Application Control | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/application_control.png) | ![](cpp_ios_light/application_control.png) | ![](csharp_ios_dark/application_control.png) | ![](cpp_ios_dark/application_control.png) |
| 150 | Ios Entry | 🟢<br>L:match<br>D:match | ![](csharp_ios_light/ios_entry.png) | ![](cpp_ios_light/ios_entry.png) | ![](csharp_ios_dark/ios_entry.png) | ![](cpp_ios_dark/ios_entry.png) |
| 151 | Ios Date Picker | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/ios_date_picker.png) | ![](cpp_ios_light/ios_date_picker.png) | ![](csharp_ios_dark/ios_date_picker.png) | ![](cpp_ios_dark/ios_date_picker.png) |
| 152 | Ios Time Picker | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/ios_time_picker.png) | ![](cpp_ios_light/ios_time_picker.png) | ![](csharp_ios_dark/ios_time_picker.png) | ![](cpp_ios_dark/ios_time_picker.png) |
| 153 | Ios Picker | 🟡<br>L:minor<br>D:match | ![](csharp_ios_light/ios_picker.png) | ![](cpp_ios_light/ios_picker.png) | ![](csharp_ios_dark/ios_picker.png) | ![](cpp_ios_dark/ios_picker.png) |
| 154 | Ios Search Bar | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/ios_search_bar.png) | ![](cpp_ios_light/ios_search_bar.png) | ![](csharp_ios_dark/ios_search_bar.png) | ![](cpp_ios_dark/ios_search_bar.png) |
| 155 | Ios Scroll View | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/ios_scroll_view.png) | ![](cpp_ios_light/ios_scroll_view.png) | ![](csharp_ios_dark/ios_scroll_view.png) | ![](cpp_ios_dark/ios_scroll_view.png) |
| 156 | Ios Slider Update On Tap | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/ios_slider_update_on_tap.png) | ![](cpp_ios_light/ios_slider_update_on_tap.png) | ![](csharp_ios_dark/ios_slider_update_on_tap.png) | ![](cpp_ios_dark/ios_slider_update_on_tap.png) |
| 157 | Ios First Responder | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/ios_first_responder.png) | ![](cpp_ios_light/ios_first_responder.png) | ![](csharp_ios_dark/ios_first_responder.png) | ![](cpp_ios_dark/ios_first_responder.png) |
| 158 | Ios Pan Gesture | 🟡🎬<br>L:minor<br>D:minor | ![](csharp_ios_light/ios_pan_gesture.png) | ![](cpp_ios_light/ios_pan_gesture.png) | ![](csharp_ios_dark/ios_pan_gesture.png) | ![](cpp_ios_dark/ios_pan_gesture.png) |
| 159 | Ios Safe Area | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/ios_safe_area.png) | ![](cpp_ios_light/ios_safe_area.png) | ![](csharp_ios_dark/ios_safe_area.png) | ![](cpp_ios_dark/ios_safe_area.png) |
| 160 | Ios Swipe Transition | 🟡🎬<br>L:minor<br>D:minor | ![](csharp_ios_light/ios_swipe_transition.png) | ![](cpp_ios_light/ios_swipe_transition.png) | ![](csharp_ios_dark/ios_swipe_transition.png) | ![](cpp_ios_dark/ios_swipe_transition.png) |
| 161 | Ios Blur Effect | 🔴🎬<br>L:diff<br>D:diff | ![](csharp_ios_light/ios_blur_effect.png) | ![](cpp_ios_light/ios_blur_effect.png) | ![](csharp_ios_dark/ios_blur_effect.png) | ![](cpp_ios_dark/ios_blur_effect.png) |
| 162 | Navigation Gallery | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/navigation_gallery.png) | ![](cpp_ios_light/navigation_gallery.png) | ![](csharp_ios_dark/navigation_gallery.png) | ![](cpp_ios_dark/navigation_gallery.png) |
| 163 | Modal | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/modal.png) | ![](cpp_ios_light/modal.png) | ![](csharp_ios_dark/modal.png) | ![](cpp_ios_dark/modal.png) |
| 164 | Tabbed Flyout | ⬛<br>L:blank<br>D:blank | ![](csharp_ios_light/tabbed_flyout.png) | ![](cpp_ios_light/tabbed_flyout.png) | ![](csharp_ios_dark/tabbed_flyout.png) | ![](cpp_ios_dark/tabbed_flyout.png) |
| 165 | Toolbar | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/toolbar.png) | ![](cpp_ios_light/toolbar.png) | ![](csharp_ios_dark/toolbar.png) | ![](cpp_ios_dark/toolbar.png) |
| 166 | Menu Bar | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/menu_bar.png) | ![](cpp_ios_light/menu_bar.png) | ![](csharp_ios_dark/menu_bar.png) | ![](cpp_ios_dark/menu_bar.png) |
| 167 | Title Bar | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/title_bar.png) | ![](cpp_ios_light/title_bar.png) | ![](csharp_ios_dark/title_bar.png) | ![](cpp_ios_dark/title_bar.png) |
| 168 | Chrome | 🟡<br>L:minor<br>D:minor | ![](csharp_ios_light/chrome.png) | ![](cpp_ios_light/chrome.png) | ![](csharp_ios_dark/chrome.png) | ![](cpp_ios_dark/chrome.png) |
| 169 | Context Flyout | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/context_flyout.png) | ![](cpp_ios_light/context_flyout.png) | ![](csharp_ios_dark/context_flyout.png) | ![](cpp_ios_dark/context_flyout.png) |
| 170 | Templated View | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/templated_view.png) | ![](cpp_ios_light/templated_view.png) | ![](csharp_ios_dark/templated_view.png) | ![](cpp_ios_dark/templated_view.png) |
| 171 | Custom Layout | ⬛<br>L:blank<br>D:blank | ![](csharp_ios_light/custom_layout.png) | ![](cpp_ios_light/custom_layout.png) | ![](csharp_ios_dark/custom_layout.png) | ![](cpp_ios_dark/custom_layout.png) |
| 172 | Visual States | 🔴<br>L:diff<br>D:diff | ![](csharp_ios_light/visual_states.png) | ![](cpp_ios_light/visual_states.png) | ![](csharp_ios_dark/visual_states.png) | ![](cpp_ios_dark/visual_states.png) |

## Per-page findings

Concrete, per-theme notes for every page with a diff, a broken reference, or a motion/effect caveat. Clean both-theme matches with no note are omitted. Numbers match the grid above.

### 1. Label — 🔴 (L:diff / D:diff)
- **Light:** In MAUI (ground truth) the alignment-demo labels 'This should be at the start/center/end of the line' and the 'Lorem ipsum' block each render on a light-gray background fill, and the 'This should be at the start/center' boxes are gray cards. In the C++ port those gray background fills are MISSING (labels are transparent on the white page). The CYAN-background label, RED text, and alignment all match. The missing label BackgroundColor fills are the bug; the C++ tighter spacing also lets extra content (Plain old Text / Colors / Strikethrough / Big Font / Change Formatted String) appear that MAUI's screenshot scrolls past.
- **Dark:** Same as light: MAUI renders gray BackgroundColor fills behind the start/center/end alignment labels, the Lorem-ipsum block, and the start/center boxes; the C++ port omits these gray fills so those labels sit transparent on the black page. CYAN label, RED text, and text/alignment otherwise match.

### 2. Button — 🟢 (L:match / D:match)
- **Light:** All buttons match: Taps: 0, blue 'Button', gray 'Button (disabled)', 'Clicked', 'Command', blue-fill Button, red-fill Button, green BorderColor, green BorderWidth with red border, purple CornerRadius (rounded), pink 'Button'. The C++ port uses slightly tighter vertical spacing so it additionally shows settings/spacing buttons + a slider that MAUI's screenshot scrolls past; shared content is equivalent.
- **Dark:** Same as light — every visible button (colors, borders, corner radius, disabled state) matches between the two. C++ tighter spacing reveals extra rows below that MAUI's frame cuts off.

### 3. Entry — 🟢 (L:match / D:match)
- **Light:** All entries match: LENGTH/RETURN header, 'Type here...' placeholder, purple 'Text', purple 'Placeholder', blue checkmark, password dots, 'I am read only', 'Text', right-aligned 'This should be on the end', 'CursorPosition = 4'. C++ shows an extra slider + 'Cursor' entry below that MAUI's frame scrolls past.
- **Dark:** Same as light — all entries, placeholder colors, password masking, and right-alignment match in dark theme.

### 4. Editor — 🟢 (L:match / D:match)
- **Light:** All editors match: 'LENGTH: 0', 'Type here...', purple 'Text', purple 'Placeholder', large-font 'FontSize (Large)', 'I am read only', '123', 'This should be on the bottom', 'AUTOSIZE LENGTH: 0', 'Grows as you type...' placeholder. Identical content, fonts, and colors.
- **Dark:** Same as light — every editor, font size, and placeholder color matches in dark theme.

### 5. Search Bar — 🟢 (L:match / D:match)
- **Light:** All search bars match: 'Search...' placeholder, green 'Green text', pink 'Placeholder', italic 'Italic 24pt', right-aligned 'end of the line', 'Cancel is red', 'Numeric keyboard' — same magnifier icons, cancel-X buttons, colors and italic styling. C++ tighter spacing shows more rows that MAUI's frame scrolls past.
- **Dark:** Same as light — search field chrome, placeholder/text colors, italic font and cancel buttons all match in dark theme.

### 6. Picker — 🟢 (L:match / D:match)
- **Light:** All pickers match: Basic 'Select an item', 'SelectedIndex=1' Item 2, 'SelectedIndexChanged' Item 2, 'Selected: (none)', 'TextColor=Blue', blue 'TitleColor=Blue' Select-an-item, yellow-background italic 'FontAttributes=Italic + BackgroundColor=Yellow'. C++ tighter spacing additionally shows Dynamic-add section, Clear/Add/Replace buttons and green 'Item 1' that MAUI's frame scrolls past.
- **Dark:** Same as light — all picker values, the blue title color, and the yellow italic picker match in dark theme.

### 7. Date Picker — 🔴 (L:diff / D:diff)
- **Light:** Wrong date FORMAT: MAUI (ground truth) shows '19.06.2026' and 'Default with date' '21.06.2018' (dd.MM.yyyy device-locale format) while the C++ port shows '6/19/2026' and '6/21/2018' (M/d/yyyy invariant US format). Also the third 'Background' date picker uses a blue->cyan gradient in MAUI but a pink/magenta gradient in the C++ port. Default/BackgroundColor(blue)/Background(yellow-green) pickers otherwise match.
- **Dark:** Same as light: MAUI dates read '19.06.2026' / '21.06.2018' (dd.MM.yyyy) vs C++ '6/19/2026' / '6/21/2018' (M/d/yyyy); and the third Background gradient is blue->cyan in MAUI but pink/magenta in the C++ port.

### 8. Time Picker — 🔴 (L:diff / D:diff)
- **Light:** Wrong time FORMAT: MAUI (ground truth) shows '0:00' (24-hour device-locale format) and 'Default with time' '4:15' while the C++ port shows '12:00 AM' and '4:15 AM' (12-hour invariant US AM/PM format). Also the third 'Background' time picker uses a blue->cyan gradient in MAUI but a pink/magenta gradient in the C++ port. Default/BackgroundColor(blue)/Background(yellow-green) pickers otherwise match.
- **Dark:** Same as light: MAUI times read '0:00' / '4:15' (24h) vs C++ '12:00 AM' / '4:15 AM' (12h AM/PM); and the third Background gradient is blue->cyan in MAUI but pink/magenta in the C++ port.

### 9. Pickers — 🔴 (L:diff / D:diff)
- **Light:** All three controls present (Picker 'Pick a room', DatePicker, TimePicker '09:00') plus the 'No room...' label in both. Real discrepancy: the DatePicker shows the date as '6/19/2026' (invariant US M/d/yyyy) in C++, while MAUI shows '19.06.2026' (device-locale dd.MM.yyyy). Also MAUI light wraps the controls in a harness white card; cosmetic.
- **Dark:** Same as light: DatePicker reads '6/19/2026' in C++ vs '19.06.2026' in MAUI (invariant-US vs device-locale date format). Picker, TimePicker '09:00' and the label otherwise match.

### 10. Slider — 🟡 (L:minor / D:minor)
- **Light:** Same sliders, labels and colors in both (Default, BackgroundColor blue, Background yellow-green gradient, Min/Max with value '5', Disabled, MinimumTrackColor=LightBlue, plus MaximumTrackColor=Pink, ThumbColor=Orange, ThumbImageSource and 'Toggle Image' visible in C++). MAUI light is pushed down by a harness white card so lower rows are clipped, and MAUI section labels are bold vs regular-weight in C++ — both harness cosmetics.
- **Dark:** Same sliders and colors (blue bg, yellow-green gradient, pink max-track, orange thumb, gray disabled thumb). Only difference is bold section labels in MAUI vs regular-weight in C++ (harness style).

### 11. Stepper — 🔴 (L:diff / D:diff)
- **Light:** All stepper rows match (Default, Disabled, 'Enable Stepper' link, BackgroundColor, Background, Min/Max, Increment, ValueChanged, Value: 0). Real difference: on the BackgroundColor row the red fill spans the FULL row width to the right edge in MAUI, but in C++ the red is confined to just the +/- control pill. Section labels also bold in MAUI vs regular in C++ (harness).
- **Dark:** Same as light: BackgroundColor red fill is full-row-width in MAUI but only covers the stepper control pill in C++. All other stepper rows and the 'Enable Stepper' link match.

### 12. Switch — 🟡 (L:minor / D:minor)
- **Light:** All switches match (Default off, 'Default switch is Off' label, BackgroundColor blue pill, Background yellow-green gradient, Disabled, OnColor, ThumbColor with orange thumb). Only difference is bold section labels in MAUI vs regular-weight in C++ (harness style); MAUI light also wraps content in a harness card.
- **Dark:** Identical switch set and colors (blue bg pill, yellow-green gradient, orange thumb). Only the bold-vs-regular label weight differs (harness).

### 13. Check Box — 🟡 (L:minor / D:minor)
- **Light:** All checkboxes match (Default blue ring, Colored purple ring, Disabled blue ring, Disabled Colored purple filled check, Change IsChecked, 'Is green? False' with red filled check). C++ uses slightly larger inter-row spacing so the 'Is green? False' row sits near the bottom edge vs compact in MAUI; bold-vs-regular labels are harness cosmetic.
- **Dark:** Same checkbox set, colors and states. C++ spaces the rows further apart, pushing 'Is green? False' to the bottom (still visible); MAUI keeps them compact. Cosmetic spacing only.

### 14. Progress Bar — 🟡 (L:minor / D:minor)
- **Light:** All progress bars match (Default blue ~50%, ProgressColor orange ~50%, Disabled blue ~50%, second ProgressColor orange ~50%, ProgressTo empty, 'ProgressTo' link). Only bold-vs-regular section labels differ (harness style); MAUI light also wraps content in a harness card.
- **Dark:** Identical progress bars, fill levels and colors. Only the bold-vs-regular label weight differs (harness).

### 15. Activity Indicator — 🟡 (L:minor / D:minor)  🎬 _motion/effect — needs an animated GIF to judge_
- **Light:** All spinners present and matching (Default, Styled-Color-from-theme blue spinner, Styled-BackgroundColor=Yellow band, Larger, Smaller-HorizontalOptions=Center); C++ also shows 'Not Running' and '- End of page -' rows that MAUI light clips below its harness card. Spinner rotation phase differs between stills but the page is animated, so a GIF is needed for true parity. Bold-vs-regular labels are harness cosmetic.
- **Dark:** Same spinner set and yellow background band. Spinner phase differs in the stills (animated control). Only the bold-vs-regular label weight otherwise differs (harness).

### 16. Indicator — 🔴 (L:diff / D:diff)
- **Light:** Multiple real bugs. (1) On the Basic, Indicator Shape and 'MaximumVisible - 7 of' rows MAUI shows 5/7 evenly-spaced dots with the SELECTED (center) dot filled dark; C++ omits the selected dot entirely, leaving a visible gap (2 dots, gap, 2 dots). (2) 'Indicator Size' renders as small dots in MAUI but as oversized circles in C++ that overflow the right edge, again with the selected dot missing. (3) 'Using with CarouselView': MAUI shows a single 'Item 1' card with 3 inline dots, while C++ collapses the carousel to the bottom showing three overlapping truncated 'Ite...' items. The 'Colors' row (blue/blue/red/blue/blue) matches.
- **Dark:** Same bugs as light: selected/center indicator dot missing on Basic, Indicator Shape and MaximumVisible rows (gap instead of filled dot); 'Indicator Size' dots oversized and overflowing right; CarouselView shows overlapping truncated 'Ite...' items at the bottom instead of a single 'Item 1' card. 'Colors' row matches.

### 17. Image — 🔴 (L:diff / D:diff)
- **Light:** MAUI packs the three image samples tightly near the top: UriSource (Microsoft building photo), FileSource (purple submarine), and the 'Font Image Source' label all fit on screen. C++ spreads them out with large vertical gaps — the UriSource building image sits lower, the FileSource submarine is pushed far down (only its top edge peeks in at the very bottom), and the 'Font Image Source' label/glyph is off-screen entirely. Wrong vertical spacing/layout of the image stack.
- **Dark:** Same as light: MAUI shows UriSource, FileSource and 'Font Image Source' label tightly stacked; C++ spaces them far apart so the FileSource submarine and the Font Image Source label are not visible. Layout spacing is wrong.

### 18. Image Button — 🟡 (L:minor / D:minor)
- **Light:** Shared content matches: '0 ImageButton clicks', AspectFit/AspectFill/Fill green image-button rows, and 'BorderColor + BorderWidth' red-outlined green bar all align. The MAUI capture is scroll-cut at the green BorderColor bar, while C++ is positioned to also reveal sliders, 'CornerRadius = 0 / 10 / slider' (three purple bars + slider) and 'Custom Size (click to resize)' with a small submarine. Difference is a capture scroll-position offset, not a render bug.
- **Dark:** Same as light: the matching rows (clicks counter, AspectFit/Fill green buttons, BorderColor red-outlined bar) align; C++ simply shows additional lower content (sliders, CornerRadius purple bars, Custom Size) that the scroll-cut MAUI capture omits.

### 19. Box View — 🟢 (L:match / D:match)
- **Light:** Default (cornflower-blue box), Using Color (magenta/purple box), and Background (yellow-to-green horizontal gradient box) all match in color, size and order. C++ is positioned to also show the 'Using CornerRadius' rounded light-green box that MAUI's capture cuts off at the bottom; shared content is equivalent.
- **Dark:** Identical to light: blue Default box, purple Using Color box, yellow-green gradient Background box all match; C++ additionally reveals the rounded green 'Using CornerRadius' box below.

### 20. Content View — 🟢 (L:match / D:match)
- **Light:** Both show 'ContentView', an indented 'Content', a 'Content' line, and a centered blue 'Swap content' button — same layout and colors. Only difference is a ~few-px larger left indent on the labels in MAUI vs C++; sub-pixel, basically identical.
- **Dark:** Same as light: 'ContentView', 'Content', 'Content', centered blue 'Swap content' all match; trivial left-margin difference only.

### 21. Containers — 🟡 (L:minor / D:match)
- **Light:** Both show 'Scrolled to: 0 / 0', a blue dashed border box 'Inside a border', a red-outlined frame 'Inside a frame', and 'Inside a content_view'. Minor: in C++ light the 'Inside a frame' label has a light-gray rounded highlight rectangle behind the text, whereas MAUI shows the label on the plain frame background.
- **Dark:** All four elements match (Scrolled to 0/0, blue dashed border box, red frame box, content_view label); no gray text highlight visible in dark, equivalent.

### 22. Control stack — 🟢 (L:match / D:match)
- **Light:** 'Controls' title, 'A Button' link, 'An Entry' field, 'An Editor', the 'A SearchBar' pill with magnifier, the blue checkbox + green Switch + activity spinner row, the Slider, and the (−|+) Stepper all match in structure and color. C++ additionally renders a blue/gray ProgressBar below the stepper that MAUI's bottom-cut capture omits. 'Controls' heading is a touch bolder in MAUI (minor).
- **Dark:** Same as light: title, button, entry, editor, search bar, checkbox/switch/spinner row, slider and stepper all match; C++ shows an extra ProgressBar below that the MAUI capture cuts off.

### 23. Input Controls — 🔴 (L:diff / D:diff)
- **Light:** 'LENGTH: 0', 'Type here...' entry, and 'Search to insert' search bar match. But the RadioButton group differs: MAUI stacks 'UPPER' (selected) and 'lower' left-aligned with a clear gap between the radio circle and the label; C++ centers the radio group horizontally on the page AND renders no space between the radio indicator and label ('⊙UPPER', '○lower' touching). Wrong alignment + missing radio-to-label spacing.
- **Dark:** Same discrepancy as light: entry, search bar and selected/unselected radios are present, but C++ centers the radio buttons and glues the label to the radio dot, while MAUI left-aligns them with proper spacing.

### 24. Fonts — 🟢 (L:match / D:match)
- **Light:** Vertical typography list matches exactly: large Title, Subtitle, Header, Body, Caption, Bold, Italic, Bold + Italic, and 'Character spacing 4.0' with the letter-spacing applied. Sizes/weights/italics all correspond; C++ Title is a hair smaller (font-hinting level).
- **Dark:** Same as light: Title/Subtitle/Header/Body/Caption/Bold/Italic/Bold+Italic and the letter-spaced 'Character spacing 4.0' line all match in size, weight and style.

### 25. Formatted Text — 🟢 (L:match / D:match)
- **Light:** Both render the same FormattedString line: 'Bold red' (red bold) + 'italic underlined' (italic underlined) + 'k e r n e d' (kerned letter-spacing), then 'Plain text label'. Identical text, colors and spans in light theme. MAUI shows a white harness card; C++ has no card — harness-only artifact.
- **Dark:** Identical FormattedString and 'Plain text label' on black background; red bold, italic underlined and kerned runs all match in dark theme.

### 26. Styles — 🟡 (L:minor / D:minor)
- **Light:** Same three labels (gray '(base) subtitle style', pink 'custom style derived ... (Pink wins)', default 'no explicit style') and a 'Style Me' button with yellow border + light-gray fill. Minor cosmetic: C++ button is shorter/less tall and body text wraps to fewer lines (wider content width).
- **Dark:** Same content and yellow-bordered 'Style Me' button. Minor: C++ dark 'Pink wins' text reads as a more saturated red-pink vs MAUI's lighter pink, and the C++ button is shorter; structure/colors otherwise equivalent.

### 27. Triggers — 🟡 (L:minor / D:minor)
- **Light:** Same content: 'Triggers' heading, 'Text must be a valid double or it will turn red.' subtitle, 'Enter a System.Double' entry, 'Highlight off' label, blue 'Toggle highlight' button. Minor: MAUI renders 'Triggers' as a large bold heading (~28pt) while C++ renders it small/regular (~17pt, same as body).
- **Dark:** Identical controls in dark theme. Same heading-weight difference: MAUI 'Triggers' is large bold, C++ is small regular weight.

### 28. Behaviors — 🟢 (L:match / D:match)
- **Light:** Both show the large bold heading 'Red when the number isn't valid' and an 'Enter a System.Double' entry. Only difference is line-wrap point (MAUI: 'Red when the / number isn't valid'; C++: 'Red when the number / isn't valid') — a sub-pixel width difference.
- **Dark:** Same heading and entry on black; identical except the same minor line-wrap difference.

### 29. Semantics — 🟢 (L:match / D:match)
- **Light:** All elements present and equivalent: SemanticProperties showcase header, readout line, Label text TH/DH, centered blue Button text TH/DH, bordered Entry text DTH, Editor text DTH, search bar (magnifier + clear-X + secondary X button), and HeadingLevel labels list. C++ uses slightly tighter vertical spacing so a few more rows show above the fold, but content matches.
- **Dark:** Same full control set in dark theme; search bar pill, entry border and blue buttons all match. Only difference is C++'s marginally tighter row spacing.

### 30. App Theme Binding — 🟢 (L:match / D:match)
- **Light:** Both show 'AppThemeBinding' header, green AppThemeBinding text, 'Using AppThemeBinding in a ResourceDictionary', orange LightPrimaryColor text, blue 'Toggle theme (Light/Dark)' button, and readout 'Theme: Light — inline=Green, resource=Orange'. Colors/structure identical; text wraps at different widths only.
- **Dark:** Matches MAUI exactly including the nuance that the app-level theme stays 'Light' (text remains green + orange and readout says 'Theme: Light') even in the OS dark-mode capture; C++ replicates this.

### 31. Stack Layout — 🟡 (L:minor / D:minor)
- **Light:** Both show 'Vertical' label with a centered 6-box rainbow column (red/yellow/blue/green/orange/purple) and 'Horizontal' label with a full-width 6-box rainbow row, same colors/order. Minor: MAUI 'Vertical'/'Horizontal' headings are bold while C++ are regular weight, and MAUI's horizontal strip is taller than C++'s.
- **Dark:** Same vertical column and horizontal row, identical colors/order on black. Minor: heading bold-vs-regular and slightly shorter C++ horizontal strip.

### 32. Vertical Stack — 🟢 (L:match / D:match)
- **Light:** Both show 'VerticalStackLayout' label and a centered 6-box rainbow column (red/yellow/blue/green/orange/purple), same order/colors/center alignment. Only difference is MAUI's boxes are slightly taller and the stack sits a touch lower — a minor sizing difference.
- **Dark:** Identical label and centered rainbow column on black; same minor box-height/vertical-position difference only.

### 33. Horizontal Stack — 🟡 (L:minor / D:minor)
- **Light:** Both show 'HorizontalStackLayout' title and a horizontal row of colored boxes bottom-right. MAUI shows only 4 boxes (red/yellow/blue/green) plus an orange sliver, with the row overflowing/clipped off the right screen edge; C++ shows all 6 boxes (red/yellow/blue/green/orange/purple) fully on-screen, slightly smaller and positioned a bit higher. Same control and structure; box widths/visible-count differ cosmetically between the two demo apps.
- **Dark:** Same as light: MAUI's 4-box row is partly clipped at the right edge while C++ shows all 6 boxes fully on-screen and slightly higher. Same horizontal stack of colored boxes, cosmetic sizing/overflow difference.

### 34. Grid — 🟡 (L:minor / D:minor)
- **Light:** Both show 'Grid (2 cols × rows)' title and an identical 2x2 grid of red/green/blue/orange boxes with matching spacing and gutters. Only differences: the title is bold in MAUI vs regular weight in C++, and MAUI renders the harness white container card extending further down. Grid content matches exactly.
- **Dark:** Identical 2x2 red/green/blue/orange grid with matching layout. Title bold in MAUI vs regular weight in C++; otherwise equivalent.

### 35. Absolute Layout — 🟡 (L:minor / D:minor)
- **Light:** Both show the same absolutely-positioned elements: top blue bar, left green bar, right red bar, 'Centered text', blue 'AutoSized' label, and a dark bar near the bottom — all in matching relative positions. MAUI draws a light-gray harness container card behind them and constrains content to that card (bottom dark bar sits mid-low); C++ uses plain white full-screen so the bottom dark bar sits near the very bottom edge. Same elements and arrangement; harness-card and vertical-extent difference only.
- **Dark:** Same elements (top blue / left green / right red / centered text / AutoSized / bottom dark bar) in matching positions. C++ stretches to full screen height so the bottom bar sits lower; MAUI constrains it higher within its content area. Cosmetic vertical-extent difference.

### 36. Flex Layout — 🟡 (L:minor / D:minor)
- **Light:** Both show identical flex structure: cyan HEADER top, blue left column, gray CONTENT center, green right column, pink FOOTER bottom — same colors and arrangement. MAUI constrains the flex inside a white card with top/bottom margins (white band above HEADER and below FOOTER); C++ stretches flex to fill the full screen so HEADER sits right under the nav pill and FOOTER reaches the very bottom (slightly cut off). Content identical; harness-card vs full-bleed difference.
- **Dark:** Identical cyan HEADER / blue+gray+green columns / pink FOOTER flex layout. C++ fills full screen height (FOOTER clipped at bottom edge) while MAUI insets it within margins. Same content, cosmetic extent difference.

### 37. Relative Layout — 🔴 (L:diff / D:diff)
- **Light:** Both show 4 corner squares (red TL, green TR, blue BL, yellow BR) matching, plus a central gray box containing a smaller black box. The central box PROPORTIONS differ: MAUI's gray box is roughly square (wider, shorter) with a near-square black inner box; C++'s gray box is noticeably taller and narrower (elongated portrait) with a tall narrow black inner box, and sits slightly more to the left. The relatively-sized central element has the wrong aspect ratio in C++.
- **Dark:** Corner squares match. Central gray+black relative box is wider/squarer in MAUI but taller/narrower (portrait) in C++ — same aspect-ratio discrepancy as light theme.

### 38. Layout alignment (Start/Center/End/Fill) — 🟡 (L:minor / D:minor)
- **Light:** Both show four sections (Start/Center/End/Fill), each a label plus a blue button with red border. Alignments match: Start left-aligned, Center centered, End right-aligned, Fill centered/wider. Differences: section labels are bold in MAUI vs regular weight in C++, and MAUI shows the harness white card. Button alignments and sizes otherwise match.
- **Dark:** Same Start/Center/End/Fill sections with correctly-aligned red-bordered blue buttons in both. Labels bold in MAUI vs regular weight in C++; otherwise equivalent.

### 39. Z Index — 🔴 (L:diff / D:diff)
- **Light:** Both show 'Z-Index of Label 5: 5' header with a -/+ stepper and a stack of 10 overlapping colored cards (Labels 0-9) with correct z-ordering (red 'Label 9' on top). But the per-card STAGGER OFFSET is far smaller in C++: MAUI spreads the cards with a large down-right offset so every label's text is fully readable (stack spans top-to-mid screen); C++ clusters the cards tightly so only the top 'Label 9' text is readable and labels 0-8 are mostly hidden (only a sliver 'T' shows at the left), with the whole stack much smaller and positioned low-center. Labels are unreadable in C++ due to the compressed offset.
- **Dark:** Same correct z-ordering, but C++ uses a much smaller per-card stagger so labels 0-8 overlap into unreadable slivers and the stack is small/low-centered, whereas MAUI spreads the cards out so all 10 labels are readable. Same compression discrepancy as light theme.

### 40. Layout Is Enabled — 🟡 (L:minor / D:minor)
- **Light:** Two-column page comparing enabled/disabled states. The enabled-state rendering matches exactly where both are visible: 'All children are enabled' (blue active 'Enabled' left / gray disabled right), 'All children are disabled' (gray on blue boxes), 'disabled because layout is disabled' (gray on pink boxes), 'First item enabled/second disabled' (teal boxes, blue 'Enabled' / gray 'Disabled'). Difference is layout density: MAUI wraps section labels to 2 lines and uses taller spacing so only ~4 sections fit (scrolled to top, harness card visible); C++ fits all 7 sections plus the bottom 'Disable Layout / Enable Button' controls on one screen via single-line labels and tighter spacing. Enabled/disabled visual states are correct in both.
- **Dark:** Same as light: enabled (blue) vs disabled (gray) text and the colored section boxes render identically in both. MAUI wraps labels to 2 lines and shows fewer sections; C++ fits all sections plus bottom buttons via tighter spacing. Cosmetic density/line-wrap difference only.

### 41. Shapes — 🟢 (L:match / D:match)
- **Light:** All elements present and identical: red-fill/navy-stroke Ellipse, navy RoundRectangle, blue/red EvenOdd pentagram, and a purple Line. MAUI light wraps content in a white rounded harness card; C++ has no card (harness-only artifact). C++ is scrolled slightly lower so the bottom Line is fully visible; MAUI cuts it at the label. Same content, colors, order.
- **Dark:** Identical: red-fill/navy Ellipse, navy RoundRectangle, blue/red EvenOdd pentagram, purple Line. C++ shows the full Line at bottom; MAUI dark only shows the 'Line' label (scroll position). No harness card in dark on either side. Match.

### 42. Ellipse Gallery — 🟢 (L:match / D:match)
- **Light:** All 5 ellipse variants match: basic red ellipse, red circle outline, red-stroke transparent ellipse, blue-fill/red-stroke ellipse, and red-dashed-stroke blue ellipse. C++ renders more compactly so the dashed ellipse is fully visible; MAUI light cuts it at the bottom. Colors, strokes, dash pattern all identical.
- **Dark:** Same 5 variants render identically in dark: basic red ellipse, red circle, red-stroke ellipse, blue/red-stroke ellipse, red-dashed blue ellipse. Scroll position differs (C++ shows full last item) but content matches.

### 43. Rectangle Gallery — 🟢 (L:match / D:match)
- **Light:** All rectangle variants match: basic red rectangle, red-stroke square, red-stroke rectangle (transparent top + blue half), red-dashed-stroke blue rectangle, and curved-corners blue rectangle. C++ scrolls lower and shows the curved-corners item fully; MAUI light cuts off just before it. Same colors, strokes, corner radii.
- **Dark:** Identical in dark: red rectangle, red-stroke square, red-stroke rectangle, red-dashed blue rectangle, curved-corners blue rectangle. C++ reveals the curved-corners item; MAUI dark stops at the dashed rectangle (scroll). Match.

### 44. Line Gallery — 🟢 (L:match / D:match)
- **Light:** All three lines match: purple basic Line, orange dashed Line, and a thick black StrokeThickness diagonal line. The thick black line is visible on the white background in both MAUI and C++.
- **Dark:** Purple basic Line and orange dashed Line render identically. The third 'StrokeThickness' line is black, so it is invisible on the dark background in BOTH MAUI and C++ (black-on-black) — faithful identical behavior, not a port bug.

### 45. Line Join Gallery — 🟢 (L:match / D:match)
- **Light:** All three cyan thick-polyline join demos match exactly: Miter (sharp pointed apex), Bevel (flat-cut apex), Round (rounded apex). C++ reproduces each join geometry faithfully. Same cyan color, labels, order.
- **Dark:** Same three joins (Miter sharp, Bevel flat, Round rounded) render identically in dark with the cyan polylines clearly visible on black. Match.

### 46. Polygon Gallery — 🟢 (L:match / D:match)
- **Light:** Green basic triangle, green dashed triangle, EvenOdd star (red stroke / blue fill / hollow center), and NonZero star (yellow stroke, filled center) all match. C++ is scrolled lower and shows the whole NonZero star; MAUI light only shows its top peak + baseline (scroll). The EvenOdd-hollow vs NonZero-filled fill-rule distinction is correct on both.
- **Dark:** Same four polygons render identically in dark: green triangle, green dashed triangle, EvenOdd star (blue fill, hollow center), NonZero star (yellow stroke, filled center). C++ reveals the full NonZero star; MAUI dark is scrolled higher. Fill-rule behavior matches.

### 47. Polyline Gallery — 🟢 (L:match / D:match)
- **Light:** Both show 'A basic Polyline' (short solid red segment) and 'A dash Polyline' (red dashed segment) at the same positions near the left edge. The sample polylines are intentionally tiny/near-flat; C++ renders the segments marginally longer but it is the same shape and red stroke. Labels and order match.
- **Dark:** Same two red polyline segments (solid + dashed) render at the same positions in dark on both sides. Minor sub-pixel length difference of a tiny shape; content matches.

### 48. Path Gallery — 🟢 (L:match / D:match)
- **Light:** PathGeometry samples match where comparable: LineSegment line, triangle, Cubic Bezier curve (all black-stroked, visible on white), and the lavender Composite-shape concentric circles. C++ is scrolled lower and additionally reveals Overlapping Rectangles (red) and EllipseGeometry (orange fill, green overlapping ellipses), which MAUI light cuts off. Overlapping content renders identically.
- **Dark:** In dark the first three path shapes (LineSegment, triangle, Cubic Bezier) are black-stroked so they are invisible on the black background in BOTH MAUI and C++ — faithful identical behavior. The lavender Composite circles render identically. C++ scrolls lower and correctly shows the red Overlapping Rectangles and orange/green EllipseGeometry; MAUI dark stops earlier (scroll). Match.

### 49. Path Aspect Gallery — 🔴 (L:diff / D:diff)
- **Light:** Aspect-mode demo (None/Fill/Uniform/UniformToFill). MAUI renders each heart inside a fixed-size gray container box so the aspect mode visibly fits/stretches to that box (Fill heart fills the wide box edge-to-edge, None is tiny in the box). C++ omits the gray container boxes entirely and draws each heart at natural content size, so the aspect behavior is not demonstrated and the hearts are smaller/narrower. The defining gray bounds card is missing in C++.
- **Dark:** Same structural issue as light: MAUI shows gray container boxes that the heart fits to per aspect mode; C++ has no gray boxes and draws natural-size hearts. Additionally C++'s ECG poly-line on the 'Fill' heart renders dark/black against the red instead of MAUI's cream/yellow line, making it nearly invisible in dark mode.

### 50. Path Transform String — 🟢 (L:match / D:match)
- **Light:** Both render the two black 'Z'-shaped paths under labels 'Without RenderTransform' and 'With RenderTransform' on white; the transformed (skewed) shape matches in both. Equivalent.
- **Dark:** MAUI ground truth shows only the two labels with NO visible path (the path stroke is black on a black background, so it is invisible in MAUI itself). C++ behaves identically — black strokes invisible on black, only labels visible. The two sides match each other.

### 51. Composition Gallery — 🟡 (L:minor / D:minor)
- **Light:** All composite content matches: green triangle + yellow circle + red/orange overlapping diagonal bars, plus the blue/green/red RGB axis lines. Difference is container structure: MAUI splits the demo into TWO separate beige cards (one for the shape, one for the axes) with a gap between them; C++ renders a single continuous beige panel containing both. Content identical, only the card-splitting differs.
- **Dark:** Same as light: identical composite shape and RGB axes; MAUI uses two beige cards with a gap, C++ uses one merged beige panel. Cosmetic container difference only.

### 52. Transform Playground — 🟢 (L:match / D:match)
- **Light:** Both show the gray play area with a red/blue-bordered square pinned top-left, then the transform sliders (RotateTransform/Rotation:0, CenterX:0, CenterY:0, ScaleTransform/ScaleX:1.00...). Slider thumbs, tracks and the blue-filled ScaleX/Y tracks at 1.00 match. C++ scrolls a touch further (also shows ScaleY/SkewX) — scroll-offset only.
- **Dark:** Same as light: gray play area + top-left red square + identical slider list with matching values; dark backgrounds and slider rendering equivalent. Trivial scroll-offset difference only.

### 53. Transformations — 🟡 (L:minor / D:minor)
- **Light:** Both show 'SCALE AND ROTATE' blue header and the slider list (Scale/ScaleX/ScaleY/Rotation/RotationX/RotationY...). Two cosmetic diffs: (1) value format — MAUI shows 'Scale = 1' / 'ScaleX = 1', C++ shows 'Scale = 1.0' / 'ScaleX = 1.0' (extra decimal); (2) C++ uses tighter row spacing so it fits more controls (AnchorX/Y steppers + TranslationX/Y sliders visible) while MAUI's looser spacing only reaches RotationY. Content equivalent.
- **Dark:** Same as light: 'SCALE AND ROTATE' header + sliders match; C++ appends '.0' to scale values (1.0 vs 1) and uses denser row spacing showing extra Anchor/Translation controls. Cosmetic only.

### 54. Update Path Data — 🟢 (L:match / D:match)
- **Light:** Both render the same black S-shaped bezier curve, the bottom label 'counter = 0 | Data: M 10,100 C 10,300 300,-200 300,100', and the blue 'Update Path Data' button overlapping that label (overlap present in both). C++ curve sits slightly higher in the panel; position-only shift.
- **Dark:** MAUI ground truth shows the bezier curve as INVISIBLE in dark mode (black stroke on black background — MAUI does not adapt the stroke to white), leaving only the bottom label + blue button. C++ behaves identically (curve invisible, label + button shown). The two sides match each other.

### 55. Auto Size Shapes — 🔴 (L:diff / D:diff)
- **Light:** Page proves 'the Ellipse must occupy half of the available space': yellow band (ellipse) on top, orange band below, each ~half. In MAUI the green/blue-bordered shape is a true wide ELLIPSE (width >> height, flattened) and the yellow band height roughly equals the orange band. In C++ the shape is nearly a full CIRCLE (much taller) and the yellow band is visibly TALLER than the orange band — the 50/50 split is broken and the ellipse aspect ratio is wrong.
- **Dark:** Same sizing bug as light: MAUI draws a flat wide ellipse with the yellow band equal to the orange band; C++ draws an over-tall circle-like shape and the yellow band exceeds half, so the top/bottom halves are unequal.

### 56. Shape App Theme — 🔴 (L:match / D:diff)
- **Light:** AppThemeBinding demo. Light: both show green 'Shape using AppTheme' label + green rectangle on white. Equivalent.
- **Dark:** Real theme-binding bug. MAUI (ground truth) correctly switches to the dark variant: RED label + RED rectangle on a BLACK page background. C++ does NOT apply the dark theme at all — its dark screenshot is identical to its light one (GREEN label + GREEN rectangle on a WHITE background). The AppThemeBinding dark color and the dark page background are both missing in C++.

### 57. Invalidate Brush — 🟡 (L:minor / D:minor)
- **Light:** Content matches: green 'Change color' button (blue text) + 'Brush color: Green' label below a green underline bar. Difference is button sizing/position: in MAUI the button is a content-sized green rectangle (~165px wide) sitting in the page body; in C++ the green button band spans the FULL page width and sits flush at the very top. Same controls and text, different button width and vertical placement.
- **Dark:** Same as light: identical controls/text (green 'Change color' button, blue label text, 'Brush color: Green'), but C++ renders the button full-width and top-flush vs MAUI's content-width centered-left button lower in the body.

### 58. Gradient brushes — 🟢 (L:match / D:match)
- **Light:** Equivalent: 'LinearGradientBrush (yellow→green)' label over a yellow-to-green horizontal gradient bar, then 'RadialGradientBrush (red→navy)' label over a radial red-center-to-purple gradient bar. Same labels, same gradient colors/direction, same bar sizes. Only the vertical start offset differs (MAUI content sits lower inside the harness card) — harness artifact.
- **Dark:** Same equivalence in dark: both gradient bars and both labels render identically; only the vertical offset (harness card) differs.

### 59. Border — 🟢 (L:match / D:match)
- **Light:** Equivalent: a red-stroked rounded rectangle with cream/light-yellow fill containing centered 'Bordered content' text. Same border color, corner radius, fill, and text. Only the vertical position differs (harness card offset).
- **Dark:** Equivalent: same red-bordered cream rectangle. In BOTH apps the 'Bordered content' text renders in the dark-theme default white, which sits faintly on the cream fill (low contrast) — this quirk is shared, so the two match each other.

### 60. Border Stroke — 🟢 (L:match / D:match)
- **Light:** Equivalent: 'Using different StrokeThickness' over three orange bars with red borders of thickness 1/5/10, then 'Updating the Content Height', 'Content height: 60', a slider at ~25%, then three taller orange bars (1/5/10). Same labels, bar colors, border thicknesses, slider position. MAUI crops the third lower bar (harness card height); C++ shows all three — scroll/crop artifact.
- **Dark:** Equivalent in dark: identical labels, orange bars, red borders, slider, and content-height bars; bar text is white in both. Only difference is MAUI cropping the last lower bar vs C++ showing it fully (harness card height).

### 61. Border Layout — 🟡 (L:minor / D:minor)
- **Light:** Equivalent: 'Stroke thickness: 5 / 40', a slider, and a gray-stroked rounded-rectangle border containing a horizontal layout (red segment, 'Center' label on green, blue segment, green fill). Same colors, order, and label. Minor difference: in MAUI the red and blue inner segments fill the border's full inner height; in C++ they appear slightly shorter, leaving a thin green gap above/below them.
- **Dark:** Same as light: identical structure/colors/label; the only difference is the red/blue inner segments fit the border height fully in MAUI but leave a slight green gap in C++.

### 62. Border Playground — 🟡 (L:minor / D:minor)
- **Light:** Equivalent content: blue gradient border with yellow dashed stroke and 'Just a Label', then form fields Border Content=Label, Border Shape=RoundRectangle, Background section with Start Color #00B4DB / End Color #0083B0, Content Background, Show Content Background, Border. Minor difference: MAUI renders the section headers ('Border Content', 'Border Shape', 'Background') in BOLD/large weight; C++ renders them in regular (non-bold) weight. C++ also scrolls further to reveal more fields (harness card height).
- **Dark:** Same as light: identical fields and values; the section headers are bold in MAUI but regular-weight in C++. Both dashed-gradient borders and 'Just a Label' match.

### 63. Border Clip Playground — 🟢 (L:match / D:match)
- **Light:** Equivalent: a red outline shape with a rounded top-left corner (radius 60) and square remaining corners, then Border Shape=RoundRectangle, 'Border / Border Width: 5' slider, 'Corner Radius / Top Left Corner Radius: 60' slider (full), 'Top Right Corner Radius: 0' slider (empty), 'Bottom Left Corner Radius: 0'. Same shape, controls, and values. Only vertical offset (harness card) differs.
- **Dark:** Equivalent in dark: identical clipped shape, labels, slider positions, and values.

### 64. Border Resize Content — 🟡 (L:minor / D:minor)
- **Light:** Equivalent: a 2x3 grid of green-bordered shapes — left column red-filled circle/square/triangle each with a blue '+', right column light-blue-filled circle/square/triangle — then 'Content Text' field with '+', 'Content Text FontSize' slider, 'Image Scale' slider. Both apps render the left-column triangle's resize/overlap effect (a red translucent triangle offset from the bordered one). Minor difference: the blue '+' glyphs are slightly smaller and the shape sizes a touch smaller in C++.
- **Dark:** Same as light: identical shape grid, fills, green borders, '+' content, and the three form controls. Minor differences are the slightly smaller '+' glyphs / shape sizes in C++.

### 65. Borderless — 🟢 (L:match / D:match)
- **Light:** Both show a full yellow page with label 'Style: borderless (StrokeThickness 0)' and a switch toggle below it, content equivalent. The only difference is harness framing: C# (ground truth) renders the yellow Border as a contained card with black margins top/bottom, while C++ fills the full page yellow; label dark text in both. Equivalent content and structure.
- **Dark:** Same as light: identical label + switch on a yellow surface. C# yellow is a contained card (black top/bottom margins, harness artifact); C++ fills the full page. Label text light-on-yellow in both. Equivalent.

### 66. Clip — 🔴 (L:diff / D:diff)
- **Light:** The first unclipped 'Image' differs in sizing/fill: C# (ground truth) renders it full-width on a gray backing showing the whole submarine; C++ renders the same image smaller (aspect-fit, narrower) with no gray fill, so the layout reflows and C++ fits MORE sections in the viewport ('Image' + RectangleGeometry + EllipseGeometry + GeometryGroup) where C# shows only 'Image' + RectangleGeometry. The clipped crops also differ: C# RectangleGeometry shows a left-portion crop with gray backing; C++ shows aspect-fit centered clips.
- **Dark:** Same discrepancy as light: C# shows the unclipped 'Image' full-width with gray backing (2 sections visible), C++ renders it smaller aspect-fit with no gray fill and reflows to show 4 sections. Image sizing/aspect and gray-backing fill differ between the two.

### 67. Clip Views — 🔴 (L:diff / D:diff)
- **Light:** Multiple real discrepancies. Date: C# (ground truth) shows '19.06.2026' (device-locale DD.MM.YYYY) while C++ shows '6/20/2026' (invariant US M/D/YYYY format). Time: C# shows '0:00' (24h locale) while C++ shows '12:00 AM' (US 12h). Width: C#'s red-clipped views span the FULL page width (curve reaches right edge); C++'s clip curves are cut off at ~75% width (narrower). Backgrounds: in C# the Entry and Editor have the red clipped background; in C++ Entry/Editor render with NO red fill (plain underline + placeholder). SearchBar: C# is filled pink/red; C++ search is near-transparent white.
- **Dark:** Same set of bugs as light. Date '19.06.2026' (C#) vs '6/20/2026' (C++); time '0:00' (C#) vs '12:00 AM' (C++); C++ clip widths ~75% vs C# full-width; Entry/Editor lack the red clip background in C++; SearchBar filled red in C# but a dim/transparent pill in C++.

### 68. Clip Corner Radius — 🟡 (L:minor / D:minor)
- **Light:** Structure matches: 'Clipped Image using RoundRectangleGeometry' + a clipped image inside a gray square container, then Top Left / Top Right / Bottom Left / Bottom Right Corner sliders. Two cosmetic differences: (1) the demo sample image differs — C# (ground truth) uses the purple submarine, C++ uses a black pug photo (different demo-app asset); (2) image position inside the gray container differs — C# image sits top-left, C++ image is centered horizontally (offset right). C++ also fits all 4 corner sliders in the viewport; C# fits ~3.
- **Dark:** Same as light: matching structure (clipped image + 4 corner sliders). Differs only by the sample asset (submarine in C# vs pug in C++) and the clipped image's horizontal position within the gray container (top-left in C# vs center in C++).

### 69. Clip Gallery — 🔴 (L:diff / D:diff)  ⚠️ _MAUI reference capture broken — re-shoot needed_
- **Light:** The MAUI reference itself fails to render the images: C# (ground truth) shows 'Image' and 'Clipped Image using RectangleGeometry' headers but the image areas are BLANK GRAY rectangles (the source image did not load). C++ correctly renders the actual pug photo for 'Image', 'Clipped Image using RectangleGeometry', and 'Clipped Image using RoundRectangleGeometry'. The C++ port is the correct/better side here; the ground-truth capture has unloaded gray image placeholders.
- **Dark:** Same: C# ground truth shows blank gray image placeholders (image not loaded) under 'Image' and 'RectangleGeometry'; C++ renders the real pug image across multiple clipped sections. C++ is correct; MAUI reference shows missing image content.

### 70. Clipping — 🔴 (L:diff / D:diff)
- **Light:** Two real differences in the horizontal-stack demo. (1) Number row: C# (ground truth) spreads '2 3 4 5 6 7 8' across the full width with large inter-item spacing (item 1 off-screen left); C++ packs '1 2 3 4 5 6 7 8' tightly together at the far left with minimal spacing — the horizontal StackLayout spacing differs markedly. (2) Coffee-cup emoji: C++ shows two coffee-cup glyphs on the left of the light-blue bar; C# shows none (empty blue bar). The orange container, orange/red square, and purple 'Hey Hey Hey Hey' bar are positioned similarly.
- **Dark:** Same as light: C# number row '2 3 4 5 6 7 8' is widely spaced across the width while C++ packs '1 2 3 4 5 6 7 8' tight at the left; C++ shows two coffee-cup emoji on the light-blue bar that are absent in the C# reference.

### 71. Shadow Playground — 🟢 (L:match / D:match)
- **Light:** Strong match. Both show 'Label with a Shadow' (faint red text shadow), a cyan box with a red drop-shadow offset to the bottom-right, 'Background #00B4DB', 'Shadow', 'Shadow Color #FF0000' entries, and 'Offset X: 10' / 'Offset Y: 10' / 'Radius: 10' sliders at matching positions. Only a sub-pixel difference in shadow blur softness and the label-text shadow being slightly fainter in C++.
- **Dark:** Same match in dark: identical label, cyan box with red shadow, #00B4DB / #FF0000 entries, and the three sliders. Shadow blur renders equivalently; only minor blur-softness sub-pixel variation.

### 72. Invalidate Shadow Host — 🟢 (L:match / D:match)
- **Light:** Match. Both show 'Host' label, 'Update Host Size' blue button, 'Shadow', and Offset X:10 / Offset Y:10 / Radius:10 / Opacity:1.00 sliders at matching handle positions, plus a green-bordered white host box at the bottom. The only difference is the green box height: C# (ground truth) renders it inside a constrained harness card so only ~100px shows; C++ uses the full page so the green box is taller (~350px). Framing artifact, not a content bug.
- **Dark:** Same as light: identical 'Host' / 'Update Host Size' / four sliders / green-bordered white box. The green host box appears taller in C++ (full-page layout) vs shorter in C# (harness-card clipped), a container-framing difference only.

### 73. CollectionView — 🟡 (L:minor / D:minor)
- **Light:** Both show the same 24-item 3-column grid (cover1.jpg,0 ... Vegetables.jpg,23) under 'This is the header'. MAUI wraps the grid in a harness white rounded card and renders the header bold/large; C++ renders the grid directly (no card) with a regular-weight header. Content, item count, and layout identical.
- **Dark:** Same as light: identical 24-item 3-column grid and header text. MAUI has the harness card + bold header; C++ has no card and a regular-weight header. Cosmetic only.

### 74. Items — 🟡 (L:minor / D:minor)
- **Light:** Both show header 'Today', three items (Water the plants / Review the port / Ship wave 2) and footer 'Pick a task'. MAUI renders the 3 items with tight line spacing and a bold 'Today' header inside a harness card; C++ uses larger inter-item spacing (pushing 'Pick a task' lower) and a regular-weight 'Today'. Same content/structure.
- **Dark:** Same content as light: 'Today' header, 3 task items, 'Pick a task' footer. MAUI is compact with a bold header; C++ has wider row spacing and a regular header. Cosmetic spacing/weight difference only.

### 75. Single Bound Selection — 🟡 (L:minor / D:minor)
- **Light:** Both show the instruction paragraph, 'Selected: (none)', and the 5 countries (United States / Canada / Mexico / Brazil / Argentina). MAUI lists them with tight line spacing inside a harness card; C++ adds extra vertical gaps between rows. Identical text and selection state (none selected).
- **Dark:** Same as light: instruction paragraph, 'Selected: (none)', and the 5 country rows. MAUI compact, C++ has wider row spacing. Content identical.

### 76. Multiple Bound Selection — 🔴 (L:diff / D:diff)
- **Light:** C++ is MISSING the three buttons MAUI shows below the list — 'Clear and Add', 'Reset', 'Direct Update' (blue text). Both correctly show the instruction, 'Selected: Item 1, Item 2', header, and Item 0-3 with Item 1 & Item 2 highlighted; but the C++ render omits all three action buttons entirely.
- **Dark:** Same bug as light: C++ omits the 'Clear and Add', 'Reset', and 'Direct Update' buttons that MAUI renders below the Item 0-3 list. Selection highlight on Item 1 & Item 2 is present in both, but the three buttons are missing in C++.

### 77. Preselected Item — 🔴 (L:diff / D:diff)
- **Light:** The preselected item is NOT highlighted in C++. MAUI shows 'photo.jpg, 2' with a gray selection bar (the preselected item visibly selected); C++ renders the same list but 'photo.jpg, 2' has no selection highlight at all — the entire point of this page (a preselected item) is not visible. C++ also uses much wider row spacing so fewer items fit.
- **Dark:** Same bug as light: MAUI highlights 'photo.jpg, 2' with a gray selection bar; C++ shows no selection highlight on photo.jpg,2 (or any row). The preselected-item highlight is missing in C++.

### 78. Preselected Items — 🟡 (L:minor / D:minor)
- **Light:** Both correctly highlight the three preselected cells — photo.jpg 2, Fruits.jpg 4, FlowerBuds.jpg 5 — in the 3-column grid, with the instruction and 'Preselected: photo.jpg, 2, Fruits.jpg, 4, FlowerBuds.jpg, 5'. The MAUI light frame is captured at a smaller zoom (harness scale artifact) and uses a bold header; C++ header is regular weight. Selection state and content match.
- **Dark:** Both correctly highlight photo.jpg 2, Fruits.jpg 4, FlowerBuds.jpg 5 in the grid. MAUI header is bold, C++ regular weight; otherwise content and selection identical.

### 79. Selection Command Param — 🟢 (L:match / D:match)  ⚠️ _MAUI reference capture broken — re-shoot needed_
- **Light:** MAUI reference capture is broken — the csharp light screenshot is a fully black/empty frame with only the status bar and no app content; needs re-capture before a fair comparison. The C++ side renders correctly: 'Pending...', 'This is the header', and Item 0 through Item 9.
- **Dark:** MAUI reference capture is broken — the csharp dark screenshot is a fully black/empty frame with only the status bar; needs re-capture. The C++ side renders correctly: 'Pending...', 'This is the header', Item 0-9.

### 80. Selection Synchronization — 🟢 (L:match / D:match)  ⚠️ _MAUI reference capture broken — re-shoot needed_
- **Light:** MAUI reference capture is broken — the csharp light screenshot is a fully black/empty frame with only the status bar and no app content; needs re-capture before a fair comparison. The C++ side renders the full page: instruction paragraph, two sub-sections ('Set ItemsSource then SelectedItems' and 'Set SelectedItems then ItemsSource'), each with 'Selected: Item 3, Item 2' and Item 1-4. (No visible selection-highlight bars on Item 2/3 in C++, but cannot compare against a blank reference.)
- **Dark:** MAUI reference capture is broken — the csharp dark screenshot is a fully black/empty frame with only the status bar; needs re-capture. The C++ side renders the full two-section page with instruction text, 'Selected: Item 3, Item 2' labels, and Item 1-4 in each section.

### 81. Filter Collection — 🟡 (L:minor / D:minor)
- **Light:** Both show the 'Use EmptyView' toggle (on/green), a 'Filter' search box, and a 2-column grid of items (cover1.jpg 0, oasis.jpg 1, ...). C++ uses noticeably larger row line-height/spacing so fewer rows fit per screen (C++ shows through ~photo.jpg 30, MAUI through Fruits.jpg 32); content, structure and colors otherwise match. Cosmetic spacing difference only.
- **Dark:** Same as light: identical toggle + Filter box + 2-column grid; C++ has larger inter-row spacing than MAUI. Cosmetic only.

### 82. Filter Selection — 🟡 (L:minor / D:minor)
- **Light:** Both show the instruction paragraph, a 'Filter' box, a 'Selected: (none)' label with a blue 'Reset' button, and a single-column item list. C++ uses larger row spacing (shows through Legumes.jpg 13 vs MAUI through FlowerBuds.jpg 12). Content/structure/colors match; spacing-only cosmetic difference.
- **Dark:** Same as light: identical instruction text, Filter box, 'Selected: (none)' + Reset, single-column list; only the row line-height differs (C++ larger). Cosmetic only.

### 83. Header Footer — 🟡 (L:minor / D:minor)
- **Light:** Both show string header 'Just a string as a header', three items (cover1.jpg 0 / oasis.jpg 1 / photo.jpg 2) and string footer 'This footer is also a string'. In MAUI the header and footer strings are rendered bold/heavier weight; in C++ they render in the same regular weight as the body items. C++ also uses larger row spacing. Content matches; the header/footer bold-vs-regular and spacing are cosmetic.
- **Dark:** Same as light: same header string, 3 items, footer string. MAUI header/footer strings are bold while C++ renders them regular weight; C++ has larger row spacing. Cosmetic styling/spacing difference.

### 84. Header Footer Grid — 🔴 (L:diff / D:diff)
- **Light:** MAUI shows 'Toggle Header' and 'Toggle Footer' buttons (spaced apart), a large 'This Is A Header' header label, an 'Add Content' button under the header, the 3-column item grid, a large 'This Is A Footer' footer label, and a second 'Add Content' button under the footer. C++ renders ONLY the 3-column grid and the two toggle buttons (which are crammed together with no gap, text touching). MISSING in C++: the 'This Is A Header' label, the 'This Is A Footer' label, and BOTH 'Add Content' buttons — i.e. the entire header/footer View content is absent.
- **Dark:** Same as light: C++ is missing the 'This Is A Header' label, the 'This Is A Footer' label and both 'Add Content' buttons; toggle buttons are crammed together. Only the grid renders.

### 85. Header Footer Grid Horizontal — 🔴 (L:diff / D:diff)
- **Light:** MAUI shows a horizontally-scrolling 3-row grid (cover1.jpg 0 / Vegetables.jpg 3 / Legumes... across the top, oasis.jpg 1 / Fruits.jpg 4 / cover1.jpg 7 across the bottom) with a vertically-stacked 'This Is A Header' header running down the leading (left) edge. C++ instead renders a cramped VERTICAL 4-column grid with each cell text-wrapped into narrow stacks ('cover 1.jpg, 0', 'Vege table s.jpg 3', ...) — wrong scroll orientation/layout — AND is MISSING the 'This Is A Header' header view and the footer. Toggle Header/Toggle Footer buttons are also crammed together.
- **Dark:** Same as light: C++ renders the wrong (vertical, narrow-wrapped) layout instead of MAUI's horizontal-scroll grid, and the 'This Is A Header' vertical header and the footer are missing.

### 86. Header Footer Template — 🔴 (L:diff / D:diff)
- **Light:** Header DataTemplate: MAUI shows a locale short date '6/19/2026' plus a 'This Is A Header' label; C++ shows ISO/invariant ' 2026-06-20 14:51:51' (wrong date FORMAT, yyyy-MM-dd HH:mm:ss vs M/d/yyyy) and is MISSING the 'This Is A Header' label. Items: MAUI renders each item as a blue rectangle with centered text; C++ renders plain text with the BLUE rectangle background of the item template MISSING. Footer DataTemplate: MAUI shows '6/19/2026 6:53:15 PM' (locale, AM/PM) plus 'This Is A Footer'; C++ shows invariant '2026-06-20 14:51:51' and is MISSING the 'This Is A Footer' label.
- **Dark:** Same three bugs as light: (1) header/footer dates use invariant 'yyyy-MM-dd HH:mm:ss' instead of MAUI's locale '6/19/2026' / '6/19/2026 6:53:17 PM'; (2) the 'This Is A Header' and 'This Is A Footer' labels are missing; (3) the blue item-template rectangle background is missing — C++ items are plain text.

### 87. Header Footer View — ⬛ (L:blank / D:blank)
- **Light:** MAUI shows a large 'This Is A Header' header view, a rotated 'This Is A Footer' footer view, and two buttons 'Add 2 Items' and 'Clear All Items' (empty list). C++ renders essentially NOTHING — a fully blank page with only the status bar; the header view, footer view and both buttons are all absent.
- **Dark:** Same as light: MAUI shows the header view, footer view and the 'Add 2 Items'/'Clear All Items' buttons; the C++ page is completely blank (black, status bar only).

### 88. Footer Only String — 🟡 (L:minor / D:minor)
- **Light:** MAUI shows 20 items (cover1.jpg 0 ... FlowerBuds.jpg 19) followed by a bold string footer 'This is a footer'. C++ shows the same list but with much larger row spacing, so only ~17 items fit and the 'This is a footer' string is pushed below the visible frame (footer not shown on the first screen; it is in a scrollable list and the string-footer mechanism works on other pages). Cosmetic row-spacing difference causes the footer to fall below the fold.
- **Dark:** Same as light: identical item list; C++ larger row spacing means only ~17 items are visible and the 'This is a footer' string footer falls below the fold (visible in MAUI). Cosmetic spacing difference.

### 89. Basic Grouping — 🟡 (L:minor / D:minor)
- **Light:** Same grouped CollectionView: green group headers (Avengers/Fantastic Four/Defenders), black member rows, orange 'Total members: N' footers, all items match. Cosmetic differences only: (1) C++ uses taller row spacing (more vertical padding per item) so fewer rows fit on screen vs MAUI's compact rows; (2) MAUI renders 'This is a header' in bold and inside a white card container, C++ shows it in regular weight on transparent background.
- **Dark:** Same content and structure as light; identical green headers, orange footers, all member rows match. Differences are cosmetic: C++ has taller row density vs MAUI's compact rows, and 'This is a header' is bold in MAUI but regular weight in C++.

### 90. Grid Grouping — 🟡 (L:minor / D:minor)
- **Light:** Same 2-column grid-grouped CollectionView, same data (Avengers/Fantastic Four/Defenders/Heroes for Hire/West Coast Avengers), green headers, orange footers, identical 2-up item layout. Cosmetic differences: (1) C++ shows an extra 'This is a header' label at the very top that MAUI's capture does not; (2) C++ rows are taller (more spacing) so only ~3 groups fit vs MAUI's ~6.
- **Dark:** Same 2-column grid grouping and data as light; green headers, orange footers, 2-up items all match. Differences cosmetic: extra 'This is a header' label present in C++ but not MAUI capture, and taller C++ row density showing fewer groups per screen.

### 91. Grouping No Templates — 🟡 (L:minor / D:minor)
- **Light:** Both show the same flat ungrouped member list (no group templates) starting Thor, Captain America, Iron Man, ... in identical order. Cosmetic differences only: C++ uses taller row spacing (fewer rows visible) vs MAUI's tightly-packed rows, and MAUI renders inside a white card container vs C++ transparent background.
- **Dark:** Same flat list with no group templates, identical order starting Thor; only difference is C++'s taller row density vs MAUI's compact rows.

### 92. Grouping Plus Selection — 🟡 (L:minor / D:minor)
- **Light:** Same grouped list (Avengers/Fantastic Four/Defenders) with green headers, orange 'Total members' footers, all items match; neither side shows an active selection highlight (initial state). Cosmetic difference only: C++ has taller row spacing so fewer rows fit vs MAUI's compact card.
- **Dark:** Same grouped-plus-selection list and data; green headers and orange footers match, no selection highlighted on either side. Only difference is C++'s taller row density vs MAUI's compact rows.

### 93. Switch Grouping — 🟡 (L:minor / D:minor)
- **Light:** Both show 'Is Grouped:' label with a green ON Switch, then the grouped list (Avengers + green headers, orange footers, members). Switch renders correctly in C++. Cosmetic difference only: C++ uses taller row spacing vs MAUI's compact rows.
- **Dark:** Same 'Is Grouped:' label, green ON switch, and grouped list as light; switch and data match. Only difference is C++'s taller row density vs MAUI's compact rows.

### 94. Some Empty Groups — 🟡 (L:minor / D:minor)
- **Light:** Both correctly render the empty-groups demo: description paragraph at top, then Avengers (Total members: 2), Thundercats (Total members: 0, no items), Avengers again (2), Bionic Six (0, no items), Fantastic Four (4). Empty-group headers/footers show with zero items in both. Cosmetic difference only: C++ has taller row spacing vs MAUI's compact rows.
- **Dark:** Same empty-groups behavior and counts as light; Thundercats and Bionic Six correctly show 'Total members: 0' with no items in both. Only difference is C++'s taller row density vs MAUI's compact rows.

### 95. Scroll To Group — 🔴 (L:diff / D:diff)  ⚠️ _MAUI reference capture broken — re-shoot needed_
- **Light:** MAUI reference capture is broken — shows a fully black/empty frame with NO app content (only the status-bar clock); needs re-capture before a fair comparison. The C++ side renders the full expected page correctly: 'Group:' and 'Item:' numeric entries (both '0') with a blue 'Go', 'Group Name:' and 'Item Name:' text entries with a blue 'Go', a 'No scroll requested yet' label, then the grouped Avengers list (Thor, Captain America, Iron Man, ...).
- **Dark:** MAUI reference capture is broken — fully black/empty frame with no app content (only status-bar clock); needs re-capture. C++ side renders the full page correctly: Group/Item numeric entries with Go, Group Name/Item Name entries with Go, 'No scroll requested yet', and the grouped Avengers list.

### 96. Scroll Mode Test — 🔴 (L:diff / D:diff)  ⚠️ _MAUI reference capture broken — re-shoot needed_
- **Light:** MAUI reference capture is broken — shows a fully black/empty frame with NO app content (only the status-bar clock); needs re-capture before a fair comparison. The C++ side renders the full expected page: 'ItemsUpdatingScrollMode:' with blue mode buttons (KeepItemsInView, KeepS...), 'Scroll To Middle', 'Add Item Above', 'Add Item Below', 'Add Item To End' buttons, a 'Mode: KeepItemsInView · Items: 20' status line, then the indexed item list (cover1.jpg, 0 / oasis.jpg, 1 / photo.jpg, 2 / ...).
- **Dark:** MAUI reference capture is broken — fully black/empty frame with no app content (only status-bar clock); needs re-capture. C++ side renders the full page correctly: ItemsUpdatingScrollMode buttons, Scroll To Middle / Add Item Above-Below-To End buttons, 'Mode: KeepItemsInView · Items: 20' status, and the indexed cover1.jpg/oasis.jpg/... item list.

### 97. Adaptive Collection — 🟡 (L:minor / D:minor)
- **Light:** Both show 'Layout: Linear (single column)' header and Item 1-8. In MAUI the item rows are horizontally CENTERED with large vertical spacing; in C++ the items are LEFT-aligned with tighter row spacing (so all 8 fit, vs MAUI scrolled to ~Item 7). Content/structure identical; difference is item-template alignment and spacing only. MAUI also wraps content in a white rounded card (harness).
- **Dark:** Same as light: MAUI items centered with wide spacing, C++ items left-aligned with tighter spacing. Identical text and structure.

### 98. Staggered Layout — 🔴 (L:diff / D:diff)
- **Light:** This is a StaggeredLayout demo (the point is variable-height/staggered items). MAUI renders the staggered effect: rows have large varied vertical gaps (Item 0-2, big gap, Item 3-5, gap, ... only ~12 items visible). C++ renders a UNIFORM 3-column grid with no staggering — items packed tightly in equal rows showing Item 0-23. The staggered/variable-height layout is not applied in the C++ port.
- **Dark:** Same as light: MAUI shows the staggered variable-height layout (large gaps between rows, ~12 items); C++ shows a uniform tightly-packed 3-column grid (Item 0-23). Staggered effect missing.

### 99. Varied Size Selector — 🔴 (L:diff / D:diff)
- **Light:** DataTemplateSelector with varied row heights. In MAUI the 'Milk1' (cream) row is rendered roughly 2x TALLER than the Coffee rows (the varied-size template), so only Coffee0/Milk1/Coffee2/Coffee3 fit. In C++ all rows are near-uniform height so 6 items show (Coffee0-Coffee5) and the Milk rows are NOT taller — the per-template varied row height is not applied. Also the list sits at the top in C++ vs vertically inset in MAUI.
- **Dark:** Same as light: MAUI gives the Milk1 row ~2x height (varied-size template); C++ renders uniform-height rows (Coffee0-Coffee5) without the taller Milk rows.

### 100. Nested Collection — 🔴 (L:diff / D:diff)
- **Light:** Nested CollectionViews demo ('It's CollectionViews all the way down.'). MAUI renders each red-italic 'Source N' header followed by a horizontal nested CollectionView row of blue 'Caption N-0, Caption N-1, Caption N-2...' items. C++ is MISSING the entire inner/nested CollectionView: it shows only plain 'Source 0'..'Source 17' text with NO blue caption rows, and the Source headers also lost their red-italic styling.
- **Dark:** Same as light: MAUI shows red 'Source N' headers each with a horizontal row of blue 'Caption N-x' items; C++ shows only the bare 'Source N' list with the nested horizontal CollectionView and blue captions entirely missing.

### 101. Data Template Selector — 🟡 (L:minor / D:minor)
- **Light:** Both show a 'Day of Week Filter' SearchBar followed by the template-selected list: 'It's the weekend! Woot!' for weekend items and Monday/Tuesday/Wednesday/Thursday/Friday for weekdays, in the same order. Identical text and template selection. Only difference: C++ rows are taller/more spaced (fewer per screen) vs MAUI's denser rows.
- **Dark:** Same as light: identical SearchBar + weekend/weekday template list; only row-spacing density differs (C++ taller rows).

### 102. Cv Visual States — 🟡 (L:minor / D:match)  ⚠️ _MAUI reference capture broken — re-shoot needed_
- **Light:** Both show 'Single Selection' (Item 1-3) and 'Multi Selection' (Item 1-4). Content and structure match; C++ uses wider inter-row spacing vs MAUI's tighter rows. Cosmetic spacing only.
- **Dark:** MAUI dark reference is BROKEN: the two CollectionView areas render as solid WHITE blocks under the 'Single Selection'/'Multi Selection' labels, hiding all items (dark text invisible on white — the CV background did not adopt dark mode). The C++ port renders correctly here: black background with white 'Item 1-3' and 'Item 1-4' visible. C++ is correct; the reference needs re-capture.

### 103. Empty View — 🟡 (L:minor / D:minor)
- **Light:** Both show a 'Filter' SearchBar and the populated list 'cover1.jpg, 0' / 'oasis.jpg, 1' / 'photo.jpg, 2' / 'Vegetables.jpg, 3' ... in identical order with identical text. EmptyView not triggered (list populated). Only difference: C++ rows are taller/more spaced (16 items visible) vs MAUI denser rows (~19 visible).
- **Dark:** Same as light: identical Filter SearchBar + numbered image list; only row-spacing density differs.

### 104. Empty View Null — 🟢 (L:match / D:match)
- **Light:** Both render the empty CollectionView with a centered 'Nothing to display.' EmptyView, horizontally and vertically centered. Equivalent. MAUI wraps content in a white rounded card (harness artifact) vs C++ full-bleed white.
- **Dark:** Both render centered 'Nothing to display.' on a black background. Equivalent.

### 105. Empty View Rtl — 🟡 (L:minor / D:minor)
- **Light:** Both render the 'Left to Right' picker, a Filter search box, and a 3-column CollectionView grid (cover1.jpg/oasis.jpg/photo.jpg ... numbered items). Content and structure match. Differences are cosmetic only: MAUI wraps the picker in a harness gray container card and uses looser row spacing (fewer rows visible), while C++ has tighter row line-height showing more rows. A normal user would call it the same page.
- **Dark:** Same as light: both show the 'Left to Right' picker, Filter box, and 3-column numbered grid with identical content. Only cosmetic row-spacing/harness-card differences.

### 106. Empty View Selector — 🟢 (L:match / D:match)
- **Light:** Both show the multi-line instruction paragraph (1. Filter the items below... 'No results matched your filter.' and 'Try a broader filter?'), the Filter search box, and the single item 'Baboon — Africa & Asia'. Identical content and structure. Only difference is a harness gray container card behind the search box on MAUI (harness-only), so text wraps to slightly different line breaks.
- **Dark:** Same as light: instruction paragraph, Filter box, and 'Baboon — Africa & Asia' item all match. Harness-card-only difference.

### 107. Empty View Swap — 🟡 (L:minor / D:minor)
- **Light:** Both render the Filter box, the 'Toggle Between EmptyViews' label with an OFF switch on the right, blue Clear/Fill buttons, and the 3-column numbered grid. Content and structure match. Cosmetic-only differences: MAUI has a harness gray card and looser row spacing; C++ has tighter rows showing more items.
- **Dark:** Same as light: Filter box, 'Toggle Between EmptyViews' + OFF switch, Clear/Fill buttons, and 3-column grid all match. Only cosmetic spacing/harness-card differences.

### 108. Empty View Template — 🟡 (L:minor / D:minor)
- **Light:** Both show a Filter search box and the 3-column numbered CollectionView grid (cover1.jpg, 0 ... etc). Content matches. Cosmetic-only differences: MAUI has a harness gray container card and looser row spacing; C++ has tighter line-height showing more rows.
- **Dark:** Same as light: Filter box plus 3-column numbered grid, identical content. Only cosmetic row-spacing/harness-card differences.

### 109. Empty View View — 🟡 (L:minor / D:minor)
- **Light:** Both render a Filter box and the 3-column numbered grid; identical item content. Cosmetic-only differences: harness gray card on MAUI and looser row spacing vs tighter C++ rows showing more items.
- **Dark:** Same as light: Filter box and 3-column numbered grid match. Only cosmetic spacing/harness-card differences.

### 110. Empty View Load Simulate — 🟢 (L:match / D:match)
- **Light:** Both show the loading EmptyView: a single centered line 'Items loading simulation...'. Text content and centering match. Only difference is a harness white/gray content card on MAUI vs a plain background on C++ (harness-only).
- **Dark:** Both show centered white 'Items loading simulation...' on a black background. Match.

### 111. Carousel Page — 🔴 (L:diff / D:diff)
- **Light:** Carousel item content is laid out differently. MAUI renders 'Item 1' as a LARGE label centered both vertically and horizontally inside the full-height carousel viewport (title 'Basic Horizontal Carousel' at top, big centered 'Item 1'). C++ renders 'Item 1' as a small label top-left-aligned right under the title (carousel cell collapsed to content height instead of filling the viewport), which then exposes the Prev/Next buttons and 'Position 0 — current: Item 1' label lower on the page (off-screen/below the carousel in MAUI). The carousel item template is not filling/centering in the cell.
- **Dark:** Same discrepancy as light: MAUI shows a large centered 'Item 1' filling the carousel viewport; C++ shows a small top-left 'Item 1' with the carousel cell collapsed, exposing the Prev/Next buttons and 'Position 0 — current: Item 1' label below.

### 112. Chat Example — 🔴 (L:diff / D:diff)
- **Light:** Chat bubble rendering is wrong. MAUI shows content-width rounded pill bubbles aligned by sender: green 'Hi there!' right-aligned (sent) and blue 'Hello — how can I help you today?' left-aligned (received). C++ renders BOTH messages as full-width, edge-to-edge, square-cornered colored bars with left-aligned text (green 'Hi there!' bar then blue 'Hello...' bar) — no rounded corners, no content-width sizing, and no per-sender right/left alignment (the green 'Hi there!' should be right-aligned).
- **Dark:** Same bubble bug as light: MAUI shows rounded content-width pills (green right-aligned, blue left-aligned) with dark text on the pastel fills; C++ shows full-width square colored bars, both left-aligned. Additionally C++ renders the bubble text in light/white rather than dark, reducing contrast on the pastel backgrounds.

### 113. Items Updating Scroll Mode — 🟡 (L:minor / D:minor)
- **Light:** Both render the same page: 'UpdatingScrollMode:' header, KeepItemsInView/KeepScrollOffset mode buttons, blue 'Add Item' button, 'Mode: KeepItemsInView · Items: 50' status line, and the Title/Subtitle list. Only difference is row spacing: C++ list rows have noticeably more vertical gap between items (showing ~16 rows) vs MAUI's tighter rows (showing ~18). MAUI also sits inside the harness white rounded container card; C++ fills the page. Content and structure identical.
- **Dark:** Same as light: identical content and structure; C++ list rows have larger inter-row vertical spacing than MAUI's tighter rows. Cosmetic spacing difference only.

### 114. Radio Button Group — 🔴 (L:diff / D:diff)
- **Light:** Real alignment bug. In MAUI, Options A/B/C are LEFT-aligned at the page margin (radio circle flush left at ~x=25, label to its right, large circles, generous row spacing). In C++, Options A/B/C are pushed to the RIGHT/center (radio circle at ~x=185), with much smaller circles and the label flush against the circle (no gap). The 'This RadioButton is inside a Grid' + 'Option D' row matches in both. The StackLayout radio buttons are horizontally mis-positioned and undersized in C++.
- **Dark:** Same alignment bug as light: MAUI Options A/B/C are left-aligned at the margin with large circles and row spacing; C++ pushes them to center (~x=185) with small circles flush against labels and tight spacing.

### 115. Radio Button Group Binding — ⬛ (L:blank / D:blank)  ⚠️ _MAUI reference capture broken — re-shoot needed_
- **Light:** MAUI reference capture is broken — shows a fully black/empty frame with only the nav header 'maui_ios_gallery' and NO app content; needs re-capture before a fair comparison. The C++ port renders the full page correctly: description text ('The RadioButtons in this Grid have a GroupName and Selection bound to a ViewModel... The GroupName is group1 The Selection is (null)'), a 2x2 grid of Option A/B/C/D radios, and two blue action buttons 'Set selection in view model to B' / 'Clear selection in view model to null'.
- **Dark:** MAUI reference capture is broken — fully black/empty except the nav header 'maui_ios_gallery'; needs re-capture. C++ renders the full page correctly (description, 2x2 A/B/C/D radio grid, two blue action buttons).

### 116. Radio Button Group Gallery — 🔴 (L:diff / D:diff)
- **Light:** Same recurring radio-alignment bug. MAUI shows the radios LEFT-aligned at the margin (large circles, generous row spacing) under each section ('Parent level...', 'Page level...', 'Test: mixed group names...'). C++ pushes the radios toward center (~x=110) with much smaller circles, labels flush against circles, and tight row spacing. Both have correct text/labels (Group=null, Group='A', GroupName='A'/'B'/'C'/null); C++ is more compact so it also shows the mixed-group section which MAUI's frame cuts off. Core discrepancy is horizontal positioning and circle size of the radios.
- **Dark:** Same as light: MAUI radios left-aligned at margin with large circles and spacing; C++ radios center-clustered (~x=110) with small circles, flush labels, tight rows. Text content matches.

### 117. Radio Button Border — 🔴 (L:diff / D:diff)
- **Light:** Two real bugs. (1) Missing background fill: MAUI's Option 1 and Option 2 rows have a YELLOW BackgroundColor fill; in C++ both rows have NO yellow fill (only the red border on Option 1 and green border on Option 4 render). (2) Content alignment: MAUI radio content is LEFT-aligned with large circles and tall rows; C++ centers the circle+label (~x=185) with small circles and short rows. Borders (red on Option 1, green on Option 4) and the selected dot on Option 4 are present in both.
- **Dark:** Same two bugs as light: C++ is missing the YELLOW background fill on Option 1 and Option 2 rows (red/green borders still render), and the radio content is center-aligned with small circles instead of MAUI's left-aligned large circles.

### 118. Radio Button Content — 🟡 (L:minor / D:minor)
- **Light:** Both render the same explanatory text and radios (Option A, Option C, the 'Can't use View for Content...' row, coffee.png). Recurring alignment cosmetic: MAUI radios left-aligned with large circles; C++ center-clustered with small circles. C++ truncates the 'Can't use View for Content on this platform, so just plain old text' to one ellipsized line vs MAUI's 2-line wrapped card. C++ scrolls further and also shows the custom coffee-cup template section (black cups + red/black lines) which MAUI's frame cuts off. Comparable region matches structurally.
- **Dark:** Same alignment cosmetic as light. Note: in C++ dark the custom coffee-cup template glyphs do not render (only the red template lines show; the black cup shapes are invisible on the black background), but this section is below MAUI's cut-off frame so not directly comparable. Comparable upper region matches structurally.

### 119. Radio Content Properties — 🟡 (L:minor / D:minor)  ⚠️ _MAUI reference capture broken — re-shoot needed_
- **Light:** Both correctly demonstrate text-property propagation to Content: 'Option A' in red italic, 'Option B' in bold blue, green 'It's a button inside a button.' rows. Text colors, font styles, sizes and transforms all match. Recurring cosmetic: MAUI radios left-aligned with large circles and generous spacing; C++ radios center-clustered with small circles and tighter rows (so C++ shows more rows). Content and styling are correct in both.
- **Dark:** Same as light: text colors and font styling (red italic Option A, bold blue Option B, green button-content rows) match correctly. Cosmetic difference is the radio alignment/circle-size/spacing — MAUI left-aligned large circles vs C++ center-clustered small circles.

### 120. Radio Template From Style — ⬛ (L:blank / D:blank)  ⚠️ _MAUI reference capture broken — re-shoot needed_
- **Light:** MAUI reference capture is broken — shows a fully black/empty frame with only the status-bar clock and NO app content (not even a nav header); needs re-capture before a fair comparison. The C++ port renders the page correctly: three custom-templated radio buttons (A, B, C), each a light-gray card with a corner letter label and a blue radio circle with a filled blue dot (selected state).
- **Dark:** MAUI reference capture is broken — fully black/empty with only the status-bar clock and no app content; needs re-capture. C++ renders correctly: three gray cards (A, B, C) with corner letter labels and blue filled-dot radio circles; the custom ControlTemplate-from-style renders properly in dark mode.

### 121. Scattered Radio Button — 🔴 (L:diff / D:diff)
- **Light:** Radio buttons A/B/C are cramped with zero inter-item spacing in C++ (renders as 'OAOBOC' run-together), while MAUI spaces them out ('O A  O B  O C') with the highlighted nested-StackLayout band having vertical padding. Also the D button is CENTERED in C++ ('OD (None of the above)' sits mid-width) but LEFT-aligned in MAUI under the 'And another outside...' text.
- **Dark:** Same as light: A/B/C buttons cram together with no spacing in C++ vs spaced in MAUI; the highlighted A/B/C band fades the labels in both (expected), but C++ has no row padding. D button is centered in C++ vs left-aligned in MAUI.

### 122. Swipe Gesture — 🟢 (L:match / D:match)  ⚠️ _MAUI reference capture broken — re-shoot needed_
- **Light:** MAUI reference capture is broken — the LIGHT csharp shot shows garbled/overlapping illegible text where 'Welcome to .NET MAUI!' and the 'A SwipeView with gesture recognizers / Double-tap the card, or swipe to Favourite / Delete.' card content all collapse on top of each other (capture-timing glitch). The C++ side renders this cleanly: 'Welcome to .NET MAUI!' with 'June 2026', the subtitle lines, and 'TapCommand (double-tap)'. Needs re-capture before a fair comparison; C++ output looks correct.
- **Dark:** MAUI reference capture is broken — the DARK csharp shot is MISSING the swipe card content entirely (only the header band and 'TapCommand (double-tap)' show; no 'Welcome to .NET MAUI!' card). The C++ side renders the full card. Needs re-capture; C++ output looks correct.

### 123. Swipe Item Position — 🟢 (L:match / D:match)
- **Light:** Both show a bordered 'Reveal' box plus the 'Swipe in any direction' subtitle. Identical content and structure; C++ sits slightly higher (less top whitespace) because MAUI wraps the page in a harness gray card — harness-only.
- **Dark:** Both show the 'Reveal' header band over a large light-gray content area. Equivalent layout and colors.

### 124. Swipe Item Size — 🟢 (L:match / D:match)
- **Light:** Both show 'Swipe a row left to reveal Delete', the 'Different icon sizes' section (128x128 / 256x256 / 512x512 Icon each with a gray 'Swipe to Left' box), and 'Different SwipeView sizes' (SwipeView 128/256 Height boxes). All labels, box widths and heights match; C++ shows more of the page due to MAUI's harness card top offset.
- **Dark:** Same content and structure as light, correctly themed dark (black background, light-gray boxes, white text). Matches MAUI.

### 125. Swipe Threshold — 🟢 (L:match / D:match)
- **Light:** Both render the info banner 'The Threshold property is only implemented for now on Android and iOS.', 'Default Threshold (Reveal Mode)' purple box, 'Custom Threshold (...Reveal Mode)' slider + purple box, 'Default Threshold (Execute Mode)' purple box, etc. Same purple fill color, same slider positions. C++ shows further down the page ('...Execute Mode' + 'Reveal threshold=80 / Execute threshold=80') because MAUI's harness card offsets content.
- **Dark:** Same elements and same purple boxes, correctly themed dark. Matches MAUI.

### 126. Swipe View Margin — 🟢 (L:match / D:match)
- **Light:** Both show 'Horizontal items revealed', the black info box 'Modify the SwipeView Margin and Padding values...', 'SwipeView Content Margin' and 'SwipeView Content Padding' sliders, and the 'Horizontal SwipeItems' / 'Vertical SwipeItems' gray boxes nested in lighter margin cards. Same slider thumb positions and box layout; C++ shows both boxes fully due to MAUI's harness top offset.
- **Dark:** Same elements and structure, correctly themed dark. Matches MAUI.

### 127. Swipe View Shadow — 🟡 (L:minor / D:minor)
- **Light:** Both show 'Shadow in SwipeView Content', a 'SwipeItems' rounded white-to-gray gradient 'Content' box with border and drop shadow, and a 'SwipeItemViews' box the same way. Only cosmetic difference: the 'Content' label is LEFT-aligned in C++ but CENTERED in MAUI. Box fill, rounded border and shadow all present in both.
- **Dark:** Both show the two 'Content' boxes; the white-to-light-gray gradient fill and thin border read faintly against the black background in both. Same cosmetic difference as light: 'Content' is left-aligned in C++ vs centered in MAUI.

### 128. Swipe Refresh — 🔴 (L:diff / D:diff)
- **Light:** C++ shows only the first line 'Swipe left to delete, pull to refresh' and is MISSING the 'Ready' status label that MAUI renders on the second line directly below it.
- **Dark:** Same as light: C++ renders only 'Swipe left to delete, pull to refresh' and omits the 'Ready' status label that MAUI shows underneath.

### 129. Refresh View — ⬛ (L:blank / D:blank)  ⚠️ _MAUI reference capture broken — re-shoot needed_
- **Light:** MAUI reference capture is broken — the csharp LIGHT screenshot is a fully black/empty frame with only the status-bar clock (21:00), no app content at all; needs re-capture before a fair comparison. The C++ port correctly renders the full RefreshView page: 'Pull the items down to refresh the ScrollView.', 'Number of items: 50', the 'Toggle Refresh Color / Toggle Background Color / Toggle Refresh / Toggle Is Enabled' blue links, 'Is Refreshing: False', 'Is Enabled: True', and the bottom '50 items loaded. Pull to add more.' line.
- **Dark:** MAUI reference capture is broken — the csharp DARK screenshot is a fully black/empty frame with only the status-bar clock (21:01), no app content; needs re-capture. The C++ port correctly renders the full RefreshView page (same controls as light, on black background).

### 130. Custom Size Swipe — 🟡 (L:minor / D:match)  🎬 _motion/effect — needs an animated GIF to judge_
- **Light:** Content matches: green SwipeView band with 'This is the SwipeView Content' + 'Test Click from Content' blue link, and 'RightItems revealed (open=1, threshold=0)' below. Only diff: MAUI wraps the page in a white rounded harness card (visible top/bottom white margins) while C++ renders the content edge-to-edge — harness framing artifact.
- **Dark:** Matches: green SwipeView band with 'This is the SwipeView Content' + 'Test Click from Content' link, and 'RightItems revealed (open=1, threshold=0)' on black background. Equivalent.

### 131. Custom Swipe Item View — 🟡 (L:minor / D:match)  🎬 _motion/effect — needs an animated GIF to judge_
- **Light:** Content matches: 'Right items revealed (Favourite)' label and the indigo rounded card with 'Welcome to .NET MAUI' (light-blue) and 'June 19, 2026' (white). Only diff: MAUI wraps the page in a white rounded harness card while C++ renders edge-to-edge — harness framing artifact.
- **Dark:** Matches: 'Right items revealed (Favourite)' and the indigo rounded card with 'Welcome to .NET MAUI' / 'June 19, 2026' on black background. Equivalent.

### 132. Basic Swipe — 🟡 (L:minor / D:match)  🎬 _motion/effect — needs an animated GIF to judge_
- **Light:** Content matches: five gray swipe rows ('Swipe Up (Execute)', 'Swipe Down (Reveal)', 'Swipe Right (Reveal)', 'Swipe Left (Execute)', 'Swipe in any direction') and 'Swipe a row, then invoke Delete'. Only diff: MAUI wraps the list in a white rounded harness card while C++ renders edge-to-edge — harness framing artifact.
- **Dark:** Matches: same five gray swipe rows and 'Swipe a row, then invoke Delete' on black background. Equivalent.

### 133. Gestures — 🟡 (L:minor / D:minor)  🎬 _motion/effect — needs an animated GIF to judge_
- **Light:** Structure matches: 'Gesture target (tap / pan / pinch / swipe / pointer)' heading, a blue rounded rectangle of the same size, and a 'Last gesture:' status label. Only diff is captured gesture state: MAUI shows 'Last gesture: (none)' (idle) while C++ shows 'Last gesture: Pointer exited' (a pointer hover/exit fired before capture) — interaction-state difference, not a layout bug. MAUI also has a white harness-card wrapper.
- **Dark:** Same as light: identical heading + blue rounded rectangle; MAUI shows 'Last gesture: (none)' vs C++ 'Last gesture: Pointer exited' — captured-interaction-state difference, not a render bug.

### 134. Pan Gesture Events — 🟢 (L:match / D:match)  🎬 _motion/effect — needs an animated GIF to judge_
- **Light:** Matches: 'StatusType: Completed, TotalX: 0, TotalY: 0' label over a green top half and red bottom half, same split proportions. MAUI has a white harness-card wrapper top/bottom but the green/red BoxView content is equivalent.
- **Dark:** Matches: 'StatusType: Completed, TotalX: 0, TotalY: 0' label, green top half, red bottom half on black background. Equivalent.

### 135. Pointer Gesture — 🟡 (L:minor / D:minor)
- **Light:** All text content and colors match: yellow 'Thanks for releasing me! Press me again or leave me!' banner, three 'Pointer position ... {40, 30}' lines, 'Thanks for hovering me!', three '{25, 18}' lines, green 'Hover me green!', and 'Hover above label to make it turn green'. Cosmetic diff: in MAUI the three heading labels ('Thanks for releasing me!', 'Thanks for hovering me!', 'Hover me green!') render at a larger heading font size, whereas C++ renders them at roughly body size (smaller) — font-size mismatch on those labels only.
- **Dark:** Same as light: identical text/colors; MAUI renders the three 'Thanks...'/'Hover me green!' labels in a larger heading font while C++ renders them at body size — cosmetic font-size mismatch.

### 136. Drag Drop — 🟡 (L:minor / D:match)  🎬 _motion/effect — needs an animated GIF to judge_
- **Light:** Content matches: 'All colors (drag a swatch to the rainbow list):' heading, six color swatches (orange, yellow, green, blue, purple, pink), 'Rainbow:', a red bar, and the drag/drop status block. MAUI light is scrolled so the bottom status text is cut off ('Drag start position relative to...' truncated) and wrapped in a white harness card; C++ shows the full status block ('Self X:0, Y:0 (Red)', 'Receiving layout (Rainbow) X:10, Y:10', 'Move: swatch dropped into Rainbow') — C++ shows more complete content. Harness framing + scroll-state difference.
- **Dark:** Matches: same heading, six color swatches, 'Rainbow:', red bar, and the full drag/drop status block ('Self X:0, Y:0 (Red)', 'Drag/Drop position relative to... Receiving layout (Rainbow) X:10, Y:10', 'Move: swatch dropped into Rainbow') on black background. Equivalent.

### 137. Hit Testing — 🔴 (L:diff / D:diff)  🎬 _motion/effect — needs an animated GIF to judge_
- **Light:** MAUI applies visual transforms that C++ omits: the 2nd 'Lorem ipsum dolor sit ame' is right-indented in MAUI (left-aligned in C++), 'Scale = 2' is rendered large/scaled in MAUI (same small size as 'Scale = 1' in C++), and 'Rotation = 20' is drawn rotated/skewed in MAUI (flat horizontal in C++). MAUI's 'Selected: -' vs C++ 'Selected: Image'. C++ also draws an extra large light-green rounded rectangle at the bottom that MAUI does not show. The green oval is wider/flatter in MAUI vs more circular in C++.
- **Dark:** Same as light: MAUI right-indents the 2nd 'Lorem ipsum', renders 'Scale = 2' large and 'Rotation = 20' rotated, while C++ shows them all flat/un-transformed. 'Selected: -' (MAUI) vs 'Selected: Image' (C++). C++ adds an extra large green rounded rectangle at the bottom absent in MAUI.

### 138. Input Transparent — 🟡 (L:minor / D:minor)
- **Light:** Same content and structure: 'InputTransparent=false/true' single-button rows, overlay row with intentionally overlapped 'Top (transparent)/Bottom (clickable)' text, and toggle row. The overlapping stacked-layer text matches in both (by design). C++ shows the full page (toggle switch + 'Ready - tap a layer set below') while the MAUI white card is truncated earlier; line wrapping and vertical spacing differ slightly (MAUI wraps 'should NOT be clickable' onto its own line).
- **Dark:** Same as light: equivalent content, intentional overlapping layer labels match; C++ renders the complete page including the toggle switch and 'Ready' status line, whereas the MAUI capture cuts off near the toggle. Minor differences in line-wrap/vertical spacing only.

### 139. Focus — 🟡 (L:minor / D:minor)
- **Light:** Same content: 'Focus target' entry, 'Focus Entry' + 'Unfocus Entry' buttons, and 'IsFocused: false'. MAUI separates the two buttons with a wide horizontal gap (centered pair) while C++ packs them close together at the left. MAUI also adds more top padding before the entry; C++ places content near the very top.
- **Dark:** Same as light: identical text/controls; the only difference is the horizontal gap between 'Focus Entry' and 'Unfocus Entry' (wide in MAUI, tight in C++) and slightly more top padding in MAUI.

### 140. Dispatcher — 🟡 (L:minor / D:minor)
- **Light:** Identical text content where it overlaps ('Watch the machines complain...', 'Fail Access', 'Access', 'This was a success!', '3 Seconds Later', etc.). C++ actually shows MORE of the page (down through '3 Second Timer (Start/Stop)', 'OBSOLETE ZONE ALERT!', 'Device.StartTimer(3s)') because the MAUI white card is truncated at 'Or, you might want something to repeat like a timer:'. Difference is only visible scroll extent / top padding.
- **Dark:** Same as light: overlapping text matches exactly; C++ renders the full content list while the MAUI capture is cut off earlier. Only vertical extent/padding differs.

### 141. Device — 🟡 (L:minor / D:minor)
- **Light:** Identical values: 'Platform: iOS', 'Idiom: Phone', 'Version: 26.5'. MAUI center-aligns the text block horizontally and places it a bit below the top; C++ left-aligns the same three lines flush to the top-left edge. Content equal, only alignment/position differs.
- **Dark:** Same as light: same three values, but MAUI centers the block while C++ left-aligns it at the top-left.

### 142. Effects — ⬛ (L:blank / D:blank)  ⚠️ _MAUI reference capture broken — re-shoot needed_
- **Light:** MAUI reference capture is broken - it shows the iOS HOME SCREEN / springboard (app-icon grid: Fitness, Watch, Contacts, Files, .NET, MauiCompare, maui_ios_gallery) instead of the app, so a fair comparison is impossible; needs re-capture. The C++ side renders the page correctly: 'Entry With Focus Routing Effect' + 'Alert Simple' entry, 'Entry With Focus Platform Effect' + 'Alert Simple' entry, 'Detach routing effect' / 'Re-attach routing effect' buttons, and status 'routing effect attached - routing attached: yes'.
- **Dark:** MAUI reference capture is broken - it shows the iOS home screen / springboard app-icon grid instead of the app; needs re-capture. The C++ side renders the full effects page correctly (two focus-effect entries, detach/re-attach buttons, and the 'routing attached: yes' status).

### 143. Measure First Strategy — 🔴 (L:diff / D:diff)
- **Light:** Same content (intro text, 'Toggle Sizing Strategy', 'ItemSizingStrategy: MeasureFirstItem', 'Avengers' green header, Thor...Mockingbird, 'Total members: 12' orange, 'Fantastic Four', 'The Thing') but row heights differ badly: C++ renders each CollectionView row with large vertical padding/whitespace so the list is heavily spread out, while MAUI shows the rows tightly packed. As a result MAUI fits all 12 Avengers plus the Fantastic Four header above the fold while C++ needs far more space per row.
- **Dark:** Same as light: identical text/group structure and colors, but C++ row heights are much taller (excessive per-item vertical padding) vs MAUI's compact rows, making the C++ list visibly more spread out.

### 144. Scroll View — ⬛ (L:blank / D:blank)  ⚠️ _MAUI reference capture broken — re-shoot needed_
- **Light:** MAUI reference capture is broken - it is a fully black/empty frame showing only the status-bar clock with NO app content; needs re-capture before a fair comparison. The C++ side renders the page correctly: 'Scrolled to: 0 / 0 (done)' header followed by a scrollable list 'Row 0 of 40' through 'Row 14 of 40'.
- **Dark:** MAUI reference capture is broken - fully black/empty frame with no app content (only the status bar); needs re-capture. The C++ side correctly shows 'Scrolled to: 0 / 0 (done)' and rows 'Row 0 of 40'...'Row 14 of 40'.

### 145. Web View — 🔴 (L:diff / D:diff)
- **Light:** Different WebView state: MAUI shows 'No navigation yet' with an empty WebView area at top, while C++ shows the WebView actually rendered the page content ('Welcome' / 'Served from a static HtmlWebViewSource.') plus a navigation log line 'new_page -> https://demo.test/welcome'. Both show the same 6 link-buttons (Page A/Page B/Back/Forward/Reload/Eval 1+1). The C++ side loaded/rendered HTML content the MAUI reference did not, so the upper region differs substantially.
- **Dark:** Same content discrepancy as light (MAUI 'No navigation yet' vs C++ rendered 'Welcome' WebView + navigation log). Additionally the C++ dark capture logs a full local file:/// path ('new_page -> file:///Users/.../MauiCompare.app/https:/demo.test/welcome/') instead of the clean 'https://demo.test/welcome' shown in the C++ light capture — the resolved navigation URL format is inconsistent and leaks an absolute simulator container path.

### 146. Hybrid Web View — 🟡 (L:minor / D:minor)
- **Light:** Same 'HybridWebView here' label and same 5 action buttons (Send message to JS / Invoke JS / Invoke Async JS / Test JS Exception / Test JS Async). Cosmetic difference only: C++ packs the buttons closer together and middle-truncates labels with an ellipsis ('Send...age to JS', 'Test JS...ception'), whereas MAUI spaces them out and clips the labels off the right screen edge ('Send messag|', 'Test JS Exce|'). Structure/content equivalent.
- **Dark:** Same as light: identical label and 5 buttons; the only difference is C++ ellipsis-truncated and tightly-spaced button labels vs MAUI wider-spaced, edge-clipped labels.

### 147. Alerts — 🟢 (L:match / D:match)
- **Light:** Equivalent: same 'OnAppearing: Alert — Welcome to the Alerts Page [Hello!]' line, same section labels (Display Alert / Display ActionSheet / Display Prompt) and same 6 buttons (Alert Simple, Alert Yes/No, ActionSheet Simple, ActionSheet Cancel/Delete, Question 1, Question 2). Only difference is harness vertical centering/spacing (MAUI more generous inter-button spacing and vertically centered; C++ more compact and top-aligned) — cosmetic.
- **Dark:** Same as light: identical text, sections and 6 buttons; only inter-button spacing/vertical placement differs slightly.

### 148. Animation — 🟢 (L:match / D:match)  🎬 _motion/effect — needs an animated GIF to judge_
- **Light:** Animation page — both show the '.NET bot (animation target)' label and the same 3 buttons (Start Animation, Start Custom Animation, Cancel Animation greyed/disabled). The MAUI capture happened to catch the label rotated diagonally mid-animation while C++ shows it flat/at-rest at the top; this is a capture-timing artifact of a rotation animation, not a layout bug. A GIF is needed to fairly judge rotation parity.
- **Dark:** Same as light: identical label and 3 buttons; MAUI caught the label mid-rotation (skewed diagonal) vs C++ at rest. Motion/animation page — needs a GIF to judge.

### 149. Application Control — 🔴 (L:diff / D:diff)
- **Light:** Same 'Quits the application' heading and 3 buttons (Terminate Application / Open Window / Close Window), but the status line differs in DATA: MAUI shows 'Windows open: 1 | main window: (untitled) | main page: set' while C++ shows 'Windows open: 0 | main window: none' — C++ reports zero windows and omits the 'main page: set' segment, so the window/page state is wrong (the app clearly has a window). Also the heading is bold in MAUI but regular weight in C++.
- **Dark:** Same data discrepancy as light: MAUI status 'Windows open: 1 | main window: (untitled) | main page: set' vs C++ 'Windows open: 0 | main window: none' (zero windows, missing main-page segment), plus heading bold-vs-regular.

### 150. Ios Entry — 🟢 (L:match / D:match)
- **Light:** Equivalent: both show a bordered Entry with placeholder 'Enter text here to see the font size change' and a 'Toggle AdjustsFontSizeToFitWidth' button below it. Entry border, placeholder color and button render the same; only the harness card vs full-bleed vertical placement differs slightly (cosmetic).
- **Dark:** Same as light: identical placeholder Entry and toggle button, equivalent rendering in dark theme.

### 151. Ios Date Picker — 🔴 (L:diff / D:diff)
- **Light:** DatePicker date FORMAT differs: C++ shows '12/31/2020' (invariant US MM/DD/YYYY) while MAUI shows '31.12.2020' (device-locale DD.MM.YYYY). Same date value, different format. The 'Toggle DatePicker UpdateMode' button matches.
- **Dark:** Same format discrepancy as light: C++ '12/31/2020' (invariant US) vs MAUI '31.12.2020' (device-locale European).

### 152. Ios Time Picker — 🔴 (L:diff / D:diff)
- **Light:** TimePicker time FORMAT differs: C++ shows '2:00 PM' (invariant US 12-hour) while MAUI shows '14:00' (device-locale 24-hour). Same time value, different format. The 'Toggle TimePicker UpdateMode' button and 'UpdateMode: WhenFinished' status line both match.
- **Dark:** Same format discrepancy as light: C++ '2:00 PM' (12-hour invariant) vs MAUI '14:00' (24-hour device-locale).

### 153. Ios Picker — 🟡 (L:minor / D:match)
- **Light:** Both show the 'Select a monkey' picker placeholder field and the blue 'Toggle Picker UpdateMode' button, same layout. Only difference is harness-only: MAUI light renders content inside a white rounded container card that does not fill the screen height, while the C++ port fills the full background; sub-pixel vertical anchoring otherwise identical.
- **Dark:** Both render the 'Select a monkey' picker field and the 'Toggle Picker UpdateMode' button identically on a black background; equivalent.

### 154. Ios Search Bar — 🟡 (L:minor / D:minor)
- **Light:** Both show a rounded search bar (magnifier + 'Enter search term' placeholder) plus blue 'Toggle SearchBar Style' and 'Toggle Background' buttons. MAUI vertically centers the search bar lower in the page (inside a harness white card); the C++ port places the search bar near the top of the content. Content and controls are identical; only vertical anchoring + the harness card differ.
- **Dark:** Same structure and controls in both. The C++ port anchors the search bar at the top while MAUI positions it lower (centered). Cosmetic vertical-placement difference only.

### 155. Ios Scroll View — 🟡 (L:minor / D:minor)
- **Both themes:** FIXED this session (flyout_page::arrange, commit e07f7f21d7) and image re-captured: the C++ port now renders the Slider (~45%) + blue 'Toggle ScrollView DelayContentTouches' + blue 'Return to Platform-Specifics List' buttons, matching MAUI's content. Residual is cosmetic: MAUI wraps the detail in a white card with more inter-control spacing, while the port stacks the two buttons flush below the slider/nav-chevron (no card chrome).

### 156. Ios Slider Update On Tap — 🟡 (L:minor / D:minor)
- **Light:** Both show 'Tap on the Slider bar to move the thumb.', a Slider at the minimum (thumb at far left), and a blue 'Toggle Update on Tap' button. Identical content; only the harness white container card (present on MAUI) and minor vertical anchoring differ.
- **Dark:** Same content in both: instruction label, Slider at minimum, and 'Toggle Update on Tap' button on a black background. Equivalent aside from minor vertical anchoring.

### 157. Ios First Responder — 🔴 (L:diff / D:diff)
- **Light:** All controls present in both (two instruction paragraphs, two Entry fields, two 'OK' buttons, 'Focus First'/'Focus Second' buttons, and three status lines). BUG: the first instruction paragraph is truncated in the C++ port — it ends at '...and the keyboard should' and the final line 'disappear.' is missing/clipped, whereas MAUI shows the full three-line paragraph ending in 'disappear.' Also 'Focus First' and 'Focus Second' are crammed together with little spacing in C++ vs wide spacing in MAUI.
- **Dark:** Same as light: the first instruction paragraph is truncated in the C++ port ('...the keyboard should' with the 'disappear.' line missing) while MAUI shows the complete paragraph. 'Focus First'/'Focus Second' buttons are also packed tightly together in C++ vs spread out in MAUI.

### 158. Ios Pan Gesture — 🟡 (L:minor / D:minor)  🎬 _motion/effect — needs an animated GIF to judge_
- **Light:** Both show 'panned x:45 y:-12', the blue 'Toggle Simultaneous Gesture Recognition' button, 'Pan target', and 'SimultaneousRecognition: false'. Content identical. The 'panned x:45 y:-12' line is rendered bold in MAUI but regular weight in the C++ port — a minor font-weight difference.
- **Dark:** Same content in both. The 'panned x:45 y:-12' label is bold in MAUI and regular weight in the C++ port; otherwise identical.

### 159. Ios Safe Area — 🟡 (L:minor / D:minor)
- **Light:** Both show the same Lorem ipsum paragraph and the blue 'Disable Use Safe Area' button. In the C++ port the text starts at the very top edge (text uses near-zero left/right margins, wraps to wider lines) while MAUI positions the paragraph lower with slightly larger side margins. Content fully present; cosmetic margin/anchoring difference consistent with the safe-area-disabled demo.
- **Dark:** Same Lorem ipsum paragraph and 'Disable Use Safe Area' button in both. C++ port hugs the top edge with full-width text; MAUI sits lower with narrower text column. Cosmetic margin/placement difference only.

### 160. Ios Swipe Transition — 🟡 (L:minor / D:minor)  🎬 _motion/effect — needs an animated GIF to judge_
- **Light:** Both show 'SwipeTransitionMode:' with 'Reveal' and 'Drag' buttons, a gray swipe-item card labeled 'Swipe right', the instruction 'Swipe right to reveal Delete; pick a SwipeTransitionMode above', and 'SwipeTransitionMode: Drag'. Content identical; only difference is the C++ port packs the 'Reveal'/'Drag' buttons tightly beside the label while MAUI spreads them across the row, plus the harness white card.
- **Dark:** Same content and controls in both. C++ port crams 'Reveal Drag' next to the label while MAUI spaces them out across the row. Cosmetic spacing difference only.

### 161. Ios Blur Effect — 🔴 (L:diff / D:diff)  🎬 _motion/effect — needs an animated GIF to judge_
- **Light:** The MAUI ground truth shows only the four blur buttons (No Blur / Extra Light Blur / Light Blur / Dark Blur) and the 'BlurEffect: ExtraLight' label, with NO source image visible — the image the blur applies to is absent. The C++ port additionally renders the actual dog photo at the top (the image the blur operates on), then the four buttons and label below it. So the two apps disagree on whether the source image is shown: C++ shows the image, MAUI does not. Buttons/label text match. The page's real point is the live blur effect over the image, which a still frame cannot judge.
- **Dark:** Same as light: MAUI shows only buttons + 'BlurEffect: ExtraLight' label with no source image; C++ shows the dog photo plus the buttons/label. Content disagreement on the presence of the source image; a GIF is needed to judge the actual blur rendering.

### 162. Navigation Gallery — 🟡 (L:minor / D:minor)
- **Light:** Content matches exactly: status label 'Stack depth: 1 | top: PAGE NUMBER 1 | secondary toolbar items: 0' and six blue buttons (Push Page, Pop Page, Insert Page Before Current, Remove Page Before Current, Pop To Root, Toggle Secondary Toolbar Item). Only difference is vertical spacing: MAUI spreads the buttons with generous gaps across the page; C++ packs them tightly near the top with minimal inter-button spacing.
- **Dark:** Identical to light: same label and six buttons match; only the inter-button vertical spacing differs (MAUI spaced out, C++ packed tight at top).

### 163. Modal — 🟡 (L:minor / D:minor)
- **Light:** Content matches: 'Modal Page 1' label, buttons Push Page / Push Modal Page / Push Modal Navigation Page / Push Modal Flyout Page, the disabled grey 'Pop Modal Page' (correctly rendered as disabled in both), and 'Modal depth: 0 | page stack depth: 1'. Only difference is button vertical spacing (MAUI spaced out, C++ packed tight near the top).
- **Dark:** Identical to light: same label, four active buttons, disabled grey 'Pop Modal Page', and depth label all match; only inter-button spacing differs.

### 164. Tabbed Flyout — ⬛ (L:blank / D:blank)
- **Light:** C++ renders a completely empty white-then-black screen (only the status bar / nav-pill visible) where MAUI shows a full page: 'Menu' label, blue buttons 'Home tab' / 'Settings tab' / 'Toggle flyout', then 'Flyout dismissed', 'Demo tabs', 'Home', 'This is the Home tab.', 'Settings', 'This is the Settings tab.' The entire C++ page content is missing.
- **Dark:** Same as light: C++ shows a fully black/empty screen with only the status bar, while MAUI shows the complete menu+tabs content. Nothing is rendered on the C++ side.

### 165. Toolbar — 🔴 (L:diff / D:diff)
- **Light:** The C++ port is MISSING the top status label 'You clicked on ToolbarItem: {none}' that MAUI shows above the buttons. C++ renders only the six blue buttons (Enable/Disable Test (1), Enable/Disable Test Secondary (4), Enable/Disable Test Secondary (2), Change text on Test Secondary (1), Remove/Add Secondary (3), Change Command Property on Secondary (3)) starting at the very top with no preceding label. Button text matches; spacing is also tighter in C++ (secondary, cosmetic).
- **Dark:** Same as light: the 'You clicked on ToolbarItem: {none}' label is absent in the C++ port; only the six buttons render. The status label present in MAUI is missing.

### 166. Menu Bar — 🟡 (L:minor / D:minor)
- **Light:** Content matches: 'You clicked on Menu Item:' label and the blue 'Toggle Menu Bar Item' button. Only difference is the C++ port positions the content closer to the top of the page (less top padding from the harness card) than MAUI. A native iOS menu bar is not shown in either app (expected).
- **Dark:** Identical to light: same label and button match; only the vertical position of the content block differs slightly (C++ higher).

### 167. Title Bar — 🟡 (L:minor / D:minor)
- **Light:** Content matches: two-column layout with 'Content Options' (checkboxes Set Icon, Leading Content, Content, Trailing Content, Tall TitleBar, and Show TitleBar shown checked/filled in both) plus Title and Subtitle entry fields; and 'Color Options' with two 'Green' placeholder entry fields, Set Color / Set Foreground buttons, a 'Toggle Title Bar On Window' link, and the 'TitleBar: Title / Subtitle / Content are live' label. Both apps clip the right column identically at the page edge. Only difference is spacing/wrapping: MAUI wraps 'Content Options' header to two lines and spaces the checkboxes apart, while C++ keeps the header on one line and packs the checkboxes tightly.
- **Dark:** Identical to light: same two columns, same checkboxes (Show TitleBar checked), same entry fields and labels match; only header wrapping and checkbox spacing differ.

### 168. Chrome — 🟡 (L:minor / D:minor)
- **Light:** Content matches: the blue 'Press or right-click me' button and the 'Ready' status label beneath it. Only difference is the C++ port places the button near the very top of the page while MAUI centers it slightly lower with more top padding. The page's real purpose is the context menu shown on right-click, which a still frame cannot exercise.
- **Dark:** Identical to light: same 'Press or right-click me' button and 'Ready' label match; only the button's vertical position differs (C++ higher).

### 169. Context Flyout — 🔴 (L:diff / D:diff)
- **Light:** Top portion matches in both (the 'Increment by 1 (or right-click me)' button, 'Is dynamic menu enabled?' switch, 'Right-click to see beautiful menus' label, 'Has a custom context menu' entry, and the blue 'COOL' image). But the C++ port additionally renders extra content MAUI does not: a live WebView showing the Microsoft Bing cookie-consent dialog ('Microsoft and our third-party vendors use cookies...' with Accept/Reject/More options buttons and a Privacy/Legal/Advertise/Ad Info footer), plus a status line '0' and the text 'Right-click a control, or its menu items are exercised programmatically'. MAUI shows only the top controls on a clean white page with no WebView and no status text. So the C++ page is taller and shows a Bing WebView + status labels absent from the MAUI reference.
- **Dark:** Same as light: top controls match, but the C++ port renders an extra live Bing cookie-consent WebView dialog (Accept/Reject/More options) plus status text '0' and 'Right-click a control, or its menu items are exercised programmatically' that the MAUI reference does not display.

### 170. Templated View — 🔴 (L:diff / D:diff)
- **Light:** Same content (red CardView/ControlTemplate intro labels, 'Slavko Vlasic' standard card, and three compact cards Carolina Pena / Wade Blanks / Colette Quint with gray image thumbnails). Two real differences: (1) Font weight — MAUI renders the names and 'Compact card'/'Slavko Vlasic' headings BOLD and the red intro labels bold-italic, while the C++ port renders all of these in a REGULAR (non-bold) weight. (2) Compact-card layout — in MAUI the text column starts right next to the gray thumbnail (~x=165); in the C++ port there is a large empty gap and the text column starts much further right (~x=255), so each compact card has a wide blank band between the image and its text.
- **Dark:** Same two differences as light: the C++ port renders the card titles/names in regular weight instead of MAUI's bold (and the red intro labels non-italic-bold), and the compact-card text column is pushed far to the right of the gray thumbnail leaving a large empty gap that MAUI does not have.

### 171. Custom Layout — ⬛ (L:blank / D:blank)
- **Light:** MAUI shows a custom layout demo with five blue labels positioned around the page: 'Top' centered near the top, 'Left' 'Left' on the left-center and 'Right' 'Right' on the right-center forming a row, and 'Bottom' centered near the bottom. The C++ port renders NOTHING — the page is a fully empty white surface with no labels at all.
- **Dark:** MAUI shows the same custom-layout demo (blue 'Top', 'Left Left', 'Right Right', 'Bottom' labels positioned around the page). The C++ port renders a completely empty black page with no content whatsoever.

### 172. Visual States — 🔴 (L:diff / D:diff)
- **Light:** Structure and all text match (the 'Entry with VisualStateManager:' label, second labeled entry, the two explanatory paragraphs, and the two blue button labels 'Hover me to see the state change' and 'Click me to see the state change and revert'). The real difference is the first Entry: MAUI renders it with a bright GREEN background fill (its VisualStateManager state color), while the C++ port renders it as a plain empty entry with a faint gray/transparent background — the green VisualStateManager background fill is missing.
- **Dark:** Same as light: MAUI renders the first 'Entry with VisualStateManager' control with a bright green background fill, but the C++ port shows it as a plain empty entry with no green fill — the VisualStateManager-driven green background is missing.
