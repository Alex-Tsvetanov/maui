# MAUI sample-port progress (full-coverage loop)

Goal: translate **all** MAUI `Controls.Sample` pages into code-first C++ `maui::samples::*_page`,
wire each into the runnable gallery, and document with macOS + iOS screenshots + GIFs. Running as an
unattended overnight loop: light parallel translation agents (≤3) feed a serial coordinator that builds
the gallery once per batch, runs + captures on both platforms, fixes issues, documents, commits, pushes,
and spawns the next batch.

MAUI surface: **270 XAML pages** in `src/Controls/samples/Controls.Sample`. Priority breadth-first.

## Already done (pre-loop, the curated 11 thematic demos)
value_controls · input_controls · pickers · formatted_text · items · shapes · containers ·
swipe_refresh · web_view · chrome · tabbed_flyout — see the per-example READMEs in this folder.

## Tier 1 — Pages/Controls (per-control demos, 22)
- [x] ButtonPage → `button_page`  *(batch 1)*
- [x] LabelPage → `label_page`  *(batch 1)*
- [x] ImagePage → `image_page`  *(batch 1)*
- [x] EntryPage → `entry_page`  *(batch 1)*
- [x] EditorPage → `editor_page`  *(batch 1)*
- [x] SearchBarPage → `search_bar_page`  *(batch 1)*
- [x] CheckBoxPage → `check_box_page`  *(batch 1)*
- [x] SwitchPage → `switch_page`  *(batch 1)*
- [x] SliderPage → `slider_page`  *(batch 1)*
- [x] StepperPage → `stepper_page`
- [x] ProgressBarPage → `progress_bar_page`
- [x] ActivityIndicatorPage → `activity_indicator_page`
- [x] BoxViewPage → `box_view_page`
- [x] DatePickerPage → `date_picker_page`
- [x] TimePickerPage → `time_picker_page`
- [x] PickerPage → `picker_page`
- [x] ImageButtonPage → `image_button_page`
- [x] IndicatorPage → `indicator_page`
- [x] RefreshViewPage → `refresh_view_page`
- [x] ShapesPage → `shapes_demo_page`
- [x] TitleBarPage → `title_bar_page` *(partial — TitleBar is Windows-mapped)*
- [x] HybridWebViewPage → `hybrid_web_view_page` *(partial — JS→.NET deferred)*

## Tier 2 — Pages/Layouts (14)
AbsoluteLayout · ClippingPage · ContentView · CustomLayout · FlexLayout · Grid · HorizontalStackLayout ·
LayoutIsEnabled · RelativeLayout · ScrollView · StackLayout · TemplatedView · VerticalStackLayout · ZIndex

## Batch 9 — CollectionView galleries (9) — full render (struct-cell fix landed)
- [x] FilterCollectionView → `filter_collection_page`
- [x] BasicGrouping → `basic_grouping_page`
- [x] SelectionModeGallery → `selection_mode_page`
- [x] HeaderFooterString → `header_footer_page`
- [x] EmptyViewStringGallery → `empty_view_page`
- [x] DataTemplateSelectorGallery → `data_template_selector_page`
- [x] AdaptiveCollectionView → `adaptive_collection_page`
- [x] SingleBoundSelection → `single_bound_selection_page`
- [x] ChatExample → `chat_example_page`
- ⚠ NOTE: custom-struct item cells donE28099t render in the gallery yet (chrome+API do); framework fix in flight.

## Batch 13 — SwipeView galleries (9)
- [x] BasicSwipeGallery → `basic_swipe_page`
- [x] SwipeItemPositionGallery → `swipe_item_position_page`
- [x] SwipeViewShadowGallery → `swipe_view_shadow_page`
- [x] CustomSwipeItemViewGallery → `custom_swipe_item_view_page`
- [x] SwipeItemSizeGallery → `swipe_item_size_page`
- [x] SwipeViewMarginGallery → `swipe_view_margin_page`
- [x] CustomSizeSwipeViewGallery → `custom_size_swipe_page`
- [x] SwipeViewGestureRecognizerGallery → `swipe_gesture_page`
- [x] HorizontalSwipeThresholdGallery → `swipe_threshold_page`

## Batch 12 — RadioButton + Shadow galleries (9)
- [x] RadioButtonGroupGallery → `radio_button_group_page`
- [x] RadioButtonGroupBindingGallery → `radio_button_group_binding_page`
- [x] ScatteredRadioButtonGallery → `scattered_radio_button_page`
- [x] RadioButtonContentGallery → `radio_button_content_page`
- [x] ContentProperties → `radio_content_properties_page`
- [x] TemplateFromStyle → `radio_template_from_style_page`
- [x] ShadowPlaygroundPage → `shadow_playground_page`
- [x] InvalidateShadowHostPage → `invalidate_shadow_host_page`
- [x] RadioButtonGroupGalleryPage → `radio_button_group_gallery_page`

## Batch 11 — BorderGalleries (9) — natively rendered Border control
- [x] BorderStyles → `border_styles_page`
- [x] BorderStroke → `border_stroke_page`
- [x] BorderPlayground → `border_playground_page`
- [x] BorderLayout → `border_layout_page`
- [x] BorderAlignment → `border_alignment_page` (HorizontalOptions deferred — see README)
- [x] BorderClipPlayground → `border_clip_playground_page`
- [x] Borderless → `borderless_page`
- [x] BorderResizeContent → `border_resize_content_page`
- [x] RadioButtonBorder → `radio_button_border_page`

## Batch 10 — ShapesGalleries: transforms / clip / mutation (9) — natively rendered
- [x] TransformPlaygroundGallery → `transform_playground_page`
- [x] PathTransformStringGallery → `path_transform_string_page`
- [x] ShapeAppThemeGallery → `shape_app_theme_page`
- [x] ClipGallery → `clip_gallery_page`
- [x] ClipViewsGallery → `clip_views_page` (Clip on every view kind)
- [x] ClipCornerRadiusGallery → `clip_corner_radius_page`
- [x] AutoSizeShapesGallery → `auto_size_shapes_page`
- [x] InvalidateBrushGallery → `invalidate_brush_page`
- [x] UpdatePathDataGallery → `update_path_data_page`

## Batch 8 — ShapesGalleries (9) — natively-rendered shape/geometry demos
- [x] EllipseGallery → `ellipse_gallery_page`
- [x] RectangleGallery → `rectangle_gallery_page`
- [x] LineGallery → `line_gallery_page`
- [x] PolygonGallery → `polygon_gallery_page` (EvenOdd vs Nonzero rendered exactly)
- [x] PolylineGallery → `polyline_gallery_page`
- [x] PathGallery → `path_gallery_page` (markup + programmatic geometry)
- [x] LineJoinGallery → `line_join_gallery_page`
- [x] PathAspectGallery → `path_aspect_gallery_page`
- [x] CompositionGallery → `composition_gallery_page`

## Batch 7 — Core tail + HitTesting (9)
- [x] ClipPage → `clip_page` (geometry clip)
- [x] ContextFlyoutPage → `context_flyout_page` (chrome-only menu exercised programmatically)
- [x] MenuBarPage → `menu_bar_page` (chrome-only, body renders)
- [x] NavigationGallery → `navigation_gallery_page`
- [x] ModalPage → `modal_page`
- [x] ApplicationControlPage → `application_control_page`
- [x] PointerGestureGalleryPage → `pointer_gesture_page`
- [x] DragAndDropBetweenLayouts → `drag_drop_page`
- [x] HitTestingPage → `hit_testing_page` (top-level Pages)

## Batch 6 — Core feature demos (9)
- [x] AlertsPage → `alerts_page` (synthesized results; native dialog deferred)
- [x] SemanticsPage → `semantics_page`
- [x] FocusPage → `focus_page`
- [x] DispatcherPage → `dispatcher_page` (manual_dispatcher virtual clock)
- [x] DevicePage → `device_page`
- [x] AppThemeBindingPage → `app_theme_binding_page`
- [x] ToolbarPage → `toolbar_page` (ToolbarItems need nav chrome — exercised programmatically)
- [x] EffectsPage → `effects_page`
- [x] InputTransparentPage → `input_transparent_page`

## Batch 5 — UserInterface + Core feature demos (9) — proves the framework subsystems
- [x] BrushesPage → `brushes_page` (Brush family over graphics::paint)
- [x] TransformationsPage → `transformations_page`
- [x] GesturesPage → `gestures_page` (recognizer family)
- [x] AnimationPage → `animation_page`
- [x] StylesPage → `styles_page`
- [x] TriggersPage → `triggers_page`
- [x] BehaviorsPage → `behaviors_page`
- [x] VisualStatesPage → `visual_states_page`
- [x] FontsPage → `fonts_page`

## Tier 3 — selected Gallery pages (Shapes / Border / CollectionView / Swipe families) — later
## Tier 4 — PlatformSpecifics/iOS (platform-config demos) — later
## Out of scope (won't port): Compatibility/*, Maps, Windows-only pages

---
*Status legend: [ ] todo · [~] in flight · [x] done+captured+documented. Updated each batch.*
