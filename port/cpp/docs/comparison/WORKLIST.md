# Non-green worklist — every page that is not green on every column

Generated from comparison.json at board commit `bfaa250fec`. 119 pages / 233 cells.
Ordered smallest-lane-first, then CLOSEST-TO-GREEN first, so each fix closes a page soonest.

`worst%` = the larger of the light/dark pixel-diff percentages. `motion` = the cell is a GIF/motion
cell whose review carries no percentage, so every percentage-based sweep has skipped it.

STATUS: OPEN | DONE <commit> | BLOCKED <one-line reason> | EXEMPT <ruling>

EVIDENCE CLASSES across the 233 non-green cells (2026-08-14):
  143  still-image diff — the review carries Light:/Dark: percentages
   62  real motion diff — BOTH columns moved, by different amounts (e.g. windows/clip: MAUI
        self-motion 13.99% vs port 15.45%, worst SSIM 0.9726 at frame 'scrolled-down')
   28  NO MOTION EVIDENCE — the harness injected an action and NEITHER column reacted. These are
        HARNESS defects: the coordinate misses its target on that lane, or the interaction is not
        reachable there. The port is not at fault and no port change can clear them — on
        carousel_page the two columns agree at SSIM 0.9991. Fix the scenario, not the port.


| # | platform | page | worst% | sev | motion | STATUS |
|---|---|---|---|---|---|---|
| 1 | windows | carousel_page | — | YEL | yes | BLOCKED harness: NO MOTION EVIDENCE — the injected action moved NEITHER column; columns agree (carousel_page SSIM 0.9991). Fix the scenario coordinate, not the port |
| 2 | windows | clip | — | YEL | yes | BLOCKED needs-windows-vm: scroll EXTENT 42px too large. Changed-region bbox MAUI (8,32,261,750) vs port (8,32,261,792); same x/top, port ends 42px lower. Port stamps panel.Height() from view->content_size() (windows/scroll_view_handler.cpp:348-354) where MAUI's ScrollViewer measures content natively |
| 3 | windows | clip_gallery | — | YEL | yes | BLOCKED needs-windows-vm: NOT the clip extent bug — scroll extent MATCHES MAUI exactly (both bbox (20,32,313,792)). Port renders 2.3pp LESS lightgray (211,211,211): MAUI 6.6% vs port 4.3%, concentrated in rows 616-792. A clipped shape under-fills in the revealed lower rows |
| 4 | windows | clip_views | — | YEL | yes | BLOCKED needs-windows-vm: ROOT CAUSE FOUND. Focused Entry repaints its whole box (port 8604px/93% vs MAUI 506px/7%). Port skips EntryHandler.Windows.cs:58-59 MapBackground -> TextBoxExtensions.UpdateBackground's TextControlBackground resource-key dance and pushes Background generically; entry_handler.cpp:823-830 documents this as 'identical AT REST, diverges only in hover/focused/disabled' |
| 5 | windows | ios_scroll_view | — | YEL | yes | BLOCKED harness: NO MOTION EVIDENCE — the injected action moved NEITHER column; columns agree (carousel_page SSIM 0.9991). Fix the scenario coordinate, not the port |
| 6 | windows | path_gallery | — | YEL | yes | OPEN |
| 7 | windows | search_bar | — | YEL | yes | BLOCKED harness: NO MOTION EVIDENCE — the injected action moved NEITHER column; columns agree (carousel_page SSIM 0.9991). Fix the scenario coordinate, not the port |
| 8 | windows | semantics | — | YEL | yes | BLOCKED harness: NO MOTION EVIDENCE — the injected action moved NEITHER column; columns agree (carousel_page SSIM 0.9991). Fix the scenario coordinate, not the port |
| 9 | windows | slider | — | YEL | yes | BLOCKED harness: NO MOTION EVIDENCE — the injected action moved NEITHER column; columns agree (carousel_page SSIM 0.9991). Fix the scenario coordinate, not the port |
| 10 | windows | swipe_refresh | — | YEL | yes | BLOCKED harness: NO MOTION EVIDENCE — the injected action moved NEITHER column; columns agree (carousel_page SSIM 0.9991). Fix the scenario coordinate, not the port |
| 11 | windows | context_flyout | 52.78 | RED |  | OPEN |
| 12 | ios | box_view | — | RED | yes | OPEN |
| 13 | ios | carousel_page | — | YEL | yes | OPEN |
| 14 | ios | clip_gallery | — | RED | yes | OPEN |
| 15 | ios | path_gallery | — | RED | yes | OPEN |
| 16 | ios | radio_button_content | — | YEL | yes | OPEN |
| 17 | ios | radio_content_properties | — | YEL | yes | OPEN |
| 18 | ios | scroll_view | — | YEL | yes | OPEN |
| 19 | ios | selection_synchronization | — | RED | yes | OPEN (ref restored 89b30d0c62; port was always correct) |
| 20 | ios | swipe_refresh | — | RED | yes | OPEN |
| 21 | ios | header_footer_grid_horizontal | 0.70 | YEL |  | OPEN |
| 22 | ios | border_resize_content | 1.13 | YEL |  | OPEN |
| 23 | ios | border_clip_playground | 1.81 | YEL |  | OPEN |
| 24 | ios | radio_button_group_gallery | 2.31 | YEL |  | OPEN |
| 25 | ios | radio_button_border | 2.76 | YEL |  | OPEN |
| 26 | ios | image | 65.19 | RED |  | OPEN |
| 27 | maccatalyst | carousel_page | — | YEL | yes | BLOCKED harness: NO MOTION EVIDENCE — the injected action moved NEITHER column; columns agree (carousel_page SSIM 0.9991). Fix the scenario coordinate, not the port |
| 28 | maccatalyst | clip | — | RED | yes | OPEN |
| 29 | maccatalyst | entry | — | YEL | yes | BLOCKED harness: NO MOTION EVIDENCE — the injected action moved NEITHER column; columns agree (carousel_page SSIM 0.9991). Fix the scenario coordinate, not the port |
| 30 | maccatalyst | path_gallery | — | RED | yes | OPEN |
| 31 | maccatalyst | pointer_gesture | — | YEL | yes | BLOCKED harness: NO MOTION EVIDENCE — the injected action moved NEITHER column; columns agree (carousel_page SSIM 0.9991). Fix the scenario coordinate, not the port |
| 32 | maccatalyst | radio_content_properties | — | YEL | yes | BLOCKED harness: NO MOTION EVIDENCE — the injected action moved NEITHER column; columns agree (carousel_page SSIM 0.9991). Fix the scenario coordinate, not the port |
| 33 | maccatalyst | selection_synchronization | — | YEL | yes | OPEN (ref restored 89b30d0c62; port was always correct) |
| 34 | maccatalyst | swipe_refresh | — | YEL | yes | BLOCKED harness: NO MOTION EVIDENCE — the injected action moved NEITHER column; columns agree (carousel_page SSIM 0.9991). Fix the scenario coordinate, not the port |
| 35 | maccatalyst | ios_date_picker | 0.12 | YEL |  | OPEN |
| 36 | maccatalyst | header_footer_grid_horizontal | 0.74 | YEL |  | OPEN |
| 37 | maccatalyst | varied_size_selector | 1.10 | YEL |  | OPEN |
| 38 | maccatalyst | radio_button_group_gallery | 1.35 | YEL |  | OPEN |
| 39 | maccatalyst | border_stroke | 2.05 | YEL |  | OPEN |
| 40 | maccatalyst | radio_button_border | 3.62 | YEL |  | OPEN |
| 41 | maccatalyst | context_flyout | 7.91 | RED |  | OPEN |
| 42 | maccatalyst | swipe_item_size | 23.89 | RED |  | BLOCKED reference-defect: real ref regression, survives a clean rebuild -> drive path (needs a swipe to fire) |
| 43 | android | activity_indicator | — | YEL | yes | OPEN |
| 44 | android | animation | — | YEL | yes | BLOCKED harness: NO MOTION EVIDENCE — the injected action moved NEITHER column; columns agree (carousel_page SSIM 0.9991). Fix the scenario coordinate, not the port |
| 45 | android | box_view | — | RED | yes | OPEN |
| 46 | android | carousel_page | — | YEL | yes | OPEN |
| 47 | android | clip | — | YEL | yes | OPEN |
| 48 | android | clip_gallery | — | RED | yes | OPEN |
| 49 | android | clip_views | — | YEL | yes | OPEN |
| 50 | android | editor | — | YEL | yes | OPEN |
| 51 | android | empty_view_rtl | — | YEL | yes | OPEN |
| 52 | android | entry | — | YEL | yes | OPEN |
| 53 | android | ios_blur_effect | — | YEL | yes | BLOCKED harness: NO MOTION EVIDENCE — the injected action moved NEITHER column; columns agree (carousel_page SSIM 0.9991). Fix the scenario coordinate, not the port |
| 54 | android | ios_picker | — | YEL | yes | OPEN |
| 55 | android | path_gallery | — | YEL | yes | OPEN |
| 56 | android | picker | — | YEL | yes | OPEN |
| 57 | android | scroll_view | — | YEL | yes | OPEN |
| 58 | android | search_bar | — | YEL | yes | OPEN |
| 59 | android | selection_synchronization | — | RED | yes | OPEN (ref restored 89b30d0c62; port was always correct) |
| 60 | android | semantics | — | YEL | yes | OPEN |
| 61 | android | slider | — | RED | yes | OPEN |
| 62 | android | stepper | — | RED | yes | BLOCKED harness: NO MOTION EVIDENCE — the injected action moved NEITHER column; columns agree (carousel_page SSIM 0.9991). Fix the scenario coordinate, not the port |
| 63 | android | invalidate_brush | 1.32 | YEL |  | OPEN |
| 64 | android | header_footer_grid | 1.35 | YEL |  | OPEN |
| 65 | android | header_footer_grid_horizontal | 1.35 | YEL |  | OPEN |
| 66 | android | items_updating_scroll_mode | 1.54 | YEL |  | OPEN |
| 67 | android | hybrid_web_view | 1.76 | YEL |  | OPEN |
| 68 | android | refresh_view | 1.85 | YEL |  | OPEN |
| 69 | android | chat_example | 2.23 | YEL |  | OPEN |
| 70 | android | swipe_view_margin | 3.29 | YEL |  | OPEN |
| 71 | android | ios_slider_update_on_tap | 3.34 | YEL |  | OPEN |
| 72 | android | border | 3.49 | YEL |  | OPEN |
| 73 | android | clip_corner_radius | 3.52 | YEL |  | OPEN |
| 74 | android | swipe_threshold | 3.60 | YEL |  | OPEN |
| 75 | android | border_layout | 3.62 | YEL |  | OPEN |
| 76 | android | border_resize_content | 3.64 | YEL |  | OPEN |
| 77 | android | clipping | 3.82 | YEL |  | OPEN |
| 78 | android | alerts | 4.26 | YEL |  | OPEN |
| 79 | android | menu_bar | 4.26 | YEL |  | OPEN |
| 80 | android | tabbed_flyout | 4.36 | YEL |  | OPEN |
| 81 | android | progress_bar | 4.50 | YEL |  | OPEN |
| 82 | android | application_control | 4.54 | YEL |  | OPEN |
| 83 | android | custom_size_swipe | 4.54 | YEL |  | OPEN |
| 84 | android | ios_safe_area | 4.54 | YEL |  | OPEN |
| 85 | android | measure_first_strategy | 4.54 | YEL |  | OPEN |
| 86 | android | modal | 4.54 | YEL |  | OPEN |
| 87 | android | navigation_gallery | 4.54 | YEL |  | OPEN |
| 88 | android | single_bound_selection | 4.54 | YEL |  | OPEN |
| 89 | android | styles | 4.54 | YEL |  | OPEN |
| 90 | android | toolbar | 4.54 | YEL |  | OPEN |
| 91 | android | update_path_data | 4.54 | YEL |  | OPEN |
| 92 | android | input_transparent | 4.69 | YEL |  | OPEN |
| 93 | android | invalidate_shadow_host | 4.83 | YEL |  | OPEN |
| 94 | android | label | 4.92 | YEL |  | OPEN |
| 95 | android | custom_layout | 6.09 | YEL |  | OPEN |
| 96 | android | indicator | 6.44 | YEL |  | OPEN |
| 97 | android | items | 6.73 | YEL |  | OPEN |
| 98 | android | multiple_bound_selection | 9.31 | RED |  | OPEN (ref restored 89b30d0c62; port was always correct) |
| 99 | android | focus | 23.27 | RED |  | OPEN |
| 100 | android | selection_command_param | 33.52 | RED |  | OPEN |
| 101 | android | layout_is_enabled | 37.78 | RED |  | OPEN |
| 102 | android | border_playground | 40.62 | RED |  | OPEN |
| 103 | android | preselected_items | 45.31 | RED |  | OPEN (ref restored 89b30d0c62; port was always correct) |
| 104 | android | transform_playground | 62.99 | RED |  | OPEN |
| 105 | android | image | 68.62 | RED |  | OPEN |
| 106 | android | border_clip_playground | 71.03 | RED |  | OPEN |
| 107 | android | shadow_playground | 71.29 | RED |  | OPEN |
| 108 | android | dispatcher | 74.20 | RED |  | OPEN |
| 109 | android | image_button | 78.49 | RED |  | OPEN |
| 110 | android | grouping_plus_selection | 78.61 | RED |  | OPEN |
| 111 | android | preselected_item | 83.70 | RED |  | OPEN |
| 112 | android | transformations | 85.83 | RED |  | OPEN |
| 113 | android | date_picker | 86.03 | RED |  | OPEN |
| 114 | android | time_picker | 86.17 | RED |  | OPEN |
| 115 | android | radio_button_group_gallery | 89.39 | RED |  | OPEN |
| 116 | android | context_flyout | 90.13 | RED |  | OPEN |
| 117 | android | content_view | 92.23 | RED |  | OPEN |
| 118 | android | border_stroke | 92.75 | RED |  | OPEN |
| 119 | android | collectionview | 93.02 | RED |  | OPEN |
