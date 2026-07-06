# .NET MAUI C++ port — visual parity comparison

Per-page MAUI-vs-C++ visual parity for the **181 gallery pages**, on **iOS**, **macOS** (Mac Catalyst + AppKit) and **Android**. Each section is collapsible and holds a discrepancy-count summary, then one subheader per page titled with a `{Sonnet}/{Gemini}` status-emoji combo (🟢 match / 🟡 minor / 🔴 major / ⬛ blank / ⏳ unreviewed). Under each page: the MAUI / C++ / C++&amp;XAML renders (light over dark; missing captures show a placeholder), then a subsubheader per review model (Sonnet, Gemini, Pixel-Perfect Score) titled with that model's own status emoji and holding its review prose. Generated from `comparison.json` by `tools/gen_readme.py` — do not edit by hand.

<details>
<summary><h2>iOS (181 examples) — click to expand</h2></summary>

Real .NET MAUI (native-default) vs the C++ port vs the compile-time-XAML gallery, captured on the same iOS simulator in light and dark. MAUI is the content ground truth.

**Discrepancy counts** (MAUI-vs-C++ parity verdicts; Sonnet 5 `claude-sonnet-5` and Gemini review each page independently):

| Classification | Sonnet 5 | Gemini |
| --- | --- | --- |
| 🟢 Match | 142 | 0 |
| 🟡 Minor | 13 | 0 |
| 🔴 Major | 16 | 0 |
| ⬛ Blank | 1 | 0 |
| ⏳ Unreviewed | 9 | 181 |

### 1. Absolute Layout — 🟢/⏳
<sub>absolute_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/absolute_layout_light.png" /></td><td><img width="300px" src="captures/ios/cpp/absolute_layout_light.png" /></td><td><img width="300px" src="captures/ios/xaml/absolute_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/absolute_layout_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/absolute_layout_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/absolute_layout_dark.png" /></td></tr></table>

ports AbsoluteLayoutPage.xaml A self-contained, code-first demo of the AbsoluteLayout control

#### 🟢 Sonnet 5 Review

Colored bars, autosized label, and centered text all match position/color/size in both light and dark themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 2. Activity Indicator — 🟢/⏳
<sub>activity_indicator</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/activity_indicator_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/activity_indicator_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/activity_indicator_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/activity_indicator_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/activity_indicator_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/activity_indicator_dark.gif" /></td></tr></table>

ports ActivityIndicatorPage.xaml (+ ActivityIndicatorPage.xaml.cs)

#### 🟢 Sonnet 5 Review

All indicator variants (default, styled, yellow background, larger, smaller) match in position, color, and size in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 3. Adaptive Collection — 🟢/⏳
<sub>adaptive_collection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/adaptive_collection_light.png" /></td><td><img width="300px" src="captures/ios/cpp/adaptive_collection_light.png" /></td><td><img width="300px" src="captures/ios/xaml/adaptive_collection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/adaptive_collection_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/adaptive_collection_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/adaptive_collection_dark.png" /></td></tr></table>

ports AdaptiveCollectionView.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.AdaptiveCollectionView)

#### 🟢 Sonnet 5 Review

Single-column list of items matches exactly in layout, spacing, and text in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 4. Alerts — 🟢/⏳
<sub>alerts</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/alerts_light.png" /></td><td><img width="300px" src="captures/ios/cpp/alerts_light.png" /></td><td><img width="300px" src="captures/ios/xaml/alerts_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/alerts_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/alerts_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/alerts_dark.png" /></td></tr></table>

ports AlertsPage.xaml (+ AlertsPage.xaml.cs) The C# AlertsPage drives the three Page dialog services — DisplayAlertAsync (simple OK + Yes/No), DisplayActionSheetAsync (simple + Cancel/Delete), and DisplayPromptAsync (two questions) — from a

#### 🟢 Sonnet 5 Review

All alert/actionsheet/prompt trigger links match text and layout in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 5. Alignment — 🟢/⏳
<sub>alignment</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/alignment_light.png" /></td><td><img width="300px" src="captures/ios/cpp/alignment_light.png" /></td><td><img width="300px" src="captures/ios/xaml/alignment_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/alignment_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/alignment_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/alignment_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;alignment&amp;quot; demo (ComparePages.Alignment()), the shipped-.NET-MAUI reference for the visual-parity comparison

#### 🟢 Sonnet 5 Review

Start/Center/End/Fill button alignment and red border/blue fill all match precisely in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 6. Animation — 🟢/⏳
<sub>animation</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/animation_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/animation_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/animation_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/animation_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/animation_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/animation_dark.gif" /></td></tr></table>

ports AnimationPage.xaml (+ AnimationPage.xaml.cs)

#### 🟢 Sonnet 5 Review

Both captures show the submarine idle animation frame identically in light and dark.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 7. App Theme Binding — 🟢/⏳
<sub>app_theme_binding</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/app_theme_binding_light.png" /></td><td><img width="300px" src="captures/ios/cpp/app_theme_binding_light.png" /></td><td><img width="300px" src="captures/ios/xaml/app_theme_binding_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/app_theme_binding_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/app_theme_binding_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/app_theme_binding_dark.png" /></td></tr></table>

ports AppThemeBindingPage.xaml

#### 🟢 Sonnet 5 Review

Green/red theme-bound text, orange resource-dictionary text, and toggle link all match in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 8. Application Control — 🟡/⏳
<sub>application_control</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/application_control_light.png" /></td><td><img width="300px" src="captures/ios/cpp/application_control_light.png" /></td><td><img width="300px" src="captures/ios/xaml/application_control_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/application_control_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/application_control_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/application_control_dark.png" /></td></tr></table>

ports ApplicationControlPage.xaml (+ .xaml.cs)

#### 🟡 Sonnet 5 Review

Layout, links, and window-count text match in both themes; only the window-title substring differs, a harmless runtime-state text difference, not a layout/color bug.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 9. Auto Size Shapes — 🟢/⏳
<sub>auto_size_shapes</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/auto_size_shapes_light.png" /></td><td><img width="300px" src="captures/ios/cpp/auto_size_shapes_light.png" /></td><td><img width="300px" src="captures/ios/xaml/auto_size_shapes_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/auto_size_shapes_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/auto_size_shapes_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/auto_size_shapes_dark.png" /></td></tr></table>

ports AutoSizeShapesGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/AutoSizeShapesGallery.xaml: a 3-row Grid (RowSpacing 0) that proves a stroked Ellipse auto-sizes to fill exactly half of the av

#### 🟢 Sonnet 5 Review

Green/orange split and ellipse with blue stroke match exactly in both light and dark themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 10. Basic Grouping — 🟢/⏳
<sub>basic_grouping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/basic_grouping_light.png" /></td><td><img width="300px" src="captures/ios/cpp/basic_grouping_light.png" /></td><td><img width="300px" src="captures/ios/xaml/basic_grouping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/basic_grouping_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/basic_grouping_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/basic_grouping_dark.png" /></td></tr></table>

ports GroupingGalleries/BasicGrouping.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 Review

Grouped list with headers, items, and total-member counts renders identically in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 11. Basic Swipe — 🟢/⏳
<sub>basic_swipe</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/basic_swipe_light.png" /></td><td><img width="300px" src="captures/ios/cpp/basic_swipe_light.png" /></td><td><img width="300px" src="captures/ios/xaml/basic_swipe_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/basic_swipe_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/basic_swipe_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/basic_swipe_dark.png" /></td></tr></table>

ports BasicSwipeGallery.xaml A code-first port of the MAUI SwipeView sub-gallery Pages/Controls/SwipeViewGalleries/BasicSwipeGallery.xaml: a vertical StackLayout of five SwipeViews, each demonstrating a different revealed-side / SwipeMode c

#### 🟢 Sonnet 5 Review

Swipe row list with gray cards and labels matches exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 12. Behaviors — 🟢/⏳
<sub>behaviors</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/behaviors_light.png" /></td><td><img width="300px" src="captures/ios/cpp/behaviors_light.png" /></td><td><img width="300px" src="captures/ios/xaml/behaviors_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/behaviors_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/behaviors_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/behaviors_dark.png" /></td></tr></table>

ports BehaviorsPage.xaml (+ .xaml.cs) and its companion Controls.Sample/Behaviors/NumericValidationBehavior.cs

#### 🟢 Sonnet 5 Review

Header text and entry placeholder render identically in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 13. Border — 🟢/⏳
<sub>border</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/border_light.png" /></td><td><img width="300px" src="captures/ios/cpp/border_light.png" /></td><td><img width="300px" src="captures/ios/xaml/border_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/border_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/border_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/border_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;border&amp;quot; demo (ComparePages.BorderPage()), the shipped-.NET-MAUI reference for the visual-parity comparison: a single Border centered on the page — red 5pt stroke, a RoundRectangle StrokeShape (Co

#### 🟢 Sonnet 5 Review

Red-bordered yellow card with 'Bordered content' text matches exactly, including the low-contrast dark-mode text which is equally faint in both MAUI and cpp.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 14. Border Alignment — ⏳/⏳
<sub>border_alignment</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

#### ⏳ Sonnet 5 Review

_Not yet reviewed._

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 15. Border Clip Playground — 🔴/⏳
<sub>border_clip_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/border_clip_playground_light.png" /></td><td><img width="300px" src="captures/ios/cpp/border_clip_playground_light.png" /></td><td><img width="300px" src="captures/ios/xaml/border_clip_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/border_clip_playground_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/border_clip_playground_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/border_clip_playground_dark.png" /></td></tr></table>

ports BorderClipPlayground.xaml (+ .xaml.cs) The C# page is an interactive Border-shape playground: a 100x100 Border (red stroke) clips an AspectFill Image (oasis.jpg) into the currently selected StrokeShape, while controls below mutate the

#### 🔴 Sonnet 5 Review

The cpp render is missing the dog photo clipped inside the round-rect border shape (shown only as an empty outline in both light and dark), while MAUI shows the actual image content inside the shape.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 16. Border Layout — 🟢/⏳
<sub>border_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/border_layout_light.png" /></td><td><img width="300px" src="captures/ios/cpp/border_layout_light.png" /></td><td><img width="300px" src="captures/ios/xaml/border_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/border_layout_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/border_layout_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/border_layout_dark.png" /></td></tr></table>

ports BorderLayout.xaml (+ BorderLayout.xaml.cs) The C# page demonstrates driving Border.StrokeThickness from a Slider: a Slider (0..40, set to 5 in OnAppearing) is bound to the Border&amp;#x27;s StrokeThickness; the Border (Silver stroke, White bac

#### 🟢 Sonnet 5 Review

Stroke-thickness slider and colored row (red/Center/blue/green) with gray border match exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 17. Border Playground — 🟢/⏳
<sub>border_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/border_playground_light.png" /></td><td><img width="300px" src="captures/ios/cpp/border_playground_light.png" /></td><td><img width="300px" src="captures/ios/xaml/border_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/border_playground_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/border_playground_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/border_playground_dark.png" /></td></tr></table>

ports BorderPlayground.xaml (+ BorderPlayground.xaml.cs) A self-contained, code-first interactive Border playground

#### 🟢 Sonnet 5 Review

Gradient-background dashed-border label card plus all form fields (colors, sliders, dropdowns) match exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 18. Border Resize Content — 🔴/⏳
<sub>border_resize_content</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/border_resize_content_light.png" /></td><td><img width="300px" src="captures/ios/cpp/border_resize_content_light.png" /></td><td><img width="300px" src="captures/ios/xaml/border_resize_content_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/border_resize_content_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/border_resize_content_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/border_resize_content_dark.png" /></td></tr></table>

ports BorderResizeContent.xaml A self-contained, code-first demo that resizes a Border&amp;#x27;s CONTENT and watches the Border track it

#### 🔴 Sonnet 5 Review

MAUI's ground-truth render shows the image slots (circle/square/triangle) as flat red/light-blue placeholder fills, while the C++ port renders an actual dog photo in those slots in both themes — a real content mismatch, not a padding artifact.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 19. Border Stroke — 🟢/⏳
<sub>border_stroke</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/border_stroke_light.png" /></td><td><img width="300px" src="captures/ios/cpp/border_stroke_light.png" /></td><td><img width="300px" src="captures/ios/xaml/border_stroke_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/border_stroke_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/border_stroke_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/border_stroke_dark.png" /></td></tr></table>

ports BorderStroke.xaml (+ BorderStroke.xaml.cs) A self-contained, code-first demo of Border StrokeThickness and how a Border tracks the height of its content

#### 🟢 Sonnet 5 Review

Stroke thickness variants, colors, and content-height slider all match exactly in both light and dark themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 20. Border Styles — ⏳/⏳
<sub>border_styles</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

#### ⏳ Sonnet 5 Review

_Not yet reviewed._

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 21. Borderless — 🟢/⏳
<sub>borderless</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/borderless_light.png" /></td><td><img width="300px" src="captures/ios/cpp/borderless_light.png" /></td><td><img width="300px" src="captures/ios/xaml/borderless_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/borderless_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/borderless_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/borderless_dark.png" /></td></tr></table>

ports Borderless.xaml A self-contained, code-first demo of a stroke-less Border

#### 🟢 Sonnet 5 Review

Borderless switch style and yellow background match exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 22. Box View — 🟢/⏳
<sub>box_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/box_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/box_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/box_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/box_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/box_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/box_view_dark.png" /></td></tr></table>

ports BoxViewPage.xaml (+ BoxViewPage.xaml.cs)

#### 🟢 Sonnet 5 Review

All BoxView variants (default, color, gradient background, corner radius, complex corner radius) match exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 23. Button — 🔴/⏳
<sub>button</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/button_light.png" /></td><td><img width="300px" src="captures/ios/cpp/button_light.png" /></td><td><img width="300px" src="captures/ios/xaml/button_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/button_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/button_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/button_dark.png" /></td></tr></table>

ports ButtonPage.xaml (+ ButtonPage.xaml.cs)

#### 🔴 Sonnet 5 Review

The 'settings' ImageButton rows render huge/oversized in cpp with a gear icon plus text pushing other controls out of view, while MAUI shows compact text-only rows — a major layout/sizing bug present in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 24. Carousel Page — 🔴/⏳
<sub>carousel_page</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/carousel_page_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/carousel_page_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/carousel_page_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/carousel_page_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/carousel_page_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/carousel_page_dark.gif" /></td></tr></table>

Carousel Page

#### 🔴 Sonnet 5 Review

MAUI's reference render is missing the Prev/Next links and the 'Position 0 — current: Item 1' label entirely (blank below Item 1), while the C++ port shows them in both themes — a major content divergence from the ground-truth capture.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 25. Carousel View — ⏳/⏳
<sub>carousel_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

#### ⏳ Sonnet 5 Review

_Not yet reviewed._

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 26. Chat Example — 🟢/⏳
<sub>chat_example</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/chat_example_light.png" /></td><td><img width="300px" src="captures/ios/cpp/chat_example_light.png" /></td><td><img width="300px" src="captures/ios/xaml/chat_example_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/chat_example_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/chat_example_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/chat_example_dark.png" /></td></tr></table>

ports ChatExample.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.ItemSizeGalleries.ChatExample), tracking the maui-compare reference demo ~/maui-compare/Pages/ChatExamplePage.cs (the visual-parity oracle)

#### 🟢 Sonnet 5 Review

Chat bubble text, colors, and layout match in both themes; the only difference is the MAUI capture crops closer to the top status bar, which is the exempted outer-harness-inset difference.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 27. Check Box — 🟢/⏳
<sub>check_box</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/check_box_light.png" /></td><td><img width="300px" src="captures/ios/cpp/check_box_light.png" /></td><td><img width="300px" src="captures/ios/xaml/check_box_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/check_box_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/check_box_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/check_box_dark.png" /></td></tr></table>

ports CheckBoxPage.xaml (+ .xaml.cs) Mirrors the MAUI gallery page: a vertical stack of headlined CheckBox states — Default, Colored (Color=Purple), Disabled, Disabled+Colored+Checked — followed by a &amp;quot;Change IsChecked&amp;quot; row pairing a Button

#### 🟢 Sonnet 5 Review

All checkbox/radio variants (default, colored, disabled, disabled colored, change-IsChecked) match exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 28. Chrome — 🟢/⏳
<sub>chrome</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/chrome_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/chrome_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/chrome_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/chrome_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/chrome_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/chrome_dark.gif" /></td></tr></table>

a self-contained demo page for the W1-11 window-chrome family: page toolbar items (primary + secondary), a menu bar (File menu with items, a separator and a sub-menu), a context flyout (right-click menu) on a button, and a tooltip — all wir

#### 🟢 Sonnet 5 Review

Layouts, text, and colors match closely in both light and dark themes; only trivial timestamp/status-bar differences.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 29. Clip — 🔴/⏳
<sub>clip</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/clip_light.png" /></td><td><img width="300px" src="captures/ios/cpp/clip_light.png" /></td><td><img width="300px" src="captures/ios/xaml/clip_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/clip_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/clip_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/clip_dark.png" /></td></tr></table>

ports ClipPage.xaml The C# page (Pages/Core/ClipPage.xaml; its .xaml.cs is an empty InitializeComponent) is a ScrollView over a StackLayout that shows the SAME dotnet_bot.png image five times, each successive copy carrying a different geome

#### 🔴 Sonnet 5 Review

MAUI's reference renders blank gray placeholders for all three clipped-image demos in both themes (image failed to load), while the C++ port correctly loads and renders the source robot image clipped to rectangle/ellipse/geometry-group shapes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 30. Clip Corner Radius — 🔴/⏳
<sub>clip_corner_radius</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/clip_corner_radius_light.png" /></td><td><img width="300px" src="captures/ios/cpp/clip_corner_radius_light.png" /></td><td><img width="300px" src="captures/ios/xaml/clip_corner_radius_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/clip_corner_radius_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/clip_corner_radius_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/clip_corner_radius_dark.png" /></td></tr></table>

ports ClipCornerRadiusGallery.xaml (+ .xaml.cs) The C# page (Pages/Controls/ShapesGalleries/ClipCornerRadiusGallery.xaml) is a StackLayout (Padding=12) that demonstrates DRIVING a RoundRectangleGeometry&amp;#x27;s per-corner CornerRadius from four s

#### 🔴 Sonnet 5 Review

MAUI shows an empty gray box for the RoundRectangleGeometry-clipped image in both themes, while cpp shows the actual loaded dog photo correctly rounded; sliders/labels otherwise match.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 31. Clip Gallery — 🔴/⏳
<sub>clip_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/clip_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/clip_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/clip_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/clip_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/clip_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/clip_gallery_dark.png" /></td></tr></table>

ports ClipGallery.xaml The C# page (Pages/Controls/ShapesGalleries/ClipGallery.xaml; its .xaml.cs is an empty InitializeComponent) is a ScrollView over a StackLayout (Padding=12) that shows the SAME &amp;quot;oasis.jpg&amp;quot; image SEVEN times — one bare

#### 🔴 Sonnet 5 Review

MAUI reference shows blank gray placeholders for all clipped-image variants in both themes, while cpp renders the actual dog photo correctly clipped in each case; layout and labels otherwise match.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 32. Clip Views — 🟢/⏳
<sub>clip_views</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/clip_views_light.png" /></td><td><img width="300px" src="captures/ios/cpp/clip_views_light.png" /></td><td><img width="300px" src="captures/ios/xaml/clip_views_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/clip_views_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/clip_views_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/clip_views_dark.png" /></td></tr></table>

ports ClipViewsGallery.xaml The C# page (Pages/Controls/ShapesGalleries/ClipViewsGallery.xaml; no code-behind beyond an empty InitializeComponent) is a ScrollView over a StackLayout (Padding=12) that proves the Clip surface (VisualElement.C

#### 🟢 Sonnet 5 Review

Layout, colors, and clipped stack shapes match well in both themes; only a minor tone difference in the semi-transparent pink/red search-bar clip strip in dark mode.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 33. Clipping — 🟡/⏳
<sub>clipping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/clipping_light.png" /></td><td><img width="300px" src="captures/ios/cpp/clipping_light.png" /></td><td><img width="300px" src="captures/ios/xaml/clipping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/clipping_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/clipping_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/clipping_dark.png" /></td></tr></table>

compare oracle ~/maui-compare/Pages/ClippingPage.cs (itself written to mirror this gallery page)

#### 🟡 Sonnet 5 Review

Layout, orange/red boxes, numbered items, and clip toggling all match in both themes; MAUI's reference fails to load the two coffee-cup icons (blank) while cpp renders them correctly, a minor content difference given the rest is pixel-accurate.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 34. Collectionview — 🟢/⏳
<sub>collectionview</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/collectionview_light.png" /></td><td><img width="300px" src="captures/ios/cpp/collectionview_light.png" /></td><td><img width="300px" src="captures/ios/xaml/collectionview_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/collectionview_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/collectionview_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/collectionview_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;collectionview&amp;quot; demo (ComparePages.CollectionViewPage()), the shipped-.NET-MAUI reference for the visual-parity comparison: a CollectionView over 24 captioned items, a string Header (&amp;quot;This is the

#### 🟢 Sonnet 5 Review

Grid of numbered file-name cells matches exactly in both themes and both list contents/ordering are identical; MAUI's status bar merely overlaps its header text at the very top (harness clipping artifact, not a port bug).

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 35. Composition Gallery — 🟢/⏳
<sub>composition_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/composition_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/composition_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/composition_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/composition_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/composition_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/composition_gallery_dark.png" /></td></tr></table>

ports CompositionGallery.xaml A self-contained, code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/CompositionGallery.xaml: a StackLayout holding two Beige 250x250 Grids (Margin 12) that compose multiple overlappi

#### 🟢 Sonnet 5 Review

Both the shape-composition canvas (triangle/circle/line overlay) and the line-diagram canvas are pixel-identical between MAUI and cpp in both light and dark themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 36. Containers — 🟢/⏳
<sub>containers</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/containers_light.png" /></td><td><img width="300px" src="captures/ios/cpp/containers_light.png" /></td><td><img width="300px" src="captures/ios/xaml/containers_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/containers_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/containers_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/containers_dark.png" /></td></tr></table>

a self-contained demo page for the W1-07 container set: a scroll_view hosting a vertical stack of content-hosting containers — a border-framed label (stroke + dashed outline + rounded shape), a legacy frame (BorderColor/CornerRadius/HasShad

#### 🟢 Sonnet 5 Review

Dashed border, solid red frame, and nested text stack match in both light and dark themes; only trivial status-bar/time differences.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 37. Content View — 🟢/⏳
<sub>content_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/content_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/content_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/content_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/content_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/content_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/content_view_dark.png" /></td></tr></table>

ports ContentViewPage.xaml (+ ContentViewPage.xaml.cs), code-first

#### 🟢 Sonnet 5 Review

ContentView/Content text hierarchy and the Swap content link match exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 38. Context Flyout — 🟢/⏳
<sub>context_flyout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/context_flyout_light.png" /></td><td><img width="300px" src="captures/ios/cpp/context_flyout_light.png" /></td><td><img width="300px" src="captures/ios/xaml/context_flyout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/context_flyout_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/context_flyout_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/context_flyout_dark.png" /></td></tr></table>

ports ContextFlyoutPage.xaml (+ ContextFlyoutPage.xaml.cs) The C# page attaches a MenuFlyout as the FlyoutBase.ContextFlyout (right-click / long-press menu) of several controls and wires each menu item to a handler: - a Button (&amp;quot;Increment b

#### 🟢 Sonnet 5 Review

Toggle, entry, button chrome, and the live Bing consent web dialog all render identically in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 39. Controls Stack — 🟢/⏳
<sub>controls_stack</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/controls_stack_light.png" /></td><td><img width="300px" src="captures/ios/cpp/controls_stack_light.png" /></td><td><img width="300px" src="captures/ios/xaml/controls_stack_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/controls_stack_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/controls_stack_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/controls_stack_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;controls_stack&amp;quot; demo (ComparePages.ControlsStack()), the shipped-.NET-MAUI reference for the visual-parity comparison: a VerticalStackLayout (Spacing 12, Padding 16) showcasing the basic widgets

#### 🟢 Sonnet 5 Review

Button, entry, editor, search bar, checkbox, switch, slider, stepper and progress bar all match in size/color/layout across both themes; negligible spacing nit before 'An Editor' in light.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 40. Custom Layout — 🟢/⏳
<sub>custom_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/custom_layout_light.png" /></td><td><img width="300px" src="captures/ios/cpp/custom_layout_light.png" /></td><td><img width="300px" src="captures/ios/xaml/custom_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/custom_layout_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/custom_layout_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/custom_layout_dark.png" /></td></tr></table>

ports CustomLayoutPage.xaml (+ CustomLayoutPage.xaml.cs)

#### 🟢 Sonnet 5 Review

Top/Left/Left/Right/Right/Bottom custom-layout positions match exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 41. Custom Size Swipe — 🟢/⏳
<sub>custom_size_swipe</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/custom_size_swipe_light.png" /></td><td><img width="300px" src="captures/ios/cpp/custom_size_swipe_light.png" /></td><td><img width="300px" src="captures/ios/xaml/custom_size_swipe_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/custom_size_swipe_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/custom_size_swipe_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/custom_size_swipe_dark.png" /></td></tr></table>

ports CustomSizeSwipeViewGallery.xaml (+ .xaml.cs) The MAUI CustomSizeSwipeViewGallery is a single SwipeView whose Left / Right / Top item collections each reveal CUSTOM-SIZED content: a SwipeItemView wrapping a Grid/StackLayout with an exp

#### 🟢 Sonnet 5 Review

SwipeView content and revealed-state text match exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 42. Custom Swipe Item View — 🟢/⏳
<sub>custom_swipe_item_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/custom_swipe_item_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/custom_swipe_item_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/custom_swipe_item_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/custom_swipe_item_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/custom_swipe_item_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/custom_swipe_item_view_dark.png" /></td></tr></table>

ports CustomSwipeItemViewGallery.xaml A self-contained, code-first port of the .NET MAUI &amp;quot;CustomSwipeItem&amp;quot; gallery: a message-list row whose right swipe reveals a CUSTOM-content swipe item (a swipe_item_view, not a plain swipe_item)

#### 🟢 Sonnet 5 Review

Custom swipe item card (purple background, title, date) matches exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 43. Cv Visual States — 🔴/⏳
<sub>cv_visual_states</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/cv_visual_states_light.png" /></td><td><img width="300px" src="captures/ios/cpp/cv_visual_states_light.png" /></td><td><img width="300px" src="captures/ios/xaml/cv_visual_states_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/cv_visual_states_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/cv_visual_states_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/cv_visual_states_dark.png" /></td></tr></table>

ports CollectionViewGalleries/SelectionGalleries/ VisualStatesGallery.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🔴 Sonnet 5 Review

Light theme matches (Item 1-4 lists render correctly), but in dark theme the C++ port's CollectionView items render as blank/invisible rows (missing item text) under both 'Single Selection' and 'Multi Selection' sections, while MAUI's dark reference correctly shows all item labels in white text.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 44. Data Template Selector — 🟢/⏳
<sub>data_template_selector</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/data_template_selector_light.png" /></td><td><img width="300px" src="captures/ios/cpp/data_template_selector_light.png" /></td><td><img width="300px" src="captures/ios/xaml/data_template_selector_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/data_template_selector_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/data_template_selector_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/data_template_selector_dark.png" /></td></tr></table>

ports DataTemplateSelectorGallery.xaml (+ DataTemplateSelectorGallery.xaml.cs, including its WeekendSelector + SearchTermSelector classes)

#### 🟢 Sonnet 5 Review

Matches in both themes: text list content, colors, and search bar rendering are identical between MAUI and the C++ port.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 45. Date Picker — 🔴/⏳
<sub>date_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/date_picker_light.png" /></td><td><img width="300px" src="captures/ios/cpp/date_picker_light.png" /></td><td><img width="300px" src="captures/ios/xaml/date_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/date_picker_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/date_picker_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/date_picker_dark.png" /></td></tr></table>

ports DatePickerPage.xaml (+ DatePickerPage.xaml.cs)

#### 🔴 Sonnet 5 Review

The third 'Background' row (gradient) is wrong color in the port: MAUI shows a pink-to-purple gradient in both themes, while the C++ port shows a blue-to-teal gradient. All other rows match correctly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 46. Device — 🟢/⏳
<sub>device</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/device_light.png" /></td><td><img width="300px" src="captures/ios/cpp/device_light.png" /></td><td><img width="300px" src="captures/ios/xaml/device_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/device_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/device_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/device_dark.png" /></td></tr></table>

ports DevicePage.xaml

#### 🟢 Sonnet 5 Review

Platform/Idiom/Version text renders identically in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 47. Dispatcher — 🟢/⏳
<sub>dispatcher</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/dispatcher_light.png" /></td><td><img width="300px" src="captures/ios/cpp/dispatcher_light.png" /></td><td><img width="300px" src="captures/ios/xaml/dispatcher_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/dispatcher_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/dispatcher_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/dispatcher_dark.png" /></td></tr></table>

ports DispatcherPage.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 Review

All text blocks and blue action links match exactly in both light and dark themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 48. Drag Drop — 🟢/⏳
<sub>drag_drop</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/drag_drop_light.png" /></td><td><img width="300px" src="captures/ios/cpp/drag_drop_light.png" /></td><td><img width="300px" src="captures/ios/xaml/drag_drop_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/drag_drop_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/drag_drop_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/drag_drop_dark.png" /></td></tr></table>

ports DragAndDropBetweenLayouts.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.DragAndDropBetweenLayouts)

#### 🟢 Sonnet 5 Review

Color swatches, rainbow list, and all status text match exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 49. Editor — 🟢/⏳
<sub>editor</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/editor_light.png" /></td><td><img width="300px" src="captures/ios/cpp/editor_light.png" /></td><td><img width="300px" src="captures/ios/xaml/editor_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/editor_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/editor_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/editor_dark.png" /></td></tr></table>

ports EditorPage.xaml (Microsoft.Maui.Controls sample gallery)

#### 🟢 Sonnet 5 Review

All editor fields, placeholder text, colored labels, and font sizes match in both themes; only a minor left-margin/inset difference (exempt).

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 50. Effects — 🟢/⏳
<sub>effects</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/effects_light.png" /></td><td><img width="300px" src="captures/ios/cpp/effects_light.png" /></td><td><img width="300px" src="captures/ios/xaml/effects_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/effects_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/effects_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/effects_dark.png" /></td></tr></table>

ports EffectsPage.xaml (Maui.Controls.Sample.Pages.EffectsPage)

#### 🟢 Sonnet 5 Review

The MAUI light-theme screenshot is a broken/mis-timed capture; the dark-theme comparison shows the C++ port's content matches MAUI exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 51. Ellipse Gallery — 🟢/⏳
<sub>ellipse_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ellipse_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/ellipse_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/ellipse_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ellipse_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/ellipse_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/ellipse_gallery_dark.png" /></td></tr></table>

ports EllipseGallery.xaml A self-contained, code-first port of the MAUI Shapes EllipseGallery (Pages/Controls/ShapesGalleries/EllipseGallery.xaml + .xaml.cs)

#### 🟢 Sonnet 5 Review

Rectangle, circle, and ellipse shapes with stroke/dash all render identically in both light and dark themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 52. Empty View — 🟢/⏳
<sub>empty_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/empty_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/empty_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_dark.png" /></td></tr></table>

ports EmptyViewStringGallery.xaml (+ EmptyViewStringGallery.xaml.cs)

#### 🟢 Sonnet 5 Review

CollectionView data list renders identically in both themes; no differences.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 53. Empty View Load Simulate — 🟢/⏳
<sub>empty_view_load_simulate</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/empty_view_load_simulate_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_load_simulate_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_load_simulate_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/empty_view_load_simulate_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_load_simulate_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_load_simulate_dark.gif" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewLoadSimulateGallery.xaml (+ EmptyViewLoadSimulateGallery.xaml.cs)

#### 🟢 Sonnet 5 Review

Loading-simulation placeholder text centered identically in both themes; no differences.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 54. Empty View Null — 🟢/⏳
<sub>empty_view_null</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/empty_view_null_light.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_null_light.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_null_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/empty_view_null_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_null_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_null_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewNullGallery.xaml (+ EmptyViewNullGallery.xaml.cs)

#### 🟢 Sonnet 5 Review

Nothing to display placeholder matches exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 55. Empty View Rtl — 🟢/⏳
<sub>empty_view_rtl</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/empty_view_rtl_light.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_rtl_light.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_rtl_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/empty_view_rtl_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_rtl_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_rtl_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewRTLGallery.xaml (+ EmptyViewRTLGallery.xaml.cs)

#### 🟢 Sonnet 5 Review

RTL/LTR toggle bar and three-column grid layout match exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 56. Empty View Selector — 🟢/⏳
<sub>empty_view_selector</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/empty_view_selector_light.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_selector_light.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_selector_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/empty_view_selector_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_selector_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_selector_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewWithDataTemplateSelector.xaml (+ .xaml.cs, incl

#### 🟢 Sonnet 5 Review

Instruction text, filter bar, and filtered single result render identically in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 57. Empty View Swap — 🟢/⏳
<sub>empty_view_swap</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/empty_view_swap_light.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_swap_light.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_swap_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/empty_view_swap_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_swap_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_swap_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewSwapGallery.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 Review

Toggle switch, Clear/Fill links, and three-column list match exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 58. Empty View Template — 🟢/⏳
<sub>empty_view_template</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/empty_view_template_light.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_template_light.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_template_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/empty_view_template_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_template_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_template_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewTemplateGallery.xaml (+ EmptyViewTemplateGallery.xaml.cs)

#### 🟢 Sonnet 5 Review

Templated three-column list renders identically in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 59. Empty View View — 🟢/⏳
<sub>empty_view_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/empty_view_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/empty_view_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_view_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewViewGallery.xaml (+ EmptyViewViewGallery.xaml.cs)

#### 🟢 Sonnet 5 Review

View-based layout matches exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 60. Entry — 🟢/⏳
<sub>entry</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/entry_light.png" /></td><td><img width="300px" src="captures/ios/cpp/entry_light.png" /></td><td><img width="300px" src="captures/ios/xaml/entry_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/entry_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/entry_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/entry_dark.png" /></td></tr></table>

ports EntryPage.xaml (Microsoft.Maui.Controls sample gallery)

#### 🟢 Sonnet 5 Review

Identical layout, purple text/placeholder colors, checkbox, cursor slider, and dark-mode inversion all match exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 61. Filter Collection — 🟢/⏳
<sub>filter_collection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/filter_collection_light.png" /></td><td><img width="300px" src="captures/ios/cpp/filter_collection_light.png" /></td><td><img width="300px" src="captures/ios/xaml/filter_collection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/filter_collection_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/filter_collection_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/filter_collection_dark.png" /></td></tr></table>

ports FilterCollectionView.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 Review

Toggle, filter bar, and two-column filename list are pixel-identical in both light and dark themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 62. Filter Selection — 🟢/⏳
<sub>filter_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/filter_selection_light.png" /></td><td><img width="300px" src="captures/ios/cpp/filter_selection_light.png" /></td><td><img width="300px" src="captures/ios/xaml/filter_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/filter_selection_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/filter_selection_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/filter_selection_dark.png" /></td></tr></table>

ports FilterSelection.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.FilterSelection)

#### 🟢 Sonnet 5 Review

Instructional text, filter bar, Reset link, Selected label, and list content match in both themes; layout is consistent.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 63. Flex Layout — 🔴/⏳
<sub>flex_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/flex_layout_light.png" /></td><td><img width="300px" src="captures/ios/cpp/flex_layout_light.png" /></td><td><img width="300px" src="captures/ios/xaml/flex_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/flex_layout_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/flex_layout_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/flex_layout_dark.png" /></td></tr></table>

ports FlexLayoutPage.xaml A self-contained, code-first demo of the FlexLayout control: the classic &amp;quot;holy grail&amp;quot; page layout built from nested flexboxes

#### 🔴 Sonnet 5 Review

Header/content/footer color bands and layout match, but in dark mode MAUI renders the HEADER/CONTENT/FOOTER labels in white text while the C++ port renders them in dark/gray text, making them nearly illegible against the colored backgrounds.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 64. Focus — 🟢/⏳
<sub>focus</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/focus_light.png" /></td><td><img width="300px" src="captures/ios/cpp/focus_light.png" /></td><td><img width="300px" src="captures/ios/xaml/focus_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/focus_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/focus_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/focus_dark.png" /></td></tr></table>

ports FocusPage.xaml (+ FocusPage.xaml.cs) The C# FocusPage is a focus-subsystem demo: an Entry whose Focused/Unfocused events (OnFocusEntryFocusChanged) append &amp;quot;Focused&amp;quot;/&amp;quot;Unfocused&amp;quot; lines to a scrolling InfoLabel, plus two buttons — &amp;quot;Focus

#### 🟢 Sonnet 5 Review

Focus target entry, Focus/Unfocus links, and IsFocused label match exactly in both light and dark themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 65. Fonts — 🟢/⏳
<sub>fonts</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/fonts_light.png" /></td><td><img width="300px" src="captures/ios/cpp/fonts_light.png" /></td><td><img width="300px" src="captures/ios/xaml/fonts_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/fonts_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/fonts_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/fonts_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;fonts&amp;quot; demo (ComparePages.Fonts()), the shipped-.NET-MAUI reference for the visual-parity comparison: a ScrollView over a VerticalStackLayout (Spacing 8, Padding 16) of nine Labels — Title/Subtit

#### 🟢 Sonnet 5 Review

All font style rows match in size, weight, italics, and color inversion for dark mode.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 66. Footer Only String — 🟢/⏳
<sub>footer_only_string</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/footer_only_string_light.png" /></td><td><img width="300px" src="captures/ios/cpp/footer_only_string_light.png" /></td><td><img width="300px" src="captures/ios/xaml/footer_only_string_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/footer_only_string_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/footer_only_string_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/footer_only_string_dark.png" /></td></tr></table>

ports FooterOnlyString.xaml (+ FooterOnlyString.xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Sonnet 5 Review

List content and bold footer text are identical between cpp and maui in both themes; only difference is harness status-bar overlap in the maui shots, which is exempted.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 67. Formatted Text — 🟢/⏳
<sub>formatted_text</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/formatted_text_light.png" /></td><td><img width="300px" src="captures/ios/cpp/formatted_text_light.png" /></td><td><img width="300px" src="captures/ios/xaml/formatted_text_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/formatted_text_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/formatted_text_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/formatted_text_dark.png" /></td></tr></table>

a self-contained demo page for the G1 rich-text slice: a label whose FormattedText is built from several styled spans (bold / italic / colored / underlined / kerned), plus a plain label proving the Text ⇄ FormattedText exclusivity

#### 🟢 Sonnet 5 Review

Formatted spans (bold red, italic underlined, kerned, plain label) render identically in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 68. Frame — ⏳/⏳
<sub>frame</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

#### ⏳ Sonnet 5 Review

_Not yet reviewed._

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 69. Gestures — 🟢/⏳
<sub>gestures</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/gestures_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/gestures_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/gestures_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/gestures_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/gestures_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/gestures_dark.gif" /></td></tr></table>

ports GesturesPage.xaml (+ .xaml.cs) The MAUI GesturesPage.xaml is a *gallery navigation* page: a CollectionView listing gesture-demo sections that the shell navigates into

#### 🟢 Sonnet 5 Review

Blue gesture-target rectangle, header/last-gesture text, and colors match exactly in both light and dark themes; only trivial outer-padding differs.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 70. Gradient — 🟢/⏳
<sub>gradient</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/gradient_light.png" /></td><td><img width="300px" src="captures/ios/cpp/gradient_light.png" /></td><td><img width="300px" src="captures/ios/xaml/gradient_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/gradient_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/gradient_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/gradient_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;gradient&amp;quot; demo (ComparePages.Gradient()), the shipped-.NET-MAUI reference for the visual-parity comparison: a VerticalStackLayout (Spacing 12, Padding 16) of two captioned 60px BoxViews — a Linea

#### 🟢 Sonnet 5 Review

LinearGradientBrush and RadialGradientBrush bars render identically in both themes, matching colors, positions, and sizes precisely.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 71. Grid — 🟢/⏳
<sub>grid</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/grid_light.png" /></td><td><img width="300px" src="captures/ios/cpp/grid_light.png" /></td><td><img width="300px" src="captures/ios/xaml/grid_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/grid_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/grid_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/grid_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;grid&amp;quot; demo (ComparePages.GridPage()), the shipped-.NET-MAUI reference for the visual-parity comparison: a Grid (Padding 16, Row/ColumnSpacing 6) with RowDefinitions Auto / 80 / 80 and two Star co

#### 🟢 Sonnet 5 Review

2x2 colored grid (red/green/blue/orange) matches exactly in size, color, and position in both light and dark themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 72. Grid Definitions — ⏳/⏳
<sub>grid_definitions</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

#### ⏳ Sonnet 5 Review

_Not yet reviewed._

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 73. Grid Grouping — 🟢/⏳
<sub>grid_grouping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/grid_grouping_light.png" /></td><td><img width="300px" src="captures/ios/cpp/grid_grouping_light.png" /></td><td><img width="300px" src="captures/ios/xaml/grid_grouping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/grid_grouping_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/grid_grouping_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/grid_grouping_dark.png" /></td></tr></table>

ports GroupingGalleries/GridGrouping.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 Review

Grouped list content, section headers (green), 'Total members' captions (orange), and layout match exactly in both themes; only the outer inset/crop differs.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 74. Grid Layout — ⏳/⏳
<sub>grid_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

#### ⏳ Sonnet 5 Review

_Not yet reviewed._

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 75. Grouping No Templates — 🟢/⏳
<sub>grouping_no_templates</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/grouping_no_templates_light.png" /></td><td><img width="300px" src="captures/ios/cpp/grouping_no_templates_light.png" /></td><td><img width="300px" src="captures/ios/xaml/grouping_no_templates_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/grouping_no_templates_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/grouping_no_templates_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/grouping_no_templates_dark.png" /></td></tr></table>

ports GroupingGalleries/GroupingNoTemplates.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 Review

Flat list of names matches exactly in ordering and text in both themes; MAUI's clock overlaps first row due to its inset crop but content is identical.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 76. Grouping Plus Selection — 🟢/⏳
<sub>grouping_plus_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/grouping_plus_selection_light.png" /></td><td><img width="300px" src="captures/ios/cpp/grouping_plus_selection_light.png" /></td><td><img width="300px" src="captures/ios/xaml/grouping_plus_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/grouping_plus_selection_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/grouping_plus_selection_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/grouping_plus_selection_dark.png" /></td></tr></table>

ports CollectionViewGalleries/GroupingGalleries/ GroupingPlusSelection.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 Review

Grouped list with headers and counts matches exactly in both themes, same as the other grouping pages.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 77. Header Footer — 🟢/⏳
<sub>header_footer</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/header_footer_light.png" /></td><td><img width="300px" src="captures/ios/cpp/header_footer_light.png" /></td><td><img width="300px" src="captures/ios/xaml/header_footer_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/header_footer_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/header_footer_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/header_footer_dark.png" /></td></tr></table>

ports HeaderFooterString.xaml (+ HeaderFooterString.xaml.cs)

#### 🟢 Sonnet 5 Review

String header/footer text and list items match exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 78. Header Footer Grid — 🟡/⏳
<sub>header_footer_grid</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/header_footer_grid_light.png" /></td><td><img width="300px" src="captures/ios/cpp/header_footer_grid_light.png" /></td><td><img width="300px" src="captures/ios/xaml/header_footer_grid_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/header_footer_grid_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/header_footer_grid_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/header_footer_grid_dark.png" /></td></tr></table>

ports HeaderFooterGrid.xaml (+ HeaderFooterGrid.xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟡 Sonnet 5 Review

Layout, grid content, and header/footer text match in both themes, but the cpp render shows a dog photo background behind the header/footer text while the MAUI reference shows a plain background (light) — likely a network-image load-timing difference in the MAUI capture rather than a port bug, but flagged as a visible discrepancy.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 79. Header Footer Grid Horizontal — 🟢/⏳
<sub>header_footer_grid_horizontal</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/header_footer_grid_horizontal_light.png" /></td><td><img width="300px" src="captures/ios/cpp/header_footer_grid_horizontal_light.png" /></td><td><img width="300px" src="captures/ios/xaml/header_footer_grid_horizontal_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/header_footer_grid_horizontal_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/header_footer_grid_horizontal_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/header_footer_grid_horizontal_dark.png" /></td></tr></table>

ports HeaderFooterGridHorizontal.xaml (+ HeaderFooterGridHorizontal.xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Sonnet 5 Review

Toggle Header/Footer links and list rows render identically in both themes; layout, text, and colors match between MAUI and cpp.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 80. Header Footer Template — 🔴/⏳
<sub>header_footer_template</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/header_footer_template_light.png" /></td><td><img width="300px" src="captures/ios/cpp/header_footer_template_light.png" /></td><td><img width="300px" src="captures/ios/xaml/header_footer_template_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/header_footer_template_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/header_footer_template_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/header_footer_template_dark.png" /></td></tr></table>

ports HeaderFooterTemplate.xaml (+ .xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🔴 Sonnet 5 Review

MAUI's header/footer template rows render as blank/transparent bands (only timestamp text visible), while the cpp port fills the header and footer template with an actual photo image in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 81. Header Footer View — 🔴/⏳
<sub>header_footer_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/header_footer_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/header_footer_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/header_footer_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/header_footer_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/header_footer_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/header_footer_view_dark.png" /></td></tr></table>

ports HeaderFooterView.xaml (+ .xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🔴 Sonnet 5 Review

MAUI shows the header/footer as plain text bands with no background image, but cpp renders full-bleed photo backgrounds behind the header/footer text in both light and dark themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 82. Hit Testing — 🟢/⏳
<sub>hit_testing</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/hit_testing_light.png" /></td><td><img width="300px" src="captures/ios/cpp/hit_testing_light.png" /></td><td><img width="300px" src="captures/ios/xaml/hit_testing_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/hit_testing_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/hit_testing_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/hit_testing_dark.png" /></td></tr></table>

ports HitTestingPage.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.HitTestingPage)

#### 🟢 Sonnet 5 Review

Selection text, shapes, scale/rotation labels, and rounded rectangle all match closely between MAUI and cpp in both themes; only trivial capture-crop differences at the bottom edge.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 83. Horizontal Stack — 🟢/⏳
<sub>horizontal_stack</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/horizontal_stack_light.png" /></td><td><img width="300px" src="captures/ios/cpp/horizontal_stack_light.png" /></td><td><img width="300px" src="captures/ios/xaml/horizontal_stack_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/horizontal_stack_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/horizontal_stack_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/horizontal_stack_dark.png" /></td></tr></table>

Horizontal Stack

#### 🟢 Sonnet 5 Review

The six colored swatches in the HorizontalStackLayout are pixel-identical in position, size and color between MAUI and cpp for both light and dark themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 84. Horizontal Stack Layout — ⏳/⏳
<sub>horizontal_stack_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

#### ⏳ Sonnet 5 Review

_Not yet reviewed._

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 85. Hybrid Web View — 🟡/⏳
<sub>hybrid_web_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/hybrid_web_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/hybrid_web_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/hybrid_web_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/hybrid_web_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/hybrid_web_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/hybrid_web_view_dark.png" /></td></tr></table>

ports HybridWebViewPage.xaml (+ HybridWebViewPage.xaml.cs)

#### 🟡 Sonnet 5 Review

MAUI's action links show full untruncated text while cpp ellipsizes them in both themes; otherwise layout matches.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 86. Image — 🟡/⏳
<sub>image</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/image_light.png" /></td><td><img width="300px" src="captures/ios/cpp/image_light.png" /></td><td><img width="300px" src="captures/ios/xaml/image_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/image_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/image_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/image_dark.png" /></td></tr></table>

ports ImagePage.xaml (+ ImagePage.xaml.cs)

#### 🟡 Sonnet 5 Review

MAUI's captured frame shows the UriSource image loaded but the FileSource image blank/not yet loaded, whereas cpp's captured frame shows both images loaded - likely a capture-timing difference rather than a rendering defect.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 87. Image Button — 🔴/⏳
<sub>image_button</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/image_button_light.png" /></td><td><img width="300px" src="captures/ios/cpp/image_button_light.png" /></td><td><img width="300px" src="captures/ios/xaml/image_button_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/image_button_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/image_button_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/image_button_dark.png" /></td></tr></table>

ports ImageButtonPage.xaml (+ ImageButtonPage.xaml.cs) A self-contained, code-first demo page for the ImageButton control (the C# gallery-page convention, mirroring the input_controls_page / image_page pattern)

#### 🔴 Sonnet 5 Review

Under Custom Size, cpp renders a small purple submarine thumbnail image in both themes that is completely absent (blank) in the MAUI reference; all other rows match.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 88. Indicator — 🟢/⏳
<sub>indicator</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/indicator_light.png" /></td><td><img width="300px" src="captures/ios/cpp/indicator_light.png" /></td><td><img width="300px" src="captures/ios/xaml/indicator_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/indicator_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/indicator_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/indicator_dark.png" /></td></tr></table>

ports IndicatorPage.xaml A self-contained, code-first demo of the IndicatorView control

#### 🟢 Sonnet 5 Review

Dot indicators, colors, shapes, and sizes match MAUI exactly in both light and dark themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 89. Input Controls — 🟢/⏳
<sub>input_controls</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/input_controls_light.png" /></td><td><img width="300px" src="captures/ios/cpp/input_controls_light.png" /></td><td><img width="300px" src="captures/ios/xaml/input_controls_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/input_controls_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/input_controls_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/input_controls_dark.png" /></td></tr></table>

a self-contained demo page for the W1-05 input-control set: editor, search_bar, radio_button (+ the radio_button_group attached grouping) and image_button on one vertical stack, wired together so every input drives a visible output (the C#

#### 🟢 Sonnet 5 Review

Entry, search bar, and radio buttons render identically to MAUI in both themes; only trivial page-inset differences (not scored).

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 90. Input Transparent — 🟢/⏳
<sub>input_transparent</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/input_transparent_light.png" /></td><td><img width="300px" src="captures/ios/cpp/input_transparent_light.png" /></td><td><img width="300px" src="captures/ios/xaml/input_transparent_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/input_transparent_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/input_transparent_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/input_transparent_dark.png" /></td></tr></table>

ports InputTransparentPage.xaml (Maui.Controls.Sample.Pages.InputTransparentPage)

#### 🟢 Sonnet 5 Review

Text, buttons, and overlapping-label rendering (a shared quirk present in both MAUI and cpp) match exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 91. Invalidate Brush — 🟢/⏳
<sub>invalidate_brush</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/invalidate_brush_light.png" /></td><td><img width="300px" src="captures/ios/cpp/invalidate_brush_light.png" /></td><td><img width="300px" src="captures/ios/xaml/invalidate_brush_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/invalidate_brush_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/invalidate_brush_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/invalidate_brush_dark.png" /></td></tr></table>

ports InvalidateBrushGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/InvalidateBrushGallery.xaml (&amp;quot;Invalidate Brushes Playground&amp;quot;): a VerticalStackLayout (Padding 12) with — - a &amp;quot;Change color&amp;quot; Bu

#### 🟢 Sonnet 5 Review

Button, border color, and label match MAUI exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 92. Invalidate Shadow Host — 🟢/⏳
<sub>invalidate_shadow_host</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/invalidate_shadow_host_light.png" /></td><td><img width="300px" src="captures/ios/cpp/invalidate_shadow_host_light.png" /></td><td><img width="300px" src="captures/ios/xaml/invalidate_shadow_host_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/invalidate_shadow_host_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/invalidate_shadow_host_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/invalidate_shadow_host_dark.png" /></td></tr></table>

ports InvalidateShadowHostPage.xaml A self-contained, code-first demo that a shadow re-applies (invalidates) when its host&amp;#x27;s size changes, mirroring the C# core gallery page (Pages/Core/ShadowGalleries/InvalidateShadowHostPage.xaml + .xaml.

#### 🟢 Sonnet 5 Review

Sliders, labels, and the shadowed box render identically to MAUI in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 93. Ios Blur Effect — 🟡/⏳
<sub>ios_blur_effect</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_blur_effect_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/ios_blur_effect_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/ios_blur_effect_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_blur_effect_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/ios_blur_effect_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/ios_blur_effect_dark.gif" /></td></tr></table>

ports iOSBlurEffectPage.xaml The .NET MAUI PlatformSpecifics sample (Pages/PlatformSpecifics/iOS/iOSBlurEffectPage.xaml + .xaml.cs): an Image (Source=&amp;quot;oasis.jpg&amp;quot;) carrying the iOSSpecific VisualElement.BlurEffect knob (XAML seeds it to Extr

#### 🟡 Sonnet 5 Review

Layout/text/links match MAUI, but the MAUI reference screenshots show no image loaded at all while cpp shows a fully loaded photo behind the blur controls; cannot fully verify blur rendering parity due to this reference gap.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 94. Ios Date Picker — 🟢/⏳
<sub>ios_date_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_date_picker_light.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_date_picker_light.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_date_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_date_picker_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_date_picker_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_date_picker_dark.png" /></td></tr></table>

ports iOSDatePickerPage.xaml (+ iOSDatePickerPage.xaml.cs)

#### 🟢 Sonnet 5 Review

Date display and toggle link match MAUI exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 95. Ios Entry — 🟢/⏳
<sub>ios_entry</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_entry_light.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_entry_light.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_entry_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_entry_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_entry_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_entry_dark.png" /></td></tr></table>

ports iOSEntryPage.xaml (+ iOSEntryPage.xaml.cs)

#### 🟢 Sonnet 5 Review

Entry placeholder text and toggle link match MAUI exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 96. Ios First Responder — 🟢/⏳
<sub>ios_first_responder</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_first_responder_light.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_first_responder_light.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_first_responder_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_first_responder_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_first_responder_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_first_responder_dark.png" /></td></tr></table>

ports iOSFirstResponderPage.xaml (+ .xaml.cs) The C# iOSFirstResponderPage is a VisualElement-first-responder demo: a StackLayout with an explanatory Label, a &amp;quot;First Entry&amp;quot; + plain &amp;quot;OK&amp;quot; Button (tapping OK dismisses the keyboard because the

#### 🟢 Sonnet 5 Review

Entry fields, OK links, focus buttons, and state text all match MAUI in both themes; only trivial status-bar clock differences.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 97. Ios Pan Gesture — 🟢/⏳
<sub>ios_pan_gesture</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_pan_gesture_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/ios_pan_gesture_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/ios_pan_gesture_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_pan_gesture_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/ios_pan_gesture_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/ios_pan_gesture_dark.gif" /></td></tr></table>

ports iOSPanGestureRecognizerPage.xaml (+ .xaml.cs) The C# iOSPanGestureRecognizerPage is a StackLayout with: a bold message Label (_messageLabel), a &amp;quot;Toggle Simultaneous Gesture Recognition&amp;quot; Button, and a grouped ListView of employees whos

#### 🟢 Sonnet 5 Review

Panned label, toggle link, target label, and recognition state text match identically in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 98. Ios Picker — 🟢/⏳
<sub>ios_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_picker_light.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_picker_light.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_picker_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_picker_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_picker_dark.png" /></td></tr></table>

ports iOSPickerPage.xaml (+ iOSPickerPage.xaml.cs)

#### 🟢 Sonnet 5 Review

Placeholder entry and toggle link render identically to MAUI in both light and dark themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 99. Ios Safe Area — 🟢/⏳
<sub>ios_safe_area</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_safe_area_light.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_safe_area_light.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_safe_area_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_safe_area_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_safe_area_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_safe_area_dark.png" /></td></tr></table>

ports iOSSafeAreaPage.xaml The .NET MAUI PlatformSpecifics sample (Pages/PlatformSpecifics/iOS/iOSSafeAreaPage.xaml + .xaml.cs): a long Lorem-ipsum Label over a &amp;quot;Disable Use Safe Area&amp;quot; button

#### 🟢 Sonnet 5 Review

Lorem ipsum paragraph and toggle link match MAUI exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 100. Ios Scroll View — 🟡/⏳
<sub>ios_scroll_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_scroll_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_scroll_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_scroll_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_scroll_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_scroll_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_scroll_view_dark.png" /></td></tr></table>

ports iOSScrollViewPage.xaml (+ iOSScrollViewPage.xaml.cs)

#### 🟡 Sonnet 5 Review

Cpp renders an extra circular back-navigation chevron button near the top-left in both themes that MAUI does not show; otherwise slider and links match.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 101. Ios Search Bar — 🟢/⏳
<sub>ios_search_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_search_bar_light.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_search_bar_light.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_search_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_search_bar_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_search_bar_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_search_bar_dark.png" /></td></tr></table>

ports iOSSearchBarPage.xaml (+ iOSSearchBarPage.xaml.cs)

#### 🟢 Sonnet 5 Review

Search bar with magnifier icon and placeholder, plus both toggle links, match MAUI in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 102. Ios Slider Update On Tap — 🟢/⏳
<sub>ios_slider_update_on_tap</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_slider_update_on_tap_light.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_slider_update_on_tap_light.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_slider_update_on_tap_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_slider_update_on_tap_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_slider_update_on_tap_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_slider_update_on_tap_dark.png" /></td></tr></table>

ports iOSSliderUpdateOnTapPage.xaml (+ iOSSliderUpdateOnTapPage.xaml.cs)

#### 🟢 Sonnet 5 Review

Slider thumb position, instructional text, and toggle link match MAUI in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 103. Ios Swipe Transition — 🟢/⏳
<sub>ios_swipe_transition</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_swipe_transition_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/ios_swipe_transition_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/ios_swipe_transition_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_swipe_transition_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/ios_swipe_transition_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/ios_swipe_transition_dark.gif" /></td></tr></table>

ports iOSSwipeViewTransitionModePage.xaml (+ .xaml.cs) The C# iOSSwipeViewTransitionModePage is a StackLayout with: a horizontal row holding a &amp;quot;SwipeTransitionMode:&amp;quot; Label + an EnumPicker over the SwipeTransitionMode enum (Reveal / Drag, Se

#### 🟢 Sonnet 5 Review

SwipeTransitionMode labels/links, swipe-right gray box, and status text all match MAUI in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 104. Ios Time Picker — 🟢/⏳
<sub>ios_time_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_time_picker_light.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_time_picker_light.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_time_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_time_picker_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_time_picker_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_time_picker_dark.png" /></td></tr></table>

ports iOSTimePickerPage.xaml The .NET MAUI PlatformSpecifics sample (Pages/PlatformSpecifics/iOS/iOSTimePickerPage.xaml + .xaml.cs): a TimePicker carrying the iOSSpecific TimePicker.UpdateMode knob (XAML seeds it to WhenFinished) over a but

#### 🟢 Sonnet 5 Review

Time value, label, and layout match MAUI in both light and dark themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 105. Items — 🟢/⏳
<sub>items</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/items_light.png" /></td><td><img width="300px" src="captures/ios/cpp/items_light.png" /></td><td><img width="300px" src="captures/ios/xaml/items_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/items_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/items_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/items_dark.png" /></td></tr></table>

a self-contained demo page for the W2-19 items core: a collection_view over a live observable items source with a templated cell, single selection driving a readout label, and an EmptyView for the cleared state (the C# CollectionView galler

#### 🟢 Sonnet 5 Review

Task list content and layout are identical to MAUI in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 106. Items Updating Scroll Mode — 🟢/⏳
<sub>items_updating_scroll_mode</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/items_updating_scroll_mode_light.png" /></td><td><img width="300px" src="captures/ios/cpp/items_updating_scroll_mode_light.png" /></td><td><img width="300px" src="captures/ios/xaml/items_updating_scroll_mode_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/items_updating_scroll_mode_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/items_updating_scroll_mode_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/items_updating_scroll_mode_dark.png" /></td></tr></table>

ports ItemsUpdatingScrollModeGallery.xaml (+ .xaml.cs) of the C# CollectionView gallery (Maui.Controls.Sample.Pages.CollectionViewGalleries.ScrollModeGalleries.ItemsUpdatingScrollModeGallery)

#### 🟢 Sonnet 5 Review

Full 50-item list with mode/toggle controls renders identically to MAUI in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 107. Label — 🟢/⏳
<sub>label</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/label_light.png" /></td><td><img width="300px" src="captures/ios/cpp/label_light.png" /></td><td><img width="300px" src="captures/ios/xaml/label_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/label_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/label_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/label_dark.png" /></td></tr></table>

ports LabelPage.xaml (+ LabelPage.xaml.cs)

#### 🟢 Sonnet 5 Review

All label styling variants (color, background, alignment, formatted spans, big font) match MAUI pixel-for-pixel in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 108. Layout Is Enabled — 🟢/⏳
<sub>layout_is_enabled</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/layout_is_enabled_light.png" /></td><td><img width="300px" src="captures/ios/cpp/layout_is_enabled_light.png" /></td><td><img width="300px" src="captures/ios/xaml/layout_is_enabled_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/layout_is_enabled_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/layout_is_enabled_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/layout_is_enabled_dark.png" /></td></tr></table>

ports LayoutIsEnabledPage.xaml (+ LayoutIsEnabledPage.xaml.cs) The C# page demonstrates how IsEnabled on a layout cascades to its children: a 2x2 grid whose left column hosts a &amp;quot;MainLayout&amp;quot; full of state-demo sub-stacks (all-enabled / all-d

#### 🟢 Sonnet 5 Review

All enabled/disabled layout states across both columns match MAUI exactly in light and dark themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 109. Line Gallery — 🟢/⏳
<sub>line_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/line_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/line_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/line_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/line_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/line_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/line_gallery_dark.png" /></td></tr></table>

ports LineGallery.xaml A self-contained, code-first port of the MAUI Shapes LineGallery (Pages/Controls/ShapesGalleries/LineGallery.xaml + .xaml.cs)

#### 🟢 Sonnet 5 Review

Basic, dash, and stroke-thickness lines match MAUI; the black stroke-thickness line is invisible against black background in dark theme for both MAUI and cpp, so this is a shared quirk not a port bug.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 110. Line Join Gallery — 🟢/⏳
<sub>line_join_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/line_join_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/line_join_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/line_join_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/line_join_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/line_join_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/line_join_gallery_dark.png" /></td></tr></table>

ports LineJoinGallery.xaml A self-contained, code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/LineJoinGallery.xaml: a StackLayout that demonstrates the three StrokeLineJoin variants on an identical open polyline

#### 🟢 Sonnet 5 Review

Miter, bevel, and round line-join shapes render identically to MAUI in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 111. Measure First Strategy — 🟢/⏳
<sub>measure_first_strategy</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/measure_first_strategy_light.png" /></td><td><img width="300px" src="captures/ios/cpp/measure_first_strategy_light.png" /></td><td><img width="300px" src="captures/ios/xaml/measure_first_strategy_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/measure_first_strategy_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/measure_first_strategy_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/measure_first_strategy_dark.png" /></td></tr></table>

ports MeasureFirstStrategy.xaml (+ .xaml.cs) of the C# CollectionView gallery (Maui.Controls.Sample.Pages.CollectionViewGalleries.GroupingGalleries.MeasureFirstStrategy)

#### 🟢 Sonnet 5 Review

CollectionView grouped list with MeasureFirstItem strategy matches MAUI content and layout in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 112. Menu Bar — 🟢/⏳
<sub>menu_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/menu_bar_light.png" /></td><td><img width="300px" src="captures/ios/cpp/menu_bar_light.png" /></td><td><img width="300px" src="captures/ios/xaml/menu_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/menu_bar_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/menu_bar_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/menu_bar_dark.png" /></td></tr></table>

ports MenuBarPage.xaml (+ MenuBarPage.cs) The C# page declares three page-level MenuBarItems (Page.MenuBarItems — the app menu bar) and a small visible body: - &amp;quot;Before File&amp;quot; : &amp;quot;Before File Action&amp;quot; (accelerator &amp;quot;b&amp;quot;), &amp;quot;Cool item 1&amp;quot;, a separat

#### 🟢 Sonnet 5 Review

Text, colors, and layout match MAUI exactly in both light and dark themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 113. Modal — 🟢/⏳
<sub>modal</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/modal_light.png" /></td><td><img width="300px" src="captures/ios/cpp/modal_light.png" /></td><td><img width="300px" src="captures/ios/xaml/modal_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/modal_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/modal_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/modal_dark.png" /></td></tr></table>

ports ModalPage.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 Review

Modal page content, links, and depth counters match MAUI exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 114. Multiple Bound Selection — 🟢/⏳
<sub>multiple_bound_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/multiple_bound_selection_light.png" /></td><td><img width="300px" src="captures/ios/cpp/multiple_bound_selection_light.png" /></td><td><img width="300px" src="captures/ios/xaml/multiple_bound_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/multiple_bound_selection_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/multiple_bound_selection_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/multiple_bound_selection_dark.png" /></td></tr></table>

ports MultipleBoundSelection.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.MultipleBoundSelection)

#### 🟢 Sonnet 5 Review

CollectionView selection highlighting, header, and items match MAUI exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 115. Navigation Gallery — 🟢/⏳
<sub>navigation_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/navigation_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/navigation_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/navigation_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/navigation_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/navigation_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/navigation_gallery_dark.png" /></td></tr></table>

ports NavigationGallery.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 Review

Navigation gallery text and links match MAUI exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 116. Nested Collection — 🟡/⏳
<sub>nested_collection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/nested_collection_light.png" /></td><td><img width="300px" src="captures/ios/cpp/nested_collection_light.png" /></td><td><img width="300px" src="captures/ios/xaml/nested_collection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/nested_collection_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/nested_collection_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/nested_collection_dark.png" /></td></tr></table>

ports NestedGalleries/NestedCollectionViewGallery.xaml (+ NestedCollectionViewGallery.xaml.cs) of the C# CollectionView gallery

#### 🟡 Sonnet 5 Review

The 'Source N' header label wraps to two lines in the cpp port (narrower column) instead of MAUI's single-line label, pushing the nested caption row right and truncating it sooner; content is still visible and correct, just laid out with a narrower left column in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 117. Pan Gesture Events — 🟢/⏳
<sub>pan_gesture_events</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/pan_gesture_events_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/pan_gesture_events_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/pan_gesture_events_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/pan_gesture_events_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/pan_gesture_events_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/pan_gesture_events_dark.gif" /></td></tr></table>

ports PanGestureEventsGallery.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.PanGestureEventsGallery)

#### 🟢 Sonnet 5 Review

Green/red status blocks and status text match MAUI exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 118. Path Aspect Gallery — 🟢/⏳
<sub>path_aspect_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/path_aspect_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/path_aspect_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/path_aspect_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/path_aspect_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/path_aspect_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/path_aspect_gallery_dark.png" /></td></tr></table>

ports PathAspectGallery.xaml A self-contained, code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PathAspectGallery.xaml: a StackLayout (Padding 12) that demonstrates the four Path Aspect modes on one identical ge

#### 🟢 Sonnet 5 Review

All four heart-aspect renders (None/Fill/Uniform/UniformToFill) match MAUI pixel-for-pixel in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 119. Path Gallery — 🟢/⏳
<sub>path_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/path_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/path_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/path_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/path_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/path_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/path_gallery_dark.png" /></td></tr></table>

ports PathGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PathGallery.xaml: a ScrollView over a StackLayout (Padding 12) that walks eight Path variants (plus two caption-only markup-string Labels

#### 🟢 Sonnet 5 Review

All path/geometry shapes match MAUI in both themes, including the same invisible black-stroke shapes in dark mode.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 120. Path Transform String — 🟢/⏳
<sub>path_transform_string</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/path_transform_string_light.png" /></td><td><img width="300px" src="captures/ios/cpp/path_transform_string_light.png" /></td><td><img width="300px" src="captures/ios/xaml/path_transform_string_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/path_transform_string_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/path_transform_string_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/path_transform_string_dark.png" /></td></tr></table>

ports PathTransformStringGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PathTransformStringGallery.xaml: a ScrollView over a StackLayout (Padding 12) that shows the SAME two-figure Path geometry

#### 🟢 Sonnet 5 Review

Both light-theme renders match pixel-for-pixel (triangles with/without RenderTransform identical); dark theme is entirely blank in both maui and cpp, so no regression.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 121. Picker — 🟢/⏳
<sub>picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/picker_light.png" /></td><td><img width="300px" src="captures/ios/cpp/picker_light.png" /></td><td><img width="300px" src="captures/ios/xaml/picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/picker_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/picker_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/picker_dark.png" /></td></tr></table>

ports PickerPage.xaml (+ PickerPage.xaml.cs) A self-contained, code-first demo page for the Picker control (the C# gallery-page convention, mirroring the value_controls_page / pickers_page pattern)

#### 🟢 Sonnet 5 Review

All picker variants match pixel-for-pixel in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 122. Pickers — 🟢/⏳
<sub>pickers</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/pickers_light.png" /></td><td><img width="300px" src="captures/ios/cpp/pickers_light.png" /></td><td><img width="300px" src="captures/ios/xaml/pickers_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/pickers_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/pickers_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/pickers_dark.png" /></td></tr></table>

a self-contained demo page for the W1-06 picker set: picker, date_picker and time_picker on one vertical stack, wired together so every selection drives a visible output (the C# gallery-page convention, code-first; the value_controls_page p

#### 🟢 Sonnet 5 Review

Room/date/time picker layout and text match in both themes; date values differ only due to capture-day date (non-bug).

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 123. Pointer Gesture — 🟢/⏳
<sub>pointer_gesture</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/pointer_gesture_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/pointer_gesture_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/pointer_gesture_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/pointer_gesture_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/pointer_gesture_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/pointer_gesture_dark.gif" /></td></tr></table>

ports PointerGestureGalleryPage.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.PointerGestureGalleryPage)

#### 🟢 Sonnet 5 Review

Pointer position labels, hover/press states, and colors match pixel-for-pixel in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 124. Polygon Gallery — 🔴/⏳
<sub>polygon_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/polygon_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/polygon_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/polygon_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/polygon_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/polygon_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/polygon_gallery_dark.png" /></td></tr></table>

ports PolygonGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PolygonGallery.xaml: a ScrollView over a StackLayout (Padding 12) that walks four Polygon variants, each under a caption Label — - &amp;quot;A

#### 🔴 Sonnet 5 Review

The 'NonZero Polygon' star is filled solid black in the MAUI reference but renders as an unfilled outline-only shape in the cpp port, in both light and dark themes; all other polygons (basic, dash, EvenOdd) match correctly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 125. Polyline Gallery — 🟢/⏳
<sub>polyline_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/polyline_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/polyline_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/polyline_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/polyline_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/polyline_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/polyline_gallery_dark.png" /></td></tr></table>

ports PolylineGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PolylineGallery.xaml: a StackLayout (Padding 12 — no ScrollView in the C# source) holding two Polyline variants, each under a caption

#### 🟢 Sonnet 5 Review

Both maui and cpp show the polyline content clipped identically at the left edge in both themes; shared harness characteristic, not a port-specific regression.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 126. Preselected Item — 🟢/⏳
<sub>preselected_item</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/preselected_item_light.png" /></td><td><img width="300px" src="captures/ios/cpp/preselected_item_light.png" /></td><td><img width="300px" src="captures/ios/xaml/preselected_item_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/preselected_item_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/preselected_item_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/preselected_item_dark.png" /></td></tr></table>

ports PreselectedItemGallery.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.PreselectedItemGallery)

#### 🟢 Sonnet 5 Review

CollectionView with preselected item renders identically in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 127. Preselected Items — 🟢/⏳
<sub>preselected_items</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/preselected_items_light.png" /></td><td><img width="300px" src="captures/ios/cpp/preselected_items_light.png" /></td><td><img width="300px" src="captures/ios/xaml/preselected_items_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/preselected_items_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/preselected_items_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/preselected_items_dark.png" /></td></tr></table>

ports PreselectedItemsGallery.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.PreselectedItemsGallery)

#### 🟢 Sonnet 5 Review

Grid CollectionView with multiple preselected items matches pixel-for-pixel in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 128. Progress Bar — 🟢/⏳
<sub>progress_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/progress_bar_light.png" /></td><td><img width="300px" src="captures/ios/cpp/progress_bar_light.png" /></td><td><img width="300px" src="captures/ios/xaml/progress_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/progress_bar_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/progress_bar_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/progress_bar_dark.png" /></td></tr></table>

ports ProgressBarPage.xaml (+ ProgressBarPage.xaml.cs)

#### 🟢 Sonnet 5 Review

Progress bars, colors, and states match MAUI in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 129. Radio Button Border — 🟢/⏳
<sub>radio_button_border</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/radio_button_border_light.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_button_border_light.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_button_border_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/radio_button_border_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_button_border_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_button_border_dark.png" /></td></tr></table>

ports RadioButtonBorder.xaml A self-contained, code-first demo of RadioButton border styling

#### 🟢 Sonnet 5 Review

Bordered radio rows with yellow/green highlights match MAUI exactly in light and dark.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 130. Radio Button Content — 🟢/⏳
<sub>radio_button_content</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/radio_button_content_light.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_button_content_light.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_button_content_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/radio_button_content_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_button_content_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_button_content_dark.png" /></td></tr></table>

ports RadioButtonContentGallery.xaml A self-contained, code-first demo of the RadioButton.Content surface

#### 🟢 Sonnet 5 Review

All content variations (string, view-fallback, image, custom template) match MAUI in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 131. Radio Button Group — 🟢/⏳
<sub>radio_button_group</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/radio_button_group_light.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_button_group_light.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_button_group_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/radio_button_group_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_button_group_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_button_group_dark.png" /></td></tr></table>

ports RadioButtonGroupGallery.xaml A self-contained, code-first demo of the RadioButtonGroup ATTACHED-PROPERTY grouping: a vertical StackLayout carries RadioButtonGroup.GroupName=&amp;quot;foo&amp;quot;, so every descendant RadioButton — including one nested

#### 🟢 Sonnet 5 Review

StackLayout and Grid-based radio groups match MAUI layout and styling in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 132. Radio Button Group Binding — 🟢/⏳
<sub>radio_button_group_binding</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/radio_button_group_binding_light.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_button_group_binding_light.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_button_group_binding_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/radio_button_group_binding_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_button_group_binding_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_button_group_binding_dark.png" /></td></tr></table>

ports RadioButtonGroupBindingGallery.xaml A code-first demo of binding the RadioButtonGroup attached properties (GroupName + SelectedValue) to a view-model

#### 🟢 Sonnet 5 Review

Bound group/selection radios match MAUI layout, text, and links in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 133. Radio Button Group Gallery — 🟢/⏳
<sub>radio_button_group_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/radio_button_group_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_button_group_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_button_group_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/radio_button_group_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_button_group_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_button_group_gallery_dark.png" /></td></tr></table>

ports RadioButtonGroupGalleryPage.xaml A self-contained, code-first demo of RadioButton grouping SCOPE, mirroring the C# controls gallery page (Pages/Controls/RadioButtonGalleries/RadioButtonGroupGalleryPage.xaml)

#### 🟢 Sonnet 5 Review

All three group-name test sections render identically to MAUI in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 134. Radio Content Properties — 🟢/⏳
<sub>radio_content_properties</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/radio_content_properties_light.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_content_properties_light.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_content_properties_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/radio_content_properties_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_content_properties_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_content_properties_dark.png" /></td></tr></table>

ports ContentProperties.xaml A self-contained, code-first demo of how RadioButton propagates the standard Text/Font properties to its Content

#### 🟢 Sonnet 5 Review

Text styling propagation (color, font, transform) to Content matches MAUI in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 135. Radio Template From Style — ⬛/⏳
<sub>radio_template_from_style</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/radio_template_from_style_light.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_template_from_style_light.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_template_from_style_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/radio_template_from_style_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_template_from_style_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_template_from_style_dark.png" /></td></tr></table>

ports TemplateFromStyle.xaml A self-contained, code-first demo of applying a RadioButton ControlTemplate

#### ⬛ Sonnet 5 Review

MAUI reference screenshots (light/dark) show the iOS home screen instead of the app content — reference capture is broken/unusable for comparison.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 136. Rectangle Gallery — 🟢/⏳
<sub>rectangle_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/rectangle_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/rectangle_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/rectangle_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/rectangle_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/rectangle_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/rectangle_gallery_dark.png" /></td></tr></table>

ports RectangleGallery.xaml A self-contained, code-first port of the MAUI Shapes RectangleGallery (Pages/Controls/ShapesGalleries/RectangleGallery.xaml + .xaml.cs)

#### 🟢 Sonnet 5 Review

Both themes render identically to MAUI: rectangle, square outline, stroke rectangle, dashed stroke, and rounded rectangle shapes all match in color, size, and position.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 137. Refresh View — 🟢/⏳
<sub>refresh_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/refresh_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/refresh_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/refresh_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/refresh_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/refresh_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/refresh_view_dark.png" /></td></tr></table>

ports RefreshViewPage.xaml (+ RefreshViewPage.xaml.cs + RefreshViewModel.cs) A self-contained, code-first demo page for the RefreshView control (the C# gallery-page convention, mirroring the swipe_refresh_page pattern)

#### 🟢 Sonnet 5 Review

Text, links, and layout match MAUI exactly in both light and dark themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 138. Relative Layout — 🟢/⏳
<sub>relative_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/relative_layout_light.png" /></td><td><img width="300px" src="captures/ios/cpp/relative_layout_light.png" /></td><td><img width="300px" src="captures/ios/xaml/relative_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/relative_layout_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/relative_layout_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/relative_layout_dark.png" /></td></tr></table>

ports RelativeLayoutPage.xaml

#### 🟢 Sonnet 5 Review

Corner-anchored colored boxes and centered gray/black rectangles match MAUI's positions and colors in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 139. Scattered Radio Button — 🟢/⏳
<sub>scattered_radio_button</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/scattered_radio_button_light.png" /></td><td><img width="300px" src="captures/ios/cpp/scattered_radio_button_light.png" /></td><td><img width="300px" src="captures/ios/xaml/scattered_radio_button_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/scattered_radio_button_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/scattered_radio_button_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/scattered_radio_button_dark.png" /></td></tr></table>

ports ScatteredRadioButtonGallery.xaml A code-first demo that radio buttons DON&amp;#x27;T have to share a container to be grouped: grouping is by GroupName, so buttons scattered across separate containers (and one bare button outside any grouped co

#### 🟢 Sonnet 5 Review

Radio buttons, labels, and highlighted group background match MAUI in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 140. Scroll Mode Test — 🟡/⏳
<sub>scroll_mode_test</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/scroll_mode_test_light.png" /></td><td><img width="300px" src="captures/ios/cpp/scroll_mode_test_light.png" /></td><td><img width="300px" src="captures/ios/xaml/scroll_mode_test_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/scroll_mode_test_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/scroll_mode_test_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/scroll_mode_test_dark.png" /></td></tr></table>

ports ScrollModeTestGallery.xaml (+ .xaml.cs) of the C# CollectionView gallery (Maui.Controls.Sample.Pages.CollectionViewGalleries.ScrollModeGalleries.ScrollModeTestGallery)

#### 🟡 Sonnet 5 Review

MAUI renders the ItemsUpdatingScrollMode selector as segmented control buttons, while the C++ port renders it as a bordered text-entry-style box; functionally equivalent but visually a different control style in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 141. Scroll To Group — 🟢/⏳
<sub>scroll_to_group</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/scroll_to_group_light.png" /></td><td><img width="300px" src="captures/ios/cpp/scroll_to_group_light.png" /></td><td><img width="300px" src="captures/ios/xaml/scroll_to_group_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/scroll_to_group_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/scroll_to_group_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/scroll_to_group_dark.png" /></td></tr></table>

ports ScrollToGalleries/ScrollToGroup.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 Review

Group/item text fields, Go links, and the full scrollable superhero list match MAUI closely in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 142. Scroll View — 🟢/⏳
<sub>scroll_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/scroll_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/scroll_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/scroll_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/scroll_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/scroll_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/scroll_view_dark.png" /></td></tr></table>

ports ScrollViewPage.xaml (+ the ScrollViewPages sub-demos: ScrollViewOrientationPage / ScrollToEndPage / ScrollToFromConstructorPage), code-first

#### 🟢 Sonnet 5 Review

Slider and links match MAUI; the C++ port additionally shows a back/nav chevron button not present in the MAUI capture, a minor harness-navigation difference that doesn't affect page content.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 143. Search Bar — 🟡/⏳
<sub>search_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/search_bar_light.png" /></td><td><img width="300px" src="captures/ios/cpp/search_bar_light.png" /></td><td><img width="300px" src="captures/ios/xaml/search_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/search_bar_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/search_bar_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/search_bar_dark.png" /></td></tr></table>

ports SearchBarPage.xaml (Microsoft.Maui.Controls sample gallery)

#### 🟡 Sonnet 5 Review

Light theme matches MAUI exactly, but in dark theme the 'Cancel is red' search bar's clear/cancel X icon renders white in the C++ port instead of red as in MAUI.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 144. Selection Command Param — 🟢/⏳
<sub>selection_command_param</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/selection_command_param_light.png" /></td><td><img width="300px" src="captures/ios/cpp/selection_command_param_light.png" /></td><td><img width="300px" src="captures/ios/xaml/selection_command_param_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/selection_command_param_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/selection_command_param_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/selection_command_param_dark.png" /></td></tr></table>

ports SelectionChangedCommandParameter.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.SelectionChangedCommandParameter)

#### 🟢 Sonnet 5 Review

Identical layout, text, and rendering in both light and dark themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 145. Selection Synchronization — 🟢/⏳
<sub>selection_synchronization</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/selection_synchronization_light.png" /></td><td><img width="300px" src="captures/ios/cpp/selection_synchronization_light.png" /></td><td><img width="300px" src="captures/ios/xaml/selection_synchronization_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/selection_synchronization_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/selection_synchronization_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/selection_synchronization_dark.png" /></td></tr></table>

ports SelectionSynchronization.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.SelectionSynchronization)

#### 🟢 Sonnet 5 Review

Matches; cpp selection rows render full-width vs MAUI's slightly narrower highlight bands, a trivial cosmetic difference.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 146. Semantics — 🟢/⏳
<sub>semantics</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/semantics_light.png" /></td><td><img width="300px" src="captures/ios/cpp/semantics_light.png" /></td><td><img width="300px" src="captures/ios/xaml/semantics_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/semantics_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/semantics_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/semantics_dark.png" /></td></tr></table>

ports SemanticsPage.xaml (+ SemanticsPage.xaml.cs) The C# SemanticsPage is an accessibility showcase: a long VerticalStackLayout where nearly every control carries SemanticProperties.Description / .Hint, plus a block of labels exercising Se

#### 🟢 Sonnet 5 Review

Pixel-accurate match across all semantic property showcase elements in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 147. Shadow Playground — 🟢/⏳
<sub>shadow_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/shadow_playground_light.png" /></td><td><img width="300px" src="captures/ios/cpp/shadow_playground_light.png" /></td><td><img width="300px" src="captures/ios/xaml/shadow_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/shadow_playground_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/shadow_playground_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/shadow_playground_dark.png" /></td></tr></table>

ports ShadowPlaygroundPage.xaml A self-contained, code-first demo of the view Shadow surface, mirroring the C# core gallery page (Pages/Core/ShadowGalleries/ShadowPlaygroundPage.xaml + .xaml.cs)

#### 🟢 Sonnet 5 Review

Shadow rendering, slider positions, and colors match exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 148. Shape App Theme — 🟢/⏳
<sub>shape_app_theme</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/shape_app_theme_light.png" /></td><td><img width="300px" src="captures/ios/cpp/shape_app_theme_light.png" /></td><td><img width="300px" src="captures/ios/xaml/shape_app_theme_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/shape_app_theme_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/shape_app_theme_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/shape_app_theme_dark.png" /></td></tr></table>

ports ShapeAppThemeGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/ShapeAppThemeGallery.xaml: a StackLayout (Padding 12) holding a caption Label and a 200x80 Rectangle, all themed via {AppThemeBi

#### 🟢 Sonnet 5 Review

Theme-driven shape color (green light / red dark) matches exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 149. Shapes — 🟢/⏳
<sub>shapes</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/shapes_light.png" /></td><td><img width="300px" src="captures/ios/cpp/shapes_light.png" /></td><td><img width="300px" src="captures/ios/xaml/shapes_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/shapes_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/shapes_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/shapes_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;shapes&amp;quot; demo (ComparePages.Shapes()), the shipped-.NET-MAUI reference for the visual-parity comparison: a ScrollView over a vertical stack of four LABELLED shapes, each bold-captioned and Start-a

#### 🟢 Sonnet 5 Review

Ellipse, RoundRectangle, EvenOdd polygon, and Line all render identically to MAUI in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 150. Single Bound Selection — 🟢/⏳
<sub>single_bound_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/single_bound_selection_light.png" /></td><td><img width="300px" src="captures/ios/cpp/single_bound_selection_light.png" /></td><td><img width="300px" src="captures/ios/xaml/single_bound_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/single_bound_selection_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/single_bound_selection_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/single_bound_selection_dark.png" /></td></tr></table>

ports SingleBoundSelection.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.SingleBoundSelection)

#### 🟢 Sonnet 5 Review

Text and layout match exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 151. Slider — 🟢/⏳
<sub>slider</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/slider_light.png" /></td><td><img width="300px" src="captures/ios/cpp/slider_light.png" /></td><td><img width="300px" src="captures/ios/xaml/slider_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/slider_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/slider_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/slider_dark.png" /></td></tr></table>

ports SliderPage.xaml (+ .xaml.cs) Mirrors the MAUI gallery page: a vertical stack of headlined Slider states — Default, BackgroundColor (Blue), Background (yellow→green LinearGradientBrush), Minimum(5)/Maximum(15) with a value readout (Val

#### 🟢 Sonnet 5 Review

All slider variants (colors, disabled state, custom thumb/track colors) match exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 152. Some Empty Groups — 🟢/⏳
<sub>some_empty_groups</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/some_empty_groups_light.png" /></td><td><img width="300px" src="captures/ios/cpp/some_empty_groups_light.png" /></td><td><img width="300px" src="captures/ios/xaml/some_empty_groups_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/some_empty_groups_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/some_empty_groups_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/some_empty_groups_dark.png" /></td></tr></table>

ports GroupingGalleries/SomeEmptyGroups.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 Review

Grouped CollectionView with empty groups, headers/footers render identically in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 153. Stack Layout — 🟢/⏳
<sub>stack_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/stack_layout_light.png" /></td><td><img width="300px" src="captures/ios/cpp/stack_layout_light.png" /></td><td><img width="300px" src="captures/ios/xaml/stack_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/stack_layout_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/stack_layout_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/stack_layout_dark.png" /></td></tr></table>

ports StackLayoutPage.xaml Demonstrates the generic maui::controls::stack_layout (the orientation-switching sibling of the fixed vertical/horizontal stacks) by nesting two inner stacks inside an outer vertical stack with a 12px margin: a &amp;quot;V

#### 🟢 Sonnet 5 Review

Vertical and horizontal color-swatch stacks match exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 154. Staggered Layout — 🟢/⏳
<sub>staggered_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/staggered_layout_light.png" /></td><td><img width="300px" src="captures/ios/cpp/staggered_layout_light.png" /></td><td><img width="300px" src="captures/ios/xaml/staggered_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/staggered_layout_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/staggered_layout_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/staggered_layout_dark.png" /></td></tr></table>

ports AlternateLayoutGalleries/StaggeredLayout.xaml (+ StaggeredLayout.xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 Review

Item grid content and layout match; minor status-bar overlap timing artifact in cpp dark shot is a capture quirk, not a layout bug.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 155. Stepper — 🟢/⏳
<sub>stepper</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/stepper_light.png" /></td><td><img width="300px" src="captures/ios/cpp/stepper_light.png" /></td><td><img width="300px" src="captures/ios/xaml/stepper_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/stepper_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/stepper_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/stepper_dark.png" /></td></tr></table>

ports StepperPage.xaml (+ StepperPage.xaml.cs)

#### 🟢 Sonnet 5 Review

All stepper variants (default, disabled, colored background, min/max, increment) match MAUI in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 156. Styles — 🟢/⏳
<sub>styles</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/styles_light.png" /></td><td><img width="300px" src="captures/ios/cpp/styles_light.png" /></td><td><img width="300px" src="captures/ios/xaml/styles_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/styles_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/styles_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/styles_dark.png" /></td></tr></table>

ports StylesPage.xaml

#### 🟢 Sonnet 5 Review

Style-derivation examples match exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 157. Swipe Gesture — 🔴/⏳
<sub>swipe_gesture</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/swipe_gesture_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/swipe_gesture_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/swipe_gesture_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/swipe_gesture_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/swipe_gesture_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/swipe_gesture_dark.gif" /></td></tr></table>

ports SwipeViewGestureRecognizerGallery.xaml (+ .xaml.cs) The MAUI SwipeViewGestureRecognizerGallery is a CollectionView of &amp;quot;message&amp;quot; rows; each row&amp;#x27;s DataTemplate is a SwipeView wired three ways, proving gesture recognizers AND swipe-item

#### 🔴 Sonnet 5 Review

In dark mode the cpp port's swipe card is missing all its content (title, subtitle, description text) leaving a blank white bar, while MAUI dark shows the full card text; light mode matches.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 158. Swipe Item Position — 🟢/⏳
<sub>swipe_item_position</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/swipe_item_position_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/swipe_item_position_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/swipe_item_position_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/swipe_item_position_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/swipe_item_position_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/swipe_item_position_dark.gif" /></td></tr></table>

ports SwipeItemPositionGallery.xaml A code-first port of the MAUI SwipeView sub-gallery Pages/Controls/SwipeViewGalleries/SwipeItemPositionGallery.xaml: a 2-row Grid (Auto / *) with a Picker on top and one SwipeView below that carries TWO S

#### 🟢 Sonnet 5 Review

Both MAUI and cpp show the same odd full-screen gray fill in dark mode (a shared MAUI-side capture quirk); light mode matches cleanly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 159. Swipe Item Size — 🟢/⏳
<sub>swipe_item_size</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/swipe_item_size_light.png" /></td><td><img width="300px" src="captures/ios/cpp/swipe_item_size_light.png" /></td><td><img width="300px" src="captures/ios/xaml/swipe_item_size_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/swipe_item_size_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/swipe_item_size_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/swipe_item_size_dark.png" /></td></tr></table>

ports SwipeItemSizeGallery.xaml A self-contained, code-first port of the .NET MAUI &amp;quot;SwipeItem Size Gallery&amp;quot;: a scrolling stack of swipe_views demonstrating how a left SwipeItem&amp;#x27;s icon size and the SwipeView content size interact

#### 🟢 Sonnet 5 Review

All icon-size and SwipeView-size variants match MAUI in both light and dark themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 160. Swipe Refresh — 🟢/⏳
<sub>swipe_refresh</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/swipe_refresh_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/swipe_refresh_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/swipe_refresh_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/swipe_refresh_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/swipe_refresh_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/swipe_refresh_dark.gif" /></td></tr></table>

a self-contained demo page for the W2-20 swipe + refresh controls: a refresh_view wrapping a swipe_view (which itself wraps a labeled row), with a readout label reflecting the latest interaction

#### 🟢 Sonnet 5 Review

Text content and layout match exactly in both light and dark themes; no visible SwipeView/RefreshView content differs from MAUI.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 161. Swipe Threshold — 🟢/⏳
<sub>swipe_threshold</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/swipe_threshold_light.png" /></td><td><img width="300px" src="captures/ios/cpp/swipe_threshold_light.png" /></td><td><img width="300px" src="captures/ios/xaml/swipe_threshold_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/swipe_threshold_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/swipe_threshold_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/swipe_threshold_dark.png" /></td></tr></table>

ports HorizontalSwipeThresholdGallery.xaml (+ .xaml.cs) The MAUI HorizontalSwipeThresholdGallery shows how SwipeView.Threshold (the swipe distance, in DIPs, the user must drag before the items settle open / execute) interacts with SwipeItem

#### 🟢 Sonnet 5 Review

All threshold demo blocks, slider positions, and colors match MAUI pixel-for-pixel in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 162. Swipe View Margin — 🟢/⏳
<sub>swipe_view_margin</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/swipe_view_margin_light.png" /></td><td><img width="300px" src="captures/ios/cpp/swipe_view_margin_light.png" /></td><td><img width="300px" src="captures/ios/xaml/swipe_view_margin_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/swipe_view_margin_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/swipe_view_margin_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/swipe_view_margin_dark.png" /></td></tr></table>

ports SwipeViewMarginGallery.xaml A self-contained, code-first port of the .NET MAUI &amp;quot;SwipeView Margin Gallery&amp;quot;: two swipe_views whose content&amp;#x27;s Margin + Padding are driven by two sliders, demonstrating that the revealed SwipeItems stay cor

#### 🟢 Sonnet 5 Review

Sliders, labels, and margin/padding demo boxes match MAUI exactly in both light and dark themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 163. Swipe View Shadow — 🔴/⏳
<sub>swipe_view_shadow</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/swipe_view_shadow_light.png" /></td><td><img width="300px" src="captures/ios/cpp/swipe_view_shadow_light.png" /></td><td><img width="300px" src="captures/ios/xaml/swipe_view_shadow_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/swipe_view_shadow_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/swipe_view_shadow_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/swipe_view_shadow_dark.png" /></td></tr></table>

ports SwipeViewShadowGallery.xaml A code-first port of the MAUI SwipeView sub-gallery Pages/Controls/SwipeViewGalleries/SwipeViewShadowGallery.xaml: a padded vertical StackLayout proving a drop Shadow renders correctly on SwipeView content

#### 🔴 Sonnet 5 Review

Light theme matches (bordered gray boxes with 'Content' text), but in dark theme the C++ port's bordered content boxes lose their light-gray background/border and become invisible against the black background, while MAUI keeps them visible with the same light-gray fill/border regardless of theme.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 164. Switch — 🟡/⏳
<sub>switch</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/switch_light.png" /></td><td><img width="300px" src="captures/ios/cpp/switch_light.png" /></td><td><img width="300px" src="captures/ios/xaml/switch_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/switch_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/switch_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/switch_dark.png" /></td></tr></table>

ports SwitchPage.xaml (+ .xaml.cs) Mirrors the MAUI gallery page: a vertical stack of headlined Switch states — Default, BackgroundColor (Blue), Background (a yellow→green LinearGradientBrush), Disabled, OnColor (Red), ThumbColor (Orange)

#### 🟡 Sonnet 5 Review

Dark theme matches including the orange ThumbColor swatch, but in light theme the C++ port's ThumbColor switch renders with a plain white thumb/gray track instead of MAUI's orange thumb.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 165. Switch Grouping — 🟢/⏳
<sub>switch_grouping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/switch_grouping_light.png" /></td><td><img width="300px" src="captures/ios/cpp/switch_grouping_light.png" /></td><td><img width="300px" src="captures/ios/xaml/switch_grouping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/switch_grouping_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/switch_grouping_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/switch_grouping_dark.png" /></td></tr></table>

ports CollectionViewGalleries/GroupingGalleries/ SwitchGrouping.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 Review

Grouped list content, colors, and toggle state match MAUI exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 166. Tabbed Flyout — 🟡/⏳
<sub>tabbed_flyout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/tabbed_flyout_light.png" /></td><td><img width="300px" src="captures/ios/cpp/tabbed_flyout_light.png" /></td><td><img width="300px" src="captures/ios/xaml/tabbed_flyout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/tabbed_flyout_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/tabbed_flyout_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/tabbed_flyout_dark.png" /></td></tr></table>

a self-contained demo page for the W1-10 tabbed + flyout vertical: a flyout_page whose FLYOUT pane is a titled menu (two buttons selecting the detail&amp;#x27;s tabs + a &amp;quot;Toggle flyout&amp;quot; presenting/dismissing itself) and whose DETAIL pane is a tabbed

#### 🟡 Sonnet 5 Review

Screenshots were captured at different navigation states so a direct pixel comparison isn't possible, but all visible content renders correctly and consistently with MAUI's UI in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 167. Templated View — 🟢/⏳
<sub>templated_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/templated_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/templated_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/templated_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/templated_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/templated_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/templated_view_dark.png" /></td></tr></table>

ports TemplatedViewPage.xaml The C# page contrasts a standard CardView control with a compact one driven by a ControlTemplate (&amp;quot;CardViewCompressed&amp;quot;) and a custom Rate control built entirely from a ControlTemplate + a heart PathGeometry

#### 🟢 Sonnet 5 Review

CardView and compact ControlTemplate cards render identically to MAUI in both light and dark themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 168. Time Picker — 🟡/⏳
<sub>time_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/time_picker_light.png" /></td><td><img width="300px" src="captures/ios/cpp/time_picker_light.png" /></td><td><img width="300px" src="captures/ios/xaml/time_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/time_picker_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/time_picker_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/time_picker_dark.png" /></td></tr></table>

ports TimePickerPage.xaml (+ TimePickerPage.xaml.cs)

#### 🟡 Sonnet 5 Review

Layout, colors, gradients and backgrounds all match in both themes; only difference is default time-format text: MAUI shows 12-hour AM/PM while cpp shows 24-hour format, a content/locale-format difference.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 169. Title Bar — 🟢/⏳
<sub>title_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/title_bar_light.png" /></td><td><img width="300px" src="captures/ios/cpp/title_bar_light.png" /></td><td><img width="300px" src="captures/ios/xaml/title_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/title_bar_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/title_bar_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/title_bar_dark.png" /></td></tr></table>

ports TitleBarPage.xaml A self-contained, code-first demo of the TitleBar control

#### 🟢 Sonnet 5 Review

Pixel-identical layout, controls, colors, and text in both light and dark themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 170. Toolbar — 🟢/⏳
<sub>toolbar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/toolbar_light.png" /></td><td><img width="300px" src="captures/ios/cpp/toolbar_light.png" /></td><td><img width="300px" src="captures/ios/xaml/toolbar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/toolbar_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/toolbar_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/toolbar_dark.png" /></td></tr></table>

ports ToolbarPage.xaml (Maui.Controls.Sample.Pages.ToolbarPage)

#### 🟢 Sonnet 5 Review

Pixel-identical layout and text in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 171. Transform Playground — 🟢/⏳
<sub>transform_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/transform_playground_light.png" /></td><td><img width="300px" src="captures/ios/cpp/transform_playground_light.png" /></td><td><img width="300px" src="captures/ios/xaml/transform_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/transform_playground_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/transform_playground_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/transform_playground_dark.png" /></td></tr></table>

ports TransformPlaygroundGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/TransformPlaygroundGallery.xaml: a 50x50 Path rectangle (red fill, blue stroke 4) sits in a 200x200 light-grey panel; belo

#### 🟢 Sonnet 5 Review

Identical layout, slider positions, colors, and text in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 172. Transformations — 🟢/⏳
<sub>transformations</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/transformations_light.png" /></td><td><img width="300px" src="captures/ios/cpp/transformations_light.png" /></td><td><img width="300px" src="captures/ios/xaml/transformations_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/transformations_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/transformations_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/transformations_dark.png" /></td></tr></table>

ports TransformationsPage.xaml (+ .xaml.cs) The MAUI TransformationsPage drives a single target view&amp;#x27;s render transforms from a column of knobs: Sliders for Scale / ScaleX / ScaleY (Maximum 10) and Rotation / RotationX / RotationY (Maximum

#### 🟢 Sonnet 5 Review

Identical layout, slider values, and text in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 173. Triggers — 🟢/⏳
<sub>triggers</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/triggers_light.png" /></td><td><img width="300px" src="captures/ios/cpp/triggers_light.png" /></td><td><img width="300px" src="captures/ios/xaml/triggers_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/triggers_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/triggers_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/triggers_dark.png" /></td></tr></table>

ports TriggersPage.xaml

#### 🟢 Sonnet 5 Review

Identical layout and text in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 174. Update Path Data — 🟢/⏳
<sub>update_path_data</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/update_path_data_light.png" /></td><td><img width="300px" src="captures/ios/cpp/update_path_data_light.png" /></td><td><img width="300px" src="captures/ios/xaml/update_path_data_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/update_path_data_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/update_path_data_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/update_path_data_dark.png" /></td></tr></table>

ports UpdatePathDataGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/UpdatePathDataGallery.xaml: a 2-row Grid (RowSpacing 0) that proves a Path repaints when its Data geometry is replaced at runti

#### 🟢 Sonnet 5 Review

Light theme matches exactly. Dark theme captures for both MAUI and cpp show the same blank rendering, so cpp matches the reference consistently.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 175. Value Controls — ⏳/⏳
<sub>value_controls</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

#### ⏳ Sonnet 5 Review

_Not yet reviewed._

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 176. Varied Size Selector — 🟢/⏳
<sub>varied_size_selector</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/varied_size_selector_light.png" /></td><td><img width="300px" src="captures/ios/cpp/varied_size_selector_light.png" /></td><td><img width="300px" src="captures/ios/xaml/varied_size_selector_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/varied_size_selector_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/varied_size_selector_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/varied_size_selector_dark.png" /></td></tr></table>

ports DataTemplateSelectorGalleries/VariedSizeDataTemplateSelectorGallery.xaml (+ VariedSizeDataTemplateSelectorGallery.xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 Review

Identical varied-height list rows, button row, and text fields in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 177. Vertical Stack — 🟢/⏳
<sub>vertical_stack</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/vertical_stack_light.png" /></td><td><img width="300px" src="captures/ios/cpp/vertical_stack_light.png" /></td><td><img width="300px" src="captures/ios/xaml/vertical_stack_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/vertical_stack_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/vertical_stack_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/vertical_stack_dark.png" /></td></tr></table>

Vertical Stack

#### 🟢 Sonnet 5 Review

Both themes render the six stacked color blocks identically in size, order, and colors, with matching header text and background.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 178. Vertical Stack Layout — ⏳/⏳
<sub>vertical_stack_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

#### ⏳ Sonnet 5 Review

_Not yet reviewed._

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 179. Visual States — 🟢/⏳
<sub>visual_states</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/visual_states_light.png" /></td><td><img width="300px" src="captures/ios/cpp/visual_states_light.png" /></td><td><img width="300px" src="captures/ios/xaml/visual_states_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/visual_states_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/visual_states_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/visual_states_dark.png" /></td></tr></table>

ports VisualStatesPage.xaml

#### 🟢 Sonnet 5 Review

All entry/button visual-state elements match in position, color, and text in both light and dark themes; minor line-wrap difference in the paragraph text is just reflow noise.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 180. Web View — 🟢/⏳
<sub>web_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/web_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/web_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/web_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/web_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/web_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/web_view_dark.png" /></td></tr></table>

a self-contained demo page for the W1-08 web_view vertical: a web_view loading a STATIC html_web_view_source (no network), back/forward/reload buttons over the handler-pushed CanGoBack/CanGoForward read-onlys, an &amp;quot;Eval 1+1&amp;quot; button driving t

#### 🟢 Sonnet 5 Review

WebView content (Welcome header, buttons, layout) matches in both themes; the displayed URL text differs (file path vs https) but that's a harness/environment artifact, not a rendering bug.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 181. Z Index — 🟢/⏳
<sub>z_index</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/z_index_light.png" /></td><td><img width="300px" src="captures/ios/cpp/z_index_light.png" /></td><td><img width="300px" src="captures/ios/xaml/z_index_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/z_index_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/z_index_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/z_index_dark.png" /></td></tr></table>

ports ZIndexPage.xaml (+ ZIndexPage.xaml.cs), code-first

#### 🟢 Sonnet 5 Review

Stacked z-index labels are pixel-identical between cpp and maui in both light and dark themes, same colors, overlap order, and text.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

</details>

<details>
<summary><h2>macOS (181 examples) — click to expand</h2></summary>

.NET MAUI on macOS **is** Mac Catalyst (UIKit) — the MAUI / C++ / C++&amp;XAML columns are the strict parity board. The **AppKit** columns are the native-NSView backend (no MAUI reference; they track completeness, C++ == C++&amp;XAML).

**Discrepancy counts** (MAUI-vs-C++ parity verdicts; Sonnet 5 `claude-sonnet-5` and Gemini review each page independently):

| Classification | Sonnet 5 | Gemini |
| --- | --- | --- |
| 🟢 Match | 89 | 0 |
| 🟡 Minor | 55 | 0 |
| 🔴 Major | 31 | 0 |
| ⬛ Blank | 6 | 0 |
| ⏳ Unreviewed | 0 | 181 |

### 1. Absolute Layout — 🟡/⏳
<sub>absolute_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/absolute_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/absolute_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/absolute_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/absolute_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/absolute_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/absolute_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/absolute_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/absolute_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/absolute_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/absolute_layout_dark.png" /></td></tr></table>

ports AbsoluteLayoutPage.xaml A self-contained, code-first demo of the AbsoluteLayout control

#### 🟡 Sonnet 5 Review

Layout and colors match in both themes, but the AutoSized blue box is shrunk tight to its text instead of MAUI's wider proportionally-sized box (light and dark).

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 2. Activity Indicator — 🟡/⏳
<sub>activity_indicator</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/activity_indicator_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/activity_indicator_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/activity_indicator_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/activity_indicator_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/activity_indicator_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/activity_indicator_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/activity_indicator_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/activity_indicator_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/activity_indicator_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/activity_indicator_dark.png" /></td></tr></table>

ports ActivityIndicatorPage.xaml (+ ActivityIndicatorPage.xaml.cs)

#### 🟡 Sonnet 5 Review

Spinners, yellow background bar and layout match in both themes, but three section labels have wrong text ('Styled - Color from theme', 'Styled - BackgroundColor=Yellow', 'Smaller - HorizontalOptions=Center' vs MAUI's 'Color', 'BackgroundColor=Yellow', 'Smaller').

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 3. Adaptive Collection — 🟡/⏳
<sub>adaptive_collection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/adaptive_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/adaptive_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/adaptive_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/adaptive_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/adaptive_collection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/adaptive_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/adaptive_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/adaptive_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/adaptive_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/adaptive_collection_dark.png" /></td></tr></table>

ports AdaptiveCollectionView.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.AdaptiveCollectionView)

#### 🟡 Sonnet 5 Review

Item list matches MAUI's centered, spaced column in both themes, but an extra 'Layout: Linear (single column)' header is rendered that MAUI does not show.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 4. Alerts — 🟡/⏳
<sub>alerts</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/alerts_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/alerts_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/alerts_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/alerts_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/alerts_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/alerts_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/alerts_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/alerts_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/alerts_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/alerts_dark.png" /></td></tr></table>

ports AlertsPage.xaml (+ AlertsPage.xaml.cs) The C# AlertsPage drives the three Page dialog services — DisplayAlertAsync (simple OK + Yes/No), DisplayActionSheetAsync (simple + Cancel/Delete), and DisplayPromptAsync (two questions) — from a

#### 🟡 Sonnet 5 Review

Buttons/links match, but an extra 'OnAppearing: Alert — Welcome to the Alerts Page [Hello!]' line is shown and the three section headers render in small regular type instead of MAUI's large bold headers (both themes).

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 5. Alignment — 🟢/⏳
<sub>alignment</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/alignment_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/alignment_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/alignment_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/alignment_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/alignment_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/alignment_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/alignment_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/alignment_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/alignment_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/alignment_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;alignment&amp;quot; demo (ComparePages.Alignment()), the shipped-.NET-MAUI reference for the visual-parity comparison

#### 🟢 Sonnet 5 Review

Start/Center/End/Fill buttons with red borders match MAUI's sizes, colors and alignments in both themes; only the exempt uniform chrome shift differs.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 6. Animation — 🟡/⏳
<sub>animation</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/animation_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/animation_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/animation_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/animation_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/animation_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/animation_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/animation_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/animation_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/animation_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/animation_dark.png" /></td></tr></table>

ports AnimationPage.xaml (+ AnimationPage.xaml.cs)

#### 🟡 Sonnet 5 Review

C1/C3: all content present and matching (purple sub image, Start/Start Custom/Cancel Animation buttons, correct colors both themes), but vertical layout differs: MAUI shows the sub mid-page with the buttons pinned near the bottom, while C++ shows the sub at the top with the buttons directly beneath it. Consistent with the known animation-state capture gap (MAUI captured after autoplay repositioning) rather than missing content.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 7. App Theme Binding — 🔴/⏳
<sub>app_theme_binding</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/app_theme_binding_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/app_theme_binding_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/app_theme_binding_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/app_theme_binding_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/app_theme_binding_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/app_theme_binding_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/app_theme_binding_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/app_theme_binding_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/app_theme_binding_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/app_theme_binding_dark.png" /></td></tr></table>

ports AppThemeBindingPage.xaml

#### 🔴 Sonnet 5 Review

C1: cpp adds content MAUI lacks (a 'Toggle theme (Light/Dark)' link and a 'Theme: Light — inline=Green, resource=Orange' status label) and both headers render plain/regular instead of MAUI's large bold headers. C3: dark theme not applied — text stays green/orange (light values) instead of MAUI's red/teal, and the status label still says 'Theme: Light'. Worst = red.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 8. Application Control — 🟡/⏳
<sub>application_control</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/application_control_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/application_control_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/application_control_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/application_control_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/application_control_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/application_control_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/application_control_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/application_control_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/application_control_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/application_control_dark.png" /></td></tr></table>

ports ApplicationControlPage.xaml (+ .xaml.cs)

#### 🟡 Sonnet 5 Review

Both themes: 'Quits the application' renders bold vs MAUI's plain text, and the status label reads 'Windows open: 1 | main window: ...' instead of MAUI's 'Application: not yet hosted'. Buttons and layout otherwise match.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 9. Auto Size Shapes — 🟢/⏳
<sub>auto_size_shapes</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/auto_size_shapes_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/auto_size_shapes_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/auto_size_shapes_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/auto_size_shapes_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/auto_size_shapes_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/auto_size_shapes_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/auto_size_shapes_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/auto_size_shapes_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/auto_size_shapes_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/auto_size_shapes_dark.png" /></td></tr></table>

ports AutoSizeShapesGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/AutoSizeShapesGallery.xaml: a 3-row Grid (RowSpacing 0) that proves a stroked Ellipse auto-sizes to fill exactly half of the av

#### 🟢 Sonnet 5 Review

Both themes match: black caption bar, yellow upper half with the blue-stroked green ellipse filling it, orange lower half, identical proportions.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 10. Basic Grouping — 🔴/⏳
<sub>basic_grouping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/basic_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/basic_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/basic_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/basic_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/basic_grouping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/basic_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/basic_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/basic_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/basic_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/basic_grouping_dark.png" /></td></tr></table>

ports GroupingGalleries/BasicGrouping.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🔴 Sonnet 5 Review

Both themes: cpp renders the full grouped roster (Avengers/Fantastic Four/... with green group headers and orange totals) while the MAUI reference shows only the header and footer with an empty list — a major content mismatch vs the ground truth (likely a MAUI-side empty grouped-CollectionView quirk worth flagging).

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 11. Basic Swipe — 🟡/⏳
<sub>basic_swipe</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/basic_swipe_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/basic_swipe_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/basic_swipe_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/basic_swipe_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/basic_swipe_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/basic_swipe_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/basic_swipe_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/basic_swipe_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/basic_swipe_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/basic_swipe_dark.png" /></td></tr></table>

ports BasicSwipeGallery.xaml A code-first port of the MAUI SwipeView sub-gallery Pages/Controls/SwipeViewGalleries/BasicSwipeGallery.xaml: a vertical StackLayout of five SwipeViews, each demonstrating a different revealed-side / SwipeMode c

#### 🟡 Sonnet 5 Review

C1/C3: five swipe boxes have correct size, width, spacing and gray fill in both themes, but the box labels are anchored top-left instead of MAUI's centered text, and a stray 'Swipe a row, then invoke Delete' helper label is rendered at the far window-left below the stack (absent in the MAUI capture). Dark theme additionally renders the box text white vs MAUI's dark-on-gray... (MAUI dark uses light text too, matches). Alignment + stray label = content diffs.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 12. Behaviors — 🟢/⏳
<sub>behaviors</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/behaviors_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/behaviors_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/behaviors_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/behaviors_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/behaviors_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/behaviors_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/behaviors_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/behaviors_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/behaviors_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/behaviors_dark.png" /></td></tr></table>

ports BehaviorsPage.xaml (+ .xaml.cs) and its companion Controls.Sample/Behaviors/NumericValidationBehavior.cs

#### 🟢 Sonnet 5 Review

Both themes match: same title and 'Enter a System.Double' entry with correct light/dark field styling; only the exempt outer inset differs (entry sits flush to the window edge).

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 13. Border — 🟢/⏳
<sub>border</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/border_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/border_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;border&amp;quot; demo (ComparePages.BorderPage()), the shipped-.NET-MAUI reference for the visual-parity comparison: a single Border centered on the page — red 5pt stroke, a RoundRectangle StrokeShape (Co

#### 🟢 Sonnet 5 Review

Light and dark both render the identical red-stroked, cream-filled rounded border with 'Bordered content' centered; the dark theme's faint light-on-cream text matches MAUI exactly. Only the exempt window-chrome shift differs.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 14. Border Alignment — 🟢/⏳
<sub>border_alignment</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/border_alignment_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_alignment_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_alignment_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_alignment_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_alignment_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/border_alignment_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_alignment_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_alignment_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_alignment_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_alignment_dark.png" /></td></tr></table>

#### 🟢 Sonnet 5 Review

Start/Center/End/Fill labeled blue boxes with red borders match MAUI in size, position and colors in both themes; only the exempt uniform vertical chrome offset differs.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 15. Border Clip Playground — 🟡/⏳
<sub>border_clip_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/border_clip_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_clip_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_clip_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_clip_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_clip_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/border_clip_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_clip_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_clip_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_clip_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_clip_playground_dark.png" /></td></tr></table>

ports BorderClipPlayground.xaml (+ .xaml.cs) The C# page is an interactive Border-shape playground: a 100x100 Border (red stroke) clips an AspectFill Image (oasis.jpg) into the currently selected StrokeShape, while controls below mutate the

#### 🟡 Sonnet 5 Review

C1/C3: clipped dog image and controls all present with correct slider values, but the red border ring renders much thicker than MAUI's thin stroke, the 'Border Shape'/'Border'/'Corner Radius' headers lose MAUI's bold large styling (all text renders in one plain size/weight), and the shape entry shows 'RoundRectangle' where MAUI's entry is empty. Same diffs in dark; dark thumbs also render slightly larger.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 16. Border Layout — 🟢/⏳
<sub>border_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/border_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/border_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_layout_dark.png" /></td></tr></table>

ports BorderLayout.xaml (+ BorderLayout.xaml.cs) The C# page demonstrates driving Border.StrokeThickness from a Slider: a Slider (0..40, set to 5 in OnAppearing) is bound to the Border&amp;#x27;s StrokeThickness; the Border (Silver stroke, White bac

#### 🟢 Sonnet 5 Review

Slider plus the green pill border with red rounded-left segment, 'Center' label and blue square match MAUI in both themes, including black text in light and white text in dark.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 17. Border Playground — 🔴/⏳
<sub>border_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/border_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/border_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_playground_dark.png" /></td></tr></table>

ports BorderPlayground.xaml (+ BorderPlayground.xaml.cs) A self-contained, code-first interactive Border playground

#### 🔴 Sonnet 5 Review

C1/C3: gradient banner + dashed yellow border match, but the control panel diverges heavily: entries are populated ('Label', 'RoundRectangle', 'Miter', 'Butt') where MAUI shows empty placeholder entries; the four color entries render with full-width colored backgrounds (blue/yellow swatch bars) vs MAUI's plain white/black entries; the radio label sits inline next to the circle at left vs MAUI's centered circle with label below; label typography is smaller; an extra Corner Radius section is already visible. Same in dark. Multiple color/content mismatches.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 18. Border Resize Content — 🟡/⏳
<sub>border_resize_content</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/border_resize_content_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_resize_content_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_resize_content_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_resize_content_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_resize_content_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/border_resize_content_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_resize_content_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_resize_content_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_resize_content_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_resize_content_dark.png" /></td></tr></table>

ports BorderResizeContent.xaml A self-contained, code-first demo that resizes a Border&amp;#x27;s CONTENT and watches the Border track it

#### 🟡 Sonnet 5 Review

C1/C3: circle and square cells match in both columns and themes (colors, green borders, blue plus, dog-image fills with light-blue letterboxing). The triangle row differs: MAUI's left triangle shows a large salmon fill with the blue plus overflowing the green outline, and its right triangle is filled by the clipped dog photo; C++ renders both triangles mostly hollow with only a small clipped fill inside the outline. Content clipped/undersized in 2 of 6 cells.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 19. Border Stroke — 🟡/⏳
<sub>border_stroke</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/border_stroke_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_stroke_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_stroke_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_stroke_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_stroke_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/border_stroke_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_stroke_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_stroke_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_stroke_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_stroke_dark.png" /></td></tr></table>

ports BorderStroke.xaml (+ BorderStroke.xaml.cs) A self-contained, code-first demo of Border StrokeThickness and how a Border tracks the height of its content

#### 🟡 Sonnet 5 Review

Light theme matches MAUI across both StrokeThickness sections. In dark theme the slider's unfilled track is rendered much darker than MAUI's light-gray track, a small chrome color diff; everything else (borders, bar heights, labels) matches.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 20. Border Styles — 🟢/⏳
<sub>border_styles</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/border_styles_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_styles_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_styles_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_styles_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_styles_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/border_styles_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_styles_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_styles_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_styles_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_styles_dark.png" /></td></tr></table>

#### 🟢 Sonnet 5 Review

Both themes match: red RoundRectangle borders, yellow 10-radius border with red stroke, and the two blue text buttons all render at MAUI sizes/colors, including the dark-theme white label inside the yellow box.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 21. Borderless — 🟢/⏳
<sub>borderless</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/borderless_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/borderless_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/borderless_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/borderless_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/borderless_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/borderless_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/borderless_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/borderless_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/borderless_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/borderless_dark.png" /></td></tr></table>

ports Borderless.xaml A self-contained, code-first demo of a stroke-less Border

#### 🟢 Sonnet 5 Review

C1/C3: yellow/pink/red color-band layout matches MAUI exactly in both themes (window-chrome title-bar tint differences are the exempt native-chrome variance).

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 22. Box View — 🟢/⏳
<sub>box_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/box_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/box_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/box_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/box_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/box_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/box_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/box_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/box_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/box_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/box_view_dark.png" /></td></tr></table>

ports BoxViewPage.xaml (+ BoxViewPage.xaml.cs)

#### 🟢 Sonnet 5 Review

All five BoxView sections (default blue, purple Color, yellow-green gradient Background, CornerRadius green, complex-corner orange) match MAUI in both themes, including dark-theme label colors.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 23. Button — 🔴/⏳
<sub>button</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/button_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/button_dark.png" /></td></tr></table>

ports ButtonPage.xaml (+ ButtonPage.xaml.cs)

#### 🔴 Sonnet 5 Review

C1/C3: text buttons, colored bars, BorderWidth red outline, CornerRadius purple pill, gray Button and green BorderWidth Changing rows all match in both themes, but the two ImageButton 'settings' rows render as huge (~5-6x taller) black blocks containing a large gear glyph next to the text, whereas MAUI shows thin single-line black bars with just centered 'settings' text. Major size/content divergence on those two rows in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 24. Carousel Page — 🔴/⏳
<sub>carousel_page</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/carousel_page_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/carousel_page_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/carousel_page_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/carousel_page_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/carousel_page_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/carousel_page_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/carousel_page_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/carousel_page_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/carousel_page_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/carousel_page_dark.png" /></td></tr></table>

Carousel Page

#### 🔴 Sonnet 5 Review

The MAUI reference capture is completely blank in both themes while cpp renders a 'Basic Horizontal Carousel' with Item 1, Prev/Next and position text — no usable ground truth to confirm parity, so both comparisons mismatch as captured; the MAUI ref likely needs recapture.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 25. Carousel View — ⬛/⏳
<sub>carousel_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/carousel_view_light.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/carousel_view_light.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/carousel_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/carousel_view_dark.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/carousel_view_dark.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/carousel_view_dark.png" /></td></tr></table>

#### ⬛ C++ Sonnet 5 Review

No code-first builder twin exists for this page (manifest.json builder_twin:false) — the gallery app's MAUI_SAMPLE_PAGE dispatch has no entry for this key. e2e.py cmd_capture skips the cpp framework for builder_twin:false keys instead of capturing a misleading value_controls fallback-page screenshot. Only the C++&amp;XAML column applies to this page.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 26. Chat Example — 🔴/⏳
<sub>chat_example</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/chat_example_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/chat_example_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/chat_example_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/chat_example_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/chat_example_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/chat_example_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/chat_example_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/chat_example_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/chat_example_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/chat_example_dark.png" /></td></tr></table>

ports ChatExample.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.ItemSizeGalleries.ChatExample), tracking the maui-compare reference demo ~/maui-compare/Pages/ChatExamplePage.cs (the visual-parity oracle)

#### 🔴 Sonnet 5 Review

C++ is missing the 'Append Random Message / Clear / Add 1000 Messages' action row that MAUI shows, and instead displays two pre-populated chat bubbles ('Hi there!', 'Hello — how can I help you today?') absent from MAUI; same mismatch in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 27. Check Box — 🟡/⏳
<sub>check_box</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/check_box_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/check_box_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/check_box_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/check_box_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/check_box_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/check_box_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/check_box_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/check_box_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/check_box_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/check_box_dark.png" /></td></tr></table>

ports CheckBoxPage.xaml (+ .xaml.cs) Mirrors the MAUI gallery page: a vertical stack of headlined CheckBox states — Default, Colored (Color=Purple), Disabled, Disabled+Colored+Checked — followed by a &amp;quot;Change IsChecked&amp;quot; row pairing a Button

#### 🟡 Sonnet 5 Review

All five sections, checkbox states, purple/red tints and the red 'Is green? False' row match in both themes, but the vertical pitch between each label and its checkbox is noticeably larger in C++ (~77px row spacing in MAUI vs ~115px), stretching the page.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 28. Chrome — 🟢/⏳
<sub>chrome</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/chrome_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/chrome_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/chrome_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/chrome_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/chrome_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/chrome_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/chrome_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/chrome_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/chrome_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/chrome_dark.png" /></td></tr></table>

a self-contained demo page for the W1-11 window-chrome family: page toolbar items (primary + secondary), a menu bar (File menu with items, a separator and a sub-menu), a context flyout (right-click menu) on a button, and a tooltip — all wir

#### 🟢 Sonnet 5 Review

Centered blue 'Press or right-click me' link and left-aligned 'Ready' label match MAUI in position, color and size in both light and dark.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 29. Clip — 🟢/⏳
<sub>clip</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/clip_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/clip_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/clip_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/clip_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/clip_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/clip_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/clip_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/clip_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/clip_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/clip_dark.png" /></td></tr></table>

ports ClipPage.xaml The C# page (Pages/Core/ClipPage.xaml; its .xaml.cs is an empty InitializeComponent) is a ScrollView over a StackLayout that shows the SAME dotnet_bot.png image five times, each successive copy carrying a different geome

#### 🟢 Sonnet 5 Review

C1/C3: cpp light+dark match MAUI exactly — same submarine image, same RectangleGeometry/EllipseGeometry/GeometryGroup/PathGeometry clip shapes and crop boundaries, identical layout and captions. No content differences.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 30. Clip Corner Radius — 🟢/⏳
<sub>clip_corner_radius</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/clip_corner_radius_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/clip_corner_radius_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/clip_corner_radius_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/clip_corner_radius_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/clip_corner_radius_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/clip_corner_radius_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/clip_corner_radius_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/clip_corner_radius_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/clip_corner_radius_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/clip_corner_radius_dark.png" /></td></tr></table>

ports ClipCornerRadiusGallery.xaml (+ .xaml.cs) The C# page (Pages/Controls/ShapesGalleries/ClipCornerRadiusGallery.xaml) is a StackLayout (Padding=12) that demonstrates DRIVING a RoundRectangleGeometry&amp;#x27;s per-corner CornerRadius from four s

#### 🟢 Sonnet 5 Review

C1/C3 (cpp vs MAUI, light/dark): clipped dog image via RoundRectangleGeometry matches (same corners rounded, same crop), all four corner-radius sliders match MAUI's labels/positions/values in both themes. Slider track color is marginally darker gray than MAUI's in light theme — trivial styling, not a content diff.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 31. Clip Gallery — 🟢/⏳
<sub>clip_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/clip_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/clip_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/clip_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/clip_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/clip_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/clip_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/clip_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/clip_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/clip_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/clip_gallery_dark.png" /></td></tr></table>

ports ClipGallery.xaml The C# page (Pages/Controls/ShapesGalleries/ClipGallery.xaml; its .xaml.cs is an empty InitializeComponent) is a ScrollView over a StackLayout (Padding=12) that shows the SAME &amp;quot;oasis.jpg&amp;quot; image SEVEN times — one bare

#### 🟢 Sonnet 5 Review

C1/C3: cpp light+dark match MAUI — same pug photo, same Rectangle/RoundRectangle/Ellipse/GeometryGroup clip crops, identical sizes and positions visible in the captured viewport.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 32. Clip Views — 🟢/⏳
<sub>clip_views</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/clip_views_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/clip_views_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/clip_views_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/clip_views_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/clip_views_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/clip_views_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/clip_views_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/clip_views_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/clip_views_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/clip_views_dark.png" /></td></tr></table>

ports ClipViewsGallery.xaml The C# page (Pages/Controls/ShapesGalleries/ClipViewsGallery.xaml; no code-behind beyond an empty InitializeComponent) is a ScrollView over a StackLayout (Padding=12) that proves the Clip surface (VisualElement.C

#### 🟢 Sonnet 5 Review

C1/C3 (cpp vs MAUI, light/dark): content matches exactly — same red curved-clip bars via BezierSegment path clipping across Entry/Editor/Grid/SearchBar/TimePicker rows, same text, same positions. No port bugs.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 33. Clipping — 🟡/⏳
<sub>clipping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/clipping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/clipping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/clipping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/clipping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/clipping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/clipping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/clipping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/clipping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/clipping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/clipping_dark.png" /></td></tr></table>

compare oracle ~/maui-compare/Pages/ClippingPage.cs (itself written to mirror this gallery page)

#### 🟡 Sonnet 5 Review

C1/C3 yellow: layout, colors, digit row, orange square, purple 'Hey' bar and blue stripe all match MAUI, but the port shows two coffee-cup images at the left end of the blue stripe in both themes that are absent from the MAUI reference (likely a MAUI-side image-load quirk, but per ruling MAUI is ground truth — extra content).

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 34. Collectionview — 🟢/⏳
<sub>collectionview</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/collectionview_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/collectionview_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/collectionview_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/collectionview_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/collectionview_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/collectionview_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/collectionview_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/collectionview_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/collectionview_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/collectionview_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;collectionview&amp;quot; demo (ComparePages.CollectionViewPage()), the shipped-.NET-MAUI reference for the visual-parity comparison: a CollectionView over 24 captioned items, a string Header (&amp;quot;This is the

#### 🟢 Sonnet 5 Review

C1/C3 both green. Header, 3-column grid of 24 'name.jpg, N' items, row pitch, fonts and light/dark colors all match MAUI. Only the exempt window-chrome offset differs.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 35. Composition Gallery — 🔴/⏳
<sub>composition_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/composition_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/composition_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/composition_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/composition_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/composition_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/composition_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/composition_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/composition_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/composition_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/composition_gallery_dark.png" /></td></tr></table>

ports CompositionGallery.xaml A self-contained, code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/CompositionGallery.xaml: a StackLayout holding two Beige 250x250 Grids (Margin 12) that compose multiple overlappi

#### 🔴 Sonnet 5 Review

In both themes the first canvas composition is wrong: the yellow circle is displaced down-right and layered over the triangle (MAUI has it up-left, under the triangle), and the pink stripe overshoots past the triangle's top-left corner; the second line canvas matches.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 36. Containers — 🟢/⏳
<sub>containers</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/containers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/containers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/containers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/containers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/containers_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/containers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/containers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/containers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/containers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/containers_dark.png" /></td></tr></table>

a self-contained demo page for the W1-07 container set: a scroll_view hosting a vertical stack of content-hosting containers — a border-framed label (stroke + dashed outline + rounded shape), a legacy frame (BorderColor/CornerRadius/HasShad

#### 🟢 Sonnet 5 Review

Scrolled-to label, dashed blue border box, red frame, and content_view text all match MAUI's layout and colors in both light and dark themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 37. Content View — 🟢/⏳
<sub>content_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/content_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/content_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/content_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/content_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/content_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/content_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/content_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/content_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/content_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/content_view_dark.png" /></td></tr></table>

ports ContentViewPage.xaml (+ ContentViewPage.xaml.cs), code-first

#### 🟢 Sonnet 5 Review

C1/C3: ContentView labels ('ContentView', two 'Content' rows) and blue 'Swap content' button match MAUI in both themes; only the exempt window-chrome vertical offset differs.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 38. Context Flyout — 🟡/⏳
<sub>context_flyout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/context_flyout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/context_flyout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/context_flyout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/context_flyout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/context_flyout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/context_flyout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/context_flyout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/context_flyout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/context_flyout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/context_flyout_dark.png" /></td></tr></table>

ports ContextFlyoutPage.xaml (+ ContextFlyoutPage.xaml.cs) The C# page attaches a MenuFlyout as the FlyoutBase.ContextFlyout (right-click / long-press menu) of several controls and wires each menu item to a handler: - a Button (&amp;quot;Increment b

#### 🟡 Sonnet 5 Review

C1/C3: 'COOL' FontImageSource button, toggle, and labels all match MAUI exactly; the only difference is whether the embedded live Bing WebView's cookie-consent banner renders as an inline bottom banner (MAUI light capture) vs. a centered modal (MAUI dark, and both C++ captures) — this is non-deterministic live web-content state/timing, not app code, so treated as minor.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 39. Controls Stack — 🟢/⏳
<sub>controls_stack</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/controls_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/controls_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/controls_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/controls_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/controls_stack_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/controls_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/controls_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/controls_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/controls_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/controls_stack_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;controls_stack&amp;quot; demo (ComparePages.ControlsStack()), the shipped-.NET-MAUI reference for the visual-parity comparison: a VerticalStackLayout (Spacing 12, Padding 16) showcasing the basic widgets

#### 🟢 Sonnet 5 Review

C1/C3: button, entry, editor, search bar, checkbox, switch, activity indicator, slider (same value) and stepper + progress bar all match in both themes; dark entry/search chrome matches.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 40. Custom Layout — 🟢/⏳
<sub>custom_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/custom_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/custom_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/custom_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/custom_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/custom_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/custom_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/custom_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/custom_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/custom_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/custom_layout_dark.png" /></td></tr></table>

ports CustomLayoutPage.xaml (+ CustomLayoutPage.xaml.cs)

#### 🟢 Sonnet 5 Review

C1/C3: custom layout places Top / Left Left / Right Right / Bottom links at the same edge positions with the same blue link styling in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 41. Custom Size Swipe — 🟡/⏳
<sub>custom_size_swipe</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/custom_size_swipe_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/custom_size_swipe_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/custom_size_swipe_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/custom_size_swipe_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/custom_size_swipe_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/custom_size_swipe_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/custom_size_swipe_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/custom_size_swipe_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/custom_size_swipe_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/custom_size_swipe_dark.png" /></td></tr></table>

ports CustomSizeSwipeViewGallery.xaml (+ .xaml.cs) The MAUI CustomSizeSwipeViewGallery is a single SwipeView whose Left / Right / Top item collections each reveal CUSTOM-SIZED content: a SwipeItemView wrapping a Grid/StackLayout with an exp

#### 🟡 Sonnet 5 Review

C1/C3: green SwipeView content band and 'Test Click from Content' link match, but the status label reads 'RightItems revealed (open=1, threshold=0)' instead of MAUI's 'Ready (swipe a side to reveal its custom-sized content)' — the capture was taken with the swipe programmatically opened (runtime state diff, no visual defect in the control itself).

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 42. Custom Swipe Item View — 🟡/⏳
<sub>custom_swipe_item_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/custom_swipe_item_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/custom_swipe_item_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/custom_swipe_item_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/custom_swipe_item_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/custom_swipe_item_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/custom_swipe_item_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/custom_swipe_item_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/custom_swipe_item_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/custom_swipe_item_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/custom_swipe_item_view_dark.png" /></td></tr></table>

ports CustomSwipeItemViewGallery.xaml A self-contained, code-first port of the .NET MAUI &amp;quot;CustomSwipeItem&amp;quot; gallery: a message-list row whose right swipe reveals a CUSTOM-content swipe item (a swipe_item_view, not a plain swipe_item)

#### 🟡 Sonnet 5 Review

C1/C3: indigo 'Welcome to .NET MAUI / June 19, 2026' card matches in size, color and text, but the status label reads 'Right items revealed (Favourite)' instead of MAUI's 'Swipe a row left to reveal the Favourite item' — swipe state exercised in the capture (runtime state diff only).

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 43. Cv Visual States — 🔴/⏳
<sub>cv_visual_states</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/cv_visual_states_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/cv_visual_states_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/cv_visual_states_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/cv_visual_states_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/cv_visual_states_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/cv_visual_states_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/cv_visual_states_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/cv_visual_states_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/cv_visual_states_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/cv_visual_states_dark.png" /></td></tr></table>

ports CollectionViewGalleries/SelectionGalleries/ VisualStatesGallery.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🔴 Sonnet 5 Review

C1 light matches (Single/Multi Selection headers + Item rows identical). C3 dark diverges badly: MAUI renders the two CollectionView item groups as solid WHITE bands (light item backgrounds retained in dark mode, item text invisible against them), while the C++ port renders a uniformly dark background with light 'Item 1..4' text. Wrong item-background colors vs the MAUI ground truth in dark = major.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 44. Data Template Selector — 🔴/⏳
<sub>data_template_selector</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/data_template_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/data_template_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/data_template_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/data_template_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/data_template_selector_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/data_template_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/data_template_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/data_template_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/data_template_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/data_template_selector_dark.png" /></td></tr></table>

ports DataTemplateSelectorGallery.xaml (+ DataTemplateSelectorGallery.xaml.cs, including its WeekendSelector + SearchTermSelector classes)

#### 🔴 Sonnet 5 Review

Both themes: MAUI shows exactly 14 rows (Sunday..Saturday twice, plain text, ~44px row pitch). The cpp build instead applies the weekend template ('It's the weekend! Woot!' for Sunday/Saturday) AND repeats the 14-item sequence endlessly past the fold with much tighter row spacing. Item text and item count both wrong vs ground truth = major, light and dark alike.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 45. Date Picker — 🟢/⏳
<sub>date_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/date_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/date_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/date_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/date_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/date_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/date_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/date_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/date_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/date_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/date_picker_dark.png" /></td></tr></table>

ports DatePickerPage.xaml (+ DatePickerPage.xaml.cs)

#### 🟢 Sonnet 5 Review

Fixed: 'Default with date'/'Default with time' now carries the shared XAML's restored Date=06/21/2018 / Time=4:15:26 attribute, matching MAUI exactly in both themes; all other rows already matched.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 46. Device — 🟡/⏳
<sub>device</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/device_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/device_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/device_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/device_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/device_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/device_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/device_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/device_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/device_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/device_dark.png" /></td></tr></table>

ports DevicePage.xaml

#### 🟡 Sonnet 5 Review

Both themes: layout, font and centering match, but the reported values differ - cpp prints 'Platform: MacCatalyst / Idiom: Desktop / Version: 26.5' while MAUI ground truth prints 'Platform: iOS / Idiom: Phone / Version: 17.0'. This is runtime device-info semantics (the port reports the Catalyst host directly, MAUI reports the iOS-compat layer), not a rendering defect - minor, but the DeviceInfo mapping should mirror MAUI's.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 47. Dispatcher — 🟢/⏳
<sub>dispatcher</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/dispatcher_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/dispatcher_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/dispatcher_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/dispatcher_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/dispatcher_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/dispatcher_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/dispatcher_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/dispatcher_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/dispatcher_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/dispatcher_dark.png" /></td></tr></table>

ports DispatcherPage.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 Review

All four texts and blue action buttons (Fail Access / Access / 3 Seconds Later / 3 Second Timer / Device.StartTimer) plus the runtime status lines ('This was a success!', 'I happened 3 seconds later!', 'I am on a 3 second timer! 3 ticks', 'OBSOLETE ZONE ALERT!') match MAUI in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 48. Drag Drop — 🟢/⏳
<sub>drag_drop</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/drag_drop_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/drag_drop_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/drag_drop_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/drag_drop_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/drag_drop_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/drag_drop_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/drag_drop_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/drag_drop_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/drag_drop_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/drag_drop_dark.png" /></td></tr></table>

ports DragAndDropBetweenLayouts.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.DragAndDropBetweenLayouts)

#### 🟢 Sonnet 5 Review

C1 (light) and C3 (dark): C++ matches MAUI exactly - full-width rainbow color swatch stack (red/orange/yellow/green/blue/indigo/violet) plus 'Rainbow:'/drag-drop status labels, identical in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 49. Editor — 🟢/⏳
<sub>editor</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/editor_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/editor_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/editor_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/editor_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/editor_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/editor_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/editor_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/editor_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/editor_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/editor_dark.png" /></td></tr></table>

ports EditorPage.xaml (Microsoft.Maui.Controls sample gallery)

#### 🟢 Sonnet 5 Review

C1/C3: all editor states (length labels, placeholders, purple Text/Placeholder colors, large font, read-only, numeric, bottom-aligned, autosize) match MAUI in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 50. Effects — 🟢/⏳
<sub>effects</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/effects_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/effects_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/effects_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/effects_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/effects_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/effects_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/effects_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/effects_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/effects_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/effects_dark.png" /></td></tr></table>

ports EffectsPage.xaml (Maui.Controls.Sample.Pages.EffectsPage)

#### 🟢 Sonnet 5 Review

C1/C3: both entries, blue detach/re-attach links and status label match MAUI in both themes; only the exempt uniform chrome-height shift differs.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 51. Ellipse Gallery — 🟢/⏳
<sub>ellipse_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/ellipse_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ellipse_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ellipse_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ellipse_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ellipse_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/ellipse_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ellipse_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ellipse_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ellipse_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ellipse_gallery_dark.png" /></td></tr></table>

ports EllipseGallery.xaml A self-contained, code-first port of the MAUI Shapes EllipseGallery (Pages/Controls/ShapesGalleries/EllipseGallery.xaml + .xaml.cs)

#### 🟢 Sonnet 5 Review

C1 (light): MAUI and cpp both show the five ellipse variants (basic ellipse, circle outline, ellipse-with-stroke pair, dashed-stroke ellipse) left-aligned (Start) at the same x-position, same sizes/colors/strokes. C3 (dark): identical content correctly re-themed, same left alignment preserved (unlike rectangle_gallery, this page's shapes remain correctly Start-aligned).

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 52. Empty View — 🟢/⏳
<sub>empty_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/empty_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/empty_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_dark.png" /></td></tr></table>

ports EmptyViewStringGallery.xaml (+ EmptyViewStringGallery.xaml.cs)

#### 🟢 Sonnet 5 Review

C1/C3: filter list content, styling, and light/dark colors all match MAUI exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 53. Empty View Load Simulate — 🟡/⏳
<sub>empty_view_load_simulate</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/empty_view_load_simulate_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_load_simulate_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_load_simulate_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_load_simulate_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_load_simulate_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/empty_view_load_simulate_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_load_simulate_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_load_simulate_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_load_simulate_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_load_simulate_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewLoadSimulateGallery.xaml (+ EmptyViewLoadSimulateGallery.xaml.cs)

#### 🟡 Sonnet 5 Review

C1/C3: 'Items loading simulation...' text content matches, but MAUI renders it top-left aligned while C++ centers it in the page — a real positional difference beyond the exempt outer-inset shift.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 54. Empty View Null — 🟡/⏳
<sub>empty_view_null</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/empty_view_null_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_null_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_null_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_null_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_null_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/empty_view_null_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_null_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_null_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_null_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_null_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewNullGallery.xaml (+ EmptyViewNullGallery.xaml.cs)

#### 🟡 Sonnet 5 Review

C1/C3: 'Nothing to display.' text matches, but MAUI aligns it top-left while C++ centers it on the page, same positional divergence as empty_view_load_simulate.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 55. Empty View Rtl — 🔴/⏳
<sub>empty_view_rtl</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/empty_view_rtl_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_rtl_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_rtl_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_rtl_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_rtl_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/empty_view_rtl_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_rtl_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_rtl_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_rtl_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_rtl_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewRTLGallery.xaml (+ EmptyViewRTLGallery.xaml.cs)

#### 🔴 Sonnet 5 Review

C1/C3: MAUI shows a Picker with unselected placeholder text 'FlowDirection' plus a 'No results matched your filter. / Maybe try a broader filter?' empty-view overlay atop the list; C++ shows the Picker already displaying a selected value 'Left to Right' and no empty-view overlay text at all — a genuine missing-content/wrong-state bug.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 56. Empty View Selector — 🟢/⏳
<sub>empty_view_selector</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/empty_view_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_selector_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/empty_view_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_selector_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewWithDataTemplateSelector.xaml (+ .xaml.cs, incl

#### 🟢 Sonnet 5 Review

C1/C3: instructional text, filter bar, and 'Baboon — Africa &amp; Asia' result row match MAUI exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 57. Empty View Swap — 🟢/⏳
<sub>empty_view_swap</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/empty_view_swap_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_swap_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_swap_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_swap_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_swap_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/empty_view_swap_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_swap_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_swap_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_swap_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_swap_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewSwapGallery.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 Review

C1/C3: toggle switch, Clear/Fill links, and full 12-item list match MAUI exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 58. Empty View Template — 🟢/⏳
<sub>empty_view_template</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/empty_view_template_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_template_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_template_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_template_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_template_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/empty_view_template_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_template_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_template_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_template_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_template_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewTemplateGallery.xaml (+ EmptyViewTemplateGallery.xaml.cs)

#### 🟢 Sonnet 5 Review

C1/C3: full item list layout and content match MAUI exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 59. Empty View View — 🔴/⏳
<sub>empty_view_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/empty_view_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/empty_view_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_view_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewViewGallery.xaml (+ EmptyViewViewGallery.xaml.cs)

#### 🔴 Sonnet 5 Review

C1/C3: MAUI overlays a bold 'No results matched your filter.' + 'Maybe try a broader filter?' empty view atop the full item list; C++ shows only the plain list with no empty-view overlay text at all — missing content.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 60. Entry — 🟢/⏳
<sub>entry</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/entry_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/entry_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/entry_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/entry_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/entry_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/entry_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/entry_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/entry_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/entry_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/entry_dark.png" /></td></tr></table>

ports EntryPage.xaml (Microsoft.Maui.Controls sample gallery)

#### 🟢 Sonnet 5 Review

C1/C3: all rows match MAUI in both themes — LENGTH/RETURN header, 'Type here...' placeholder, purple Text/Placeholder entries, checkmark button, password dots, read-only, 'Text', right-aligned 'This should be on the end', CursorPosition = 4 slider at the same position, and 'Cursor' entry.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 61. Filter Collection — 🔴/⏳
<sub>filter_collection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/filter_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/filter_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/filter_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/filter_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/filter_collection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/filter_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/filter_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/filter_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/filter_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/filter_collection_dark.png" /></td></tr></table>

ports FilterCollectionView.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🔴 Sonnet 5 Review

C1/C3: MAUI shows an orange 'Nothing to see here' empty-view message below the list (with 'Use EmptyView' toggle ON); C++ shows the identical toggle/list but the orange empty-view message is completely absent — missing content bug.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 62. Filter Selection — 🟢/⏳
<sub>filter_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/filter_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/filter_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/filter_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/filter_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/filter_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/filter_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/filter_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/filter_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/filter_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/filter_selection_dark.png" /></td></tr></table>

ports FilterSelection.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.FilterSelection)

#### 🟢 Sonnet 5 Review

C1/C3: instructional text, Reset link, 'Selected: (none)' state, and full item list match MAUI exactly in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 63. Flex Layout — 🟡/⏳
<sub>flex_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/flex_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/flex_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/flex_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/flex_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/flex_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/flex_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/flex_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/flex_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/flex_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/flex_layout_dark.png" /></td></tr></table>

ports FlexLayoutPage.xaml A self-contained, code-first demo of the FlexLayout control: the classic &amp;quot;holy grail&amp;quot; page layout built from nested flexboxes

#### 🟡 Sonnet 5 Review

C1 green: header/footer bars, blue/green columns and gray content all match. C3 yellow: in dark theme the HEADER and CONTENT label text renders dark gray where MAUI renders it white/light (label default text color not theme-adaptive in dark).

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 64. Focus — 🟢/⏳
<sub>focus</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/focus_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/focus_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/focus_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/focus_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/focus_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/focus_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/focus_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/focus_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/focus_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/focus_dark.png" /></td></tr></table>

ports FocusPage.xaml (+ FocusPage.xaml.cs) The C# FocusPage is a focus-subsystem demo: an Entry whose Focused/Unfocused events (OnFocusEntryFocusChanged) append &amp;quot;Focused&amp;quot;/&amp;quot;Unfocused&amp;quot; lines to a scrolling InfoLabel, plus two buttons — &amp;quot;Focus

#### 🟢 Sonnet 5 Review

C1/C3 green: Focus target entry, blue 'Focus Entry'/'Unfocus Entry' buttons and 'IsFocused: false' label all match in both themes; only the exempt outer-inset/window-chrome shift differs.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 65. Fonts — 🟢/⏳
<sub>fonts</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/fonts_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/fonts_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/fonts_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/fonts_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/fonts_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/fonts_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/fonts_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/fonts_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/fonts_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/fonts_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;fonts&amp;quot; demo (ComparePages.Fonts()), the shipped-.NET-MAUI reference for the visual-parity comparison: a ScrollView over a VerticalStackLayout (Spacing 8, Padding 16) of nine Labels — Title/Subtit

#### 🟢 Sonnet 5 Review

C1/C3 green: Title/Subtitle/Header/Body/Caption size ramp, Bold, Italic, Bold+Italic and 'Character spacing 4.0' all render identically in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 66. Footer Only String — 🟡/⏳
<sub>footer_only_string</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/footer_only_string_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/footer_only_string_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/footer_only_string_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/footer_only_string_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/footer_only_string_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/footer_only_string_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/footer_only_string_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/footer_only_string_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/footer_only_string_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/footer_only_string_dark.png" /></td></tr></table>

ports FooterOnlyString.xaml (+ FooterOnlyString.xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟡 Sonnet 5 Review

C1/C3: all 20 items (cover1.jpg,0 … FlowerBuds.jpg,19) and the bold 'This is a footer' string present in both themes; only diff is row spacing — MAUI rows ~44px apart, cpp ~26px, so the list ends at y~615 vs MAUI y~950. Minor internal-spacing diff.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 67. Formatted Text — 🟢/⏳
<sub>formatted_text</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/formatted_text_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/formatted_text_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/formatted_text_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/formatted_text_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/formatted_text_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/formatted_text_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/formatted_text_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/formatted_text_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/formatted_text_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/formatted_text_dark.png" /></td></tr></table>

a self-contained demo page for the G1 rich-text slice: a label whose FormattedText is built from several styled spans (bold / italic / colored / underlined / kerned), plus a plain label proving the Text ⇄ FormattedText exclusivity

#### 🟢 Sonnet 5 Review

C1/C3: formatted span (bold red, italic underlined, kerned) and plain label match MAUI in both themes; only exempt window-chrome offset differs.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 68. Frame — ⬛/⏳
<sub>frame</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/frame_light.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/frame_light.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/frame_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/frame_dark.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/frame_dark.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/frame_dark.png" /></td></tr></table>

#### ⬛ C++ Sonnet 5 Review

No code-first builder twin exists for this page (manifest.json builder_twin:false) — the gallery app's MAUI_SAMPLE_PAGE dispatch has no entry for this key. e2e.py cmd_capture skips the cpp framework for builder_twin:false keys instead of capturing a misleading value_controls fallback-page screenshot. Only the C++&amp;XAML column applies to this page.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 69. Gestures — 🟡/⏳
<sub>gestures</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/gestures_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/gestures_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/gestures_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/gestures_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/gestures_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/gestures_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/gestures_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/gestures_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/gestures_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/gestures_dark.png" /></td></tr></table>

ports GesturesPage.xaml (+ .xaml.cs) The MAUI GesturesPage.xaml is a *gallery navigation* page: a CollectionView listing gesture-demo sections that the shell navigates into

#### 🟡 Sonnet 5 Review

C1/C3: layout and blue gesture target match, but status label reads 'Last gesture: Pointer exited' vs MAUI's '(none)' — capture-time pointer state, minor runtime-state diff in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 70. Gradient — 🟢/⏳
<sub>gradient</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/gradient_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/gradient_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/gradient_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/gradient_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/gradient_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/gradient_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/gradient_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/gradient_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/gradient_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/gradient_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;gradient&amp;quot; demo (ComparePages.Gradient()), the shipped-.NET-MAUI reference for the visual-parity comparison: a VerticalStackLayout (Spacing 12, Padding 16) of two captioned 60px BoxViews — a Linea

#### 🟢 Sonnet 5 Review

C1/C3: linear yellow→green and radial red→navy gradient bars match MAUI in both themes (colors, stops, bar sizes).

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 71. Grid — 🟢/⏳
<sub>grid</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/grid_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/grid_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/grid_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/grid_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/grid_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/grid_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/grid_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/grid_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/grid_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/grid_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;grid&amp;quot; demo (ComparePages.GridPage()), the shipped-.NET-MAUI reference for the visual-parity comparison: a Grid (Padding 16, Row/ColumnSpacing 6) with RowDefinitions Auto / 80 / 80 and two Star co

#### 🟢 Sonnet 5 Review

C1/C3: 2x2 grid (red/green/blue/orange) matches MAUI in both themes — cell sizes, spacing and colors identical.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 72. Grid Definitions — ⬛/⏳
<sub>grid_definitions</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/grid_definitions_light.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/grid_definitions_light.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/grid_definitions_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/grid_definitions_dark.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/grid_definitions_dark.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/grid_definitions_dark.png" /></td></tr></table>

#### ⬛ C++ Sonnet 5 Review

No code-first builder twin exists for this page (manifest.json builder_twin:false) — the gallery app's MAUI_SAMPLE_PAGE dispatch has no entry for this key. e2e.py cmd_capture skips the cpp framework for builder_twin:false keys instead of capturing a misleading value_controls fallback-page screenshot. Only the C++&amp;XAML column applies to this page.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 73. Grid Grouping — 🔴/⏳
<sub>grid_grouping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/grid_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/grid_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/grid_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/grid_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/grid_grouping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/grid_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/grid_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/grid_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/grid_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/grid_grouping_dark.png" /></td></tr></table>

ports GroupingGalleries/GridGrouping.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🔴 Sonnet 5 Review

C1/C3: MAUI reference renders a completely BLANK page in both themes (suspected broken MAUI ref capture — flag per ruling 3), while C++ renders a full grouped CollectionView (header 'This is a header', 2-column group sections Avengers/Fantastic Four/Defenders/Heroes for Hire/West Coast Avengers/Great Lakes Avengers with green group headers and orange 'Total members' footers, page footer). Content cannot match a blank ground truth; needs MAUI recapture/ruling before the C++ render can be scored properly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 74. Grid Layout — ⬛/⏳
<sub>grid_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/grid_layout_light.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/grid_layout_light.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/grid_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/grid_layout_dark.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/grid_layout_dark.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/grid_layout_dark.png" /></td></tr></table>

#### ⬛ C++ Sonnet 5 Review

No code-first builder twin exists for this page (manifest.json builder_twin:false) — the gallery app's MAUI_SAMPLE_PAGE dispatch has no entry for this key. e2e.py cmd_capture skips the cpp framework for builder_twin:false keys instead of capturing a misleading value_controls fallback-page screenshot. Only the C++&amp;XAML column applies to this page.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 75. Grouping No Templates — 🔴/⏳
<sub>grouping_no_templates</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/grouping_no_templates_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/grouping_no_templates_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/grouping_no_templates_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/grouping_no_templates_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/grouping_no_templates_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/grouping_no_templates_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/grouping_no_templates_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/grouping_no_templates_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/grouping_no_templates_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/grouping_no_templates_dark.png" /></td></tr></table>

ports GroupingGalleries/GroupingNoTemplates.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🔴 Sonnet 5 Review

C1/C3: MAUI reference renders BLANK in both themes (suspected broken MAUI ref — real MAUI without templates should show item ToString; flag per ruling 3), while C++ renders a flat ungrouped list of all hero names (Thor…Doorman). Cannot be reconciled against a blank ground truth; needs MAUI recapture/ruling.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 76. Grouping Plus Selection — 🔴/⏳
<sub>grouping_plus_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/grouping_plus_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/grouping_plus_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/grouping_plus_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/grouping_plus_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/grouping_plus_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/grouping_plus_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/grouping_plus_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/grouping_plus_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/grouping_plus_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/grouping_plus_selection_dark.png" /></td></tr></table>

ports CollectionViewGalleries/GroupingGalleries/ GroupingPlusSelection.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🔴 Sonnet 5 Review

C1/C3: MAUI reference is BLANK in both themes (suspected broken MAUI ref capture — flag per ruling 3), while C++ shows the full grouped list (green group headers Avengers/Fantastic Four/Defenders/Heroes for Hire/West Coast Avengers/Great Lakes Avengers, member rows, orange 'Total members: N' footers). Blank ground truth cannot validate the render; needs MAUI recapture/ruling.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 77. Header Footer — 🟡/⏳
<sub>header_footer</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/header_footer_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/header_footer_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/header_footer_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/header_footer_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/header_footer_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/header_footer_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/header_footer_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/header_footer_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/header_footer_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/header_footer_dark.png" /></td></tr></table>

ports HeaderFooterString.xaml (+ HeaderFooterString.xaml.cs)

#### 🟡 Sonnet 5 Review

C1/C3: bold string header 'Just a string as a header', 3 items and bold footer 'This footer is also a string' all present in both themes; rows are tighter than MAUI (items span ~90px vs MAUI ~130px). Minor internal-spacing diff only.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 78. Header Footer Grid — 🟡/⏳
<sub>header_footer_grid</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/header_footer_grid_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/header_footer_grid_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/header_footer_grid_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/header_footer_grid_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/header_footer_grid_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/header_footer_grid_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/header_footer_grid_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/header_footer_grid_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/header_footer_grid_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/header_footer_grid_dark.png" /></td></tr></table>

ports HeaderFooterGrid.xaml (+ HeaderFooterGrid.xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟡 Sonnet 5 Review

C1/C3: Toggle Header/Footer links, header image with 'This Is A Header' + Add Content, 3-column grid (10 items) and footer image + Add Content all present with correct theming (header/footer label faint in light, bright in dark). Diffs: the rotated 'This Is A Footer' label sits ABOVE the footer image (colliding with the last grid row) instead of below it as in MAUI, and grid row spacing is tighter (grid ends ~y435 vs MAUI ~y465). Minor layout offsets.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 79. Header Footer Grid Horizontal — 🟡/⏳
<sub>header_footer_grid_horizontal</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/header_footer_grid_horizontal_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/header_footer_grid_horizontal_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/header_footer_grid_horizontal_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/header_footer_grid_horizontal_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/header_footer_grid_horizontal_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/header_footer_grid_horizontal_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/header_footer_grid_horizontal_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/header_footer_grid_horizontal_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/header_footer_grid_horizontal_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/header_footer_grid_horizontal_dark.png" /></td></tr></table>

ports HeaderFooterGridHorizontal.xaml (+ HeaderFooterGridHorizontal.xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟡 Sonnet 5 Review

MAUI's light/dark captures are scrolled to show only the first two rows of the grid (cover1.jpg/oasis.jpg row and Vegetables/Fruits/Legumes row), missing the third row (photo.jpg, FlowerBuds.jpg, oasis.jpg) and the footer/second 'Add Content' link that cpp renders. All visible rows and the header match well (text, layout, header image). No confirmed divergence in the parts that can be compared, but the comparison is incomplete due to MAUI's capture cropping.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 80. Header Footer Template — 🔴/⏳
<sub>header_footer_template</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/header_footer_template_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/header_footer_template_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/header_footer_template_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/header_footer_template_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/header_footer_template_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/header_footer_template_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/header_footer_template_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/header_footer_template_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/header_footer_template_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/header_footer_template_dark.png" /></td></tr></table>

ports HeaderFooterTemplate.xaml (+ .xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🔴 Sonnet 5 Review

MAUI's templated cells render as plain blue rectangles containing only centered text (e.g. 'cover1.jpg, 0') with no thumbnail image, in both light and dark. The cpp build additionally renders a small photo thumbnail ABOVE each blue rectangle that does not exist in the MAUI reference — extra visual content not present in ground truth. This is a genuine content/template divergence (extra image elements), present in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 81. Header Footer View — 🟡/⏳
<sub>header_footer_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/header_footer_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/header_footer_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/header_footer_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/header_footer_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/header_footer_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/header_footer_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/header_footer_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/header_footer_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/header_footer_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/header_footer_view_dark.png" /></td></tr></table>

ports HeaderFooterView.xaml (+ .xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟡 Sonnet 5 Review

MAUI's own light/dark captures are truncated/scrolled to show only the header image and header text ('This Is A Header'), missing the footer text, collection view, and 'Add 2 Items'/'Clear All Items' buttons that both cpp and xaml render further down. For the content that IS visible in the MAUI capture (header image, header text placement/color), cpp matches exactly. Cannot fully verify the footer/list section against MAUI since the MAUI capture doesn't show it, so classifying as a minor/incomplete-comparison rather than a confirmed bug.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 82. Hit Testing — 🟢/⏳
<sub>hit_testing</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/hit_testing_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/hit_testing_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/hit_testing_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/hit_testing_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/hit_testing_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/hit_testing_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/hit_testing_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/hit_testing_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/hit_testing_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/hit_testing_dark.png" /></td></tr></table>

ports HitTestingPage.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.HitTestingPage)

#### 🟢 Sonnet 5 Review

All four comparisons match closely: shapes (ellipse, rounded rectangle), text labels (Scale=1/2, Rotation=20), submarine 3D model, and 'Lorem ipsum' text all align in position, size, color, and font weight in both light and dark themes. Only trivial anti-aliasing differences.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 83. Horizontal Stack — 🟡/⏳
<sub>horizontal_stack</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/horizontal_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/horizontal_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/horizontal_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/horizontal_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/horizontal_stack_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/horizontal_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/horizontal_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/horizontal_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/horizontal_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/horizontal_stack_dark.png" /></td></tr></table>

Horizontal Stack

#### 🟡 Sonnet 5 Review

C1/C3 yellow: the six colored squares match MAUI in size, colors, spacing and vertical position, but the whole row is indented ~230px from the left edge whereas MAUI (and the xaml column) place it flush left; cpp also shows a 'HorizontalStackLayout' title label MAUI's crop hides (exempt). Same in dark.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 84. Horizontal Stack Layout — ⬛/⏳
<sub>horizontal_stack_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/horizontal_stack_layout_light.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/horizontal_stack_layout_light.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/horizontal_stack_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/horizontal_stack_layout_dark.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/horizontal_stack_layout_dark.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/horizontal_stack_layout_dark.png" /></td></tr></table>

#### ⬛ C++ Sonnet 5 Review

No code-first builder twin exists for this page (manifest.json builder_twin:false) — the gallery app's MAUI_SAMPLE_PAGE dispatch has no entry for this key. e2e.py cmd_capture skips the cpp framework for builder_twin:false keys instead of capturing a misleading value_controls fallback-page screenshot. Only the C++&amp;XAML column applies to this page.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 85. Hybrid Web View — 🔴/⏳
<sub>hybrid_web_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/hybrid_web_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/hybrid_web_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/hybrid_web_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/hybrid_web_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/hybrid_web_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/hybrid_web_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/hybrid_web_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/hybrid_web_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/hybrid_web_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/hybrid_web_view_dark.png" /></td></tr></table>

ports HybridWebViewPage.xaml (+ HybridWebViewPage.xaml.cs)

#### 🔴 Sonnet 5 Review

C1/C3 red: the HybridWebView surface is missing entirely in the C++ build — MAUI renders a large light-gray webview area with the 'HybridWebView (HybridRoot=HybridSamplePage)' placeholder text in both themes, but the C++ capture shows only the header row (label + 5 link buttons) over an empty white region (light) / a white void below the dark header (dark). Major missing content.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 86. Image — 🟢/⏳
<sub>image</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/image_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/image_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/image_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/image_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/image_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/image_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/image_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/image_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/image_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/image_dark.png" /></td></tr></table>

ports ImagePage.xaml (+ ImagePage.xaml.cs)

#### 🟢 Sonnet 5 Review

C1/C3 green. UriSource (Microsoft building) and FileSource (dotnet-bot submarine on purple) both load with matching sizes, placement and colors in both themes; only the exempt uniform chrome-height shift.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 87. Image Button — 🟢/⏳
<sub>image_button</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/image_button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/image_button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/image_button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/image_button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/image_button_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/image_button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/image_button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/image_button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/image_button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/image_button_dark.png" /></td></tr></table>

ports ImageButtonPage.xaml (+ ImageButtonPage.xaml.cs) A self-contained, code-first demo page for the ImageButton control (the C# gallery-page convention, mirroring the input_controls_page / image_page pattern)

#### 🟢 Sonnet 5 Review

MAUI's capture is scrolled slightly further down (missing the top 'N ImageButton clicks' line) but all remaining visible content — AspectFit/AspectFill/Fill green boxes, red-bordered box, purple corner-radius bars + sliders, submarine custom-size image, green padding bar — matches cpp precisely in position, size, and color, in both light and dark.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 88. Indicator — 🟡/⏳
<sub>indicator</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/indicator_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/indicator_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/indicator_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/indicator_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/indicator_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/indicator_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/indicator_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/indicator_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/indicator_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/indicator_dark.png" /></td></tr></table>

ports IndicatorPage.xaml A self-contained, code-first demo of the IndicatorView control

#### 🟡 Sonnet 5 Review

C1/C3 yellow with a maui_quirk flag: the MAUI reference renders NO indicator content at all in either theme — just the 8 section labels on a blank page (no dots, no yellow Colors band, no carousel). The C++ build renders what the page describes: dot rows for Basic/Shape/Size/MaximumVisible, the yellow Colors band with colored dots, and the 'Item 1' carousel with its indicator. Strictly vs MAUI this is extra content, but the MAUI ref looks like a broken IndicatorView render on Catalyst — needs a user ruling before treating as a port bug.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 89. Input Controls — 🟡/⏳
<sub>input_controls</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/input_controls_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/input_controls_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/input_controls_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/input_controls_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/input_controls_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/input_controls_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/input_controls_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/input_controls_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/input_controls_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/input_controls_dark.png" /></td></tr></table>

a self-contained demo page for the W1-05 input-control set: editor, search_bar, radio_button (+ the radio_button_group attached grouping) and image_button on one vertical stack, wired together so every input drives a visible output (the C#

#### 🟡 Sonnet 5 Review

C1/C3 yellow: page structure matches exactly (LENGTH: 0, 'Type here...' entry, 'Search to insert' search bar, UPPER/lower radio group) in both themes; only visible diff is the radio-button glyph weight — MAUI draws a noticeably thicker ring with a larger filled center dot, the port draws a thinner ring with a smaller dot. Minor cosmetic.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 90. Input Transparent — 🟢/⏳
<sub>input_transparent</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/input_transparent_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/input_transparent_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/input_transparent_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/input_transparent_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/input_transparent_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/input_transparent_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/input_transparent_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/input_transparent_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/input_transparent_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/input_transparent_dark.png" /></td></tr></table>

ports InputTransparentPage.xaml (Maui.Controls.Sample.Pages.InputTransparentPage)

#### 🟢 Sonnet 5 Review

C1/C3 green: all four sections, blue link buttons, the toggle switch, status text, and even the same overlapping-label rendering of the stacked overlay buttons ('Bottom (clickable)/Top (transparent)' and 'Test Button/Bottom layer' superimposed) match MAUI in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 91. Invalidate Brush — 🟡/⏳
<sub>invalidate_brush</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/invalidate_brush_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/invalidate_brush_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/invalidate_brush_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/invalidate_brush_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/invalidate_brush_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/invalidate_brush_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/invalidate_brush_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/invalidate_brush_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/invalidate_brush_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/invalidate_brush_dark.png" /></td></tr></table>

ports InvalidateBrushGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/InvalidateBrushGallery.xaml (&amp;quot;Invalidate Brushes Playground&amp;quot;): a VerticalStackLayout (Padding 12) with — - a &amp;quot;Change color&amp;quot; Bu

#### 🟡 Sonnet 5 Review

C1/C3: green GraphicsView line renders flush-left directly under the Change color button instead of horizontally centered near the top as in MAUI (both themes). Line color/size and button/label content match.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 92. Invalidate Shadow Host — 🟢/⏳
<sub>invalidate_shadow_host</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/invalidate_shadow_host_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/invalidate_shadow_host_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/invalidate_shadow_host_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/invalidate_shadow_host_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/invalidate_shadow_host_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/invalidate_shadow_host_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/invalidate_shadow_host_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/invalidate_shadow_host_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/invalidate_shadow_host_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/invalidate_shadow_host_dark.png" /></td></tr></table>

ports InvalidateShadowHostPage.xaml A self-contained, code-first demo that a shadow re-applies (invalidates) when its host&amp;#x27;s size changes, mirroring the C# core gallery page (Pages/Core/ShadowGalleries/InvalidateShadowHostPage.xaml + .xaml.

#### 🟢 Sonnet 5 Review

C1/C3: MAUI vs C++ (light+dark) match — 'Update Host Size' link, shadow sliders (offset X/Y=10, radius=10, opacity=1.00), and the green-bordered host box (white fill) all identical in both themes. Notification banner artifact in corner exempt.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 93. Ios Blur Effect — 🟢/⏳
<sub>ios_blur_effect</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/ios_blur_effect_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_blur_effect_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_blur_effect_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_blur_effect_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_blur_effect_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/ios_blur_effect_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_blur_effect_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_blur_effect_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_blur_effect_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_blur_effect_dark.png" /></td></tr></table>

ports iOSBlurEffectPage.xaml The .NET MAUI PlatformSpecifics sample (Pages/PlatformSpecifics/iOS/iOSBlurEffectPage.xaml + .xaml.cs): an Image (Source=&amp;quot;oasis.jpg&amp;quot;) carrying the iOSSpecific VisualElement.BlurEffect knob (XAML seeds it to Extr

#### 🟢 Sonnet 5 Review

C1/C3 green. Pug photo (no blur applied, ExtraLight state), the four blue blur links and the 'BlurEffect: ExtraLight' label all match MAUI in both themes; only the exempt uniform chrome shift.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 94. Ios Date Picker — 🟢/⏳
<sub>ios_date_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/ios_date_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_date_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_date_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_date_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_date_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/ios_date_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_date_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_date_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_date_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_date_picker_dark.png" /></td></tr></table>

ports iOSDatePickerPage.xaml (+ iOSDatePickerPage.xaml.cs)

#### 🟢 Sonnet 5 Review

C1/C3: matches MAUI in both themes — '31.12.2020' label top-left and centered 'Toggle DatePicker UpdateMode' link.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 95. Ios Entry — 🟢/⏳
<sub>ios_entry</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/ios_entry_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_entry_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_entry_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_entry_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_entry_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/ios_entry_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_entry_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_entry_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_entry_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_entry_dark.png" /></td></tr></table>

ports iOSEntryPage.xaml (+ iOSEntryPage.xaml.cs)

#### 🟢 Sonnet 5 Review

C1/C3: matches MAUI in both themes — full-width entry with 'Enter text here to see the font size change' placeholder and the centered toggle link; dark-mode field fill matches.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 96. Ios First Responder — 🟢/⏳
<sub>ios_first_responder</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/ios_first_responder_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_first_responder_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_first_responder_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_first_responder_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_first_responder_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/ios_first_responder_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_first_responder_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_first_responder_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_first_responder_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_first_responder_dark.png" /></td></tr></table>

ports iOSFirstResponderPage.xaml (+ .xaml.cs) The C# iOSFirstResponderPage is a VisualElement-first-responder demo: a StackLayout with an explanatory Label, a &amp;quot;First Entry&amp;quot; + plain &amp;quot;OK&amp;quot; Button (tapping OK dismisses the keyboard because the

#### 🟢 Sonnet 5 Review

C1/C3: matches MAUI in both themes — two instruction labels, two entries, two OK links, Focus First/Focus Second links, and the three IsFocused/CanBecomeFirstResponder status lines all identical.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 97. Ios Pan Gesture — 🟡/⏳
<sub>ios_pan_gesture</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/ios_pan_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_pan_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_pan_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_pan_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_pan_gesture_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/ios_pan_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_pan_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_pan_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_pan_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_pan_gesture_dark.png" /></td></tr></table>

ports iOSPanGestureRecognizerPage.xaml (+ .xaml.cs) The C# iOSPanGestureRecognizerPage is a StackLayout with: a bold message Label (_messageLabel), a &amp;quot;Toggle Simultaneous Gesture Recognition&amp;quot; Button, and a grouped ListView of employees whos

#### 🟡 Sonnet 5 Review

C1/C3: layout, link and status labels match, but the header label reads 'panned x:45 y:-12' instead of MAUI's 'Pan the target. If you pan it, this Label will change.' — the capture ran after a pan interaction (runtime-state divergence, not a rendering bug). Both themes otherwise identical.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 98. Ios Picker — 🟢/⏳
<sub>ios_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/ios_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/ios_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_picker_dark.png" /></td></tr></table>

ports iOSPickerPage.xaml (+ iOSPickerPage.xaml.cs)

#### 🟢 Sonnet 5 Review

C1/C3: full-width 'Select a monkey' picker field and centered 'Toggle Picker UpdateMode' link match MAUI in both light and dark (dark shows the same black full-width field strip). Only the exempt chrome offset differs.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 99. Ios Safe Area — 🟢/⏳
<sub>ios_safe_area</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/ios_safe_area_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_safe_area_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_safe_area_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_safe_area_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_safe_area_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/ios_safe_area_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_safe_area_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_safe_area_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_safe_area_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_safe_area_dark.png" /></td></tr></table>

ports iOSSafeAreaPage.xaml The .NET MAUI PlatformSpecifics sample (Pages/PlatformSpecifics/iOS/iOSSafeAreaPage.xaml + .xaml.cs): a long Lorem-ipsum Label over a &amp;quot;Disable Use Safe Area&amp;quot; button

#### 🟢 Sonnet 5 Review

C1/C3: lorem-ipsum paragraph wraps identically and the centered 'Disable Use Safe Area' link matches in both themes; only the exempt window-chrome shift differs.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 100. Ios Scroll View — 🟡/⏳
<sub>ios_scroll_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/ios_scroll_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_scroll_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_scroll_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_scroll_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_scroll_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/ios_scroll_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_scroll_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_scroll_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_scroll_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_scroll_view_dark.png" /></td></tr></table>

ports iOSScrollViewPage.xaml (+ iOSScrollViewPage.xaml.cs)

#### 🟡 Sonnet 5 Review

C1/C3: slider position/colors and both blue links match, but the cpp gallery shows a stray floating sidebar-toggle button in the top-left corner (both themes) that MAUI does not render — extra harness chrome overlapping the page area.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 101. Ios Search Bar — 🟢/⏳
<sub>ios_search_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/ios_search_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_search_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_search_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_search_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_search_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/ios_search_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_search_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_search_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_search_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_search_bar_dark.png" /></td></tr></table>

ports iOSSearchBarPage.xaml (+ iOSSearchBarPage.xaml.cs)

#### 🟢 Sonnet 5 Review

C1/C3: rounded search field with magnifier icon and 'Enter search term' placeholder plus the two toggle links all match MAUI in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 102. Ios Slider Update On Tap — 🟢/⏳
<sub>ios_slider_update_on_tap</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/ios_slider_update_on_tap_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_slider_update_on_tap_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_slider_update_on_tap_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_slider_update_on_tap_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_slider_update_on_tap_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/ios_slider_update_on_tap_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_slider_update_on_tap_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_slider_update_on_tap_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_slider_update_on_tap_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_slider_update_on_tap_dark.png" /></td></tr></table>

ports iOSSliderUpdateOnTapPage.xaml (+ iOSSliderUpdateOnTapPage.xaml.cs)

#### 🟢 Sonnet 5 Review

C1/C3: instruction label, slider at minimum with thumb at far left, and 'Toggle Update on Tap' link all match MAUI in both themes; only the exempt chrome offset differs.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 103. Ios Swipe Transition — 🟢/⏳
<sub>ios_swipe_transition</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/ios_swipe_transition_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_swipe_transition_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_swipe_transition_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_swipe_transition_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_swipe_transition_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/ios_swipe_transition_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_swipe_transition_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_swipe_transition_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_swipe_transition_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_swipe_transition_dark.png" /></td></tr></table>

ports iOSSwipeViewTransitionModePage.xaml (+ .xaml.cs) The C# iOSSwipeViewTransitionModePage is a StackLayout with: a horizontal row holding a &amp;quot;SwipeTransitionMode:&amp;quot; Label + an EnumPicker over the SwipeTransitionMode enum (Reveal / Drag, Se

#### 🟢 Sonnet 5 Review

C1/C3 green. Reveal/Drag links, the gray 'Swipe right' swipe cell (white text on gray in dark, matching MAUI), and both caption lines ('Swipe right to reveal Delete...', 'SwipeTransitionMode: Drag') match in both themes; only the exempt chrome shift.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 104. Ios Time Picker — 🟢/⏳
<sub>ios_time_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/ios_time_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_time_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_time_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_time_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_time_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/ios_time_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_time_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_time_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_time_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_time_picker_dark.png" /></td></tr></table>

ports iOSTimePickerPage.xaml The .NET MAUI PlatformSpecifics sample (Pages/PlatformSpecifics/iOS/iOSTimePickerPage.xaml + .xaml.cs): a TimePicker carrying the iOSSpecific TimePicker.UpdateMode knob (XAML seeds it to WhenFinished) over a but

#### 🟢 Sonnet 5 Review

C1/C3: matches — '14:00' label, blue 'Toggle TimePicker UpdateMode' link, and 'UpdateMode: WhenFinished' text identical in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 105. Items — 🟢/⏳
<sub>items</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/items_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/items_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/items_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/items_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/items_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/items_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/items_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/items_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/items_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/items_dark.png" /></td></tr></table>

a self-contained demo page for the W2-19 items core: a collection_view over a live observable items source with a templated cell, single selection driving a readout label, and an EmptyView for the cleared state (the C# CollectionView galler

#### 🟢 Sonnet 5 Review

C1/C3: matches — bold 'Today' header, three task rows, and 'Pick a task' label identical in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 106. Items Updating Scroll Mode — 🟢/⏳
<sub>items_updating_scroll_mode</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/items_updating_scroll_mode_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/items_updating_scroll_mode_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/items_updating_scroll_mode_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/items_updating_scroll_mode_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/items_updating_scroll_mode_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/items_updating_scroll_mode_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/items_updating_scroll_mode_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/items_updating_scroll_mode_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/items_updating_scroll_mode_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/items_updating_scroll_mode_dark.png" /></td></tr></table>

ports ItemsUpdatingScrollModeGallery.xaml (+ .xaml.cs) of the C# CollectionView gallery (Maui.Controls.Sample.Pages.CollectionViewGalleries.ScrollModeGalleries.ItemsUpdatingScrollModeGallery)

#### 🟢 Sonnet 5 Review

C1/C3: matches — mode links, Add Item, status line, and 50-row list render at the same size and density as MAUI in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 107. Label — 🔴/⏳
<sub>label</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/label_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/label_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/label_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/label_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/label_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/label_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/label_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/label_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/label_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/label_dark.png" /></td></tr></table>

ports LabelPage.xaml (+ LabelPage.xaml.cs)

#### 🔴 Sonnet 5 Review

C1/C3: two content bugs in both themes — (1) the trailing 'Plain old Text' formatted-string run loses its blue color (renders default black/white); (2) the eight lorem LineBreakMode paragraphs render truncated single-line variants (head/middle/tail ellipsis) where MAUI wraps all of them to two lines. Top alignment/background sections match.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 108. Layout Is Enabled — 🟡/⏳
<sub>layout_is_enabled</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/layout_is_enabled_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/layout_is_enabled_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/layout_is_enabled_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/layout_is_enabled_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/layout_is_enabled_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/layout_is_enabled_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/layout_is_enabled_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/layout_is_enabled_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/layout_is_enabled_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/layout_is_enabled_dark.png" /></td></tr></table>

ports LayoutIsEnabledPage.xaml (+ LayoutIsEnabledPage.xaml.cs) The C# page demonstrates how IsEnabled on a layout cascades to its children: a 2x2 grid whose left column hosts a &amp;quot;MainLayout&amp;quot; full of state-demo sub-stacks (all-enabled / all-d

#### 🟡 Sonnet 5 Review

C1/C3: colored sections and enabled/disabled states all match, but the right-column radio group is laid out differently in both themes — cpp stacks the three 'Enable/Disable ...' labels together with the three radios clustered below, while MAUI interleaves each label with its radio.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 109. Line Gallery — 🟢/⏳
<sub>line_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/line_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/line_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/line_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/line_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/line_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/line_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/line_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/line_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/line_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/line_gallery_dark.png" /></td></tr></table>

ports LineGallery.xaml A self-contained, code-first port of the MAUI Shapes LineGallery (Pages/Controls/ShapesGalleries/LineGallery.xaml + .xaml.cs)

#### 🟢 Sonnet 5 Review

C1/C3: all three lines (purple basic, orange dashed, thick black StrokeThickness) match MAUI in color, angle, position and labels in both themes; black stroke correctly stays black in dark mode.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 110. Line Join Gallery — 🟢/⏳
<sub>line_join_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/line_join_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/line_join_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/line_join_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/line_join_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/line_join_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/line_join_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/line_join_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/line_join_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/line_join_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/line_join_gallery_dark.png" /></td></tr></table>

ports LineJoinGallery.xaml A self-contained, code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/LineJoinGallery.xaml: a StackLayout that demonstrates the three StrokeLineJoin variants on an identical open polyline

#### 🟢 Sonnet 5 Review

C1/C3: Miter/Bevel/Round cyan zig-zags match MAUI exactly in shape, join rendering, color and labels in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 111. Measure First Strategy — 🔴/⏳
<sub>measure_first_strategy</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/measure_first_strategy_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/measure_first_strategy_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/measure_first_strategy_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/measure_first_strategy_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/measure_first_strategy_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/measure_first_strategy_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/measure_first_strategy_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/measure_first_strategy_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/measure_first_strategy_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/measure_first_strategy_dark.png" /></td></tr></table>

ports MeasureFirstStrategy.xaml (+ .xaml.cs) of the C# CollectionView gallery (Maui.Controls.Sample.Pages.CollectionViewGalleries.GroupingGalleries.MeasureFirstStrategy)

#### 🔴 Sonnet 5 Review

C1/C3: header/link/strategy label match, but the C++ render shows the full CollectionView content (Avengers/Fantastic Four/... groups with colored headers and totals) while the MAUI reference renders an empty list below the labels in both themes. Content presence differs vs the MAUI ground truth (likely a MAUI-side MeasureFirstItem known-issue blanking the list — flag for ruling; the port arguably renders the intended content).

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 112. Menu Bar — 🟢/⏳
<sub>menu_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/menu_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/menu_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/menu_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/menu_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/menu_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/menu_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/menu_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/menu_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/menu_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/menu_bar_dark.png" /></td></tr></table>

ports MenuBarPage.xaml (+ MenuBarPage.cs) The C# page declares three page-level MenuBarItems (Page.MenuBarItems — the app menu bar) and a small visible body: - &amp;quot;Before File&amp;quot; : &amp;quot;Before File Action&amp;quot; (accelerator &amp;quot;b&amp;quot;), &amp;quot;Cool item 1&amp;quot;, a separat

#### 🟢 Sonnet 5 Review

C1/C3: label 'You clicked on Menu Item:' and blue 'Toggle Menu Bar Item' link match MAUI in both themes; only the exempt outer inset differs (cpp label sits at the window edge).

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 113. Modal — 🟢/⏳
<sub>modal</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/modal_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/modal_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/modal_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/modal_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/modal_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/modal_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/modal_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/modal_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/modal_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/modal_dark.png" /></td></tr></table>

ports ModalPage.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 Review

C1/C3: 'Modal Page 1', four blue push links, disabled gray 'Pop Modal Page', and the depth status line all match MAUI in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 114. Multiple Bound Selection — 🟡/⏳
<sub>multiple_bound_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/multiple_bound_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/multiple_bound_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/multiple_bound_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/multiple_bound_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/multiple_bound_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/multiple_bound_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/multiple_bound_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/multiple_bound_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/multiple_bound_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/multiple_bound_selection_dark.png" /></td></tr></table>

ports MultipleBoundSelection.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.MultipleBoundSelection)

#### 🟡 Sonnet 5 Review

C1/C3: text, header, items and three blue buttons match, but the C++ render draws a gray full-width selection highlight over Item 1/Item 2 in both themes while the MAUI reference shows no selection highlight (even though its label says Item 1, Item 2 are selected — likely a MAUI-side capture quirk of not painting selected cells). Visual difference vs ground truth, minor.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 115. Navigation Gallery — 🟢/⏳
<sub>navigation_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/navigation_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/navigation_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/navigation_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/navigation_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/navigation_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/navigation_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/navigation_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/navigation_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/navigation_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/navigation_gallery_dark.png" /></td></tr></table>

ports NavigationGallery.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 Review

C1/C3 match: status line (Stack depth: 1 | top: PAGE NUMBER 1 | secondary toolbar items: 0) and all six blue action links identical in text, color, spacing and centering in both themes; only exempt window-chrome/title differences.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 116. Nested Collection — 🔴/⏳
<sub>nested_collection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/nested_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/nested_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/nested_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/nested_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/nested_collection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/nested_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/nested_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/nested_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/nested_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/nested_collection_dark.png" /></td></tr></table>

ports NestedGalleries/NestedCollectionViewGallery.xaml (+ NestedCollectionViewGallery.xaml.cs) of the C# CollectionView gallery

#### 🔴 Sonnet 5 Review

C1/C3: MAUI reference renders only the header label ('It's CollectionViews all the way down.') with an EMPTY body in both themes, while cpp renders full nested content (red 'Source N' rotated labels + rows of blue 'Caption N-M' links). Content diverges from the MAUI ground truth in both themes. NOTE: the MAUI ref itself looks like a broken/blank CollectionView-in-CollectionView render — likely a MAUI-side quirk needing a user ruling (parity policy rule 3); cpp's output is plausibly the intended content.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 117. Pan Gesture Events — 🟡/⏳
<sub>pan_gesture_events</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/pan_gesture_events_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/pan_gesture_events_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/pan_gesture_events_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/pan_gesture_events_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/pan_gesture_events_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/pan_gesture_events_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/pan_gesture_events_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/pan_gesture_events_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/pan_gesture_events_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/pan_gesture_events_dark.png" /></td></tr></table>

ports PanGestureEventsGallery.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.PanGestureEventsGallery)

#### 🟡 Sonnet 5 Review

C1/C3: green/red half panels and layout match exactly in both themes, but the status readout differs — MAUI shows 'StatusType: Completed, TotalX: 12, TotalY: -8' while cpp shows 'TotalX: 0, TotalY: 0'. The injected pan completes but reports zero deltas in the cpp builder app (xaml reports 12/-8 correctly), suggesting a pan-gesture total-delta reporting gap in the cpp capture path.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 118. Path Aspect Gallery — 🔴/⏳
<sub>path_aspect_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/path_aspect_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/path_aspect_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/path_aspect_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/path_aspect_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/path_aspect_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/path_aspect_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/path_aspect_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/path_aspect_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/path_aspect_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/path_aspect_gallery_dark.png" /></td></tr></table>

ports PathAspectGallery.xaml A self-contained, code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PathAspectGallery.xaml: a StackLayout (Padding 12) that demonstrates the four Path Aspect modes on one identical ge

#### 🔴 Sonnet 5 Review

C1/C3: cpp renders a completely different path than MAUI in both themes — heart-with-ECG shapes (and the 'None' cell shows a tiny unscaled heart in the corner) versus MAUI's red octagon filling each gray box under None/Fill/Uniform/UniformToFill. Wrong geometry content, major diff.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 119. Path Gallery — 🔴/⏳
<sub>path_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/path_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/path_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/path_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/path_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/path_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/path_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/path_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/path_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/path_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/path_gallery_dark.png" /></td></tr></table>

ports PathGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PathGallery.xaml: a ScrollView over a StackLayout (Padding 12) that walks eight Path variants (plus two caption-only markup-string Labels

#### 🔴 Sonnet 5 Review

C1/C3: cpp diverges heavily from the MAUI reference in both themes: 'Composite shape' renders as concentric lavender rings vs MAUI's plain filled circle; 'Overlapping Rectangles' as two overlapping even-odd squares vs MAUI's single solid red square; 'EllipseGeometry' as four overlapping orange circles vs MAUI's one circle; 'Multiple Line Segments' star is filled cyan vs MAUI's unfilled dark-red outline; shapes are left-aligned vs MAUI's horizontally centered. (cpp arguably renders the doc-intended geometry — possible MAUI-side quirk worth a ruling — but vs ground truth this is a major mismatch.)

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 120. Path Transform String — 🟡/⏳
<sub>path_transform_string</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/path_transform_string_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/path_transform_string_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/path_transform_string_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/path_transform_string_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/path_transform_string_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/path_transform_string_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/path_transform_string_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/path_transform_string_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/path_transform_string_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/path_transform_string_dark.png" /></td></tr></table>

ports PathTransformStringGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PathTransformStringGallery.xaml: a ScrollView over a StackLayout (Padding 12) that shows the SAME two-figure Path geometry

#### 🟡 Sonnet 5 Review

C1/C3: both flag paths render with correct shape and the skew transform is applied, but the 'Without RenderTransform' flag is noticeably smaller in cpp (~25% narrower/shorter) than MAUI's in both themes; the transformed flag matches. Scale/measure diff on the untransformed path.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 121. Picker — 🔴/⏳
<sub>picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/picker_dark.png" /></td></tr></table>

ports PickerPage.xaml (+ PickerPage.xaml.cs) A self-contained, code-first demo page for the Picker control (the C# gallery-page convention, mirroring the value_controls_page / pickers_page pattern)

#### 🔴 Sonnet 5 Review

C1/C3: cpp shows selected text where MAUI shows placeholders — 'Item 2' in the SelectedIndex=1 and SelectedIndexChanged pickers and white 'Item 1' on the green markup picker; MAUI (ground truth) renders all three with empty/placeholder text. Layout, colors and the yellow/green backgrounds otherwise match. Possible MAUI-Catalyst quirk (preselected text not rendered), but per ruling 1 MAUI is ground truth.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 122. Pickers — 🟡/⏳
<sub>pickers</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/pickers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/pickers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/pickers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/pickers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/pickers_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/pickers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/pickers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/pickers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/pickers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/pickers_dark.png" /></td></tr></table>

a self-contained demo page for the W1-06 picker set: picker, date_picker and time_picker on one vertical stack, wired together so every selection drives a visible output (the C# gallery-page convention, code-first; the value_controls_page p

#### 🟡 Sonnet 5 Review

C1/C3: summary label differs — cpp shows 'No room on 7/5/2026 at 09:00' while MAUI shows 'No room on (no date) at (no time)' (cpp eagerly propagates default date/time to the bound label). Picker row, date '5.07.2026' and time '09:00' otherwise match in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 123. Pointer Gesture — 🟡/⏳
<sub>pointer_gesture</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/pointer_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/pointer_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/pointer_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/pointer_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/pointer_gesture_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/pointer_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/pointer_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/pointer_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/pointer_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/pointer_gesture_dark.png" /></td></tr></table>

ports PointerGestureGalleryPage.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.PointerGestureGalleryPage)

#### 🟡 Sonnet 5 Review

C1/C3: Content and text match MAUI in both themes, but the three section titles ('Hover, press, and release me!', 'Hover me!', 'Hover me green!') render as large bold headline-style text in C++, whereas MAUI renders them as plain small text at the same size/weight as the description line below. This is a real font-size/weight content difference in both light and dark. cpp verdict = yellow.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 124. Polygon Gallery — 🟢/⏳
<sub>polygon_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/polygon_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/polygon_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/polygon_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/polygon_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/polygon_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/polygon_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/polygon_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/polygon_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/polygon_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/polygon_gallery_dark.png" /></td></tr></table>

ports PolygonGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PolygonGallery.xaml: a ScrollView over a StackLayout (Padding 12) that walks four Polygon variants, each under a caption Label — - &amp;quot;A

#### 🟢 Sonnet 5 Review

C1/C3: all four shapes match in both themes — green triangle, dashed green triangle, EvenOdd red/blue star, NonZero yellow/black star; identical geometry, colors, dash pattern and positions.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 125. Polyline Gallery — 🟢/⏳
<sub>polyline_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/polyline_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/polyline_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/polyline_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/polyline_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/polyline_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/polyline_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/polyline_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/polyline_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/polyline_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/polyline_gallery_dark.png" /></td></tr></table>

ports PolylineGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PolylineGallery.xaml: a StackLayout (Padding 12 — no ScrollView in the C# source) holding two Polyline variants, each under a caption

#### 🟢 Sonnet 5 Review

C1/C3: both themes match — red ECG-style basic polyline and dotted dash polyline with identical geometry, stroke color and dash pattern.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 126. Preselected Item — 🟡/⏳
<sub>preselected_item</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/preselected_item_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/preselected_item_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/preselected_item_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/preselected_item_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/preselected_item_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/preselected_item_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/preselected_item_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/preselected_item_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/preselected_item_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/preselected_item_dark.png" /></td></tr></table>

ports PreselectedItemGallery.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.PreselectedItemGallery)

#### 🟡 Sonnet 5 Review

C1/C3: all 50 rows + header + preselected label present, but cpp draws a gray full-width selection band on 'photo.jpg, 2' that the MAUI capture does not show, and row spacing is much tighter (~26px vs MAUI's ~44px row pitch). Same in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 127. Preselected Items — 🟡/⏳
<sub>preselected_items</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/preselected_items_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/preselected_items_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/preselected_items_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/preselected_items_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/preselected_items_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/preselected_items_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/preselected_items_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/preselected_items_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/preselected_items_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/preselected_items_dark.png" /></td></tr></table>

ports PreselectedItemsGallery.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.PreselectedItemsGallery)

#### 🟡 Sonnet 5 Review

C1/C3: 4-column grid content matches, but cpp shows gray selection bands on the three preselected items (photo.jpg 2, Fruits.jpg 4, FlowerBuds.jpg 5) that the MAUI capture does not render, and row pitch is tighter (~26px vs ~44px). Both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 128. Progress Bar — 🟡/⏳
<sub>progress_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/progress_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/progress_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/progress_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/progress_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/progress_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/progress_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/progress_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/progress_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/progress_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/progress_bar_dark.png" /></td></tr></table>

ports ProgressBarPage.xaml (+ ProgressBarPage.xaml.cs)

#### 🟡 Sonnet 5 Review

C1/C3: bars, colors (blue/orange), 50% fill, disabled row, and ProgressTo link all match in both themes, but the 4th label 'ProgressColor' is rendered bold/large in cpp where MAUI shows it small/regular weight.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 129. Radio Button Border — 🟡/⏳
<sub>radio_button_border</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/radio_button_border_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_button_border_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_button_border_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_button_border_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_button_border_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/radio_button_border_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_button_border_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_button_border_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_button_border_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_button_border_dark.png" /></td></tr></table>

ports RadioButtonBorder.xaml A self-contained, code-first demo of RadioButton border styling

#### 🟡 Sonnet 5 Review

C1/C3: option rows, red/green borders, yellow backgrounds, and dark-mode white-on-yellow text all match, but cpp renders the 'RadioButton with Border' title in regular weight where MAUI has it bold, and row heights/vertical spacing are tighter than MAUI in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 130. Radio Button Content — 🟢/⏳
<sub>radio_button_content</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/radio_button_content_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_button_content_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_button_content_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_button_content_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_button_content_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/radio_button_content_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_button_content_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_button_content_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_button_content_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_button_content_dark.png" /></td></tr></table>

ports RadioButtonContentGallery.xaml A self-contained, code-first demo of the RadioButton.Content surface

#### 🟢 Sonnet 5 Review

C1/C3: all radio rows, bordered ControlTemplate box and the custom-template black/red underline group match MAUI in both themes; only trivial spacing/AA differences.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 131. Radio Button Group — 🟡/⏳
<sub>radio_button_group</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/radio_button_group_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_button_group_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_button_group_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_button_group_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_button_group_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/radio_button_group_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_button_group_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_button_group_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_button_group_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_button_group_dark.png" /></td></tr></table>

ports RadioButtonGroupGallery.xaml A self-contained, code-first demo of the RadioButtonGroup ATTACHED-PROPERTY grouping: a vertical StackLayout carries RadioButtonGroup.GroupName=&amp;quot;foo&amp;quot;, so every descendant RadioButton — including one nested

#### 🟡 Sonnet 5 Review

C1/C3: content matches in both themes (Options A-C, grid row with Option D, all text), but cpp's vertical spacing between option rows is slightly tighter than MAUI.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 132. Radio Button Group Binding — 🟢/⏳
<sub>radio_button_group_binding</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/radio_button_group_binding_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_button_group_binding_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_button_group_binding_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_button_group_binding_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_button_group_binding_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/radio_button_group_binding_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_button_group_binding_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_button_group_binding_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_button_group_binding_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_button_group_binding_dark.png" /></td></tr></table>

ports RadioButtonGroupBindingGallery.xaml A code-first demo of binding the RadioButtonGroup attached properties (GroupName + SelectedValue) to a view-model

#### 🟢 Sonnet 5 Review

C1/C3: matches MAUI in both themes — two-column radio layout (A/C left, B/D right), '(null)' selection label, and both blue action links match.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 133. Radio Button Group Gallery — 🟢/⏳
<sub>radio_button_group_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/radio_button_group_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_button_group_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_button_group_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_button_group_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_button_group_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/radio_button_group_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_button_group_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_button_group_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_button_group_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_button_group_gallery_dark.png" /></td></tr></table>

ports RadioButtonGroupGalleryPage.xaml A self-contained, code-first demo of RadioButton grouping SCOPE, mirroring the C# controls gallery page (Pages/Controls/RadioButtonGalleries/RadioButtonGroupGalleryPage.xaml)

#### 🟢 Sonnet 5 Review

C1/C3: all three radio groups, labels, and 'Selected: (none)' lines match MAUI exactly in both themes; only the exempt outer-inset differs.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 134. Radio Content Properties — 🟢/⏳
<sub>radio_content_properties</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/radio_content_properties_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_content_properties_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_content_properties_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_content_properties_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_content_properties_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/radio_content_properties_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_content_properties_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_content_properties_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_content_properties_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_content_properties_dark.png" /></td></tr></table>

ports ContentProperties.xaml A self-contained, code-first demo of how RadioButton propagates the standard Text/Font properties to its Content

#### 🟢 Sonnet 5 Review

C1/C3: red italic Option A, blue bold OPTION B (uppercase), and all five green bold button-content radios render with matching colors, fonts, and spacing in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 135. Radio Template From Style — 🟢/⏳
<sub>radio_template_from_style</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/radio_template_from_style_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_template_from_style_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_template_from_style_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_template_from_style_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_template_from_style_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/radio_template_from_style_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_template_from_style_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_template_from_style_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_template_from_style_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_template_from_style_dark.png" /></td></tr></table>

ports TemplateFromStyle.xaml A self-contained, code-first demo of applying a RadioButton ControlTemplate

#### 🟢 Sonnet 5 Review

C1 (light): MAUI and cpp both show three radio cards (A, B, C) with light-gray background boxes and blue-outlined unselected radio circles in the top-right of each, correctly Start-aligned (narrow width) at the left of the window. C3 (dark): same layout correctly re-themed — dark background, light card boxes retained, matching radio circle style/position. Content, spacing and alignment all match.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 136. Rectangle Gallery — 🟡/⏳
<sub>rectangle_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/rectangle_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/rectangle_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/rectangle_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/rectangle_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/rectangle_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/rectangle_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/rectangle_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/rectangle_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/rectangle_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/rectangle_gallery_dark.png" /></td></tr></table>

ports RectangleGallery.xaml A self-contained, code-first port of the MAUI Shapes RectangleGallery (Pages/Controls/ShapesGalleries/RectangleGallery.xaml + .xaml.cs)

#### 🟡 Sonnet 5 Review

C1 (light): shape content, colors, strokes, and dash patterns for all five rectangle variants (basic, square, stroke, stroke-dash, curved-corners) match MAUI exactly — but MAUI renders the whole column left-aligned (Start) near x~20-240px, while cpp renders the identical column shifted right and centered around x~890-1110px in the window. This is a real horizontal-position/alignment regression (HorizontalOptions=Start not honored for this page's container), not a uniform outer-inset shift, since the shapes are ~750px further right rather than a consistent small margin. C3 (dark): same centering-vs-Start-alignment discrepancy persists identically. Collapsed verdict = yellow (content/shapes fully correct, but position wrong in both themes).

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 137. Refresh View — 🟢/⏳
<sub>refresh_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/refresh_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/refresh_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/refresh_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/refresh_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/refresh_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/refresh_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/refresh_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/refresh_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/refresh_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/refresh_view_dark.png" /></td></tr></table>

ports RefreshViewPage.xaml (+ RefreshViewPage.xaml.cs + RefreshViewModel.cs) A self-contained, code-first demo page for the RefreshView control (the C# gallery-page convention, mirroring the swipe_refresh_page pattern)

#### 🟢 Sonnet 5 Review

C1/C3: header text, four blue link-style buttons, Is Refreshing/Is Enabled state lines, and '50 items loaded' all match MAUI in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 138. Relative Layout — 🟢/⏳
<sub>relative_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/relative_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/relative_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/relative_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/relative_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/relative_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/relative_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/relative_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/relative_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/relative_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/relative_layout_dark.png" /></td></tr></table>

ports RelativeLayoutPage.xaml

#### 🟢 Sonnet 5 Review

C1/C3: four corner squares (red TL, green TR, blue BL, yellow BR), gray panel, and black inner box all positioned and colored as in MAUI in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 139. Scattered Radio Button — 🟡/⏳
<sub>scattered_radio_button</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/scattered_radio_button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/scattered_radio_button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/scattered_radio_button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/scattered_radio_button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/scattered_radio_button_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/scattered_radio_button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/scattered_radio_button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/scattered_radio_button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/scattered_radio_button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/scattered_radio_button_dark.png" /></td></tr></table>

ports ScatteredRadioButtonGallery.xaml A code-first demo that radio buttons DON&amp;#x27;T have to share a container to be grouped: grouping is by GroupName, so buttons scattered across separate containers (and one bare button outside any grouped co

#### 🟡 Sonnet 5 Review

C1/C3: all content present (labels, A/B/C strip, D radio) and the dark-theme faint white-on-light strip artifact is reproduced identically; but the label block is vertically tighter than MAUI (line spacing between the three intro labels and around the strip is compressed) — minor spacing diff.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 140. Scroll Mode Test — 🟡/⏳
<sub>scroll_mode_test</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/scroll_mode_test_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/scroll_mode_test_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/scroll_mode_test_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/scroll_mode_test_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/scroll_mode_test_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/scroll_mode_test_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/scroll_mode_test_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/scroll_mode_test_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/scroll_mode_test_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/scroll_mode_test_dark.png" /></td></tr></table>

ports ScrollModeTestGallery.xaml (+ .xaml.cs) of the C# CollectionView gallery (Maui.Controls.Sample.Pages.CollectionViewGalleries.ScrollModeGalleries.ScrollModeTestGallery)

#### 🟡 Sonnet 5 Review

C1/C3: layout, links and 20-item list match, but the cpp Entry displays 'KeepItemsInView' where MAUI's entry is empty, and list row pitch is tighter (~26px vs ~44px). Both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 141. Scroll To Group — 🔴/⏳
<sub>scroll_to_group</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/scroll_to_group_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/scroll_to_group_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/scroll_to_group_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/scroll_to_group_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/scroll_to_group_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/scroll_to_group_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/scroll_to_group_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/scroll_to_group_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/scroll_to_group_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/scroll_to_group_dark.png" /></td></tr></table>

ports ScrollToGalleries/ScrollToGroup.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🔴 Sonnet 5 Review

C1/C3: cpp renders the full grouped CollectionView (Avengers/Fantastic Four/Defenders... with green headers and orange footers) while the MAUI reference shows NO list at all below 'No scroll requested yet' in both themes — major content mismatch vs the ground truth. Likely a MAUI-side empty-grouped-list capture quirk (needs a ruling), but per the binding rules this diverges from the reference.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 142. Scroll View — 🟡/⏳
<sub>scroll_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/scroll_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/scroll_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/scroll_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/scroll_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/scroll_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/scroll_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/scroll_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/scroll_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/scroll_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/scroll_view_dark.png" /></td></tr></table>

ports ScrollViewPage.xaml (+ the ScrollViewPages sub-demos: ScrollViewOrientationPage / ScrollToEndPage / ScrollToFromConstructorPage), code-first

#### 🟡 Sonnet 5 Review

C1/C3: rows, spacing and colors match MAUI in both themes; only diff is the status label reads 'Scrolled to: 0 / 0 (done)' vs MAUI's 'Scrolled to: 0 / 0' — extra '(done)' suffix text.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 143. Search Bar — 🟢/⏳
<sub>search_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/search_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/search_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/search_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/search_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/search_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/search_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/search_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/search_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/search_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/search_bar_dark.png" /></td></tr></table>

ports SearchBarPage.xaml (Microsoft.Maui.Controls sample gallery)

#### 🟢 Sonnet 5 Review

C1/C3: all seven search bars match MAUI in both themes — placeholder vs text states, italic 24pt field, right-aligned 'end of the line', clear buttons and field chrome all reproduce; only the exempt chrome inset differs.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 144. Selection Command Param — 🔴/⏳
<sub>selection_command_param</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/selection_command_param_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/selection_command_param_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/selection_command_param_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/selection_command_param_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/selection_command_param_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/selection_command_param_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/selection_command_param_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/selection_command_param_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/selection_command_param_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/selection_command_param_dark.png" /></td></tr></table>

ports SelectionChangedCommandParameter.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.SelectionChangedCommandParameter)

#### 🔴 Sonnet 5 Review

C1/C3: cell template layout is broken — each item renders as two stacked lines ('Item N' / 'This is item N') with tight spacing, where MAUI renders a single line 'Item N — This is item N' (em-dash joined) with taller rows. Same in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 145. Selection Synchronization — 🟡/⏳
<sub>selection_synchronization</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/selection_synchronization_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/selection_synchronization_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/selection_synchronization_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/selection_synchronization_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/selection_synchronization_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/selection_synchronization_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/selection_synchronization_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/selection_synchronization_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/selection_synchronization_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/selection_synchronization_dark.png" /></td></tr></table>

ports SelectionSynchronization.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.SelectionSynchronization)

#### 🟡 Sonnet 5 Review

C1/C3: text, list layout and structure match, but the C++ port renders gray selection-highlight bars on the selected rows (Item 2/3 in the first two CollectionViews, Item 2 in the third) in both themes, while the MAUI reference shows no visible selection highlight at all. Extra selection chrome vs ground truth (arguably a MAUI Catalyst render quirk, but MAUI is binding).

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 146. Semantics — 🟢/⏳
<sub>semantics</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/semantics_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/semantics_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/semantics_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/semantics_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/semantics_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/semantics_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/semantics_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/semantics_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/semantics_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/semantics_dark.png" /></td></tr></table>

ports SemanticsPage.xaml (+ SemanticsPage.xaml.cs) The C# SemanticsPage is an accessibility showcase: a long VerticalStackLayout where nearly every control carries SemanticProperties.Description / .Hint, plus a block of labels exercising Se

#### 🟢 Sonnet 5 Review

C1/C3: all labels, buttons, entry/editor/search bar, heading list and focus link match MAUI in both themes, including the black entry field and dark search bar in dark mode. Only the exempt uniform outer offset differs.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 147. Shadow Playground — 🟢/⏳
<sub>shadow_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/shadow_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/shadow_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/shadow_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/shadow_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/shadow_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/shadow_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/shadow_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/shadow_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/shadow_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/shadow_playground_dark.png" /></td></tr></table>

ports ShadowPlaygroundPage.xaml A self-contained, code-first demo of the view Shadow surface, mirroring the C# core gallery page (Pages/Core/ShadowGalleries/ShadowPlaygroundPage.xaml + .xaml.cs)

#### 🟢 Sonnet 5 Review

C1/C3: MAUI vs C++ (light+dark) pixel-identical — label, blue rect with red shadow, background/shadow-color fields, all four sliders (offset X/Y, radius, opacity) at matching positions, 'Remove Shadow' link. Notification banner in C++ dark/light corner is an OS artifact, exempt.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 148. Shape App Theme — 🟡/⏳
<sub>shape_app_theme</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/shape_app_theme_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/shape_app_theme_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/shape_app_theme_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/shape_app_theme_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/shape_app_theme_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/shape_app_theme_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/shape_app_theme_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/shape_app_theme_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/shape_app_theme_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/shape_app_theme_dark.png" /></td></tr></table>

ports ShapeAppThemeGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/ShapeAppThemeGallery.xaml: a StackLayout (Padding 12) holding a caption Label and a 200x80 Rectangle, all themed via {AppThemeBi

#### 🟡 Sonnet 5 Review

C1 (light) green: MAUI/cpp both show a green rectangle labeled 'Shape using AppTheme', matching. C3 (dark) red: MAUI shows the shape switched to RED via AppThemeBinding, but cpp still renders GREEN (the light-theme color) — AppThemeBinding on the shape's Fill is not re-evaluated for dark theme in the cpp build. Collapsed cpp verdict = worst(C1 green, C3 red) = yellow (content/color is wrong in dark, but structurally identical and an isolated single-property miss).

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 149. Shapes — 🟢/⏳
<sub>shapes</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/shapes_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/shapes_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/shapes_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/shapes_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/shapes_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/shapes_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/shapes_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/shapes_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/shapes_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/shapes_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;shapes&amp;quot; demo (ComparePages.Shapes()), the shipped-.NET-MAUI reference for the visual-parity comparison: a ScrollView over a vertical stack of four LABELLED shapes, each bold-captioned and Start-a

#### 🟢 Sonnet 5 Review

C1 (light): MAUI and cpp both show the four shape sections — Ellipse (red fill, blue stroke), RoundRectangle (solid navy), EvenOdd Polygon pentagram (blue fill, red stroke, correct even-odd hole), and Line (purple diagonal) — same sizes, colors, and left alignment. C3 (dark): identical content correctly re-themed with dark background and light section labels; shapes' own colors unchanged (correct, since shape fills are explicit not theme-bound here).

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 150. Single Bound Selection — 🟢/⏳
<sub>single_bound_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/single_bound_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/single_bound_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/single_bound_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/single_bound_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/single_bound_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/single_bound_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/single_bound_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/single_bound_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/single_bound_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/single_bound_selection_dark.png" /></td></tr></table>

ports SingleBoundSelection.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.SingleBoundSelection)

#### 🟢 Sonnet 5 Review

C1 (light) and C3 (dark): C++ matches MAUI - identical instructional text, 'Selected: (none)' label, Reset/Clear links, and the 5-country CollectionView list (United States/Canada/Mexico/Brazil/Argentina), correct dark-theme text/background inversion.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 151. Slider — 🟢/⏳
<sub>slider</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/slider_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/slider_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/slider_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/slider_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/slider_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/slider_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/slider_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/slider_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/slider_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/slider_dark.png" /></td></tr></table>

ports SliderPage.xaml (+ .xaml.cs) Mirrors the MAUI gallery page: a vertical stack of headlined Slider states — Default, BackgroundColor (Blue), Background (yellow→green LinearGradientBrush), Minimum(5)/Maximum(15) with a value readout (Val

#### 🟢 Sonnet 5 Review

C1/C3: all slider variants (default, background color, min/max range, disabled, custom track/thumb colors, image thumb, custom multi-color slider, dynamic update) render identically to MAUI in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 152. Some Empty Groups — 🔴/⏳
<sub>some_empty_groups</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/some_empty_groups_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/some_empty_groups_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/some_empty_groups_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/some_empty_groups_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/some_empty_groups_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/some_empty_groups_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/some_empty_groups_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/some_empty_groups_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/some_empty_groups_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/some_empty_groups_dark.png" /></td></tr></table>

ports GroupingGalleries/SomeEmptyGroups.xaml (+ .xaml.cs)

#### 🔴 Sonnet 5 Review

C1/C3: MAUI ground truth shows ONLY the intro label with an empty CollectionView area (both themes), while the C++ build renders a full grouped list (Avengers/Thundercats/Bionic Six/Fantastic Four with green headers and orange footers). Content present in C++ that MAUI does not render = divergence per ruling 1. NOTE: the MAUI reference itself looks like a broken/empty CollectionView render (ruling-3 maui_quirk candidate — the page description says groups SHOULD display headers/footers); recommend a user ruling before treating the C++ render as the bug.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 153. Stack Layout — 🟢/⏳
<sub>stack_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/stack_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/stack_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/stack_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/stack_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/stack_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/stack_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/stack_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/stack_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/stack_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/stack_layout_dark.png" /></td></tr></table>

ports StackLayoutPage.xaml Demonstrates the generic maui::controls::stack_layout (the orientation-switching sibling of the fixed vertical/horizontal stacks) by nesting two inner stacks inside an outer vertical stack with a 12px margin: a &amp;quot;V

#### 🟢 Sonnet 5 Review

C1/C3: vertical 6-square rainbow column and horizontal row match MAUI in size, colors, spacing and placement in both themes; only the exempt uniform chrome offset differs.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 154. Staggered Layout — 🔴/⏳
<sub>staggered_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/staggered_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/staggered_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/staggered_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/staggered_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/staggered_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/staggered_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/staggered_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/staggered_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/staggered_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/staggered_layout_dark.png" /></td></tr></table>

ports AlternateLayoutGalleries/StaggeredLayout.xaml (+ StaggeredLayout.xaml.cs) of the C# CollectionView gallery

#### 🔴 Sonnet 5 Review

C1/C3: broken row spacing in both themes — MAUI lays the 24 items in 8 compact rows (~25pt apart, Items 0-23 all visible at the top), but the C++ build spaces rows ~220pt apart so only Items 0-20 fit on screen and Items 21-23 are pushed below the fold. Text/columns themselves are correct.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 155. Stepper — 🟢/⏳
<sub>stepper</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/stepper_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/stepper_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/stepper_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/stepper_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/stepper_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/stepper_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/stepper_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/stepper_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/stepper_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/stepper_dark.png" /></td></tr></table>

ports StepperPage.xaml (+ StepperPage.xaml.cs)

#### 🟢 Sonnet 5 Review

C1/C3: all seven labeled steppers, the 'Enable Stepper' link, full-width red BackgroundColor bar behind the stepper, and 'Value: 0' match MAUI in both themes. The old red-bg compact-not-fullwidth bug is gone. Only the exempt uniform chrome-height shift differs.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 156. Styles — 🟢/⏳
<sub>styles</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/styles_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/styles_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/styles_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/styles_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/styles_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/styles_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/styles_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/styles_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/styles_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/styles_dark.png" /></td></tr></table>

ports StylesPage.xaml

#### 🟢 Sonnet 5 Review

C1/C3: gray base-subtitle label, pink derived-style label, default-styled label, and the 'Style Me' button with light-gray fill and yellow border all match MAUI in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 157. Swipe Gesture — 🟡/⏳
<sub>swipe_gesture</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/swipe_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_gesture_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/swipe_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_gesture_dark.png" /></td></tr></table>

ports SwipeViewGestureRecognizerGallery.xaml (+ .xaml.cs) The MAUI SwipeViewGestureRecognizerGallery is a CollectionView of &amp;quot;message&amp;quot; rows; each row&amp;#x27;s DataTemplate is a SwipeView wired three ways, proving gesture recognizers AND swipe-item

#### 🟡 Sonnet 5 Review

C1/C3: status label reads 'TapCommand (double-tap)' instead of MAUI's 'Ready (double-tap row / swipe to favourite or delete)' in both themes — a tap fired during capture, so the dynamic status text diverges; all other content (banner, card header, sub-labels, dark-mode colors) matches.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 158. Swipe Item Position — 🟡/⏳
<sub>swipe_item_position</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/swipe_item_position_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_item_position_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_item_position_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_item_position_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_item_position_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/swipe_item_position_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_item_position_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_item_position_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_item_position_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_item_position_dark.png" /></td></tr></table>

ports SwipeItemPositionGallery.xaml A code-first port of the MAUI SwipeView sub-gallery Pages/Controls/SwipeViewGalleries/SwipeItemPositionGallery.xaml: a 2-row Grid (Auto / *) with a Picker on top and one SwipeView below that carries TWO S

#### 🟡 Sonnet 5 Review

C1/C3: picker shows a selected value 'Reveal' where MAUI shows the empty 'Select a Mode' placeholder in both themes (capture-time selection state); rest of the page (label, gray swipe area in dark) matches.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 159. Swipe Item Size — 🟢/⏳
<sub>swipe_item_size</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/swipe_item_size_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_item_size_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_item_size_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_item_size_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_item_size_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/swipe_item_size_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_item_size_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_item_size_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_item_size_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_item_size_dark.png" /></td></tr></table>

ports SwipeItemSizeGallery.xaml A self-contained, code-first port of the .NET MAUI &amp;quot;SwipeItem Size Gallery&amp;quot;: a scrolling stack of swipe_views demonstrating how a left SwipeItem&amp;#x27;s icon size and the SwipeView content size interact

#### 🟢 Sonnet 5 Review

C1/C3 match: all six SwipeView rows (3 icon-size rows, 3 height rows 128/256/512) present with identical gray fills, centered 'Swipe to Left' labels, row heights and label text in both light and dark; dark theme correctly keeps light-gray row fills with light label text like MAUI. Only the exempt window-chrome inset differs.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 160. Swipe Refresh — 🟢/⏳
<sub>swipe_refresh</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/swipe_refresh_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_refresh_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_refresh_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_refresh_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_refresh_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/swipe_refresh_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_refresh_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_refresh_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_refresh_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_refresh_dark.png" /></td></tr></table>

a self-contained demo page for the W2-20 swipe + refresh controls: a refresh_view wrapping a swipe_view (which itself wraps a labeled row), with a readout label reflecting the latest interaction

#### 🟢 Sonnet 5 Review

C1/C3: matches MAUI in both themes — 'Swipe left to delete, pull to refresh' and 'Ready' render identically on white/dark backgrounds.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 161. Swipe Threshold — 🟢/⏳
<sub>swipe_threshold</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/swipe_threshold_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_threshold_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_threshold_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_threshold_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_threshold_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/swipe_threshold_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_threshold_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_threshold_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_threshold_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_threshold_dark.png" /></td></tr></table>

ports HorizontalSwipeThresholdGallery.xaml (+ .xaml.cs) The MAUI HorizontalSwipeThresholdGallery shows how SwipeView.Threshold (the swipe distance, in DIPs, the user must drag before the items settle open / execute) interacts with SwipeItem

#### 🟢 Sonnet 5 Review

C1/C3: matches MAUI in both themes — black notice banner, four labeled sections, both custom-threshold sliders at the same positions, identical indigo swipe blocks, and the 'Reveal threshold=80 / Execute threshold=80' footer.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 162. Swipe View Margin — 🟡/⏳
<sub>swipe_view_margin</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/swipe_view_margin_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_view_margin_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_view_margin_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_view_margin_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_view_margin_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/swipe_view_margin_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_view_margin_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_view_margin_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_view_margin_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_view_margin_dark.png" /></td></tr></table>

ports SwipeViewMarginGallery.xaml A self-contained, code-first port of the .NET MAUI &amp;quot;SwipeView Margin Gallery&amp;quot;: two swipe_views whose content&amp;#x27;s Margin + Padding are driven by two sliders, demonstrating that the revealed SwipeItems stay cor

#### 🟡 Sonnet 5 Review

C1/C3: status label reads 'Horizontal items revealed' instead of MAUI's 'Adjust the sliders, then open a row to verify item positioning' in both themes (a swipe fired during capture); banner, sliders, and both SwipeItems panels otherwise match.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 163. Swipe View Shadow — 🟡/⏳
<sub>swipe_view_shadow</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/swipe_view_shadow_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_view_shadow_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_view_shadow_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_view_shadow_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_view_shadow_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/swipe_view_shadow_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_view_shadow_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_view_shadow_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_view_shadow_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_view_shadow_dark.png" /></td></tr></table>

ports SwipeViewShadowGallery.xaml A code-first port of the MAUI SwipeView sub-gallery Pages/Controls/SwipeViewGalleries/SwipeViewShadowGallery.xaml: a padded vertical StackLayout proving a drop Shadow renders correctly on SwipeView content

#### 🟡 Sonnet 5 Review

C1: light-theme SwipeView content boxes render with a gray gradient/shadow wash inside them where MAUI shows flat white interiors — minor cosmetic shadow-rendering diff. C3: dark theme matches (boxes, borders, centered 'Content' labels all align).

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 164. Switch — 🟢/⏳
<sub>switch</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/switch_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/switch_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/switch_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/switch_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/switch_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/switch_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/switch_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/switch_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/switch_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/switch_dark.png" /></td></tr></table>

ports SwitchPage.xaml (+ .xaml.cs) Mirrors the MAUI gallery page: a vertical stack of headlined Switch states — Default, BackgroundColor (Blue), Background (a yellow→green LinearGradientBrush), Disabled, OnColor (Red), ThumbColor (Orange)

#### 🟢 Sonnet 5 Review

All switch rows (Default, BackgroundColor [blue track], Background, Disabled [dimmed], OnColor, ThumbColor) match MAUI exactly in position, on/off state, track color, and thumb appearance, in both light and dark themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 165. Switch Grouping — 🔴/⏳
<sub>switch_grouping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/switch_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/switch_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/switch_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/switch_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/switch_grouping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/switch_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/switch_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/switch_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/switch_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/switch_grouping_dark.png" /></td></tr></table>

ports CollectionViewGalleries/GroupingGalleries/ SwitchGrouping.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🔴 Sonnet 5 Review

C1/C3: C++ renders the full grouped hero list (Avengers/Fantastic Four/... with green headers and orange footers) below the 'Is Grouped' switch, but the MAUI reference capture shows only the switch with an empty page in both themes. Content structurally differs from ground truth (likely a MAUI-side blank-CollectionView capture quirk worth a user ruling, but per policy MAUI is ground truth).

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 166. Tabbed Flyout — 🔴/⏳
<sub>tabbed_flyout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/tabbed_flyout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/tabbed_flyout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/tabbed_flyout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/tabbed_flyout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/tabbed_flyout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/tabbed_flyout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/tabbed_flyout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/tabbed_flyout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/tabbed_flyout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/tabbed_flyout_dark.png" /></td></tr></table>

a self-contained demo page for the W1-10 tabbed + flyout vertical: a flyout_page whose FLYOUT pane is a titled menu (two buttons selecting the detail&amp;#x27;s tabs + a &amp;quot;Toggle flyout&amp;quot; presenting/dismissing itself) and whose DETAIL pane is a tabbed

#### 🔴 Sonnet 5 Review

C1/C3: completely different page structure. MAUI shows 'Home tab'/'Settings tab'/'Toggle flyout' link buttons plus 'Flyout dismissed' and 'This is the Home tab.' labels; C++ instead renders a native segmented Home/Settings tab bar at the window top, the 'This is the Home tab.' label is clipped off the top edge, and the three action links and 'Flyout dismissed' text are missing. Both themes affected.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 167. Templated View — 🟢/⏳
<sub>templated_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/templated_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/templated_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/templated_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/templated_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/templated_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/templated_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/templated_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/templated_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/templated_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/templated_view_dark.png" /></td></tr></table>

ports TemplatedViewPage.xaml The C# page contrasts a standard CardView control with a compact one driven by a ControlTemplate (&amp;quot;CardViewCompressed&amp;quot;) and a custom Rate control built entirely from a ControlTemplate + a heart PathGeometry

#### 🟢 Sonnet 5 Review

C1/C3: matches MAUI in both themes — red italic section headers, standard CardView (Slavko Vlasic + lorem), and three compact cards with gray thumbnail, bold titles and body text all align.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 168. Time Picker — 🟢/⏳
<sub>time_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/time_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/time_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/time_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/time_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/time_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/time_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/time_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/time_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/time_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/time_picker_dark.png" /></td></tr></table>

ports TimePickerPage.xaml (+ TimePickerPage.xaml.cs)

#### 🟢 Sonnet 5 Review

Fixed: 'Default with date'/'Default with time' now carries the shared XAML's restored Date=06/21/2018 / Time=4:15:26 attribute, matching MAUI exactly in both themes; all other rows already matched.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 169. Title Bar — 🟡/⏳
<sub>title_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/title_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/title_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/title_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/title_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/title_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/title_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/title_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/title_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/title_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/title_bar_dark.png" /></td></tr></table>

ports TitleBarPage.xaml A self-contained, code-first demo of the TitleBar control

#### 🟡 Sonnet 5 Review

C1/C3: all content present (radio list, Title/Subtitle entries, Color Options links, status label) in both themes, but the radio rows differ: MAUI renders circle+label inline in compact rows while cpp puts the label offset above-right of the circle and roughly doubles the row spacing, stretching the Content Options list. Minor cosmetic layout drift only.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 170. Toolbar — 🟢/⏳
<sub>toolbar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/toolbar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/toolbar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/toolbar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/toolbar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/toolbar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/toolbar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/toolbar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/toolbar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/toolbar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/toolbar_dark.png" /></td></tr></table>

ports ToolbarPage.xaml (Maui.Controls.Sample.Pages.ToolbarPage)

#### 🟢 Sonnet 5 Review

C1/C3 match: 'You clicked on ToolbarItem: {none}' status line plus all six centered blue link-buttons (Enable/Disable Test (1), two Enable/Disable Secondary, Change text on Test Secondary (1), Remove/Add Secondary (3), Change Command Property on Secondary (3)) with identical text, color, spacing and centering in light and dark. Only the exempt chrome inset differs.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 171. Transform Playground — 🟢/⏳
<sub>transform_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/transform_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/transform_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/transform_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/transform_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/transform_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/transform_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/transform_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/transform_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/transform_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/transform_playground_dark.png" /></td></tr></table>

ports TransformPlaygroundGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/TransformPlaygroundGallery.xaml: a 50x50 Path rectangle (red fill, blue stroke 4) sits in a 200x200 light-grey panel; belo

#### 🟢 Sonnet 5 Review

C1 (light): MAUI and cpp both show identical layout — red/blue-bordered square in top area, gray canvas, full slider stack (RotateTransform/ScaleTransform/SkewTransform/TranslateTransform) with matching values and slider thumb positions. C3 (dark): same content correctly re-themed to dark background with light text and matching slider positions. No content differences found.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 172. Transformations — 🟢/⏳
<sub>transformations</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/transformations_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/transformations_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/transformations_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/transformations_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/transformations_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/transformations_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/transformations_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/transformations_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/transformations_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/transformations_dark.png" /></td></tr></table>

ports TransformationsPage.xaml (+ .xaml.cs) The MAUI TransformationsPage drives a single target view&amp;#x27;s render transforms from a column of knobs: Sliders for Scale / ScaleX / ScaleY (Maximum 10) and Rotation / RotationX / RotationY (Maximum

#### 🟢 Sonnet 5 Review

C1/C3: SCALE AND ROTATE link, all 8 sliders with identical thumb positions/fill, and the AnchorX/AnchorY stepper pairs match MAUI in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 173. Triggers — 🟢/⏳
<sub>triggers</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/triggers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/triggers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/triggers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/triggers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/triggers_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/triggers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/triggers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/triggers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/triggers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/triggers_dark.png" /></td></tr></table>

ports TriggersPage.xaml

#### 🟢 Sonnet 5 Review

C1/C3: 'Triggers' header, description, placeholder entry, 'Highlight off' label and 'Toggle highlight' link all match MAUI in both themes, including the dark-theme black entry field.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 174. Update Path Data — 🔴/⏳
<sub>update_path_data</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/update_path_data_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/update_path_data_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/update_path_data_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/update_path_data_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/update_path_data_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/update_path_data_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/update_path_data_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/update_path_data_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/update_path_data_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/update_path_data_dark.png" /></td></tr></table>

ports UpdatePathDataGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/UpdatePathDataGallery.xaml: a 2-row Grid (RowSpacing 0) that proves a Path repaints when its Data geometry is replaced at runti

#### 🔴 Sonnet 5 Review

C1/C3: the path 'M 10,100 C 10,300 300,-200 300,100' renders as a smooth cubic-Bezier S-curve in cpp, but MAUI renders it as straight zig-zag segments through the control points — wrong path geometry in both themes (major content difference). Status label and Update Path Data button match.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 175. Value Controls — 🟢/⏳
<sub>value_controls</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/value_controls_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/value_controls_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/value_controls_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/value_controls_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/value_controls_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/value_controls_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/value_controls_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/value_controls_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/value_controls_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/value_controls_dark.png" /></td></tr></table>

#### 🟢 Sonnet 5 Review

C1/C3: slider (25%), stepper, progress bar, switch and activity-indicator ring all match MAUI in size, color and position in both themes; only the exempt window-chrome offset differs.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 176. Varied Size Selector — 🔴/⏳
<sub>varied_size_selector</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/varied_size_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/varied_size_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/varied_size_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/varied_size_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/varied_size_selector_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/varied_size_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/varied_size_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/varied_size_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/varied_size_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/varied_size_selector_dark.png" /></td></tr></table>

ports DataTemplateSelectorGalleries/VariedSizeDataTemplateSelectorGallery.xaml (+ VariedSizeDataTemplateSelectorGallery.xaml.cs) of the C# CollectionView gallery

#### 🔴 Sonnet 5 Review

C1/C3: MAUI renders six uniform tall wheat rows and an empty 'Select a template' picker; cpp renders alternating dark-brown/cream rows with varied (compressed) heights — Coffee2/Coffee3 merged with no gap — and the picker pre-filled with 'Latte'. Wrong item colors, wrong row sizing and wrong picker state in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 177. Vertical Stack — 🟡/⏳
<sub>vertical_stack</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/vertical_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/vertical_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/vertical_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/vertical_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/vertical_stack_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/vertical_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/vertical_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/vertical_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/vertical_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/vertical_stack_dark.png" /></td></tr></table>

Vertical Stack

#### 🟡 Sonnet 5 Review

C1/C3: all six colored squares present with correct colors and sizes, but cpp stacks them with zero spacing while MAUI has clear gaps between squares (both themes).

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 178. Vertical Stack Layout — ⬛/⏳
<sub>vertical_stack_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/vertical_stack_layout_light.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/vertical_stack_layout_light.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/vertical_stack_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/vertical_stack_layout_dark.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/vertical_stack_layout_dark.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/vertical_stack_layout_dark.png" /></td></tr></table>

#### ⬛ C++ Sonnet 5 Review

No code-first builder twin exists for this page (manifest.json builder_twin:false) — the gallery app's MAUI_SAMPLE_PAGE dispatch has no entry for this key. e2e.py cmd_capture skips the cpp framework for builder_twin:false keys instead of capturing a misleading value_controls fallback-page screenshot. Only the C++&amp;XAML column applies to this page.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 179. Visual States — 🟢/⏳
<sub>visual_states</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/visual_states_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/visual_states_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/visual_states_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/visual_states_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/visual_states_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/visual_states_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/visual_states_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/visual_states_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/visual_states_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/visual_states_dark.png" /></td></tr></table>

ports VisualStatesPage.xaml

#### 🟢 Sonnet 5 Review

C1/C3: green VSM entry, disabled 2nd entry with placeholder, and both state-change buttons match MAUI in both themes; text and colors identical.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 180. Web View — 🟡/⏳
<sub>web_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/web_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/web_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/web_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/web_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/web_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/web_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/web_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/web_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/web_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/web_view_dark.png" /></td></tr></table>

a self-contained demo page for the W1-08 web_view vertical: a web_view loading a STATIC html_web_view_source (no network), back/forward/reload buttons over the handler-pushed CanGoBack/CanGoForward read-onlys, an &amp;quot;Eval 1+1&amp;quot; button driving t

#### 🟡 Sonnet 5 Review

C1/C3: all content present (status labels, Page A/B/Back/Forward/Reload/Eval buttons) and the web page actually renders ('Welcome' heading), but the WebView region is collapsed to its content height so everything below sits ~270px higher than in MAUI, which reserves a tall webview area (blank white in light, white band in dark). Layout/height of the webview differs in both themes.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 181. Z Index — 🟢/⏳
<sub>z_index</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/z_index_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/z_index_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/z_index_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/z_index_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/z_index_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="../../../maui-reference/captures/maccatalyst/z_index_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/z_index_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/z_index_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/z_index_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/z_index_dark.png" /></td></tr></table>

ports ZIndexPage.xaml (+ ZIndexPage.xaml.cs), code-first

#### 🟢 Sonnet 5 Review

C1/C3 match: identical diagonal stack of 10 colored z-index labels (mint/orange/purple/red/green/blue/... with the big red z-index-9 block on top), same stepper row 'Z-Index of Label 5: 5' with -/+ segmented control, same label text colors (black in light, white in dark), same sizes and overlap order in both themes. Only the exempt window-chrome vertical offset differs.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

</details>

<details>
<summary><h2>Android (181 examples) — click to expand</h2></summary>

Real .NET MAUI vs the C++ port vs the compile-time-XAML gallery, captured on the same Android emulator. Android is captured single-theme, so the Dark row is a placeholder.

**Discrepancy counts** (MAUI-vs-C++ parity verdicts; Sonnet 5 `claude-sonnet-5` and Gemini review each page independently):

| Classification | Sonnet 5 | Gemini |
| --- | --- | --- |
| 🟢 Match | 145 | 0 |
| 🟡 Minor | 15 | 0 |
| 🔴 Major | 12 | 0 |
| ⬛ Blank | 0 | 0 |
| ⏳ Unreviewed | 9 | 181 |

### 1. Absolute Layout — 🟢/⏳
<sub>absolute_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/absolute_layout_light.png" /></td><td><img width="300px" src="captures/android/cpp/absolute_layout_light.png" /></td><td><img width="300px" src="captures/android/xaml/absolute_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports AbsoluteLayoutPage.xaml A self-contained, code-first demo of the AbsoluteLayout control

#### 🟢 Sonnet 5 Review

Layout, colors, and text positions of all elements match exactly between MAUI and cpp.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 2. Activity Indicator — 🟢/⏳
<sub>activity_indicator</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/activity_indicator_light.png" /></td><td><img width="300px" src="captures/android/cpp/activity_indicator_light.png" /></td><td><img width="300px" src="captures/android/xaml/activity_indicator_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ActivityIndicatorPage.xaml (+ ActivityIndicatorPage.xaml.cs)

#### 🟢 Sonnet 5 Review

All activity indicator styles (default, themed color, yellow background, larger, smaller) match MAUI in size, color, and position.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 3. Adaptive Collection — 🟢/⏳
<sub>adaptive_collection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/adaptive_collection_light.png" /></td><td><img width="300px" src="captures/android/cpp/adaptive_collection_light.png" /></td><td><img width="300px" src="captures/android/xaml/adaptive_collection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports AdaptiveCollectionView.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.AdaptiveCollectionView)

#### 🟢 Sonnet 5 Review

Single-column item list layout, text, and spacing match MAUI exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 4. Alerts — 🟢/⏳
<sub>alerts</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/alerts_light.png" /></td><td><img width="300px" src="captures/android/cpp/alerts_light.png" /></td><td><img width="300px" src="captures/android/xaml/alerts_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports AlertsPage.xaml (+ AlertsPage.xaml.cs) The C# AlertsPage drives the three Page dialog services — DisplayAlertAsync (simple OK + Yes/No), DisplayActionSheetAsync (simple + Cancel/Delete), and DisplayPromptAsync (two questions) — from a

#### 🟢 Sonnet 5 Review

All alert/actionsheet/prompt buttons match MAUI in text, order, and gray button styling.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 5. Alignment — 🟢/⏳
<sub>alignment</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/alignment_light.png" /></td><td><img width="300px" src="captures/android/cpp/alignment_light.png" /></td><td><img width="300px" src="captures/android/xaml/alignment_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;alignment&amp;quot; demo (ComparePages.Alignment()), the shipped-.NET-MAUI reference for the visual-parity comparison

#### 🟢 Sonnet 5 Review

Start/Center/End/Fill alignment demo renders identically to MAUI with matching blue boxes and red outlines.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 6. Animation — 🟡/⏳
<sub>animation</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/animation_light.png" /></td><td><img width="300px" src="captures/android/cpp/animation_light.png" /></td><td><img width="300px" src="captures/android/xaml/animation_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports AnimationPage.xaml (+ AnimationPage.xaml.cs)

#### 🟡 Sonnet 5 Review

The 'Cancel Animation' button renders nearly invisible (very light gray) in cpp while MAUI shows it with the same solid gray as the other buttons, indicating a disabled-state color difference.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 7. App Theme Binding — 🟢/⏳
<sub>app_theme_binding</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/app_theme_binding_light.png" /></td><td><img width="300px" src="captures/android/cpp/app_theme_binding_light.png" /></td><td><img width="300px" src="captures/android/xaml/app_theme_binding_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports AppThemeBindingPage.xaml

#### 🟢 Sonnet 5 Review

Green/orange theme-bound text and toggle button match MAUI exactly in color and layout.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 8. Application Control — 🟢/⏳
<sub>application_control</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/application_control_light.png" /></td><td><img width="300px" src="captures/android/cpp/application_control_light.png" /></td><td><img width="300px" src="captures/android/xaml/application_control_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ApplicationControlPage.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 Review

Button layout and status text match MAUI; only the window title text differs which is expected runtime content, not a bug.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 9. Auto Size Shapes — 🟢/⏳
<sub>auto_size_shapes</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/auto_size_shapes_light.png" /></td><td><img width="300px" src="captures/android/cpp/auto_size_shapes_light.png" /></td><td><img width="300px" src="captures/android/xaml/auto_size_shapes_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports AutoSizeShapesGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/AutoSizeShapesGallery.xaml: a 3-row Grid (RowSpacing 0) that proves a stroked Ellipse auto-sizes to fill exactly half of the av

#### 🟢 Sonnet 5 Review

The green ellipse with blue outline fills the yellow region identically in both renders, with matching proportions.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 10. Basic Grouping — 🟢/⏳
<sub>basic_grouping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/basic_grouping_light.png" /></td><td><img width="300px" src="captures/android/cpp/basic_grouping_light.png" /></td><td><img width="300px" src="captures/android/xaml/basic_grouping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports GroupingGalleries/BasicGrouping.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 Review

Grouped list with headers (Avengers, Fantastic Four, Defenders, etc.) and 'Total members' counts match MAUI in color and text exactly, just scrolled to a different position.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 11. Basic Swipe — 🟢/⏳
<sub>basic_swipe</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/basic_swipe_light.png" /></td><td><img width="300px" src="captures/android/cpp/basic_swipe_light.png" /></td><td><img width="300px" src="captures/android/xaml/basic_swipe_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports BasicSwipeGallery.xaml A code-first port of the MAUI SwipeView sub-gallery Pages/Controls/SwipeViewGalleries/BasicSwipeGallery.xaml: a vertical StackLayout of five SwipeViews, each demonstrating a different revealed-side / SwipeMode c

#### 🟢 Sonnet 5 Review

All five swipe-direction demo rows match MAUI in text, sizing, and gray background.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 12. Behaviors — 🟢/⏳
<sub>behaviors</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/behaviors_light.png" /></td><td><img width="300px" src="captures/android/cpp/behaviors_light.png" /></td><td><img width="300px" src="captures/android/xaml/behaviors_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports BehaviorsPage.xaml (+ .xaml.cs) and its companion Controls.Sample/Behaviors/NumericValidationBehavior.cs

#### 🟢 Sonnet 5 Review

Entry field with 'Enter a System.Double' placeholder and header text match MAUI exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 13. Border — 🟢/⏳
<sub>border</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/border_light.png" /></td><td><img width="300px" src="captures/android/cpp/border_light.png" /></td><td><img width="300px" src="captures/android/xaml/border_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;border&amp;quot; demo (ComparePages.BorderPage()), the shipped-.NET-MAUI reference for the visual-parity comparison: a single Border centered on the page — red 5pt stroke, a RoundRectangle StrokeShape (Co

#### 🟢 Sonnet 5 Review

Bordered content box with red outline and light-yellow fill matches MAUI in size, position, and text.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 14. Border Alignment — ⏳/⏳
<sub>border_alignment</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/android/xaml/border_alignment_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

#### ⏳ Sonnet 5 Review

_Not yet reviewed._

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 15. Border Clip Playground — 🟡/⏳
<sub>border_clip_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/border_clip_playground_light.png" /></td><td><img width="300px" src="captures/android/cpp/border_clip_playground_light.png" /></td><td><img width="300px" src="captures/android/xaml/border_clip_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports BorderClipPlayground.xaml (+ .xaml.cs) The C# page is an interactive Border-shape playground: a 100x100 Border (red stroke) clips an AspectFill Image (oasis.jpg) into the currently selected StrokeShape, while controls below mutate the

#### 🟡 Sonnet 5 Review

The bottom-right corner of the bordered dog image appears rounded in MAUI (per the Bottom Right Corner Radius: 12 slider) but renders as a sharp square corner in cpp.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 16. Border Layout — 🟡/⏳
<sub>border_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/border_layout_light.png" /></td><td><img width="300px" src="captures/android/cpp/border_layout_light.png" /></td><td><img width="300px" src="captures/android/xaml/border_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports BorderLayout.xaml (+ BorderLayout.xaml.cs) The C# page demonstrates driving Border.StrokeThickness from a Slider: a Slider (0..40, set to 5 in OnAppearing) is bound to the Border&amp;#x27;s StrokeThickness; the Border (Silver stroke, White bac

#### 🟡 Sonnet 5 Review

MAUI's bordered bar sits inset from the screen edges showing full rounded corners on both ends, while cpp's bar runs flush to the left screen edge, clipping off the left rounded corner and red segment.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 17. Border Playground — 🟡/⏳
<sub>border_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/border_playground_light.png" /></td><td><img width="300px" src="captures/android/cpp/border_playground_light.png" /></td><td><img width="300px" src="captures/android/xaml/border_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports BorderPlayground.xaml (+ BorderPlayground.xaml.cs) A self-contained, code-first interactive Border playground

#### 🟡 Sonnet 5 Review

Layout and colors match closely; only the outer status-bar/page-padding differs per policy (not scored), core border/content/gradient rendering matches MAUI.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 18. Border Resize Content — 🟡/⏳
<sub>border_resize_content</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/border_resize_content_light.png" /></td><td><img width="300px" src="captures/android/cpp/border_resize_content_light.png" /></td><td><img width="300px" src="captures/android/xaml/border_resize_content_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports BorderResizeContent.xaml A self-contained, code-first demo that resizes a Border&amp;#x27;s CONTENT and watches the Border track it

#### 🟡 Sonnet 5 Review

Shapes, colors and images match, but the top-left red circle is missing the thin border/inset ring that MAUI's reference shows around the plus-sign circle.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 19. Border Stroke — 🟢/⏳
<sub>border_stroke</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/border_stroke_light.png" /></td><td><img width="300px" src="captures/android/cpp/border_stroke_light.png" /></td><td><img width="300px" src="captures/android/xaml/border_stroke_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports BorderStroke.xaml (+ BorderStroke.xaml.cs) A self-contained, code-first demo of Border StrokeThickness and how a Border tracks the height of its content

#### 🟢 Sonnet 5 Review

Stroke thickness variations and orange/red boxes match MAUI exactly in size, color, and text.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 20. Border Styles — ⏳/⏳
<sub>border_styles</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/android/xaml/border_styles_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

#### ⏳ Sonnet 5 Review

_Not yet reviewed._

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 21. Borderless — 🟢/⏳
<sub>borderless</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/borderless_light.png" /></td><td><img width="300px" src="captures/android/cpp/borderless_light.png" /></td><td><img width="300px" src="captures/android/xaml/borderless_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports Borderless.xaml A self-contained, code-first demo of a stroke-less Border

#### 🟢 Sonnet 5 Review

Yellow background and toggle switch match MAUI precisely, aside from the unscored status-bar padding.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 22. Box View — 🟢/⏳
<sub>box_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/box_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/box_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/box_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports BoxViewPage.xaml (+ BoxViewPage.xaml.cs)

#### 🟢 Sonnet 5 Review

All four labeled box views (solid, color, gradient, rounded) match MAUI in color, size and corner radius.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 23. Button — 🟢/⏳
<sub>button</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/button_light.png" /></td><td><img width="300px" src="captures/android/cpp/button_light.png" /></td><td><img width="300px" src="captures/android/xaml/button_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ButtonPage.xaml (+ ButtonPage.xaml.cs)

#### 🟢 Sonnet 5 Review

All button variants (colors, borders, strikethrough text, black settings buttons) match MAUI in order, color, and text.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 24. Carousel Page — 🔴/⏳
<sub>carousel_page</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/carousel_page_light.png" /></td><td><img width="300px" src="captures/android/cpp/carousel_page_light.png" /></td><td><img width="300px" src="captures/android/xaml/carousel_page_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

Carousel Page

#### 🔴 Sonnet 5 Review

Item 1 label and Prev/Next buttons are shifted much lower on the page (near vertical middle) in the C++ render versus being grouped near the top in MAUI.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 25. Carousel View — ⏳/⏳
<sub>carousel_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/android/xaml/carousel_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

#### ⏳ Sonnet 5 Review

_Not yet reviewed._

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 26. Chat Example — 🟡/⏳
<sub>chat_example</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/chat_example_light.png" /></td><td><img width="300px" src="captures/android/cpp/chat_example_light.png" /></td><td><img width="300px" src="captures/android/xaml/chat_example_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ChatExample.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.ItemSizeGalleries.ChatExample), tracking the maui-compare reference demo ~/maui-compare/Pages/ChatExamplePage.cs (the visual-parity oracle)

#### 🟡 Sonnet 5 Review

C++ shows both the sent 'Hi there!' bubble and the reply, while MAUI's capture only shows the reply bubble, indicating differing capture/interaction state rather than a rendering bug.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 27. Check Box — 🟢/⏳
<sub>check_box</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/check_box_light.png" /></td><td><img width="300px" src="captures/android/cpp/check_box_light.png" /></td><td><img width="300px" src="captures/android/xaml/check_box_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports CheckBoxPage.xaml (+ .xaml.cs) Mirrors the MAUI gallery page: a vertical stack of headlined CheckBox states — Default, Colored (Color=Purple), Disabled, Disabled+Colored+Checked — followed by a &amp;quot;Change IsChecked&amp;quot; row pairing a Button

#### 🟢 Sonnet 5 Review

All checkbox states (default, colored, disabled, disabled-colored) and the IsChecked toggle row match MAUI exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 28. Chrome — 🟢/⏳
<sub>chrome</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/chrome_light.png" /></td><td><img width="300px" src="captures/android/cpp/chrome_light.png" /></td><td><img width="300px" src="captures/android/xaml/chrome_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a self-contained demo page for the W1-11 window-chrome family: page toolbar items (primary + secondary), a menu bar (File menu with items, a separator and a sub-menu), a context flyout (right-click menu) on a button, and a tooltip — all wir

#### 🟢 Sonnet 5 Review

The 'Press or right-click me' button and 'Ready' status text match MAUI in size, color and position.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 29. Clip — 🟢/⏳
<sub>clip</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/clip_light.png" /></td><td><img width="300px" src="captures/android/cpp/clip_light.png" /></td><td><img width="300px" src="captures/android/xaml/clip_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ClipPage.xaml The C# page (Pages/Core/ClipPage.xaml; its .xaml.cs is an empty InitializeComponent) is a ScrollView over a StackLayout that shows the SAME dotnet_bot.png image five times, each successive copy carrying a different geome

#### 🟢 Sonnet 5 Review

Clipped image variants (rectangle, ellipse, geometry group) render identically to MAUI in shape and content.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 30. Clip Corner Radius — 🟢/⏳
<sub>clip_corner_radius</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/clip_corner_radius_light.png" /></td><td><img width="300px" src="captures/android/cpp/clip_corner_radius_light.png" /></td><td><img width="300px" src="captures/android/xaml/clip_corner_radius_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ClipCornerRadiusGallery.xaml (+ .xaml.cs) The C# page (Pages/Controls/ShapesGalleries/ClipCornerRadiusGallery.xaml) is a StackLayout (Padding=12) that demonstrates DRIVING a RoundRectangleGeometry&amp;#x27;s per-corner CornerRadius from four s

#### 🟢 Sonnet 5 Review

Clipped rounded-rectangle image and the four corner-radius sliders match MAUI exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 31. Clip Gallery — 🟢/⏳
<sub>clip_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/clip_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/clip_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/clip_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ClipGallery.xaml The C# page (Pages/Controls/ShapesGalleries/ClipGallery.xaml; its .xaml.cs is an empty InitializeComponent) is a ScrollView over a StackLayout (Padding=12) that shows the SAME &amp;quot;oasis.jpg&amp;quot; image SEVEN times — one bare

#### 🟢 Sonnet 5 Review

Image, RectangleGeometry, and RoundRectangleGeometry clipped image sections all match MAUI in layout and content.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 32. Clip Views — 🟢/⏳
<sub>clip_views</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/clip_views_light.png" /></td><td><img width="300px" src="captures/android/cpp/clip_views_light.png" /></td><td><img width="300px" src="captures/android/xaml/clip_views_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ClipViewsGallery.xaml The C# page (Pages/Controls/ShapesGalleries/ClipViewsGallery.xaml; no code-behind beyond an empty InitializeComponent) is a ScrollView over a StackLayout (Padding=12) that proves the Clip surface (VisualElement.C

#### 🟢 Sonnet 5 Review

All seven clipped-shape view rows (button, date entry, editor, grid, search icon, time) match MAUI aside from a one-day date difference from capture timing.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 33. Clipping — 🔴/⏳
<sub>clipping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/clipping_light.png" /></td><td><img width="300px" src="captures/android/cpp/clipping_light.png" /></td><td><img width="300px" src="captures/android/xaml/clipping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

compare oracle ~/maui-compare/Pages/ClippingPage.cs (itself written to mirror this gallery page)

#### 🔴 Sonnet 5 Review

The numbered box row displays fewer, wider boxes (1-4 plus partial 5) than MAUI's five full boxes plus partial 6-8, and the bottom row shows two coffee-cup icons in C++ versus a plain light-blue bar with no icons in MAUI, indicating a real layout/content bug.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 34. Collectionview — 🟡/⏳
<sub>collectionview</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/collectionview_light.png" /></td><td><img width="300px" src="captures/android/cpp/collectionview_light.png" /></td><td><img width="300px" src="captures/android/xaml/collectionview_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;collectionview&amp;quot; demo (ComparePages.CollectionViewPage()), the shipped-.NET-MAUI reference for the visual-parity comparison: a CollectionView over 24 captioned items, a string Header (&amp;quot;This is the

#### 🟡 Sonnet 5 Review

C++ shows an extra 'This is the header' text row above the grid that MAUI does not display.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 35. Composition Gallery — 🟢/⏳
<sub>composition_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/composition_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/composition_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/composition_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports CompositionGallery.xaml A self-contained, code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/CompositionGallery.xaml: a StackLayout holding two Beige 250x250 Grids (Margin 12) that compose multiple overlappi

#### 🟢 Sonnet 5 Review

Shapes composition and line diagram match exactly between MAUI and C++.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 36. Containers — 🟢/⏳
<sub>containers</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/containers_light.png" /></td><td><img width="300px" src="captures/android/cpp/containers_light.png" /></td><td><img width="300px" src="captures/android/xaml/containers_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a self-contained demo page for the W1-07 container set: a scroll_view hosting a vertical stack of content-hosting containers — a border-framed label (stroke + dashed outline + rounded shape), a legacy frame (BorderColor/CornerRadius/HasShad

#### 🟢 Sonnet 5 Review

Border, frame, and content_view boxes with dashed/solid outlines match exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 37. Content View — 🟢/⏳
<sub>content_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/content_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/content_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/content_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ContentViewPage.xaml (+ ContentViewPage.xaml.cs), code-first

#### 🟢 Sonnet 5 Review

ContentView swap layout with nested Content/Swap content button matches exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 38. Context Flyout — 🔴/⏳
<sub>context_flyout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/context_flyout_light.png" /></td><td><img width="300px" src="captures/android/cpp/context_flyout_light.png" /></td><td><img width="300px" src="captures/android/xaml/context_flyout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ContextFlyoutPage.xaml (+ ContextFlyoutPage.xaml.cs) The C# page attaches a MenuFlyout as the FlyoutBase.ContextFlyout (right-click / long-press menu) of several controls and wires each menu item to a handler: - a Button (&amp;quot;Increment b

#### 🔴 Sonnet 5 Review

C++ renders a broken WebView 'Webpage not available' error page instead of the expected switch/entry/image context-menu demo UI.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 39. Controls Stack — 🟡/⏳
<sub>controls_stack</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/controls_stack_light.png" /></td><td><img width="300px" src="captures/android/cpp/controls_stack_light.png" /></td><td><img width="300px" src="captures/android/xaml/controls_stack_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;controls_stack&amp;quot; demo (ComparePages.ControlsStack()), the shipped-.NET-MAUI reference for the visual-parity comparison: a VerticalStackLayout (Spacing 12, Padding 16) showcasing the basic widgets

#### 🟡 Sonnet 5 Review

The ActivityIndicator (third control in the row) renders as a malformed small squiggle/comma shape in C++ instead of MAUI's circular spinner ring; everything else matches.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 40. Custom Layout — 🟡/⏳
<sub>custom_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/custom_layout_light.png" /></td><td><img width="300px" src="captures/android/cpp/custom_layout_light.png" /></td><td><img width="300px" src="captures/android/xaml/custom_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports CustomLayoutPage.xaml (+ CustomLayoutPage.xaml.cs)

#### 🟡 Sonnet 5 Review

MAUI's Top/Bottom bars render as translucent gray with small triangle corner markers while C++ renders them as solid flat gray rectangles without the markers; layout and text otherwise match.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 41. Custom Size Swipe — 🟢/⏳
<sub>custom_size_swipe</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/custom_size_swipe_light.png" /></td><td><img width="300px" src="captures/android/cpp/custom_size_swipe_light.png" /></td><td><img width="300px" src="captures/android/xaml/custom_size_swipe_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports CustomSizeSwipeViewGallery.xaml (+ .xaml.cs) The MAUI CustomSizeSwipeViewGallery is a single SwipeView whose Left / Right / Top item collections each reveal CUSTOM-SIZED content: a SwipeItemView wrapping a Grid/StackLayout with an exp

#### 🟢 Sonnet 5 Review

SwipeView content, button, and revealed-state text match exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 42. Custom Swipe Item View — 🟢/⏳
<sub>custom_swipe_item_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/custom_swipe_item_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/custom_swipe_item_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/custom_swipe_item_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports CustomSwipeItemViewGallery.xaml A self-contained, code-first port of the .NET MAUI &amp;quot;CustomSwipeItem&amp;quot; gallery: a message-list row whose right swipe reveals a CUSTOM-content swipe item (a swipe_item_view, not a plain swipe_item)

#### 🟢 Sonnet 5 Review

Custom swipe item card with title/date and purple background matches exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 43. Cv Visual States — 🟢/⏳
<sub>cv_visual_states</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/cv_visual_states_light.png" /></td><td><img width="300px" src="captures/android/cpp/cv_visual_states_light.png" /></td><td><img width="300px" src="captures/android/xaml/cv_visual_states_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports CollectionViewGalleries/SelectionGalleries/ VisualStatesGallery.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 Review

Single/Multi selection item lists match exactly in text and layout.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 44. Data Template Selector — 🟢/⏳
<sub>data_template_selector</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/data_template_selector_light.png" /></td><td><img width="300px" src="captures/android/cpp/data_template_selector_light.png" /></td><td><img width="300px" src="captures/android/xaml/data_template_selector_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports DataTemplateSelectorGallery.xaml (+ DataTemplateSelectorGallery.xaml.cs, including its WeekendSelector + SearchTermSelector classes)

#### 🟢 Sonnet 5 Review

Day-of-week templated list content and repeated pattern match exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 45. Date Picker — 🔴/⏳
<sub>date_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/date_picker_light.png" /></td><td><img width="300px" src="captures/android/cpp/date_picker_light.png" /></td><td><img width="300px" src="captures/android/xaml/date_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports DatePickerPage.xaml (+ DatePickerPage.xaml.cs)

#### 🔴 Sonnet 5 Review

The third gradient DatePicker background renders blue-to-teal in MAUI but red/pink-to-purple in C++, a real color content bug; C++ capture is also cut off before showing the final IsFocused/Set-to-null rows visible in MAUI.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 46. Device — 🟢/⏳
<sub>device</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/device_light.png" /></td><td><img width="300px" src="captures/android/cpp/device_light.png" /></td><td><img width="300px" src="captures/android/xaml/device_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports DevicePage.xaml

#### 🟢 Sonnet 5 Review

Platform/Idiom/Version text block matches exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 47. Dispatcher — 🟢/⏳
<sub>dispatcher</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/dispatcher_light.png" /></td><td><img width="300px" src="captures/android/cpp/dispatcher_light.png" /></td><td><img width="300px" src="captures/android/xaml/dispatcher_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports DispatcherPage.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 Review

All dispatcher demo buttons and status text match exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 48. Drag Drop — 🟢/⏳
<sub>drag_drop</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/drag_drop_light.png" /></td><td><img width="300px" src="captures/android/cpp/drag_drop_light.png" /></td><td><img width="300px" src="captures/android/xaml/drag_drop_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports DragAndDropBetweenLayouts.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.DragAndDropBetweenLayouts)

#### 🟢 Sonnet 5 Review

Color swatches, rainbow list, and drag/drop position text all match exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 49. Editor — 🟢/⏳
<sub>editor</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/editor_light.png" /></td><td><img width="300px" src="captures/android/cpp/editor_light.png" /></td><td><img width="300px" src="captures/android/xaml/editor_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports EditorPage.xaml (Microsoft.Maui.Controls sample gallery)

#### 🟢 Sonnet 5 Review

Layout, colors, and text content match MAUI reference exactly; only status-bar chrome differs.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 50. Effects — 🟢/⏳
<sub>effects</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/effects_light.png" /></td><td><img width="300px" src="captures/android/cpp/effects_light.png" /></td><td><img width="300px" src="captures/android/xaml/effects_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports EffectsPage.xaml (Maui.Controls.Sample.Pages.EffectsPage)

#### 🟢 Sonnet 5 Review

Entry fields, disabled buttons, and status label render identically to the MAUI reference.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 51. Ellipse Gallery — 🟢/⏳
<sub>ellipse_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ellipse_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/ellipse_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/ellipse_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports EllipseGallery.xaml A self-contained, code-first port of the MAUI Shapes EllipseGallery (Pages/Controls/ShapesGalleries/EllipseGallery.xaml + .xaml.cs)

#### 🟢 Sonnet 5 Review

All shapes (rectangle, circle, ellipses with stroke/dash) match position, size, and color exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 52. Empty View — 🟢/⏳
<sub>empty_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/empty_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/empty_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/empty_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports EmptyViewStringGallery.xaml (+ EmptyViewStringGallery.xaml.cs)

#### 🟢 Sonnet 5 Review

Filter bar and scrollable file list match MAUI reference exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 53. Empty View Load Simulate — 🟢/⏳
<sub>empty_view_load_simulate</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/empty_view_load_simulate_light.png" /></td><td><img width="300px" src="captures/android/cpp/empty_view_load_simulate_light.png" /></td><td><img width="300px" src="captures/android/xaml/empty_view_load_simulate_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewLoadSimulateGallery.xaml (+ EmptyViewLoadSimulateGallery.xaml.cs)

#### 🟢 Sonnet 5 Review

Loading-simulation text is centered identically in both renders.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 54. Empty View Null — 🟢/⏳
<sub>empty_view_null</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/empty_view_null_light.png" /></td><td><img width="300px" src="captures/android/cpp/empty_view_null_light.png" /></td><td><img width="300px" src="captures/android/xaml/empty_view_null_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewNullGallery.xaml (+ EmptyViewNullGallery.xaml.cs)

#### 🟢 Sonnet 5 Review

Nothing to display. centered message matches exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 55. Empty View Rtl — 🟢/⏳
<sub>empty_view_rtl</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/empty_view_rtl_light.png" /></td><td><img width="300px" src="captures/android/cpp/empty_view_rtl_light.png" /></td><td><img width="300px" src="captures/android/xaml/empty_view_rtl_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewRTLGallery.xaml (+ EmptyViewRTLGallery.xaml.cs)

#### 🟢 Sonnet 5 Review

Three-column filtered list layout and content match MAUI reference exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 56. Empty View Selector — 🟢/⏳
<sub>empty_view_selector</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/empty_view_selector_light.png" /></td><td><img width="300px" src="captures/android/cpp/empty_view_selector_light.png" /></td><td><img width="300px" src="captures/android/xaml/empty_view_selector_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewWithDataTemplateSelector.xaml (+ .xaml.cs, incl

#### 🟢 Sonnet 5 Review

Instructional text, filter bar, and single result row match exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 57. Empty View Swap — 🟢/⏳
<sub>empty_view_swap</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/empty_view_swap_light.png" /></td><td><img width="300px" src="captures/android/cpp/empty_view_swap_light.png" /></td><td><img width="300px" src="captures/android/xaml/empty_view_swap_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewSwapGallery.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 Review

Toggle switch, Clear/Fill buttons, and three-column list all match MAUI reference.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 58. Empty View Template — 🟢/⏳
<sub>empty_view_template</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/empty_view_template_light.png" /></td><td><img width="300px" src="captures/android/cpp/empty_view_template_light.png" /></td><td><img width="300px" src="captures/android/xaml/empty_view_template_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewTemplateGallery.xaml (+ EmptyViewTemplateGallery.xaml.cs)

#### 🟢 Sonnet 5 Review

Three-column filtered list matches MAUI reference exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 59. Empty View View — 🟢/⏳
<sub>empty_view_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/empty_view_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/empty_view_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/empty_view_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewViewGallery.xaml (+ EmptyViewViewGallery.xaml.cs)

#### 🟢 Sonnet 5 Review

Three-column filtered list matches MAUI reference exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 60. Entry — 🟢/⏳
<sub>entry</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/entry_light.png" /></td><td><img width="300px" src="captures/android/cpp/entry_light.png" /></td><td><img width="300px" src="captures/android/xaml/entry_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports EntryPage.xaml (Microsoft.Maui.Controls sample gallery)

#### 🟢 Sonnet 5 Review

All entry fields, checkbox, password dots, cursor slider, and labels match MAUI reference exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 61. Filter Collection — 🟢/⏳
<sub>filter_collection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/filter_collection_light.png" /></td><td><img width="300px" src="captures/android/cpp/filter_collection_light.png" /></td><td><img width="300px" src="captures/android/xaml/filter_collection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports FilterCollectionView.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 Review

Use EmptyView toggle and two-column filtered file list match MAUI reference exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 62. Filter Selection — 🟢/⏳
<sub>filter_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/filter_selection_light.png" /></td><td><img width="300px" src="captures/android/cpp/filter_selection_light.png" /></td><td><img width="300px" src="captures/android/xaml/filter_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports FilterSelection.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.FilterSelection)

#### 🟢 Sonnet 5 Review

Instructional text, Reset button, Selected label, and list match MAUI reference exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 63. Flex Layout — 🟢/⏳
<sub>flex_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/flex_layout_light.png" /></td><td><img width="300px" src="captures/android/cpp/flex_layout_light.png" /></td><td><img width="300px" src="captures/android/xaml/flex_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports FlexLayoutPage.xaml A self-contained, code-first demo of the FlexLayout control: the classic &amp;quot;holy grail&amp;quot; page layout built from nested flexboxes

#### 🟢 Sonnet 5 Review

Header/content/footer flex layout with blue/gray/green columns and pink footer all match MAUI reference exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 64. Focus — 🟢/⏳
<sub>focus</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/focus_light.png" /></td><td><img width="300px" src="captures/android/cpp/focus_light.png" /></td><td><img width="300px" src="captures/android/xaml/focus_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports FocusPage.xaml (+ FocusPage.xaml.cs) The C# FocusPage is a focus-subsystem demo: an Entry whose Focused/Unfocused events (OnFocusEntryFocusChanged) append &amp;quot;Focused&amp;quot;/&amp;quot;Unfocused&amp;quot; lines to a scrolling InfoLabel, plus two buttons — &amp;quot;Focus

#### 🟢 Sonnet 5 Review

Entry, two buttons, and IsFocused label render identically in position, size, and text.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 65. Fonts — 🟢/⏳
<sub>fonts</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/fonts_light.png" /></td><td><img width="300px" src="captures/android/cpp/fonts_light.png" /></td><td><img width="300px" src="captures/android/xaml/fonts_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;fonts&amp;quot; demo (ComparePages.Fonts()), the shipped-.NET-MAUI reference for the visual-parity comparison: a ScrollView over a VerticalStackLayout (Spacing 8, Padding 16) of nine Labels — Title/Subtit

#### 🟢 Sonnet 5 Review

All font style rows (title, subtitle, header, body, caption, bold, italic, character spacing) match MAUI in size and weight.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 66. Footer Only String — 🟢/⏳
<sub>footer_only_string</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/footer_only_string_light.png" /></td><td><img width="300px" src="captures/android/cpp/footer_only_string_light.png" /></td><td><img width="300px" src="captures/android/xaml/footer_only_string_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports FooterOnlyString.xaml (+ FooterOnlyString.xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Sonnet 5 Review

List content and footer string text match exactly; only vertical scroll offset differs between the two captures.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 67. Formatted Text — 🟢/⏳
<sub>formatted_text</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/formatted_text_light.png" /></td><td><img width="300px" src="captures/android/cpp/formatted_text_light.png" /></td><td><img width="300px" src="captures/android/xaml/formatted_text_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a self-contained demo page for the G1 rich-text slice: a label whose FormattedText is built from several styled spans (bold / italic / colored / underlined / kerned), plus a plain label proving the Text ⇄ FormattedText exclusivity

#### 🟢 Sonnet 5 Review

Formatted text spans (bold red, italic underlined, kerned, plain) render identically in color, style, and layout.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 68. Frame — ⏳/⏳
<sub>frame</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/android/xaml/frame_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

#### ⏳ Sonnet 5 Review

_Not yet reviewed._

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 69. Gestures — 🟢/⏳
<sub>gestures</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/gestures_light.png" /></td><td><img width="300px" src="captures/android/cpp/gestures_light.png" /></td><td><img width="300px" src="captures/android/xaml/gestures_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports GesturesPage.xaml (+ .xaml.cs) The MAUI GesturesPage.xaml is a *gallery navigation* page: a CollectionView listing gesture-demo sections that the shell navigates into

#### 🟢 Sonnet 5 Review

Gesture target rectangle and layout match; last-gesture text differs only due to runtime interaction state, not a rendering bug.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 70. Gradient — 🟢/⏳
<sub>gradient</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/gradient_light.png" /></td><td><img width="300px" src="captures/android/cpp/gradient_light.png" /></td><td><img width="300px" src="captures/android/xaml/gradient_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;gradient&amp;quot; demo (ComparePages.Gradient()), the shipped-.NET-MAUI reference for the visual-parity comparison: a VerticalStackLayout (Spacing 12, Padding 16) of two captioned 60px BoxViews — a Linea

#### 🟢 Sonnet 5 Review

Linear yellow-to-green and radial red-to-navy gradients render identically in colors and bounds.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 71. Grid — 🟢/⏳
<sub>grid</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/grid_light.png" /></td><td><img width="300px" src="captures/android/cpp/grid_light.png" /></td><td><img width="300px" src="captures/android/xaml/grid_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;grid&amp;quot; demo (ComparePages.GridPage()), the shipped-.NET-MAUI reference for the visual-parity comparison: a Grid (Padding 16, Row/ColumnSpacing 6) with RowDefinitions Auto / 80 / 80 and two Star co

#### 🟢 Sonnet 5 Review

2x2 color grid (red/green/blue/orange) matches exactly in position and size.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 72. Grid Definitions — ⏳/⏳
<sub>grid_definitions</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/android/xaml/grid_definitions_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

#### ⏳ Sonnet 5 Review

_Not yet reviewed._

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 73. Grid Grouping — 🟢/⏳
<sub>grid_grouping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/grid_grouping_light.png" /></td><td><img width="300px" src="captures/android/cpp/grid_grouping_light.png" /></td><td><img width="300px" src="captures/android/xaml/grid_grouping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports GroupingGalleries/GridGrouping.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 Review

Grouped two-column list content and orange/green group labels match; only scroll position differs.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 74. Grid Layout — ⏳/⏳
<sub>grid_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/android/xaml/grid_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

#### ⏳ Sonnet 5 Review

_Not yet reviewed._

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 75. Grouping No Templates — 🟢/⏳
<sub>grouping_no_templates</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/grouping_no_templates_light.png" /></td><td><img width="300px" src="captures/android/cpp/grouping_no_templates_light.png" /></td><td><img width="300px" src="captures/android/xaml/grouping_no_templates_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports GroupingGalleries/GroupingNoTemplates.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 Review

Flat grouped list of hero names matches exactly aside from scroll offset.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 76. Grouping Plus Selection — 🟢/⏳
<sub>grouping_plus_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/grouping_plus_selection_light.png" /></td><td><img width="300px" src="captures/android/cpp/grouping_plus_selection_light.png" /></td><td><img width="300px" src="captures/android/xaml/grouping_plus_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports CollectionViewGalleries/GroupingGalleries/ GroupingPlusSelection.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 Review

Grouped list with green group headers and orange total-member counts matches exactly aside from scroll offset.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 77. Header Footer — 🟢/⏳
<sub>header_footer</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/header_footer_light.png" /></td><td><img width="300px" src="captures/android/cpp/header_footer_light.png" /></td><td><img width="300px" src="captures/android/xaml/header_footer_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports HeaderFooterString.xaml (+ HeaderFooterString.xaml.cs)

#### 🟢 Sonnet 5 Review

Header string, image list rows, and footer string text match exactly aside from scroll offset.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 78. Header Footer Grid — 🟢/⏳
<sub>header_footer_grid</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/header_footer_grid_light.png" /></td><td><img width="300px" src="captures/android/cpp/header_footer_grid_light.png" /></td><td><img width="300px" src="captures/android/xaml/header_footer_grid_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports HeaderFooterGrid.xaml (+ HeaderFooterGrid.xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Sonnet 5 Review

Header image/title, three-column image grid, footer image/title, and buttons all render pixel-consistent with MAUI.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 79. Header Footer Grid Horizontal — 🟢/⏳
<sub>header_footer_grid_horizontal</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/header_footer_grid_horizontal_light.png" /></td><td><img width="300px" src="captures/android/cpp/header_footer_grid_horizontal_light.png" /></td><td><img width="300px" src="captures/android/xaml/header_footer_grid_horizontal_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports HeaderFooterGridHorizontal.xaml (+ HeaderFooterGridHorizontal.xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Sonnet 5 Review

Horizontal-scroll grid with header/footer images and toggle buttons match; only scroll position within the horizontal list differs.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 80. Header Footer Template — 🟢/⏳
<sub>header_footer_template</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/header_footer_template_light.png" /></td><td><img width="300px" src="captures/android/cpp/header_footer_template_light.png" /></td><td><img width="300px" src="captures/android/xaml/header_footer_template_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports HeaderFooterTemplate.xaml (+ .xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Sonnet 5 Review

Header/footer templated views with blue rows, image thumbnails, and footer image render the same structure; scroll offset and timestamp differ due to capture timing, not a bug.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 81. Header Footer View — 🟢/⏳
<sub>header_footer_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/header_footer_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/header_footer_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/header_footer_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports HeaderFooterView.xaml (+ .xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Sonnet 5 Review

Header image with title, footer image with title, and Add/Clear buttons render identically aside from vertical scroll offset.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 82. Hit Testing — 🔴/⏳
<sub>hit_testing</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/hit_testing_light.png" /></td><td><img width="300px" src="captures/android/cpp/hit_testing_light.png" /></td><td><img width="300px" src="captures/android/xaml/hit_testing_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports HitTestingPage.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.HitTestingPage)

#### 🔴 Sonnet 5 Review

C++ render extends past the right edge of the viewport (Scale=2 bar and green box overflow horizontally / are cut off), unlike MAUI which fits the frame.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 83. Horizontal Stack — 🟢/⏳
<sub>horizontal_stack</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/horizontal_stack_light.png" /></td><td><img width="300px" src="captures/android/cpp/horizontal_stack_light.png" /></td><td><img width="300px" src="captures/android/xaml/horizontal_stack_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

Horizontal Stack

#### 🟢 Sonnet 5 Review

Colored stripe layout, text, and positions match MAUI closely.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 84. Horizontal Stack Layout — ⏳/⏳
<sub>horizontal_stack_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/android/xaml/horizontal_stack_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

#### ⏳ Sonnet 5 Review

_Not yet reviewed._

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 85. Hybrid Web View — 🟢/⏳
<sub>hybrid_web_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/hybrid_web_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/hybrid_web_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/hybrid_web_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports HybridWebViewPage.xaml (+ HybridWebViewPage.xaml.cs)

#### 🟢 Sonnet 5 Review

Buttons, text, and webview error state match MAUI (button label wrapping differs slightly but content and layout are correct).

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 86. Image — 🔴/⏳
<sub>image</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/image_light.png" /></td><td><img width="300px" src="captures/android/cpp/image_light.png" /></td><td><img width="300px" src="captures/android/xaml/image_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ImagePage.xaml (+ ImagePage.xaml.cs)

#### 🔴 Sonnet 5 Review

The animated-gif preview box shows a black background with a small colored heart icon instead of MAUI's dimmed/rotated submarine image, and content below (Stream Source, Opacity) is missing from view.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 87. Image Button — 🔴/⏳
<sub>image_button</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/image_button_light.png" /></td><td><img width="300px" src="captures/android/cpp/image_button_light.png" /></td><td><img width="300px" src="captures/android/xaml/image_button_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ImageButtonPage.xaml (+ ImageButtonPage.xaml.cs) A self-contained, code-first demo page for the ImageButton control (the C# gallery-page convention, mirroring the input_controls_page / image_page pattern)

#### 🔴 Sonnet 5 Review

Missing corner-radius arrow glyphs (replaced by thin magenta lines) and the entire lower section (Use Online Source button, Background toggle text) is cut off/missing compared to MAUI.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 88. Indicator — 🔴/⏳
<sub>indicator</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/indicator_light.png" /></td><td><img width="300px" src="captures/android/cpp/indicator_light.png" /></td><td><img width="300px" src="captures/android/xaml/indicator_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports IndicatorPage.xaml A self-contained, code-first demo of the IndicatorView control

#### 🔴 Sonnet 5 Review

CarouselView content shows only Item 1 instead of MAUI's multi-item content, indicating missing/incomplete CarouselView items.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 89. Input Controls — 🟢/⏳
<sub>input_controls</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/input_controls_light.png" /></td><td><img width="300px" src="captures/android/cpp/input_controls_light.png" /></td><td><img width="300px" src="captures/android/xaml/input_controls_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a self-contained demo page for the W1-05 input-control set: editor, search_bar, radio_button (+ the radio_button_group attached grouping) and image_button on one vertical stack, wired together so every input drives a visible output (the C#

#### 🟢 Sonnet 5 Review

Entry, search bar, and radio buttons render identically to MAUI in text, style, and layout.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 90. Input Transparent — 🟢/⏳
<sub>input_transparent</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/input_transparent_light.png" /></td><td><img width="300px" src="captures/android/cpp/input_transparent_light.png" /></td><td><img width="300px" src="captures/android/xaml/input_transparent_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports InputTransparentPage.xaml (Maui.Controls.Sample.Pages.InputTransparentPage)

#### 🟢 Sonnet 5 Review

All buttons, toggle switch, and instructional text match MAUI exactly aside from minor uniform padding difference.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 91. Invalidate Brush — 🟢/⏳
<sub>invalidate_brush</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/invalidate_brush_light.png" /></td><td><img width="300px" src="captures/android/cpp/invalidate_brush_light.png" /></td><td><img width="300px" src="captures/android/xaml/invalidate_brush_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports InvalidateBrushGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/InvalidateBrushGallery.xaml (&amp;quot;Invalidate Brushes Playground&amp;quot;): a VerticalStackLayout (Padding 12) with — - a &amp;quot;Change color&amp;quot; Bu

#### 🟢 Sonnet 5 Review

Change color button and brush color text match MAUI precisely.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 92. Invalidate Shadow Host — 🔴/⏳
<sub>invalidate_shadow_host</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/invalidate_shadow_host_light.png" /></td><td><img width="300px" src="captures/android/cpp/invalidate_shadow_host_light.png" /></td><td><img width="300px" src="captures/android/xaml/invalidate_shadow_host_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports InvalidateShadowHostPage.xaml A self-contained, code-first demo that a shadow re-applies (invalidates) when its host&amp;#x27;s size changes, mirroring the C# core gallery page (Pages/Core/ShadowGalleries/InvalidateShadowHostPage.xaml + .xaml.

#### 🔴 Sonnet 5 Review

C++ render is missing the red shadow glow beneath the green bordered box that MAUI clearly renders.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 93. Ios Blur Effect — 🟢/⏳
<sub>ios_blur_effect</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_blur_effect_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_blur_effect_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_blur_effect_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports iOSBlurEffectPage.xaml The .NET MAUI PlatformSpecifics sample (Pages/PlatformSpecifics/iOS/iOSBlurEffectPage.xaml + .xaml.cs): an Image (Source=&amp;quot;oasis.jpg&amp;quot;) carrying the iOSSpecific VisualElement.BlurEffect knob (XAML seeds it to Extr

#### 🟢 Sonnet 5 Review

Image, blur option buttons, and status text all match MAUI.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 94. Ios Date Picker — 🟢/⏳
<sub>ios_date_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_date_picker_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_date_picker_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_date_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports iOSDatePickerPage.xaml (+ iOSDatePickerPage.xaml.cs)

#### 🟢 Sonnet 5 Review

Date text field and toggle button match MAUI exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 95. Ios Entry — 🟢/⏳
<sub>ios_entry</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_entry_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_entry_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_entry_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports iOSEntryPage.xaml (+ iOSEntryPage.xaml.cs)

#### 🟢 Sonnet 5 Review

Entry placeholder text and toggle button match MAUI exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 96. Ios First Responder — 🟢/⏳
<sub>ios_first_responder</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_first_responder_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_first_responder_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_first_responder_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports iOSFirstResponderPage.xaml (+ .xaml.cs) The C# iOSFirstResponderPage is a VisualElement-first-responder demo: a StackLayout with an explanatory Label, a &amp;quot;First Entry&amp;quot; + plain &amp;quot;OK&amp;quot; Button (tapping OK dismisses the keyboard because the

#### 🟢 Sonnet 5 Review

All entries, OK buttons, focus buttons, and status text match MAUI closely (only minor spacing differences).

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 97. Ios Pan Gesture — 🟢/⏳
<sub>ios_pan_gesture</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_pan_gesture_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_pan_gesture_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_pan_gesture_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports iOSPanGestureRecognizerPage.xaml (+ .xaml.cs) The C# iOSPanGestureRecognizerPage is a StackLayout with: a bold message Label (_messageLabel), a &amp;quot;Toggle Simultaneous Gesture Recognition&amp;quot; Button, and a grouped ListView of employees whos

#### 🟢 Sonnet 5 Review

Pan coordinates text, toggle button, and status labels all match MAUI exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 98. Ios Picker — 🟢/⏳
<sub>ios_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_picker_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_picker_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports iOSPickerPage.xaml (+ iOSPickerPage.xaml.cs)

#### 🟢 Sonnet 5 Review

Picker label and toggle button render identically, matching layout, text, and colors.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 99. Ios Safe Area — 🟢/⏳
<sub>ios_safe_area</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_safe_area_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_safe_area_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_safe_area_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports iOSSafeAreaPage.xaml The .NET MAUI PlatformSpecifics sample (Pages/PlatformSpecifics/iOS/iOSSafeAreaPage.xaml + .xaml.cs): a long Lorem-ipsum Label over a &amp;quot;Disable Use Safe Area&amp;quot; button

#### 🟢 Sonnet 5 Review

Lorem ipsum paragraph and Disable Use Safe Area button match in text, layout, and color.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 100. Ios Scroll View — 🟢/⏳
<sub>ios_scroll_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_scroll_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_scroll_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_scroll_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports iOSScrollViewPage.xaml (+ iOSScrollViewPage.xaml.cs)

#### 🟢 Sonnet 5 Review

Slider and both action buttons match in position, sizing, and text.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 101. Ios Search Bar — 🟢/⏳
<sub>ios_search_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_search_bar_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_search_bar_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_search_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports iOSSearchBarPage.xaml (+ iOSSearchBarPage.xaml.cs)

#### 🟢 Sonnet 5 Review

Search bar with icon and two toggle buttons match closely in layout and styling.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 102. Ios Slider Update On Tap — 🟢/⏳
<sub>ios_slider_update_on_tap</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_slider_update_on_tap_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_slider_update_on_tap_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_slider_update_on_tap_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports iOSSliderUpdateOnTapPage.xaml (+ iOSSliderUpdateOnTapPage.xaml.cs)

#### 🟢 Sonnet 5 Review

Slider thumb position and toggle button match MAUI closely.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 103. Ios Swipe Transition — 🟢/⏳
<sub>ios_swipe_transition</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_swipe_transition_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_swipe_transition_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_swipe_transition_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports iOSSwipeViewTransitionModePage.xaml (+ .xaml.cs) The C# iOSSwipeViewTransitionModePage is a StackLayout with: a horizontal row holding a &amp;quot;SwipeTransitionMode:&amp;quot; Label + an EnumPicker over the SwipeTransitionMode enum (Reveal / Drag, Se

#### 🟢 Sonnet 5 Review

SwipeTransitionMode buttons, swipe box, and status text all match.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 104. Ios Time Picker — 🟢/⏳
<sub>ios_time_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_time_picker_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_time_picker_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_time_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports iOSTimePickerPage.xaml The .NET MAUI PlatformSpecifics sample (Pages/PlatformSpecifics/iOS/iOSTimePickerPage.xaml + .xaml.cs): a TimePicker carrying the iOSSpecific TimePicker.UpdateMode knob (XAML seeds it to WhenFinished) over a but

#### 🟢 Sonnet 5 Review

Time picker text field, toggle button, and status label match exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 105. Items — 🟢/⏳
<sub>items</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/items_light.png" /></td><td><img width="300px" src="captures/android/cpp/items_light.png" /></td><td><img width="300px" src="captures/android/xaml/items_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a self-contained demo page for the W2-19 items core: a collection_view over a live observable items source with a templated cell, single selection driving a readout label, and an EmptyView for the cleared state (the C# CollectionView galler

#### 🟢 Sonnet 5 Review

Task list items and 'Pick a task' text render identically.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 106. Items Updating Scroll Mode — 🟢/⏳
<sub>items_updating_scroll_mode</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/items_updating_scroll_mode_light.png" /></td><td><img width="300px" src="captures/android/cpp/items_updating_scroll_mode_light.png" /></td><td><img width="300px" src="captures/android/xaml/items_updating_scroll_mode_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ItemsUpdatingScrollModeGallery.xaml (+ .xaml.cs) of the C# CollectionView gallery (Maui.Controls.Sample.Pages.CollectionViewGalleries.ScrollModeGalleries.ItemsUpdatingScrollModeGallery)

#### 🟢 Sonnet 5 Review

Toggle buttons, Add Item button, and item list rows match content and layout (minor line-spacing looks slightly tighter in C++ but content identical).

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 107. Label — 🟢/⏳
<sub>label</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/label_light.png" /></td><td><img width="300px" src="captures/android/cpp/label_light.png" /></td><td><img width="300px" src="captures/android/xaml/label_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports LabelPage.xaml (+ LabelPage.xaml.cs)

#### 🟢 Sonnet 5 Review

All label formatting demos (colors, alignment, strikethrough, big font) match MAUI precisely.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 108. Layout Is Enabled — 🟡/⏳
<sub>layout_is_enabled</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/layout_is_enabled_light.png" /></td><td><img width="300px" src="captures/android/cpp/layout_is_enabled_light.png" /></td><td><img width="300px" src="captures/android/xaml/layout_is_enabled_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports LayoutIsEnabledPage.xaml (+ LayoutIsEnabledPage.xaml.cs) The C# page demonstrates how IsEnabled on a layout cascades to its children: a 2x2 grid whose left column hosts a &amp;quot;MainLayout&amp;quot; full of state-demo sub-stacks (all-enabled / all-d

#### 🟡 Sonnet 5 Review

Layout structure and text match, but the 'disabled' background/box colors are noticeably lighter/less saturated in the C++ render than MAUI's darker gray-blue tint.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 109. Line Gallery — 🟢/⏳
<sub>line_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/line_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/line_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/line_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports LineGallery.xaml A self-contained, code-first port of the MAUI Shapes LineGallery (Pages/Controls/ShapesGalleries/LineGallery.xaml + .xaml.cs)

#### 🟢 Sonnet 5 Review

Basic line, dash line, and stroke-thickness line all match in color, position, and style.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 110. Line Join Gallery — 🟢/⏳
<sub>line_join_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/line_join_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/line_join_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/line_join_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports LineJoinGallery.xaml A self-contained, code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/LineJoinGallery.xaml: a StackLayout that demonstrates the three StrokeLineJoin variants on an identical open polyline

#### 🟢 Sonnet 5 Review

Miter, bevel, and round line-join examples render identically in shape and color.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 111. Measure First Strategy — 🟢/⏳
<sub>measure_first_strategy</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/measure_first_strategy_light.png" /></td><td><img width="300px" src="captures/android/cpp/measure_first_strategy_light.png" /></td><td><img width="300px" src="captures/android/xaml/measure_first_strategy_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports MeasureFirstStrategy.xaml (+ .xaml.cs) of the C# CollectionView gallery (Maui.Controls.Sample.Pages.CollectionViewGalleries.GroupingGalleries.MeasureFirstStrategy)

#### 🟢 Sonnet 5 Review

CollectionView grouped list content, headers, and totals all match MAUI exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 112. Menu Bar — 🟢/⏳
<sub>menu_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/menu_bar_light.png" /></td><td><img width="300px" src="captures/android/cpp/menu_bar_light.png" /></td><td><img width="300px" src="captures/android/xaml/menu_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports MenuBarPage.xaml (+ MenuBarPage.cs) The C# page declares three page-level MenuBarItems (Page.MenuBarItems — the app menu bar) and a small visible body: - &amp;quot;Before File&amp;quot; : &amp;quot;Before File Action&amp;quot; (accelerator &amp;quot;b&amp;quot;), &amp;quot;Cool item 1&amp;quot;, a separat

#### 🟢 Sonnet 5 Review

Menu bar item toggle button and status text match exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 113. Modal — 🟢/⏳
<sub>modal</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/modal_light.png" /></td><td><img width="300px" src="captures/android/cpp/modal_light.png" /></td><td><img width="300px" src="captures/android/xaml/modal_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ModalPage.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 Review

Layout, buttons, and text match exactly; only status-bar time/theme differ trivially.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 114. Multiple Bound Selection — 🟢/⏳
<sub>multiple_bound_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/multiple_bound_selection_light.png" /></td><td><img width="300px" src="captures/android/cpp/multiple_bound_selection_light.png" /></td><td><img width="300px" src="captures/android/xaml/multiple_bound_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports MultipleBoundSelection.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.MultipleBoundSelection)

#### 🟢 Sonnet 5 Review

Selected items, orange highlighting, and layout match exactly between MAUI and C++.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 115. Navigation Gallery — 🟢/⏳
<sub>navigation_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/navigation_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/navigation_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/navigation_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports NavigationGallery.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 Review

All six buttons and header text match layout and wording exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 116. Nested Collection — 🟢/⏳
<sub>nested_collection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/nested_collection_light.png" /></td><td><img width="300px" src="captures/android/cpp/nested_collection_light.png" /></td><td><img width="300px" src="captures/android/xaml/nested_collection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports NestedGalleries/NestedCollectionViewGallery.xaml (+ NestedCollectionViewGallery.xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 Review

Nested CollectionViews with captions render identically in content and spacing.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 117. Pan Gesture Events — 🟡/⏳
<sub>pan_gesture_events</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/pan_gesture_events_light.png" /></td><td><img width="300px" src="captures/android/cpp/pan_gesture_events_light.png" /></td><td><img width="300px" src="captures/android/xaml/pan_gesture_events_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports PanGestureEventsGallery.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.PanGestureEventsGallery)

#### 🟡 Sonnet 5 Review

Green/red gesture blocks match in color and text, but the C++ render leaves a white gap below the red block where MAUI's red extends further down.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 118. Path Aspect Gallery — 🟢/⏳
<sub>path_aspect_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/path_aspect_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/path_aspect_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/path_aspect_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports PathAspectGallery.xaml A self-contained, code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PathAspectGallery.xaml: a StackLayout (Padding 12) that demonstrates the four Path Aspect modes on one identical ge

#### 🟢 Sonnet 5 Review

All four heart-icon aspect variants (None/Fill/Uniform/UniformToFill) match in size and color exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 119. Path Gallery — 🟢/⏳
<sub>path_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/path_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/path_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/path_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports PathGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PathGallery.xaml: a ScrollView over a StackLayout (Padding 12) that walks eight Path variants (plus two caption-only markup-string Labels

#### 🟢 Sonnet 5 Review

All path shapes (line, triangle, bezier, composite circles, overlapping rectangles, ellipse geometry) match precisely; only trailing content is scrolled off in both similarly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 120. Path Transform String — 🟢/⏳
<sub>path_transform_string</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/path_transform_string_light.png" /></td><td><img width="300px" src="captures/android/cpp/path_transform_string_light.png" /></td><td><img width="300px" src="captures/android/xaml/path_transform_string_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports PathTransformStringGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PathTransformStringGallery.xaml: a ScrollView over a StackLayout (Padding 12) that shows the SAME two-figure Path geometry

#### 🟢 Sonnet 5 Review

Both without and with RenderTransform triangle shapes match exactly in position and size.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 121. Picker — 🟡/⏳
<sub>picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/picker_light.png" /></td><td><img width="300px" src="captures/android/cpp/picker_light.png" /></td><td><img width="300px" src="captures/android/xaml/picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports PickerPage.xaml (+ PickerPage.xaml.cs) A self-contained, code-first demo page for the Picker control (the C# gallery-page convention, mirroring the value_controls_page / pickers_page pattern)

#### 🟡 Sonnet 5 Review

All picker rows and colors match, but the C++ render cuts off the final green Items(markup) section that is fully visible in the MAUI reference.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 122. Pickers — 🟢/⏳
<sub>pickers</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/pickers_light.png" /></td><td><img width="300px" src="captures/android/cpp/pickers_light.png" /></td><td><img width="300px" src="captures/android/xaml/pickers_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a self-contained demo page for the W1-06 picker set: picker, date_picker and time_picker on one vertical stack, wired together so every selection drives a visible output (the C# gallery-page convention, code-first; the value_controls_page p

#### 🟢 Sonnet 5 Review

Room/date/time picker fields match exactly; the date text differs only due to a different capture day (not a bug).

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 123. Pointer Gesture — 🟢/⏳
<sub>pointer_gesture</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/pointer_gesture_light.png" /></td><td><img width="300px" src="captures/android/cpp/pointer_gesture_light.png" /></td><td><img width="300px" src="captures/android/xaml/pointer_gesture_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports PointerGestureGalleryPage.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.PointerGestureGalleryPage)

#### 🟢 Sonnet 5 Review

All pointer-position labels and colors (yellow, green) match exactly between the two renders.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 124. Polygon Gallery — 🟢/⏳
<sub>polygon_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/polygon_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/polygon_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/polygon_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports PolygonGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PolygonGallery.xaml: a ScrollView over a StackLayout (Padding 12) that walks four Polygon variants, each under a caption Label — - &amp;quot;A

#### 🟢 Sonnet 5 Review

All four polygon examples (basic, dash, EvenOdd star, NonZero star) match exactly in shape and color.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 125. Polyline Gallery — 🟢/⏳
<sub>polyline_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/polyline_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/polyline_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/polyline_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports PolylineGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PolylineGallery.xaml: a StackLayout (Padding 12 — no ScrollView in the C# source) holding two Polyline variants, each under a caption

#### 🟢 Sonnet 5 Review

Basic and dash polyline examples match exactly in color and style.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 126. Preselected Item — 🟢/⏳
<sub>preselected_item</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/preselected_item_light.png" /></td><td><img width="300px" src="captures/android/cpp/preselected_item_light.png" /></td><td><img width="300px" src="captures/android/xaml/preselected_item_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports PreselectedItemGallery.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.PreselectedItemGallery)

#### 🟢 Sonnet 5 Review

Preselected orange row and full list content match exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 127. Preselected Items — 🟢/⏳
<sub>preselected_items</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/preselected_items_light.png" /></td><td><img width="300px" src="captures/android/cpp/preselected_items_light.png" /></td><td><img width="300px" src="captures/android/xaml/preselected_items_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports PreselectedItemsGallery.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.PreselectedItemsGallery)

#### 🟢 Sonnet 5 Review

Multiple preselected orange cells and full grid list match exactly, aside from trivial column-width wrapping differences.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 128. Progress Bar — 🟢/⏳
<sub>progress_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/progress_bar_light.png" /></td><td><img width="300px" src="captures/android/cpp/progress_bar_light.png" /></td><td><img width="300px" src="captures/android/xaml/progress_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ProgressBarPage.xaml (+ ProgressBarPage.xaml.cs)

#### 🟢 Sonnet 5 Review

Layout, colors, and progress bar states all match MAUI reference precisely.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 129. Radio Button Border — 🟢/⏳
<sub>radio_button_border</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/radio_button_border_light.png" /></td><td><img width="300px" src="captures/android/cpp/radio_button_border_light.png" /></td><td><img width="300px" src="captures/android/xaml/radio_button_border_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports RadioButtonBorder.xaml A self-contained, code-first demo of RadioButton border styling

#### 🟢 Sonnet 5 Review

Border colors, radio states, and text all match exactly between MAUI and C++.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 130. Radio Button Content — 🟡/⏳
<sub>radio_button_content</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/radio_button_content_light.png" /></td><td><img width="300px" src="captures/android/cpp/radio_button_content_light.png" /></td><td><img width="300px" src="captures/android/xaml/radio_button_content_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports RadioButtonContentGallery.xaml A self-contained, code-first demo of the RadioButton.Content surface

#### 🟡 Sonnet 5 Review

Content matches but the C++ render is missing the thin horizontal divider lines above/below the templated radio row.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 131. Radio Button Group — 🟢/⏳
<sub>radio_button_group</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/radio_button_group_light.png" /></td><td><img width="300px" src="captures/android/cpp/radio_button_group_light.png" /></td><td><img width="300px" src="captures/android/xaml/radio_button_group_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports RadioButtonGroupGallery.xaml A self-contained, code-first demo of the RadioButtonGroup ATTACHED-PROPERTY grouping: a vertical StackLayout carries RadioButtonGroup.GroupName=&amp;quot;foo&amp;quot;, so every descendant RadioButton — including one nested

#### 🟢 Sonnet 5 Review

Grid and stack layout radio buttons match exactly, including the grid-positioned Option D.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 132. Radio Button Group Binding — 🟢/⏳
<sub>radio_button_group_binding</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/radio_button_group_binding_light.png" /></td><td><img width="300px" src="captures/android/cpp/radio_button_group_binding_light.png" /></td><td><img width="300px" src="captures/android/xaml/radio_button_group_binding_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports RadioButtonGroupBindingGallery.xaml A code-first demo of binding the RadioButtonGroup attached properties (GroupName + SelectedValue) to a view-model

#### 🟢 Sonnet 5 Review

Bound radio group layout, labels, and action buttons match exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 133. Radio Button Group Gallery — 🟢/⏳
<sub>radio_button_group_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/radio_button_group_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/radio_button_group_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/radio_button_group_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports RadioButtonGroupGalleryPage.xaml A self-contained, code-first demo of RadioButton grouping SCOPE, mirroring the C# controls gallery page (Pages/Controls/RadioButtonGalleries/RadioButtonGroupGalleryPage.xaml)

#### 🟢 Sonnet 5 Review

All three grouped radio sections with group names render identically.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 134. Radio Content Properties — 🟢/⏳
<sub>radio_content_properties</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/radio_content_properties_light.png" /></td><td><img width="300px" src="captures/android/cpp/radio_content_properties_light.png" /></td><td><img width="300px" src="captures/android/xaml/radio_content_properties_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ContentProperties.xaml A self-contained, code-first demo of how RadioButton propagates the standard Text/Font properties to its Content

#### 🟢 Sonnet 5 Review

Custom text colors, fonts, and styled radio content all match precisely.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 135. Radio Template From Style — 🟢/⏳
<sub>radio_template_from_style</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/radio_template_from_style_light.png" /></td><td><img width="300px" src="captures/android/cpp/radio_template_from_style_light.png" /></td><td><img width="300px" src="captures/android/xaml/radio_template_from_style_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports TemplateFromStyle.xaml A self-contained, code-first demo of applying a RadioButton ControlTemplate

#### 🟢 Sonnet 5 Review

Custom card-style radio template with blue circle indicators matches exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 136. Rectangle Gallery — 🟢/⏳
<sub>rectangle_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/rectangle_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/rectangle_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/rectangle_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports RectangleGallery.xaml A self-contained, code-first port of the MAUI Shapes RectangleGallery (Pages/Controls/ShapesGalleries/RectangleGallery.xaml + .xaml.cs)

#### 🟢 Sonnet 5 Review

All rectangle shape variants (basic, square, stroke, dash, rounded corners) render identically.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 137. Refresh View — 🟢/⏳
<sub>refresh_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/refresh_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/refresh_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/refresh_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports RefreshViewPage.xaml (+ RefreshViewPage.xaml.cs + RefreshViewModel.cs) A self-contained, code-first demo page for the RefreshView control (the C# gallery-page convention, mirroring the swipe_refresh_page pattern)

#### 🟢 Sonnet 5 Review

RefreshView controls, labels, and state text match exactly between reference and port.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 138. Relative Layout — 🟢/⏳
<sub>relative_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/relative_layout_light.png" /></td><td><img width="300px" src="captures/android/cpp/relative_layout_light.png" /></td><td><img width="300px" src="captures/android/xaml/relative_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports RelativeLayoutPage.xaml

#### 🟢 Sonnet 5 Review

Corner-anchored colored boxes and centered nested rectangle layout match exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 139. Scattered Radio Button — 🟢/⏳
<sub>scattered_radio_button</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/scattered_radio_button_light.png" /></td><td><img width="300px" src="captures/android/cpp/scattered_radio_button_light.png" /></td><td><img width="300px" src="captures/android/xaml/scattered_radio_button_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ScatteredRadioButtonGallery.xaml A code-first demo that radio buttons DON&amp;#x27;T have to share a container to be grouped: grouping is by GroupName, so buttons scattered across separate containers (and one bare button outside any grouped co

#### 🟢 Sonnet 5 Review

Nested and grouped radio buttons across containers render identically, including the highlighted background row.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 140. Scroll Mode Test — 🟢/⏳
<sub>scroll_mode_test</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/scroll_mode_test_light.png" /></td><td><img width="300px" src="captures/android/cpp/scroll_mode_test_light.png" /></td><td><img width="300px" src="captures/android/xaml/scroll_mode_test_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ScrollModeTestGallery.xaml (+ .xaml.cs) of the C# CollectionView gallery (Maui.Controls.Sample.Pages.CollectionViewGalleries.ScrollModeGalleries.ScrollModeTestGallery)

#### 🟢 Sonnet 5 Review

ItemsUpdatingScrollMode picker, buttons, and item list content match exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 141. Scroll To Group — 🟢/⏳
<sub>scroll_to_group</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/scroll_to_group_light.png" /></td><td><img width="300px" src="captures/android/cpp/scroll_to_group_light.png" /></td><td><img width="300px" src="captures/android/xaml/scroll_to_group_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ScrollToGalleries/ScrollToGroup.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 Review

Form fields, buttons, and grouped superhero list content match; only the scroll viewport differs slightly which is expected scroll state.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 142. Scroll View — 🟢/⏳
<sub>scroll_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/scroll_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/scroll_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/scroll_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ScrollViewPage.xaml (+ the ScrollViewPages sub-demos: ScrollViewOrientationPage / ScrollToEndPage / ScrollToFromConstructorPage), code-first

#### 🟢 Sonnet 5 Review

Row list content and structure match; the two captures simply show different scroll positions, both valid states.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 143. Search Bar — 🟢/⏳
<sub>search_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/search_bar_light.png" /></td><td><img width="300px" src="captures/android/cpp/search_bar_light.png" /></td><td><img width="300px" src="captures/android/xaml/search_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports SearchBarPage.xaml (Microsoft.Maui.Controls sample gallery)

#### 🟢 Sonnet 5 Review

Text list with colors, placeholder, italic, and clear icons match exactly between MAUI and C++.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 144. Selection Command Param — 🟢/⏳
<sub>selection_command_param</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/selection_command_param_light.png" /></td><td><img width="300px" src="captures/android/cpp/selection_command_param_light.png" /></td><td><img width="300px" src="captures/android/xaml/selection_command_param_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports SelectionChangedCommandParameter.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.SelectionChangedCommandParameter)

#### 🟢 Sonnet 5 Review

Identical scrollable list of header/item text lines in both renders.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 145. Selection Synchronization — 🟢/⏳
<sub>selection_synchronization</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/selection_synchronization_light.png" /></td><td><img width="300px" src="captures/android/cpp/selection_synchronization_light.png" /></td><td><img width="300px" src="captures/android/xaml/selection_synchronization_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports SelectionSynchronization.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.SelectionSynchronization)

#### 🟢 Sonnet 5 Review

Orange-highlighted selected items (Item 2, Item 3) match exactly in both renders, only page-padding differs.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 146. Semantics — 🟢/⏳
<sub>semantics</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/semantics_light.png" /></td><td><img width="300px" src="captures/android/cpp/semantics_light.png" /></td><td><img width="300px" src="captures/android/xaml/semantics_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports SemanticsPage.xaml (+ SemanticsPage.xaml.cs) The C# SemanticsPage is an accessibility showcase: a long VerticalStackLayout where nearly every control carries SemanticProperties.Description / .Hint, plus a block of labels exercising Se

#### 🟢 Sonnet 5 Review

All labels, buttons, entry/editor fields, search bar, and heading levels render identically.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 147. Shadow Playground — 🟢/⏳
<sub>shadow_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/shadow_playground_light.png" /></td><td><img width="300px" src="captures/android/cpp/shadow_playground_light.png" /></td><td><img width="300px" src="captures/android/xaml/shadow_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ShadowPlaygroundPage.xaml A self-contained, code-first demo of the view Shadow surface, mirroring the C# core gallery page (Pages/Core/ShadowGalleries/ShadowPlaygroundPage.xaml + .xaml.cs)

#### 🟢 Sonnet 5 Review

Cyan box with red shadow, color fields, and sliders at matching values render identically.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 148. Shape App Theme — 🟢/⏳
<sub>shape_app_theme</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/shape_app_theme_light.png" /></td><td><img width="300px" src="captures/android/cpp/shape_app_theme_light.png" /></td><td><img width="300px" src="captures/android/xaml/shape_app_theme_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ShapeAppThemeGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/ShapeAppThemeGallery.xaml: a StackLayout (Padding 12) holding a caption Label and a 200x80 Rectangle, all themed via {AppThemeBi

#### 🟢 Sonnet 5 Review

Green rectangle shape and title text match exactly between both renders.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 149. Shapes — 🟢/⏳
<sub>shapes</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/shapes_light.png" /></td><td><img width="300px" src="captures/android/cpp/shapes_light.png" /></td><td><img width="300px" src="captures/android/xaml/shapes_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;shapes&amp;quot; demo (ComparePages.Shapes()), the shipped-.NET-MAUI reference for the visual-parity comparison: a ScrollView over a vertical stack of four LABELLED shapes, each bold-captioned and Start-a

#### 🟢 Sonnet 5 Review

Ellipse, round rectangle, pentagram polygon, and diagonal line all match in shape, color, and position.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 150. Single Bound Selection — 🟢/⏳
<sub>single_bound_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/single_bound_selection_light.png" /></td><td><img width="300px" src="captures/android/cpp/single_bound_selection_light.png" /></td><td><img width="300px" src="captures/android/xaml/single_bound_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports SingleBoundSelection.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.SingleBoundSelection)

#### 🟢 Sonnet 5 Review

Instruction text and country list match exactly between both renders.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 151. Slider — 🟢/⏳
<sub>slider</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/slider_light.png" /></td><td><img width="300px" src="captures/android/cpp/slider_light.png" /></td><td><img width="300px" src="captures/android/xaml/slider_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports SliderPage.xaml (+ .xaml.cs) Mirrors the MAUI gallery page: a vertical stack of headlined Slider states — Default, BackgroundColor (Blue), Background (yellow→green LinearGradientBrush), Minimum(5)/Maximum(15) with a value readout (Val

#### 🟢 Sonnet 5 Review

All slider variants (background color, gradient, disabled, custom track/thumb colors) match; C++ additionally shows more content below the fold due to less padding, consistent with allowed policy.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 152. Some Empty Groups — 🟢/⏳
<sub>some_empty_groups</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/some_empty_groups_light.png" /></td><td><img width="300px" src="captures/android/cpp/some_empty_groups_light.png" /></td><td><img width="300px" src="captures/android/xaml/some_empty_groups_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports GroupingGalleries/SomeEmptyGroups.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 Review

Grouped list with empty group headers and member counts match exactly in text and color.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 153. Stack Layout — 🟢/⏳
<sub>stack_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/stack_layout_light.png" /></td><td><img width="300px" src="captures/android/cpp/stack_layout_light.png" /></td><td><img width="300px" src="captures/android/xaml/stack_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports StackLayoutPage.xaml Demonstrates the generic maui::controls::stack_layout (the orientation-switching sibling of the fixed vertical/horizontal stacks) by nesting two inner stacks inside an outer vertical stack with a 12px margin: a &amp;quot;V

#### 🟢 Sonnet 5 Review

Vertical and horizontal colored-box stacks match exactly in color, size, and order.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 154. Staggered Layout — 🟢/⏳
<sub>staggered_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/staggered_layout_light.png" /></td><td><img width="300px" src="captures/android/cpp/staggered_layout_light.png" /></td><td><img width="300px" src="captures/android/xaml/staggered_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports AlternateLayoutGalleries/StaggeredLayout.xaml (+ StaggeredLayout.xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 Review

Staggered grid of numbered items shows the same masonry pattern; scroll offset differs slightly due to page padding but content matches.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 155. Stepper — 🔴/⏳
<sub>stepper</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/stepper_light.png" /></td><td><img width="300px" src="captures/android/cpp/stepper_light.png" /></td><td><img width="300px" src="captures/android/xaml/stepper_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports StepperPage.xaml (+ StepperPage.xaml.cs)

#### 🔴 Sonnet 5 Review

In MAUI the red BackgroundColor fills the full stepper row width, but in C++ the red background only covers the minus-button area, not the full row.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 156. Styles — 🟢/⏳
<sub>styles</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/styles_light.png" /></td><td><img width="300px" src="captures/android/cpp/styles_light.png" /></td><td><img width="300px" src="captures/android/xaml/styles_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports StylesPage.xaml

#### 🟢 Sonnet 5 Review

Base subtitle style, pink custom style, default style, and outlined button match exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 157. Swipe Gesture — 🟢/⏳
<sub>swipe_gesture</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/swipe_gesture_light.png" /></td><td><img width="300px" src="captures/android/cpp/swipe_gesture_light.png" /></td><td><img width="300px" src="captures/android/xaml/swipe_gesture_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports SwipeViewGestureRecognizerGallery.xaml (+ .xaml.cs) The MAUI SwipeViewGestureRecognizerGallery is a CollectionView of &amp;quot;message&amp;quot; rows; each row&amp;#x27;s DataTemplate is a SwipeView wired three ways, proving gesture recognizers AND swipe-item

#### 🟢 Sonnet 5 Review

C++ renders a clean card (title, date, description, TapCommand line) while the MAUI reference screenshot itself has garbled overlapping text (a broken/stale capture), so the port's content is correct.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 158. Swipe Item Position — 🟢/⏳
<sub>swipe_item_position</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/swipe_item_position_light.png" /></td><td><img width="300px" src="captures/android/cpp/swipe_item_position_light.png" /></td><td><img width="300px" src="captures/android/xaml/swipe_item_position_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports SwipeItemPositionGallery.xaml A code-first port of the MAUI SwipeView sub-gallery Pages/Controls/SwipeViewGalleries/SwipeItemPositionGallery.xaml: a 2-row Grid (Auto / *) with a Picker on top and one SwipeView below that carries TWO S

#### 🟢 Sonnet 5 Review

Reveal SwipeView with label and subtitle text render identically in both.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 159. Swipe Item Size — 🟢/⏳
<sub>swipe_item_size</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/swipe_item_size_light.png" /></td><td><img width="300px" src="captures/android/cpp/swipe_item_size_light.png" /></td><td><img width="300px" src="captures/android/xaml/swipe_item_size_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports SwipeItemSizeGallery.xaml A self-contained, code-first port of the .NET MAUI &amp;quot;SwipeItem Size Gallery&amp;quot;: a scrolling stack of swipe_views demonstrating how a left SwipeItem&amp;#x27;s icon size and the SwipeView content size interact

#### 🟢 Sonnet 5 Review

All differently-sized icon and SwipeView rows render matching gray bars and labels.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 160. Swipe Refresh — 🟢/⏳
<sub>swipe_refresh</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/swipe_refresh_light.png" /></td><td><img width="300px" src="captures/android/cpp/swipe_refresh_light.png" /></td><td><img width="300px" src="captures/android/xaml/swipe_refresh_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a self-contained demo page for the W2-20 swipe + refresh controls: a refresh_view wrapping a swipe_view (which itself wraps a labeled row), with a readout label reflecting the latest interaction

#### 🟢 Sonnet 5 Review

Header text and Ready status line match exactly between MAUI and C++.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 161. Swipe Threshold — 🟢/⏳
<sub>swipe_threshold</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/swipe_threshold_light.png" /></td><td><img width="300px" src="captures/android/cpp/swipe_threshold_light.png" /></td><td><img width="300px" src="captures/android/xaml/swipe_threshold_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports HorizontalSwipeThresholdGallery.xaml (+ .xaml.cs) The MAUI HorizontalSwipeThresholdGallery shows how SwipeView.Threshold (the swipe distance, in DIPs, the user must drag before the items settle open / execute) interacts with SwipeItem

#### 🟢 Sonnet 5 Review

Warning banner, section labels, purple bars, and sliders all match precisely.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 162. Swipe View Margin — 🟡/⏳
<sub>swipe_view_margin</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/swipe_view_margin_light.png" /></td><td><img width="300px" src="captures/android/cpp/swipe_view_margin_light.png" /></td><td><img width="300px" src="captures/android/xaml/swipe_view_margin_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports SwipeViewMarginGallery.xaml A self-contained, code-first port of the .NET MAUI &amp;quot;SwipeView Margin Gallery&amp;quot;: two swipe_views whose content&amp;#x27;s Margin + Padding are driven by two sliders, demonstrating that the revealed SwipeItems stay cor

#### 🟡 Sonnet 5 Review

Layout and colors match but body text renders in lighter/lower-contrast gray in the C++ version making it slightly washed out.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 163. Swipe View Shadow — 🟢/⏳
<sub>swipe_view_shadow</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/swipe_view_shadow_light.png" /></td><td><img width="300px" src="captures/android/cpp/swipe_view_shadow_light.png" /></td><td><img width="300px" src="captures/android/xaml/swipe_view_shadow_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports SwipeViewShadowGallery.xaml A code-first port of the MAUI SwipeView sub-gallery Pages/Controls/SwipeViewGalleries/SwipeViewShadowGallery.xaml: a padded vertical StackLayout proving a drop Shadow renders correctly on SwipeView content

#### 🟢 Sonnet 5 Review

Rounded bordered content boxes with shadow render identically in both.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 164. Switch — 🟢/⏳
<sub>switch</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/switch_light.png" /></td><td><img width="300px" src="captures/android/cpp/switch_light.png" /></td><td><img width="300px" src="captures/android/xaml/switch_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports SwitchPage.xaml (+ .xaml.cs) Mirrors the MAUI gallery page: a vertical stack of headlined Switch states — Default, BackgroundColor (Blue), Background (a yellow→green LinearGradientBrush), Disabled, OnColor (Red), ThumbColor (Orange)

#### 🟢 Sonnet 5 Review

All switch states (default, background color/gradient, disabled, on-color, thumb-color) match colors and positions exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 165. Switch Grouping — 🟢/⏳
<sub>switch_grouping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/switch_grouping_light.png" /></td><td><img width="300px" src="captures/android/cpp/switch_grouping_light.png" /></td><td><img width="300px" src="captures/android/xaml/switch_grouping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports CollectionViewGalleries/GroupingGalleries/ SwitchGrouping.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 Review

Grouped list with headers, member names, and totals in orange/green match, differing only by minor scroll-position offset.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 166. Tabbed Flyout — 🔴/⏳
<sub>tabbed_flyout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/tabbed_flyout_light.png" /></td><td><img width="300px" src="captures/android/cpp/tabbed_flyout_light.png" /></td><td><img width="300px" src="captures/android/xaml/tabbed_flyout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a self-contained demo page for the W1-10 tabbed + flyout vertical: a flyout_page whose FLYOUT pane is a titled menu (two buttons selecting the detail&amp;#x27;s tabs + a &amp;quot;Toggle flyout&amp;quot; presenting/dismissing itself) and whose DETAIL pane is a tabbed

#### 🔴 Sonnet 5 Review

MAUI shows the flyout menu with Home/Settings/Toggle buttons and flyout-dismissed state, while C++ shows a bottom tab bar with Home/Settings tabs and different page content -- structurally different flyout navigation implementation.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 167. Templated View — 🟢/⏳
<sub>templated_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/templated_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/templated_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/templated_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports TemplatedViewPage.xaml The C# page contrasts a standard CardView control with a compact one driven by a ControlTemplate (&amp;quot;CardViewCompressed&amp;quot;) and a custom Rate control built entirely from a ControlTemplate + a heart PathGeometry

#### 🟢 Sonnet 5 Review

CardView and compact ControlTemplate cards with names and descriptions render identically.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 168. Time Picker — 🔴/⏳
<sub>time_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/time_picker_light.png" /></td><td><img width="300px" src="captures/android/cpp/time_picker_light.png" /></td><td><img width="300px" src="captures/android/xaml/time_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports TimePickerPage.xaml (+ TimePickerPage.xaml.cs)

#### 🔴 Sonnet 5 Review

The third gradient Background TimePicker renders a blue-to-cyan gradient in MAUI but a red-to-purple/pink gradient in C++, and the C++ page is also missing the lower Disabled/IsFocused rows visible in MAUI's scroll position.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 169. Title Bar — 🟢/⏳
<sub>title_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/title_bar_light.png" /></td><td><img width="300px" src="captures/android/cpp/title_bar_light.png" /></td><td><img width="300px" src="captures/android/xaml/title_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports TitleBarPage.xaml A self-contained, code-first demo of the TitleBar control

#### 🟢 Sonnet 5 Review

Content Options and Color Options columns with checkboxes, text fields, and buttons render identically.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 170. Toolbar — 🟢/⏳
<sub>toolbar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/toolbar_light.png" /></td><td><img width="300px" src="captures/android/cpp/toolbar_light.png" /></td><td><img width="300px" src="captures/android/xaml/toolbar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ToolbarPage.xaml (Maui.Controls.Sample.Pages.ToolbarPage)

#### 🟢 Sonnet 5 Review

All six toolbar action buttons and header text match exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 171. Transform Playground — 🟢/⏳
<sub>transform_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/transform_playground_light.png" /></td><td><img width="300px" src="captures/android/cpp/transform_playground_light.png" /></td><td><img width="300px" src="captures/android/xaml/transform_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports TransformPlaygroundGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/TransformPlaygroundGallery.xaml: a 50x50 Path rectangle (red fill, blue stroke 4) sits in a 200x200 light-grey panel; belo

#### 🟢 Sonnet 5 Review

Red/blue square, all transform sliders, and labels match exactly including slider positions.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 172. Transformations — 🟢/⏳
<sub>transformations</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/transformations_light.png" /></td><td><img width="300px" src="captures/android/cpp/transformations_light.png" /></td><td><img width="300px" src="captures/android/xaml/transformations_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports TransformationsPage.xaml (+ .xaml.cs) The MAUI TransformationsPage drives a single target view&amp;#x27;s render transforms from a column of knobs: Sliders for Scale / ScaleX / ScaleY (Maximum 10) and Rotation / RotationX / RotationY (Maximum

#### 🟢 Sonnet 5 Review

Scale/Rotation/Anchor/Translation sliders and buttons all match in layout, labels, and values.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 173. Triggers — 🟢/⏳
<sub>triggers</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/triggers_light.png" /></td><td><img width="300px" src="captures/android/cpp/triggers_light.png" /></td><td><img width="300px" src="captures/android/xaml/triggers_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports TriggersPage.xaml

#### 🟢 Sonnet 5 Review

Layout, text, and toggle button match the MAUI reference exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 174. Update Path Data — 🟢/⏳
<sub>update_path_data</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/update_path_data_light.png" /></td><td><img width="300px" src="captures/android/cpp/update_path_data_light.png" /></td><td><img width="300px" src="captures/android/xaml/update_path_data_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports UpdatePathDataGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/UpdatePathDataGallery.xaml: a 2-row Grid (RowSpacing 0) that proves a Path repaints when its Data geometry is replaced at runti

#### 🟢 Sonnet 5 Review

Path curve rendering and data label are identical to MAUI.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 175. Value Controls — ⏳/⏳
<sub>value_controls</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/android/xaml/value_controls_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

#### ⏳ Sonnet 5 Review

_Not yet reviewed._

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 176. Varied Size Selector — 🟢/⏳
<sub>varied_size_selector</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/varied_size_selector_light.png" /></td><td><img width="300px" src="captures/android/cpp/varied_size_selector_light.png" /></td><td><img width="300px" src="captures/android/xaml/varied_size_selector_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports DataTemplateSelectorGalleries/VariedSizeDataTemplateSelectorGallery.xaml (+ VariedSizeDataTemplateSelectorGallery.xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 Review

Coffee/Milk colored bands and text match reference exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 177. Vertical Stack — 🟢/⏳
<sub>vertical_stack</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/vertical_stack_light.png" /></td><td><img width="300px" src="captures/android/cpp/vertical_stack_light.png" /></td><td><img width="300px" src="captures/android/xaml/vertical_stack_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

Vertical Stack

#### 🟢 Sonnet 5 Review

Six colored squares stack identically to MAUI reference.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 178. Vertical Stack Layout — ⏳/⏳
<sub>vertical_stack_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="captures/android/xaml/vertical_stack_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

#### ⏳ Sonnet 5 Review

_Not yet reviewed._

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 179. Visual States — 🟢/⏳
<sub>visual_states</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/visual_states_light.png" /></td><td><img width="300px" src="captures/android/cpp/visual_states_light.png" /></td><td><img width="300px" src="captures/android/xaml/visual_states_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports VisualStatesPage.xaml

#### 🟢 Sonnet 5 Review

Entry, button, and text visual-state layout matches MAUI exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 180. Web View — 🟡/⏳
<sub>web_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/web_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/web_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/web_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a self-contained demo page for the W1-08 web_view vertical: a web_view loading a STATIC html_web_view_source (no network), back/forward/reload buttons over the handler-pushed CanGoBack/CanGoForward read-onlys, an &amp;quot;Eval 1+1&amp;quot; button driving t

#### 🟡 Sonnet 5 Review

C++ shows 'No navigation yet' status text instead of MAUI's navigated-event text, indicating the navigated-event callback text wasn't triggered/updated on load.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

### 181. Z Index — 🟢/⏳
<sub>z_index</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/z_index_light.png" /></td><td><img width="300px" src="captures/android/cpp/z_index_light.png" /></td><td><img width="300px" src="captures/android/xaml/z_index_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ZIndexPage.xaml (+ ZIndexPage.xaml.cs), code-first

#### 🟢 Sonnet 5 Review

Overlapping z-indexed colored label stack matches MAUI reference exactly.

#### ⏳ Gemini Review

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score

_Not yet computed — no automated pixel-diff score is recorded for this page yet._

</details>
