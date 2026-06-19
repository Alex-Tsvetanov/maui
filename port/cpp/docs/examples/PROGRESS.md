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
- [ ] StepperPage → `stepper_page`
- [ ] ProgressBarPage → `progress_bar_page`
- [ ] ActivityIndicatorPage → `activity_indicator_page`
- [ ] BoxViewPage → `box_view_page`
- [ ] DatePickerPage → `date_picker_page`
- [ ] TimePickerPage → `time_picker_page`
- [ ] PickerPage → `picker_page`
- [ ] ImageButtonPage → `image_button_page`
- [ ] IndicatorPage → `indicator_page`
- [ ] RefreshViewPage → `refresh_view_page`
- [ ] ShapesPage → `shapes_demo_page`
- [ ] TitleBarPage → `title_bar_page` *(partial — TitleBar is Windows-mapped)*
- [ ] HybridWebViewPage → `hybrid_web_view_page` *(partial — JS→.NET deferred)*

## Tier 2 — Pages/Layouts (14)
AbsoluteLayout · ClippingPage · ContentView · CustomLayout · FlexLayout · Grid · HorizontalStackLayout ·
LayoutIsEnabled · RelativeLayout · ScrollView · StackLayout · TemplatedView · VerticalStackLayout · ZIndex

## Tier 3 — selected Gallery pages (Shapes / Border / CollectionView / Swipe families) — later
## Tier 4 — PlatformSpecifics/iOS (platform-config demos) — later
## Out of scope (won't port): Compatibility/*, Maps, Windows-only pages

---
*Status legend: [ ] todo · [~] in flight · [x] done+captured+documented. Updated each batch.*
