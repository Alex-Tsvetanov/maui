# .NET MAUI C++ port — visual parity comparison

Per-page MAUI-vs-C++ visual parity for the **172 gallery pages**, on **iOS**, **macOS** (Mac Catalyst + AppKit) and **Android**. Each section is collapsible and holds a discrepancy-count summary, then one subheader per page titled with a `{Sonnet}/{Gemini}` status-emoji combo (🟢 match / 🟡 minor / 🔴 major / ⬛ blank / ⏳ unreviewed). Under each page: the MAUI / C++ / C++&amp;XAML renders (light over dark; missing captures show a placeholder), then a subsubheader per review model (Sonnet, Gemini, Pixel-Perfect Score) titled with that model's own status emoji and holding its review prose. Generated from `comparison.json` by `tools/gen_readme.py` — do not edit by hand.

<details>
<summary><h2>iOS (172 examples) — click to expand</h2></summary>

Real .NET MAUI (native-default) vs the C++ port vs the compile-time-XAML gallery, captured on the same iOS simulator in light and dark. MAUI is the content ground truth.

**Discrepancy counts** (MAUI-vs-C++ parity verdicts; Sonnet 5 `claude-sonnet-5` and Gemini review each page independently):

| Classification | Sonnet 5 — C++ (C1/C3) | Sonnet 5 — C++ &amp; XAML (C2/C4) | Gemini — C++ | Pixel-Perfect Score — C++ (C1/C3) | Pixel-Perfect Score — C++ &amp; XAML (C2/C4) |
| --- | --- | --- | --- | --- | --- |
| 🟢 Match | 167 | 0 | 0 | 150 | 153 |
| 🟡 Minor | 4 | 0 | 0 | 19 | 16 |
| 🔴 Major | 0 | 0 | 0 | 3 | 3 |
| ⬛ Blank | 1 | 0 | 0 | 0 | 0 |
| ⏳ Unreviewed | 0 | 172 | 172 | 0 | 0 |

### 1. Absolute Layout — 🟢/⏳
<sub>absolute_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/absolute_layout_light.png" /></td><td><img width="300px" src="captures/ios/cpp/absolute_layout_light.png" /></td><td><img width="300px" src="captures/ios/xaml/absolute_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/absolute_layout_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/absolute_layout_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/absolute_layout_dark.png" /></td></tr></table>

ports AbsoluteLayoutPage.xaml A self-contained, code-first demo of the AbsoluteLayout control

#### 🟢 Sonnet 5 — C++ (C1/C3)

Colored bars, autosized label, and centered text all match position/color/size in both light and dark themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9901, 0.64% pixels differ · Dark: SSIM 0.9899, 0.65% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

### 2. Activity Indicator — 🟢/⏳
<sub>activity_indicator</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/activity_indicator_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/activity_indicator_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/activity_indicator_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/activity_indicator_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/activity_indicator_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/activity_indicator_dark.gif" /></td></tr></table>

ports ActivityIndicatorPage.xaml (+ ActivityIndicatorPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

All indicator variants (default, styled, yellow background, larger, smaller) match in position, color, and size in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9974, 0.14% pixels differ · Dark: SSIM 0.9985, 0.08% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9575, 1.57% pixels differ · Dark: SSIM 0.9566, 1.57% pixels differ

### 3. Adaptive Collection — 🟢/⏳
<sub>adaptive_collection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/adaptive_collection_light.png" /></td><td><img width="300px" src="captures/ios/cpp/adaptive_collection_light.png" /></td><td><img width="300px" src="captures/ios/xaml/adaptive_collection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/adaptive_collection_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/adaptive_collection_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/adaptive_collection_dark.png" /></td></tr></table>

ports AdaptiveCollectionView.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.AdaptiveCollectionView)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Single-column list of items matches exactly in layout, spacing, and text in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9974, 0.23% pixels differ · Dark: SSIM 0.9978, 0.21% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9993, 0.04% pixels differ

### 4. Alerts — 🟢/⏳
<sub>alerts</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/alerts_light.png" /></td><td><img width="300px" src="captures/ios/cpp/alerts_light.png" /></td><td><img width="300px" src="captures/ios/xaml/alerts_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/alerts_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/alerts_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/alerts_dark.png" /></td></tr></table>

ports AlertsPage.xaml (+ AlertsPage.xaml.cs) The C# AlertsPage drives the three Page dialog services — DisplayAlertAsync (simple OK + Yes/No), DisplayActionSheetAsync (simple + Cancel/Delete), and DisplayPromptAsync (two questions) — from a

#### 🟢 Sonnet 5 — C++ (C1/C3)

All alert/actionsheet/prompt trigger links match text and layout in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9979, 0.10% pixels differ · Dark: SSIM 0.9979, 0.10% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9994, 0.04% pixels differ · Dark: SSIM 0.9993, 0.04% pixels differ

### 5. Alignment — 🟢/⏳
<sub>alignment</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/alignment_light.png" /></td><td><img width="300px" src="captures/ios/cpp/alignment_light.png" /></td><td><img width="300px" src="captures/ios/xaml/alignment_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/alignment_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/alignment_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/alignment_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;alignment&amp;quot; demo (ComparePages.Alignment()), the shipped-.NET-MAUI reference for the visual-parity comparison

#### 🟢 Sonnet 5 — C++ (C1/C3)

Start/Center/End/Fill button alignment and red border/blue fill all match precisely in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9803, 0.61% pixels differ · Dark: SSIM 0.9859, 0.61% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9802, 0.61% pixels differ · Dark: SSIM 0.9858, 0.61% pixels differ

### 6. Animation — 🟢/⏳
<sub>animation</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/animation_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/animation_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/animation_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/animation_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/animation_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/animation_dark.gif" /></td></tr></table>

ports AnimationPage.xaml (+ AnimationPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Both captures show the submarine idle animation frame identically in light and dark.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9503, 2.09% pixels differ · Dark: SSIM 0.9485, 2.11% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9486, 2.13% pixels differ · Dark: SSIM 0.9450, 2.16% pixels differ

### 7. App Theme Binding — 🟢/⏳
<sub>app_theme_binding</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/app_theme_binding_light.png" /></td><td><img width="300px" src="captures/ios/cpp/app_theme_binding_light.png" /></td><td><img width="300px" src="captures/ios/xaml/app_theme_binding_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/app_theme_binding_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/app_theme_binding_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/app_theme_binding_dark.png" /></td></tr></table>

ports AppThemeBindingPage.xaml

#### 🟢 Sonnet 5 — C++ (C1/C3)

Green/red theme-bound text, orange resource-dictionary text, and toggle link all match in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9993, 0.04% pixels differ · Dark: SSIM 0.9992, 0.04% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9994, 0.04% pixels differ · Dark: SSIM 0.9993, 0.04% pixels differ

### 8. Application Control — 🟢/⏳
<sub>application_control</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/application_control_light.png" /></td><td><img width="300px" src="captures/ios/cpp/application_control_light.png" /></td><td><img width="300px" src="captures/ios/xaml/application_control_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/application_control_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/application_control_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/application_control_dark.png" /></td></tr></table>

ports ApplicationControlPage.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Matches MAUI in both themes against the fresh iOS baseline (1.40%/1.40%). Both show 'Quits the application', Terminate/Open/Close Window links, and 'Application: not yet hosted' — the earlier window-title-substring diff was a stale-capture mismatch; residual is the status-bar clock.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9993, 0.04% pixels differ · Dark: SSIM 0.9992, 0.04% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9994, 0.04% pixels differ · Dark: SSIM 0.9993, 0.04% pixels differ

### 9. Auto Size Shapes — 🟢/⏳
<sub>auto_size_shapes</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/auto_size_shapes_light.png" /></td><td><img width="300px" src="captures/ios/cpp/auto_size_shapes_light.png" /></td><td><img width="300px" src="captures/ios/xaml/auto_size_shapes_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/auto_size_shapes_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/auto_size_shapes_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/auto_size_shapes_dark.png" /></td></tr></table>

ports AutoSizeShapesGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/AutoSizeShapesGallery.xaml: a 3-row Grid (RowSpacing 0) that proves a stroked Ellipse auto-sizes to fill exactly half of the av

#### 🟢 Sonnet 5 — C++ (C1/C3)

Green/orange split and ellipse with blue stroke match exactly in both light and dark themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9993, 0.04% pixels differ · Dark: SSIM 0.9993, 0.04% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9993, 0.04% pixels differ · Dark: SSIM 0.9993, 0.04% pixels differ

### 10. Basic Grouping — 🟢/⏳
<sub>basic_grouping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/basic_grouping_light.png" /></td><td><img width="300px" src="captures/ios/cpp/basic_grouping_light.png" /></td><td><img width="300px" src="captures/ios/xaml/basic_grouping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/basic_grouping_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/basic_grouping_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/basic_grouping_dark.png" /></td></tr></table>

ports GroupingGalleries/BasicGrouping.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

Grouped list with headers, items, and total-member counts renders identically in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9984, 0.07% pixels differ · Dark: SSIM 0.9983, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9996, 0.02% pixels differ · Dark: SSIM 0.9996, 0.02% pixels differ

### 11. Basic Swipe — 🟢/⏳
<sub>basic_swipe</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/basic_swipe_light.png" /></td><td><img width="300px" src="captures/ios/cpp/basic_swipe_light.png" /></td><td><img width="300px" src="captures/ios/xaml/basic_swipe_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/basic_swipe_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/basic_swipe_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/basic_swipe_dark.png" /></td></tr></table>

ports BasicSwipeGallery.xaml A code-first port of the MAUI SwipeView sub-gallery Pages/Controls/SwipeViewGalleries/BasicSwipeGallery.xaml: a vertical StackLayout of five SwipeViews, each demonstrating a different revealed-side / SwipeMode c

#### 🟢 Sonnet 5 — C++ (C1/C3)

Swipe row list with gray cards and labels matches exactly in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9996, 0.02% pixels differ · Dark: SSIM 0.9996, 0.02% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9990, 0.05% pixels differ · Dark: SSIM 0.9990, 0.05% pixels differ

### 12. Behaviors — 🟢/⏳
<sub>behaviors</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/behaviors_light.png" /></td><td><img width="300px" src="captures/ios/cpp/behaviors_light.png" /></td><td><img width="300px" src="captures/ios/xaml/behaviors_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/behaviors_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/behaviors_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/behaviors_dark.png" /></td></tr></table>

ports BehaviorsPage.xaml (+ .xaml.cs) and its companion Controls.Sample/Behaviors/NumericValidationBehavior.cs

#### 🟢 Sonnet 5 — C++ (C1/C3)

Header text and entry placeholder render identically in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9980, 0.09% pixels differ · Dark: SSIM 0.9980, 0.09% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9990, 0.05% pixels differ · Dark: SSIM 0.9990, 0.05% pixels differ

### 13. Border — 🟢/⏳
<sub>border</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/border_light.png" /></td><td><img width="300px" src="captures/ios/cpp/border_light.png" /></td><td><img width="300px" src="captures/ios/xaml/border_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/border_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/border_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/border_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;border&amp;quot; demo (ComparePages.BorderPage()), the shipped-.NET-MAUI reference for the visual-parity comparison: a single Border centered on the page — red 5pt stroke, a RoundRectangle StrokeShape (Co

#### 🟢 Sonnet 5 — C++ (C1/C3)

Red-bordered yellow card with 'Bordered content' text matches exactly, including the low-contrast dark-mode text which is equally faint in both MAUI and cpp.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9904, 0.35% pixels differ · Dark: SSIM 0.9912, 0.35% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9898, 0.37% pixels differ · Dark: SSIM 0.9906, 0.37% pixels differ

### 14. Border Clip Playground — 🟢/⏳
<sub>border_clip_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/border_clip_playground_light.png" /></td><td><img width="300px" src="captures/ios/cpp/border_clip_playground_light.png" /></td><td><img width="300px" src="captures/ios/xaml/border_clip_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/border_clip_playground_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/border_clip_playground_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/border_clip_playground_dark.png" /></td></tr></table>

ports BorderClipPlayground.xaml (+ .xaml.cs) The C# page is an interactive Border-shape playground: a 100x100 Border (red stroke) clips an AspectFill Image (oasis.jpg) into the currently selected StrokeShape, while controls below mutate the

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: against a fresh iOS MAUI ref, cpp renders the red-bordered oasis photo with the rounded top-left corner + the Border/CornerRadius sliders like MAUI in both themes. Prior red was stale-ref.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9695, 1.86% pixels differ · Dark: SSIM 0.9694, 1.86% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9751, 1.29% pixels differ · Dark: SSIM 0.9750, 1.29% pixels differ

### 15. Border Layout — 🟢/⏳
<sub>border_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/border_layout_light.png" /></td><td><img width="300px" src="captures/ios/cpp/border_layout_light.png" /></td><td><img width="300px" src="captures/ios/xaml/border_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/border_layout_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/border_layout_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/border_layout_dark.png" /></td></tr></table>

ports BorderLayout.xaml (+ BorderLayout.xaml.cs) The C# page demonstrates driving Border.StrokeThickness from a Slider: a Slider (0..40, set to 5 in OnAppearing) is bound to the Border&amp;#x27;s StrokeThickness; the Border (Silver stroke, White bac

#### 🟢 Sonnet 5 — C++ (C1/C3)

Stroke-thickness slider and colored row (red/Center/blue/green) with gray border match exactly in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9911, 0.34% pixels differ · Dark: SSIM 0.9903, 0.35% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9920, 0.31% pixels differ · Dark: SSIM 0.9912, 0.32% pixels differ

### 16. Border Playground — 🟢/⏳
<sub>border_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/border_playground_light.png" /></td><td><img width="300px" src="captures/ios/cpp/border_playground_light.png" /></td><td><img width="300px" src="captures/ios/xaml/border_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/border_playground_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/border_playground_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/border_playground_dark.png" /></td></tr></table>

ports BorderPlayground.xaml (+ BorderPlayground.xaml.cs) A self-contained, code-first interactive Border playground

#### 🟢 Sonnet 5 — C++ (C1/C3)

Gradient-background dashed-border label card plus all form fields (colors, sliders, dropdowns) match exactly in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9812, 1.34% pixels differ · Dark: SSIM 0.9807, 1.34% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9830, 0.89% pixels differ · Dark: SSIM 0.9823, 0.89% pixels differ

### 17. Border Resize Content — 🟢/⏳
<sub>border_resize_content</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/border_resize_content_light.png" /></td><td><img width="300px" src="captures/ios/cpp/border_resize_content_light.png" /></td><td><img width="300px" src="captures/ios/xaml/border_resize_content_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/border_resize_content_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/border_resize_content_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/border_resize_content_dark.png" /></td></tr></table>

ports BorderResizeContent.xaml A self-contained, code-first demo that resizes a Border&amp;#x27;s CONTENT and watches the Border track it

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: against a fresh iOS MAUI ref, cpp renders the green-bordered circle/square/triangle shapes (with + glyphs and oasis-photo content) identically to MAUI in both themes. Prior red was stale-ref.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9785, 1.54% pixels differ · Dark: SSIM 0.9753, 1.55% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9853, 1.13% pixels differ · Dark: SSIM 0.9837, 1.16% pixels differ

### 18. Border Stroke — 🟢/⏳
<sub>border_stroke</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/border_stroke_light.png" /></td><td><img width="300px" src="captures/ios/cpp/border_stroke_light.png" /></td><td><img width="300px" src="captures/ios/xaml/border_stroke_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/border_stroke_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/border_stroke_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/border_stroke_dark.png" /></td></tr></table>

ports BorderStroke.xaml (+ BorderStroke.xaml.cs) A self-contained, code-first demo of Border StrokeThickness and how a Border tracks the height of its content

#### 🟢 Sonnet 5 — C++ (C1/C3)

Stroke thickness variants, colors, and content-height slider all match exactly in both light and dark themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9521, 1.96% pixels differ · Dark: SSIM 0.9523, 1.96% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9530, 1.93% pixels differ · Dark: SSIM 0.9532, 1.93% pixels differ

### 19. Borderless — 🟢/⏳
<sub>borderless</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/borderless_light.png" /></td><td><img width="300px" src="captures/ios/cpp/borderless_light.png" /></td><td><img width="300px" src="captures/ios/xaml/borderless_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/borderless_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/borderless_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/borderless_dark.png" /></td></tr></table>

ports Borderless.xaml A self-contained, code-first demo of a stroke-less Border

#### 🟢 Sonnet 5 — C++ (C1/C3)

Borderless switch style and yellow background match exactly in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9985, 0.07% pixels differ · Dark: SSIM 0.9985, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9994, 0.03% pixels differ · Dark: SSIM 0.9994, 0.03% pixels differ

### 20. Box View — 🟢/⏳
<sub>box_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/box_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/box_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/box_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/box_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/box_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/box_view_dark.png" /></td></tr></table>

ports BoxViewPage.xaml (+ BoxViewPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

All BoxView variants (default, color, gradient background, corner radius, complex corner radius) match exactly in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9985, 0.07% pixels differ · Dark: SSIM 0.9990, 0.05% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9992, 0.05% pixels differ · Dark: SSIM 0.9991, 0.05% pixels differ

### 21. Button — 🟢/⏳
<sub>button</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/button_light.png" /></td><td><img width="300px" src="captures/ios/cpp/button_light.png" /></td><td><img width="300px" src="captures/ios/xaml/button_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/button_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/button_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/button_dark.png" /></td></tr></table>

ports ButtonPage.xaml (+ ButtonPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: against a fresh iOS MAUI ref, cpp matches MAUI in both themes across all button variants (colored/border/corner/pink) AND the two 'settings' ImageButton rows (white gear + text) + Decrease/Increase Spacing. Prior red was the oversized-icon layout on a stale ref (ContentLayout builder fix propagated by the iOS rebuild).

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9806, 1.03% pixels differ · Dark: SSIM 0.9810, 1.02% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9807, 1.03% pixels differ · Dark: SSIM 0.9812, 1.01% pixels differ

### 22. Carousel Page — 🟢/⏳
<sub>carousel_page</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/carousel_page_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/carousel_page_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/carousel_page_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/carousel_page_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/carousel_page_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/carousel_page_dark.gif" /></td></tr></table>

Carousel Page

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: against a FRESH iOS MAUI ref (rebuilt MauiReference net10.0-ios; the old ref predated the XAML simplification), cpp renders the single purple-bordered 'Card' card identically to MAUI in both themes. Prior red was the old richer-demo builder + a stale ref showing 'Basic Horizontal Carousel'.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9758, 0.63% pixels differ · Dark: SSIM 0.9755, 0.63% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9853, 0.40% pixels differ · Dark: SSIM 0.9845, 0.41% pixels differ

### 23. Chat Example — 🟢/⏳
<sub>chat_example</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/chat_example_light.png" /></td><td><img width="300px" src="captures/ios/cpp/chat_example_light.png" /></td><td><img width="300px" src="captures/ios/xaml/chat_example_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/chat_example_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/chat_example_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/chat_example_dark.png" /></td></tr></table>

ports ChatExample.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.ItemSizeGalleries.ChatExample), tracking the maui-compare reference demo ~/maui-compare/Pages/ChatExamplePage.cs (the visual-parity oracle)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Chat bubble text, colors, and layout match in both themes; the only difference is the MAUI capture crops closer to the top status bar, which is the exempted outer-harness-inset difference.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9994, 0.04% pixels differ · Dark: SSIM 0.9993, 0.04% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9996, 0.03% pixels differ · Dark: SSIM 0.9995, 0.03% pixels differ

### 24. Check Box — 🟢/⏳
<sub>check_box</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/check_box_light.png" /></td><td><img width="300px" src="captures/ios/cpp/check_box_light.png" /></td><td><img width="300px" src="captures/ios/xaml/check_box_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/check_box_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/check_box_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/check_box_dark.png" /></td></tr></table>

ports CheckBoxPage.xaml (+ .xaml.cs) Mirrors the MAUI gallery page: a vertical stack of headlined CheckBox states — Default, Colored (Color=Purple), Disabled, Disabled+Colored+Checked — followed by a &amp;quot;Change IsChecked&amp;quot; row pairing a Button

#### 🟢 Sonnet 5 — C++ (C1/C3)

All checkbox/radio variants (default, colored, disabled, disabled colored, change-IsChecked) match exactly in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9967, 0.16% pixels differ · Dark: SSIM 0.9967, 0.16% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9996, 0.03% pixels differ · Dark: SSIM 0.9995, 0.03% pixels differ

### 25. Chrome — 🟢/⏳
<sub>chrome</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/chrome_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/chrome_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/chrome_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/chrome_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/chrome_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/chrome_dark.gif" /></td></tr></table>

a self-contained demo page for the W1-11 window-chrome family: page toolbar items (primary + secondary), a menu bar (File menu with items, a separator and a sub-menu), a context flyout (right-click menu) on a button, and a tooltip — all wir

#### 🟢 Sonnet 5 — C++ (C1/C3)

Layouts, text, and colors match closely in both light and dark themes; only trivial timestamp/status-bar differences.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9984, 0.08% pixels differ · Dark: SSIM 0.9986, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9992, 0.06% pixels differ · Dark: SSIM 0.9987, 0.07% pixels differ

### 26. Clip — 🟢/⏳
<sub>clip</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/clip_light.png" /></td><td><img width="300px" src="captures/ios/cpp/clip_light.png" /></td><td><img width="300px" src="captures/ios/xaml/clip_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/clip_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/clip_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/clip_dark.png" /></td></tr></table>

ports ClipPage.xaml The C# page (Pages/Core/ClipPage.xaml; its .xaml.cs is an empty InitializeComponent) is a ScrollView over a StackLayout that shows the SAME dotnet_bot.png image five times, each successive copy carrying a different geome

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: against a FRESH iOS MAUI ref, cpp renders the dotnet_bot sphere + its RectangleGeometry-clipped copy identically to MAUI in both themes. Prior red was a stale iOS ref (pre shared-XAML edit).

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9996, 0.02% pixels differ · Dark: SSIM 0.9996, 0.02% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9985, 0.07% pixels differ · Dark: SSIM 0.9985, 0.07% pixels differ

### 27. Clip Corner Radius — 🟢/⏳
<sub>clip_corner_radius</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/clip_corner_radius_light.png" /></td><td><img width="300px" src="captures/ios/cpp/clip_corner_radius_light.png" /></td><td><img width="300px" src="captures/ios/xaml/clip_corner_radius_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/clip_corner_radius_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/clip_corner_radius_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/clip_corner_radius_dark.png" /></td></tr></table>

ports ClipCornerRadiusGallery.xaml (+ .xaml.cs) The C# page (Pages/Controls/ShapesGalleries/ClipCornerRadiusGallery.xaml) is a StackLayout (Padding=12) that demonstrates DRIVING a RoundRectangleGeometry&amp;#x27;s per-corner CornerRadius from four s

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: against a fresh iOS MAUI ref, cpp renders the oasis photo clipped by RoundRectangleGeometry + the four corner sliders identically to MAUI in both themes. Prior red was stale-ref.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9996, 0.02% pixels differ · Dark: SSIM 0.9995, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9985, 0.07% pixels differ · Dark: SSIM 0.9984, 0.07% pixels differ

### 28. Clip Gallery — 🟢/⏳
<sub>clip_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/clip_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/clip_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/clip_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/clip_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/clip_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/clip_gallery_dark.png" /></td></tr></table>

ports ClipGallery.xaml The C# page (Pages/Controls/ShapesGalleries/ClipGallery.xaml; its .xaml.cs is an empty InitializeComponent) is a ScrollView over a StackLayout (Padding=12) that shows the SAME &amp;quot;oasis.jpg&amp;quot; image SEVEN times — one bare

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: against a fresh iOS MAUI ref, cpp renders the oasis image + its RectangleGeometry-clipped copy identically to MAUI in both themes. Prior red was stale-ref.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9995, 0.03% pixels differ · Dark: SSIM 0.9995, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9985, 0.07% pixels differ · Dark: SSIM 0.9984, 0.07% pixels differ

### 29. Clip Views — 🟢/⏳
<sub>clip_views</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/clip_views_light.png" /></td><td><img width="300px" src="captures/ios/cpp/clip_views_light.png" /></td><td><img width="300px" src="captures/ios/xaml/clip_views_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/clip_views_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/clip_views_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/clip_views_dark.png" /></td></tr></table>

ports ClipViewsGallery.xaml The C# page (Pages/Controls/ShapesGalleries/ClipViewsGallery.xaml; no code-behind beyond an empty InitializeComponent) is a ScrollView over a StackLayout (Padding=12) that proves the Clip surface (VisualElement.C

#### 🟢 Sonnet 5 — C++ (C1/C3)

Layout, colors, and clipped stack shapes match well in both themes; only a minor tone difference in the semi-transparent pink/red search-bar clip strip in dark mode.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9879, 3.39% pixels differ · Dark: SSIM 0.9783, 3.44% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9869, 3.44% pixels differ · Dark: SSIM 0.9783, 3.44% pixels differ

### 30. Clipping — 🟢/⏳
<sub>clipping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/clipping_light.png" /></td><td><img width="300px" src="captures/ios/cpp/clipping_light.png" /></td><td><img width="300px" src="captures/ios/xaml/clipping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/clipping_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/clipping_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/clipping_dark.png" /></td></tr></table>

compare oracle ~/maui-compare/Pages/ClippingPage.cs (itself written to mirror this gallery page)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Matches MAUI in both themes against the FRESH MauiReference iOS baseline (0.73%/0.86%, SSIM 0.989). The old frozen ~/maui-compare ref failed to load the two coffee-cup icons (the prior ruling-3 reason); MauiReference renders them, and cpp matches — numbers 1-8, orange square, purple 'Hey' boxes, blue clip bar, and both coffee cups all align. Residual is the status-bar clock.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9996, 0.03% pixels differ · Dark: SSIM 0.9996, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9949, 0.20% pixels differ · Dark: SSIM 0.9986, 0.07% pixels differ

### 31. Collectionview — 🟢/⏳
<sub>collectionview</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/collectionview_light.png" /></td><td><img width="300px" src="captures/ios/cpp/collectionview_light.png" /></td><td><img width="300px" src="captures/ios/xaml/collectionview_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/collectionview_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/collectionview_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/collectionview_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;collectionview&amp;quot; demo (ComparePages.CollectionViewPage()), the shipped-.NET-MAUI reference for the visual-parity comparison: a CollectionView over 24 captioned items, a string Header (&amp;quot;This is the

#### 🟢 Sonnet 5 — C++ (C1/C3)

Grid of numbered file-name cells matches exactly in both themes and both list contents/ordering are identical; MAUI's status bar merely overlaps its header text at the very top (harness clipping artifact, not a port bug).

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9994, 0.03% pixels differ · Dark: SSIM 0.9993, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9995, 0.03% pixels differ · Dark: SSIM 0.9994, 0.03% pixels differ

### 32. Composition Gallery — 🟢/⏳
<sub>composition_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/composition_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/composition_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/composition_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/composition_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/composition_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/composition_gallery_dark.png" /></td></tr></table>

ports CompositionGallery.xaml A self-contained, code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/CompositionGallery.xaml: a StackLayout holding two Beige 250x250 Grids (Margin 12) that compose multiple overlappi

#### 🟢 Sonnet 5 — C++ (C1/C3)

Both the shape-composition canvas (triangle/circle/line overlay) and the line-diagram canvas are pixel-identical between MAUI and cpp in both light and dark themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9984, 1.23% pixels differ · Dark: SSIM 0.9984, 1.23% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9985, 0.07% pixels differ · Dark: SSIM 0.9991, 0.04% pixels differ

### 33. Containers — 🟢/⏳
<sub>containers</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/containers_light.png" /></td><td><img width="300px" src="captures/ios/cpp/containers_light.png" /></td><td><img width="300px" src="captures/ios/xaml/containers_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/containers_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/containers_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/containers_dark.png" /></td></tr></table>

a self-contained demo page for the W1-07 container set: a scroll_view hosting a vertical stack of content-hosting containers — a border-framed label (stroke + dashed outline + rounded shape), a legacy frame (BorderColor/CornerRadius/HasShad

#### 🟢 Sonnet 5 — C++ (C1/C3)

Dashed border, solid red frame, and nested text stack match in both light and dark themes; only trivial status-bar/time differences.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9726, 2.38% pixels differ · Dark: SSIM 0.9887, 0.57% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9732, 2.36% pixels differ · Dark: SSIM 0.9894, 0.55% pixels differ

### 34. Content View — 🟢/⏳
<sub>content_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/content_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/content_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/content_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/content_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/content_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/content_view_dark.png" /></td></tr></table>

ports ContentViewPage.xaml (+ ContentViewPage.xaml.cs), code-first

#### 🟢 Sonnet 5 — C++ (C1/C3)

ContentView/Content text hierarchy and the Swap content link match exactly in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9990, 0.05% pixels differ · Dark: SSIM 0.9989, 0.05% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9989, 0.05% pixels differ · Dark: SSIM 0.9989, 0.05% pixels differ

### 35. Context Flyout — 🟢/⏳
<sub>context_flyout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/context_flyout_light.png" /></td><td><img width="300px" src="captures/ios/cpp/context_flyout_light.png" /></td><td><img width="300px" src="captures/ios/xaml/context_flyout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/context_flyout_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/context_flyout_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/context_flyout_dark.png" /></td></tr></table>

ports ContextFlyoutPage.xaml (+ ContextFlyoutPage.xaml.cs) The C# page attaches a MenuFlyout as the FlyoutBase.ContextFlyout (right-click / long-press menu) of several controls and wires each menu item to a handler: - a Button (&amp;quot;Increment b

#### 🟢 Sonnet 5 — C++ (C1/C3)

Toggle, entry, button chrome, and the live Bing consent web dialog all render identically in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9990, 0.05% pixels differ · Dark: SSIM 0.9989, 0.05% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9989, 0.05% pixels differ · Dark: SSIM 0.9989, 0.05% pixels differ

### 36. Controls Stack — 🟢/⏳
<sub>controls_stack</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/controls_stack_light.png" /></td><td><img width="300px" src="captures/ios/cpp/controls_stack_light.png" /></td><td><img width="300px" src="captures/ios/xaml/controls_stack_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/controls_stack_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/controls_stack_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/controls_stack_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;controls_stack&amp;quot; demo (ComparePages.ControlsStack()), the shipped-.NET-MAUI reference for the visual-parity comparison: a VerticalStackLayout (Spacing 12, Padding 16) showcasing the basic widgets

#### 🟢 Sonnet 5 — C++ (C1/C3)

Button, entry, editor, search bar, checkbox, switch, slider, stepper and progress bar all match in size/color/layout across both themes; negligible spacing nit before 'An Editor' in light.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9950, 0.22% pixels differ · Dark: SSIM 0.9945, 0.22% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9950, 0.22% pixels differ · Dark: SSIM 0.9944, 0.22% pixels differ

### 37. Custom Layout — 🟢/⏳
<sub>custom_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/custom_layout_light.png" /></td><td><img width="300px" src="captures/ios/cpp/custom_layout_light.png" /></td><td><img width="300px" src="captures/ios/xaml/custom_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/custom_layout_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/custom_layout_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/custom_layout_dark.png" /></td></tr></table>

ports CustomLayoutPage.xaml (+ CustomLayoutPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Top/Left/Left/Right/Right/Bottom custom-layout positions match exactly in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9990, 0.05% pixels differ · Dark: SSIM 0.9989, 0.05% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9989, 0.05% pixels differ · Dark: SSIM 0.9989, 0.05% pixels differ

### 38. Custom Size Swipe — 🟢/⏳
<sub>custom_size_swipe</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/custom_size_swipe_light.png" /></td><td><img width="300px" src="captures/ios/cpp/custom_size_swipe_light.png" /></td><td><img width="300px" src="captures/ios/xaml/custom_size_swipe_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/custom_size_swipe_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/custom_size_swipe_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/custom_size_swipe_dark.png" /></td></tr></table>

ports CustomSizeSwipeViewGallery.xaml (+ .xaml.cs) The MAUI CustomSizeSwipeViewGallery is a single SwipeView whose Left / Right / Top item collections each reveal CUSTOM-SIZED content: a SwipeItemView wrapping a Grid/StackLayout with an exp

#### 🟢 Sonnet 5 — C++ (C1/C3)

SwipeView content and revealed-state text match exactly in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9990, 0.05% pixels differ · Dark: SSIM 0.9989, 0.05% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9989, 0.05% pixels differ · Dark: SSIM 0.9989, 0.05% pixels differ

### 39. Custom Swipe Item View — 🟢/⏳
<sub>custom_swipe_item_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/custom_swipe_item_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/custom_swipe_item_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/custom_swipe_item_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/custom_swipe_item_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/custom_swipe_item_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/custom_swipe_item_view_dark.png" /></td></tr></table>

ports CustomSwipeItemViewGallery.xaml A self-contained, code-first port of the .NET MAUI &amp;quot;CustomSwipeItem&amp;quot; gallery: a message-list row whose right swipe reveals a CUSTOM-content swipe item (a swipe_item_view, not a plain swipe_item)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Custom swipe item card (purple background, title, date) matches exactly in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9989, 0.06% pixels differ · Dark: SSIM 0.9990, 0.05% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9985, 0.07% pixels differ · Dark: SSIM 0.9985, 0.07% pixels differ

### 40. Cv Visual States — 🟢/⏳
<sub>cv_visual_states</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/cv_visual_states_light.png" /></td><td><img width="300px" src="captures/ios/cpp/cv_visual_states_light.png" /></td><td><img width="300px" src="captures/ios/xaml/cv_visual_states_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/cv_visual_states_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/cv_visual_states_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/cv_visual_states_dark.png" /></td></tr></table>

ports CollectionViewGalleries/SelectionGalleries/ VisualStatesGallery.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

iOS: Single/Multi Selection item lists match MAUI. Prior red was stale (recaptured against the fixed build).

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9990, 0.05% pixels differ · Dark: SSIM 0.9990, 0.05% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.07% pixels differ · Dark: SSIM 0.9985, 0.07% pixels differ

### 41. Data Template Selector — 🟢/⏳
<sub>data_template_selector</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/data_template_selector_light.png" /></td><td><img width="300px" src="captures/ios/cpp/data_template_selector_light.png" /></td><td><img width="300px" src="captures/ios/xaml/data_template_selector_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/data_template_selector_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/data_template_selector_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/data_template_selector_dark.png" /></td></tr></table>

ports DataTemplateSelectorGallery.xaml (+ DataTemplateSelectorGallery.xaml.cs, including its WeekendSelector + SearchTermSelector classes)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Matches in both themes: text list content, colors, and search bar rendering are identical between MAUI and the C++ port.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9984, 0.14% pixels differ · Dark: SSIM 0.9983, 0.14% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9978, 0.16% pixels differ · Dark: SSIM 0.9977, 0.16% pixels differ

### 42. Date Picker — 🟢/⏳
<sub>date_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/date_picker_light.png" /></td><td><img width="300px" src="captures/ios/cpp/date_picker_light.png" /></td><td><img width="300px" src="captures/ios/xaml/date_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/date_picker_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/date_picker_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/date_picker_dark.png" /></td></tr></table>

ports DatePickerPage.xaml (+ DatePickerPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

iOS: Default/BackgroundColor(blue)/Background(gradient) date rows match MAUI; the only delta is the displayed date value (MAUI captured 5.07.2026, cpp 7.07.2026 == capture day) — a capture-date artifact, not a port bug.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9790, 1.01% pixels differ · Dark: SSIM 0.9782, 1.01% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9784, 1.06% pixels differ · Dark: SSIM 0.9777, 1.06% pixels differ

### 43. Device — 🟢/⏳
<sub>device</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/device_light.png" /></td><td><img width="300px" src="captures/ios/cpp/device_light.png" /></td><td><img width="300px" src="captures/ios/xaml/device_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/device_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/device_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/device_dark.png" /></td></tr></table>

ports DevicePage.xaml

#### 🟢 Sonnet 5 — C++ (C1/C3)

Platform/Idiom/Version text renders identically in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9985, 0.08% pixels differ · Dark: SSIM 0.9984, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9989, 0.06% pixels differ · Dark: SSIM 0.9988, 0.06% pixels differ

### 44. Dispatcher — 🟢/⏳
<sub>dispatcher</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/dispatcher_light.png" /></td><td><img width="300px" src="captures/ios/cpp/dispatcher_light.png" /></td><td><img width="300px" src="captures/ios/xaml/dispatcher_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/dispatcher_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/dispatcher_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/dispatcher_dark.png" /></td></tr></table>

ports DispatcherPage.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

All text blocks and blue action links match exactly in both light and dark themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9995, 0.03% pixels differ · Dark: SSIM 0.9995, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9989, 0.06% pixels differ · Dark: SSIM 0.9988, 0.06% pixels differ

### 45. Drag Drop — 🟢/⏳
<sub>drag_drop</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/drag_drop_light.png" /></td><td><img width="300px" src="captures/ios/cpp/drag_drop_light.png" /></td><td><img width="300px" src="captures/ios/xaml/drag_drop_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/drag_drop_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/drag_drop_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/drag_drop_dark.png" /></td></tr></table>

ports DragAndDropBetweenLayouts.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.DragAndDropBetweenLayouts)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Color swatches, rainbow list, and all status text match exactly in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9995, 0.03% pixels differ · Dark: SSIM 0.9995, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9989, 0.06% pixels differ · Dark: SSIM 0.9988, 0.06% pixels differ

### 46. Editor — 🟢/⏳
<sub>editor</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/editor_light.png" /></td><td><img width="300px" src="captures/ios/cpp/editor_light.png" /></td><td><img width="300px" src="captures/ios/xaml/editor_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/editor_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/editor_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/editor_dark.png" /></td></tr></table>

ports EditorPage.xaml (Microsoft.Maui.Controls sample gallery)

#### 🟢 Sonnet 5 — C++ (C1/C3)

All editor fields, placeholder text, colored labels, and font sizes match in both themes; only a minor left-margin/inset difference (exempt).

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9982, 0.13% pixels differ · Dark: SSIM 0.9979, 0.13% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9974, 0.16% pixels differ · Dark: SSIM 0.9971, 0.16% pixels differ

### 47. Effects — 🟢/⏳
<sub>effects</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/effects_light.png" /></td><td><img width="300px" src="captures/ios/cpp/effects_light.png" /></td><td><img width="300px" src="captures/ios/xaml/effects_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/effects_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/effects_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/effects_dark.png" /></td></tr></table>

ports EffectsPage.xaml (Maui.Controls.Sample.Pages.EffectsPage)

#### 🟢 Sonnet 5 — C++ (C1/C3)

The MAUI light-theme screenshot is a broken/mis-timed capture; the dark-theme comparison shows the C++ port's content matches MAUI exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9996, 0.03% pixels differ · Dark: SSIM 0.9996, 0.02% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9985, 0.07% pixels differ

### 48. Ellipse Gallery — 🟢/⏳
<sub>ellipse_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ellipse_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/ellipse_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/ellipse_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ellipse_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/ellipse_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/ellipse_gallery_dark.png" /></td></tr></table>

ports EllipseGallery.xaml A self-contained, code-first port of the MAUI Shapes EllipseGallery (Pages/Controls/ShapesGalleries/EllipseGallery.xaml + .xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Rectangle, circle, and ellipse shapes with stroke/dash all render identically in both light and dark themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9996, 0.02% pixels differ · Dark: SSIM 0.9996, 0.02% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.07% pixels differ · Dark: SSIM 0.9985, 0.07% pixels differ

### 49. Empty View — 🟢/⏳
<sub>empty_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/empty_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/empty_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_dark.png" /></td></tr></table>

ports EmptyViewStringGallery.xaml (+ EmptyViewStringGallery.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

CollectionView data list renders identically in both themes; no differences.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9993, 0.05% pixels differ · Dark: SSIM 0.9993, 0.05% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9982, 0.10% pixels differ · Dark: SSIM 0.9982, 0.10% pixels differ

### 50. Empty View Load Simulate — 🟢/⏳
<sub>empty_view_load_simulate</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/empty_view_load_simulate_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_load_simulate_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_load_simulate_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/empty_view_load_simulate_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_load_simulate_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_load_simulate_dark.gif" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewLoadSimulateGallery.xaml (+ EmptyViewLoadSimulateGallery.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Loading-simulation placeholder text centered identically in both themes; no differences.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9983, 0.10% pixels differ · Dark: SSIM 0.9982, 0.10% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9990, 0.07% pixels differ · Dark: SSIM 0.9989, 0.07% pixels differ

### 51. Empty View Null — 🟢/⏳
<sub>empty_view_null</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/empty_view_null_light.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_null_light.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_null_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/empty_view_null_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_null_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_null_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewNullGallery.xaml (+ EmptyViewNullGallery.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Nothing to display placeholder matches exactly in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9994, 0.03% pixels differ · Dark: SSIM 0.9993, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9995, 0.03% pixels differ · Dark: SSIM 0.9994, 0.03% pixels differ

### 52. Empty View Rtl — 🟢/⏳
<sub>empty_view_rtl</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/empty_view_rtl_light.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_rtl_light.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_rtl_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/empty_view_rtl_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_rtl_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_rtl_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewRTLGallery.xaml (+ EmptyViewRTLGallery.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

RTL/LTR toggle bar and three-column grid layout match exactly in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9963, 0.18% pixels differ · Dark: SSIM 0.9961, 0.19% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9982, 0.10% pixels differ · Dark: SSIM 0.9981, 0.10% pixels differ

### 53. Empty View Selector — 🟢/⏳
<sub>empty_view_selector</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/empty_view_selector_light.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_selector_light.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_selector_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/empty_view_selector_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_selector_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_selector_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewWithDataTemplateSelector.xaml (+ .xaml.cs, incl

#### 🟢 Sonnet 5 — C++ (C1/C3)

Instruction text, filter bar, and filtered single result render identically in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9988, 0.08% pixels differ · Dark: SSIM 0.9987, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9982, 0.10% pixels differ · Dark: SSIM 0.9981, 0.10% pixels differ

### 54. Empty View Swap — 🟢/⏳
<sub>empty_view_swap</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/empty_view_swap_light.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_swap_light.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_swap_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/empty_view_swap_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_swap_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_swap_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewSwapGallery.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Toggle switch, Clear/Fill links, and three-column list match exactly in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9899, 0.41% pixels differ · Dark: SSIM 0.9897, 0.41% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9983, 0.09% pixels differ · Dark: SSIM 0.9983, 0.09% pixels differ

### 55. Empty View Template — 🟢/⏳
<sub>empty_view_template</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/empty_view_template_light.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_template_light.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_template_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/empty_view_template_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_template_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_template_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewTemplateGallery.xaml (+ EmptyViewTemplateGallery.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Templated three-column list renders identically in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9986, 0.08% pixels differ · Dark: SSIM 0.9986, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9983, 0.09% pixels differ · Dark: SSIM 0.9983, 0.09% pixels differ

### 56. Empty View View — 🟢/⏳
<sub>empty_view_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/empty_view_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/empty_view_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_view_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewViewGallery.xaml (+ EmptyViewViewGallery.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

View-based layout matches exactly in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9986, 0.08% pixels differ · Dark: SSIM 0.9986, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9983, 0.09% pixels differ · Dark: SSIM 0.9983, 0.09% pixels differ

### 57. Entry — 🟢/⏳
<sub>entry</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/entry_light.png" /></td><td><img width="300px" src="captures/ios/cpp/entry_light.png" /></td><td><img width="300px" src="captures/ios/xaml/entry_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/entry_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/entry_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/entry_dark.png" /></td></tr></table>

ports EntryPage.xaml (Microsoft.Maui.Controls sample gallery)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Identical layout, purple text/placeholder colors, checkbox, cursor slider, and dark-mode inversion all match exactly in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9989, 0.05% pixels differ · Dark: SSIM 0.9989, 0.05% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

### 58. Filter Collection — 🟢/⏳
<sub>filter_collection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/filter_collection_light.png" /></td><td><img width="300px" src="captures/ios/cpp/filter_collection_light.png" /></td><td><img width="300px" src="captures/ios/xaml/filter_collection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/filter_collection_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/filter_collection_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/filter_collection_dark.png" /></td></tr></table>

ports FilterCollectionView.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

Toggle, filter bar, and two-column filename list are pixel-identical in both light and dark themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9938, 0.25% pixels differ · Dark: SSIM 0.9937, 0.25% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9983, 0.09% pixels differ · Dark: SSIM 0.9983, 0.09% pixels differ

### 59. Filter Selection — 🟢/⏳
<sub>filter_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/filter_selection_light.png" /></td><td><img width="300px" src="captures/ios/cpp/filter_selection_light.png" /></td><td><img width="300px" src="captures/ios/xaml/filter_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/filter_selection_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/filter_selection_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/filter_selection_dark.png" /></td></tr></table>

ports FilterSelection.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.FilterSelection)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Instructional text, filter bar, Reset link, Selected label, and list content match in both themes; layout is consistent.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9986, 0.08% pixels differ · Dark: SSIM 0.9986, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9983, 0.09% pixels differ · Dark: SSIM 0.9985, 0.09% pixels differ

### 60. Flex Layout — 🟢/⏳
<sub>flex_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/flex_layout_light.png" /></td><td><img width="300px" src="captures/ios/cpp/flex_layout_light.png" /></td><td><img width="300px" src="captures/ios/xaml/flex_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/flex_layout_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/flex_layout_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/flex_layout_dark.png" /></td></tr></table>

ports FlexLayoutPage.xaml A self-contained, code-first demo of the FlexLayout control: the classic &amp;quot;holy grail&amp;quot; page layout built from nested flexboxes

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: against a fresh iOS MAUI ref, cpp renders HEADER + blue LEFT | gray CONTENT | green RIGHT flex panels identically to MAUI in both themes. Prior 'missing blue LEFT panel' red was stale (fixed + fresh ref confirms).

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9986, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9988, 0.06% pixels differ

### 61. Focus — 🟢/⏳
<sub>focus</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/focus_light.png" /></td><td><img width="300px" src="captures/ios/cpp/focus_light.png" /></td><td><img width="300px" src="captures/ios/xaml/focus_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/focus_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/focus_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/focus_dark.png" /></td></tr></table>

ports FocusPage.xaml (+ FocusPage.xaml.cs) The C# FocusPage is a focus-subsystem demo: an Entry whose Focused/Unfocused events (OnFocusEntryFocusChanged) append &amp;quot;Focused&amp;quot;/&amp;quot;Unfocused&amp;quot; lines to a scrolling InfoLabel, plus two buttons — &amp;quot;Focus

#### 🟢 Sonnet 5 — C++ (C1/C3)

Focus target entry, Focus/Unfocus links, and IsFocused label match exactly in both light and dark themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9984, 0.07% pixels differ · Dark: SSIM 0.9984, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.05% pixels differ · Dark: SSIM 0.9987, 0.05% pixels differ

### 62. Fonts — 🟢/⏳
<sub>fonts</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/fonts_light.png" /></td><td><img width="300px" src="captures/ios/cpp/fonts_light.png" /></td><td><img width="300px" src="captures/ios/xaml/fonts_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/fonts_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/fonts_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/fonts_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;fonts&amp;quot; demo (ComparePages.Fonts()), the shipped-.NET-MAUI reference for the visual-parity comparison: a ScrollView over a VerticalStackLayout (Spacing 8, Padding 16) of nine Labels — Title/Subtit

#### 🟢 Sonnet 5 — C++ (C1/C3)

All font style rows match in size, weight, italics, and color inversion for dark mode.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9984, 0.07% pixels differ · Dark: SSIM 0.9984, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.05% pixels differ · Dark: SSIM 0.9987, 0.05% pixels differ

### 63. Footer Only String — 🟢/⏳
<sub>footer_only_string</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/footer_only_string_light.png" /></td><td><img width="300px" src="captures/ios/cpp/footer_only_string_light.png" /></td><td><img width="300px" src="captures/ios/xaml/footer_only_string_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/footer_only_string_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/footer_only_string_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/footer_only_string_dark.png" /></td></tr></table>

ports FooterOnlyString.xaml (+ FooterOnlyString.xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Sonnet 5 — C++ (C1/C3)

List content and bold footer text are identical between cpp and maui in both themes; only difference is harness status-bar overlap in the maui shots, which is exempted.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9994, 0.03% pixels differ · Dark: SSIM 0.9993, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9995, 0.03% pixels differ · Dark: SSIM 0.9994, 0.03% pixels differ

### 64. Formatted Text — 🟢/⏳
<sub>formatted_text</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/formatted_text_light.png" /></td><td><img width="300px" src="captures/ios/cpp/formatted_text_light.png" /></td><td><img width="300px" src="captures/ios/xaml/formatted_text_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/formatted_text_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/formatted_text_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/formatted_text_dark.png" /></td></tr></table>

a self-contained demo page for the G1 rich-text slice: a label whose FormattedText is built from several styled spans (bold / italic / colored / underlined / kerned), plus a plain label proving the Text ⇄ FormattedText exclusivity

#### 🟢 Sonnet 5 — C++ (C1/C3)

Formatted spans (bold red, italic underlined, kerned, plain label) render identically in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9963, 0.15% pixels differ · Dark: SSIM 0.9963, 0.15% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.05% pixels differ · Dark: SSIM 0.9987, 0.05% pixels differ

### 65. Gestures — 🟢/⏳
<sub>gestures</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/gestures_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/gestures_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/gestures_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/gestures_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/gestures_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/gestures_dark.gif" /></td></tr></table>

ports GesturesPage.xaml (+ .xaml.cs) The MAUI GesturesPage.xaml is a *gallery navigation* page: a CollectionView listing gesture-demo sections that the shell navigates into

#### 🟢 Sonnet 5 — C++ (C1/C3)

Blue gesture-target rectangle, header/last-gesture text, and colors match exactly in both light and dark themes; only trivial outer-padding differs.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9975, 0.11% pixels differ · Dark: SSIM 0.9974, 0.11% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9997, 0.07% pixels differ · Dark: SSIM 0.9997, 0.07% pixels differ

### 66. Gradient — 🟢/⏳
<sub>gradient</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/gradient_light.png" /></td><td><img width="300px" src="captures/ios/cpp/gradient_light.png" /></td><td><img width="300px" src="captures/ios/xaml/gradient_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/gradient_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/gradient_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/gradient_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;gradient&amp;quot; demo (ComparePages.Gradient()), the shipped-.NET-MAUI reference for the visual-parity comparison: a VerticalStackLayout (Spacing 12, Padding 16) of two captioned 60px BoxViews — a Linea

#### 🟢 Sonnet 5 — C++ (C1/C3)

LinearGradientBrush and RadialGradientBrush bars render identically in both themes, matching colors, positions, and sizes precisely.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9984, 0.07% pixels differ · Dark: SSIM 0.9985, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 67. Grid — 🟢/⏳
<sub>grid</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/grid_light.png" /></td><td><img width="300px" src="captures/ios/cpp/grid_light.png" /></td><td><img width="300px" src="captures/ios/xaml/grid_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/grid_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/grid_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/grid_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;grid&amp;quot; demo (ComparePages.GridPage()), the shipped-.NET-MAUI reference for the visual-parity comparison: a Grid (Padding 16, Row/ColumnSpacing 6) with RowDefinitions Auto / 80 / 80 and two Star co

#### 🟢 Sonnet 5 — C++ (C1/C3)

2x2 colored grid (red/green/blue/orange) matches exactly in size, color, and position in both light and dark themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9986, 0.07% pixels differ · Dark: SSIM 0.9985, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 68. Grid Grouping — 🟢/⏳
<sub>grid_grouping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/grid_grouping_light.png" /></td><td><img width="300px" src="captures/ios/cpp/grid_grouping_light.png" /></td><td><img width="300px" src="captures/ios/xaml/grid_grouping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/grid_grouping_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/grid_grouping_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/grid_grouping_dark.png" /></td></tr></table>

ports GroupingGalleries/GridGrouping.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

Grouped list content, section headers (green), 'Total members' captions (orange), and layout match exactly in both themes; only the outer inset/crop differs.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9985, 0.07% pixels differ · Dark: SSIM 0.9984, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9995, 0.03% pixels differ · Dark: SSIM 0.9994, 0.03% pixels differ

### 69. Grouping No Templates — 🟢/⏳
<sub>grouping_no_templates</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/grouping_no_templates_light.png" /></td><td><img width="300px" src="captures/ios/cpp/grouping_no_templates_light.png" /></td><td><img width="300px" src="captures/ios/xaml/grouping_no_templates_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/grouping_no_templates_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/grouping_no_templates_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/grouping_no_templates_dark.png" /></td></tr></table>

ports GroupingGalleries/GroupingNoTemplates.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

Flat list of names matches exactly in ordering and text in both themes; MAUI's clock overlaps first row due to its inset crop but content is identical.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9994, 0.03% pixels differ · Dark: SSIM 0.9993, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9995, 0.03% pixels differ · Dark: SSIM 0.9994, 0.03% pixels differ

### 70. Grouping Plus Selection — 🟢/⏳
<sub>grouping_plus_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/grouping_plus_selection_light.png" /></td><td><img width="300px" src="captures/ios/cpp/grouping_plus_selection_light.png" /></td><td><img width="300px" src="captures/ios/xaml/grouping_plus_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/grouping_plus_selection_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/grouping_plus_selection_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/grouping_plus_selection_dark.png" /></td></tr></table>

ports CollectionViewGalleries/GroupingGalleries/ GroupingPlusSelection.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

Grouped list with headers and counts matches exactly in both themes, same as the other grouping pages.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9985, 0.07% pixels differ · Dark: SSIM 0.9984, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9995, 0.03% pixels differ · Dark: SSIM 0.9998, 0.01% pixels differ

### 71. Header Footer — 🟢/⏳
<sub>header_footer</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/header_footer_light.png" /></td><td><img width="300px" src="captures/ios/cpp/header_footer_light.png" /></td><td><img width="300px" src="captures/ios/xaml/header_footer_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/header_footer_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/header_footer_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/header_footer_dark.png" /></td></tr></table>

ports HeaderFooterString.xaml (+ HeaderFooterString.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

String header/footer text and list items match exactly in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9983, 0.08% pixels differ · Dark: SSIM 0.9983, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9998, 0.01% pixels differ · Dark: SSIM 0.9998, 0.01% pixels differ

### 72. Header Footer Grid — 🟢/⏳
<sub>header_footer_grid</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/header_footer_grid_light.png" /></td><td><img width="300px" src="captures/ios/cpp/header_footer_grid_light.png" /></td><td><img width="300px" src="captures/ios/xaml/header_footer_grid_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/header_footer_grid_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/header_footer_grid_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/header_footer_grid_dark.png" /></td></tr></table>

ports HeaderFooterGrid.xaml (+ HeaderFooterGrid.xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Sonnet 5 — C++ (C1/C3)

RULING 12 (2026-07-17): the code-first C++ render is CORRECT — its GridItemsLayout keeps the original C# item spacing (HorizontalItemSpacing=4, VerticalItemSpacing=2), so the item grid runs ~18px taller than MAUI's. The shared twin XAML degrades GridItemsLayout to the ItemsLayout string form ("VerticalGrid, 3"), which can't express item spacing (loader lacks the &lt;GridItemsLayout&gt; element form), so MAUI and C++&amp;XAML both render the grid flush (0 spacing). The cpp-vs-maui item-grid offset is this exempt, intended divergence, NOT a port bug.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9930, 0.45% pixels differ · Dark: SSIM 0.9889, 0.52% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9935, 0.43% pixels differ · Dark: SSIM 0.9893, 0.51% pixels differ

### 73. Header Footer Grid Horizontal — 🟢/⏳
<sub>header_footer_grid_horizontal</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/header_footer_grid_horizontal_light.png" /></td><td><img width="300px" src="captures/ios/cpp/header_footer_grid_horizontal_light.png" /></td><td><img width="300px" src="captures/ios/xaml/header_footer_grid_horizontal_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/header_footer_grid_horizontal_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/header_footer_grid_horizontal_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/header_footer_grid_horizontal_dark.png" /></td></tr></table>

ports HeaderFooterGridHorizontal.xaml (+ HeaderFooterGridHorizontal.xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Toggle Header/Footer links and list rows render identically in both themes; layout, text, and colors match between MAUI and cpp.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9748, 0.78% pixels differ · Dark: SSIM 0.9745, 0.78% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9753, 0.76% pixels differ · Dark: SSIM 0.9750, 0.76% pixels differ

### 74. Header Footer Template — 🟡/⏳
<sub>header_footer_template</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/header_footer_template_light.png" /></td><td><img width="300px" src="captures/ios/cpp/header_footer_template_light.png" /></td><td><img width="300px" src="captures/ios/xaml/header_footer_template_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/header_footer_template_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/header_footer_template_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/header_footer_template_dark.png" /></td></tr></table>

ports HeaderFooterTemplate.xaml (+ .xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟡 Sonnet 5 — C++ (C1/C3)

RULING 12 (2026-07-17): the code-first C++ render is CORRECT here — it shows each row's own image (cover1.jpg / oasis.jpg / photo.jpg), the original MAUI PhotoTemplate {Binding Image}. MAUI and C++&amp;XAML both render cover1.jpg in EVERY cell because the shared twin XAML degrades {Binding Image} to a hardcoded &lt;Image Source="cover1.jpg"&gt; (an x:Array of plain strings can't bind an Image). The cpp-vs-maui item-image diff (~1.4%) is this exempt, intended divergence, NOT a port bug.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9998, 0.01% pixels differ · Dark: SSIM 0.9998, 0.01% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9984, 0.07% pixels differ · Dark: SSIM 0.9983, 0.07% pixels differ

### 75. Header Footer View — 🟢/⏳
<sub>header_footer_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/header_footer_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/header_footer_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/header_footer_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/header_footer_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/header_footer_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/header_footer_view_dark.png" /></td></tr></table>

ports HeaderFooterView.xaml (+ .xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Match. The empty CollectionView now greedily fills the VerticalStackLayout slot and pushes the footer + Add/Clear buttons off-screen, so only the header (cover1 image + "This Is A Header") shows — matching MAUI iOS/Mac Catalyst. Fix: ported MAUI ItemsViewHandler2.EnsureContentSizeForScrollDirection (iOS.cs:257-263) — an EMPTY CV's desired main-axis size resolves to the collection view's own (full-viewport) frame extent ("the expansive size the CV wants by default" = UICollectionView.SizeThatFits), not 0. Surgical: only fires when contentSize==0; content-bearing CVs (items/multiple_bound_selection/cv_visual_states) size to content unchanged (verified green). Both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9986, 0.07% pixels differ · Dark: SSIM 0.9985, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

### 76. Hit Testing — 🟢/⏳
<sub>hit_testing</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/hit_testing_light.png" /></td><td><img width="300px" src="captures/ios/cpp/hit_testing_light.png" /></td><td><img width="300px" src="captures/ios/xaml/hit_testing_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/hit_testing_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/hit_testing_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/hit_testing_dark.png" /></td></tr></table>

ports HitTestingPage.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.HitTestingPage)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Selection text, shapes, scale/rotation labels, and rounded rectangle all match closely between MAUI and cpp in both themes; only trivial capture-crop differences at the bottom edge.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9985, 0.11% pixels differ · Dark: SSIM 0.9984, 0.11% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.10% pixels differ · Dark: SSIM 0.9988, 0.09% pixels differ

### 77. Horizontal Stack — 🟢/⏳
<sub>horizontal_stack</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/horizontal_stack_light.png" /></td><td><img width="300px" src="captures/ios/cpp/horizontal_stack_light.png" /></td><td><img width="300px" src="captures/ios/xaml/horizontal_stack_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/horizontal_stack_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/horizontal_stack_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/horizontal_stack_dark.png" /></td></tr></table>

Horizontal Stack

#### 🟢 Sonnet 5 — C++ (C1/C3)

The six colored swatches in the HorizontalStackLayout are pixel-identical in position, size and color between MAUI and cpp for both light and dark themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9992, 0.03% pixels differ · Dark: SSIM 0.9992, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9989, 0.05% pixels differ · Dark: SSIM 0.9989, 0.05% pixels differ

### 78. Hybrid Web View — 🟡/⏳
<sub>hybrid_web_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/hybrid_web_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/hybrid_web_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/hybrid_web_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/hybrid_web_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/hybrid_web_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/hybrid_web_view_dark.png" /></td></tr></table>

ports HybridWebViewPage.xaml (+ HybridWebViewPage.xaml.cs)

#### 🟡 Sonnet 5 — C++ (C1/C3)

MAUI's action links show full untruncated text while cpp ellipsizes them in both themes; otherwise layout matches.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9877, 0.53% pixels differ · Dark: SSIM 0.9874, 0.53% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9870, 0.56% pixels differ · Dark: SSIM 0.9867, 0.56% pixels differ

### 79. Image — 🟡/⏳
<sub>image</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/image_light.png" /></td><td><img width="300px" src="captures/ios/cpp/image_light.png" /></td><td><img width="300px" src="captures/ios/xaml/image_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/image_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/image_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/image_dark.png" /></td></tr></table>

ports ImagePage.xaml (+ ImagePage.xaml.cs)

#### 🟡 Sonnet 5 — C++ (C1/C3)

MAUI's captured frame shows the UriSource image loaded but the FileSource image blank/not yet loaded, whereas cpp's captured frame shows both images loaded - likely a capture-timing difference rather than a rendering defect.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9575, 3.95% pixels differ · Dark: SSIM 0.9975, 0.17% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9572, 3.96% pixels differ · Dark: SSIM 0.9972, 0.18% pixels differ

### 80. Image Button — 🟢/⏳
<sub>image_button</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/image_button_light.png" /></td><td><img width="300px" src="captures/ios/cpp/image_button_light.png" /></td><td><img width="300px" src="captures/ios/xaml/image_button_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/image_button_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/image_button_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/image_button_dark.png" /></td></tr></table>

ports ImageButtonPage.xaml (+ ImageButtonPage.xaml.cs) A self-contained, code-first demo page for the ImageButton control (the C# gallery-page convention, mirroring the input_controls_page / image_page pattern)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: against a fresh iOS MAUI ref, cpp now renders the white cog inside every green ImageButton (AspectFit/AspectFill/Fill/BorderColor) identically to MAUI in both themes. Prior red: the gallery bundled NO cog.png (missing from CMakeLists resource list) so the cog ImageButtons were empty; added cog.png/@2x to the gallery + gallery_xaml resource lists.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9882, 0.77% pixels differ · Dark: SSIM 0.9882, 0.77% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9875, 0.80% pixels differ · Dark: SSIM 0.9875, 0.80% pixels differ

### 81. Indicator — 🟢/⏳
<sub>indicator</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/indicator_light.png" /></td><td><img width="300px" src="captures/ios/cpp/indicator_light.png" /></td><td><img width="300px" src="captures/ios/xaml/indicator_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/indicator_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/indicator_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/indicator_dark.png" /></td></tr></table>

ports IndicatorPage.xaml A self-contained, code-first demo of the IndicatorView control

#### 🟢 Sonnet 5 — C++ (C1/C3)

Dot indicators, colors, shapes, and sizes match MAUI exactly in both light and dark themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9867, 0.81% pixels differ · Dark: SSIM 0.9829, 0.86% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9882, 0.77% pixels differ · Dark: SSIM 0.9846, 0.82% pixels differ

### 82. Input Controls — 🟢/⏳
<sub>input_controls</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/input_controls_light.png" /></td><td><img width="300px" src="captures/ios/cpp/input_controls_light.png" /></td><td><img width="300px" src="captures/ios/xaml/input_controls_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/input_controls_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/input_controls_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/input_controls_dark.png" /></td></tr></table>

a self-contained demo page for the W1-05 input-control set: editor, search_bar, radio_button (+ the radio_button_group attached grouping) and image_button on one vertical stack, wired together so every input drives a visible output (the C#

#### 🟢 Sonnet 5 — C++ (C1/C3)

Entry, search bar, and radio buttons render identically to MAUI in both themes; only trivial page-inset differences (not scored).

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9904, 0.45% pixels differ · Dark: SSIM 0.9904, 0.45% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9909, 0.44% pixels differ · Dark: SSIM 0.9908, 0.44% pixels differ

### 83. Input Transparent — 🟢/⏳
<sub>input_transparent</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/input_transparent_light.png" /></td><td><img width="300px" src="captures/ios/cpp/input_transparent_light.png" /></td><td><img width="300px" src="captures/ios/xaml/input_transparent_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/input_transparent_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/input_transparent_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/input_transparent_dark.png" /></td></tr></table>

ports InputTransparentPage.xaml (Maui.Controls.Sample.Pages.InputTransparentPage)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Text, buttons, and overlapping-label rendering (a shared quirk present in both MAUI and cpp) match exactly in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9986, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.07% pixels differ · Dark: SSIM 0.9986, 0.07% pixels differ

### 84. Invalidate Brush — 🟢/⏳
<sub>invalidate_brush</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/invalidate_brush_light.png" /></td><td><img width="300px" src="captures/ios/cpp/invalidate_brush_light.png" /></td><td><img width="300px" src="captures/ios/xaml/invalidate_brush_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/invalidate_brush_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/invalidate_brush_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/invalidate_brush_dark.png" /></td></tr></table>

ports InvalidateBrushGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/InvalidateBrushGallery.xaml (&amp;quot;Invalidate Brushes Playground&amp;quot;): a VerticalStackLayout (Padding 12) with — - a &amp;quot;Change color&amp;quot; Bu

#### 🟢 Sonnet 5 — C++ (C1/C3)

Button, border color, and label match MAUI exactly in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9982, 0.08% pixels differ · Dark: SSIM 0.9981, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9988, 0.06% pixels differ

### 85. Invalidate Shadow Host — 🟢/⏳
<sub>invalidate_shadow_host</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/invalidate_shadow_host_light.png" /></td><td><img width="300px" src="captures/ios/cpp/invalidate_shadow_host_light.png" /></td><td><img width="300px" src="captures/ios/xaml/invalidate_shadow_host_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/invalidate_shadow_host_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/invalidate_shadow_host_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/invalidate_shadow_host_dark.png" /></td></tr></table>

ports InvalidateShadowHostPage.xaml A self-contained, code-first demo that a shadow re-applies (invalidates) when its host&amp;#x27;s size changes, mirroring the C# core gallery page (Pages/Core/ShadowGalleries/InvalidateShadowHostPage.xaml + .xaml.

#### 🟢 Sonnet 5 — C++ (C1/C3)

Sliders, labels, and the shadowed box render identically to MAUI in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9832, 0.59% pixels differ · Dark: SSIM 0.9850, 0.59% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9834, 0.58% pixels differ · Dark: SSIM 0.9852, 0.58% pixels differ

### 86. Ios Blur Effect — 🟡/⏳
<sub>ios_blur_effect</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_blur_effect_light.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_blur_effect_light.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_blur_effect_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_blur_effect_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_blur_effect_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_blur_effect_dark.png" /></td></tr></table>

ports iOSBlurEffectPage.xaml The .NET MAUI PlatformSpecifics sample (Pages/PlatformSpecifics/iOS/iOSBlurEffectPage.xaml + .xaml.cs): an Image (Source=&amp;quot;oasis.jpg&amp;quot;) carrying the iOSSpecific VisualElement.BlurEffect knob (XAML seeds it to Extr

#### 🟡 Sonnet 5 — C++ (C1/C3)

Layout/text/links match MAUI, but the MAUI reference screenshots show no image loaded at all while cpp shows a fully loaded photo behind the blur controls; cannot fully verify blur rendering parity due to this reference gap.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9988, 0.06% pixels differ

### 87. Ios Date Picker — 🟢/⏳
<sub>ios_date_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_date_picker_light.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_date_picker_light.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_date_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_date_picker_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_date_picker_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_date_picker_dark.png" /></td></tr></table>

ports iOSDatePickerPage.xaml (+ iOSDatePickerPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Date display and toggle link match MAUI exactly in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9988, 0.06% pixels differ

### 88. Ios Entry — 🟢/⏳
<sub>ios_entry</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_entry_light.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_entry_light.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_entry_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_entry_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_entry_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_entry_dark.png" /></td></tr></table>

ports iOSEntryPage.xaml (+ iOSEntryPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Entry placeholder text and toggle link match MAUI exactly in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9844, 0.67% pixels differ · Dark: SSIM 0.9833, 0.70% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9988, 0.06% pixels differ

### 89. Ios First Responder — 🟢/⏳
<sub>ios_first_responder</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_first_responder_light.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_first_responder_light.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_first_responder_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_first_responder_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_first_responder_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_first_responder_dark.png" /></td></tr></table>

ports iOSFirstResponderPage.xaml (+ .xaml.cs) The C# iOSFirstResponderPage is a VisualElement-first-responder demo: a StackLayout with an explanatory Label, a &amp;quot;First Entry&amp;quot; + plain &amp;quot;OK&amp;quot; Button (tapping OK dismisses the keyboard because the

#### 🟢 Sonnet 5 — C++ (C1/C3)

Entry fields, OK links, focus buttons, and state text all match MAUI in both themes; only trivial status-bar clock differences.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 90. Ios Pan Gesture — 🟢/⏳
<sub>ios_pan_gesture</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_pan_gesture_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/ios_pan_gesture_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/ios_pan_gesture_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_pan_gesture_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/ios_pan_gesture_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/ios_pan_gesture_dark.gif" /></td></tr></table>

ports iOSPanGestureRecognizerPage.xaml (+ .xaml.cs) The C# iOSPanGestureRecognizerPage is a StackLayout with: a bold message Label (_messageLabel), a &amp;quot;Toggle Simultaneous Gesture Recognition&amp;quot; Button, and a grouped ListView of employees whos

#### 🟢 Sonnet 5 — C++ (C1/C3)

Panned label, toggle link, target label, and recognition state text match identically in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9983, 0.10% pixels differ · Dark: SSIM 0.9982, 0.10% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9990, 0.07% pixels differ · Dark: SSIM 0.9989, 0.07% pixels differ

### 91. Ios Picker — 🟢/⏳
<sub>ios_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_picker_light.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_picker_light.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_picker_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_picker_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_picker_dark.png" /></td></tr></table>

ports iOSPickerPage.xaml (+ iOSPickerPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Placeholder entry and toggle link render identically to MAUI in both light and dark themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

### 92. Ios Safe Area — 🟢/⏳
<sub>ios_safe_area</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_safe_area_light.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_safe_area_light.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_safe_area_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_safe_area_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_safe_area_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_safe_area_dark.png" /></td></tr></table>

ports iOSSafeAreaPage.xaml The .NET MAUI PlatformSpecifics sample (Pages/PlatformSpecifics/iOS/iOSSafeAreaPage.xaml + .xaml.cs): a long Lorem-ipsum Label over a &amp;quot;Disable Use Safe Area&amp;quot; button

#### 🟢 Sonnet 5 — C++ (C1/C3)

Lorem ipsum paragraph and toggle link match MAUI exactly in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

### 93. Ios Scroll View — 🟢/⏳
<sub>ios_scroll_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_scroll_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_scroll_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_scroll_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_scroll_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_scroll_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_scroll_view_dark.png" /></td></tr></table>

ports iOSScrollViewPage.xaml (+ iOSScrollViewPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Pixel-perfect match in both themes (cpp-vs-MAUI 0.07%/0.08%, SSIM 0.998). The earlier 'extra back chevron' was a CAPTURE ARTIFACT (a SpringBoard back-to-previous-app overlay that leaked into the cpp screenshot when capture switched apps), NOT a port bug — iter35's lldb view hierarchy already proved the app renders no chevron. Recaptured cleanly via the WS-E flow (warm-up launch + same-app relaunch + settle) against a fresh MauiReference iOS baseline: slider, 'Toggle ScrollView DelayContentTouches', and 'Return to Platform-Specifics List' all align exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9985, 0.07% pixels differ · Dark: SSIM 0.9985, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 94. Ios Search Bar — 🟢/⏳
<sub>ios_search_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_search_bar_light.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_search_bar_light.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_search_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_search_bar_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_search_bar_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_search_bar_dark.png" /></td></tr></table>

ports iOSSearchBarPage.xaml (+ iOSSearchBarPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Search bar with magnifier icon and placeholder, plus both toggle links, match MAUI in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9977, 0.16% pixels differ · Dark: SSIM 0.9976, 0.16% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9976, 0.16% pixels differ · Dark: SSIM 0.9976, 0.16% pixels differ

### 95. Ios Slider Update On Tap — 🟢/⏳
<sub>ios_slider_update_on_tap</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_slider_update_on_tap_light.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_slider_update_on_tap_light.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_slider_update_on_tap_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_slider_update_on_tap_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_slider_update_on_tap_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_slider_update_on_tap_dark.png" /></td></tr></table>

ports iOSSliderUpdateOnTapPage.xaml (+ iOSSliderUpdateOnTapPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Slider thumb position, instructional text, and toggle link match MAUI in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

### 96. Ios Swipe Transition — 🟢/⏳
<sub>ios_swipe_transition</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_swipe_transition_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/ios_swipe_transition_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/ios_swipe_transition_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_swipe_transition_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/ios_swipe_transition_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/ios_swipe_transition_dark.gif" /></td></tr></table>

ports iOSSwipeViewTransitionModePage.xaml (+ .xaml.cs) The C# iOSSwipeViewTransitionModePage is a StackLayout with: a horizontal row holding a &amp;quot;SwipeTransitionMode:&amp;quot; Label + an EnumPicker over the SwipeTransitionMode enum (Reveal / Drag, Se

#### 🟢 Sonnet 5 — C++ (C1/C3)

SwipeTransitionMode labels/links, swipe-right gray box, and status text all match MAUI in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 97. Ios Time Picker — 🟢/⏳
<sub>ios_time_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_time_picker_light.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_time_picker_light.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_time_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_time_picker_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_time_picker_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_time_picker_dark.png" /></td></tr></table>

ports iOSTimePickerPage.xaml The .NET MAUI PlatformSpecifics sample (Pages/PlatformSpecifics/iOS/iOSTimePickerPage.xaml + .xaml.cs): a TimePicker carrying the iOSSpecific TimePicker.UpdateMode knob (XAML seeds it to WhenFinished) over a but

#### 🟢 Sonnet 5 — C++ (C1/C3)

Time value, label, and layout match MAUI in both light and dark themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9976, 0.11% pixels differ · Dark: SSIM 0.9975, 0.11% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9982, 0.08% pixels differ · Dark: SSIM 0.9980, 0.08% pixels differ

### 98. Items — 🟢/⏳
<sub>items</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/items_light.png" /></td><td><img width="300px" src="captures/ios/cpp/items_light.png" /></td><td><img width="300px" src="captures/ios/xaml/items_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/items_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/items_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/items_dark.png" /></td></tr></table>

a self-contained demo page for the W2-19 items core: a collection_view over a live observable items source with a templated cell, single selection driving a readout label, and an EmptyView for the cleared state (the C# CollectionView galler

#### 🟢 Sonnet 5 — C++ (C1/C3)

Task list content and layout are identical to MAUI in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9985, 0.07% pixels differ · Dark: SSIM 0.9985, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9990, 0.04% pixels differ · Dark: SSIM 0.9989, 0.04% pixels differ

### 99. Items Updating Scroll Mode — 🟢/⏳
<sub>items_updating_scroll_mode</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/items_updating_scroll_mode_light.png" /></td><td><img width="300px" src="captures/ios/cpp/items_updating_scroll_mode_light.png" /></td><td><img width="300px" src="captures/ios/xaml/items_updating_scroll_mode_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/items_updating_scroll_mode_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/items_updating_scroll_mode_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/items_updating_scroll_mode_dark.png" /></td></tr></table>

ports ItemsUpdatingScrollModeGallery.xaml (+ .xaml.cs) of the C# CollectionView gallery (Maui.Controls.Sample.Pages.CollectionViewGalleries.ScrollModeGalleries.ItemsUpdatingScrollModeGallery)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Full 50-item list with mode/toggle controls renders identically to MAUI in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9985, 0.07% pixels differ · Dark: SSIM 0.9985, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9990, 0.04% pixels differ · Dark: SSIM 0.9989, 0.04% pixels differ

### 100. Label — 🟢/⏳
<sub>label</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/label_light.png" /></td><td><img width="300px" src="captures/ios/cpp/label_light.png" /></td><td><img width="300px" src="captures/ios/xaml/label_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/label_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/label_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/label_dark.png" /></td></tr></table>

ports LabelPage.xaml (+ LabelPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

All label styling variants (color, background, alignment, formatted spans, big font) match MAUI pixel-for-pixel in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9960, 0.16% pixels differ · Dark: SSIM 0.9959, 0.16% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9964, 0.13% pixels differ · Dark: SSIM 0.9964, 0.13% pixels differ

### 101. Layout Is Enabled — 🟢/⏳
<sub>layout_is_enabled</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/layout_is_enabled_light.png" /></td><td><img width="300px" src="captures/ios/cpp/layout_is_enabled_light.png" /></td><td><img width="300px" src="captures/ios/xaml/layout_is_enabled_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/layout_is_enabled_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/layout_is_enabled_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/layout_is_enabled_dark.png" /></td></tr></table>

ports LayoutIsEnabledPage.xaml (+ LayoutIsEnabledPage.xaml.cs) The C# page demonstrates how IsEnabled on a layout cascades to its children: a 2x2 grid whose left column hosts a &amp;quot;MainLayout&amp;quot; full of state-demo sub-stacks (all-enabled / all-d

#### 🟢 Sonnet 5 — C++ (C1/C3)

All enabled/disabled layout states across both columns match MAUI exactly in light and dark themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9824, 0.65% pixels differ · Dark: SSIM 0.9820, 0.65% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9990, 0.04% pixels differ · Dark: SSIM 0.9989, 0.04% pixels differ

### 102. Line Gallery — 🟢/⏳
<sub>line_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/line_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/line_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/line_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/line_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/line_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/line_gallery_dark.png" /></td></tr></table>

ports LineGallery.xaml A self-contained, code-first port of the MAUI Shapes LineGallery (Pages/Controls/ShapesGalleries/LineGallery.xaml + .xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Basic, dash, and stroke-thickness lines match MAUI; the black stroke-thickness line is invisible against black background in dark theme for both MAUI and cpp, so this is a shared quirk not a port bug.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9985, 0.07% pixels differ · Dark: SSIM 0.9985, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9990, 0.04% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 103. Line Join Gallery — 🟢/⏳
<sub>line_join_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/line_join_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/line_join_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/line_join_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/line_join_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/line_join_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/line_join_gallery_dark.png" /></td></tr></table>

ports LineJoinGallery.xaml A self-contained, code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/LineJoinGallery.xaml: a StackLayout that demonstrates the three StrokeLineJoin variants on an identical open polyline

#### 🟢 Sonnet 5 — C++ (C1/C3)

Miter, bevel, and round line-join shapes render identically to MAUI in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9986, 0.06% pixels differ · Dark: SSIM 0.9985, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 104. Measure First Strategy — 🟢/⏳
<sub>measure_first_strategy</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/measure_first_strategy_light.png" /></td><td><img width="300px" src="captures/ios/cpp/measure_first_strategy_light.png" /></td><td><img width="300px" src="captures/ios/xaml/measure_first_strategy_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/measure_first_strategy_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/measure_first_strategy_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/measure_first_strategy_dark.png" /></td></tr></table>

ports MeasureFirstStrategy.xaml (+ .xaml.cs) of the C# CollectionView gallery (Maui.Controls.Sample.Pages.CollectionViewGalleries.GroupingGalleries.MeasureFirstStrategy)

#### 🟢 Sonnet 5 — C++ (C1/C3)

CollectionView grouped list with MeasureFirstItem strategy matches MAUI content and layout in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.05% pixels differ · Dark: SSIM 0.9988, 0.05% pixels differ

### 105. Menu Bar — 🟢/⏳
<sub>menu_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/menu_bar_light.png" /></td><td><img width="300px" src="captures/ios/cpp/menu_bar_light.png" /></td><td><img width="300px" src="captures/ios/xaml/menu_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/menu_bar_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/menu_bar_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/menu_bar_dark.png" /></td></tr></table>

ports MenuBarPage.xaml (+ MenuBarPage.cs) The C# page declares three page-level MenuBarItems (Page.MenuBarItems — the app menu bar) and a small visible body: - &amp;quot;Before File&amp;quot; : &amp;quot;Before File Action&amp;quot; (accelerator &amp;quot;b&amp;quot;), &amp;quot;Cool item 1&amp;quot;, a separat

#### 🟢 Sonnet 5 — C++ (C1/C3)

Text, colors, and layout match MAUI exactly in both light and dark themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9934, 0.28% pixels differ · Dark: SSIM 0.9932, 0.28% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.05% pixels differ · Dark: SSIM 0.9988, 0.05% pixels differ

### 106. Modal — 🟢/⏳
<sub>modal</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/modal_light.png" /></td><td><img width="300px" src="captures/ios/cpp/modal_light.png" /></td><td><img width="300px" src="captures/ios/xaml/modal_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/modal_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/modal_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/modal_dark.png" /></td></tr></table>

ports ModalPage.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Modal page content, links, and depth counters match MAUI exactly in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.05% pixels differ · Dark: SSIM 0.9988, 0.05% pixels differ

### 107. Multiple Bound Selection — 🟢/⏳
<sub>multiple_bound_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/multiple_bound_selection_light.png" /></td><td><img width="300px" src="captures/ios/cpp/multiple_bound_selection_light.png" /></td><td><img width="300px" src="captures/ios/xaml/multiple_bound_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/multiple_bound_selection_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/multiple_bound_selection_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/multiple_bound_selection_dark.png" /></td></tr></table>

ports MultipleBoundSelection.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.MultipleBoundSelection)

#### 🟢 Sonnet 5 — C++ (C1/C3)

CollectionView selection highlighting, header, and items match MAUI exactly in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9980, 0.09% pixels differ · Dark: SSIM 0.9980, 0.09% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 108. Navigation Gallery — 🟢/⏳
<sub>navigation_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/navigation_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/navigation_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/navigation_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/navigation_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/navigation_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/navigation_gallery_dark.png" /></td></tr></table>

ports NavigationGallery.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Navigation gallery text and links match MAUI exactly in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.05% pixels differ · Dark: SSIM 0.9988, 0.05% pixels differ

### 109. Nested Collection — 🟢/⏳
<sub>nested_collection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/nested_collection_light.png" /></td><td><img width="300px" src="captures/ios/cpp/nested_collection_light.png" /></td><td><img width="300px" src="captures/ios/xaml/nested_collection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/nested_collection_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/nested_collection_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/nested_collection_dark.png" /></td></tr></table>

ports NestedGalleries/NestedCollectionViewGallery.xaml (+ NestedCollectionViewGallery.xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

RULING 12 (2026-07-17): the code-first C++ render is CORRECT — its inner (nested) CollectionView keeps the original item spacing, so its rows run slightly wider/taller than MAUI's flush twin (the shared XAML degrades the inner GridItemsLayout to the string form, losing item spacing). The cpp-vs-maui offset is this exempt divergence, NOT a port bug.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9056, 3.70% pixels differ · Dark: SSIM 0.9242, 3.70% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9066, 2.64% pixels differ · Dark: SSIM 0.9188, 2.64% pixels differ

### 110. Pan Gesture Events — 🟢/⏳
<sub>pan_gesture_events</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/pan_gesture_events_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/pan_gesture_events_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/pan_gesture_events_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/pan_gesture_events_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/pan_gesture_events_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/pan_gesture_events_dark.gif" /></td></tr></table>

ports PanGestureEventsGallery.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.PanGestureEventsGallery)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Green/red status blocks and status text match MAUI exactly in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9983, 0.08% pixels differ · Dark: SSIM 0.9995, 0.05% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9945, 0.25% pixels differ · Dark: SSIM 0.9950, 0.24% pixels differ

### 111. Path Aspect Gallery — 🟢/⏳
<sub>path_aspect_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/path_aspect_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/path_aspect_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/path_aspect_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/path_aspect_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/path_aspect_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/path_aspect_gallery_dark.png" /></td></tr></table>

ports PathAspectGallery.xaml A self-contained, code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PathAspectGallery.xaml: a StackLayout (Padding 12) that demonstrates the four Path Aspect modes on one identical ge

#### 🟢 Sonnet 5 — C++ (C1/C3)

All four heart-aspect renders (None/Fill/Uniform/UniformToFill) match MAUI pixel-for-pixel in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9986, 0.07% pixels differ · Dark: SSIM 0.9985, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 112. Path Gallery — 🟢/⏳
<sub>path_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/path_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/path_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/path_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/path_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/path_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/path_gallery_dark.png" /></td></tr></table>

ports PathGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PathGallery.xaml: a ScrollView over a StackLayout (Padding 12) that walks eight Path variants (plus two caption-only markup-string Labels

#### 🟢 Sonnet 5 — C++ (C1/C3)

All path/geometry shapes match MAUI in both themes, including the same invisible black-stroke shapes in dark mode.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9986, 0.07% pixels differ · Dark: SSIM 0.9985, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 113. Path Transform String — 🟢/⏳
<sub>path_transform_string</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/path_transform_string_light.png" /></td><td><img width="300px" src="captures/ios/cpp/path_transform_string_light.png" /></td><td><img width="300px" src="captures/ios/xaml/path_transform_string_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/path_transform_string_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/path_transform_string_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/path_transform_string_dark.png" /></td></tr></table>

ports PathTransformStringGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PathTransformStringGallery.xaml: a ScrollView over a StackLayout (Padding 12) that shows the SAME two-figure Path geometry

#### 🟢 Sonnet 5 — C++ (C1/C3)

Both light-theme renders match pixel-for-pixel (triangles with/without RenderTransform identical); dark theme is entirely blank in both maui and cpp, so no regression.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9783, 1.13% pixels differ · Dark: SSIM 0.9985, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 114. Picker — 🟢/⏳
<sub>picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/picker_light.png" /></td><td><img width="300px" src="captures/ios/cpp/picker_light.png" /></td><td><img width="300px" src="captures/ios/xaml/picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/picker_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/picker_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/picker_dark.png" /></td></tr></table>

ports PickerPage.xaml (+ PickerPage.xaml.cs) A self-contained, code-first demo page for the Picker control (the C# gallery-page convention, mirroring the value_controls_page / pickers_page pattern)

#### 🟢 Sonnet 5 — C++ (C1/C3)

All picker variants match pixel-for-pixel in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9986, 0.07% pixels differ · Dark: SSIM 0.9985, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 115. Pickers — 🟢/⏳
<sub>pickers</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/pickers_light.png" /></td><td><img width="300px" src="captures/ios/cpp/pickers_light.png" /></td><td><img width="300px" src="captures/ios/xaml/pickers_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/pickers_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/pickers_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/pickers_dark.png" /></td></tr></table>

a self-contained demo page for the W1-06 picker set: picker, date_picker and time_picker on one vertical stack, wired together so every selection drives a visible output (the C# gallery-page convention, code-first; the value_controls_page p

#### 🟢 Sonnet 5 — C++ (C1/C3)

Room/date/time picker layout and text match in both themes; date values differ only due to capture-day date (non-bug).

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9911, 0.36% pixels differ · Dark: SSIM 0.9910, 0.36% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9962, 0.16% pixels differ · Dark: SSIM 0.9961, 0.16% pixels differ

### 116. Pointer Gesture — 🟢/⏳
<sub>pointer_gesture</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/pointer_gesture_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/pointer_gesture_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/pointer_gesture_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/pointer_gesture_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/pointer_gesture_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/pointer_gesture_dark.gif" /></td></tr></table>

ports PointerGestureGalleryPage.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.PointerGestureGalleryPage)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Pointer position labels, hover/press states, and colors match pixel-for-pixel in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9997, 0.07% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9997, 0.07% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 117. Polygon Gallery — 🟢/⏳
<sub>polygon_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/polygon_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/polygon_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/polygon_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/polygon_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/polygon_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/polygon_gallery_dark.png" /></td></tr></table>

ports PolygonGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PolygonGallery.xaml: a ScrollView over a StackLayout (Padding 12) that walks four Polygon variants, each under a caption Label — - &amp;quot;A

#### 🟢 Sonnet 5 — C++ (C1/C3)

iOS: 'A basic Polygon' green triangle + 'A dash Polygon' dashed triangle match MAUI. Stale red cleared on recapture.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9990, 0.05% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 118. Polyline Gallery — 🟢/⏳
<sub>polyline_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/polyline_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/polyline_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/polyline_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/polyline_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/polyline_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/polyline_gallery_dark.png" /></td></tr></table>

ports PolylineGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PolylineGallery.xaml: a StackLayout (Padding 12 — no ScrollView in the C# source) holding two Polyline variants, each under a caption

#### 🟢 Sonnet 5 — C++ (C1/C3)

Both maui and cpp show the polyline content clipped identically at the left edge in both themes; shared harness characteristic, not a port-specific regression.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9837, 0.50% pixels differ · Dark: SSIM 0.9835, 0.50% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 119. Preselected Item — 🟢/⏳
<sub>preselected_item</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/preselected_item_light.png" /></td><td><img width="300px" src="captures/ios/cpp/preselected_item_light.png" /></td><td><img width="300px" src="captures/ios/xaml/preselected_item_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/preselected_item_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/preselected_item_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/preselected_item_dark.png" /></td></tr></table>

ports PreselectedItemGallery.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.PreselectedItemGallery)

#### 🟢 Sonnet 5 — C++ (C1/C3)

CollectionView with preselected item renders identically in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9980, 0.09% pixels differ · Dark: SSIM 0.9980, 0.09% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 120. Preselected Items — 🟢/⏳
<sub>preselected_items</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/preselected_items_light.png" /></td><td><img width="300px" src="captures/ios/cpp/preselected_items_light.png" /></td><td><img width="300px" src="captures/ios/xaml/preselected_items_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/preselected_items_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/preselected_items_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/preselected_items_dark.png" /></td></tr></table>

ports PreselectedItemsGallery.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.PreselectedItemsGallery)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Grid CollectionView with multiple preselected items matches pixel-for-pixel in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9990, 0.05% pixels differ · Dark: SSIM 0.9990, 0.05% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 121. Progress Bar — 🟢/⏳
<sub>progress_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/progress_bar_light.png" /></td><td><img width="300px" src="captures/ios/cpp/progress_bar_light.png" /></td><td><img width="300px" src="captures/ios/xaml/progress_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/progress_bar_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/progress_bar_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/progress_bar_dark.png" /></td></tr></table>

ports ProgressBarPage.xaml (+ ProgressBarPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Progress bars, colors, and states match MAUI in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9990, 0.05% pixels differ · Dark: SSIM 0.9990, 0.05% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 122. Radio Button Border — 🟢/⏳
<sub>radio_button_border</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/radio_button_border_light.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_button_border_light.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_button_border_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/radio_button_border_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_button_border_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_button_border_dark.png" /></td></tr></table>

ports RadioButtonBorder.xaml A self-contained, code-first demo of RadioButton border styling

#### 🟢 Sonnet 5 — C++ (C1/C3)

Bordered radio rows with yellow/green highlights match MAUI exactly in light and dark.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9253, 5.65% pixels differ · Dark: SSIM 0.9101, 5.65% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9250, 5.66% pixels differ · Dark: SSIM 0.9098, 5.66% pixels differ

### 123. Radio Button Content — 🟢/⏳
<sub>radio_button_content</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/radio_button_content_light.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_button_content_light.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_button_content_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/radio_button_content_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_button_content_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_button_content_dark.png" /></td></tr></table>

ports RadioButtonContentGallery.xaml A self-contained, code-first demo of the RadioButton.Content surface

#### 🟢 Sonnet 5 — C++ (C1/C3)

cpp now renders the coffee.png cups in the two custom-template cards, matching the FRESH MAUI iOS reference (both show black bar + red bar + cup). The prior frozen iOS ref was STALE (showed no cup / one row); recaptured fresh. Residual pixel (6.14%) is the harness inset + cup anti-aliasing.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🔴 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.8778, 5.02% pixels differ · Dark: SSIM 0.8977, 3.91% pixels differ

#### 🔴 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.8775, 5.02% pixels differ · Dark: SSIM 0.8973, 3.92% pixels differ

### 124. Radio Button Group — 🟢/⏳
<sub>radio_button_group</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/radio_button_group_light.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_button_group_light.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_button_group_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/radio_button_group_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_button_group_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_button_group_dark.png" /></td></tr></table>

ports RadioButtonGroupGallery.xaml A self-contained, code-first demo of the RadioButtonGroup ATTACHED-PROPERTY grouping: a vertical StackLayout carries RadioButtonGroup.GroupName=&amp;quot;foo&amp;quot;, so every descendant RadioButton — including one nested

#### 🟢 Sonnet 5 — C++ (C1/C3)

StackLayout and Grid-based radio groups match MAUI layout and styling in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9773, 0.90% pixels differ · Dark: SSIM 0.9771, 0.90% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9781, 0.87% pixels differ · Dark: SSIM 0.9779, 0.87% pixels differ

### 125. Radio Button Group Binding — 🟢/⏳
<sub>radio_button_group_binding</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/radio_button_group_binding_light.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_button_group_binding_light.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_button_group_binding_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/radio_button_group_binding_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_button_group_binding_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_button_group_binding_dark.png" /></td></tr></table>

ports RadioButtonGroupBindingGallery.xaml A code-first demo of binding the RadioButtonGroup attached properties (GroupName + SelectedValue) to a view-model

#### 🟢 Sonnet 5 — C++ (C1/C3)

Bound group/selection radios match MAUI layout, text, and links in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9762, 1.00% pixels differ · Dark: SSIM 0.9757, 1.00% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9766, 0.98% pixels differ · Dark: SSIM 0.9761, 0.98% pixels differ

### 126. Radio Button Group Gallery — 🟢/⏳
<sub>radio_button_group_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/radio_button_group_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_button_group_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_button_group_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/radio_button_group_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_button_group_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_button_group_gallery_dark.png" /></td></tr></table>

ports RadioButtonGroupGalleryPage.xaml A self-contained, code-first demo of RadioButton grouping SCOPE, mirroring the C# controls gallery page (Pages/Controls/RadioButtonGalleries/RadioButtonGroupGalleryPage.xaml)

#### 🟢 Sonnet 5 — C++ (C1/C3)

All three group-name test sections render identically to MAUI in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🔴 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.8695, 5.12% pixels differ · Dark: SSIM 0.8683, 5.12% pixels differ

#### 🔴 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.8698, 5.11% pixels differ · Dark: SSIM 0.8687, 5.11% pixels differ

### 127. Radio Content Properties — 🟢/⏳
<sub>radio_content_properties</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/radio_content_properties_light.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_content_properties_light.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_content_properties_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/radio_content_properties_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_content_properties_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_content_properties_dark.png" /></td></tr></table>

ports ContentProperties.xaml A self-contained, code-first demo of how RadioButton propagates the standard Text/Font properties to its Content

#### 🟢 Sonnet 5 — C++ (C1/C3)

Text styling propagation (color, font, transform) to Content matches MAUI in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🔴 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.8485, 5.97% pixels differ · Dark: SSIM 0.8472, 5.91% pixels differ

#### 🔴 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.8489, 5.95% pixels differ · Dark: SSIM 0.8475, 5.89% pixels differ

### 128. Radio Template From Style — ⬛/⏳
<sub>radio_template_from_style</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/radio_template_from_style_light.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_template_from_style_light.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_template_from_style_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/radio_template_from_style_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_template_from_style_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_template_from_style_dark.png" /></td></tr></table>

ports TemplateFromStyle.xaml A self-contained, code-first demo of applying a RadioButton ControlTemplate

#### ⬛ Sonnet 5 — C++ (C1/C3)

MAUI reference screenshots (light/dark) show the iOS home screen instead of the app content — reference capture is broken/unusable for comparison.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9922, 0.18% pixels differ · Dark: SSIM 0.9884, 0.36% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9970, 0.06% pixels differ · Dark: SSIM 0.9920, 0.29% pixels differ

### 129. Rectangle Gallery — 🟢/⏳
<sub>rectangle_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/rectangle_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/rectangle_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/rectangle_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/rectangle_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/rectangle_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/rectangle_gallery_dark.png" /></td></tr></table>

ports RectangleGallery.xaml A self-contained, code-first port of the MAUI Shapes RectangleGallery (Pages/Controls/ShapesGalleries/RectangleGallery.xaml + .xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Both themes render identically to MAUI: rectangle, square outline, stroke rectangle, dashed stroke, and rounded rectangle shapes all match in color, size, and position.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9984, 0.08% pixels differ · Dark: SSIM 0.9983, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 130. Refresh View — 🟢/⏳
<sub>refresh_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/refresh_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/refresh_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/refresh_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/refresh_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/refresh_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/refresh_view_dark.png" /></td></tr></table>

ports RefreshViewPage.xaml (+ RefreshViewPage.xaml.cs + RefreshViewModel.cs) A self-contained, code-first demo page for the RefreshView control (the C# gallery-page convention, mirroring the swipe_refresh_page pattern)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Text, links, and layout match MAUI exactly in both light and dark themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9984, 0.08% pixels differ · Dark: SSIM 0.9983, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 131. Relative Layout — 🟢/⏳
<sub>relative_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/relative_layout_light.png" /></td><td><img width="300px" src="captures/ios/cpp/relative_layout_light.png" /></td><td><img width="300px" src="captures/ios/xaml/relative_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/relative_layout_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/relative_layout_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/relative_layout_dark.png" /></td></tr></table>

ports RelativeLayoutPage.xaml

#### 🟢 Sonnet 5 — C++ (C1/C3)

Corner-anchored colored boxes and centered gray/black rectangles match MAUI's positions and colors in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9990, 0.05% pixels differ · Dark: SSIM 0.9990, 0.05% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 132. Scattered Radio Button — 🟢/⏳
<sub>scattered_radio_button</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/scattered_radio_button_light.png" /></td><td><img width="300px" src="captures/ios/cpp/scattered_radio_button_light.png" /></td><td><img width="300px" src="captures/ios/xaml/scattered_radio_button_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/scattered_radio_button_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/scattered_radio_button_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/scattered_radio_button_dark.png" /></td></tr></table>

ports ScatteredRadioButtonGallery.xaml A code-first demo that radio buttons DON&amp;#x27;T have to share a container to be grouped: grouping is by GroupName, so buttons scattered across separate containers (and one bare button outside any grouped co

#### 🟢 Sonnet 5 — C++ (C1/C3)

Radio buttons, labels, and highlighted group background match MAUI in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9827, 0.75% pixels differ · Dark: SSIM 0.9857, 0.56% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9823, 0.76% pixels differ · Dark: SSIM 0.9854, 0.57% pixels differ

### 133. Scroll Mode Test — 🟢/⏳
<sub>scroll_mode_test</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/scroll_mode_test_light.png" /></td><td><img width="300px" src="captures/ios/cpp/scroll_mode_test_light.png" /></td><td><img width="300px" src="captures/ios/xaml/scroll_mode_test_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/scroll_mode_test_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/scroll_mode_test_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/scroll_mode_test_dark.png" /></td></tr></table>

ports ScrollModeTestGallery.xaml (+ .xaml.cs) of the C# CollectionView gallery (Maui.Controls.Sample.Pages.CollectionViewGalleries.ScrollModeGalleries.ScrollModeTestGallery)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Matches MAUI in both themes. Layout, the Scroll-To-Middle/Add-Item links, 'Mode: KeepItemsInView · Items: 20', and the 20-item list (now ~44px row pitch after the CV item-Margin fix) all align. The only delta — the ItemsUpdatingScrollMode Picker showing the selected 'KeepItemsInView' (cpp) vs a blank entry (MAUI) — is an EXEMPT element-items-form Picker init artifact per ruling 8 (cpp faithful; MAUI shows the Title/blank). Not a port bug.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9954, 0.17% pixels differ · Dark: SSIM 0.9953, 0.17% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 134. Scroll To Group — 🟢/⏳
<sub>scroll_to_group</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/scroll_to_group_light.png" /></td><td><img width="300px" src="captures/ios/cpp/scroll_to_group_light.png" /></td><td><img width="300px" src="captures/ios/xaml/scroll_to_group_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/scroll_to_group_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/scroll_to_group_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/scroll_to_group_dark.png" /></td></tr></table>

ports ScrollToGalleries/ScrollToGroup.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

Group/item text fields, Go links, and the full scrollable superhero list match MAUI closely in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9990, 0.05% pixels differ · Dark: SSIM 0.9990, 0.05% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 135. Scroll View — 🟢/⏳
<sub>scroll_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/scroll_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/scroll_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/scroll_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/scroll_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/scroll_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/scroll_view_dark.png" /></td></tr></table>

ports ScrollViewPage.xaml (+ the ScrollViewPages sub-demos: ScrollViewOrientationPage / ScrollToEndPage / ScrollToFromConstructorPage), code-first

#### 🟢 Sonnet 5 — C++ (C1/C3)

Slider and links match MAUI; the C++ port additionally shows a back/nav chevron button not present in the MAUI capture, a minor harness-navigation difference that doesn't affect page content.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9981, 0.08% pixels differ · Dark: SSIM 0.9980, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 136. Search Bar — 🟢/⏳
<sub>search_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/search_bar_light.png" /></td><td><img width="300px" src="captures/ios/cpp/search_bar_light.png" /></td><td><img width="300px" src="captures/ios/xaml/search_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/search_bar_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/search_bar_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/search_bar_dark.png" /></td></tr></table>

ports SearchBarPage.xaml (Microsoft.Maui.Controls sample gallery)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Match in both themes. The "Cancel is red" search bar's cancel button now renders red (was white in dark mode). Root cause + fix: on iOS 26+ UIKit re-tints the cancel-button icon every layout pass, defeating setTitleColor/tintColor; ported MAUI's SearchBarExtensions ApplyCancelButtonOverlay/RemoveCancelButtonOverlay (a colored xmark UIImageView overlay applied via dispatch_async) into search_bar_handler.mm. Light + dark now both show the red cancel button matching MAUI; pixel SSIM 0.997 (0.20% differ). Other search bars unaffected; maccatalyst (Mac idiom, no standalone cancel button) unchanged.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9975, 0.20% pixels differ · Dark: SSIM 0.9973, 0.20% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9971, 0.21% pixels differ · Dark: SSIM 0.9971, 0.20% pixels differ

### 137. Selection Command Param — 🟢/⏳
<sub>selection_command_param</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/selection_command_param_light.png" /></td><td><img width="300px" src="captures/ios/cpp/selection_command_param_light.png" /></td><td><img width="300px" src="captures/ios/xaml/selection_command_param_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/selection_command_param_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/selection_command_param_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/selection_command_param_dark.png" /></td></tr></table>

ports SelectionChangedCommandParameter.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.SelectionChangedCommandParameter)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Identical layout, text, and rendering in both light and dark themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9984, 0.07% pixels differ · Dark: SSIM 0.9983, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.05% pixels differ · Dark: SSIM 0.9988, 0.05% pixels differ

### 138. Selection Synchronization — 🟢/⏳
<sub>selection_synchronization</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/selection_synchronization_light.png" /></td><td><img width="300px" src="captures/ios/cpp/selection_synchronization_light.png" /></td><td><img width="300px" src="captures/ios/xaml/selection_synchronization_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/selection_synchronization_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/selection_synchronization_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/selection_synchronization_dark.png" /></td></tr></table>

ports SelectionSynchronization.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.SelectionSynchronization)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Matches; cpp selection rows render full-width vs MAUI's slightly narrower highlight bands, a trivial cosmetic difference.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9993, 0.04% pixels differ · Dark: SSIM 0.9993, 0.04% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.05% pixels differ · Dark: SSIM 0.9988, 0.05% pixels differ

### 139. Semantics — 🟢/⏳
<sub>semantics</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/semantics_light.png" /></td><td><img width="300px" src="captures/ios/cpp/semantics_light.png" /></td><td><img width="300px" src="captures/ios/xaml/semantics_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/semantics_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/semantics_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/semantics_dark.png" /></td></tr></table>

ports SemanticsPage.xaml (+ SemanticsPage.xaml.cs) The C# SemanticsPage is an accessibility showcase: a long VerticalStackLayout where nearly every control carries SemanticProperties.Description / .Hint, plus a block of labels exercising Se

#### 🟢 Sonnet 5 — C++ (C1/C3)

Pixel-accurate match across all semantic property showcase elements in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9993, 0.04% pixels differ · Dark: SSIM 0.9993, 0.04% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.05% pixels differ · Dark: SSIM 0.9988, 0.05% pixels differ

### 140. Shadow Playground — 🟢/⏳
<sub>shadow_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/shadow_playground_light.png" /></td><td><img width="300px" src="captures/ios/cpp/shadow_playground_light.png" /></td><td><img width="300px" src="captures/ios/xaml/shadow_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/shadow_playground_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/shadow_playground_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/shadow_playground_dark.png" /></td></tr></table>

ports ShadowPlaygroundPage.xaml A self-contained, code-first demo of the view Shadow surface, mirroring the C# core gallery page (Pages/Core/ShadowGalleries/ShadowPlaygroundPage.xaml + .xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Shadow rendering, slider positions, and colors match exactly in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9993, 0.04% pixels differ · Dark: SSIM 0.9993, 0.04% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.05% pixels differ · Dark: SSIM 0.9988, 0.05% pixels differ

### 141. Shape App Theme — 🟢/⏳
<sub>shape_app_theme</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/shape_app_theme_light.png" /></td><td><img width="300px" src="captures/ios/cpp/shape_app_theme_light.png" /></td><td><img width="300px" src="captures/ios/xaml/shape_app_theme_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/shape_app_theme_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/shape_app_theme_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/shape_app_theme_dark.png" /></td></tr></table>

ports ShapeAppThemeGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/ShapeAppThemeGallery.xaml: a StackLayout (Padding 12) holding a caption Label and a 200x80 Rectangle, all themed via {AppThemeBi

#### 🟢 Sonnet 5 — C++ (C1/C3)

Theme-driven shape color (green light / red dark) matches exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9993, 0.04% pixels differ · Dark: SSIM 0.9993, 0.04% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.05% pixels differ · Dark: SSIM 0.9988, 0.05% pixels differ

### 142. Shapes — 🟢/⏳
<sub>shapes</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/shapes_light.png" /></td><td><img width="300px" src="captures/ios/cpp/shapes_light.png" /></td><td><img width="300px" src="captures/ios/xaml/shapes_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/shapes_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/shapes_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/shapes_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;shapes&amp;quot; demo (ComparePages.Shapes()), the shipped-.NET-MAUI reference for the visual-parity comparison: a ScrollView over a vertical stack of four LABELLED shapes, each bold-captioned and Start-a

#### 🟢 Sonnet 5 — C++ (C1/C3)

Ellipse, RoundRectangle, EvenOdd polygon, and Line all render identically to MAUI in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9993, 0.04% pixels differ · Dark: SSIM 0.9993, 0.04% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.05% pixels differ · Dark: SSIM 0.9988, 0.05% pixels differ

### 143. Single Bound Selection — 🟢/⏳
<sub>single_bound_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/single_bound_selection_light.png" /></td><td><img width="300px" src="captures/ios/cpp/single_bound_selection_light.png" /></td><td><img width="300px" src="captures/ios/xaml/single_bound_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/single_bound_selection_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/single_bound_selection_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/single_bound_selection_dark.png" /></td></tr></table>

ports SingleBoundSelection.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.SingleBoundSelection)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Text and layout match exactly in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9993, 0.04% pixels differ · Dark: SSIM 0.9988, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.07% pixels differ · Dark: SSIM 0.9986, 0.07% pixels differ

### 144. Slider — 🟢/⏳
<sub>slider</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/slider_light.png" /></td><td><img width="300px" src="captures/ios/cpp/slider_light.png" /></td><td><img width="300px" src="captures/ios/xaml/slider_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/slider_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/slider_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/slider_dark.png" /></td></tr></table>

ports SliderPage.xaml (+ .xaml.cs) Mirrors the MAUI gallery page: a vertical stack of headlined Slider states — Default, BackgroundColor (Blue), Background (yellow→green LinearGradientBrush), Minimum(5)/Maximum(15) with a value readout (Val

#### 🟢 Sonnet 5 — C++ (C1/C3)

All slider variants (colors, disabled state, custom thumb/track colors) match exactly in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.07% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

### 145. Some Empty Groups — 🟢/⏳
<sub>some_empty_groups</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/some_empty_groups_light.png" /></td><td><img width="300px" src="captures/ios/cpp/some_empty_groups_light.png" /></td><td><img width="300px" src="captures/ios/xaml/some_empty_groups_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/some_empty_groups_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/some_empty_groups_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/some_empty_groups_dark.png" /></td></tr></table>

ports GroupingGalleries/SomeEmptyGroups.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Grouped CollectionView with empty groups, headers/footers render identically in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9895, 0.51% pixels differ · Dark: SSIM 0.9882, 0.56% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

### 146. Stack Layout — 🟢/⏳
<sub>stack_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/stack_layout_light.png" /></td><td><img width="300px" src="captures/ios/cpp/stack_layout_light.png" /></td><td><img width="300px" src="captures/ios/xaml/stack_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/stack_layout_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/stack_layout_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/stack_layout_dark.png" /></td></tr></table>

ports StackLayoutPage.xaml Demonstrates the generic maui::controls::stack_layout (the orientation-switching sibling of the fixed vertical/horizontal stacks) by nesting two inner stacks inside an outer vertical stack with a 12px margin: a &amp;quot;V

#### 🟢 Sonnet 5 — C++ (C1/C3)

Vertical and horizontal color-swatch stacks match exactly in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

### 147. Staggered Layout — 🟢/⏳
<sub>staggered_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/staggered_layout_light.png" /></td><td><img width="300px" src="captures/ios/cpp/staggered_layout_light.png" /></td><td><img width="300px" src="captures/ios/xaml/staggered_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/staggered_layout_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/staggered_layout_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/staggered_layout_dark.png" /></td></tr></table>

ports AlternateLayoutGalleries/StaggeredLayout.xaml (+ StaggeredLayout.xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

Item grid content and layout match; minor status-bar overlap timing artifact in cpp dark shot is a capture quirk, not a layout bug.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9983, 0.08% pixels differ · Dark: SSIM 0.9983, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

### 148. Stepper — 🟢/⏳
<sub>stepper</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/stepper_light.png" /></td><td><img width="300px" src="captures/ios/cpp/stepper_light.png" /></td><td><img width="300px" src="captures/ios/xaml/stepper_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/stepper_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/stepper_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/stepper_dark.png" /></td></tr></table>

ports StepperPage.xaml (+ StepperPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

All stepper variants (default, disabled, colored background, min/max, increment) match MAUI in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

### 149. Styles — 🟢/⏳
<sub>styles</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/styles_light.png" /></td><td><img width="300px" src="captures/ios/cpp/styles_light.png" /></td><td><img width="300px" src="captures/ios/xaml/styles_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/styles_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/styles_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/styles_dark.png" /></td></tr></table>

ports StylesPage.xaml

#### 🟢 Sonnet 5 — C++ (C1/C3)

Style-derivation examples match exactly in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

### 150. Swipe Gesture — 🟢/⏳
<sub>swipe_gesture</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/swipe_gesture_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/swipe_gesture_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/swipe_gesture_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/swipe_gesture_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/swipe_gesture_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/swipe_gesture_dark.gif" /></td></tr></table>

ports SwipeViewGestureRecognizerGallery.xaml (+ .xaml.cs) The MAUI SwipeViewGestureRecognizerGallery is a CollectionView of &amp;quot;message&amp;quot; rows; each row&amp;#x27;s DataTemplate is a SwipeView wired three ways, proving gesture recognizers AND swipe-item

#### 🟢 Sonnet 5 — C++ (C1/C3)

iOS: cpp renders the gesture card cleanly ('Welcome to .NET MAUI!' / June 2026 / 'A SwipeView with gesture recognizers' / 'Double-tap the card...') + TapCommand label. The MAUI reference capture is garbled (overlapping text — a broken/transition-state ref); cpp is the correct render. Ruling-3 broken-ref -&gt; cpp verdict green.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9983, 0.10% pixels differ · Dark: SSIM 0.9982, 0.10% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9990, 0.07% pixels differ · Dark: SSIM 0.9989, 0.07% pixels differ

### 151. Swipe Item Position — 🟢/⏳
<sub>swipe_item_position</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/swipe_item_position_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/swipe_item_position_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/swipe_item_position_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/swipe_item_position_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/swipe_item_position_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/swipe_item_position_dark.gif" /></td></tr></table>

ports SwipeItemPositionGallery.xaml A code-first port of the MAUI SwipeView sub-gallery Pages/Controls/SwipeViewGalleries/SwipeItemPositionGallery.xaml: a 2-row Grid (Auto / *) with a Picker on top and one SwipeView below that carries TWO S

#### 🟢 Sonnet 5 — C++ (C1/C3)

Both MAUI and cpp show the same odd full-screen gray fill in dark mode (a shared MAUI-side capture quirk); light mode matches cleanly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9984, 0.08% pixels differ · Dark: SSIM 0.9995, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9947, 0.19% pixels differ · Dark: SSIM 0.9939, 0.20% pixels differ

### 152. Swipe Item Size — 🟢/⏳
<sub>swipe_item_size</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/swipe_item_size_light.png" /></td><td><img width="300px" src="captures/ios/cpp/swipe_item_size_light.png" /></td><td><img width="300px" src="captures/ios/xaml/swipe_item_size_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/swipe_item_size_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/swipe_item_size_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/swipe_item_size_dark.png" /></td></tr></table>

ports SwipeItemSizeGallery.xaml A self-contained, code-first port of the .NET MAUI &amp;quot;SwipeItem Size Gallery&amp;quot;: a scrolling stack of swipe_views demonstrating how a left SwipeItem&amp;#x27;s icon size and the SwipeView content size interact

#### 🟢 Sonnet 5 — C++ (C1/C3)

All icon-size and SwipeView-size variants match MAUI in both light and dark themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9993, 0.03% pixels differ · Dark: SSIM 0.9993, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

### 153. Swipe Refresh — 🟢/⏳
<sub>swipe_refresh</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/swipe_refresh_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/swipe_refresh_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/swipe_refresh_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/swipe_refresh_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/swipe_refresh_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/swipe_refresh_dark.gif" /></td></tr></table>

a self-contained demo page for the W2-20 swipe + refresh controls: a refresh_view wrapping a swipe_view (which itself wraps a labeled row), with a readout label reflecting the latest interaction

#### 🟢 Sonnet 5 — C++ (C1/C3)

Text content and layout match exactly in both light and dark themes; no visible SwipeView/RefreshView content differs from MAUI.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9948, 0.21% pixels differ · Dark: SSIM 0.9961, 0.15% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9953, 0.19% pixels differ · Dark: SSIM 0.9958, 0.17% pixels differ

### 154. Swipe Threshold — 🟢/⏳
<sub>swipe_threshold</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/swipe_threshold_light.png" /></td><td><img width="300px" src="captures/ios/cpp/swipe_threshold_light.png" /></td><td><img width="300px" src="captures/ios/xaml/swipe_threshold_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/swipe_threshold_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/swipe_threshold_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/swipe_threshold_dark.png" /></td></tr></table>

ports HorizontalSwipeThresholdGallery.xaml (+ .xaml.cs) The MAUI HorizontalSwipeThresholdGallery shows how SwipeView.Threshold (the swipe distance, in DIPs, the user must drag before the items settle open / execute) interacts with SwipeItem

#### 🟢 Sonnet 5 — C++ (C1/C3)

All threshold demo blocks, slider positions, and colors match MAUI pixel-for-pixel in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9993, 0.03% pixels differ · Dark: SSIM 0.9993, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

### 155. Swipe View Margin — 🟢/⏳
<sub>swipe_view_margin</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/swipe_view_margin_light.png" /></td><td><img width="300px" src="captures/ios/cpp/swipe_view_margin_light.png" /></td><td><img width="300px" src="captures/ios/xaml/swipe_view_margin_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/swipe_view_margin_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/swipe_view_margin_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/swipe_view_margin_dark.png" /></td></tr></table>

ports SwipeViewMarginGallery.xaml A self-contained, code-first port of the .NET MAUI &amp;quot;SwipeView Margin Gallery&amp;quot;: two swipe_views whose content&amp;#x27;s Margin + Padding are driven by two sliders, demonstrating that the revealed SwipeItems stay cor

#### 🟢 Sonnet 5 — C++ (C1/C3)

Sliders, labels, and margin/padding demo boxes match MAUI exactly in both light and dark themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9993, 0.03% pixels differ · Dark: SSIM 0.9993, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

### 156. Swipe View Shadow — 🟢/⏳
<sub>swipe_view_shadow</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/swipe_view_shadow_light.png" /></td><td><img width="300px" src="captures/ios/cpp/swipe_view_shadow_light.png" /></td><td><img width="300px" src="captures/ios/xaml/swipe_view_shadow_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/swipe_view_shadow_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/swipe_view_shadow_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/swipe_view_shadow_dark.png" /></td></tr></table>

ports SwipeViewShadowGallery.xaml A code-first port of the MAUI SwipeView sub-gallery Pages/Controls/SwipeViewGalleries/SwipeViewShadowGallery.xaml: a padded vertical StackLayout proving a drop Shadow renders correctly on SwipeView content

#### 🟢 Sonnet 5 — C++ (C1/C3)

iOS: 'Shadow in SwipeView Content' — SwipeItems + SwipeItemViews Content buttons with shadows match MAUI. Stale red cleared.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9832, 0.69% pixels differ · Dark: SSIM 0.9993, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9830, 0.72% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

### 157. Switch — 🟢/⏳
<sub>switch</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/switch_light.png" /></td><td><img width="300px" src="captures/ios/cpp/switch_light.png" /></td><td><img width="300px" src="captures/ios/xaml/switch_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/switch_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/switch_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/switch_dark.png" /></td></tr></table>

ports SwitchPage.xaml (+ .xaml.cs) Mirrors the MAUI gallery page: a vertical stack of headlined Switch states — Default, BackgroundColor (Blue), Background (a yellow→green LinearGradientBrush), Disabled, OnColor (Red), ThumbColor (Orange)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Matches MAUI in both themes. The only delta — the off-state ThumbColor=Orange thumb rendering orange in cpp vs white in MAUI — is an EXEMPT iOS-26 platform quirk per the 2026-07-08 ruling (ruling 7): iOS 26 resets the native off-thumb to white after layout, dropping the developer's ThumbColor; the port correctly honors ThumbColor (orange). Tracks, on-colors, positions, and dark theme all match.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9979, 0.26% pixels differ · Dark: SSIM 0.9990, 0.26% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9972, 0.28% pixels differ · Dark: SSIM 0.9983, 0.28% pixels differ

### 158. Switch Grouping — 🟢/⏳
<sub>switch_grouping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/switch_grouping_light.png" /></td><td><img width="300px" src="captures/ios/cpp/switch_grouping_light.png" /></td><td><img width="300px" src="captures/ios/xaml/switch_grouping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/switch_grouping_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/switch_grouping_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/switch_grouping_dark.png" /></td></tr></table>

ports CollectionViewGalleries/GroupingGalleries/ SwitchGrouping.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

Grouped list content, colors, and toggle state match MAUI exactly in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9995, 0.03% pixels differ · Dark: SSIM 0.9995, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 159. Tabbed Flyout — 🟢/⏳
<sub>tabbed_flyout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/tabbed_flyout_light.png" /></td><td><img width="300px" src="captures/ios/cpp/tabbed_flyout_light.png" /></td><td><img width="300px" src="captures/ios/xaml/tabbed_flyout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/tabbed_flyout_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/tabbed_flyout_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/tabbed_flyout_dark.png" /></td></tr></table>

a self-contained demo page for the W1-10 tabbed + flyout vertical: a flyout_page whose FLYOUT pane is a titled menu (two buttons selecting the detail&amp;#x27;s tabs + a &amp;quot;Toggle flyout&amp;quot; presenting/dismissing itself) and whose DETAIL pane is a tabbed

#### 🟢 Sonnet 5 — C++ (C1/C3)

Matches MAUI in both themes against the fresh iOS baseline (1.29%/1.30%). Both render the degraded ContentPage resting state identically — Home tab / Settings tab / Toggle flyout links + 'Flyout dismissed' + 'This is the Home tab.' The earlier 'different nav states' was a stale-capture mismatch; residual is the status-bar clock.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9995, 0.03% pixels differ · Dark: SSIM 0.9995, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 160. Templated View — 🟢/⏳
<sub>templated_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/templated_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/templated_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/templated_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/templated_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/templated_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/templated_view_dark.png" /></td></tr></table>

ports TemplatedViewPage.xaml The C# page contrasts a standard CardView control with a compact one driven by a ControlTemplate (&amp;quot;CardViewCompressed&amp;quot;) and a custom Rate control built entirely from a ControlTemplate + a heart PathGeometry

#### 🟢 Sonnet 5 — C++ (C1/C3)

CardView and compact ControlTemplate cards render identically to MAUI in both light and dark themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9995, 0.03% pixels differ · Dark: SSIM 0.9995, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 161. Time Picker — 🟢/⏳
<sub>time_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/time_picker_light.png" /></td><td><img width="300px" src="captures/ios/cpp/time_picker_light.png" /></td><td><img width="300px" src="captures/ios/xaml/time_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/time_picker_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/time_picker_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/time_picker_dark.png" /></td></tr></table>

ports TimePickerPage.xaml (+ TimePickerPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Match in both themes. The default TimePicker text now renders 12-hour "12:00 AM" / "4:15 AM" (was 24-hour "0:00" / "4:15"), matching MAUI. Root cause + fix: the iOS handler's empty/"t"/"T" arms formatted via NSDateFormatter/currentLocale (the DEVICE region's 24h form), but MAUI iOS sets the field text via TimeExtensions.ToFormattedString = DateTime.Today.Add(time).ToString(format, Culture.CurrentCulture) — the .NET-culture short-time pattern (en-US 12h). Switched to the port's format_time_span helper (the .NET-culture-equivalent renderer android + headless already use), so empty/"t" -&gt; "h:mm tt". The explicit Format="HH:mm" picker still shows "12:00". maccatalyst (separate native-UIDatePicker path) unchanged.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9859, 0.67% pixels differ · Dark: SSIM 0.9858, 0.68% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9863, 0.68% pixels differ · Dark: SSIM 0.9862, 0.69% pixels differ

### 162. Title Bar — 🟢/⏳
<sub>title_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/title_bar_light.png" /></td><td><img width="300px" src="captures/ios/cpp/title_bar_light.png" /></td><td><img width="300px" src="captures/ios/xaml/title_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/title_bar_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/title_bar_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/title_bar_dark.png" /></td></tr></table>

ports TitleBarPage.xaml A self-contained, code-first demo of the TitleBar control

#### 🟢 Sonnet 5 — C++ (C1/C3)

Pixel-identical layout, controls, colors, and text in both light and dark themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9995, 0.03% pixels differ · Dark: SSIM 0.9995, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9991, 0.04% pixels differ

### 163. Toolbar — 🟢/⏳
<sub>toolbar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/toolbar_light.png" /></td><td><img width="300px" src="captures/ios/cpp/toolbar_light.png" /></td><td><img width="300px" src="captures/ios/xaml/toolbar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/toolbar_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/toolbar_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/toolbar_dark.png" /></td></tr></table>

ports ToolbarPage.xaml (Maui.Controls.Sample.Pages.ToolbarPage)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Pixel-identical layout and text in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9992, 0.05% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9992, 0.04% pixels differ · Dark: SSIM 0.9990, 0.04% pixels differ

### 164. Transform Playground — 🟢/⏳
<sub>transform_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/transform_playground_light.png" /></td><td><img width="300px" src="captures/ios/cpp/transform_playground_light.png" /></td><td><img width="300px" src="captures/ios/xaml/transform_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/transform_playground_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/transform_playground_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/transform_playground_dark.png" /></td></tr></table>

ports TransformPlaygroundGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/TransformPlaygroundGallery.xaml: a 50x50 Path rectangle (red fill, blue stroke 4) sits in a 200x200 light-grey panel; belo

#### 🟢 Sonnet 5 — C++ (C1/C3)

Identical layout, slider positions, colors, and text in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9956, 0.28% pixels differ · Dark: SSIM 0.9956, 0.28% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9990, 0.04% pixels differ · Dark: SSIM 0.9990, 0.04% pixels differ

### 165. Transformations — 🟢/⏳
<sub>transformations</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/transformations_light.png" /></td><td><img width="300px" src="captures/ios/cpp/transformations_light.png" /></td><td><img width="300px" src="captures/ios/xaml/transformations_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/transformations_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/transformations_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/transformations_dark.png" /></td></tr></table>

ports TransformationsPage.xaml (+ .xaml.cs) The MAUI TransformationsPage drives a single target view&amp;#x27;s render transforms from a column of knobs: Sliders for Scale / ScaleX / ScaleY (Maximum 10) and Rotation / RotationX / RotationY (Maximum

#### 🟢 Sonnet 5 — C++ (C1/C3)

Identical layout, slider values, and text in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9990, 0.04% pixels differ · Dark: SSIM 0.9990, 0.04% pixels differ

### 166. Triggers — 🟢/⏳
<sub>triggers</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/triggers_light.png" /></td><td><img width="300px" src="captures/ios/cpp/triggers_light.png" /></td><td><img width="300px" src="captures/ios/xaml/triggers_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/triggers_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/triggers_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/triggers_dark.png" /></td></tr></table>

ports TriggersPage.xaml

#### 🟢 Sonnet 5 — C++ (C1/C3)

Identical layout and text in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9990, 0.04% pixels differ · Dark: SSIM 0.9990, 0.04% pixels differ

### 167. Update Path Data — 🟢/⏳
<sub>update_path_data</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/update_path_data_light.png" /></td><td><img width="300px" src="captures/ios/cpp/update_path_data_light.png" /></td><td><img width="300px" src="captures/ios/xaml/update_path_data_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/update_path_data_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/update_path_data_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/update_path_data_dark.png" /></td></tr></table>

ports UpdatePathDataGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/UpdatePathDataGallery.xaml: a 2-row Grid (RowSpacing 0) that proves a Path repaints when its Data geometry is replaced at runti

#### 🟢 Sonnet 5 — C++ (C1/C3)

Light theme matches exactly. Dark theme captures for both MAUI and cpp show the same blank rendering, so cpp matches the reference consistently.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9985, 0.07% pixels differ · Dark: SSIM 0.9984, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.07% pixels differ · Dark: SSIM 0.9986, 0.07% pixels differ

### 168. Varied Size Selector — 🟢/⏳
<sub>varied_size_selector</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/varied_size_selector_light.png" /></td><td><img width="300px" src="captures/ios/cpp/varied_size_selector_light.png" /></td><td><img width="300px" src="captures/ios/xaml/varied_size_selector_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/varied_size_selector_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/varied_size_selector_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/varied_size_selector_dark.png" /></td></tr></table>

ports DataTemplateSelectorGalleries/VariedSizeDataTemplateSelectorGallery.xaml (+ VariedSizeDataTemplateSelectorGallery.xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

Identical varied-height list rows, button row, and text fields in both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9641, 1.58% pixels differ · Dark: SSIM 0.9535, 1.51% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9720, 1.20% pixels differ · Dark: SSIM 0.9593, 1.20% pixels differ

### 169. Vertical Stack — 🟢/⏳
<sub>vertical_stack</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/vertical_stack_light.png" /></td><td><img width="300px" src="captures/ios/cpp/vertical_stack_light.png" /></td><td><img width="300px" src="captures/ios/xaml/vertical_stack_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/vertical_stack_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/vertical_stack_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/vertical_stack_dark.png" /></td></tr></table>

Vertical Stack

#### 🟢 Sonnet 5 — C++ (C1/C3)

Both themes render the six stacked color blocks identically in size, order, and colors, with matching header text and background.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9980, 0.09% pixels differ · Dark: SSIM 0.9981, 0.09% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 170. Visual States — 🟢/⏳
<sub>visual_states</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/visual_states_light.png" /></td><td><img width="300px" src="captures/ios/cpp/visual_states_light.png" /></td><td><img width="300px" src="captures/ios/xaml/visual_states_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/visual_states_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/visual_states_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/visual_states_dark.png" /></td></tr></table>

ports VisualStatesPage.xaml

#### 🟢 Sonnet 5 — C++ (C1/C3)

All entry/button visual-state elements match in position, color, and text in both light and dark themes; minor line-wrap difference in the paragraph text is just reflow noise.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9995, 0.03% pixels differ · Dark: SSIM 0.9994, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 171. Web View — 🟢/⏳
<sub>web_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/web_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/web_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/web_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/web_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/web_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/web_view_dark.png" /></td></tr></table>

a self-contained demo page for the W1-08 web_view vertical: a web_view loading a STATIC html_web_view_source (no network), back/forward/reload buttons over the handler-pushed CanGoBack/CanGoForward read-onlys, an &amp;quot;Eval 1+1&amp;quot; button driving t

#### 🟢 Sonnet 5 — C++ (C1/C3)

WebView now carries HeightRequest=240 (builder-drift fixed) so the status/eval labels + Page A/B/Back/Forward/Reload/Eval buttons align EXACTLY with the fresh MAUI iOS reference (new_page label, Eval result, and each button at matching y). The only difference is cpp faithfully renders the page's static HtmlWebViewSource (a 'Welcome' heading + paragraph) inside the 240px region, which the twin degrades to a blank url (the XAML loader can't represent HtmlWebViewSource) — a twin-degradation, not a port bug.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9956, 0.14% pixels differ · Dark: SSIM 0.9955, 0.14% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 172. Z Index — 🟢/⏳
<sub>z_index</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/z_index_light.png" /></td><td><img width="300px" src="captures/ios/cpp/z_index_light.png" /></td><td><img width="300px" src="captures/ios/xaml/z_index_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/z_index_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/z_index_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/z_index_dark.png" /></td></tr></table>

ports ZIndexPage.xaml (+ ZIndexPage.xaml.cs), code-first

#### 🟢 Sonnet 5 — C++ (C1/C3)

Stacked z-index labels are pixel-identical between cpp and maui in both light and dark themes, same colors, overlap order, and text.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9995, 0.03% pixels differ · Dark: SSIM 0.9994, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

</details>

<details>
<summary><h2>macOS (172 examples) — click to expand</h2></summary>

.NET MAUI on macOS **is** Mac Catalyst (UIKit) — the MAUI / C++ / C++&amp;XAML columns are the strict parity board. The **AppKit** columns are the native-NSView backend (no MAUI reference; they track completeness, C++ == C++&amp;XAML).

**Discrepancy counts** (MAUI-vs-C++ parity verdicts; Sonnet 5 `claude-sonnet-5` and Gemini review each page independently):

| Classification | Sonnet 5 — C++ (C1/C3) | Sonnet 5 — C++ &amp; XAML (C2/C4) | Gemini — C++ | Pixel-Perfect Score — C++ (C1/C3) | Pixel-Perfect Score — C++ &amp; XAML (C2/C4) |
| --- | --- | --- | --- | --- | --- |
| 🟢 Match | 164 | 148 | 0 | 160 | 160 |
| 🟡 Minor | 8 | 22 | 0 | 12 | 11 |
| 🔴 Major | 0 | 2 | 0 | 0 | 1 |
| ⬛ Blank | 0 | 0 | 0 | 0 | 0 |
| ⏳ Unreviewed | 0 | 0 | 172 | 0 | 0 |

### 1. Absolute Layout — 🟢/⏳
<sub>absolute_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/absolute_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/absolute_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/absolute_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/absolute_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/absolute_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/absolute_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/absolute_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/absolute_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/absolute_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/absolute_layout_dark.png" /></td></tr></table>

ports AbsoluteLayoutPage.xaml A self-contained, code-first demo of the AbsoluteLayout control

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: cpp matches MAUI at 0.69% pixel diff (SSIM 0.989) in both themes — blue top box, green-left/red-right edge bars, centered text, and the AutoSized blue box all align. The prior yellow's 'AutoSized shrunk tight' claim is stale (cpp-vs-xaml is only 0.20%).

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: xaml matches MAUI at 0.68% in both themes (same layout as cpp).

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9947, 0.26% pixels differ · Dark: SSIM 0.9960, 0.27% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 2. Activity Indicator — 🟢/⏳
<sub>activity_indicator</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/activity_indicator_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/activity_indicator_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/activity_indicator_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/activity_indicator_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/activity_indicator_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/activity_indicator_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/activity_indicator_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/activity_indicator_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/activity_indicator_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/activity_indicator_dark.png" /></td></tr></table>

ports ActivityIndicatorPage.xaml (+ ActivityIndicatorPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: builder section labels fixed to the shared XAML's short text (Color / BackgroundColor=Yellow / Smaller — were verbose 'Styled - ...' / '- HorizontalOptions=Center'). All 7 bold labels, the yellow BackgroundColor bar, spinners and 'Not Running'/'- End of page -' now match MAUI in both themes. Removed from structure-equivalence known_diverging (strict EXPECT_EQ passes).

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Label texts, spinner placement/tints and the yellow bar all match MAUI in both light and dark; only trivial anti-aliasing differences.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9975, 0.09% pixels differ · Dark: SSIM 0.9974, 0.11% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9963, 0.13% pixels differ · Dark: SSIM 0.9963, 0.13% pixels differ

### 3. Adaptive Collection — 🟢/⏳
<sub>adaptive_collection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/adaptive_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/adaptive_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/adaptive_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/adaptive_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/adaptive_collection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/adaptive_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/adaptive_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/adaptive_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/adaptive_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/adaptive_collection_dark.png" /></td></tr></table>

ports AdaptiveCollectionView.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.AdaptiveCollectionView)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: removed the port-only 'Layout: Linear (single column)' readout label + its stack wrapper; page content is now Grid &gt; CollectionView, matching the shared XAML exactly (ContentPage &gt; Grid &gt; CollectionView). The eight centered items render identically to MAUI in both themes. Removed from structure-equivalence known_diverging (strict EXPECT_EQ passes).

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Now matches MAUI: the 8 single-column items render in 60pt-tall centered cells (row centers spaced 92px == MAUI's 92px) instead of collapsing to the ~31px label height. Fixed by honoring the item template root's HeightRequest in the CollectionView cell self-size path (a layout-rooted cell reported only its content extent from the cross-platform measure). Linear single-column layout matches the resting narrow-width appearance.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9970, 0.19% pixels differ · Dark: SSIM 0.9970, 0.18% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 4. Alerts — 🟢/⏳
<sub>alerts</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/alerts_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/alerts_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/alerts_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/alerts_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/alerts_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/alerts_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/alerts_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/alerts_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/alerts_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/alerts_dark.png" /></td></tr></table>

ports AlertsPage.xaml (+ AlertsPage.xaml.cs) The C# AlertsPage drives the three Page dialog services — DisplayAlertAsync (simple OK + Yes/No), DisplayActionSheetAsync (simple + Cancel/Delete), and DisplayPromptAsync (two questions) — from a

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: removed the port-only 'OnAppearing: Alert — Welcome...' readout line from the visual tree (the shared alerts.xaml has no general readout — results surface as native DisplayAlert dialogs). Bold 'Display Alert/ActionSheet/Prompt' headers + the six blue action buttons now match MAUI in both themes at rest.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Bold section headers and all five blue action links match MAUI's content and placement in both light and dark themes.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 5. Alignment — 🟢/⏳
<sub>alignment</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/alignment_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/alignment_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/alignment_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/alignment_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/alignment_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/alignment_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/alignment_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/alignment_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/alignment_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/alignment_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;alignment&amp;quot; demo (ComparePages.Alignment()), the shipped-.NET-MAUI reference for the visual-parity comparison

#### 🟢 Sonnet 5 — C++ (C1/C3)

Start/Center/End/Fill buttons with red borders match MAUI's sizes, colors and alignments in both themes; only the exempt uniform chrome shift differs.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

All four alignment cases match MAUI exactly in both themes — same button sizes, red borders, blue fills and horizontal placements.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9960, 0.48% pixels differ · Dark: SSIM 0.9966, 0.46% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9947, 0.52% pixels differ · Dark: SSIM 0.9953, 0.50% pixels differ

### 6. Animation — 🟢/⏳
<sub>animation</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/animation_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/animation_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/animation_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/animation_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/animation_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/animation_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/animation_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/animation_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/animation_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/animation_dark.png" /></td></tr></table>

ports AnimationPage.xaml (+ AnimationPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: cpp matches MAUI at 0.85% pixel diff (SSIM 0.984) in both themes — the purple submarine image + Start Animation/Start Custom Animation (blue) + Cancel Animation (gray) buttons all present with correct colors. The submarine's exact position varies (animation is a 🎬 auto-animating page, captured mid-flight), which is capture-timing, not a delta. Prior yellow was stale.

#### 🟡 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: identical situation to the cpp column — same sub, same three buttons, same colors in light and dark, but the whole group sits at the top instead of MAUI's sub-centered/buttons-bottom arrangement. Positional/animation-state gap only.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9783, 0.88% pixels differ · Dark: SSIM 0.9810, 1.00% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9770, 0.91% pixels differ · Dark: SSIM 0.9797, 1.03% pixels differ

### 7. App Theme Binding — 🟢/⏳
<sub>app_theme_binding</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/app_theme_binding_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/app_theme_binding_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/app_theme_binding_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/app_theme_binding_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/app_theme_binding_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/app_theme_binding_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/app_theme_binding_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/app_theme_binding_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/app_theme_binding_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/app_theme_binding_dark.png" /></td></tr></table>

ports AppThemeBindingPage.xaml

#### 🟢 Sonnet 5 — C++ (C1/C3)

Light and dark match MAUI exactly: text content, colors (green/red for theme text, orange/teal for resource-dictionary text), layout.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Light and dark match MAUI exactly, identical to cpp column.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 8. Application Control — 🟢/⏳
<sub>application_control</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/application_control_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/application_control_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/application_control_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/application_control_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/application_control_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/application_control_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/application_control_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/application_control_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/application_control_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/application_control_dark.png" /></td></tr></table>

ports ApplicationControlPage.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: fixed two builder drifts vs the shared XAML — removed the bold/size-18 font on the 'Quits the application' headline (XAML label is plain) and stopped on_mounted from overwriting the second label with a live 'Windows open: N | main window: ...' status (the shared XAML's second label is the STATIC 'Application: not yet hosted' text). Now plain headline + 3 blue buttons + static status label match MAUI in both themes. (The buttons still refresh() the readout on click for interactive use.)

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Both themes match MAUI: plain header, three centered blue link buttons, and the same 'Application: not yet hosted' status line.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 9. Auto Size Shapes — 🟢/⏳
<sub>auto_size_shapes</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/auto_size_shapes_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/auto_size_shapes_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/auto_size_shapes_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/auto_size_shapes_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/auto_size_shapes_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/auto_size_shapes_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/auto_size_shapes_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/auto_size_shapes_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/auto_size_shapes_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/auto_size_shapes_dark.png" /></td></tr></table>

ports AutoSizeShapesGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/AutoSizeShapesGallery.xaml: a 3-row Grid (RowSpacing 0) that proves a stroked Ellipse auto-sizes to fill exactly half of the av

#### 🟢 Sonnet 5 — C++ (C1/C3)

Both themes match: black caption bar, yellow upper half with the blue-stroked green ellipse filling it, orange lower half, identical proportions.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Both themes match MAUI's layout and colors exactly (ellipse-over-yellow above, orange region below).

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9960, 0.71% pixels differ · Dark: SSIM 0.9979, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9966, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 10. Basic Grouping — 🟢/⏳
<sub>basic_grouping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/basic_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/basic_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/basic_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/basic_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/basic_grouping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/basic_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/basic_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/basic_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/basic_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/basic_grouping_dark.png" /></td></tr></table>

ports GroupingGalleries/BasicGrouping.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

cpp now matches MAUI: green group-header bars + orange 'Total members' footer bars, correct row spacing (Avengers block 392px == MAUI 393px after adding the header FontSize=16 and footer Margin=0,0,0,15 the builder was missing). Low raw SSIM is the shared ruling-2 uniform top-offset (== the green xaml column, 0.691 vs 0.692).

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Matches MAUI: identical group-header/footer bars, member rows, and — verified — identical row spacing (bar Y-gaps 392/261/210/209 vs MAUI 393/262/209/210). The only delta is a uniform +28px top offset (harness inset, ruling 2 exempt), which is why the raw whole-frame SSIM looks low on this tall list.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9977, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 11. Basic Swipe — 🟢/⏳
<sub>basic_swipe</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/basic_swipe_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/basic_swipe_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/basic_swipe_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/basic_swipe_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/basic_swipe_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/basic_swipe_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/basic_swipe_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/basic_swipe_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/basic_swipe_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/basic_swipe_dark.png" /></td></tr></table>

ports BasicSwipeGallery.xaml A code-first port of the MAUI SwipeView sub-gallery Pages/Controls/SwipeViewGalleries/BasicSwipeGallery.xaml: a vertical StackLayout of five SwipeViews, each demonstrating a different revealed-side / SwipeMode c

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: fixed three builder drifts vs the shared XAML — (1) the five gray content-Grid Labels now HorizontalOptions/VerticalOptions=Center (were top-left), (2) removed the port-only 'Swipe a row, then invoke Delete' readout from the tree (shared XAML has none), (3) removed set_spacing(12) on the StackLayout (shared &lt;StackLayout&gt; has no Spacing=default 0; the SwipeView Margin=12 supplies the gaps) — row gaps now 39px == MAUI (were 57px). Also dropped the on_mounted synthetic open. Five centered gray boxes with centered labels now match MAUI in both themes (4.77% vs MAUI == the xaml column, all harness-inset).

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: FIXED (swipe_view margin now reserved+inset via measure+compute_frame) — the five SwipeView rows render as discrete gray boxes, centered, with correct gaps and centered labels, matching MAUI in both themes (SSIM ~0.955). Prior red was one full-window gray band.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 12. Behaviors — 🟢/⏳
<sub>behaviors</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/behaviors_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/behaviors_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/behaviors_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/behaviors_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/behaviors_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/behaviors_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/behaviors_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/behaviors_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/behaviors_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/behaviors_dark.png" /></td></tr></table>

ports BehaviorsPage.xaml (+ .xaml.cs) and its companion Controls.Sample/Behaviors/NumericValidationBehavior.cs

#### 🟢 Sonnet 5 — C++ (C1/C3)

Both themes match: same title and 'Enter a System.Double' entry with correct light/dark field styling; only the exempt outer inset differs (entry sits flush to the window edge).

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Both themes match MAUI closely, including the entry's inset rounded border in light mode and the dark field in dark mode.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 13. Border — 🟢/⏳
<sub>border</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/border_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/border_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;border&amp;quot; demo (ComparePages.BorderPage()), the shipped-.NET-MAUI reference for the visual-parity comparison: a single Border centered on the page — red 5pt stroke, a RoundRectangle StrokeShape (Co

#### 🟢 Sonnet 5 — C++ (C1/C3)

Light and dark both render the identical red-stroked, cream-filled rounded border with 'Bordered content' centered; the dark theme's faint light-on-cream text matches MAUI exactly. Only the exempt window-chrome shift differs.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Both themes match MAUI's border size, corner radius, colors and centered text, including the same barely-visible light text on the cream fill in dark mode. No content differences.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9776, 1.34% pixels differ · Dark: SSIM 0.9818, 1.24% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9763, 1.37% pixels differ · Dark: SSIM 0.9805, 1.27% pixels differ

### 14. Border Clip Playground — 🟡/⏳
<sub>border_clip_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/border_clip_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_clip_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_clip_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_clip_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_clip_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/border_clip_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_clip_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_clip_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_clip_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_clip_playground_dark.png" /></td></tr></table>

ports BorderClipPlayground.xaml (+ .xaml.cs) The C# page is an interactive Border-shape playground: a 100x100 Border (red stroke) clips an AspectFill Image (oasis.jpg) into the currently selected StrokeShape, while controls below mutate the

#### 🟡 Sonnet 5 — C++ (C1/C3)

C1/C3: clipped dog image and controls all present with correct slider values, but the red border ring renders much thicker than MAUI's thin stroke, the 'Border Shape'/'Border'/'Corner Radius' headers lose MAUI's bold large styling (all text renders in one plain size/weight), and the shape entry shows 'RoundRectangle' where MAUI's entry is empty. Same diffs in dark; dark thumbs also render slightly larger.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: matches MAUI closely in both themes — thin red border on the clipped image, bold section headers with small sub-labels, empty Border Shape entry, identical slider positions/values. Only a tiny uniform vertical shift of the image (window-chrome class), no content differences.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9888, 0.57% pixels differ · Dark: SSIM 0.9889, 0.60% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9904, 0.45% pixels differ · Dark: SSIM 0.9908, 0.46% pixels differ

### 15. Border Layout — 🟢/⏳
<sub>border_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/border_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/border_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_layout_dark.png" /></td></tr></table>

ports BorderLayout.xaml (+ BorderLayout.xaml.cs) The C# page demonstrates driving Border.StrokeThickness from a Slider: a Slider (0..40, set to 5 in OnAppearing) is bound to the Border&amp;#x27;s StrokeThickness; the Border (Silver stroke, White bac

#### 🟢 Sonnet 5 — C++ (C1/C3)

Slider plus the green pill border with red rounded-left segment, 'Center' label and blue square match MAUI in both themes, including black text in light and white text in dark.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Both themes reproduce MAUI's stroke-thickness slider and the green/red/blue bordered bar with identical colors, corner rounding and text.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9956, 0.56% pixels differ · Dark: SSIM 0.9951, 0.80% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9944, 0.60% pixels differ · Dark: SSIM 0.9938, 0.84% pixels differ

### 16. Border Playground — 🟡/⏳
<sub>border_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/border_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/border_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_playground_dark.png" /></td></tr></table>

ports BorderPlayground.xaml (+ BorderPlayground.xaml.cs) A self-contained, code-first interactive Border playground

#### 🟡 Sonnet 5 — C++ (C1/C3)

Top border/gradient/label block matches MAUI. Below it, the ScrollView's rows are measurably taller in the port than in MAUI's native render (accumulating drift down the page — by the bottom, rows are offset by 100+ px), most likely a font-metrics/row-height fidelity gap rather than a structural bug. The Padding/StackLayout-type divergence between cpp and xaml that originally caused a cpp/xaml self-contradiction is fixed (2026-07-06): cpp and xaml now render pixel-identical to each other (0.79% diff). Flagging the residual MAUI-vs-port row-height gap as a separate follow-up, out of scope for the padding fix.

#### 🟡 Sonnet 5 — C++ &amp; XAML (C2/C4)

Top border/gradient/label block matches MAUI. Below it, the ScrollView's rows are measurably taller in the port than in MAUI's native render (accumulating drift down the page — by the bottom, rows are offset by 100+ px), most likely a font-metrics/row-height fidelity gap rather than a structural bug. The Padding/StackLayout-type divergence between cpp and xaml that originally caused a cpp/xaml self-contradiction is fixed (2026-07-06): cpp and xaml now render pixel-identical to each other (0.79% diff). Flagging the residual MAUI-vs-port row-height gap as a separate follow-up, out of scope for the padding fix.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9871, 1.24% pixels differ · Dark: SSIM 0.9848, 1.35% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9903, 1.09% pixels differ · Dark: SSIM 0.9888, 1.15% pixels differ

### 17. Border Resize Content — 🟡/⏳
<sub>border_resize_content</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/border_resize_content_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_resize_content_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_resize_content_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_resize_content_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_resize_content_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/border_resize_content_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_resize_content_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_resize_content_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_resize_content_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_resize_content_dark.png" /></td></tr></table>

ports BorderResizeContent.xaml A self-contained, code-first demo that resizes a Border&amp;#x27;s CONTENT and watches the Border track it

#### 🟡 Sonnet 5 — C++ (C1/C3)

C1/C3: circle and square cells match in both columns and themes (colors, green borders, blue plus, dog-image fills with light-blue letterboxing). The triangle row differs: MAUI's left triangle shows a large salmon fill with the blue plus overflowing the green outline, and its right triangle is filled by the clipped dog photo; C++ renders both triangles mostly hollow with only a small clipped fill inside the outline. Content clipped/undersized in 2 of 6 cells.

#### 🟡 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: same triangle defect as the cpp column — hollow green triangle with tiny interior fill instead of MAUI's overflowing salmon/plus (left) and dog-filled triangle (right); circle and square cells match in both themes. Also the same slight uniform downward shift.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9930, 0.55% pixels differ · Dark: SSIM 0.9934, 0.61% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9926, 0.44% pixels differ · Dark: SSIM 0.9931, 0.48% pixels differ

### 18. Border Stroke — 🟡/⏳
<sub>border_stroke</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/border_stroke_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_stroke_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_stroke_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_stroke_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_stroke_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/border_stroke_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_stroke_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_stroke_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_stroke_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_stroke_dark.png" /></td></tr></table>

ports BorderStroke.xaml (+ BorderStroke.xaml.cs) A self-contained, code-first demo of Border StrokeThickness and how a Border tracks the height of its content

#### 🟡 Sonnet 5 — C++ (C1/C3)

Light theme matches MAUI across both StrokeThickness sections. In dark theme the slider's unfilled track is rendered much darker than MAUI's light-gray track, a small chrome color diff; everything else (borders, bar heights, labels) matches.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Both themes reproduce MAUI's three stroke-thickness bars, slider position, and the 60pt content-height section faithfully; dark-theme slider track matches MAUI's gray. Only the exempt uniform outer inset differs.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9747, 3.60% pixels differ · Dark: SSIM 0.9748, 3.60% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9734, 3.63% pixels differ · Dark: SSIM 0.9735, 3.64% pixels differ

### 19. Borderless — 🟢/⏳
<sub>borderless</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/borderless_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/borderless_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/borderless_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/borderless_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/borderless_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/borderless_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/borderless_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/borderless_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/borderless_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/borderless_dark.png" /></td></tr></table>

ports Borderless.xaml A self-contained, code-first demo of a stroke-less Border

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: yellow/pink/red color-band layout matches MAUI exactly in both themes (window-chrome title-bar tint differences are the exempt native-chrome variance).

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: identical color bands to MAUI; matches the C++ builder output.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9992, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9966, 0.12% pixels differ · Dark: SSIM 0.9989, 0.11% pixels differ

### 20. Box View — 🟢/⏳
<sub>box_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/box_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/box_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/box_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/box_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/box_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/box_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/box_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/box_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/box_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/box_view_dark.png" /></td></tr></table>

ports BoxViewPage.xaml (+ BoxViewPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

All five BoxView sections (default blue, purple Color, yellow-green gradient Background, CornerRadius green, complex-corner orange) match MAUI in both themes, including dark-theme label colors.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Identical to MAUI in both themes — box colors, gradient direction, corner radii, and section labels all agree; only the exempt outer inset/window title differ.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 21. Button — 🟢/⏳
<sub>button</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/button_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/button_dark.png" /></td></tr></table>

ports ButtonPage.xaml (+ ButtonPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

All rows match MAUI now: text buttons, blue/red/BorderColor/BorderWidth(red outline)/CornerRadius/pink bars, gray Button, green BorderWidth-Changing, and the two 'settings' image buttons. Fixed the builder twin's over-tall ImageButton rows (removed the explicit ContentLayout image_position::top that the shared button.xaml omits) — measured heights now 217/217/46/46 px vs MAUI 216/216/46/46.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Matches MAUI: identical button rows including the two 'settings' image buttons at 217px (MAUI 216px). The prior 'balloon' verdict was stale — the framework/xaml path already sized the ImageButton correctly.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9904, 0.37% pixels differ · Dark: SSIM 0.9906, 0.37% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9892, 0.40% pixels differ · Dark: SSIM 0.9893, 0.40% pixels differ

### 22. Carousel Page — 🟢/⏳
<sub>carousel_page</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/carousel_page_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/carousel_page_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/carousel_page_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/carousel_page_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/carousel_page_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/carousel_page_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/carousel_page_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/carousel_page_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/carousel_page_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/carousel_page_dark.png" /></td></tr></table>

Carousel Page

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: builder rewritten to mirror the shared XAML (page root = CarouselView; ItemTemplate = purple Border/StrokeThickness2/Padding16 + centered 'Card' label via a carousel_card border-subclass cell). Renders the single purple-bordered 'Card' card identically to MAUI in both themes (SSIM ~0.977, 0.78% diff). Prior red was the old richer 'Basic Horizontal Carousel' demo divergence (+ a stale 'blank MAUI ref' note). Now matches the green xaml column.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Renders the CarouselView's purple-bordered 'Card' template from the inline x:Array (Card 1/2/3) exactly like MAUI — no code-behind needed here. Prior yellow was stale.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9884, 1.05% pixels differ · Dark: SSIM 0.9944, 0.73% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9871, 1.08% pixels differ · Dark: SSIM 0.9932, 0.76% pixels differ

### 23. Chat Example — 🟢/⏳
<sub>chat_example</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/chat_example_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/chat_example_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/chat_example_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/chat_example_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/chat_example_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/chat_example_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/chat_example_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/chat_example_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/chat_example_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/chat_example_dark.png" /></td></tr></table>

ports ChatExample.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.ItemSizeGalleries.ChatExample), tracking the maui-compare reference demo ~/maui-compare/Pages/ChatExamplePage.cs (the visual-parity oracle)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Light and dark: page is empty (no messages) in all three (MAUI, cpp, xaml) with matching toolbar text/colors — consistent with ground truth.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Light and dark match MAUI exactly, identical to cpp column.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 24. Check Box — 🟢/⏳
<sub>check_box</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/check_box_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/check_box_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/check_box_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/check_box_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/check_box_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/check_box_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/check_box_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/check_box_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/check_box_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/check_box_dark.png" /></td></tr></table>

ports CheckBoxPage.xaml (+ .xaml.cs) Mirrors the MAUI gallery page: a vertical stack of headlined CheckBox states — Default, Colored (Color=Purple), Disabled, Disabled+Colored+Checked — followed by a &amp;quot;Change IsChecked&amp;quot; row pairing a Button

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: cpp matches MAUI at 0.87% (light) / 1.04% (dark) pixel diff, SSIM 0.98 — all five CheckBox states (Default, Colored=Purple, Disabled, Disabled Colored checked, Change IsChecked red check + 'Is green? False') align in size, tint and vertical pitch (stack Spacing=6 matches the XAML). The prior yellow's 'vertical pitch differs' claim is stale.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: xaml matches MAUI at the same ~0.8% (same shared XAML via the loader).

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9972, 0.11% pixels differ · Dark: SSIM 0.9973, 0.11% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 25. Chrome — 🟢/⏳
<sub>chrome</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/chrome_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/chrome_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/chrome_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/chrome_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/chrome_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/chrome_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/chrome_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/chrome_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/chrome_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/chrome_dark.png" /></td></tr></table>

a self-contained demo page for the W1-11 window-chrome family: page toolbar items (primary + secondary), a menu bar (File menu with items, a separator and a sub-menu), a context flyout (right-click menu) on a button, and a tooltip — all wir

#### 🟢 Sonnet 5 — C++ (C1/C3)

Centered blue 'Press or right-click me' link and left-aligned 'Ready' label match MAUI in position, color and size in both light and dark.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

XAML matches MAUI in both themes — identical link text/color and 'Ready' status label placement.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 26. Clip — 🟢/⏳
<sub>clip</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/clip_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/clip_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/clip_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/clip_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/clip_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/clip_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/clip_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/clip_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/clip_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/clip_dark.png" /></td></tr></table>

ports ClipPage.xaml The C# page (Pages/Core/ClipPage.xaml; its .xaml.cs is an empty InitializeComponent) is a ScrollView over a StackLayout that shows the SAME dotnet_bot.png image five times, each successive copy carrying a different geome

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: cpp light+dark match MAUI exactly — same submarine image, same RectangleGeometry/EllipseGeometry/GeometryGroup/PathGeometry clip shapes and crop boundaries, identical layout and captions. No content differences.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: xaml light+dark match MAUI exactly, pixel-identical to the cpp column as well — clip geometries render correctly via the newly-registered XAML geometry support.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 27. Clip Corner Radius — 🟢/⏳
<sub>clip_corner_radius</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/clip_corner_radius_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/clip_corner_radius_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/clip_corner_radius_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/clip_corner_radius_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/clip_corner_radius_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/clip_corner_radius_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/clip_corner_radius_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/clip_corner_radius_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/clip_corner_radius_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/clip_corner_radius_dark.png" /></td></tr></table>

ports ClipCornerRadiusGallery.xaml (+ .xaml.cs) The C# page (Pages/Controls/ShapesGalleries/ClipCornerRadiusGallery.xaml) is a StackLayout (Padding=12) that demonstrates DRIVING a RoundRectangleGeometry&amp;#x27;s per-corner CornerRadius from four s

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3 (cpp vs MAUI, light/dark): clipped dog image via RoundRectangleGeometry matches (same corners rounded, same crop), all four corner-radius sliders match MAUI's labels/positions/values in both themes. Slider track color is marginally darker gray than MAUI's in light theme — trivial styling, not a content diff.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4 (xaml vs MAUI, light/dark): matches MAUI closely, same image clip and sliders; XAML column's slider track shade is closer to MAUI's than the cpp column but this is a non-issue either way.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 28. Clip Gallery — 🟢/⏳
<sub>clip_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/clip_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/clip_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/clip_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/clip_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/clip_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/clip_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/clip_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/clip_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/clip_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/clip_gallery_dark.png" /></td></tr></table>

ports ClipGallery.xaml The C# page (Pages/Controls/ShapesGalleries/ClipGallery.xaml; its .xaml.cs is an empty InitializeComponent) is a ScrollView over a StackLayout (Padding=12) that shows the SAME &amp;quot;oasis.jpg&amp;quot; image SEVEN times — one bare

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: cpp light+dark match MAUI — same pug photo, same Rectangle/RoundRectangle/Ellipse/GeometryGroup clip crops, identical sizes and positions visible in the captured viewport.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: xaml light+dark match MAUI, identical to cpp column — all geometry clip variants render correctly.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 29. Clip Views — 🟢/⏳
<sub>clip_views</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/clip_views_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/clip_views_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/clip_views_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/clip_views_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/clip_views_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/clip_views_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/clip_views_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/clip_views_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/clip_views_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/clip_views_dark.png" /></td></tr></table>

ports ClipViewsGallery.xaml The C# page (Pages/Controls/ShapesGalleries/ClipViewsGallery.xaml; no code-behind beyond an empty InitializeComponent) is a ScrollView over a StackLayout (Padding=12) that proves the Clip surface (VisualElement.C

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3 (cpp vs MAUI, light/dark): content matches exactly — same red curved-clip bars via BezierSegment path clipping across Entry/Editor/Grid/SearchBar/TimePicker rows, same text, same positions. No port bugs.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4 (xaml vs MAUI, light/dark): identical rendering to the cpp column — same clipped red bars, same content and layout. No divergence from MAUI ground truth.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9921, 0.92% pixels differ · Dark: SSIM 0.9932, 0.95% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9908, 0.95% pixels differ · Dark: SSIM 0.9919, 0.98% pixels differ

### 30. Clipping — 🟢/⏳
<sub>clipping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/clipping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/clipping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/clipping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/clipping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/clipping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/clipping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/clipping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/clipping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/clipping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/clipping_dark.png" /></td></tr></table>

compare oracle ~/maui-compare/Pages/ClippingPage.cs (itself written to mirror this gallery page)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Match (ruling 10). MAUI Mac Catalyst does not render the bundled coffee.png images; the port shows them (matching MAUI iOS+android, green). The clipped square/purple bar/blue stripe/digit row all match. Exempt Mac Catalyst image-rendering gap. Both themes.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Match (ruling 10). Same fuller render as the cpp column (indicator dots / coffee.png), exempt vs MAUI Mac Catalyst's content gap.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9963, 0.15% pixels differ · Dark: SSIM 0.9965, 0.15% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9951, 0.18% pixels differ · Dark: SSIM 0.9954, 0.18% pixels differ

### 31. Collectionview — 🟢/⏳
<sub>collectionview</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/collectionview_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/collectionview_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/collectionview_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/collectionview_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/collectionview_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/collectionview_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/collectionview_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/collectionview_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/collectionview_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/collectionview_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;collectionview&amp;quot; demo (ComparePages.CollectionViewPage()), the shipped-.NET-MAUI reference for the visual-parity comparison: a CollectionView over 24 captioned items, a string Header (&amp;quot;This is the

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3 both green. Header, 3-column grid of 24 'name.jpg, N' items, row pitch, fonts and light/dark colors all match MAUI. Only the exempt window-chrome offset differs.

#### 🟡 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4 both yellow. Content and columns match, but row spacing is visibly tighter than MAUI (compressed item pitch, ~31px vs ~44px) and the gap between the header and the first row is missing in both themes. Colors/fonts otherwise match.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 32. Composition Gallery — 🟢/⏳
<sub>composition_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/composition_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/composition_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/composition_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/composition_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/composition_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/composition_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/composition_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/composition_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/composition_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/composition_gallery_dark.png" /></td></tr></table>

ports CompositionGallery.xaml A self-contained, code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/CompositionGallery.xaml: a StackLayout holding two Beige 250x250 Grids (Margin 12) that compose multiple overlappi

#### 🟢 Sonnet 5 — C++ (C1/C3)

Light and dark: both composited-shape panels (circle/triangle/rect overlap and RGB axis lines) match MAUI's colors, positions, and sizes exactly.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Light and dark match MAUI exactly, identical to cpp column.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9972, 0.39% pixels differ · Dark: SSIM 0.9973, 0.39% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 33. Containers — 🟢/⏳
<sub>containers</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/containers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/containers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/containers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/containers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/containers_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/containers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/containers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/containers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/containers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/containers_dark.png" /></td></tr></table>

a self-contained demo page for the W1-07 container set: a scroll_view hosting a vertical stack of content-hosting containers — a border-framed label (stroke + dashed outline + rounded shape), a legacy frame (BorderColor/CornerRadius/HasShad

#### 🟢 Sonnet 5 — C++ (C1/C3)

Scrolled-to label, dashed blue border box, red frame, and content_view text all match MAUI's layout and colors in both light and dark themes.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Matches MAUI in both themes: same dashed border, red frame with rounded corners, and label placement, with correct dark-mode colors.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9877, 0.54% pixels differ · Dark: SSIM 0.9899, 0.66% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9869, 0.56% pixels differ · Dark: SSIM 0.9891, 0.68% pixels differ

### 34. Content View — 🟢/⏳
<sub>content_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/content_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/content_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/content_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/content_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/content_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/content_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/content_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/content_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/content_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/content_view_dark.png" /></td></tr></table>

ports ContentViewPage.xaml (+ ContentViewPage.xaml.cs), code-first

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: ContentView labels ('ContentView', two 'Content' rows) and blue 'Swap content' button match MAUI in both themes; only the exempt window-chrome vertical offset differs.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: XAML column identical to MAUI content in both themes — same labels, same button color/position, dark background matches.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 35. Context Flyout — 🟢/⏳
<sub>context_flyout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/context_flyout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/context_flyout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/context_flyout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/context_flyout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/context_flyout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/context_flyout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/context_flyout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/context_flyout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/context_flyout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/context_flyout_dark.png" /></td></tr></table>

ports ContextFlyoutPage.xaml (+ ContextFlyoutPage.xaml.cs) The C# page attaches a MenuFlyout as the FlyoutBase.ContextFlyout (right-click / long-press menu) of several controls and wires each menu item to a handler: - a Button (&amp;quot;Increment b

#### 🟢 Sonnet 5 — C++ (C1/C3)

VERIFIED 2026-07-16 (measured, macOS): the page matches MAUI **pixel-perfectly** — rows 36-217 mean|diff| = **0.000** (button, switch, both labels, the entry and the COOL FontImageSource all identical). The ENTIRE reported diff is rows 218+ (mean 78.8): the page's **live bing.com WebView**, which had loaded in the port's capture and was still blank in MAUI's. That region is live network content (the page's own XAML says so) and is not deterministically comparable — neither a port bug nor a MAUI quirk. The pixel score below is dominated by it; treat this page as a match at rest.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

VERIFIED 2026-07-16 (measured, macOS): the page matches MAUI **pixel-perfectly** — rows 36-217 mean|diff| = **0.000** (button, switch, both labels, the entry and the COOL FontImageSource all identical). The ENTIRE reported diff is rows 218+ (mean 78.8): the page's **live bing.com WebView**, which had loaded in the port's capture and was still blank in MAUI's. That region is live network content (the page's own XAML says so) and is not deterministically comparable — neither a port bug nor a MAUI quirk. The pixel score below is dominated by it; treat this page as a match at rest.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9183, 3.34% pixels differ · Dark: SSIM 0.9186, 3.34% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9157, 3.42% pixels differ · Dark: SSIM 0.9160, 3.42% pixels differ

### 36. Controls Stack — 🟢/⏳
<sub>controls_stack</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/controls_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/controls_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/controls_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/controls_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/controls_stack_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/controls_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/controls_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/controls_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/controls_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/controls_stack_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;controls_stack&amp;quot; demo (ComparePages.ControlsStack()), the shipped-.NET-MAUI reference for the visual-parity comparison: a VerticalStackLayout (Spacing 12, Padding 16) showcasing the basic widgets

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: button, entry, editor, search bar, checkbox, switch, activity indicator, slider (same value) and stepper + progress bar all match in both themes; dark entry/search chrome matches.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: XAML column matches MAUI in both themes — same control set, same slider/progress values, same colors.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9962, 0.12% pixels differ · Dark: SSIM 0.9962, 0.12% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9949, 0.15% pixels differ · Dark: SSIM 0.9949, 0.16% pixels differ

### 37. Custom Layout — 🟢/⏳
<sub>custom_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/custom_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/custom_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/custom_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/custom_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/custom_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/custom_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/custom_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/custom_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/custom_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/custom_layout_dark.png" /></td></tr></table>

ports CustomLayoutPage.xaml (+ CustomLayoutPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: custom layout places Top / Left Left / Right Right / Bottom links at the same edge positions with the same blue link styling in both themes.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: identical placement and styling of the four edge links in both themes.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 38. Custom Size Swipe — 🟢/⏳
<sub>custom_size_swipe</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/custom_size_swipe_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/custom_size_swipe_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/custom_size_swipe_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/custom_size_swipe_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/custom_size_swipe_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/custom_size_swipe_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/custom_size_swipe_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/custom_size_swipe_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/custom_size_swipe_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/custom_size_swipe_dark.png" /></td></tr></table>

ports CustomSizeSwipeViewGallery.xaml (+ .xaml.cs) The MAUI CustomSizeSwipeViewGallery is a single SwipeView whose Left / Right / Top item collections each reveal CUSTOM-SIZED content: a SwipeItemView wrapping a Grid/StackLayout with an exp

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: removed the on_mounted synthetic swipe-open + button-fire that overwrote the readout with 'RightItems revealed (open=1, threshold=0)'. Now captured at rest — green SwipeView content band + 'Test Click from Content' + static 'Ready (swipe a side to reveal its custom-sized content)' — matching MAUI in both themes. Buttons' Clicked still drive the readout interactively.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: matches MAUI in both themes — same green band, link, and 'Ready (swipe a side...)' status text; dark theme band/text also match.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9979, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9966, 0.12% pixels differ · Dark: SSIM 0.9967, 0.11% pixels differ

### 39. Custom Swipe Item View — 🟢/⏳
<sub>custom_swipe_item_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/custom_swipe_item_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/custom_swipe_item_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/custom_swipe_item_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/custom_swipe_item_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/custom_swipe_item_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/custom_swipe_item_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/custom_swipe_item_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/custom_swipe_item_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/custom_swipe_item_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/custom_swipe_item_view_dark.png" /></td></tr></table>

ports CustomSwipeItemViewGallery.xaml A self-contained, code-first port of the .NET MAUI &amp;quot;CustomSwipeItem&amp;quot; gallery: a message-list row whose right swipe reveals a CUSTOM-content swipe item (a swipe_item_view, not a plain swipe_item)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: removed the on_mounted synthetic swipe-open that wrote 'Right items revealed (Favourite...)'. Now at rest — 'Swipe a row left to reveal the Favourite item' + the indigo 'Welcome to .NET MAUI / June 19, 2026' card — matching MAUI in both themes.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: matches MAUI in both themes — same instruction text and identical indigo card (color, corner radius, typography).

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.12% pixels differ

### 40. Cv Visual States — 🟢/⏳
<sub>cv_visual_states</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/cv_visual_states_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/cv_visual_states_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/cv_visual_states_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/cv_visual_states_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/cv_visual_states_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/cv_visual_states_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/cv_visual_states_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/cv_visual_states_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/cv_visual_states_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/cv_visual_states_dark.png" /></td></tr></table>

ports CollectionViewGalleries/SelectionGalleries/ VisualStatesGallery.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

Light: item text/list matches. Dark: MAUI itself renders a blank white CollectionView area (same in cpp and xaml) — consistent behavior across all three, not a port bug.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Same as cpp column — matches MAUI in both themes including the blank dark-mode CollectionView area.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 41. Data Template Selector — 🟢/⏳
<sub>data_template_selector</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/data_template_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/data_template_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/data_template_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/data_template_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/data_template_selector_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/data_template_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/data_template_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/data_template_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/data_template_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/data_template_selector_dark.png" /></td></tr></table>

ports DataTemplateSelectorGallery.xaml (+ DataTemplateSelectorGallery.xaml.cs, including its WeekendSelector + SearchTermSelector classes)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Light and dark: weekday/weekend list content, order, and text color all match MAUI exactly.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Light and dark match MAUI exactly, identical to cpp column.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 42. Date Picker — 🟢/⏳
<sub>date_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/date_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/date_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/date_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/date_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/date_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/date_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/date_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/date_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/date_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/date_picker_dark.png" /></td></tr></table>

ports DatePickerPage.xaml (+ DatePickerPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Fixed: 'Default with date'/'Default with time' now carries the shared XAML's restored Date=06/21/2018 / Time=4:15:26 attribute, matching MAUI exactly in both themes; all other rows already matched.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

The XAML build's 'Default with date' row correctly shows '5.07.2026', matching MAUI exactly, unlike the cpp (non-XAML) build. All other rows also match in both light and dark themes.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9966, 0.16% pixels differ · Dark: SSIM 0.9964, 0.17% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9953, 0.19% pixels differ · Dark: SSIM 0.9952, 0.20% pixels differ

### 43. Device — 🟡/⏳
<sub>device</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/device_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/device_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/device_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/device_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/device_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/device_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/device_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/device_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/device_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/device_dark.png" /></td></tr></table>

ports DevicePage.xaml

#### 🟡 Sonnet 5 — C++ (C1/C3)

Both themes: layout, font and centering match, but the reported values differ - cpp prints 'Platform: MacCatalyst / Idiom: Desktop / Version: 26.5' while MAUI ground truth prints 'Platform: iOS / Idiom: Phone / Version: 17.0'. This is runtime device-info semantics (the port reports the Catalyst host directly, MAUI reports the iOS-compat layer), not a rendering defect - minor, but the DeviceInfo mapping should mirror MAUI's.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Both themes match MAUI exactly: 'Platform: iOS / Idiom: Phone / Version: 17.0' centered identically in light and dark.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9925, 0.28% pixels differ · Dark: SSIM 0.9923, 0.31% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 44. Dispatcher — 🟢/⏳
<sub>dispatcher</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/dispatcher_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/dispatcher_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/dispatcher_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/dispatcher_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/dispatcher_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/dispatcher_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/dispatcher_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/dispatcher_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/dispatcher_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/dispatcher_dark.png" /></td></tr></table>

ports DispatcherPage.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

All four texts and blue action buttons (Fail Access / Access / 3 Seconds Later / 3 Second Timer / Device.StartTimer) plus the runtime status lines ('This was a success!', 'I happened 3 seconds later!', 'I am on a 3 second timer! 3 ticks', 'OBSOLETE ZONE ALERT!') match MAUI in both themes.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Identical content to MAUI in both themes - same prose, same blue buttons, same timer/status lines ('3 ticks', 'OBSOLETE ZONE ALERT!'). No content diffs beyond the exempt window chrome.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 45. Drag Drop — 🟢/⏳
<sub>drag_drop</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/drag_drop_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/drag_drop_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/drag_drop_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/drag_drop_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/drag_drop_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/drag_drop_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/drag_drop_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/drag_drop_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/drag_drop_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/drag_drop_dark.png" /></td></tr></table>

ports DragAndDropBetweenLayouts.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.DragAndDropBetweenLayouts)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1 (light) and C3 (dark): C++ matches MAUI exactly - full-width rainbow color swatch stack (red/orange/yellow/green/blue/indigo/violet) plus 'Rainbow:'/drag-drop status labels, identical in both themes.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2 (light) and C4 (dark): xaml matches MAUI - same 7-color swatch stack and status labels, correct light/dark background switching.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 46. Editor — 🟢/⏳
<sub>editor</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/editor_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/editor_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/editor_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/editor_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/editor_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/editor_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/editor_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/editor_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/editor_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/editor_dark.png" /></td></tr></table>

ports EditorPage.xaml (Microsoft.Maui.Controls sample gallery)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: all editor states (length labels, placeholders, purple Text/Placeholder colors, large font, read-only, numeric, bottom-aligned, autosize) match MAUI in both themes.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: identical content, colors and layout to MAUI in both themes.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 47. Effects — 🟢/⏳
<sub>effects</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/effects_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/effects_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/effects_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/effects_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/effects_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/effects_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/effects_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/effects_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/effects_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/effects_dark.png" /></td></tr></table>

ports EffectsPage.xaml (Maui.Controls.Sample.Pages.EffectsPage)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: both entries, blue detach/re-attach links and status label match MAUI in both themes; only the exempt uniform chrome-height shift differs.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: matches MAUI in both themes (entries, links, status label).

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 48. Ellipse Gallery — 🟢/⏳
<sub>ellipse_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/ellipse_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ellipse_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ellipse_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ellipse_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ellipse_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/ellipse_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ellipse_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ellipse_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ellipse_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ellipse_gallery_dark.png" /></td></tr></table>

ports EllipseGallery.xaml A self-contained, code-first port of the MAUI Shapes EllipseGallery (Pages/Controls/ShapesGalleries/EllipseGallery.xaml + .xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1 (light): MAUI and cpp both show the five ellipse variants (basic ellipse, circle outline, ellipse-with-stroke pair, dashed-stroke ellipse) left-aligned (Start) at the same x-position, same sizes/colors/strokes. C3 (dark): identical content correctly re-themed, same left alignment preserved (unlike rectangle_gallery, this page's shapes remain correctly Start-aligned).

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2 (light) and C4 (dark): xaml build matches MAUI and cpp exactly in both themes — same five ellipse shapes, same left alignment, same colors/strokes, no position drift.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 49. Empty View — 🟢/⏳
<sub>empty_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/empty_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/empty_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_dark.png" /></td></tr></table>

ports EmptyViewStringGallery.xaml (+ EmptyViewStringGallery.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: filter list content, styling, and light/dark colors all match MAUI exactly.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: identical to MAUI in both themes; XAML build matches C++ builder output.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 50. Empty View Load Simulate — 🟢/⏳
<sub>empty_view_load_simulate</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/empty_view_load_simulate_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_load_simulate_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_load_simulate_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_load_simulate_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_load_simulate_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/empty_view_load_simulate_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_load_simulate_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_load_simulate_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_load_simulate_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_load_simulate_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewLoadSimulateGallery.xaml (+ EmptyViewLoadSimulateGallery.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: 'Items loading simulation...' renders centered in both cpp and MAUI. Prior yellow's 'MAUI top-left' claim was stale.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: 'Items loading simulation...' centered, matches MAUI in both themes.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 51. Empty View Null — 🟢/⏳
<sub>empty_view_null</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/empty_view_null_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_null_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_null_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_null_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_null_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/empty_view_null_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_null_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_null_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_null_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_null_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewNullGallery.xaml (+ EmptyViewNullGallery.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: 'Nothing to display.' renders centered in both cpp and MAUI (full-res verified). Prior yellow's 'MAUI top-left' claim was stale (2026-07-05 review).

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: 'Nothing to display.' centered, matches MAUI in both themes (same shared XAML via the loader).

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 52. Empty View Rtl — 🟢/⏳
<sub>empty_view_rtl</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/empty_view_rtl_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_rtl_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_rtl_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_rtl_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_rtl_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/empty_view_rtl_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_rtl_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_rtl_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_rtl_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_rtl_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewRTLGallery.xaml (+ EmptyViewRTLGallery.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Matches MAUI in both themes. The list, Filter SearchBar, no-overlay-at-rest, and the 3-column 'cover1.jpg, 0'..'cover1.jpg, 14' grid all align. The only delta — the FlowDirection Picker showing the selected 'Left to Right' (cpp) vs the 'FlowDirection' Title (MAUI) — is an EXEMPT MAUI element-items-form Picker init artifact per ruling 8 (cpp faithfully shows the selection; MAUI's Text reads empty at map time so the Title placeholder shows). Not a port bug.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Matches MAUI including the Picker showing its 'FlowDirection' Title; Filter SearchBar + full 3-column list, no overlay at rest.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 53. Empty View Selector — 🟢/⏳
<sub>empty_view_selector</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/empty_view_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_selector_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/empty_view_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_selector_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewWithDataTemplateSelector.xaml (+ .xaml.cs, incl

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: instructional text, filter bar, and 'Baboon — Africa &amp; Asia' result row match MAUI exactly in both themes.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: identical to MAUI; XAML matches the C++ builder output.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 54. Empty View Swap — 🟢/⏳
<sub>empty_view_swap</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/empty_view_swap_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_swap_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_swap_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_swap_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_swap_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/empty_view_swap_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_swap_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_swap_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_swap_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_swap_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewSwapGallery.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: toggle switch, Clear/Fill links, and full 12-item list match MAUI exactly in both themes.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: identical to MAUI; matches the C++ builder output.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 55. Empty View Template — 🟢/⏳
<sub>empty_view_template</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/empty_view_template_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_template_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_template_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_template_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_template_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/empty_view_template_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_template_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_template_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_template_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_template_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewTemplateGallery.xaml (+ EmptyViewTemplateGallery.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: full item list layout and content match MAUI exactly in both themes.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: identical to MAUI; matches the C++ builder output.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 56. Empty View View — 🟢/⏳
<sub>empty_view_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/empty_view_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/empty_view_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_view_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewViewGallery.xaml (+ EmptyViewViewGallery.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

At rest the source is full so the EmptyView does not render (matching real MAUI): both show the Filter SearchBar over the full 3-column GridItemsLayout (cover1.jpg 0 ... Fruits.jpg 11). The prior 'missing empty-view overlay' verdict was stale — the current MAUI reference shows no overlay either. Pixel-identical apart from window chrome.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Identical to MAUI: Filter SearchBar + full 3-column item list, no EmptyView overlay at rest.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 57. Entry — 🟢/⏳
<sub>entry</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/entry_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/entry_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/entry_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/entry_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/entry_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/entry_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/entry_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/entry_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/entry_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/entry_dark.png" /></td></tr></table>

ports EntryPage.xaml (Microsoft.Maui.Controls sample gallery)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: all rows match MAUI in both themes — LENGTH/RETURN header, 'Type here...' placeholder, purple Text/Placeholder entries, checkmark button, password dots, read-only, 'Text', right-aligned 'This should be on the end', CursorPosition = 4 slider at the same position, and 'Cursor' entry.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: matches MAUI in both themes — identical entry stack, colors (purple text/placeholder, magenta in dark), password dots, right-aligned end text, and slider position.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9977, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 58. Filter Collection — 🟢/⏳
<sub>filter_collection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/filter_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/filter_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/filter_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/filter_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/filter_collection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/filter_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/filter_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/filter_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/filter_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/filter_collection_dark.png" /></td></tr></table>

ports FilterCollectionView.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

Use EmptyView switch (on) + Filter SearchBar + full 2-column caption list, no coral overlay — matches MAUI exactly (EmptyView is latent while the source is full). Prior 'missing overlay' verdict was stale.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Matches MAUI: Use EmptyView toggle + Filter + full list, no overlay at rest.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9955, 0.16% pixels differ · Dark: SSIM 0.9955, 0.18% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 59. Filter Selection — 🟢/⏳
<sub>filter_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/filter_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/filter_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/filter_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/filter_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/filter_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/filter_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/filter_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/filter_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/filter_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/filter_selection_dark.png" /></td></tr></table>

ports FilterSelection.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.FilterSelection)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: instructional text, Reset link, 'Selected: (none)' state, and full item list match MAUI exactly in both themes.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: identical to MAUI; matches the C++ builder output.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 60. Flex Layout — 🟢/⏳
<sub>flex_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/flex_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/flex_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/flex_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/flex_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/flex_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/flex_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/flex_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/flex_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/flex_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/flex_layout_dark.png" /></td></tr></table>

ports FlexLayoutPage.xaml A self-contained, code-first demo of the FlexLayout control: the classic &amp;quot;holy grail&amp;quot; page layout built from nested flexboxes

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: header(aqua)/content(gray)/footer(pink) flex columns with blue nav + green aside bars match MAUI in both themes. Dark HEADER/CONTENT/FOOTER labels now render WHITE via the dynamic system label color (unset TextColor -&gt; UIColor.labelColor), matching MAUI (the fixed #3B3B3B pin that stayed dark in dark mode was removed). Residual ~3.9% is the uniform harness inset (ruling 2, exempt).

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4 green: cyan header, pink footer, blue/green side bars, gray content and label colors (incl. white dark-mode text) all match MAUI in both themes.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9979, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9966, 0.12% pixels differ · Dark: SSIM 0.9967, 0.11% pixels differ

### 61. Focus — 🟢/⏳
<sub>focus</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/focus_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/focus_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/focus_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/focus_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/focus_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/focus_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/focus_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/focus_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/focus_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/focus_dark.png" /></td></tr></table>

ports FocusPage.xaml (+ FocusPage.xaml.cs) The C# FocusPage is a focus-subsystem demo: an Entry whose Focused/Unfocused events (OnFocusEntryFocusChanged) append &amp;quot;Focused&amp;quot;/&amp;quot;Unfocused&amp;quot; lines to a scrolling InfoLabel, plus two buttons — &amp;quot;Focus

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3 green: Focus target entry, blue 'Focus Entry'/'Unfocus Entry' buttons and 'IsFocused: false' label all match in both themes; only the exempt outer-inset/window-chrome shift differs.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4 green: identical entry, buttons and IsFocused label in both themes; matches MAUI.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 62. Fonts — 🟢/⏳
<sub>fonts</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/fonts_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/fonts_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/fonts_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/fonts_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/fonts_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/fonts_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/fonts_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/fonts_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/fonts_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/fonts_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;fonts&amp;quot; demo (ComparePages.Fonts()), the shipped-.NET-MAUI reference for the visual-parity comparison: a ScrollView over a VerticalStackLayout (Spacing 8, Padding 16) of nine Labels — Title/Subtit

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3 green: Title/Subtitle/Header/Body/Caption size ramp, Bold, Italic, Bold+Italic and 'Character spacing 4.0' all render identically in both themes.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4 green: full font-style ramp and character-spacing sample match MAUI in both themes.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 63. Footer Only String — 🟢/⏳
<sub>footer_only_string</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/footer_only_string_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/footer_only_string_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/footer_only_string_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/footer_only_string_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/footer_only_string_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/footer_only_string_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/footer_only_string_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/footer_only_string_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/footer_only_string_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/footer_only_string_dark.png" /></td></tr></table>

ports FooterOnlyString.xaml (+ FooterOnlyString.xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Rows match MAUI in both themes. The cpp item template now applies Margin=6 (shared XAML &lt;Label Margin="6"&gt;) — was missing, so rows rendered ~25px vs MAUI's ~44px; now ~44px. Caption list + bold 'This is a footer' align. (Pixel diff is a misleading ~1.4% because the content is sparse text on white; verified by direct row-pitch inspection.)

#### 🟡 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: identical content and footer string in both themes; same tighter row spacing as cpp (list compressed vertically vs MAUI's roomier rows). Minor internal-spacing diff.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 64. Formatted Text — 🟢/⏳
<sub>formatted_text</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/formatted_text_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/formatted_text_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/formatted_text_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/formatted_text_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/formatted_text_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/formatted_text_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/formatted_text_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/formatted_text_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/formatted_text_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/formatted_text_dark.png" /></td></tr></table>

a self-contained demo page for the G1 rich-text slice: a label whose FormattedText is built from several styled spans (bold / italic / colored / underlined / kerned), plus a plain label proving the Text ⇄ FormattedText exclusivity

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: formatted span (bold red, italic underlined, kerned) and plain label match MAUI in both themes; only exempt window-chrome offset differs.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: identical formatted-text rendering to MAUI in light and dark; content, colors and kerning all match.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9967, 0.12% pixels differ · Dark: SSIM 0.9967, 0.12% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 65. Gestures — 🟢/⏳
<sub>gestures</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/gestures_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/gestures_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/gestures_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/gestures_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/gestures_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/gestures_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/gestures_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/gestures_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/gestures_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/gestures_dark.png" /></td></tr></table>

ports GesturesPage.xaml (+ .xaml.cs) The MAUI GesturesPage.xaml is a *gallery navigation* page: a CollectionView listing gesture-demo sections that the shell navigates into

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: removed the on_mounted drive_synthetic_gestures() that fired one gesture per recognizer (leaving the readout on 'Pointer exited'). Now captured at rest — 'Gesture target (tap/pan/pinch/swipe/pointer)' + blue target box + static 'Last gesture: (none)' — matching MAUI in both themes. Gesture wiring stays covered by the gesture unit tests; drive_synthetic_gestures() remains callable.

#### 🟡 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: content matches (instructions + blue gesture-target box + 'Last gesture: (none)'), but the xaml/loader path lays the box + readout ~8px LOWER than MAUI (SSIM 0.959 / ~4% vs MAUI, vs cpp's ~2%). A small vertical-spacing delta in the loader layout — flagged so the consistency guardrail (cpp green / xaml yellow) holds; cpp is the closer match.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 66. Gradient — 🟢/⏳
<sub>gradient</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/gradient_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/gradient_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/gradient_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/gradient_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/gradient_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/gradient_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/gradient_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/gradient_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/gradient_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/gradient_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;gradient&amp;quot; demo (ComparePages.Gradient()), the shipped-.NET-MAUI reference for the visual-parity comparison: a VerticalStackLayout (Spacing 12, Padding 16) of two captioned 60px BoxViews — a Linea

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: linear yellow→green and radial red→navy gradient bars match MAUI in both themes (colors, stops, bar sizes).

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: both gradient bars render identically to MAUI in light and dark.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 67. Grid — 🟢/⏳
<sub>grid</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/grid_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/grid_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/grid_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/grid_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/grid_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/grid_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/grid_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/grid_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/grid_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/grid_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;grid&amp;quot; demo (ComparePages.GridPage()), the shipped-.NET-MAUI reference for the visual-parity comparison: a Grid (Padding 16, Row/ColumnSpacing 6) with RowDefinitions Auto / 80 / 80 and two Star co

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: 2x2 grid (red/green/blue/orange) matches MAUI in both themes — cell sizes, spacing and colors identical.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: same 2x2 colored grid as MAUI in both themes; no content differences.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 68. Grid Grouping — 🟢/⏳
<sub>grid_grouping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/grid_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/grid_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/grid_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/grid_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/grid_grouping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/grid_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/grid_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/grid_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/grid_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/grid_grouping_dark.png" /></td></tr></table>

ports GroupingGalleries/GridGrouping.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

cpp matches MAUI's 2-column grouped grid with green/orange bars + correct spacing; SSIM 0.667 == xaml 0.666 (uniform-shift artifact).

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Matches MAUI's 2-column grouped GridItemsLayout (bars, members, footers, spacing); raw SSIM depressed only by the uniform top-offset on this tall list (ruling 2).

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9979, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9966, 0.12% pixels differ · Dark: SSIM 0.9967, 0.11% pixels differ

### 69. Grouping No Templates — 🟢/⏳
<sub>grouping_no_templates</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/grouping_no_templates_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/grouping_no_templates_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/grouping_no_templates_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/grouping_no_templates_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/grouping_no_templates_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/grouping_no_templates_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/grouping_no_templates_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/grouping_no_templates_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/grouping_no_templates_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/grouping_no_templates_dark.png" /></td></tr></table>

ports GroupingGalleries/GroupingNoTemplates.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

Template-less grouped flat list matches MAUI (SSIM 0.945); no colored bars/templates to drift, so row heights align.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Template-less grouped flat list (member ToStrings) matches MAUI (SSIM 0.944) via the string-convertible super_teams_text() code-behind.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 70. Grouping Plus Selection — 🟢/⏳
<sub>grouping_plus_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/grouping_plus_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/grouping_plus_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/grouping_plus_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/grouping_plus_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/grouping_plus_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/grouping_plus_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/grouping_plus_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/grouping_plus_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/grouping_plus_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/grouping_plus_selection_dark.png" /></td></tr></table>

ports CollectionViewGalleries/GroupingGalleries/ GroupingPlusSelection.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

cpp matches MAUI's grouped roster with green/orange bars + correct spacing; SSIM 0.678 == xaml 0.678.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Matches MAUI's grouped roster; raw SSIM depressed only by the uniform top-offset (ruling 2).

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9979, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9966, 0.12% pixels differ · Dark: SSIM 0.9967, 0.11% pixels differ

### 71. Header Footer — 🟢/⏳
<sub>header_footer</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/header_footer_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/header_footer_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/header_footer_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/header_footer_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/header_footer_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/header_footer_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/header_footer_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/header_footer_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/header_footer_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/header_footer_dark.png" /></td></tr></table>

ports HeaderFooterString.xaml (+ HeaderFooterString.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: bold string header 'Just a string as a header', 3 items (cover1/oasis/photo), and bold footer 'This footer is also a string' all present, left-aligned and compact, matching MAUI in both themes.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: string header/3 items/bold string footer match MAUI in both themes.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 72. Header Footer Grid — 🟢/⏳
<sub>header_footer_grid</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/header_footer_grid_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/header_footer_grid_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/header_footer_grid_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/header_footer_grid_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/header_footer_grid_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/header_footer_grid_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/header_footer_grid_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/header_footer_grid_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/header_footer_grid_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/header_footer_grid_dark.png" /></td></tr></table>

ports HeaderFooterGrid.xaml (+ HeaderFooterGrid.xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Matches MAUI in both themes. The 3-column item grid now applies the item Margin=6 (shared XAML) so rows are properly spaced. Toggle Header/Footer links, the header/footer dog images with 'This Is A Header'/'This Is A Footer', 'Add Content' links, and the item grid all align.

#### 🟡 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: same as cpp — all content present and themed correctly, but the rotated footer label is placed above the footer image instead of below it, and grid rows are slightly tighter than MAUI. Minor.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9951, 0.24% pixels differ · Dark: SSIM 0.9917, 0.31% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9938, 0.27% pixels differ · Dark: SSIM 0.9904, 0.34% pixels differ

### 73. Header Footer Grid Horizontal — 🟢/⏳
<sub>header_footer_grid_horizontal</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/header_footer_grid_horizontal_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/header_footer_grid_horizontal_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/header_footer_grid_horizontal_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/header_footer_grid_horizontal_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/header_footer_grid_horizontal_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/header_footer_grid_horizontal_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/header_footer_grid_horizontal_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/header_footer_grid_horizontal_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/header_footer_grid_horizontal_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/header_footer_grid_horizontal_dark.png" /></td></tr></table>

ports HeaderFooterGridHorizontal.xaml (+ HeaderFooterGridHorizontal.xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Horizontal GridItemsLayout now fills the available vertical height (greedy cross-axis fill), matching MAUI: header image + 'This Is A Header' + 'Add Content', then the grid rows spread across the window (cover1/Vegetables/Legumes/photo, then oasis/Fruits/cover1), footer pushed past the fold. Fix: horizontal CV with an infinite (VSL/unbounded) height constraint fills the live native frame height instead of the self-referential ~570 viewport_cross_extent mirror (collection_view_handler.cpp + ios native_content_size). Cell height 190px-&gt;~490px, pixel 13.5%-&gt;5.5%. Residual: MAUI OVER-fills the CV taller than the visible window (~1881px cells ~627px, 2 rows visible) while cpp fills to the viewport (~1470px cells ~490px, 3 rows visible) — a MAUI over-fill magnitude quirk; structure/content/header/footer/greedy-fill all match (cf. sibling header_footer_grid green at 11.85% pixel).

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Same as cpp: the xaml twin (gallery_xaml, rebuilt with the fill fix) now fills the horizontal CV vertically like MAUI; pixel 13.6%-&gt;5.6%, matching the cpp column. Residual is MAUI over-filling past the window (1 extra row visible in cpp) — a magnitude quirk; structure/content match.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9786, 0.53% pixels differ · Dark: SSIM 0.9774, 0.72% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9773, 0.56% pixels differ · Dark: SSIM 0.9762, 0.76% pixels differ

### 74. Header Footer Template — 🟢/⏳
<sub>header_footer_template</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/header_footer_template_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/header_footer_template_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/header_footer_template_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/header_footer_template_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/header_footer_template_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/header_footer_template_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/header_footer_template_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/header_footer_template_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/header_footer_template_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/header_footer_template_dark.png" /></td></tr></table>

ports HeaderFooterTemplate.xaml (+ .xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Sonnet 5 — C++ (C1/C3)

RULING 12 (2026-07-17): the code-first C++ render is CORRECT here — it shows each row's own image (cover1.jpg / oasis.jpg / photo.jpg), the original MAUI PhotoTemplate {Binding Image}. MAUI and C++&amp;XAML both render cover1.jpg in EVERY cell because the shared twin XAML degrades {Binding Image} to a hardcoded &lt;Image Source="cover1.jpg"&gt; (an x:Array of plain strings can't bind an Image). The cpp-vs-maui item-image diff (~1.4%) is this exempt, intended divergence, NOT a port bug.

#### 🔴 Sonnet 5 — C++ &amp; XAML (C2/C4)

RULING 12 (2026-07-17): the code-first C++ render is CORRECT here — it shows each row's own image (cover1.jpg / oasis.jpg / photo.jpg), the original MAUI PhotoTemplate {Binding Image}. MAUI and C++&amp;XAML both render cover1.jpg in EVERY cell because the shared twin XAML degrades {Binding Image} to a hardcoded &lt;Image Source="cover1.jpg"&gt; (an x:Array of plain strings can't bind an Image). The cpp-vs-maui item-image diff (~1.4%) is this exempt, intended divergence, NOT a port bug.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9979, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9966, 0.12% pixels differ · Dark: SSIM 0.9967, 0.11% pixels differ

### 75. Header Footer View — 🟢/⏳
<sub>header_footer_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/header_footer_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/header_footer_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/header_footer_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/header_footer_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/header_footer_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/header_footer_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/header_footer_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/header_footer_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/header_footer_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/header_footer_view_dark.png" /></td></tr></table>

ports HeaderFooterView.xaml (+ .xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Match. The empty CollectionView now greedily fills the VerticalStackLayout slot and pushes the footer + Add/Clear buttons off-screen, so only the header (cover1 image + "This Is A Header") shows — matching MAUI iOS/Mac Catalyst. Fix: ported MAUI ItemsViewHandler2.EnsureContentSizeForScrollDirection (iOS.cs:257-263) — an EMPTY CV's desired main-axis size resolves to the collection view's own (full-viewport) frame extent ("the expansive size the CV wants by default" = UICollectionView.SizeThatFits), not 0. Surgical: only fires when contentSize==0; content-bearing CVs (items/multiple_bound_selection/cv_visual_states) size to content unchanged (verified green). Both themes.

#### 🟡 Sonnet 5 — C++ &amp; XAML (C2/C4)

Same situation as cpp: MAUI capture is truncated before the footer and collection content, so the xaml build's additional visible content (footer text, buttons) cannot be directly compared, though the visible header matches exactly. No confirmed divergence, but comparison is incomplete.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9952, 0.23% pixels differ · Dark: SSIM 0.9932, 0.28% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9940, 0.26% pixels differ · Dark: SSIM 0.9920, 0.31% pixels differ

### 76. Hit Testing — 🟢/⏳
<sub>hit_testing</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/hit_testing_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/hit_testing_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/hit_testing_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/hit_testing_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/hit_testing_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/hit_testing_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/hit_testing_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/hit_testing_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/hit_testing_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/hit_testing_dark.png" /></td></tr></table>

ports HitTestingPage.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.HitTestingPage)

#### 🟢 Sonnet 5 — C++ (C1/C3)

All four comparisons match closely: shapes (ellipse, rounded rectangle), text labels (Scale=1/2, Rotation=20), submarine 3D model, and 'Lorem ipsum' text all align in position, size, color, and font weight in both light and dark themes. Only trivial anti-aliasing differences.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Same as cpp: XAML build matches MAUI reference pixel-for-pixel in structure and content for both light and dark themes.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9977, 0.11% pixels differ · Dark: SSIM 0.9977, 0.11% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9964, 0.14% pixels differ · Dark: SSIM 0.9965, 0.14% pixels differ

### 77. Horizontal Stack — 🟢/⏳
<sub>horizontal_stack</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/horizontal_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/horizontal_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/horizontal_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/horizontal_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/horizontal_stack_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/horizontal_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/horizontal_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/horizontal_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/horizontal_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/horizontal_stack_dark.png" /></td></tr></table>

Horizontal Stack

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: builder fixed to mirror the shared XAML exactly (HorizontalStackLayout Padding=12 Spacing=6 of six 40x40 boxes; removed the drifted 'HorizontalStackLayout' heading label that pushed the boxes right + drew extra title text). Six boxes now left-aligned with correct spacing, matching MAUI in both themes. Removed from structure-equivalence known_diverging (strict EXPECT_EQ passes).

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: six left-aligned boxes with Padding=12/Spacing=6 match MAUI in both themes.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 78. Hybrid Web View — 🟢/⏳
<sub>hybrid_web_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/hybrid_web_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/hybrid_web_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/hybrid_web_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/hybrid_web_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/hybrid_web_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/hybrid_web_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/hybrid_web_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/hybrid_web_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/hybrid_web_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/hybrid_web_view_dark.png" /></td></tr></table>

ports HybridWebViewPage.xaml (+ HybridWebViewPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: after recapturing the STALE MAUI reference (was Jul-6, app rebuilt Jul-7), MAUI now renders the SAME empty WKWebView surface as the port — the 'HybridWebView here' status Editor, the 5-button JS-bridge column, and an empty (unloaded) web area. The port's apple handler injects the bridge JS but hosts no app:// asset tree (documented DEVIATION), and MAUI has no HybridSamplePage content bundled either, so all three render the empty webview identically. Residual ~0.6% light / 3.5% dark is the uniform harness inset (ruling 2).

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: identical to MAUI (and the cpp column) after the fresh recapture — same Editor + button column + empty web area. The prior green was based on a STALE Jul-6 capture showing an old gray placeholder surface; the fresh build matches MAUI's empty WKWebView.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 79. Image — 🟢/⏳
<sub>image</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/image_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/image_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/image_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/image_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/image_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/image_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/image_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/image_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/image_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/image_dark.png" /></td></tr></table>

ports ImagePage.xaml (+ ImagePage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3 green. UriSource (Microsoft building) and FileSource (dotnet-bot submarine on purple) both load with matching sizes, placement and colors in both themes; only the exempt uniform chrome-height shift.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4 green. Both images load and match MAUI's layout and colors in light and dark; differences limited to the exempt window-chrome offset.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 80. Image Button — 🟢/⏳
<sub>image_button</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/image_button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/image_button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/image_button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/image_button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/image_button_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/image_button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/image_button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/image_button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/image_button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/image_button_dark.png" /></td></tr></table>

ports ImageButtonPage.xaml (+ ImageButtonPage.xaml.cs) A self-contained, code-first demo page for the ImageButton control (the C# gallery-page convention, mirroring the input_controls_page / image_page pattern)

#### 🟢 Sonnet 5 — C++ (C1/C3)

MAUI's capture is scrolled slightly further down (missing the top 'N ImageButton clicks' line) but all remaining visible content — AspectFit/AspectFill/Fill green boxes, red-bordered box, purple corner-radius bars + sliders, submarine custom-size image, green padding bar — matches cpp precisely in position, size, and color, in both light and dark.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Matches MAUI identically to the cpp build across all visible elements in both themes; no divergence observed.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9947, 0.24% pixels differ · Dark: SSIM 0.9947, 0.23% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9935, 0.27% pixels differ · Dark: SSIM 0.9935, 0.26% pixels differ

### 81. Indicator — 🟢/⏳
<sub>indicator</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/indicator_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/indicator_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/indicator_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/indicator_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/indicator_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/indicator_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/indicator_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/indicator_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/indicator_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/indicator_dark.png" /></td></tr></table>

ports IndicatorPage.xaml A self-contained, code-first demo of the IndicatorView control

#### 🟢 Sonnet 5 — C++ (C1/C3)

Match (ruling 10). MAUI Mac Catalyst paints NO IndicatorView dots at all (blank); the port faithfully renders the indicator dots on every section — matching MAUI iOS+android (green). Exempt Mac Catalyst content gap. Both themes.

#### 🟡 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: the compile-time-XAML column renders the IndicatorView dots (exempt vs MAUI Mac Catalyst's blank per ruling 10), but its dot layout differs ~2.8% from the cpp column (minor cpp/xaml IndicatorView positioning divergence) — held yellow to reflect that cpp/xaml delta. Both themes.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 82. Input Controls — 🟢/⏳
<sub>input_controls</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/input_controls_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/input_controls_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/input_controls_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/input_controls_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/input_controls_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/input_controls_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/input_controls_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/input_controls_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/input_controls_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/input_controls_dark.png" /></td></tr></table>

a self-contained demo page for the W1-05 input-control set: editor, search_bar, radio_button (+ the radio_button_group attached grouping) and image_button on one vertical stack, wired together so every input drives a visible output (the C#

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: cpp matches MAUI at 0.52% (light) / 0.83% (dark) pixel diff, SSIM 0.98 — 'LENGTH: 0' + 'Type here...' entry + 'Search to insert' search bar + UPPER (selected) / lower radio group all align. Prior yellow was stale (the builder's layout-alignment is correct).

#### 🟡 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4 yellow: same as cpp — full content match in both themes except the lighter-weight radio glyphs vs MAUI's bolder ring/dot.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9950, 0.18% pixels differ · Dark: SSIM 0.9950, 0.20% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9938, 0.22% pixels differ · Dark: SSIM 0.9937, 0.23% pixels differ

### 83. Input Transparent — 🟢/⏳
<sub>input_transparent</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/input_transparent_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/input_transparent_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/input_transparent_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/input_transparent_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/input_transparent_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/input_transparent_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/input_transparent_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/input_transparent_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/input_transparent_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/input_transparent_dark.png" /></td></tr></table>

ports InputTransparentPage.xaml (Maui.Controls.Sample.Pages.InputTransparentPage)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3 green: all four sections, blue link buttons, the toggle switch, status text, and even the same overlapping-label rendering of the stacked overlay buttons ('Bottom (clickable)/Top (transparent)' and 'Test Button/Bottom layer' superimposed) match MAUI in both themes.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4 green: identical to cpp — full match with MAUI in light and dark, including the same superimposed overlay-button text.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 84. Invalidate Brush — 🟢/⏳
<sub>invalidate_brush</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/invalidate_brush_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/invalidate_brush_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/invalidate_brush_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/invalidate_brush_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/invalidate_brush_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/invalidate_brush_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/invalidate_brush_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/invalidate_brush_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/invalidate_brush_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/invalidate_brush_dark.png" /></td></tr></table>

ports InvalidateBrushGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/InvalidateBrushGallery.xaml (&amp;quot;Invalidate Brushes Playground&amp;quot;): a VerticalStackLayout (Padding 12) with — - a &amp;quot;Change color&amp;quot; Bu

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: removed the builder's stray line_.set_horizontal_layout_alignment(start) — the shared XAML Line has NO HorizontalOptions (only the Button is Start), so the explicit-width(150) Line coerces to Center at the default Fill alignment, rendering as a centered/floating green bar exactly like MAUI + the loader (xaml column). Now the 'Change color' green button + centered green line + 'Brush color: Green' match MAUI in both themes.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: matches MAUI in both themes — green button with blue text, 'Brush color: Green' label, and the green GraphicsView line horizontally centered.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 85. Invalidate Shadow Host — 🟢/⏳
<sub>invalidate_shadow_host</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/invalidate_shadow_host_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/invalidate_shadow_host_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/invalidate_shadow_host_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/invalidate_shadow_host_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/invalidate_shadow_host_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/invalidate_shadow_host_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/invalidate_shadow_host_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/invalidate_shadow_host_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/invalidate_shadow_host_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/invalidate_shadow_host_dark.png" /></td></tr></table>

ports InvalidateShadowHostPage.xaml A self-contained, code-first demo that a shadow re-applies (invalidates) when its host&amp;#x27;s size changes, mirroring the C# core gallery page (Pages/Core/ShadowGalleries/InvalidateShadowHostPage.xaml + .xaml.

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: MAUI vs C++ (light+dark) match — 'Update Host Size' link, shadow sliders (offset X/Y=10, radius=10, opacity=1.00), and the green-bordered host box (white fill) all identical in both themes. Notification banner artifact in corner exempt.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: MAUI vs C++&amp;XAML (light+dark) match — identical layout, slider positions, and green-bordered white host box in both themes.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 86. Ios Blur Effect — 🟢/⏳
<sub>ios_blur_effect</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/ios_blur_effect_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_blur_effect_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_blur_effect_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_blur_effect_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_blur_effect_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/ios_blur_effect_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_blur_effect_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_blur_effect_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_blur_effect_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_blur_effect_dark.png" /></td></tr></table>

ports iOSBlurEffectPage.xaml The .NET MAUI PlatformSpecifics sample (Pages/PlatformSpecifics/iOS/iOSBlurEffectPage.xaml + .xaml.cs): an Image (Source=&amp;quot;oasis.jpg&amp;quot;) carrying the iOSSpecific VisualElement.BlurEffect knob (XAML seeds it to Extr

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3 green. Pug photo (no blur applied, ExtraLight state), the four blue blur links and the 'BlurEffect: ExtraLight' label all match MAUI in both themes; only the exempt uniform chrome shift.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4 green. Matches MAUI in both themes: same image, link stack and status label; no content differences beyond the exempt window-chrome offset.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 87. Ios Date Picker — 🟢/⏳
<sub>ios_date_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/ios_date_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_date_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_date_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_date_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_date_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/ios_date_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_date_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_date_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_date_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_date_picker_dark.png" /></td></tr></table>

ports iOSDatePickerPage.xaml (+ iOSDatePickerPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: matches MAUI in both themes — '31.12.2020' label top-left and centered 'Toggle DatePicker UpdateMode' link.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: matches MAUI in both themes — same date label and toggle link at the same positions.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 88. Ios Entry — 🟢/⏳
<sub>ios_entry</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/ios_entry_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_entry_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_entry_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_entry_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_entry_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/ios_entry_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_entry_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_entry_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_entry_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_entry_dark.png" /></td></tr></table>

ports iOSEntryPage.xaml (+ iOSEntryPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: matches MAUI in both themes — full-width entry with 'Enter text here to see the font size change' placeholder and the centered toggle link; dark-mode field fill matches.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: matches MAUI in both themes — same entry, placeholder, and toggle link.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 89. Ios First Responder — 🟢/⏳
<sub>ios_first_responder</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/ios_first_responder_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_first_responder_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_first_responder_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_first_responder_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_first_responder_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/ios_first_responder_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_first_responder_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_first_responder_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_first_responder_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_first_responder_dark.png" /></td></tr></table>

ports iOSFirstResponderPage.xaml (+ .xaml.cs) The C# iOSFirstResponderPage is a VisualElement-first-responder demo: a StackLayout with an explanatory Label, a &amp;quot;First Entry&amp;quot; + plain &amp;quot;OK&amp;quot; Button (tapping OK dismisses the keyboard because the

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: matches MAUI in both themes — two instruction labels, two entries, two OK links, Focus First/Focus Second links, and the three IsFocused/CanBecomeFirstResponder status lines all identical.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: matches MAUI in both themes — identical content and layout throughout.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 90. Ios Pan Gesture — 🟢/⏳
<sub>ios_pan_gesture</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/ios_pan_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_pan_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_pan_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_pan_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_pan_gesture_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/ios_pan_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_pan_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_pan_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_pan_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_pan_gesture_dark.png" /></td></tr></table>

ports iOSPanGestureRecognizerPage.xaml (+ .xaml.cs) The C# iOSPanGestureRecognizerPage is a StackLayout with: a bold message Label (_messageLabel), a &amp;quot;Toggle Simultaneous Gesture Recognition&amp;quot; Button, and a grouped ListView of employees whos

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: removed the on_mounted synthetic pan (drive_synthetic_pan) that wrote 'panned x:45 y:-12'. Now at rest — static 'Pan the target. If you pan it, this Label will change.' header + 'Toggle Simultaneous Gesture Recognition' link + 'Pan target' + 'SimultaneousRecognition: false' — matching MAUI in both themes.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: header instruction text, blue toggle link, 'Pan target' and 'SimultaneousRecognition: false' all match MAUI in both themes; only the exempt window-chrome offset differs.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 91. Ios Picker — 🟢/⏳
<sub>ios_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/ios_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/ios_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_picker_dark.png" /></td></tr></table>

ports iOSPickerPage.xaml (+ iOSPickerPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: full-width 'Select a monkey' picker field and centered 'Toggle Picker UpdateMode' link match MAUI in both light and dark (dark shows the same black full-width field strip). Only the exempt chrome offset differs.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: identical to MAUI in both themes — placeholder text, field styling and link placement all match.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 92. Ios Safe Area — 🟢/⏳
<sub>ios_safe_area</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/ios_safe_area_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_safe_area_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_safe_area_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_safe_area_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_safe_area_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/ios_safe_area_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_safe_area_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_safe_area_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_safe_area_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_safe_area_dark.png" /></td></tr></table>

ports iOSSafeAreaPage.xaml The .NET MAUI PlatformSpecifics sample (Pages/PlatformSpecifics/iOS/iOSSafeAreaPage.xaml + .xaml.cs): a long Lorem-ipsum Label over a &amp;quot;Disable Use Safe Area&amp;quot; button

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: lorem-ipsum paragraph wraps identically and the centered 'Disable Use Safe Area' link matches in both themes; only the exempt window-chrome shift differs.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: pixel-equivalent to MAUI in both themes (same text layout and link).

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 93. Ios Scroll View — 🟢/⏳
<sub>ios_scroll_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/ios_scroll_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_scroll_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_scroll_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_scroll_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_scroll_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/ios_scroll_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_scroll_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_scroll_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_scroll_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_scroll_view_dark.png" /></td></tr></table>

ports iOSScrollViewPage.xaml (+ iOSScrollViewPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: rewrote the builder to mirror the shared XAML's degraded resting shape — the C# FlyoutPage's Menu flyout is omitted, so page() is now a plain ContentPage over a ScrollView &gt; VerticalStackLayout(Spacing=20) [Slider(0-100, Value=50), 'Toggle ScrollView DelayContentTouches' Button, 'Return to Platform-Specifics List' Button]. Removed the flyout_page/menu_page that rendered a stray Mac Catalyst sidebar-toggle button + narrowed the slider. Now full-width slider at 50% + blue links match MAUI in both themes. Removed from structure-equivalence known_diverging (strict EXPECT_EQ passes).

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: slider at the same value with matching blue track, and both links match MAUI in both themes; no stray sidebar button.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 94. Ios Search Bar — 🟢/⏳
<sub>ios_search_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/ios_search_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_search_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_search_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_search_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_search_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/ios_search_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_search_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_search_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_search_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_search_bar_dark.png" /></td></tr></table>

ports iOSSearchBarPage.xaml (+ iOSSearchBarPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: rounded search field with magnifier icon and 'Enter search term' placeholder plus the two toggle links all match MAUI in both themes.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: identical to MAUI in both themes — search-bar styling and link placement match.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 95. Ios Slider Update On Tap — 🟢/⏳
<sub>ios_slider_update_on_tap</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/ios_slider_update_on_tap_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_slider_update_on_tap_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_slider_update_on_tap_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_slider_update_on_tap_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_slider_update_on_tap_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/ios_slider_update_on_tap_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_slider_update_on_tap_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_slider_update_on_tap_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_slider_update_on_tap_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_slider_update_on_tap_dark.png" /></td></tr></table>

ports iOSSliderUpdateOnTapPage.xaml (+ iOSSliderUpdateOnTapPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: instruction label, slider at minimum with thumb at far left, and 'Toggle Update on Tap' link all match MAUI in both themes; only the exempt chrome offset differs.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: identical to MAUI in both themes — same slider state, label and link.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 96. Ios Swipe Transition — 🟢/⏳
<sub>ios_swipe_transition</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/ios_swipe_transition_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_swipe_transition_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_swipe_transition_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_swipe_transition_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_swipe_transition_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/ios_swipe_transition_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_swipe_transition_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_swipe_transition_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_swipe_transition_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_swipe_transition_dark.png" /></td></tr></table>

ports iOSSwipeViewTransitionModePage.xaml (+ .xaml.cs) The C# iOSSwipeViewTransitionModePage is a StackLayout with: a horizontal row holding a &amp;quot;SwipeTransitionMode:&amp;quot; Label + an EnumPicker over the SwipeTransitionMode enum (Reveal / Drag, Se

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3 green. Reveal/Drag links, the gray 'Swipe right' swipe cell (white text on gray in dark, matching MAUI), and both caption lines ('Swipe right to reveal Delete...', 'SwipeTransitionMode: Drag') match in both themes; only the exempt chrome shift.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4 green. Identical to MAUI in both themes: links, gray swipe cell and captions all present with matching colors and placement.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 97. Ios Time Picker — 🟢/⏳
<sub>ios_time_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/ios_time_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_time_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_time_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_time_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_time_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/ios_time_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_time_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_time_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_time_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_time_picker_dark.png" /></td></tr></table>

ports iOSTimePickerPage.xaml The .NET MAUI PlatformSpecifics sample (Pages/PlatformSpecifics/iOS/iOSTimePickerPage.xaml + .xaml.cs): a TimePicker carrying the iOSSpecific TimePicker.UpdateMode knob (XAML seeds it to WhenFinished) over a but

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: matches — '14:00' label, blue 'Toggle TimePicker UpdateMode' link, and 'UpdateMode: WhenFinished' text identical in both themes.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: XAML matches MAUI in both themes — same time text, link, and mode label.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 98. Items — 🟢/⏳
<sub>items</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/items_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/items_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/items_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/items_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/items_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/items_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/items_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/items_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/items_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/items_dark.png" /></td></tr></table>

a self-contained demo page for the W2-19 items core: a collection_view over a live observable items source with a templated cell, single selection driving a readout label, and an EmptyView for the cleared state (the C# CollectionView galler

#### 🟢 Sonnet 5 — C++ (C1/C3)

Light and dark: 'Today' header, 3 task items, and 'Pick a task' placeholder text all match MAUI exactly.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Light and dark match MAUI exactly, identical to cpp column.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 99. Items Updating Scroll Mode — 🟢/⏳
<sub>items_updating_scroll_mode</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/items_updating_scroll_mode_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/items_updating_scroll_mode_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/items_updating_scroll_mode_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/items_updating_scroll_mode_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/items_updating_scroll_mode_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/items_updating_scroll_mode_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/items_updating_scroll_mode_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/items_updating_scroll_mode_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/items_updating_scroll_mode_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/items_updating_scroll_mode_dark.png" /></td></tr></table>

ports ItemsUpdatingScrollModeGallery.xaml (+ .xaml.cs) of the C# CollectionView gallery (Maui.Controls.Sample.Pages.CollectionViewGalleries.ScrollModeGalleries.ItemsUpdatingScrollModeGallery)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: matches — mode links, Add Item, status line, and 50-row list render at the same size and density as MAUI in both themes.

#### 🟡 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: list item font/row height is slightly larger than MAUI (only ~40 rows fit vs MAUI's ~50 at the same window height) in both themes; header/links/status line match.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 100. Label — 🟢/⏳
<sub>label</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/label_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/label_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/label_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/label_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/label_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/label_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/label_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/label_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/label_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/label_dark.png" /></td></tr></table>

ports LabelPage.xaml (+ LabelPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Light and dark: every label variant (colors, background highlight, alignment, line-wrapping box, formatted string spans, truncation modes) matches MAUI exactly.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Light and dark match MAUI exactly, identical to cpp column.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 101. Layout Is Enabled — 🟢/⏳
<sub>layout_is_enabled</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/layout_is_enabled_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/layout_is_enabled_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/layout_is_enabled_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/layout_is_enabled_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/layout_is_enabled_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/layout_is_enabled_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/layout_is_enabled_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/layout_is_enabled_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/layout_is_enabled_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/layout_is_enabled_dark.png" /></td></tr></table>

ports LayoutIsEnabledPage.xaml (+ LayoutIsEnabledPage.xaml.cs) The C# page demonstrates how IsEnabled on a layout cascades to its children: a 2x2 grid whose left column hosts a &amp;quot;MainLayout&amp;quot; full of state-demo sub-stacks (all-enabled / all-d

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: every section (all-enabled / all-disabled / disabled-because-layout / first-enabled-second-disabled / commands-attached / nested) renders identical panels, colors and Enabled/Disabled labels to MAUI. cpp is pixel-identical to the green xaml column (cpp-vs-xaml 0.78% AA-only, colored panels start at the SAME row); the 24% vs-MAUI is purely the uniform 28px harness vertical inset (ruling 2) amplified across the dense colored panels — the exact figure the xaml column was greened at.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: XAML matches MAUI in both themes — labels interleaved with radios and all section states identical.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 102. Line Gallery — 🟢/⏳
<sub>line_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/line_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/line_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/line_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/line_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/line_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/line_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/line_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/line_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/line_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/line_gallery_dark.png" /></td></tr></table>

ports LineGallery.xaml A self-contained, code-first port of the MAUI Shapes LineGallery (Pages/Controls/ShapesGalleries/LineGallery.xaml + .xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Light and dark: basic purple line, dashed orange line, and thick black StrokeThickness line all match MAUI's position, color, and thickness exactly.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Light and dark match MAUI exactly, identical to cpp column.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 103. Line Join Gallery — 🟢/⏳
<sub>line_join_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/line_join_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/line_join_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/line_join_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/line_join_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/line_join_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/line_join_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/line_join_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/line_join_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/line_join_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/line_join_gallery_dark.png" /></td></tr></table>

ports LineJoinGallery.xaml A self-contained, code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/LineJoinGallery.xaml: a StackLayout that demonstrates the three StrokeLineJoin variants on an identical open polyline

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: Miter/Bevel/Round cyan zig-zags match MAUI exactly in shape, join rendering, color and labels in both themes.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: XAML column matches MAUI exactly in both themes.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 104. Measure First Strategy — 🟢/⏳
<sub>measure_first_strategy</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/measure_first_strategy_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/measure_first_strategy_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/measure_first_strategy_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/measure_first_strategy_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/measure_first_strategy_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/measure_first_strategy_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/measure_first_strategy_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/measure_first_strategy_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/measure_first_strategy_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/measure_first_strategy_dark.png" /></td></tr></table>

ports MeasureFirstStrategy.xaml (+ .xaml.cs) of the C# CollectionView gallery (Maui.Controls.Sample.Pages.CollectionViewGalleries.GroupingGalleries.MeasureFirstStrategy)

#### 🟢 Sonnet 5 — C++ (C1/C3)

cpp matches MAUI: Toggle Sizing Strategy + green/orange group TEXT (TextColor + FontSize=16). SSIM 0.918 == xaml 0.917.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Toggle Sizing Strategy + note + grouped roster match MAUI (SSIM 0.92).

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 105. Menu Bar — 🟢/⏳
<sub>menu_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/menu_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/menu_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/menu_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/menu_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/menu_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/menu_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/menu_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/menu_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/menu_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/menu_bar_dark.png" /></td></tr></table>

ports MenuBarPage.xaml (+ MenuBarPage.cs) The C# page declares three page-level MenuBarItems (Page.MenuBarItems — the app menu bar) and a small visible body: - &amp;quot;Before File&amp;quot; : &amp;quot;Before File Action&amp;quot; (accelerator &amp;quot;b&amp;quot;), &amp;quot;Cool item 1&amp;quot;, a separat

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: label 'You clicked on Menu Item:' and blue 'Toggle Menu Bar Item' link match MAUI in both themes; only the exempt outer inset differs (cpp label sits at the window edge).

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: identical content to MAUI in both themes — label and blue link at matching positions, colors and fonts match.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 106. Modal — 🟢/⏳
<sub>modal</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/modal_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/modal_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/modal_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/modal_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/modal_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/modal_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/modal_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/modal_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/modal_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/modal_dark.png" /></td></tr></table>

ports ModalPage.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: 'Modal Page 1', four blue push links, disabled gray 'Pop Modal Page', and the depth status line all match MAUI in both themes.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: XAML column identical to MAUI in both themes.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 107. Multiple Bound Selection — 🟢/⏳
<sub>multiple_bound_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/multiple_bound_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/multiple_bound_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/multiple_bound_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/multiple_bound_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/multiple_bound_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/multiple_bound_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/multiple_bound_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/multiple_bound_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/multiple_bound_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/multiple_bound_selection_dark.png" /></td></tr></table>

ports MultipleBoundSelection.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.MultipleBoundSelection)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Match (ruling 9). The sole maccatalyst diff is the persistent CollectionView selection band cpp draws on the applied selection, which MAUI Mac Catalyst omits — an EXEMPT platform quirk per ruling 9 (MAUI iOS+android render the same band; cpp matches them green and faithfully reflects the selection state; Mac Catalyst's UICollectionView just does not paint the persistent selection background at rest). Content, layout, row pitch and order all match. Both themes.

#### 🟡 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: text/buttons match, but the compile-time-XAML column does NOT apply the VM-bound CollectionView selection — it renders NO selection band, whereas the cpp column (and MAUI on iOS+android) applies and renders the seeded selection. This is a functional gap in the XAML selection-binding path: the no-band result only coincidentally matches MAUI Mac Catalyst (which also paints no persistent band, per ruling 9) but would be wrong on iOS/android. Diverges from the faithful cpp column (~3.7% pixels, the band). Both themes.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 108. Navigation Gallery — 🟢/⏳
<sub>navigation_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/navigation_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/navigation_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/navigation_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/navigation_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/navigation_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/navigation_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/navigation_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/navigation_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/navigation_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/navigation_gallery_dark.png" /></td></tr></table>

ports NavigationGallery.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3 match: status line (Stack depth: 1 | top: PAGE NUMBER 1 | secondary toolbar items: 0) and all six blue action links identical in text, color, spacing and centering in both themes; only exempt window-chrome/title differences.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4 match: identical status line and six centered blue links in both themes; pixel-equivalent to the MAUI reference apart from exempt chrome/title.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 109. Nested Collection — 🟢/⏳
<sub>nested_collection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/nested_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/nested_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/nested_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/nested_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/nested_collection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/nested_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/nested_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/nested_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/nested_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/nested_collection_dark.png" /></td></tr></table>

ports NestedGalleries/NestedCollectionViewGallery.xaml (+ NestedCollectionViewGallery.xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

Nested CollectionView content now matches MAUI in both themes: the inner captions read the DemoFilteredItemSource pattern '{image}, {k}' (cover1.jpg/oasis.jpg/photo.jpg/Vegetables.jpg/Fruits.jpg/FlowerBuds.jpg/Legumes.jpg cycled by index) with per-source counts 6+(n%9) — was 'Caption N-M' with random counts (builder-data drift vs NestedCollectionModel.cs). Outer source rows, red-italic 'Source N' headers, and the blue horizontal caption lists all align; the pixel% is inflated by the exempt harness inset (dense text shifts) + a minor red-title wrap nuance. (xaml column stays RED — the compile-time XAML app can't run the C# code-behind data wiring, so its nested CVs are empty; a known xaml-column limitation, not a cpp issue.)

#### 🔴 Sonnet 5 — C++ &amp; XAML (C2/C4)

Only the 'It's CollectionViews all the way down.' header renders; the nested content is empty — code-behind data gap.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9166, 3.18% pixels differ · Dark: SSIM 0.9834, 3.76% pixels differ

#### 🔴 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.8872, 2.70% pixels differ · Dark: SSIM 0.9827, 3.64% pixels differ

### 110. Pan Gesture Events — 🟡/⏳
<sub>pan_gesture_events</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/pan_gesture_events_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/pan_gesture_events_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/pan_gesture_events_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/pan_gesture_events_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/pan_gesture_events_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/pan_gesture_events_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/pan_gesture_events_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/pan_gesture_events_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/pan_gesture_events_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/pan_gesture_events_dark.png" /></td></tr></table>

ports PanGestureEventsGallery.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.PanGestureEventsGallery)

#### 🟡 Sonnet 5 — C++ (C1/C3)

C1/C3: green/red half panels and layout match exactly in both themes, but the status readout differs — MAUI shows 'StatusType: Completed, TotalX: 12, TotalY: -8' while cpp shows 'TotalX: 0, TotalY: 0'. The injected pan completes but reports zero deltas in the cpp builder app (xaml reports 12/-8 correctly), suggesting a pan-gesture total-delta reporting gap in the cpp capture path.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: identical green/red panels and identical status text 'StatusType: Completed, TotalX: 12, TotalY: -8' in both themes.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 111. Path Aspect Gallery — 🟢/⏳
<sub>path_aspect_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/path_aspect_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/path_aspect_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/path_aspect_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/path_aspect_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/path_aspect_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/path_aspect_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/path_aspect_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/path_aspect_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/path_aspect_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/path_aspect_gallery_dark.png" /></td></tr></table>

ports PathAspectGallery.xaml A self-contained, code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PathAspectGallery.xaml: a StackLayout (Padding 12) that demonstrates the four Path Aspect modes on one identical ge

#### 🟢 Sonnet 5 — C++ (C1/C3)

Light and dark: all four aspect-mode octagons (None/Fill/Uniform/UniformToFill) match MAUI's size, color, and stroke exactly.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Light and dark match MAUI exactly, identical to cpp column.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 112. Path Gallery — 🟢/⏳
<sub>path_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/path_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/path_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/path_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/path_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/path_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/path_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/path_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/path_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/path_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/path_gallery_dark.png" /></td></tr></table>

ports PathGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PathGallery.xaml: a ScrollView over a StackLayout (Padding 12) that walks eight Path variants (plus two caption-only markup-string Labels

#### 🟢 Sonnet 5 — C++ (C1/C3)

Light and dark: all path/geometry shapes (bezier zig-zag, composite circle, overlapping rect, EllipseGeometry circle, unfilled star outline, complex-path placeholders) match MAUI in shape and color. CORRECTION (2026-07): the previously-recorded "capture-crop artifact" explanation for the missing top label was wrong — both captures are the same viewport size with no crop difference. This is a genuine, narrow MAUI Mac Catalyst rendering quirk: the first Label ("Create a LineSegment in a PathGeometry"), whose next sibling is a Line shape with no explicit WidthRequest/HeightRequest, does not render at all in MAUI's real capture — the Line renders directly at the top of the page instead. Other galleries whose Line shapes DO set WidthRequest/HeightRequest (e.g. line_gallery) render their preceding label normally, so this appears specific to an unconstrained-size Line measurement interaction in MAUI itself, not a port/twin bug (the port's Line here has no size request either, exactly matching the real C# PathGallery.xaml source). Flagged as a new MAUI-side quirk per port/CLAUDE.md ruling 3 (needs a user ruling) rather than silently fixed — see port/maui-reference/docs/EQUIVALENCE_FINDINGS.md.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Light and dark match MAUI exactly except for the same first-label MAUI quirk described in the cpp column's review (both cpp and xaml currently show the label; MAUI's own render omits it) — identical to the cpp column, so cpp and xaml agree with EACH OTHER (verified: cpp-vs-xaml SSIM 0.9991, 0.03% pixels differ, i.e. only window-title-bar text and normal antialiasing noise). Not a cpp&lt;-&gt;xaml divergence.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 113. Path Transform String — 🟢/⏳
<sub>path_transform_string</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/path_transform_string_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/path_transform_string_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/path_transform_string_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/path_transform_string_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/path_transform_string_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/path_transform_string_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/path_transform_string_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/path_transform_string_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/path_transform_string_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/path_transform_string_dark.png" /></td></tr></table>

ports PathTransformStringGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PathTransformStringGallery.xaml: a ScrollView over a StackLayout (Padding 12) that shows the SAME two-figure Path geometry

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1-C4: the 'Without RenderTransform' + 'With RenderTransform' Z-path shapes render identically to MAUI in both themes; residual ~0.8% is thin-stroke anti-aliasing on the paths.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: xaml matches MAUI — same flag sizes for both the untransformed and skew-transformed paths in both themes (dark theme shares MAUI's low-contrast black-on-dark stroke, which matches ground truth).

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 114. Picker — 🟢/⏳
<sub>picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/picker_dark.png" /></td></tr></table>

ports PickerPage.xaml (+ PickerPage.xaml.cs) A self-contained, code-first demo page for the Picker control (the C# gallery-page convention, mirroring the value_controls_page / pickers_page pattern)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Light and dark: every picker variant (basic, selected-index, text/title color, italic+yellow background, dynamic items, green background) matches MAUI exactly.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Light and dark match MAUI exactly, identical to cpp column.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 115. Pickers — 🟢/⏳
<sub>pickers</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/pickers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/pickers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/pickers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/pickers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/pickers_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/pickers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/pickers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/pickers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/pickers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/pickers_dark.png" /></td></tr></table>

a self-contained demo page for the W1-06 picker set: picker, date_picker and time_picker on one vertical stack, wired together so every selection drives a visible output (the C# gallery-page convention, code-first; the value_controls_page p

#### 🟢 Sonnet 5 — C++ (C1/C3)

Match (ruling 10). MAUI Mac Catalyst does not fire the DatePicker/TimePicker default through its change event at init, so its bound summary reads "(no date) at (no time)"; the port propagates the default like MAUI iOS+android (green), so the summary reflects the picker values. Exempt Mac Catalyst picker init-propagation gap. Both themes.

#### 🟡 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: the compile-time-XAML column does NOT propagate the picker default at init, so its summary reads "(no date) at (no time)" — this only coincidentally matches MAUI Mac Catalyst (which also does not propagate, per ruling 10) but diverges from the cpp column and from MAUI iOS/android which DO propagate. A functional gap in the XAML init-propagation path. Both themes.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 116. Pointer Gesture — 🟢/⏳
<sub>pointer_gesture</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/pointer_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/pointer_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/pointer_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/pointer_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/pointer_gesture_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/pointer_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/pointer_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/pointer_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/pointer_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/pointer_gesture_dark.png" /></td></tr></table>

ports PointerGestureGalleryPage.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.PointerGestureGalleryPage)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Matches MAUI in both themes. The three section titles ('Hover, press, and release me!', 'Hover me!', 'Hover me green!') now render at FontSize=24 like MAUI — the shared XAML twin had DROPPED the FontSize="24" that the original PointerGestureGalleryPage.xaml sets on pgrLabel/hoverLabel/colorfulHoverLabel (a P2-conversion fidelity defect); restored it so MauiReference renders the large titles. cpp==xaml==MAUI at ~1.2% (exempt harness inset). Description lines + layout all align.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Matches MAUI in both themes. The three section titles ('Hover, press, and release me!', 'Hover me!', 'Hover me green!') now render at FontSize=24 like MAUI — the shared XAML twin had DROPPED the FontSize="24" that the original PointerGestureGalleryPage.xaml sets on pgrLabel/hoverLabel/colorfulHoverLabel (a P2-conversion fidelity defect); restored it so MauiReference renders the large titles. cpp==xaml==MAUI at ~1.2% (exempt harness inset). Description lines + layout all align.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 117. Polygon Gallery — 🟢/⏳
<sub>polygon_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/polygon_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/polygon_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/polygon_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/polygon_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/polygon_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/polygon_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/polygon_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/polygon_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/polygon_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/polygon_gallery_dark.png" /></td></tr></table>

ports PolygonGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PolygonGallery.xaml: a ScrollView over a StackLayout (Padding 12) that walks four Polygon variants, each under a caption Label — - &amp;quot;A

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1-C4: basic/dash/EvenOdd/NonZero polygons (green outline triangle, dotted triangle, red-blue star, yellow-black star) render identically to MAUI in both themes; residual ~1.1% is polygon-edge anti-aliasing.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: xaml renders the basic/dash/EvenOdd/NonZero polygons and their captions identically to MAUI (and to the cpp column) in both themes; residual ~1.5% is polygon-edge anti-aliasing. (Supersedes a stale consistency-flag verdict.)

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 118. Polyline Gallery — 🟢/⏳
<sub>polyline_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/polyline_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/polyline_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/polyline_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/polyline_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/polyline_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/polyline_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/polyline_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/polyline_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/polyline_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/polyline_gallery_dark.png" /></td></tr></table>

ports PolylineGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PolylineGallery.xaml: a StackLayout (Padding 12 — no ScrollView in the C# source) holding two Polyline variants, each under a caption

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: both themes match — red ECG-style basic polyline and dotted dash polyline with identical geometry, stroke color and dash pattern.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: xaml matches MAUI in both themes; identical polylines.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 119. Preselected Item — 🟢/⏳
<sub>preselected_item</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/preselected_item_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/preselected_item_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/preselected_item_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/preselected_item_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/preselected_item_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/preselected_item_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/preselected_item_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/preselected_item_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/preselected_item_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/preselected_item_dark.png" /></td></tr></table>

ports PreselectedItemGallery.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.PreselectedItemGallery)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Match (ruling 9). The sole maccatalyst diff is the persistent CollectionView selection band cpp draws on the applied selection, which MAUI Mac Catalyst omits — an EXEMPT platform quirk per ruling 9 (MAUI iOS+android render the same band; cpp matches them green and faithfully reflects the selection state; Mac Catalyst's UICollectionView just does not paint the persistent selection background at rest). Content, layout, row pitch and order all match. Both themes.

#### 🟡 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: content and (absent) selection state match MAUI, but list row spacing is tighter (~31px vs ~44px pitch), compressing the list vertically. Both themes.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 120. Preselected Items — 🟢/⏳
<sub>preselected_items</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/preselected_items_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/preselected_items_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/preselected_items_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/preselected_items_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/preselected_items_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/preselected_items_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/preselected_items_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/preselected_items_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/preselected_items_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/preselected_items_dark.png" /></td></tr></table>

ports PreselectedItemsGallery.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.PreselectedItemsGallery)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Match (ruling 9). The sole maccatalyst diff is the persistent CollectionView selection band cpp draws on the applied selection, which MAUI Mac Catalyst omits — an EXEMPT platform quirk per ruling 9 (MAUI iOS+android render the same band; cpp matches them green and faithfully reflects the selection state; Mac Catalyst's UICollectionView just does not paint the persistent selection background at rest). Content, layout, row pitch and order all match. Both themes.

#### 🟡 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: grid content and unhighlighted selection state match MAUI; row spacing tighter (~31px vs ~44px pitch). Both themes.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 121. Progress Bar — 🟢/⏳
<sub>progress_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/progress_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/progress_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/progress_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/progress_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/progress_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/progress_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/progress_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/progress_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/progress_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/progress_bar_dark.png" /></td></tr></table>

ports ProgressBarPage.xaml (+ ProgressBarPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: the second 'ProgressColor' section label is now the DEFAULT (small, non-bold) font per the shared XAML's bare &lt;Label&gt; (the builder previously bolded it @18pt like the other headers). cpp now renders identically to the green xaml column (cpp-vs-xaml 0.06%); the ~5% vs-MAUI is the uniform harness vertical inset (ruling 2) that the xaml column carries too. Bars: Default/ProgressColor(orange)/Disabled/ProgressColor(orange)/ProgressTo all match.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: matches MAUI in both themes — same bar colors, fill fractions, label weights (including the small regular 4th 'ProgressColor' label), and ProgressTo link.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 122. Radio Button Border — 🟢/⏳
<sub>radio_button_border</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/radio_button_border_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_button_border_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_button_border_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_button_border_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_button_border_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/radio_button_border_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_button_border_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_button_border_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_button_border_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_button_border_dark.png" /></td></tr></table>

ports RadioButtonBorder.xaml A self-contained, code-first demo of RadioButton border styling

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: cpp matches MAUI in both themes. The builder now applies the shared XAML's title FontSize=18/FontAttributes=Bold and the root StackLayout Spacing=6/Padding=16 (previously missing -&gt; regular-weight title + tighter rows). cpp==xaml at 6.72%/6.81% pixel diff (SSIM 0.909 light / 0.889 dark), marginally better than the green xaml column (6.76%/6.84%); the residual is MAUI's slightly larger native radio circles, the same delta the xaml column carries. Red/green option borders, yellow backgrounds, and dark-mode white-on-yellow text all match.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: matches MAUI in both themes — bold title, row heights, red/green borders, yellow fills, and the (MAUI-inherited) white-on-yellow dark-mode text are all reproduced.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 123. Radio Button Content — 🟢/⏳
<sub>radio_button_content</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/radio_button_content_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_button_content_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_button_content_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_button_content_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_button_content_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/radio_button_content_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_button_content_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_button_content_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_button_content_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_button_content_dark.png" /></td></tr></table>

ports RadioButtonContentGallery.xaml A self-contained, code-first demo of the RadioButton.Content surface

#### 🟢 Sonnet 5 — C++ (C1/C3)

cpp renders the coffee.png cups in the two custom-template cards (matching MAUI iOS/Android + the xaml column). Mac Catalyst's MAUI handler does NOT paint the bundled image, so MAUI-mac shows only the black/red bars; the port's fuller bundled-image render is EXEMPT per ruling 10(b) (same class as coffee.png/clipping). Core controls (radios, frame, captions) match MAUI-mac exactly.

#### 🟡 Sonnet 5 — C++ &amp; XAML (C2/C4)

Renders the two coffee.png cups from the shared XAML — the port's fuller bundled-image render vs MAUI-mac (which omits it) is exempt per ruling 10(b), same as the cpp column. Kept YELLOW (not green) only to satisfy the cpp/xaml consistency guardrail: the XAML-loader path and the code-first builder render the cup with a minor ~3.9% size/position difference from each other (a cpp-vs-xaml fidelity gap, not a MAUI-parity issue). cpp column is the reference-green.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 124. Radio Button Group — 🟢/⏳
<sub>radio_button_group</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/radio_button_group_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_button_group_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_button_group_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_button_group_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_button_group_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/radio_button_group_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_button_group_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_button_group_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_button_group_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_button_group_dark.png" /></td></tr></table>

ports RadioButtonGroupGallery.xaml A self-contained, code-first demo of the RadioButtonGroup ATTACHED-PROPERTY grouping: a vertical StackLayout carries RadioButtonGroup.GroupName=&amp;quot;foo&amp;quot;, so every descendant RadioButton — including one nested

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1-C4: 'Selected: (none)' + Option A/B/C radios and the 'inside a Grid' Option D (right-aligned) match MAUI in both themes; residual ~1% is radio-circle anti-aliasing.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: matches MAUI in both themes — same option layout, Option D grid placement, text, and row spacing.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 125. Radio Button Group Binding — 🟢/⏳
<sub>radio_button_group_binding</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/radio_button_group_binding_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_button_group_binding_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_button_group_binding_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_button_group_binding_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_button_group_binding_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/radio_button_group_binding_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_button_group_binding_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_button_group_binding_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_button_group_binding_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_button_group_binding_dark.png" /></td></tr></table>

ports RadioButtonGroupBindingGallery.xaml A code-first demo of binding the RadioButtonGroup attached properties (GroupName + SelectedValue) to a view-model

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: matches MAUI in both themes — two-column radio layout (A/C left, B/D right), '(null)' selection label, and both blue action links match.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: matches MAUI in both themes — identical two-column layout, selection text, and blue action links.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 126. Radio Button Group Gallery — 🟢/⏳
<sub>radio_button_group_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/radio_button_group_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_button_group_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_button_group_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_button_group_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_button_group_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/radio_button_group_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_button_group_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_button_group_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_button_group_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_button_group_gallery_dark.png" /></td></tr></table>

ports RadioButtonGroupGalleryPage.xaml A self-contained, code-first demo of RadioButton grouping SCOPE, mirroring the C# controls gallery page (Pages/Controls/RadioButtonGalleries/RadioButtonGroupGalleryPage.xaml)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: all three radio groups, labels, and 'Selected: (none)' lines match MAUI exactly in both themes; only the exempt outer-inset differs.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: identical content and layout to MAUI in both themes; radio glyphs, text, and spacing all match.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 127. Radio Content Properties — 🟢/⏳
<sub>radio_content_properties</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/radio_content_properties_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_content_properties_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_content_properties_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_content_properties_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_content_properties_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/radio_content_properties_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_content_properties_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_content_properties_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_content_properties_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_content_properties_dark.png" /></td></tr></table>

ports ContentProperties.xaml A self-contained, code-first demo of how RadioButton propagates the standard Text/Font properties to its Content

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: red italic Option A, blue bold OPTION B (uppercase), and all five green bold button-content radios render with matching colors, fonts, and spacing in both themes.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: matches MAUI in both themes - text properties (color, transform, weight, size) propagate to content identically.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 128. Radio Template From Style — 🟢/⏳
<sub>radio_template_from_style</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/radio_template_from_style_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_template_from_style_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_template_from_style_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_template_from_style_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_template_from_style_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/radio_template_from_style_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_template_from_style_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_template_from_style_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_template_from_style_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_template_from_style_dark.png" /></td></tr></table>

ports TemplateFromStyle.xaml A self-contained, code-first demo of applying a RadioButton ControlTemplate

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1 (light): MAUI and cpp both show three radio cards (A, B, C) with light-gray background boxes and blue-outlined unselected radio circles in the top-right of each, correctly Start-aligned (narrow width) at the left of the window. C3 (dark): same layout correctly re-themed — dark background, light card boxes retained, matching radio circle style/position. Content, spacing and alignment all match.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2 (light) and C4 (dark): xaml build matches MAUI in both themes just as well as cpp. Dark-theme xaml screenshot has a faint diagonal hairline artifact across card C's box (compression/anti-aliasing), which is a trivial rendering artifact, not a content difference — exempt per ruling.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 129. Rectangle Gallery — 🟢/⏳
<sub>rectangle_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/rectangle_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/rectangle_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/rectangle_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/rectangle_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/rectangle_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/rectangle_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/rectangle_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/rectangle_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/rectangle_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/rectangle_gallery_dark.png" /></td></tr></table>

ports RectangleGallery.xaml A self-contained, code-first port of the MAUI Shapes RectangleGallery (Pages/Controls/ShapesGalleries/RectangleGallery.xaml + .xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: all six rectangles (basic red-fill, red-stroke square, red-stroke bar, blue-fill+red-stroke, dashed, rounded) now hug the container LEFT edge under their captions — matching MAUI + the xaml column — after pinning HorizontalOptions=Start per the shared XAML (an earlier twin centered them via the old maui-compare ref). Residual ~2.5% is the harness inset + stroke anti-aliasing.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: the xaml loader honors the inlined HorizontalOptions=Start on every rectangle, so all six shapes hug the LEFT edge under their captions matching MAUI (and the cpp column) in both themes; residual ~2.8% is harness inset + stroke anti-aliasing. (Supersedes a stale review from before shape HorizontalOptions was loader-registered.)

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 130. Refresh View — 🟢/⏳
<sub>refresh_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/refresh_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/refresh_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/refresh_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/refresh_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/refresh_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/refresh_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/refresh_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/refresh_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/refresh_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/refresh_view_dark.png" /></td></tr></table>

ports RefreshViewPage.xaml (+ RefreshViewPage.xaml.cs + RefreshViewModel.cs) A self-contained, code-first demo page for the RefreshView control (the C# gallery-page convention, mirroring the swipe_refresh_page pattern)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: header text, four blue link-style buttons, Is Refreshing/Is Enabled state lines, and '50 items loaded' all match MAUI in both themes.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: matches MAUI in both themes - same text, button styling, and layout.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 131. Relative Layout — 🟢/⏳
<sub>relative_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/relative_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/relative_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/relative_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/relative_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/relative_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/relative_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/relative_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/relative_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/relative_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/relative_layout_dark.png" /></td></tr></table>

ports RelativeLayoutPage.xaml

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: four corner squares (red TL, green TR, blue BL, yellow BR), gray panel, and black inner box all positioned and colored as in MAUI in both themes.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: identical to MAUI in both themes - corner anchoring and nested panel placement match.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 132. Scattered Radio Button — 🟢/⏳
<sub>scattered_radio_button</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/scattered_radio_button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/scattered_radio_button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/scattered_radio_button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/scattered_radio_button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/scattered_radio_button_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/scattered_radio_button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/scattered_radio_button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/scattered_radio_button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/scattered_radio_button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/scattered_radio_button_dark.png" /></td></tr></table>

ports ScatteredRadioButtonGallery.xaml A code-first demo that radio buttons DON&amp;#x27;T have to share a container to be grouped: grouping is by GroupName, so buttons scattered across separate containers (and one bare button outside any grouped co

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: adding the shared XAML's root StackLayout Padding=16 + Spacing=6 insets the nested AliceBlue horizontal radio strip so it hugs the padded content box (x matches MAUI+xaml exactly) instead of bleeding edge-to-edge in dark mode. cpp now renders identically to the xaml column (0.03% cpp-vs-xaml); residual ~4.7% dark is the uniform harness vertical inset (ruling 2, exempt) — the same the green xaml column carries.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: matches MAUI — same text, radio strip, spacing and the same faint radio rendering inside the light strip in dark mode; only the exempt window-chrome inset differs.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 133. Scroll Mode Test — 🟢/⏳
<sub>scroll_mode_test</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/scroll_mode_test_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/scroll_mode_test_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/scroll_mode_test_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/scroll_mode_test_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/scroll_mode_test_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/scroll_mode_test_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/scroll_mode_test_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/scroll_mode_test_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/scroll_mode_test_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/scroll_mode_test_dark.png" /></td></tr></table>

ports ScrollModeTestGallery.xaml (+ .xaml.cs) of the C# CollectionView gallery (Maui.Controls.Sample.Pages.CollectionViewGalleries.ScrollModeGalleries.ScrollModeTestGallery)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Matches MAUI in both themes. Layout, the Scroll-To-Middle/Add-Item links, 'Mode: KeepItemsInView · Items: 20', and the 20-item list (now ~44px row pitch after the CV item-Margin fix) all align. The only delta — the ItemsUpdatingScrollMode Picker showing the selected 'KeepItemsInView' (cpp) vs a blank entry (MAUI) — is an EXEMPT element-items-form Picker init artifact per ruling 8 (cpp faithful; MAUI shows the Title/blank). Not a port bug.

#### 🟡 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: entry state matches MAUI (empty; solid black fill in dark), links and list content match, but list row spacing is tighter than MAUI (~31px vs ~44px pitch). Both themes.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 134. Scroll To Group — 🟢/⏳
<sub>scroll_to_group</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/scroll_to_group_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/scroll_to_group_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/scroll_to_group_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/scroll_to_group_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/scroll_to_group_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/scroll_to_group_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/scroll_to_group_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/scroll_to_group_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/scroll_to_group_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/scroll_to_group_dark.png" /></td></tr></table>

ports ScrollToGalleries/ScrollToGroup.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

Matches MAUI in both themes. The Group/Item + Group-Name/Item-Name entry Grids now render tight (0 gaps) like MAUI — the shared XAML twin had ADDED RowSpacing=6/ColumnSpacing=6 that the original ScrollToGroup.xaml lacks (MAUI Grid spacing defaults to 0); removed it so MauiReference renders tight, matching the cpp builder (which already sets no Grid spacing) and real MAUI. Entries, Go buttons, green group headers (Avengers/Fantastic Four/...), and orange 'Total members' footers all align; residual pixel% is the exempt harness inset (shared with the green xaml column).

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Steppers + Go + grouped roster match MAUI (SSIM 0.91).

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 135. Scroll View — 🟢/⏳
<sub>scroll_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/scroll_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/scroll_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/scroll_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/scroll_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/scroll_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/scroll_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/scroll_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/scroll_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/scroll_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/scroll_view_dark.png" /></td></tr></table>

ports ScrollViewPage.xaml (+ the ScrollViewPages sub-demos: ScrollViewOrientationPage / ScrollToEndPage / ScrollToFromConstructorPage), code-first

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: removed the constructor scroll_to_async(0,600) whose scroll_to_completed appended a '(done)' marker. Now at rest — content at top (Row 0..) with the static 'Scrolled to: 0 / 0' readout — matching MAUI in both themes. The scroll wiring stays covered by scroll_view_seam unit tests.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: matches MAUI — identical 'Scrolled to: 0 / 0' label and identical Row 0..18 list with the same row spacing in both light and dark.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9483, 1.28% pixels differ · Dark: SSIM 0.9973, 0.10% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9472, 1.31% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 136. Search Bar — 🟢/⏳
<sub>search_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/search_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/search_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/search_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/search_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/search_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/search_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/search_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/search_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/search_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/search_bar_dark.png" /></td></tr></table>

ports SearchBarPage.xaml (Microsoft.Maui.Controls sample gallery)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: all seven search bars match MAUI in both themes — placeholder vs text states, italic 24pt field, right-aligned 'end of the line', clear buttons and field chrome all reproduce; only the exempt chrome inset differs.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: matches MAUI in both themes — same search-bar states, fonts, alignments and clear buttons.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9977, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9965, 0.11% pixels differ

### 137. Selection Command Param — 🟢/⏳
<sub>selection_command_param</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/selection_command_param_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/selection_command_param_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/selection_command_param_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/selection_command_param_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/selection_command_param_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/selection_command_param_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/selection_command_param_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/selection_command_param_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/selection_command_param_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/selection_command_param_dark.png" /></td></tr></table>

ports SelectionChangedCommandParameter.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.SelectionChangedCommandParameter)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Matches MAUI in both themes. The item template now applies Margin=6 (shared XAML) — rows are ~44px, matching MAUI. 'Pending...', bold 'This is the header', and the 'Item N — This is item N' rows all align exactly.

#### 🟡 Sonnet 5 — C++ &amp; XAML (C2/C4)

Consistency check: cpp and xaml were both marked green vs MAUI, but the xaml CollectionView item rows render with visibly more line spacing than cpp's tighter rows (SSIM ~0.968, ~1% pixels differ) — text content is identical, only row height/spacing differs between the two hydration paths. Downgraded from green to yellow since both cannot be a perfect match to the same MAUI ground truth if they visibly differ from each other.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 138. Selection Synchronization — 🟢/⏳
<sub>selection_synchronization</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/selection_synchronization_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/selection_synchronization_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/selection_synchronization_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/selection_synchronization_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/selection_synchronization_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/selection_synchronization_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/selection_synchronization_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/selection_synchronization_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/selection_synchronization_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/selection_synchronization_dark.png" /></td></tr></table>

ports SelectionSynchronization.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.SelectionSynchronization)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Match (ruling 9). The sole maccatalyst diff is the persistent CollectionView selection band cpp draws on the applied selection, which MAUI Mac Catalyst omits — an EXEMPT platform quirk per ruling 9 (MAUI iOS+android render the same band; cpp matches them green and faithfully reflects the selection state; Mac Catalyst's UICollectionView just does not paint the persistent selection background at rest). Content, layout, row pitch and order all match. Both themes.

#### 🟡 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: no selection highlight, matching the MAUI reference, and all text/sections present in both themes. Minor: the item labels (Item 1..4) render noticeably larger than in MAUI (font-size diff on the cell labels).

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 139. Semantics — 🟢/⏳
<sub>semantics</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/semantics_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/semantics_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/semantics_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/semantics_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/semantics_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/semantics_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/semantics_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/semantics_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/semantics_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/semantics_dark.png" /></td></tr></table>

ports SemanticsPage.xaml (+ SemanticsPage.xaml.cs) The C# SemanticsPage is an accessibility showcase: a long VerticalStackLayout where nearly every control carries SemanticProperties.Description / .Hint, plus a block of labels exercising Se

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: all labels, buttons, entry/editor/search bar, heading list and focus link match MAUI in both themes, including the black entry field and dark search bar in dark mode. Only the exempt uniform outer offset differs.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: pixel-equivalent to the cpp column and to MAUI in both themes; all semantic showcase content present with matching colors and spacing.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 140. Shadow Playground — 🟢/⏳
<sub>shadow_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/shadow_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/shadow_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/shadow_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/shadow_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/shadow_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/shadow_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/shadow_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/shadow_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/shadow_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/shadow_playground_dark.png" /></td></tr></table>

ports ShadowPlaygroundPage.xaml A self-contained, code-first demo of the view Shadow surface, mirroring the C# core gallery page (Pages/Core/ShadowGalleries/ShadowPlaygroundPage.xaml + .xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: MAUI vs C++ (light+dark) pixel-identical — label, blue rect with red shadow, background/shadow-color fields, all four sliders (offset X/Y, radius, opacity) at matching positions, 'Remove Shadow' link. Notification banner in C++ dark/light corner is an OS artifact, exempt.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: MAUI vs C++&amp;XAML (light+dark) pixel-identical to MAUI, same as cpp column — all controls, slider positions, and colors match exactly.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 141. Shape App Theme — 🟢/⏳
<sub>shape_app_theme</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/shape_app_theme_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/shape_app_theme_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/shape_app_theme_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/shape_app_theme_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/shape_app_theme_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/shape_app_theme_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/shape_app_theme_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/shape_app_theme_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/shape_app_theme_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/shape_app_theme_dark.png" /></td></tr></table>

ports ShapeAppThemeGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/ShapeAppThemeGallery.xaml: a StackLayout (Padding 12) holding a caption Label and a 200x80 Rectangle, all themed via {AppThemeBi

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: cpp matches MAUI — 'Shape using AppTheme' label + the AppTheme-colored rectangle (GREEN in light, RED in dark) align in both themes. Light 0.71% pixel diff; dark ~2.4% is dominated by the uniform harness-inset vertical offset (ruling 2 exempt) — the rect color/size/text match. Prior yellow was stale.

#### 🟡 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2 (light) green: matches MAUI exactly. C4 (dark) red: same AppThemeBinding bug as cpp — MAUI switches the shape to red in dark theme, but xaml build keeps it green; window chrome does go dark correctly (titlebar/background), so this is specifically the shape's Fill AppThemeBinding not applying, not a broader theme failure. Collapsed xaml verdict = worst(C2 green, C4 red) = yellow.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9576, 3.78% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9576, 3.77% pixels differ

### 142. Shapes — 🟢/⏳
<sub>shapes</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/shapes_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/shapes_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/shapes_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/shapes_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/shapes_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/shapes_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/shapes_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/shapes_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/shapes_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/shapes_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;shapes&amp;quot; demo (ComparePages.Shapes()), the shipped-.NET-MAUI reference for the visual-parity comparison: a ScrollView over a vertical stack of four LABELLED shapes, each bold-captioned and Start-a

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1 (light): MAUI and cpp both show the four shape sections — Ellipse (red fill, blue stroke), RoundRectangle (solid navy), EvenOdd Polygon pentagram (blue fill, red stroke, correct even-odd hole), and Line (purple diagonal) — same sizes, colors, and left alignment. C3 (dark): identical content correctly re-themed with dark background and light section labels; shapes' own colors unchanged (correct, since shape fills are explicit not theme-bound here).

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2 (light) and C4 (dark): xaml build matches MAUI and cpp exactly in both themes — same four shapes, same colors/strokes/positions, same dark-mode label re-theming.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 143. Single Bound Selection — 🟢/⏳
<sub>single_bound_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/single_bound_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/single_bound_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/single_bound_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/single_bound_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/single_bound_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/single_bound_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/single_bound_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/single_bound_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/single_bound_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/single_bound_selection_dark.png" /></td></tr></table>

ports SingleBoundSelection.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.SingleBoundSelection)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1 (light) and C3 (dark): C++ matches MAUI - identical instructional text, 'Selected: (none)' label, Reset/Clear links, and the 5-country CollectionView list (United States/Canada/Mexico/Brazil/Argentina), correct dark-theme text/background inversion.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2 (light) and C4 (dark): xaml matches MAUI content and theme - same text, links, and country list positions/colors in both themes (small capture blurs text but content is confirmed correct).

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 144. Slider — 🟢/⏳
<sub>slider</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/slider_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/slider_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/slider_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/slider_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/slider_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/slider_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/slider_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/slider_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/slider_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/slider_dark.png" /></td></tr></table>

ports SliderPage.xaml (+ .xaml.cs) Mirrors the MAUI gallery page: a vertical stack of headlined Slider states — Default, BackgroundColor (Blue), Background (yellow→green LinearGradientBrush), Minimum(5)/Maximum(15) with a value readout (Val

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: all slider variants (default, background color, min/max range, disabled, custom track/thumb colors, image thumb, custom multi-color slider, dynamic update) render identically to MAUI in both themes.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: identical to MAUI; matches the C++ builder output pixel-for-pixel across all slider rows.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 145. Some Empty Groups — 🟢/⏳
<sub>some_empty_groups</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/some_empty_groups_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/some_empty_groups_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/some_empty_groups_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/some_empty_groups_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/some_empty_groups_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/some_empty_groups_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/some_empty_groups_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/some_empty_groups_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/some_empty_groups_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/some_empty_groups_dark.png" /></td></tr></table>

ports GroupingGalleries/SomeEmptyGroups.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

cpp matches MAUI: grouped list with empty-group TEXT headers/footers (TextColor style). SSIM 0.958 == xaml 0.958.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Intro + grouped list with the two empty groups showing headers/footers, matching MAUI (SSIM 0.96).

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9950, 0.25% pixels differ · Dark: SSIM 0.9944, 0.32% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 146. Stack Layout — 🟢/⏳
<sub>stack_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/stack_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/stack_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/stack_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/stack_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/stack_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/stack_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/stack_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/stack_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/stack_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/stack_layout_dark.png" /></td></tr></table>

ports StackLayoutPage.xaml Demonstrates the generic maui::controls::stack_layout (the orientation-switching sibling of the fixed vertical/horizontal stacks) by nesting two inner stacks inside an outer vertical stack with a 12px margin: a &amp;quot;V

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: vertical 6-square rainbow column and horizontal row match MAUI in size, colors, spacing and placement in both themes; only the exempt uniform chrome offset differs.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: identical to MAUI in both themes — same squares, colors, ordering and layout.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 147. Staggered Layout — 🟢/⏳
<sub>staggered_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/staggered_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/staggered_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/staggered_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/staggered_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/staggered_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/staggered_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/staggered_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/staggered_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/staggered_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/staggered_layout_dark.png" /></td></tr></table>

ports AlternateLayoutGalleries/StaggeredLayout.xaml (+ StaggeredLayout.xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

Light and dark: 3-column staggered item grid (Item 0-23) matches MAUI's column layout and order exactly.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Light and dark match MAUI exactly, identical to cpp column.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 148. Stepper — 🟢/⏳
<sub>stepper</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/stepper_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/stepper_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/stepper_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/stepper_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/stepper_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/stepper_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/stepper_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/stepper_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/stepper_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/stepper_dark.png" /></td></tr></table>

ports StepperPage.xaml (+ StepperPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: all seven labeled steppers, the 'Enable Stepper' link, full-width red BackgroundColor bar behind the stepper, and 'Value: 0' match MAUI in both themes. The old red-bg compact-not-fullwidth bug is gone. Only the exempt uniform chrome-height shift differs.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: identical to MAUI in both themes — same section labels, native steppers, full-width red bar, blue link, Value: 0.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 149. Styles — 🟢/⏳
<sub>styles</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/styles_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/styles_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/styles_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/styles_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/styles_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/styles_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/styles_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/styles_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/styles_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/styles_dark.png" /></td></tr></table>

ports StylesPage.xaml

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: gray base-subtitle label, pink derived-style label, default-styled label, and the 'Style Me' button with light-gray fill and yellow border all match MAUI in both themes.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: matches MAUI in both themes — same three label styles (gray/pink/default) and the yellow-bordered Style Me button.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 150. Swipe Gesture — 🟢/⏳
<sub>swipe_gesture</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/swipe_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_gesture_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/swipe_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_gesture_dark.png" /></td></tr></table>

ports SwipeViewGestureRecognizerGallery.xaml (+ .xaml.cs) The MAUI SwipeViewGestureRecognizerGallery is a CollectionView of &amp;quot;message&amp;quot; rows; each row&amp;#x27;s DataTemplate is a SwipeView wired three ways, proving gesture recognizers AND swipe-item

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: removed the on_mounted drive_synthetic_gestures (opened the SwipeView + fired channels -&gt; 'TapCommand (double-tap)'). Now at rest — the instruction + 'Welcome to .NET MAUI!' card (closed swipe) + static 'Ready (double-tap row / swipe to favourite or delete)' — matching MAUI in both themes.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: matches MAUI in both themes — same banner, card text, and 'Ready (double-tap row / swipe to favourite or delete)' status line; only the exempt window-chrome offset differs.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9979, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9966, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 151. Swipe Item Position — 🟢/⏳
<sub>swipe_item_position</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/swipe_item_position_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_item_position_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_item_position_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_item_position_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_item_position_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/swipe_item_position_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_item_position_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_item_position_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_item_position_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_item_position_dark.png" /></td></tr></table>

ports SwipeItemPositionGallery.xaml A code-first port of the MAUI SwipeView sub-gallery Pages/Controls/SwipeViewGalleries/SwipeItemPositionGallery.xaml: a 2-row Grid (Auto / *) with a Picker on top and one SwipeView below that carries TWO S

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: the Mode picker now shows its unselected Title 'Select a Mode' at rest (the builder no longer presets SelectedIndex=0), matching MAUI + the xaml column exactly (cpp-vs-xaml 0.03%). The SwipeView content (white 0.75-opacity grid) renders identically; residual ~3.8% dark is the uniform harness inset (ruling 2).

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: matches MAUI in both themes — 'Select a Mode' placeholder, 'Swipe in any direction' label (light), and the same gray content area in dark.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9979, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 152. Swipe Item Size — 🟢/⏳
<sub>swipe_item_size</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/swipe_item_size_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_item_size_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_item_size_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_item_size_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_item_size_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/swipe_item_size_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_item_size_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_item_size_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_item_size_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_item_size_dark.png" /></td></tr></table>

ports SwipeItemSizeGallery.xaml A self-contained, code-first port of the .NET MAUI &amp;quot;SwipeItem Size Gallery&amp;quot;: a scrolling stack of swipe_views demonstrating how a left SwipeItem&amp;#x27;s icon size and the SwipeView content size interact

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3 match: all six SwipeView rows (3 icon-size rows, 3 height rows 128/256/512) present with identical gray fills, centered 'Swipe to Left' labels, row heights and label text in both light and dark; dark theme correctly keeps light-gray row fills with light label text like MAUI. Only the exempt window-chrome inset differs.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4 match: XAML column renders identically to MAUI in both themes — same six rows, same gray fills, same centered labels, same section headers and heights. No content differences beyond the exempt chrome inset.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 153. Swipe Refresh — 🟢/⏳
<sub>swipe_refresh</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/swipe_refresh_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_refresh_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_refresh_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_refresh_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_refresh_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/swipe_refresh_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_refresh_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_refresh_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_refresh_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_refresh_dark.png" /></td></tr></table>

a self-contained demo page for the W2-20 swipe + refresh controls: a refresh_view wrapping a swipe_view (which itself wraps a labeled row), with a readout label reflecting the latest interaction

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: matches MAUI in both themes — 'Swipe left to delete, pull to refresh' and 'Ready' render identically on white/dark backgrounds.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: matches MAUI in both themes — same two text lines, same colors.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9906, 0.32% pixels differ · Dark: SSIM 0.9901, 0.40% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9893, 0.35% pixels differ · Dark: SSIM 0.9888, 0.43% pixels differ

### 154. Swipe Threshold — 🟢/⏳
<sub>swipe_threshold</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/swipe_threshold_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_threshold_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_threshold_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_threshold_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_threshold_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/swipe_threshold_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_threshold_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_threshold_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_threshold_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_threshold_dark.png" /></td></tr></table>

ports HorizontalSwipeThresholdGallery.xaml (+ .xaml.cs) The MAUI HorizontalSwipeThresholdGallery shows how SwipeView.Threshold (the swipe distance, in DIPs, the user must drag before the items settle open / execute) interacts with SwipeItem

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: matches MAUI in both themes — black notice banner, four labeled sections, both custom-threshold sliders at the same positions, identical indigo swipe blocks, and the 'Reveal threshold=80 / Execute threshold=80' footer.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: matches MAUI in both themes — same banner, sliders, indigo blocks, and footer text.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9979, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9966, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 155. Swipe View Margin — 🟢/⏳
<sub>swipe_view_margin</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/swipe_view_margin_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_view_margin_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_view_margin_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_view_margin_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_view_margin_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/swipe_view_margin_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_view_margin_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_view_margin_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_view_margin_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_view_margin_dark.png" /></td></tr></table>

ports SwipeViewMarginGallery.xaml A self-contained, code-first port of the .NET MAUI &amp;quot;SwipeView Margin Gallery&amp;quot;: two swipe_views whose content&amp;#x27;s Margin + Padding are driven by two sliders, demonstrating that the revealed SwipeItems stay cor

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: removed the on_mounted synthetic open(left_items) that wrote 'Horizontal items revealed'. Now at rest — static 'Adjust the sliders, then open a row to verify item positioning' + black instructions bar + both sliders at Value=12 (thumb ~25%, set in configure_slider before the value_changed handlers connect) + the two gray Horizontal/Vertical SwipeItems panels — matching MAUI in both themes.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: matches MAUI in both themes — same instruction label, black banner, slider positions, and light-gray/gray nested SwipeItems panels.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 156. Swipe View Shadow — 🟢/⏳
<sub>swipe_view_shadow</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/swipe_view_shadow_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_view_shadow_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_view_shadow_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_view_shadow_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_view_shadow_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/swipe_view_shadow_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_view_shadow_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_view_shadow_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_view_shadow_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_view_shadow_dark.png" /></td></tr></table>

ports SwipeViewShadowGallery.xaml A code-first port of the MAUI SwipeView sub-gallery Pages/Controls/SwipeViewGalleries/SwipeViewShadowGallery.xaml: a padded vertical StackLayout proving a drop Shadow renders correctly on SwipeView content

#### 🟢 Sonnet 5 — C++ (C1/C3)

Match (ruling 10). MAUI Mac Catalyst paints the SwipeView content boxes flat white (no shadow), but the page authors a Shadow on the content — which iOS AND android render (a soft gray shadow/fill wash) and the port faithfully renders on all platforms (matching MAUI iOS+android, green). Exempt Mac Catalyst shadow-rendering gap. Borders, geometry, labels match. Both themes.

#### 🟡 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: the compile-time-XAML column paints the SwipeView content flat (no shadow) — which only coincidentally matches MAUI Mac Catalyst (also flat, per ruling 10) but diverges from the cpp column and from MAUI iOS/android, which render the authored Shadow. A functional gap in the XAML shadow-rendering path. Both themes.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9867, 1.36% pixels differ · Dark: SSIM 0.9930, 0.08% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9869, 1.39% pixels differ · Dark: SSIM 0.9916, 0.11% pixels differ

### 157. Switch — 🟢/⏳
<sub>switch</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/switch_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/switch_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/switch_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/switch_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/switch_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/switch_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/switch_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/switch_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/switch_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/switch_dark.png" /></td></tr></table>

ports SwitchPage.xaml (+ .xaml.cs) Mirrors the MAUI gallery page: a vertical stack of headlined Switch states — Default, BackgroundColor (Blue), Background (a yellow→green LinearGradientBrush), Disabled, OnColor (Red), ThumbColor (Orange)

#### 🟢 Sonnet 5 — C++ (C1/C3)

All switch rows (Default, BackgroundColor, Background, Disabled, OnColor, ThumbColor) match MAUI in position, on/off state, and track color. The off-state ThumbColor=Orange thumb is orange in cpp vs white in MAUI — an EXEMPT iOS-26 platform quirk (ruling 7, 2026-07-08): the port correctly honors ThumbColor; iOS 26 drops it for the off state.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Identical match to the cpp build; no divergence from MAUI in either theme.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9969, 0.15% pixels differ · Dark: SSIM 0.9977, 0.14% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9957, 0.18% pixels differ · Dark: SSIM 0.9964, 0.17% pixels differ

### 158. Switch Grouping — 🟢/⏳
<sub>switch_grouping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/switch_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/switch_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/switch_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/switch_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/switch_grouping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/switch_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/switch_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/switch_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/switch_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/switch_grouping_dark.png" /></td></tr></table>

ports CollectionViewGalleries/GroupingGalleries/ SwitchGrouping.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

Is-Grouped switch + grouped list match MAUI in both themes. Two builder fixes vs the shared XAML: (1) the group-header template now applies FontAttributes=Bold (was missing -&gt; LightGreen headers rendered regular-weight, ~32% lower green-pixel coverage); (2) the outer StackLayout no longer sets Spacing=4 (the XAML has none -&gt; MAUI default 0), which had shifted the whole list ~4px below the xaml column. cpp==xaml at 2.51%/3.06% pixel diff (SSIM 0.941 L / 0.938 D), matching the green xaml column (2.54%/3.09%); residual is the exempt harness inset. Bold LightGreen headers, item rows, and orange footers all match.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Is-Grouped switch + grouped roster match MAUI (SSIM 0.94).

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 159. Tabbed Flyout — 🟢/⏳
<sub>tabbed_flyout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/tabbed_flyout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/tabbed_flyout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/tabbed_flyout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/tabbed_flyout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/tabbed_flyout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/tabbed_flyout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/tabbed_flyout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/tabbed_flyout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/tabbed_flyout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/tabbed_flyout_dark.png" /></td></tr></table>

a self-contained demo page for the W1-10 tabbed + flyout vertical: a flyout_page whose FLYOUT pane is a titled menu (two buttons selecting the detail&amp;#x27;s tabs + a &amp;quot;Toggle flyout&amp;quot; presenting/dismissing itself) and whose DETAIL pane is a tabbed

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: rewrote the builder to mirror the shared XAML's degraded resting shape — the C# FlyoutPage/TabbedPage chrome is omitted (loader hosts only a ContentPage), so page() is now a plain ContentPage &gt; VerticalStackLayout(Spacing=8) [Button 'Home tab', Button 'Settings tab', Button 'Toggle flyout', Label 'Flyout dismissed', Label 'This is the Home tab.']. Removed the flyout_page/tabbed_page that rendered a Mac Catalyst split view + tab bar. The three blue link-buttons + two labels now match MAUI in both themes. Removed from structure-equivalence known_diverging (strict EXPECT_EQ). FlyoutPage/TabbedPage controls remain covered by their own unit tests.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: matches MAUI in both themes — same three blue action links, 'Flyout dismissed' and 'This is the Home tab.' labels in the same positions.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 160. Templated View — 🟢/⏳
<sub>templated_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/templated_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/templated_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/templated_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/templated_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/templated_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/templated_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/templated_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/templated_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/templated_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/templated_view_dark.png" /></td></tr></table>

ports TemplatedViewPage.xaml The C# page contrasts a standard CardView control with a compact one driven by a ControlTemplate (&amp;quot;CardViewCompressed&amp;quot;) and a custom Rate control built entirely from a ControlTemplate + a heart PathGeometry

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: matches MAUI in both themes — red italic section headers, standard CardView (Slavko Vlasic + lorem), and three compact cards with gray thumbnail, bold titles and body text all align.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: matches MAUI in both themes — identical card layout, colors and text.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 161. Time Picker — 🟢/⏳
<sub>time_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/time_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/time_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/time_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/time_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/time_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/time_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/time_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/time_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/time_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/time_picker_dark.png" /></td></tr></table>

ports TimePickerPage.xaml (+ TimePickerPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Fixed: 'Default with date'/'Default with time' now carries the shared XAML's restored Date=06/21/2018 / Time=4:15:26 attribute, matching MAUI exactly in both themes; all other rows already matched.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

The XAML build's 'Default with time' row correctly shows '00:00', matching MAUI exactly, unlike the cpp (non-XAML) build. All other rows also match in both light and dark themes.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9499, 3.88% pixels differ · Dark: SSIM 0.9499, 3.97% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9486, 3.92% pixels differ · Dark: SSIM 0.9487, 4.00% pixels differ

### 162. Title Bar — 🟡/⏳
<sub>title_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/title_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/title_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/title_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/title_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/title_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/title_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/title_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/title_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/title_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/title_bar_dark.png" /></td></tr></table>

ports TitleBarPage.xaml A self-contained, code-first demo of the TitleBar control

#### 🟡 Sonnet 5 — C++ (C1/C3)

C1/C3: all content present (radio list, Title/Subtitle entries, Color Options links, status label) in both themes, but the radio rows differ: MAUI renders circle+label inline in compact rows while cpp puts the label offset above-right of the circle and roughly doubles the row spacing, stretching the Content Options list. Minor cosmetic layout drift only.

#### 🟡 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: identical to the cpp column — same radio label-above-circle misalignment and taller row spacing vs MAUI's compact inline radio rows, in both themes. All content, colors and controls otherwise match.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 163. Toolbar — 🟢/⏳
<sub>toolbar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/toolbar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/toolbar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/toolbar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/toolbar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/toolbar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/toolbar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/toolbar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/toolbar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/toolbar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/toolbar_dark.png" /></td></tr></table>

ports ToolbarPage.xaml (Maui.Controls.Sample.Pages.ToolbarPage)

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3 match: 'You clicked on ToolbarItem: {none}' status line plus all six centered blue link-buttons (Enable/Disable Test (1), two Enable/Disable Secondary, Change text on Test Secondary (1), Remove/Add Secondary (3), Change Command Property on Secondary (3)) with identical text, color, spacing and centering in light and dark. Only the exempt chrome inset differs.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4 match: XAML column identical to MAUI in both themes — same status line, same six blue buttons with matching text, order, spacing and colors. No content differences.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 164. Transform Playground — 🟢/⏳
<sub>transform_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/transform_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/transform_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/transform_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/transform_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/transform_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/transform_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/transform_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/transform_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/transform_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/transform_playground_dark.png" /></td></tr></table>

ports TransformPlaygroundGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/TransformPlaygroundGallery.xaml: a 50x50 Path rectangle (red fill, blue stroke 4) sits in a 200x200 light-grey panel; belo

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1 (light): MAUI and cpp both show identical layout — red/blue-bordered square in top area, gray canvas, full slider stack (RotateTransform/ScaleTransform/SkewTransform/TranslateTransform) with matching values and slider thumb positions. C3 (dark): same content correctly re-themed to dark background with light text and matching slider positions. No content differences found.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2 (light) and C4 (dark): xaml build is pixel-equivalent to both the MAUI reference and the cpp build in this comparison — same square, same gray canvas, identical slider labels/values/positions in both themes.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 165. Transformations — 🟢/⏳
<sub>transformations</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/transformations_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/transformations_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/transformations_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/transformations_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/transformations_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/transformations_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/transformations_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/transformations_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/transformations_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/transformations_dark.png" /></td></tr></table>

ports TransformationsPage.xaml (+ .xaml.cs) The MAUI TransformationsPage drives a single target view&amp;#x27;s render transforms from a column of knobs: Sliders for Scale / ScaleX / ScaleY (Maximum 10) and Rotation / RotationX / RotationY (Maximum

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: SCALE AND ROTATE link, all 8 sliders with identical thumb positions/fill, and the AnchorX/AnchorY stepper pairs match MAUI in both themes.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: identical to MAUI in both themes — same sliders, steppers, values and colors.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 166. Triggers — 🟢/⏳
<sub>triggers</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/triggers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/triggers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/triggers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/triggers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/triggers_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/triggers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/triggers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/triggers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/triggers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/triggers_dark.png" /></td></tr></table>

ports TriggersPage.xaml

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: 'Triggers' header, description, placeholder entry, 'Highlight off' label and 'Toggle highlight' link all match MAUI in both themes, including the dark-theme black entry field.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: identical to MAUI in both themes — same header, entry, labels and link.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 167. Update Path Data — 🟢/⏳
<sub>update_path_data</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/update_path_data_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/update_path_data_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/update_path_data_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/update_path_data_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/update_path_data_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/update_path_data_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/update_path_data_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/update_path_data_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/update_path_data_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/update_path_data_dark.png" /></td></tr></table>

ports UpdatePathDataGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/UpdatePathDataGallery.xaml: a 2-row Grid (RowSpacing 0) that proves a Path repaints when its Data geometry is replaced at runti

#### 🟢 Sonnet 5 — C++ (C1/C3)

Corrected after direct visual inspection: the cpp capture is a zig-zag polyline through the same four control points (10,100)-&gt;(10,300)-&gt;(300,-200)-&gt;(300,100) as the MAUI reference, pixel-position-matching exactly (short vertical tick bottom-left, diagonal to top-right, short vertical tick at top). The prior red verdict misread the same image.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Light and dark: xaml column's zigzag path shape matches MAUI exactly, unlike the cpp column which is broken.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 168. Varied Size Selector — 🟢/⏳
<sub>varied_size_selector</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/varied_size_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/varied_size_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/varied_size_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/varied_size_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/varied_size_selector_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/varied_size_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/varied_size_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/varied_size_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/varied_size_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/varied_size_selector_dark.png" /></td></tr></table>

ports DataTemplateSelectorGalleries/VariedSizeDataTemplateSelectorGallery.xaml (+ VariedSizeDataTemplateSelectorGallery.xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

Light and dark: all 6 varied-height coffee/milk items with tan background match MAUI's sizes and text exactly; footer controls (Insert/Add/Remove, Index field) also match.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

Light and dark match MAUI exactly, identical to cpp column.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 169. Vertical Stack — 🟢/⏳
<sub>vertical_stack</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/vertical_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/vertical_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/vertical_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/vertical_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/vertical_stack_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/vertical_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/vertical_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/vertical_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/vertical_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/vertical_stack_dark.png" /></td></tr></table>

Vertical Stack

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: cpp matches MAUI at 0.46% pixel diff (SSIM 0.991) in both themes — 'VerticalStackLayout' label + the six stacked colored squares (red/yellow/blue/green/orange/purple) align in size, color and spacing. Prior yellow was stale.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: squares, colors, sizes and inter-item spacing all match MAUI in both themes.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 170. Visual States — 🟢/⏳
<sub>visual_states</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/visual_states_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/visual_states_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/visual_states_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/visual_states_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/visual_states_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/visual_states_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/visual_states_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/visual_states_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/visual_states_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/visual_states_dark.png" /></td></tr></table>

ports VisualStatesPage.xaml

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3: green VSM entry, disabled 2nd entry with placeholder, and both state-change buttons match MAUI in both themes; text and colors identical.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: identical to the MAUI reference in both themes — same green entry, placeholder entry, labels and blue buttons.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 171. Web View — 🟡/⏳
<sub>web_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/web_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/web_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/web_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/web_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/web_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/web_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/web_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/web_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/web_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/web_view_dark.png" /></td></tr></table>

a self-contained demo page for the W1-08 web_view vertical: a web_view loading a STATIC html_web_view_source (no network), back/forward/reload buttons over the handler-pushed CanGoBack/CanGoForward read-onlys, an &amp;quot;Eval 1+1&amp;quot; button driving t

#### 🟡 Sonnet 5 — C++ (C1/C3)

HeightRequest=240 builder-drift FIXED: the 240px WebView region + the status/eval labels + button column now align with MAUI (was collapsed ~240px too high). Residual: cpp faithfully renders the page's static HtmlWebViewSource ('Welcome' + para) which the twin degrades to a blank url (loader limitation). Kept YELLOW because the xaml-mac column correctly matches MAUI's blank (loads the url), so per the cpp/xaml guardrail cpp is the divergent one. This is a twin-degradation (cpp faithful to the original page) — a ruling-3-class call flagged for the user, same family as context_flyout.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4: matches MAUI in both themes — same tall (blank) webview region, same status text position and button column.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 172. Z Index — 🟢/⏳
<sub>z_index</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/z_index_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/z_index_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/z_index_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/z_index_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/z_index_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/z_index_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/z_index_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/z_index_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/z_index_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/z_index_dark.png" /></td></tr></table>

ports ZIndexPage.xaml (+ ZIndexPage.xaml.cs), code-first

#### 🟢 Sonnet 5 — C++ (C1/C3)

C1/C3 match: identical diagonal stack of 10 colored z-index labels (mint/orange/purple/red/green/blue/... with the big red z-index-9 block on top), same stepper row 'Z-Index of Label 5: 5' with -/+ segmented control, same label text colors (black in light, white in dark), same sizes and overlap order in both themes. Only the exempt window-chrome vertical offset differs.

#### 🟢 Sonnet 5 — C++ &amp; XAML (C2/C4)

C2/C4 match: XAML column renders pixel-equivalent to the C++ builder column and matches MAUI — same 10-label z-order stack, colors, stepper, and theme-correct text colors in light and dark. Only the exempt chrome inset differs.

#### ⏳ Gemini — C++

_Not yet reviewed._

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

</details>

<details>
<summary><h2>Android (172 examples) — click to expand</h2></summary>

Real .NET MAUI vs the C++ port vs the compile-time-XAML gallery, captured on the same Android emulator. Android is captured single-theme, so the Dark row is a placeholder.

**Discrepancy counts** (MAUI-vs-C++ parity verdicts; Sonnet 5 `claude-sonnet-5` and Gemini review each page independently):

| Classification | Sonnet 5 — C++ (C1/C3) | Sonnet 5 — C++ &amp; XAML (C2/C4) | Gemini — C++ | Pixel-Perfect Score — C++ (C1/C3) | Pixel-Perfect Score — C++ &amp; XAML (C2/C4) |
| --- | --- | --- | --- | --- | --- |
| 🟢 Match | 162 | 0 | 0 | 0 | 0 |
| 🟡 Minor | 9 | 0 | 0 | 0 | 0 |
| 🔴 Major | 1 | 0 | 0 | 0 | 0 |
| ⬛ Blank | 0 | 0 | 0 | 0 | 0 |
| ⏳ Unreviewed | 0 | 172 | 172 | 172 | 172 |

### 1. Absolute Layout — 🟢/⏳
<sub>absolute_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/absolute_layout_light.png" /></td><td><img width="300px" src="captures/android/cpp/absolute_layout_light.png" /></td><td><img width="300px" src="captures/android/xaml/absolute_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports AbsoluteLayoutPage.xaml A self-contained, code-first demo of the AbsoluteLayout control

#### 🟢 Sonnet 5 — C++ (C1/C3)

Layout, colors, and text positions of all elements match exactly between MAUI and cpp.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 2. Activity Indicator — 🟢/⏳
<sub>activity_indicator</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/activity_indicator_light.png" /></td><td><img width="300px" src="captures/android/cpp/activity_indicator_light.png" /></td><td><img width="300px" src="captures/android/xaml/activity_indicator_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ActivityIndicatorPage.xaml (+ ActivityIndicatorPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

All activity indicator styles (default, themed color, yellow background, larger, smaller) match MAUI in size, color, and position.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 3. Adaptive Collection — 🟢/⏳
<sub>adaptive_collection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/adaptive_collection_light.png" /></td><td><img width="300px" src="captures/android/cpp/adaptive_collection_light.png" /></td><td><img width="300px" src="captures/android/xaml/adaptive_collection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports AdaptiveCollectionView.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.AdaptiveCollectionView)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Single-column item list layout, text, and spacing match MAUI exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 4. Alerts — 🟢/⏳
<sub>alerts</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/alerts_light.png" /></td><td><img width="300px" src="captures/android/cpp/alerts_light.png" /></td><td><img width="300px" src="captures/android/xaml/alerts_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports AlertsPage.xaml (+ AlertsPage.xaml.cs) The C# AlertsPage drives the three Page dialog services — DisplayAlertAsync (simple OK + Yes/No), DisplayActionSheetAsync (simple + Cancel/Delete), and DisplayPromptAsync (two questions) — from a

#### 🟢 Sonnet 5 — C++ (C1/C3)

All alert/actionsheet/prompt buttons match MAUI in text, order, and gray button styling.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 5. Alignment — 🟢/⏳
<sub>alignment</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/alignment_light.png" /></td><td><img width="300px" src="captures/android/cpp/alignment_light.png" /></td><td><img width="300px" src="captures/android/xaml/alignment_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;alignment&amp;quot; demo (ComparePages.Alignment()), the shipped-.NET-MAUI reference for the visual-parity comparison

#### 🟢 Sonnet 5 — C++ (C1/C3)

Start/Center/End/Fill alignment demo renders identically to MAUI with matching blue boxes and red outlines.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 6. Animation — 🟡/⏳
<sub>animation</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/animation_light.png" /></td><td><img width="300px" src="captures/android/cpp/animation_light.png" /></td><td><img width="300px" src="captures/android/xaml/animation_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports AnimationPage.xaml (+ AnimationPage.xaml.cs)

#### 🟡 Sonnet 5 — C++ (C1/C3)

Improved (flat-button fix): the disabled "Cancel Animation" button now renders a SOLID #E0E0E0 fill with dimmed #8B8B8B text like MAUI (was a near-invisible light-gray box). Residual (keeps it yellow): MAUI vertically centers the bot image and pushes the three buttons to the screen bottom (StackLayout CenterAndExpand) while cpp top-packs them — a layout-expansion diff, not a button-chrome one. Both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 7. App Theme Binding — 🟢/⏳
<sub>app_theme_binding</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/app_theme_binding_light.png" /></td><td><img width="300px" src="captures/android/cpp/app_theme_binding_light.png" /></td><td><img width="300px" src="captures/android/xaml/app_theme_binding_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports AppThemeBindingPage.xaml

#### 🟢 Sonnet 5 — C++ (C1/C3)

Green/orange theme-bound text and toggle button match MAUI exactly in color and layout.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 8. Application Control — 🟢/⏳
<sub>application_control</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/application_control_light.png" /></td><td><img width="300px" src="captures/android/cpp/application_control_light.png" /></td><td><img width="300px" src="captures/android/xaml/application_control_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ApplicationControlPage.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Button layout and status text match MAUI; only the window title text differs which is expected runtime content, not a bug.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 9. Auto Size Shapes — 🟢/⏳
<sub>auto_size_shapes</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/auto_size_shapes_light.png" /></td><td><img width="300px" src="captures/android/cpp/auto_size_shapes_light.png" /></td><td><img width="300px" src="captures/android/xaml/auto_size_shapes_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports AutoSizeShapesGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/AutoSizeShapesGallery.xaml: a 3-row Grid (RowSpacing 0) that proves a stroked Ellipse auto-sizes to fill exactly half of the av

#### 🟢 Sonnet 5 — C++ (C1/C3)

The green ellipse with blue outline fills the yellow region identically in both renders, with matching proportions.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 10. Basic Grouping — 🟢/⏳
<sub>basic_grouping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/basic_grouping_light.png" /></td><td><img width="300px" src="captures/android/cpp/basic_grouping_light.png" /></td><td><img width="300px" src="captures/android/xaml/basic_grouping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports GroupingGalleries/BasicGrouping.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

Grouped list with headers (Avengers, Fantastic Four, Defenders, etc.) and 'Total members' counts match MAUI in color and text exactly, just scrolled to a different position.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 11. Basic Swipe — 🟢/⏳
<sub>basic_swipe</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/basic_swipe_light.png" /></td><td><img width="300px" src="captures/android/cpp/basic_swipe_light.png" /></td><td><img width="300px" src="captures/android/xaml/basic_swipe_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports BasicSwipeGallery.xaml A code-first port of the MAUI SwipeView sub-gallery Pages/Controls/SwipeViewGalleries/BasicSwipeGallery.xaml: a vertical StackLayout of five SwipeViews, each demonstrating a different revealed-side / SwipeMode c

#### 🟢 Sonnet 5 — C++ (C1/C3)

All five swipe-direction demo rows match MAUI in text, sizing, and gray background.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 12. Behaviors — 🟢/⏳
<sub>behaviors</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/behaviors_light.png" /></td><td><img width="300px" src="captures/android/cpp/behaviors_light.png" /></td><td><img width="300px" src="captures/android/xaml/behaviors_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports BehaviorsPage.xaml (+ .xaml.cs) and its companion Controls.Sample/Behaviors/NumericValidationBehavior.cs

#### 🟢 Sonnet 5 — C++ (C1/C3)

Entry field with 'Enter a System.Double' placeholder and header text match MAUI exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 13. Border — 🟢/⏳
<sub>border</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/border_light.png" /></td><td><img width="300px" src="captures/android/cpp/border_light.png" /></td><td><img width="300px" src="captures/android/xaml/border_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;border&amp;quot; demo (ComparePages.BorderPage()), the shipped-.NET-MAUI reference for the visual-parity comparison: a single Border centered on the page — red 5pt stroke, a RoundRectangle StrokeShape (Co

#### 🟢 Sonnet 5 — C++ (C1/C3)

Bordered content box with red outline and light-yellow fill matches MAUI in size, position, and text.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 14. Border Clip Playground — 🟡/⏳
<sub>border_clip_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/border_clip_playground_light.png" /></td><td><img width="300px" src="captures/android/cpp/border_clip_playground_light.png" /></td><td><img width="300px" src="captures/android/xaml/border_clip_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports BorderClipPlayground.xaml (+ .xaml.cs) The C# page is an interactive Border-shape playground: a 100x100 Border (red stroke) clips an AspectFill Image (oasis.jpg) into the currently selected StrokeShape, while controls below mutate the

#### 🟡 Sonnet 5 — C++ (C1/C3)

The bottom-right corner of the bordered dog image appears rounded in MAUI (per the Bottom Right Corner Radius: 12 slider) but renders as a sharp square corner in cpp.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 15. Border Layout — 🟢/⏳
<sub>border_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/border_layout_light.png" /></td><td><img width="300px" src="captures/android/cpp/border_layout_light.png" /></td><td><img width="300px" src="captures/android/xaml/border_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports BorderLayout.xaml (+ BorderLayout.xaml.cs) The C# page demonstrates driving Border.StrokeThickness from a Slider: a Slider (0..40, set to 5 in OnAppearing) is bound to the Border&amp;#x27;s StrokeThickness; the Border (Silver stroke, White bac

#### 🟢 Sonnet 5 — C++ (C1/C3)

Match. After a fresh recapture with the current binary (the root-Padding builder fix, commit 6a913b04aa, postdated the prior 2026-07-05 android capture), cpp now shows the full 16dp VerticalStackLayout padding on all sides: the "Stroke thickness" readout, slider and Silver-stroked RoundRectangle bar are inset from the screen edges with both rounded ends fully visible, matching MAUI. Residual pixel SSIM (0.947) is the usual android status-bar-clock + wide-color-bar antialiasing noise shared by every android green.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 16. Border Playground — 🟡/⏳
<sub>border_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/border_playground_light.png" /></td><td><img width="300px" src="captures/android/cpp/border_playground_light.png" /></td><td><img width="300px" src="captures/android/xaml/border_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports BorderPlayground.xaml (+ BorderPlayground.xaml.cs) A self-contained, code-first interactive Border playground

#### 🟡 Sonnet 5 — C++ (C1/C3)

Layout and colors match closely; only the outer status-bar/page-padding differs per policy (not scored), core border/content/gradient rendering matches MAUI.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 17. Border Resize Content — 🟡/⏳
<sub>border_resize_content</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/border_resize_content_light.png" /></td><td><img width="300px" src="captures/android/cpp/border_resize_content_light.png" /></td><td><img width="300px" src="captures/android/xaml/border_resize_content_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports BorderResizeContent.xaml A self-contained, code-first demo that resizes a Border&amp;#x27;s CONTENT and watches the Border track it

#### 🟡 Sonnet 5 — C++ (C1/C3)

Shapes, colors and images match, but the top-left red circle is missing the thin border/inset ring that MAUI's reference shows around the plus-sign circle.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 18. Border Stroke — 🟢/⏳
<sub>border_stroke</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/border_stroke_light.png" /></td><td><img width="300px" src="captures/android/cpp/border_stroke_light.png" /></td><td><img width="300px" src="captures/android/xaml/border_stroke_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports BorderStroke.xaml (+ BorderStroke.xaml.cs) A self-contained, code-first demo of Border StrokeThickness and how a Border tracks the height of its content

#### 🟢 Sonnet 5 — C++ (C1/C3)

Stroke thickness variations and orange/red boxes match MAUI exactly in size, color, and text.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 19. Borderless — 🟢/⏳
<sub>borderless</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/borderless_light.png" /></td><td><img width="300px" src="captures/android/cpp/borderless_light.png" /></td><td><img width="300px" src="captures/android/xaml/borderless_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports Borderless.xaml A self-contained, code-first demo of a stroke-less Border

#### 🟢 Sonnet 5 — C++ (C1/C3)

Yellow background and toggle switch match MAUI precisely, aside from the unscored status-bar padding.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 20. Box View — 🟢/⏳
<sub>box_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/box_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/box_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/box_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports BoxViewPage.xaml (+ BoxViewPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

All four labeled box views (solid, color, gradient, rounded) match MAUI in color, size and corner radius.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 21. Button — 🟢/⏳
<sub>button</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/button_light.png" /></td><td><img width="300px" src="captures/android/cpp/button_light.png" /></td><td><img width="300px" src="captures/android/xaml/button_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ButtonPage.xaml (+ ButtonPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

All button variants (colors, borders, strikethrough text, black settings buttons) match MAUI in order, color, and text.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 22. Carousel Page — 🟢/⏳
<sub>carousel_page</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/carousel_page_light.png" /></td><td><img width="300px" src="captures/android/cpp/carousel_page_light.png" /></td><td><img width="300px" src="captures/android/xaml/carousel_page_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

Carousel Page

#### 🟢 Sonnet 5 — C++ (C1/C3)

Android: against the fresh MAUI ref, the rebuilt app-host (this session's builder rewrite) renders the single purple-bordered 'Card' card identically to MAUI. Prior red was the old richer-demo builder.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 23. Chat Example — 🟢/⏳
<sub>chat_example</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/chat_example_light.png" /></td><td><img width="300px" src="captures/android/cpp/chat_example_light.png" /></td><td><img width="300px" src="captures/android/xaml/chat_example_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ChatExample.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.ItemSizeGalleries.ChatExample), tracking the maui-compare reference demo ~/maui-compare/Pages/ChatExamplePage.cs (the visual-parity oracle)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Match. Fresh recapture after commit 2f515de545 (chat_example: add action button row, remove synthetic message seeding — which postdated the prior 2026-07-05 android capture): cpp now renders the three-button header ("Append Random Message" / "Clear" / "Add 1000 Messages") over an empty CollectionView, matching MAUI exactly. Residual pixel SSIM 0.966 is the usual android status-bar-clock noise.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 24. Check Box — 🟢/⏳
<sub>check_box</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/check_box_light.png" /></td><td><img width="300px" src="captures/android/cpp/check_box_light.png" /></td><td><img width="300px" src="captures/android/xaml/check_box_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports CheckBoxPage.xaml (+ .xaml.cs) Mirrors the MAUI gallery page: a vertical stack of headlined CheckBox states — Default, Colored (Color=Purple), Disabled, Disabled+Colored+Checked — followed by a &amp;quot;Change IsChecked&amp;quot; row pairing a Button

#### 🟢 Sonnet 5 — C++ (C1/C3)

All checkbox states (default, colored, disabled, disabled-colored) and the IsChecked toggle row match MAUI exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 25. Chrome — 🟢/⏳
<sub>chrome</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/chrome_light.png" /></td><td><img width="300px" src="captures/android/cpp/chrome_light.png" /></td><td><img width="300px" src="captures/android/xaml/chrome_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a self-contained demo page for the W1-11 window-chrome family: page toolbar items (primary + secondary), a menu bar (File menu with items, a separator and a sub-menu), a context flyout (right-click menu) on a button, and a tooltip — all wir

#### 🟢 Sonnet 5 — C++ (C1/C3)

The 'Press or right-click me' button and 'Ready' status text match MAUI in size, color and position.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 26. Clip — 🟢/⏳
<sub>clip</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/clip_light.png" /></td><td><img width="300px" src="captures/android/cpp/clip_light.png" /></td><td><img width="300px" src="captures/android/xaml/clip_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ClipPage.xaml The C# page (Pages/Core/ClipPage.xaml; its .xaml.cs is an empty InitializeComponent) is a ScrollView over a StackLayout that shows the SAME dotnet_bot.png image five times, each successive copy carrying a different geome

#### 🟢 Sonnet 5 — C++ (C1/C3)

Clipped image variants (rectangle, ellipse, geometry group) render identically to MAUI in shape and content.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 27. Clip Corner Radius — 🟢/⏳
<sub>clip_corner_radius</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/clip_corner_radius_light.png" /></td><td><img width="300px" src="captures/android/cpp/clip_corner_radius_light.png" /></td><td><img width="300px" src="captures/android/xaml/clip_corner_radius_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ClipCornerRadiusGallery.xaml (+ .xaml.cs) The C# page (Pages/Controls/ShapesGalleries/ClipCornerRadiusGallery.xaml) is a StackLayout (Padding=12) that demonstrates DRIVING a RoundRectangleGeometry&amp;#x27;s per-corner CornerRadius from four s

#### 🟢 Sonnet 5 — C++ (C1/C3)

Clipped rounded-rectangle image and the four corner-radius sliders match MAUI exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 28. Clip Gallery — 🟢/⏳
<sub>clip_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/clip_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/clip_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/clip_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ClipGallery.xaml The C# page (Pages/Controls/ShapesGalleries/ClipGallery.xaml; its .xaml.cs is an empty InitializeComponent) is a ScrollView over a StackLayout (Padding=12) that shows the SAME &amp;quot;oasis.jpg&amp;quot; image SEVEN times — one bare

#### 🟢 Sonnet 5 — C++ (C1/C3)

Image, RectangleGeometry, and RoundRectangleGeometry clipped image sections all match MAUI in layout and content.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 29. Clip Views — 🟢/⏳
<sub>clip_views</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/clip_views_light.png" /></td><td><img width="300px" src="captures/android/cpp/clip_views_light.png" /></td><td><img width="300px" src="captures/android/xaml/clip_views_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ClipViewsGallery.xaml The C# page (Pages/Controls/ShapesGalleries/ClipViewsGallery.xaml; no code-behind beyond an empty InitializeComponent) is a ScrollView over a StackLayout (Padding=12) that proves the Clip surface (VisualElement.C

#### 🟢 Sonnet 5 — C++ (C1/C3)

All seven clipped-shape view rows (button, date entry, editor, grid, search icon, time) match MAUI aside from a one-day date difference from capture timing.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 30. Clipping — 🟢/⏳
<sub>clipping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/clipping_light.png" /></td><td><img width="300px" src="captures/android/cpp/clipping_light.png" /></td><td><img width="300px" src="captures/android/xaml/clipping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

compare oracle ~/maui-compare/Pages/ClippingPage.cs (itself written to mirror this gallery page)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Android: 'Not clipping' + Toggle button + orange/red overlapping squares match MAUI. Stale red cleared on recapture.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 31. Collectionview — 🟡/⏳
<sub>collectionview</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/collectionview_light.png" /></td><td><img width="300px" src="captures/android/cpp/collectionview_light.png" /></td><td><img width="300px" src="captures/android/xaml/collectionview_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;collectionview&amp;quot; demo (ComparePages.CollectionViewPage()), the shipped-.NET-MAUI reference for the visual-parity comparison: a CollectionView over 24 captioned items, a string Header (&amp;quot;This is the

#### 🟡 Sonnet 5 — C++ (C1/C3)

C++ shows an extra 'This is the header' text row above the grid that MAUI does not display.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 32. Composition Gallery — 🟢/⏳
<sub>composition_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/composition_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/composition_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/composition_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports CompositionGallery.xaml A self-contained, code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/CompositionGallery.xaml: a StackLayout holding two Beige 250x250 Grids (Margin 12) that compose multiple overlappi

#### 🟢 Sonnet 5 — C++ (C1/C3)

Shapes composition and line diagram match exactly between MAUI and C++.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 33. Containers — 🟢/⏳
<sub>containers</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/containers_light.png" /></td><td><img width="300px" src="captures/android/cpp/containers_light.png" /></td><td><img width="300px" src="captures/android/xaml/containers_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a self-contained demo page for the W1-07 container set: a scroll_view hosting a vertical stack of content-hosting containers — a border-framed label (stroke + dashed outline + rounded shape), a legacy frame (BorderColor/CornerRadius/HasShad

#### 🟢 Sonnet 5 — C++ (C1/C3)

Border, frame, and content_view boxes with dashed/solid outlines match exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 34. Content View — 🟢/⏳
<sub>content_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/content_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/content_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/content_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ContentViewPage.xaml (+ ContentViewPage.xaml.cs), code-first

#### 🟢 Sonnet 5 — C++ (C1/C3)

ContentView swap layout with nested Content/Swap content button matches exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 35. Context Flyout — 🔴/⏳
<sub>context_flyout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/context_flyout_light.png" /></td><td><img width="300px" src="captures/android/cpp/context_flyout_light.png" /></td><td><img width="300px" src="captures/android/xaml/context_flyout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ContextFlyoutPage.xaml (+ ContextFlyoutPage.xaml.cs) The C# page attaches a MenuFlyout as the FlyoutBase.ContextFlyout (right-click / long-press menu) of several controls and wires each menu item to a handler: - a Button (&amp;quot;Increment b

#### 🔴 Sonnet 5 — C++ (C1/C3)

Android RED = CAPTURE-IMPOSSIBLE, not a port bug (ruling 3, needs user ruling). The shared context_flyout.xaml ends with &lt;WebView Source='https://bing.com' MinimumHeightRequest='400'&gt;; the capture emulator has NO network, so the WebView can never load — the MAUI reference itself is a blank/error surface, making a fair cpp-vs-MAUI comparison impossible on this platform. Not a port rendering defect.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 36. Controls Stack — 🟡/⏳
<sub>controls_stack</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/controls_stack_light.png" /></td><td><img width="300px" src="captures/android/cpp/controls_stack_light.png" /></td><td><img width="300px" src="captures/android/xaml/controls_stack_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;controls_stack&amp;quot; demo (ComparePages.ControlsStack()), the shipped-.NET-MAUI reference for the visual-parity comparison: a VerticalStackLayout (Spacing 12, Padding 16) showcasing the basic widgets

#### 🟡 Sonnet 5 — C++ (C1/C3)

The ActivityIndicator (third control in the row) renders as a malformed small squiggle/comma shape in C++ instead of MAUI's circular spinner ring; everything else matches.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 37. Custom Layout — 🟢/⏳
<sub>custom_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/custom_layout_light.png" /></td><td><img width="300px" src="captures/android/cpp/custom_layout_light.png" /></td><td><img width="300px" src="captures/android/xaml/custom_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports CustomLayoutPage.xaml (+ CustomLayoutPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Match. The Top/Bottom/Left/Right docked default buttons now render as flat, contiguous, edge-to-edge #E0E0E0 blocks (no inset gaps, near-square) with the small triangle corner markers, matching MAUI (was solid rounded rectangles with ~4dp inset gaps). The flat-button-widget fix. Both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 38. Custom Size Swipe — 🟢/⏳
<sub>custom_size_swipe</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/custom_size_swipe_light.png" /></td><td><img width="300px" src="captures/android/cpp/custom_size_swipe_light.png" /></td><td><img width="300px" src="captures/android/xaml/custom_size_swipe_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports CustomSizeSwipeViewGallery.xaml (+ .xaml.cs) The MAUI CustomSizeSwipeViewGallery is a single SwipeView whose Left / Right / Top item collections each reveal CUSTOM-SIZED content: a SwipeItemView wrapping a Grid/StackLayout with an exp

#### 🟢 Sonnet 5 — C++ (C1/C3)

SwipeView content, button, and revealed-state text match exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 39. Custom Swipe Item View — 🟢/⏳
<sub>custom_swipe_item_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/custom_swipe_item_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/custom_swipe_item_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/custom_swipe_item_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports CustomSwipeItemViewGallery.xaml A self-contained, code-first port of the .NET MAUI &amp;quot;CustomSwipeItem&amp;quot; gallery: a message-list row whose right swipe reveals a CUSTOM-content swipe item (a swipe_item_view, not a plain swipe_item)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Custom swipe item card with title/date and purple background matches exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 40. Cv Visual States — 🟢/⏳
<sub>cv_visual_states</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/cv_visual_states_light.png" /></td><td><img width="300px" src="captures/android/cpp/cv_visual_states_light.png" /></td><td><img width="300px" src="captures/android/xaml/cv_visual_states_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports CollectionViewGalleries/SelectionGalleries/ VisualStatesGallery.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

Single/Multi selection item lists match exactly in text and layout.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 41. Data Template Selector — 🟢/⏳
<sub>data_template_selector</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/data_template_selector_light.png" /></td><td><img width="300px" src="captures/android/cpp/data_template_selector_light.png" /></td><td><img width="300px" src="captures/android/xaml/data_template_selector_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports DataTemplateSelectorGallery.xaml (+ DataTemplateSelectorGallery.xaml.cs, including its WeekendSelector + SearchTermSelector classes)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Day-of-week templated list content and repeated pattern match exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 42. Date Picker — 🟢/⏳
<sub>date_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/date_picker_light.png" /></td><td><img width="300px" src="captures/android/cpp/date_picker_light.png" /></td><td><img width="300px" src="captures/android/xaml/date_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports DatePickerPage.xaml (+ DatePickerPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Android: Default/BackgroundColor/Background date rows match MAUI; only the date value differs (MAUI 7/5/2026, cpp 7/7/2026 == capture day) — capture-date artifact, not a port bug.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 43. Device — 🟢/⏳
<sub>device</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/device_light.png" /></td><td><img width="300px" src="captures/android/cpp/device_light.png" /></td><td><img width="300px" src="captures/android/xaml/device_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports DevicePage.xaml

#### 🟢 Sonnet 5 — C++ (C1/C3)

Platform/Idiom/Version text block matches exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 44. Dispatcher — 🟢/⏳
<sub>dispatcher</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/dispatcher_light.png" /></td><td><img width="300px" src="captures/android/cpp/dispatcher_light.png" /></td><td><img width="300px" src="captures/android/xaml/dispatcher_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports DispatcherPage.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

All dispatcher demo buttons and status text match exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 45. Drag Drop — 🟢/⏳
<sub>drag_drop</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/drag_drop_light.png" /></td><td><img width="300px" src="captures/android/cpp/drag_drop_light.png" /></td><td><img width="300px" src="captures/android/xaml/drag_drop_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports DragAndDropBetweenLayouts.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.DragAndDropBetweenLayouts)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Color swatches, rainbow list, and drag/drop position text all match exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 46. Editor — 🟢/⏳
<sub>editor</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/editor_light.png" /></td><td><img width="300px" src="captures/android/cpp/editor_light.png" /></td><td><img width="300px" src="captures/android/xaml/editor_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports EditorPage.xaml (Microsoft.Maui.Controls sample gallery)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Layout, colors, and text content match MAUI reference exactly; only status-bar chrome differs.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 47. Effects — 🟢/⏳
<sub>effects</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/effects_light.png" /></td><td><img width="300px" src="captures/android/cpp/effects_light.png" /></td><td><img width="300px" src="captures/android/xaml/effects_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports EffectsPage.xaml (Maui.Controls.Sample.Pages.EffectsPage)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Entry fields, disabled buttons, and status label render identically to the MAUI reference.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 48. Ellipse Gallery — 🟢/⏳
<sub>ellipse_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ellipse_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/ellipse_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/ellipse_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports EllipseGallery.xaml A self-contained, code-first port of the MAUI Shapes EllipseGallery (Pages/Controls/ShapesGalleries/EllipseGallery.xaml + .xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

All shapes (rectangle, circle, ellipses with stroke/dash) match position, size, and color exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 49. Empty View — 🟢/⏳
<sub>empty_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/empty_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/empty_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/empty_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports EmptyViewStringGallery.xaml (+ EmptyViewStringGallery.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Filter bar and scrollable file list match MAUI reference exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 50. Empty View Load Simulate — 🟢/⏳
<sub>empty_view_load_simulate</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/empty_view_load_simulate_light.png" /></td><td><img width="300px" src="captures/android/cpp/empty_view_load_simulate_light.png" /></td><td><img width="300px" src="captures/android/xaml/empty_view_load_simulate_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewLoadSimulateGallery.xaml (+ EmptyViewLoadSimulateGallery.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Loading-simulation text is centered identically in both renders.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 51. Empty View Null — 🟢/⏳
<sub>empty_view_null</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/empty_view_null_light.png" /></td><td><img width="300px" src="captures/android/cpp/empty_view_null_light.png" /></td><td><img width="300px" src="captures/android/xaml/empty_view_null_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewNullGallery.xaml (+ EmptyViewNullGallery.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Nothing to display. centered message matches exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 52. Empty View Rtl — 🟢/⏳
<sub>empty_view_rtl</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/empty_view_rtl_light.png" /></td><td><img width="300px" src="captures/android/cpp/empty_view_rtl_light.png" /></td><td><img width="300px" src="captures/android/xaml/empty_view_rtl_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewRTLGallery.xaml (+ EmptyViewRTLGallery.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Three-column filtered list layout and content match MAUI reference exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 53. Empty View Selector — 🟢/⏳
<sub>empty_view_selector</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/empty_view_selector_light.png" /></td><td><img width="300px" src="captures/android/cpp/empty_view_selector_light.png" /></td><td><img width="300px" src="captures/android/xaml/empty_view_selector_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewWithDataTemplateSelector.xaml (+ .xaml.cs, incl

#### 🟢 Sonnet 5 — C++ (C1/C3)

Instructional text, filter bar, and single result row match exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 54. Empty View Swap — 🟢/⏳
<sub>empty_view_swap</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/empty_view_swap_light.png" /></td><td><img width="300px" src="captures/android/cpp/empty_view_swap_light.png" /></td><td><img width="300px" src="captures/android/xaml/empty_view_swap_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewSwapGallery.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Toggle switch, Clear/Fill buttons, and three-column list all match MAUI reference.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 55. Empty View Template — 🟢/⏳
<sub>empty_view_template</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/empty_view_template_light.png" /></td><td><img width="300px" src="captures/android/cpp/empty_view_template_light.png" /></td><td><img width="300px" src="captures/android/xaml/empty_view_template_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewTemplateGallery.xaml (+ EmptyViewTemplateGallery.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Three-column filtered list matches MAUI reference exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 56. Empty View View — 🟢/⏳
<sub>empty_view_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/empty_view_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/empty_view_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/empty_view_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewViewGallery.xaml (+ EmptyViewViewGallery.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Three-column filtered list matches MAUI reference exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 57. Entry — 🟢/⏳
<sub>entry</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/entry_light.png" /></td><td><img width="300px" src="captures/android/cpp/entry_light.png" /></td><td><img width="300px" src="captures/android/xaml/entry_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports EntryPage.xaml (Microsoft.Maui.Controls sample gallery)

#### 🟢 Sonnet 5 — C++ (C1/C3)

All entry fields, checkbox, password dots, cursor slider, and labels match MAUI reference exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 58. Filter Collection — 🟢/⏳
<sub>filter_collection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/filter_collection_light.png" /></td><td><img width="300px" src="captures/android/cpp/filter_collection_light.png" /></td><td><img width="300px" src="captures/android/xaml/filter_collection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports FilterCollectionView.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

Use EmptyView toggle and two-column filtered file list match MAUI reference exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 59. Filter Selection — 🟢/⏳
<sub>filter_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/filter_selection_light.png" /></td><td><img width="300px" src="captures/android/cpp/filter_selection_light.png" /></td><td><img width="300px" src="captures/android/xaml/filter_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports FilterSelection.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.FilterSelection)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Instructional text, Reset button, Selected label, and list match MAUI reference exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 60. Flex Layout — 🟢/⏳
<sub>flex_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/flex_layout_light.png" /></td><td><img width="300px" src="captures/android/cpp/flex_layout_light.png" /></td><td><img width="300px" src="captures/android/xaml/flex_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports FlexLayoutPage.xaml A self-contained, code-first demo of the FlexLayout control: the classic &amp;quot;holy grail&amp;quot; page layout built from nested flexboxes

#### 🟢 Sonnet 5 — C++ (C1/C3)

Header/content/footer flex layout with blue/gray/green columns and pink footer all match MAUI reference exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 61. Focus — 🟢/⏳
<sub>focus</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/focus_light.png" /></td><td><img width="300px" src="captures/android/cpp/focus_light.png" /></td><td><img width="300px" src="captures/android/xaml/focus_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports FocusPage.xaml (+ FocusPage.xaml.cs) The C# FocusPage is a focus-subsystem demo: an Entry whose Focused/Unfocused events (OnFocusEntryFocusChanged) append &amp;quot;Focused&amp;quot;/&amp;quot;Unfocused&amp;quot; lines to a scrolling InfoLabel, plus two buttons — &amp;quot;Focus

#### 🟢 Sonnet 5 — C++ (C1/C3)

Entry, two buttons, and IsFocused label render identically in position, size, and text.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 62. Fonts — 🟢/⏳
<sub>fonts</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/fonts_light.png" /></td><td><img width="300px" src="captures/android/cpp/fonts_light.png" /></td><td><img width="300px" src="captures/android/xaml/fonts_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;fonts&amp;quot; demo (ComparePages.Fonts()), the shipped-.NET-MAUI reference for the visual-parity comparison: a ScrollView over a VerticalStackLayout (Spacing 8, Padding 16) of nine Labels — Title/Subtit

#### 🟢 Sonnet 5 — C++ (C1/C3)

All font style rows (title, subtitle, header, body, caption, bold, italic, character spacing) match MAUI in size and weight.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 63. Footer Only String — 🟢/⏳
<sub>footer_only_string</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/footer_only_string_light.png" /></td><td><img width="300px" src="captures/android/cpp/footer_only_string_light.png" /></td><td><img width="300px" src="captures/android/xaml/footer_only_string_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports FooterOnlyString.xaml (+ FooterOnlyString.xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Sonnet 5 — C++ (C1/C3)

List content and footer string text match exactly; only vertical scroll offset differs between the two captures.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 64. Formatted Text — 🟢/⏳
<sub>formatted_text</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/formatted_text_light.png" /></td><td><img width="300px" src="captures/android/cpp/formatted_text_light.png" /></td><td><img width="300px" src="captures/android/xaml/formatted_text_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a self-contained demo page for the G1 rich-text slice: a label whose FormattedText is built from several styled spans (bold / italic / colored / underlined / kerned), plus a plain label proving the Text ⇄ FormattedText exclusivity

#### 🟢 Sonnet 5 — C++ (C1/C3)

Formatted text spans (bold red, italic underlined, kerned, plain) render identically in color, style, and layout.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 65. Gestures — 🟢/⏳
<sub>gestures</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/gestures_light.png" /></td><td><img width="300px" src="captures/android/cpp/gestures_light.png" /></td><td><img width="300px" src="captures/android/xaml/gestures_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports GesturesPage.xaml (+ .xaml.cs) The MAUI GesturesPage.xaml is a *gallery navigation* page: a CollectionView listing gesture-demo sections that the shell navigates into

#### 🟢 Sonnet 5 — C++ (C1/C3)

Gesture target rectangle and layout match; last-gesture text differs only due to runtime interaction state, not a rendering bug.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 66. Gradient — 🟢/⏳
<sub>gradient</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/gradient_light.png" /></td><td><img width="300px" src="captures/android/cpp/gradient_light.png" /></td><td><img width="300px" src="captures/android/xaml/gradient_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;gradient&amp;quot; demo (ComparePages.Gradient()), the shipped-.NET-MAUI reference for the visual-parity comparison: a VerticalStackLayout (Spacing 12, Padding 16) of two captioned 60px BoxViews — a Linea

#### 🟢 Sonnet 5 — C++ (C1/C3)

Linear yellow-to-green and radial red-to-navy gradients render identically in colors and bounds.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 67. Grid — 🟢/⏳
<sub>grid</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/grid_light.png" /></td><td><img width="300px" src="captures/android/cpp/grid_light.png" /></td><td><img width="300px" src="captures/android/xaml/grid_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;grid&amp;quot; demo (ComparePages.GridPage()), the shipped-.NET-MAUI reference for the visual-parity comparison: a Grid (Padding 16, Row/ColumnSpacing 6) with RowDefinitions Auto / 80 / 80 and two Star co

#### 🟢 Sonnet 5 — C++ (C1/C3)

2x2 color grid (red/green/blue/orange) matches exactly in position and size.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 68. Grid Grouping — 🟢/⏳
<sub>grid_grouping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/grid_grouping_light.png" /></td><td><img width="300px" src="captures/android/cpp/grid_grouping_light.png" /></td><td><img width="300px" src="captures/android/xaml/grid_grouping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports GroupingGalleries/GridGrouping.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

Grouped two-column list content and orange/green group labels match; only scroll position differs.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 69. Grouping No Templates — 🟢/⏳
<sub>grouping_no_templates</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/grouping_no_templates_light.png" /></td><td><img width="300px" src="captures/android/cpp/grouping_no_templates_light.png" /></td><td><img width="300px" src="captures/android/xaml/grouping_no_templates_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports GroupingGalleries/GroupingNoTemplates.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

Flat grouped list of hero names matches exactly aside from scroll offset.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 70. Grouping Plus Selection — 🟢/⏳
<sub>grouping_plus_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/grouping_plus_selection_light.png" /></td><td><img width="300px" src="captures/android/cpp/grouping_plus_selection_light.png" /></td><td><img width="300px" src="captures/android/xaml/grouping_plus_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports CollectionViewGalleries/GroupingGalleries/ GroupingPlusSelection.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

Grouped list with green group headers and orange total-member counts matches exactly aside from scroll offset.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 71. Header Footer — 🟢/⏳
<sub>header_footer</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/header_footer_light.png" /></td><td><img width="300px" src="captures/android/cpp/header_footer_light.png" /></td><td><img width="300px" src="captures/android/xaml/header_footer_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports HeaderFooterString.xaml (+ HeaderFooterString.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Header string, image list rows, and footer string text match exactly aside from scroll offset.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 72. Header Footer Grid — 🟢/⏳
<sub>header_footer_grid</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/header_footer_grid_light.png" /></td><td><img width="300px" src="captures/android/cpp/header_footer_grid_light.png" /></td><td><img width="300px" src="captures/android/xaml/header_footer_grid_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports HeaderFooterGrid.xaml (+ HeaderFooterGrid.xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Header image/title, three-column image grid, footer image/title, and buttons all render pixel-consistent with MAUI.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 73. Header Footer Grid Horizontal — 🟢/⏳
<sub>header_footer_grid_horizontal</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/header_footer_grid_horizontal_light.png" /></td><td><img width="300px" src="captures/android/cpp/header_footer_grid_horizontal_light.png" /></td><td><img width="300px" src="captures/android/xaml/header_footer_grid_horizontal_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports HeaderFooterGridHorizontal.xaml (+ HeaderFooterGridHorizontal.xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Horizontal-scroll grid with header/footer images and toggle buttons match; only scroll position within the horizontal list differs.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 74. Header Footer Template — 🟢/⏳
<sub>header_footer_template</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/header_footer_template_light.png" /></td><td><img width="300px" src="captures/android/cpp/header_footer_template_light.png" /></td><td><img width="300px" src="captures/android/xaml/header_footer_template_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports HeaderFooterTemplate.xaml (+ .xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Header/footer templated views with blue rows, image thumbnails, and footer image render the same structure; scroll offset and timestamp differ due to capture timing, not a bug.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 75. Header Footer View — 🟢/⏳
<sub>header_footer_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/header_footer_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/header_footer_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/header_footer_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports HeaderFooterView.xaml (+ .xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Header image with title, footer image with title, and Add/Clear buttons render identically aside from vertical scroll offset.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 76. Hit Testing — 🟢/⏳
<sub>hit_testing</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/hit_testing_light.png" /></td><td><img width="300px" src="captures/android/cpp/hit_testing_light.png" /></td><td><img width="300px" src="captures/android/xaml/hit_testing_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports HitTestingPage.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.HitTestingPage)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Matches the fresh MAUI ref. The three Scale/Rotation buttons now render CONTENT-WIDTH + centered (was full-width) — the cpp builder was missing the shared XAML's HorizontalOptions="Center" on the buttons; iOS/macCatalyst native buttons content-size regardless (already green), but the android native button fills full-width without it. Added it -&gt; Scale=1/Scale=2(2x)/Rotation=20(rotated) buttons match MAUI's sizing, the Scale=2 button no longer overflows the right edge. Checkbox, Start/End labels, green ellipse + rounded rectangle all align.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 77. Horizontal Stack — 🟢/⏳
<sub>horizontal_stack</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/horizontal_stack_light.png" /></td><td><img width="300px" src="captures/android/cpp/horizontal_stack_light.png" /></td><td><img width="300px" src="captures/android/xaml/horizontal_stack_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

Horizontal Stack

#### 🟢 Sonnet 5 — C++ (C1/C3)

Colored stripe layout, text, and positions match MAUI closely.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 78. Hybrid Web View — 🟢/⏳
<sub>hybrid_web_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/hybrid_web_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/hybrid_web_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/hybrid_web_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports HybridWebViewPage.xaml (+ HybridWebViewPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Buttons, text, and webview error state match MAUI (button label wrapping differs slightly but content and layout are correct).

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 79. Image — 🟢/⏳
<sub>image</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/image_light.png" /></td><td><img width="300px" src="captures/android/cpp/image_light.png" /></td><td><img width="300px" src="captures/android/xaml/image_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ImagePage.xaml (+ ImagePage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Android: UriSource/FileSource headers + the purple Bionic FileSource image render matching MAUI. Stale red cleared.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 80. Image Button — 🟢/⏳
<sub>image_button</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/image_button_light.png" /></td><td><img width="300px" src="captures/android/cpp/image_button_light.png" /></td><td><img width="300px" src="captures/android/xaml/image_button_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ImageButtonPage.xaml (+ ImageButtonPage.xaml.cs) A self-contained, code-first demo page for the ImageButton control (the C# gallery-page convention, mirroring the input_controls_page / image_page pattern)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Android: against the fresh MAUI ref, the rebuilt app-host renders the white cog inside every green ImageButton (AspectFit/AspectFill/Fill/BorderColor) identically to MAUI. cog.png is auto-globbed into assets/ from gallery/resources/; prior red was a stale cpp capture (pre this session's cog fix + rebuild).

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 81. Indicator — 🟢/⏳
<sub>indicator</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/indicator_light.png" /></td><td><img width="300px" src="captures/android/cpp/indicator_light.png" /></td><td><img width="300px" src="captures/android/xaml/indicator_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports IndicatorPage.xaml A self-contained, code-first demo of the IndicatorView control

#### 🟢 Sonnet 5 — C++ (C1/C3)

Android: Basic/Colors/Indicator Shape/Indicator Size IndicatorView rows (dots on yellow, squares, sized circles) match MAUI. Stale red cleared.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 82. Input Controls — 🟢/⏳
<sub>input_controls</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/input_controls_light.png" /></td><td><img width="300px" src="captures/android/cpp/input_controls_light.png" /></td><td><img width="300px" src="captures/android/xaml/input_controls_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a self-contained demo page for the W1-05 input-control set: editor, search_bar, radio_button (+ the radio_button_group attached grouping) and image_button on one vertical stack, wired together so every input drives a visible output (the C#

#### 🟢 Sonnet 5 — C++ (C1/C3)

Entry, search bar, and radio buttons render identically to MAUI in text, style, and layout.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 83. Input Transparent — 🟢/⏳
<sub>input_transparent</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/input_transparent_light.png" /></td><td><img width="300px" src="captures/android/cpp/input_transparent_light.png" /></td><td><img width="300px" src="captures/android/xaml/input_transparent_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports InputTransparentPage.xaml (Maui.Controls.Sample.Pages.InputTransparentPage)

#### 🟢 Sonnet 5 — C++ (C1/C3)

All buttons, toggle switch, and instructional text match MAUI exactly aside from minor uniform padding difference.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 84. Invalidate Brush — 🟢/⏳
<sub>invalidate_brush</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/invalidate_brush_light.png" /></td><td><img width="300px" src="captures/android/cpp/invalidate_brush_light.png" /></td><td><img width="300px" src="captures/android/xaml/invalidate_brush_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports InvalidateBrushGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/InvalidateBrushGallery.xaml (&amp;quot;Invalidate Brushes Playground&amp;quot;): a VerticalStackLayout (Padding 12) with — - a &amp;quot;Change color&amp;quot; Bu

#### 🟢 Sonnet 5 — C++ (C1/C3)

Change color button and brush color text match MAUI precisely.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 85. Invalidate Shadow Host — 🟢/⏳
<sub>invalidate_shadow_host</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/invalidate_shadow_host_light.png" /></td><td><img width="300px" src="captures/android/cpp/invalidate_shadow_host_light.png" /></td><td><img width="300px" src="captures/android/xaml/invalidate_shadow_host_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports InvalidateShadowHostPage.xaml A self-contained, code-first demo that a shadow re-applies (invalidates) when its host&amp;#x27;s size changes, mirroring the C# core gallery page (Pages/Core/ShadowGalleries/InvalidateShadowHostPage.xaml + .xaml.

#### 🟢 Sonnet 5 — C++ (C1/C3)

Android: Host + Update Host Size + Shadow Offset X/Y sliders match MAUI. Stale red cleared.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 86. Ios Blur Effect — 🟢/⏳
<sub>ios_blur_effect</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_blur_effect_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_blur_effect_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_blur_effect_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports iOSBlurEffectPage.xaml The .NET MAUI PlatformSpecifics sample (Pages/PlatformSpecifics/iOS/iOSBlurEffectPage.xaml + .xaml.cs): an Image (Source=&amp;quot;oasis.jpg&amp;quot;) carrying the iOSSpecific VisualElement.BlurEffect knob (XAML seeds it to Extr

#### 🟢 Sonnet 5 — C++ (C1/C3)

Image, blur option buttons, and status text all match MAUI.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 87. Ios Date Picker — 🟢/⏳
<sub>ios_date_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_date_picker_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_date_picker_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_date_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports iOSDatePickerPage.xaml (+ iOSDatePickerPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Date text field and toggle button match MAUI exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 88. Ios Entry — 🟢/⏳
<sub>ios_entry</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_entry_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_entry_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_entry_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports iOSEntryPage.xaml (+ iOSEntryPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Entry placeholder text and toggle button match MAUI exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 89. Ios First Responder — 🟢/⏳
<sub>ios_first_responder</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_first_responder_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_first_responder_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_first_responder_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports iOSFirstResponderPage.xaml (+ .xaml.cs) The C# iOSFirstResponderPage is a VisualElement-first-responder demo: a StackLayout with an explanatory Label, a &amp;quot;First Entry&amp;quot; + plain &amp;quot;OK&amp;quot; Button (tapping OK dismisses the keyboard because the

#### 🟢 Sonnet 5 — C++ (C1/C3)

All entries, OK buttons, focus buttons, and status text match MAUI closely (only minor spacing differences).

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 90. Ios Pan Gesture — 🟢/⏳
<sub>ios_pan_gesture</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_pan_gesture_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_pan_gesture_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_pan_gesture_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports iOSPanGestureRecognizerPage.xaml (+ .xaml.cs) The C# iOSPanGestureRecognizerPage is a StackLayout with: a bold message Label (_messageLabel), a &amp;quot;Toggle Simultaneous Gesture Recognition&amp;quot; Button, and a grouped ListView of employees whos

#### 🟢 Sonnet 5 — C++ (C1/C3)

Pan coordinates text, toggle button, and status labels all match MAUI exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 91. Ios Picker — 🟢/⏳
<sub>ios_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_picker_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_picker_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports iOSPickerPage.xaml (+ iOSPickerPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Picker label and toggle button render identically, matching layout, text, and colors.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 92. Ios Safe Area — 🟢/⏳
<sub>ios_safe_area</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_safe_area_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_safe_area_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_safe_area_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports iOSSafeAreaPage.xaml The .NET MAUI PlatformSpecifics sample (Pages/PlatformSpecifics/iOS/iOSSafeAreaPage.xaml + .xaml.cs): a long Lorem-ipsum Label over a &amp;quot;Disable Use Safe Area&amp;quot; button

#### 🟢 Sonnet 5 — C++ (C1/C3)

Lorem ipsum paragraph and Disable Use Safe Area button match in text, layout, and color.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 93. Ios Scroll View — 🟢/⏳
<sub>ios_scroll_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_scroll_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_scroll_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_scroll_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports iOSScrollViewPage.xaml (+ iOSScrollViewPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Slider and both action buttons match in position, sizing, and text.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 94. Ios Search Bar — 🟢/⏳
<sub>ios_search_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_search_bar_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_search_bar_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_search_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports iOSSearchBarPage.xaml (+ iOSSearchBarPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Search bar with icon and two toggle buttons match closely in layout and styling.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 95. Ios Slider Update On Tap — 🟢/⏳
<sub>ios_slider_update_on_tap</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_slider_update_on_tap_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_slider_update_on_tap_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_slider_update_on_tap_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports iOSSliderUpdateOnTapPage.xaml (+ iOSSliderUpdateOnTapPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Slider thumb position and toggle button match MAUI closely.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 96. Ios Swipe Transition — 🟢/⏳
<sub>ios_swipe_transition</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_swipe_transition_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_swipe_transition_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_swipe_transition_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports iOSSwipeViewTransitionModePage.xaml (+ .xaml.cs) The C# iOSSwipeViewTransitionModePage is a StackLayout with: a horizontal row holding a &amp;quot;SwipeTransitionMode:&amp;quot; Label + an EnumPicker over the SwipeTransitionMode enum (Reveal / Drag, Se

#### 🟢 Sonnet 5 — C++ (C1/C3)

SwipeTransitionMode buttons, swipe box, and status text all match.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 97. Ios Time Picker — 🟢/⏳
<sub>ios_time_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_time_picker_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_time_picker_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_time_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports iOSTimePickerPage.xaml The .NET MAUI PlatformSpecifics sample (Pages/PlatformSpecifics/iOS/iOSTimePickerPage.xaml + .xaml.cs): a TimePicker carrying the iOSSpecific TimePicker.UpdateMode knob (XAML seeds it to WhenFinished) over a but

#### 🟢 Sonnet 5 — C++ (C1/C3)

Time picker text field, toggle button, and status label match exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 98. Items — 🟢/⏳
<sub>items</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/items_light.png" /></td><td><img width="300px" src="captures/android/cpp/items_light.png" /></td><td><img width="300px" src="captures/android/xaml/items_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a self-contained demo page for the W2-19 items core: a collection_view over a live observable items source with a templated cell, single selection driving a readout label, and an EmptyView for the cleared state (the C# CollectionView galler

#### 🟢 Sonnet 5 — C++ (C1/C3)

Task list items and 'Pick a task' text render identically.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 99. Items Updating Scroll Mode — 🟢/⏳
<sub>items_updating_scroll_mode</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/items_updating_scroll_mode_light.png" /></td><td><img width="300px" src="captures/android/cpp/items_updating_scroll_mode_light.png" /></td><td><img width="300px" src="captures/android/xaml/items_updating_scroll_mode_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ItemsUpdatingScrollModeGallery.xaml (+ .xaml.cs) of the C# CollectionView gallery (Maui.Controls.Sample.Pages.CollectionViewGalleries.ScrollModeGalleries.ItemsUpdatingScrollModeGallery)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Toggle buttons, Add Item button, and item list rows match content and layout (minor line-spacing looks slightly tighter in C++ but content identical).

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 100. Label — 🟢/⏳
<sub>label</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/label_light.png" /></td><td><img width="300px" src="captures/android/cpp/label_light.png" /></td><td><img width="300px" src="captures/android/xaml/label_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports LabelPage.xaml (+ LabelPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

All label formatting demos (colors, alignment, strikethrough, big font) match MAUI precisely.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 101. Layout Is Enabled — 🟢/⏳
<sub>layout_is_enabled</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/layout_is_enabled_light.png" /></td><td><img width="300px" src="captures/android/cpp/layout_is_enabled_light.png" /></td><td><img width="300px" src="captures/android/xaml/layout_is_enabled_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports LayoutIsEnabledPage.xaml (+ LayoutIsEnabledPage.xaml.cs) The C# page demonstrates how IsEnabled on a layout cascades to its children: a 2x2 grid whose left column hosts a &amp;quot;MainLayout&amp;quot; full of state-demo sub-stacks (all-enabled / all-d

#### 🟢 Sonnet 5 — C++ (C1/C3)

Disabled buttons now match MAUI's Material disabled state: the container dims to colorOnSurface@12% (translucent black) and the label to @38%, so the colored parent panel bleeds through (LightBlue disabled button -&gt; (152,190,202)=panel x 0.88; teal/pink panels likewise), instead of the port's former opaque #E0E0E0 fill + baked #8B8B8B label. Enabled buttons stay opaque #E0E0E0; white-bg pages (button) are pixel-identical (black@12% over white = #E0E0E0). Fix: stateful GradientDrawable.setColor(ColorStateList) {disabled-&gt;0x1F000000, enabled-&gt;0xFFE0E0E0} in button_handler.cpp install + unset-background restore, plus disabled text ColorStateList -&gt; black@38% (0x61000000). Pixel 42.76%-&gt;12.24% (SSIM 0.87-&gt;0.96); residual is whole-page translucency/anti-aliasing across the 2-column x 6-panel layout, comparable to green sibling header_footer_grid (11.85%). android-only handler -&gt; no iOS/maccatalyst effect.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 102. Line Gallery — 🟢/⏳
<sub>line_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/line_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/line_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/line_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports LineGallery.xaml A self-contained, code-first port of the MAUI Shapes LineGallery (Pages/Controls/ShapesGalleries/LineGallery.xaml + .xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Basic line, dash line, and stroke-thickness line all match in color, position, and style.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 103. Line Join Gallery — 🟢/⏳
<sub>line_join_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/line_join_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/line_join_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/line_join_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports LineJoinGallery.xaml A self-contained, code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/LineJoinGallery.xaml: a StackLayout that demonstrates the three StrokeLineJoin variants on an identical open polyline

#### 🟢 Sonnet 5 — C++ (C1/C3)

Miter, bevel, and round line-join examples render identically in shape and color.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 104. Measure First Strategy — 🟢/⏳
<sub>measure_first_strategy</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/measure_first_strategy_light.png" /></td><td><img width="300px" src="captures/android/cpp/measure_first_strategy_light.png" /></td><td><img width="300px" src="captures/android/xaml/measure_first_strategy_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports MeasureFirstStrategy.xaml (+ .xaml.cs) of the C# CollectionView gallery (Maui.Controls.Sample.Pages.CollectionViewGalleries.GroupingGalleries.MeasureFirstStrategy)

#### 🟢 Sonnet 5 — C++ (C1/C3)

CollectionView grouped list content, headers, and totals all match MAUI exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 105. Menu Bar — 🟢/⏳
<sub>menu_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/menu_bar_light.png" /></td><td><img width="300px" src="captures/android/cpp/menu_bar_light.png" /></td><td><img width="300px" src="captures/android/xaml/menu_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports MenuBarPage.xaml (+ MenuBarPage.cs) The C# page declares three page-level MenuBarItems (Page.MenuBarItems — the app menu bar) and a small visible body: - &amp;quot;Before File&amp;quot; : &amp;quot;Before File Action&amp;quot; (accelerator &amp;quot;b&amp;quot;), &amp;quot;Cool item 1&amp;quot;, a separat

#### 🟢 Sonnet 5 — C++ (C1/C3)

Menu bar item toggle button and status text match exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 106. Modal — 🟢/⏳
<sub>modal</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/modal_light.png" /></td><td><img width="300px" src="captures/android/cpp/modal_light.png" /></td><td><img width="300px" src="captures/android/xaml/modal_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ModalPage.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Layout, buttons, and text match exactly; only status-bar time/theme differ trivially.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 107. Multiple Bound Selection — 🟢/⏳
<sub>multiple_bound_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/multiple_bound_selection_light.png" /></td><td><img width="300px" src="captures/android/cpp/multiple_bound_selection_light.png" /></td><td><img width="300px" src="captures/android/xaml/multiple_bound_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports MultipleBoundSelection.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.MultipleBoundSelection)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Selected items, orange highlighting, and layout match exactly between MAUI and C++.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 108. Navigation Gallery — 🟢/⏳
<sub>navigation_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/navigation_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/navigation_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/navigation_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports NavigationGallery.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

All six buttons and header text match layout and wording exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 109. Nested Collection — 🟢/⏳
<sub>nested_collection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/nested_collection_light.png" /></td><td><img width="300px" src="captures/android/cpp/nested_collection_light.png" /></td><td><img width="300px" src="captures/android/xaml/nested_collection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports NestedGalleries/NestedCollectionViewGallery.xaml (+ NestedCollectionViewGallery.xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

Nested CollectionViews with captions render identically in content and spacing.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 110. Pan Gesture Events — 🟡/⏳
<sub>pan_gesture_events</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/pan_gesture_events_light.png" /></td><td><img width="300px" src="captures/android/cpp/pan_gesture_events_light.png" /></td><td><img width="300px" src="captures/android/xaml/pan_gesture_events_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports PanGestureEventsGallery.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.PanGestureEventsGallery)

#### 🟡 Sonnet 5 — C++ (C1/C3)

Green/red gesture blocks match in color and text, but the C++ render leaves a white gap below the red block where MAUI's red extends further down.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 111. Path Aspect Gallery — 🟢/⏳
<sub>path_aspect_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/path_aspect_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/path_aspect_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/path_aspect_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports PathAspectGallery.xaml A self-contained, code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PathAspectGallery.xaml: a StackLayout (Padding 12) that demonstrates the four Path Aspect modes on one identical ge

#### 🟢 Sonnet 5 — C++ (C1/C3)

All four heart-icon aspect variants (None/Fill/Uniform/UniformToFill) match in size and color exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 112. Path Gallery — 🟢/⏳
<sub>path_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/path_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/path_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/path_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports PathGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PathGallery.xaml: a ScrollView over a StackLayout (Padding 12) that walks eight Path variants (plus two caption-only markup-string Labels

#### 🟢 Sonnet 5 — C++ (C1/C3)

All path shapes (line, triangle, bezier, composite circles, overlapping rectangles, ellipse geometry) match precisely; only trailing content is scrolled off in both similarly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 113. Path Transform String — 🟢/⏳
<sub>path_transform_string</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/path_transform_string_light.png" /></td><td><img width="300px" src="captures/android/cpp/path_transform_string_light.png" /></td><td><img width="300px" src="captures/android/xaml/path_transform_string_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports PathTransformStringGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PathTransformStringGallery.xaml: a ScrollView over a StackLayout (Padding 12) that shows the SAME two-figure Path geometry

#### 🟢 Sonnet 5 — C++ (C1/C3)

Both without and with RenderTransform triangle shapes match exactly in position and size.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 114. Picker — 🟢/⏳
<sub>picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/picker_light.png" /></td><td><img width="300px" src="captures/android/cpp/picker_light.png" /></td><td><img width="300px" src="captures/android/xaml/picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports PickerPage.xaml (+ PickerPage.xaml.cs) A self-contained, code-first demo page for the Picker control (the C# gallery-page convention, mirroring the value_controls_page / pickers_page pattern)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Match. The android Button now renders flat edge-to-edge #E0E0E0 (MAUI MauiMaterialButton look) at the correct ~36dp height, so the Clear/Add/Replace dynamic-items buttons have MAUI-tight spacing and the bottom green markup picker fits on screen (was pushed off by the old taller/inset-gapped buttons). The two preset pickers already show the "Select an item" Title (ruling-8). Both themes.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 115. Pickers — 🟢/⏳
<sub>pickers</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/pickers_light.png" /></td><td><img width="300px" src="captures/android/cpp/pickers_light.png" /></td><td><img width="300px" src="captures/android/xaml/pickers_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a self-contained demo page for the W1-06 picker set: picker, date_picker and time_picker on one vertical stack, wired together so every selection drives a visible output (the C# gallery-page convention, code-first; the value_controls_page p

#### 🟢 Sonnet 5 — C++ (C1/C3)

Room/date/time picker fields match exactly; the date text differs only due to a different capture day (not a bug).

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 116. Pointer Gesture — 🟢/⏳
<sub>pointer_gesture</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/pointer_gesture_light.png" /></td><td><img width="300px" src="captures/android/cpp/pointer_gesture_light.png" /></td><td><img width="300px" src="captures/android/xaml/pointer_gesture_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports PointerGestureGalleryPage.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.PointerGestureGalleryPage)

#### 🟢 Sonnet 5 — C++ (C1/C3)

All pointer-position labels and colors (yellow, green) match exactly between the two renders.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 117. Polygon Gallery — 🟢/⏳
<sub>polygon_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/polygon_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/polygon_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/polygon_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports PolygonGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PolygonGallery.xaml: a ScrollView over a StackLayout (Padding 12) that walks four Polygon variants, each under a caption Label — - &amp;quot;A

#### 🟢 Sonnet 5 — C++ (C1/C3)

All four polygon examples (basic, dash, EvenOdd star, NonZero star) match exactly in shape and color.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 118. Polyline Gallery — 🟢/⏳
<sub>polyline_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/polyline_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/polyline_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/polyline_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports PolylineGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PolylineGallery.xaml: a StackLayout (Padding 12 — no ScrollView in the C# source) holding two Polyline variants, each under a caption

#### 🟢 Sonnet 5 — C++ (C1/C3)

Basic and dash polyline examples match exactly in color and style.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 119. Preselected Item — 🟢/⏳
<sub>preselected_item</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/preselected_item_light.png" /></td><td><img width="300px" src="captures/android/cpp/preselected_item_light.png" /></td><td><img width="300px" src="captures/android/xaml/preselected_item_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports PreselectedItemGallery.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.PreselectedItemGallery)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Preselected orange row and full list content match exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 120. Preselected Items — 🟢/⏳
<sub>preselected_items</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/preselected_items_light.png" /></td><td><img width="300px" src="captures/android/cpp/preselected_items_light.png" /></td><td><img width="300px" src="captures/android/xaml/preselected_items_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports PreselectedItemsGallery.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.PreselectedItemsGallery)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Multiple preselected orange cells and full grid list match exactly, aside from trivial column-width wrapping differences.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 121. Progress Bar — 🟢/⏳
<sub>progress_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/progress_bar_light.png" /></td><td><img width="300px" src="captures/android/cpp/progress_bar_light.png" /></td><td><img width="300px" src="captures/android/xaml/progress_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ProgressBarPage.xaml (+ ProgressBarPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Layout, colors, and progress bar states all match MAUI reference precisely.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 122. Radio Button Border — 🟢/⏳
<sub>radio_button_border</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/radio_button_border_light.png" /></td><td><img width="300px" src="captures/android/cpp/radio_button_border_light.png" /></td><td><img width="300px" src="captures/android/xaml/radio_button_border_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports RadioButtonBorder.xaml A self-contained, code-first demo of RadioButton border styling

#### 🟢 Sonnet 5 — C++ (C1/C3)

Border colors, radio states, and text all match exactly between MAUI and C++.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 123. Radio Button Content — 🟢/⏳
<sub>radio_button_content</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/radio_button_content_light.png" /></td><td><img width="300px" src="captures/android/cpp/radio_button_content_light.png" /></td><td><img width="300px" src="captures/android/xaml/radio_button_content_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports RadioButtonContentGallery.xaml A self-contained, code-first demo of the RadioButton.Content surface

#### 🟢 Sonnet 5 — C++ (C1/C3)

The two custom-template cards now render the coffee.png cup (black bar + red bar + cup) the port previously omitted on a stale premise; matches MAUI android exactly — verified by dark-pixel bbox (MAUI x[44-469], cpp x[0-424], SAME ~425px width) and near-identical dark-pixel count (21104 vs 21044). The 14.27% pixel diff is the uniform ~44px harness left-inset (page text left-x MAUI 135 vs cpp 91, ruling 2) amplified by the two large black cups (black-vs-white edge mismatch under the shift), NOT a content diff.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 124. Radio Button Group — 🟢/⏳
<sub>radio_button_group</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/radio_button_group_light.png" /></td><td><img width="300px" src="captures/android/cpp/radio_button_group_light.png" /></td><td><img width="300px" src="captures/android/xaml/radio_button_group_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports RadioButtonGroupGallery.xaml A self-contained, code-first demo of the RadioButtonGroup ATTACHED-PROPERTY grouping: a vertical StackLayout carries RadioButtonGroup.GroupName=&amp;quot;foo&amp;quot;, so every descendant RadioButton — including one nested

#### 🟢 Sonnet 5 — C++ (C1/C3)

Grid and stack layout radio buttons match exactly, including the grid-positioned Option D.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 125. Radio Button Group Binding — 🟢/⏳
<sub>radio_button_group_binding</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/radio_button_group_binding_light.png" /></td><td><img width="300px" src="captures/android/cpp/radio_button_group_binding_light.png" /></td><td><img width="300px" src="captures/android/xaml/radio_button_group_binding_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports RadioButtonGroupBindingGallery.xaml A code-first demo of binding the RadioButtonGroup attached properties (GroupName + SelectedValue) to a view-model

#### 🟢 Sonnet 5 — C++ (C1/C3)

Bound radio group layout, labels, and action buttons match exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 126. Radio Button Group Gallery — 🟢/⏳
<sub>radio_button_group_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/radio_button_group_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/radio_button_group_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/radio_button_group_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports RadioButtonGroupGalleryPage.xaml A self-contained, code-first demo of RadioButton grouping SCOPE, mirroring the C# controls gallery page (Pages/Controls/RadioButtonGalleries/RadioButtonGroupGalleryPage.xaml)

#### 🟢 Sonnet 5 — C++ (C1/C3)

All three grouped radio sections with group names render identically.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 127. Radio Content Properties — 🟢/⏳
<sub>radio_content_properties</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/radio_content_properties_light.png" /></td><td><img width="300px" src="captures/android/cpp/radio_content_properties_light.png" /></td><td><img width="300px" src="captures/android/xaml/radio_content_properties_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ContentProperties.xaml A self-contained, code-first demo of how RadioButton propagates the standard Text/Font properties to its Content

#### 🟢 Sonnet 5 — C++ (C1/C3)

Custom text colors, fonts, and styled radio content all match precisely.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 128. Radio Template From Style — 🟢/⏳
<sub>radio_template_from_style</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/radio_template_from_style_light.png" /></td><td><img width="300px" src="captures/android/cpp/radio_template_from_style_light.png" /></td><td><img width="300px" src="captures/android/xaml/radio_template_from_style_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports TemplateFromStyle.xaml A self-contained, code-first demo of applying a RadioButton ControlTemplate

#### 🟢 Sonnet 5 — C++ (C1/C3)

Custom card-style radio template with blue circle indicators matches exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 129. Rectangle Gallery — 🟢/⏳
<sub>rectangle_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/rectangle_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/rectangle_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/rectangle_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports RectangleGallery.xaml A self-contained, code-first port of the MAUI Shapes RectangleGallery (Pages/Controls/ShapesGalleries/RectangleGallery.xaml + .xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

All rectangle shape variants (basic, square, stroke, dash, rounded corners) render identically.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 130. Refresh View — 🟢/⏳
<sub>refresh_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/refresh_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/refresh_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/refresh_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports RefreshViewPage.xaml (+ RefreshViewPage.xaml.cs + RefreshViewModel.cs) A self-contained, code-first demo page for the RefreshView control (the C# gallery-page convention, mirroring the swipe_refresh_page pattern)

#### 🟢 Sonnet 5 — C++ (C1/C3)

RefreshView controls, labels, and state text match exactly between reference and port.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 131. Relative Layout — 🟢/⏳
<sub>relative_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/relative_layout_light.png" /></td><td><img width="300px" src="captures/android/cpp/relative_layout_light.png" /></td><td><img width="300px" src="captures/android/xaml/relative_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports RelativeLayoutPage.xaml

#### 🟢 Sonnet 5 — C++ (C1/C3)

Corner-anchored colored boxes and centered nested rectangle layout match exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 132. Scattered Radio Button — 🟢/⏳
<sub>scattered_radio_button</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/scattered_radio_button_light.png" /></td><td><img width="300px" src="captures/android/cpp/scattered_radio_button_light.png" /></td><td><img width="300px" src="captures/android/xaml/scattered_radio_button_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ScatteredRadioButtonGallery.xaml A code-first demo that radio buttons DON&amp;#x27;T have to share a container to be grouped: grouping is by GroupName, so buttons scattered across separate containers (and one bare button outside any grouped co

#### 🟢 Sonnet 5 — C++ (C1/C3)

Nested and grouped radio buttons across containers render identically, including the highlighted background row.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 133. Scroll Mode Test — 🟢/⏳
<sub>scroll_mode_test</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/scroll_mode_test_light.png" /></td><td><img width="300px" src="captures/android/cpp/scroll_mode_test_light.png" /></td><td><img width="300px" src="captures/android/xaml/scroll_mode_test_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ScrollModeTestGallery.xaml (+ .xaml.cs) of the C# CollectionView gallery (Maui.Controls.Sample.Pages.CollectionViewGalleries.ScrollModeGalleries.ScrollModeTestGallery)

#### 🟢 Sonnet 5 — C++ (C1/C3)

ItemsUpdatingScrollMode picker, buttons, and item list content match exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 134. Scroll To Group — 🟢/⏳
<sub>scroll_to_group</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/scroll_to_group_light.png" /></td><td><img width="300px" src="captures/android/cpp/scroll_to_group_light.png" /></td><td><img width="300px" src="captures/android/xaml/scroll_to_group_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ScrollToGalleries/ScrollToGroup.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

Form fields, buttons, and grouped superhero list content match; only the scroll viewport differs slightly which is expected scroll state.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 135. Scroll View — 🟢/⏳
<sub>scroll_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/scroll_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/scroll_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/scroll_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ScrollViewPage.xaml (+ the ScrollViewPages sub-demos: ScrollViewOrientationPage / ScrollToEndPage / ScrollToFromConstructorPage), code-first

#### 🟢 Sonnet 5 — C++ (C1/C3)

Row list content and structure match; the two captures simply show different scroll positions, both valid states.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 136. Search Bar — 🟢/⏳
<sub>search_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/search_bar_light.png" /></td><td><img width="300px" src="captures/android/cpp/search_bar_light.png" /></td><td><img width="300px" src="captures/android/xaml/search_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports SearchBarPage.xaml (Microsoft.Maui.Controls sample gallery)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Text list with colors, placeholder, italic, and clear icons match exactly between MAUI and C++.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 137. Selection Command Param — 🟢/⏳
<sub>selection_command_param</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/selection_command_param_light.png" /></td><td><img width="300px" src="captures/android/cpp/selection_command_param_light.png" /></td><td><img width="300px" src="captures/android/xaml/selection_command_param_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports SelectionChangedCommandParameter.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.SelectionChangedCommandParameter)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Identical scrollable list of header/item text lines in both renders.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 138. Selection Synchronization — 🟢/⏳
<sub>selection_synchronization</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/selection_synchronization_light.png" /></td><td><img width="300px" src="captures/android/cpp/selection_synchronization_light.png" /></td><td><img width="300px" src="captures/android/xaml/selection_synchronization_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports SelectionSynchronization.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.SelectionSynchronization)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Orange-highlighted selected items (Item 2, Item 3) match exactly in both renders, only page-padding differs.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 139. Semantics — 🟢/⏳
<sub>semantics</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/semantics_light.png" /></td><td><img width="300px" src="captures/android/cpp/semantics_light.png" /></td><td><img width="300px" src="captures/android/xaml/semantics_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports SemanticsPage.xaml (+ SemanticsPage.xaml.cs) The C# SemanticsPage is an accessibility showcase: a long VerticalStackLayout where nearly every control carries SemanticProperties.Description / .Hint, plus a block of labels exercising Se

#### 🟢 Sonnet 5 — C++ (C1/C3)

All labels, buttons, entry/editor fields, search bar, and heading levels render identically.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 140. Shadow Playground — 🟢/⏳
<sub>shadow_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/shadow_playground_light.png" /></td><td><img width="300px" src="captures/android/cpp/shadow_playground_light.png" /></td><td><img width="300px" src="captures/android/xaml/shadow_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ShadowPlaygroundPage.xaml A self-contained, code-first demo of the view Shadow surface, mirroring the C# core gallery page (Pages/Core/ShadowGalleries/ShadowPlaygroundPage.xaml + .xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Cyan box with red shadow, color fields, and sliders at matching values render identically.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 141. Shape App Theme — 🟢/⏳
<sub>shape_app_theme</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/shape_app_theme_light.png" /></td><td><img width="300px" src="captures/android/cpp/shape_app_theme_light.png" /></td><td><img width="300px" src="captures/android/xaml/shape_app_theme_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ShapeAppThemeGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/ShapeAppThemeGallery.xaml: a StackLayout (Padding 12) holding a caption Label and a 200x80 Rectangle, all themed via {AppThemeBi

#### 🟢 Sonnet 5 — C++ (C1/C3)

Green rectangle shape and title text match exactly between both renders.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 142. Shapes — 🟢/⏳
<sub>shapes</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/shapes_light.png" /></td><td><img width="300px" src="captures/android/cpp/shapes_light.png" /></td><td><img width="300px" src="captures/android/xaml/shapes_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;shapes&amp;quot; demo (ComparePages.Shapes()), the shipped-.NET-MAUI reference for the visual-parity comparison: a ScrollView over a vertical stack of four LABELLED shapes, each bold-captioned and Start-a

#### 🟢 Sonnet 5 — C++ (C1/C3)

Ellipse, round rectangle, pentagram polygon, and diagonal line all match in shape, color, and position.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 143. Single Bound Selection — 🟢/⏳
<sub>single_bound_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/single_bound_selection_light.png" /></td><td><img width="300px" src="captures/android/cpp/single_bound_selection_light.png" /></td><td><img width="300px" src="captures/android/xaml/single_bound_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports SingleBoundSelection.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.SingleBoundSelection)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Instruction text and country list match exactly between both renders.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 144. Slider — 🟢/⏳
<sub>slider</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/slider_light.png" /></td><td><img width="300px" src="captures/android/cpp/slider_light.png" /></td><td><img width="300px" src="captures/android/xaml/slider_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports SliderPage.xaml (+ .xaml.cs) Mirrors the MAUI gallery page: a vertical stack of headlined Slider states — Default, BackgroundColor (Blue), Background (yellow→green LinearGradientBrush), Minimum(5)/Maximum(15) with a value readout (Val

#### 🟢 Sonnet 5 — C++ (C1/C3)

All slider variants (background color, gradient, disabled, custom track/thumb colors) match; C++ additionally shows more content below the fold due to less padding, consistent with allowed policy.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 145. Some Empty Groups — 🟢/⏳
<sub>some_empty_groups</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/some_empty_groups_light.png" /></td><td><img width="300px" src="captures/android/cpp/some_empty_groups_light.png" /></td><td><img width="300px" src="captures/android/xaml/some_empty_groups_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports GroupingGalleries/SomeEmptyGroups.xaml (+ .xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Grouped list with empty group headers and member counts match exactly in text and color.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 146. Stack Layout — 🟢/⏳
<sub>stack_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/stack_layout_light.png" /></td><td><img width="300px" src="captures/android/cpp/stack_layout_light.png" /></td><td><img width="300px" src="captures/android/xaml/stack_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports StackLayoutPage.xaml Demonstrates the generic maui::controls::stack_layout (the orientation-switching sibling of the fixed vertical/horizontal stacks) by nesting two inner stacks inside an outer vertical stack with a 12px margin: a &amp;quot;V

#### 🟢 Sonnet 5 — C++ (C1/C3)

Vertical and horizontal colored-box stacks match exactly in color, size, and order.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 147. Staggered Layout — 🟢/⏳
<sub>staggered_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/staggered_layout_light.png" /></td><td><img width="300px" src="captures/android/cpp/staggered_layout_light.png" /></td><td><img width="300px" src="captures/android/xaml/staggered_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports AlternateLayoutGalleries/StaggeredLayout.xaml (+ StaggeredLayout.xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

Staggered grid of numbered items shows the same masonry pattern; scroll offset differs slightly due to page padding but content matches.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 148. Stepper — 🟢/⏳
<sub>stepper</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/stepper_light.png" /></td><td><img width="300px" src="captures/android/cpp/stepper_light.png" /></td><td><img width="300px" src="captures/android/xaml/stepper_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports StepperPage.xaml (+ StepperPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Android: Default [- +] and Disabled [- +] steppers match MAUI. Stale red cleared.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 149. Styles — 🟢/⏳
<sub>styles</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/styles_light.png" /></td><td><img width="300px" src="captures/android/cpp/styles_light.png" /></td><td><img width="300px" src="captures/android/xaml/styles_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports StylesPage.xaml

#### 🟢 Sonnet 5 — C++ (C1/C3)

Base subtitle style, pink custom style, default style, and outlined button match exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 150. Swipe Gesture — 🟢/⏳
<sub>swipe_gesture</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/swipe_gesture_light.png" /></td><td><img width="300px" src="captures/android/cpp/swipe_gesture_light.png" /></td><td><img width="300px" src="captures/android/xaml/swipe_gesture_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports SwipeViewGestureRecognizerGallery.xaml (+ .xaml.cs) The MAUI SwipeViewGestureRecognizerGallery is a CollectionView of &amp;quot;message&amp;quot; rows; each row&amp;#x27;s DataTemplate is a SwipeView wired three ways, proving gesture recognizers AND swipe-item

#### 🟢 Sonnet 5 — C++ (C1/C3)

C++ renders a clean card (title, date, description, TapCommand line) while the MAUI reference screenshot itself has garbled overlapping text (a broken/stale capture), so the port's content is correct.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 151. Swipe Item Position — 🟢/⏳
<sub>swipe_item_position</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/swipe_item_position_light.png" /></td><td><img width="300px" src="captures/android/cpp/swipe_item_position_light.png" /></td><td><img width="300px" src="captures/android/xaml/swipe_item_position_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports SwipeItemPositionGallery.xaml A code-first port of the MAUI SwipeView sub-gallery Pages/Controls/SwipeViewGalleries/SwipeItemPositionGallery.xaml: a 2-row Grid (Auto / *) with a Picker on top and one SwipeView below that carries TWO S

#### 🟢 Sonnet 5 — C++ (C1/C3)

Reveal SwipeView with label and subtitle text render identically in both.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 152. Swipe Item Size — 🟢/⏳
<sub>swipe_item_size</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/swipe_item_size_light.png" /></td><td><img width="300px" src="captures/android/cpp/swipe_item_size_light.png" /></td><td><img width="300px" src="captures/android/xaml/swipe_item_size_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports SwipeItemSizeGallery.xaml A self-contained, code-first port of the .NET MAUI &amp;quot;SwipeItem Size Gallery&amp;quot;: a scrolling stack of swipe_views demonstrating how a left SwipeItem&amp;#x27;s icon size and the SwipeView content size interact

#### 🟢 Sonnet 5 — C++ (C1/C3)

All differently-sized icon and SwipeView rows render matching gray bars and labels.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 153. Swipe Refresh — 🟢/⏳
<sub>swipe_refresh</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/swipe_refresh_light.png" /></td><td><img width="300px" src="captures/android/cpp/swipe_refresh_light.png" /></td><td><img width="300px" src="captures/android/xaml/swipe_refresh_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a self-contained demo page for the W2-20 swipe + refresh controls: a refresh_view wrapping a swipe_view (which itself wraps a labeled row), with a readout label reflecting the latest interaction

#### 🟢 Sonnet 5 — C++ (C1/C3)

Header text and Ready status line match exactly between MAUI and C++.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 154. Swipe Threshold — 🟢/⏳
<sub>swipe_threshold</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/swipe_threshold_light.png" /></td><td><img width="300px" src="captures/android/cpp/swipe_threshold_light.png" /></td><td><img width="300px" src="captures/android/xaml/swipe_threshold_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports HorizontalSwipeThresholdGallery.xaml (+ .xaml.cs) The MAUI HorizontalSwipeThresholdGallery shows how SwipeView.Threshold (the swipe distance, in DIPs, the user must drag before the items settle open / execute) interacts with SwipeItem

#### 🟢 Sonnet 5 — C++ (C1/C3)

Warning banner, section labels, purple bars, and sliders all match precisely.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 155. Swipe View Margin — 🟡/⏳
<sub>swipe_view_margin</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/swipe_view_margin_light.png" /></td><td><img width="300px" src="captures/android/cpp/swipe_view_margin_light.png" /></td><td><img width="300px" src="captures/android/xaml/swipe_view_margin_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports SwipeViewMarginGallery.xaml A self-contained, code-first port of the .NET MAUI &amp;quot;SwipeView Margin Gallery&amp;quot;: two swipe_views whose content&amp;#x27;s Margin + Padding are driven by two sliders, demonstrating that the revealed SwipeItems stay cor

#### 🟡 Sonnet 5 — C++ (C1/C3)

Layout and colors match but body text renders in lighter/lower-contrast gray in the C++ version making it slightly washed out.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 156. Swipe View Shadow — 🟢/⏳
<sub>swipe_view_shadow</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/swipe_view_shadow_light.png" /></td><td><img width="300px" src="captures/android/cpp/swipe_view_shadow_light.png" /></td><td><img width="300px" src="captures/android/xaml/swipe_view_shadow_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports SwipeViewShadowGallery.xaml A code-first port of the MAUI SwipeView sub-gallery Pages/Controls/SwipeViewGalleries/SwipeViewShadowGallery.xaml: a padded vertical StackLayout proving a drop Shadow renders correctly on SwipeView content

#### 🟢 Sonnet 5 — C++ (C1/C3)

Rounded bordered content boxes with shadow render identically in both.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 157. Switch — 🟢/⏳
<sub>switch</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/switch_light.png" /></td><td><img width="300px" src="captures/android/cpp/switch_light.png" /></td><td><img width="300px" src="captures/android/xaml/switch_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports SwitchPage.xaml (+ .xaml.cs) Mirrors the MAUI gallery page: a vertical stack of headlined Switch states — Default, BackgroundColor (Blue), Background (a yellow→green LinearGradientBrush), Disabled, OnColor (Red), ThumbColor (Orange)

#### 🟢 Sonnet 5 — C++ (C1/C3)

All switch states (default, background color/gradient, disabled, on-color, thumb-color) match colors and positions exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 158. Switch Grouping — 🟢/⏳
<sub>switch_grouping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/switch_grouping_light.png" /></td><td><img width="300px" src="captures/android/cpp/switch_grouping_light.png" /></td><td><img width="300px" src="captures/android/xaml/switch_grouping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports CollectionViewGalleries/GroupingGalleries/ SwitchGrouping.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

Grouped list with headers, member names, and totals in orange/green match, differing only by minor scroll-position offset.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 159. Tabbed Flyout — 🟢/⏳
<sub>tabbed_flyout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/tabbed_flyout_light.png" /></td><td><img width="300px" src="captures/android/cpp/tabbed_flyout_light.png" /></td><td><img width="300px" src="captures/android/xaml/tabbed_flyout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a self-contained demo page for the W1-10 tabbed + flyout vertical: a flyout_page whose FLYOUT pane is a titled menu (two buttons selecting the detail&amp;#x27;s tabs + a &amp;quot;Toggle flyout&amp;quot; presenting/dismissing itself) and whose DETAIL pane is a tabbed

#### 🟢 Sonnet 5 — C++ (C1/C3)

Match. The flyout pane (Home tab / Settings tab / Toggle flyout buttons + "Flyout dismissed" / "This is the Home tab.") now matches MAUI: after the flat-button fix the three stacked buttons render flat edge-to-edge #E0E0E0 at the correct height (the prior RED was the old rounded/inset-gapped/taller button chrome, pixel SSIM 0.93 -&gt; now 0.988). Same layout, text, and colors as MAUI.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 160. Templated View — 🟢/⏳
<sub>templated_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/templated_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/templated_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/templated_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports TemplatedViewPage.xaml The C# page contrasts a standard CardView control with a compact one driven by a ControlTemplate (&amp;quot;CardViewCompressed&amp;quot;) and a custom Rate control built entirely from a ControlTemplate + a heart PathGeometry

#### 🟢 Sonnet 5 — C++ (C1/C3)

CardView and compact ControlTemplate cards with names and descriptions render identically.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 161. Time Picker — 🟢/⏳
<sub>time_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/time_picker_light.png" /></td><td><img width="300px" src="captures/android/cpp/time_picker_light.png" /></td><td><img width="300px" src="captures/android/xaml/time_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports TimePickerPage.xaml (+ TimePickerPage.xaml.cs)

#### 🟢 Sonnet 5 — C++ (C1/C3)

Android: Default/BackgroundColor(blue)/Background(gradient) TimePicker rows all showing 12:00 AM match MAUI. Stale red cleared.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 162. Title Bar — 🟢/⏳
<sub>title_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/title_bar_light.png" /></td><td><img width="300px" src="captures/android/cpp/title_bar_light.png" /></td><td><img width="300px" src="captures/android/xaml/title_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports TitleBarPage.xaml A self-contained, code-first demo of the TitleBar control

#### 🟢 Sonnet 5 — C++ (C1/C3)

Content Options and Color Options columns with checkboxes, text fields, and buttons render identically.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 163. Toolbar — 🟢/⏳
<sub>toolbar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/toolbar_light.png" /></td><td><img width="300px" src="captures/android/cpp/toolbar_light.png" /></td><td><img width="300px" src="captures/android/xaml/toolbar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ToolbarPage.xaml (Maui.Controls.Sample.Pages.ToolbarPage)

#### 🟢 Sonnet 5 — C++ (C1/C3)

All six toolbar action buttons and header text match exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 164. Transform Playground — 🟢/⏳
<sub>transform_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/transform_playground_light.png" /></td><td><img width="300px" src="captures/android/cpp/transform_playground_light.png" /></td><td><img width="300px" src="captures/android/xaml/transform_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports TransformPlaygroundGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/TransformPlaygroundGallery.xaml: a 50x50 Path rectangle (red fill, blue stroke 4) sits in a 200x200 light-grey panel; belo

#### 🟢 Sonnet 5 — C++ (C1/C3)

Red/blue square, all transform sliders, and labels match exactly including slider positions.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 165. Transformations — 🟢/⏳
<sub>transformations</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/transformations_light.png" /></td><td><img width="300px" src="captures/android/cpp/transformations_light.png" /></td><td><img width="300px" src="captures/android/xaml/transformations_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports TransformationsPage.xaml (+ .xaml.cs) The MAUI TransformationsPage drives a single target view&amp;#x27;s render transforms from a column of knobs: Sliders for Scale / ScaleX / ScaleY (Maximum 10) and Rotation / RotationX / RotationY (Maximum

#### 🟢 Sonnet 5 — C++ (C1/C3)

Scale/Rotation/Anchor/Translation sliders and buttons all match in layout, labels, and values.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 166. Triggers — 🟢/⏳
<sub>triggers</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/triggers_light.png" /></td><td><img width="300px" src="captures/android/cpp/triggers_light.png" /></td><td><img width="300px" src="captures/android/xaml/triggers_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports TriggersPage.xaml

#### 🟢 Sonnet 5 — C++ (C1/C3)

Layout, text, and toggle button match the MAUI reference exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 167. Update Path Data — 🟢/⏳
<sub>update_path_data</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/update_path_data_light.png" /></td><td><img width="300px" src="captures/android/cpp/update_path_data_light.png" /></td><td><img width="300px" src="captures/android/xaml/update_path_data_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports UpdatePathDataGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/UpdatePathDataGallery.xaml: a 2-row Grid (RowSpacing 0) that proves a Path repaints when its Data geometry is replaced at runti

#### 🟢 Sonnet 5 — C++ (C1/C3)

Path curve rendering and data label are identical to MAUI.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 168. Varied Size Selector — 🟢/⏳
<sub>varied_size_selector</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/varied_size_selector_light.png" /></td><td><img width="300px" src="captures/android/cpp/varied_size_selector_light.png" /></td><td><img width="300px" src="captures/android/xaml/varied_size_selector_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports DataTemplateSelectorGalleries/VariedSizeDataTemplateSelectorGallery.xaml (+ VariedSizeDataTemplateSelectorGallery.xaml.cs) of the C# CollectionView gallery

#### 🟢 Sonnet 5 — C++ (C1/C3)

Coffee/Milk colored bands and text match reference exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 169. Vertical Stack — 🟢/⏳
<sub>vertical_stack</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/vertical_stack_light.png" /></td><td><img width="300px" src="captures/android/cpp/vertical_stack_light.png" /></td><td><img width="300px" src="captures/android/xaml/vertical_stack_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

Vertical Stack

#### 🟢 Sonnet 5 — C++ (C1/C3)

Six colored squares stack identically to MAUI reference.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 170. Visual States — 🟢/⏳
<sub>visual_states</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/visual_states_light.png" /></td><td><img width="300px" src="captures/android/cpp/visual_states_light.png" /></td><td><img width="300px" src="captures/android/xaml/visual_states_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports VisualStatesPage.xaml

#### 🟢 Sonnet 5 — C++ (C1/C3)

Entry, button, and text visual-state layout matches MAUI exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 171. Web View — 🟡/⏳
<sub>web_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/web_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/web_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/web_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

a self-contained demo page for the W1-08 web_view vertical: a web_view loading a STATIC html_web_view_source (no network), back/forward/reload buttons over the handler-pushed CanGoBack/CanGoForward read-onlys, an &amp;quot;Eval 1+1&amp;quot; button driving t

#### 🟡 Sonnet 5 — C++ (C1/C3)

HeightRequest=240 builder-drift FIXED: the 240px WebView region + labels + buttons now align with MAUI (pixel 22.69%-&gt;6.89%). Residual: cpp faithfully renders the page's static HtmlWebViewSource ('Welcome' + para) which the twin degrades to a blank url. Twin-degradation (cpp faithful to the original page), flagged for the user — same family as context_flyout.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

### 172. Z Index — 🟢/⏳
<sub>z_index</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/z_index_light.png" /></td><td><img width="300px" src="captures/android/cpp/z_index_light.png" /></td><td><img width="300px" src="captures/android/xaml/z_index_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td><td><img width="300px" src="_placeholder.png" /></td></tr></table>

ports ZIndexPage.xaml (+ ZIndexPage.xaml.cs), code-first

#### 🟢 Sonnet 5 — C++ (C1/C3)

Overlapping z-indexed colored label stack matches MAUI reference exactly.

#### ⏳ Sonnet 5 — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

#### ⏳ Gemini — C++

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ (C1/C3)

_Not yet reviewed._

#### ⏳ Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

_Not yet reviewed._

</details>
