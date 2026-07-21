# .NET MAUI C++ port — visual parity comparison

Per-page MAUI-vs-C++ visual parity for the **172 gallery pages**, on **iOS**, **macOS** (Mac Catalyst + AppKit) and **Android**. Each section is collapsible and holds a discrepancy-count summary, then one subheader per page titled with a `{Sonnet}/{Gemini}` status-emoji combo (🟢 match / 🟡 minor / 🔴 major / ⬛ blank / ⏳ unreviewed). Under each page: the MAUI / C++ / C++&amp;XAML renders (light over dark; missing captures show a placeholder), then a subsubheader per review model (Sonnet, Gemini, Pixel-Perfect Score) titled with that model's own status emoji and holding its review prose. Generated from `comparison.json` by `tools/gen_readme.py` — do not edit by hand.

<details>
<summary><h2>iOS (172 examples) — click to expand</h2></summary>

Real .NET MAUI (native-default) vs the C++ port vs the compile-time-XAML gallery, captured on the same iOS simulator in light and dark. MAUI is the content ground truth.

**Discrepancy counts** (MAUI-vs-C++ parity verdicts from the deterministic pixel-perfect score — SSIM + per-pixel diff; AI-based review has been invalidated/removed):

| Classification | Pixel-Perfect Score — C++ (C1/C3) | Pixel-Perfect Score — C++ &amp; XAML (C2/C4) |
| --- | --- | --- |
| 🟢 Match | 157 | 158 |
| 🟡 Minor | 15 | 14 |
| 🔴 Major | 0 | 0 |
| ⬛ Blank | 0 | 0 |
| ⏳ Unreviewed | 0 | 0 |

### 1. Absolute Layout — 🟢/🟢
<sub>absolute_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/absolute_layout_light.png" /></td><td><img width="300px" src="captures/ios/cpp/absolute_layout_light.png" /></td><td><img width="300px" src="captures/ios/xaml/absolute_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/absolute_layout_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/absolute_layout_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/absolute_layout_dark.png" /></td></tr></table>

ports AbsoluteLayoutPage.xaml A self-contained, code-first demo of the AbsoluteLayout control

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9901, 0.64% pixels differ · Dark: SSIM 0.9899, 0.65% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

### 2. Activity Indicator — 🟢/🟡
<sub>activity_indicator</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/activity_indicator_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/activity_indicator_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/activity_indicator_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/activity_indicator_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/activity_indicator_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/activity_indicator_dark.gif" /></td></tr></table>

ports ActivityIndicatorPage.xaml (+ ActivityIndicatorPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9974, 0.14% pixels differ · Dark: SSIM 0.9985, 0.08% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9575, 1.57% pixels differ · Dark: SSIM 0.9566, 1.57% pixels differ

### 3. Adaptive Collection — 🟢/🟢
<sub>adaptive_collection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/adaptive_collection_light.png" /></td><td><img width="300px" src="captures/ios/cpp/adaptive_collection_light.png" /></td><td><img width="300px" src="captures/ios/xaml/adaptive_collection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/adaptive_collection_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/adaptive_collection_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/adaptive_collection_dark.png" /></td></tr></table>

ports AdaptiveCollectionView.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.AdaptiveCollectionView)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9974, 0.23% pixels differ · Dark: SSIM 0.9978, 0.21% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9993, 0.04% pixels differ

### 4. Alerts — 🟢/🟢
<sub>alerts</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/alerts_light.png" /></td><td><img width="300px" src="captures/ios/cpp/alerts_light.png" /></td><td><img width="300px" src="captures/ios/xaml/alerts_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/alerts_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/alerts_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/alerts_dark.png" /></td></tr></table>

ports AlertsPage.xaml (+ AlertsPage.xaml.cs) The C# AlertsPage drives the three Page dialog services — DisplayAlertAsync (simple OK + Yes/No), DisplayActionSheetAsync (simple + Cancel/Delete), and DisplayPromptAsync (two questions) — from a

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9979, 0.10% pixels differ · Dark: SSIM 0.9979, 0.10% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9994, 0.04% pixels differ · Dark: SSIM 0.9993, 0.04% pixels differ

### 5. Alignment — 🟢/🟢
<sub>alignment</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/alignment_light.png" /></td><td><img width="300px" src="captures/ios/cpp/alignment_light.png" /></td><td><img width="300px" src="captures/ios/xaml/alignment_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/alignment_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/alignment_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/alignment_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;alignment&amp;quot; demo (ComparePages.Alignment()), the shipped-.NET-MAUI reference for the visual-parity comparison

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9803, 0.61% pixels differ · Dark: SSIM 0.9859, 0.61% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9802, 0.61% pixels differ · Dark: SSIM 0.9858, 0.61% pixels differ

### 6. Animation — 🟡/🟡
<sub>animation</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/animation_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/animation_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/animation_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/animation_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/animation_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/animation_dark.gif" /></td></tr></table>

ports AnimationPage.xaml (+ AnimationPage.xaml.cs)

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9503, 2.09% pixels differ · Dark: SSIM 0.9485, 2.11% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9486, 2.13% pixels differ · Dark: SSIM 0.9450, 2.16% pixels differ

### 7. App Theme Binding — 🟢/🟢
<sub>app_theme_binding</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/app_theme_binding_light.png" /></td><td><img width="300px" src="captures/ios/cpp/app_theme_binding_light.png" /></td><td><img width="300px" src="captures/ios/xaml/app_theme_binding_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/app_theme_binding_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/app_theme_binding_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/app_theme_binding_dark.png" /></td></tr></table>

ports AppThemeBindingPage.xaml

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9993, 0.04% pixels differ · Dark: SSIM 0.9992, 0.04% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9994, 0.04% pixels differ · Dark: SSIM 0.9993, 0.04% pixels differ

### 8. Application Control — 🟢/🟢
<sub>application_control</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/application_control_light.png" /></td><td><img width="300px" src="captures/ios/cpp/application_control_light.png" /></td><td><img width="300px" src="captures/ios/xaml/application_control_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/application_control_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/application_control_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/application_control_dark.png" /></td></tr></table>

ports ApplicationControlPage.xaml (+ .xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9993, 0.04% pixels differ · Dark: SSIM 0.9992, 0.04% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9994, 0.04% pixels differ · Dark: SSIM 0.9993, 0.04% pixels differ

### 9. Auto Size Shapes — 🟢/🟢
<sub>auto_size_shapes</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/auto_size_shapes_light.png" /></td><td><img width="300px" src="captures/ios/cpp/auto_size_shapes_light.png" /></td><td><img width="300px" src="captures/ios/xaml/auto_size_shapes_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/auto_size_shapes_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/auto_size_shapes_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/auto_size_shapes_dark.png" /></td></tr></table>

ports AutoSizeShapesGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/AutoSizeShapesGallery.xaml: a 3-row Grid (RowSpacing 0) that proves a stroked Ellipse auto-sizes to fill exactly half of the av

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9993, 0.04% pixels differ · Dark: SSIM 0.9993, 0.04% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9993, 0.04% pixels differ · Dark: SSIM 0.9993, 0.04% pixels differ

### 10. Basic Grouping — 🟢/🟢
<sub>basic_grouping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/basic_grouping_light.png" /></td><td><img width="300px" src="captures/ios/cpp/basic_grouping_light.png" /></td><td><img width="300px" src="captures/ios/xaml/basic_grouping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/basic_grouping_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/basic_grouping_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/basic_grouping_dark.png" /></td></tr></table>

ports GroupingGalleries/BasicGrouping.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9984, 0.07% pixels differ · Dark: SSIM 0.9983, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9984, 0.07% pixels differ · Dark: SSIM 0.9996, 0.02% pixels differ

### 11. Basic Swipe — 🟢/🟢
<sub>basic_swipe</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/basic_swipe_light.png" /></td><td><img width="300px" src="captures/ios/cpp/basic_swipe_light.png" /></td><td><img width="300px" src="captures/ios/xaml/basic_swipe_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/basic_swipe_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/basic_swipe_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/basic_swipe_dark.png" /></td></tr></table>

ports BasicSwipeGallery.xaml A code-first port of the MAUI SwipeView sub-gallery Pages/Controls/SwipeViewGalleries/BasicSwipeGallery.xaml: a vertical StackLayout of five SwipeViews, each demonstrating a different revealed-side / SwipeMode c

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9996, 0.02% pixels differ · Dark: SSIM 0.9996, 0.02% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9990, 0.05% pixels differ · Dark: SSIM 0.9990, 0.05% pixels differ

### 12. Behaviors — 🟢/🟢
<sub>behaviors</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/behaviors_light.png" /></td><td><img width="300px" src="captures/ios/cpp/behaviors_light.png" /></td><td><img width="300px" src="captures/ios/xaml/behaviors_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/behaviors_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/behaviors_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/behaviors_dark.png" /></td></tr></table>

ports BehaviorsPage.xaml (+ .xaml.cs) and its companion Controls.Sample/Behaviors/NumericValidationBehavior.cs

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9980, 0.09% pixels differ · Dark: SSIM 0.9980, 0.09% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9990, 0.05% pixels differ · Dark: SSIM 0.9990, 0.05% pixels differ

### 13. Border — 🟢/🟢
<sub>border</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/border_light.png" /></td><td><img width="300px" src="captures/ios/cpp/border_light.png" /></td><td><img width="300px" src="captures/ios/xaml/border_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/border_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/border_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/border_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;border&amp;quot; demo (ComparePages.BorderPage()), the shipped-.NET-MAUI reference for the visual-parity comparison: a single Border centered on the page — red 5pt stroke, a RoundRectangle StrokeShape (Co

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9902, 0.36% pixels differ · Dark: SSIM 0.9912, 0.35% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9898, 0.37% pixels differ · Dark: SSIM 0.9906, 0.37% pixels differ

### 14. Border Clip Playground — 🟡/🟡
<sub>border_clip_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/border_clip_playground_light.png" /></td><td><img width="300px" src="captures/ios/cpp/border_clip_playground_light.png" /></td><td><img width="300px" src="captures/ios/xaml/border_clip_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/border_clip_playground_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/border_clip_playground_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/border_clip_playground_dark.png" /></td></tr></table>

ports BorderClipPlayground.xaml (+ .xaml.cs) The C# page is an interactive Border-shape playground: a 100x100 Border (red stroke) clips an AspectFill Image (oasis.jpg) into the currently selected StrokeShape, while controls below mutate the

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9695, 1.86% pixels differ · Dark: SSIM 0.9694, 1.86% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9751, 1.29% pixels differ · Dark: SSIM 0.9750, 1.29% pixels differ

### 15. Border Layout — 🟢/🟢
<sub>border_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/border_layout_light.png" /></td><td><img width="300px" src="captures/ios/cpp/border_layout_light.png" /></td><td><img width="300px" src="captures/ios/xaml/border_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/border_layout_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/border_layout_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/border_layout_dark.png" /></td></tr></table>

ports BorderLayout.xaml (+ BorderLayout.xaml.cs) The C# page demonstrates driving Border.StrokeThickness from a Slider: a Slider (0..40, set to 5 in OnAppearing) is bound to the Border&amp;#x27;s StrokeThickness; the Border (Silver stroke, White bac

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9911, 0.34% pixels differ · Dark: SSIM 0.9903, 0.35% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9920, 0.31% pixels differ · Dark: SSIM 0.9912, 0.32% pixels differ

### 16. Border Playground — 🟡/🟢
<sub>border_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/border_playground_light.png" /></td><td><img width="300px" src="captures/ios/cpp/border_playground_light.png" /></td><td><img width="300px" src="captures/ios/xaml/border_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/border_playground_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/border_playground_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/border_playground_dark.png" /></td></tr></table>

ports BorderPlayground.xaml (+ BorderPlayground.xaml.cs) A self-contained, code-first interactive Border playground

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9817, 1.32% pixels differ · Dark: SSIM 0.9807, 1.34% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9830, 0.89% pixels differ · Dark: SSIM 0.9823, 0.89% pixels differ

### 17. Border Resize Content — 🟡/🟡
<sub>border_resize_content</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/border_resize_content_light.png" /></td><td><img width="300px" src="captures/ios/cpp/border_resize_content_light.png" /></td><td><img width="300px" src="captures/ios/xaml/border_resize_content_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/border_resize_content_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/border_resize_content_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/border_resize_content_dark.png" /></td></tr></table>

ports BorderResizeContent.xaml A self-contained, code-first demo that resizes a Border&amp;#x27;s CONTENT and watches the Border track it

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9791, 1.52% pixels differ · Dark: SSIM 0.9753, 1.55% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9853, 1.13% pixels differ · Dark: SSIM 0.9837, 1.16% pixels differ

### 18. Border Stroke — 🟡/🟡
<sub>border_stroke</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/border_stroke_light.png" /></td><td><img width="300px" src="captures/ios/cpp/border_stroke_light.png" /></td><td><img width="300px" src="captures/ios/xaml/border_stroke_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/border_stroke_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/border_stroke_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/border_stroke_dark.png" /></td></tr></table>

ports BorderStroke.xaml (+ BorderStroke.xaml.cs) A self-contained, code-first demo of Border StrokeThickness and how a Border tracks the height of its content

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9526, 1.95% pixels differ · Dark: SSIM 0.9523, 1.96% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9530, 1.93% pixels differ · Dark: SSIM 0.9532, 1.93% pixels differ

### 19. Borderless — 🟢/🟢
<sub>borderless</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/borderless_light.png" /></td><td><img width="300px" src="captures/ios/cpp/borderless_light.png" /></td><td><img width="300px" src="captures/ios/xaml/borderless_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/borderless_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/borderless_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/borderless_dark.png" /></td></tr></table>

ports Borderless.xaml A self-contained, code-first demo of a stroke-less Border

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9985, 0.07% pixels differ · Dark: SSIM 0.9985, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9994, 0.03% pixels differ · Dark: SSIM 0.9994, 0.03% pixels differ

### 20. Box View — 🟢/🟢
<sub>box_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/box_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/box_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/box_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/box_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/box_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/box_view_dark.png" /></td></tr></table>

ports BoxViewPage.xaml (+ BoxViewPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9985, 0.07% pixels differ · Dark: SSIM 0.9990, 0.05% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9992, 0.05% pixels differ · Dark: SSIM 0.9991, 0.05% pixels differ

### 21. Button — 🟡/🟡
<sub>button</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/button_light.png" /></td><td><img width="300px" src="captures/ios/cpp/button_light.png" /></td><td><img width="300px" src="captures/ios/xaml/button_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/button_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/button_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/button_dark.png" /></td></tr></table>

ports ButtonPage.xaml (+ ButtonPage.xaml.cs)

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9806, 1.03% pixels differ · Dark: SSIM 0.9810, 1.02% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9807, 1.03% pixels differ · Dark: SSIM 0.9812, 1.01% pixels differ

### 22. Carousel Page — 🟡/🟢
<sub>carousel_page</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/carousel_page_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/carousel_page_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/carousel_page_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/carousel_page_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/carousel_page_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/carousel_page_dark.gif" /></td></tr></table>

Carousel Page

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9758, 0.63% pixels differ · Dark: SSIM 0.9755, 0.63% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9853, 0.40% pixels differ · Dark: SSIM 0.9845, 0.41% pixels differ

### 23. Chat Example — 🟢/🟢
<sub>chat_example</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/chat_example_light.png" /></td><td><img width="300px" src="captures/ios/cpp/chat_example_light.png" /></td><td><img width="300px" src="captures/ios/xaml/chat_example_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/chat_example_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/chat_example_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/chat_example_dark.png" /></td></tr></table>

ports ChatExample.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.ItemSizeGalleries.ChatExample), tracking the maui-compare reference demo ~/maui-compare/Pages/ChatExamplePage.cs (the visual-parity oracle)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9994, 0.04% pixels differ · Dark: SSIM 0.9993, 0.04% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9996, 0.03% pixels differ · Dark: SSIM 0.9995, 0.03% pixels differ

### 24. Check Box — 🟢/🟢
<sub>check_box</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/check_box_light.png" /></td><td><img width="300px" src="captures/ios/cpp/check_box_light.png" /></td><td><img width="300px" src="captures/ios/xaml/check_box_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/check_box_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/check_box_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/check_box_dark.png" /></td></tr></table>

ports CheckBoxPage.xaml (+ .xaml.cs) Mirrors the MAUI gallery page: a vertical stack of headlined CheckBox states — Default, Colored (Color=Purple), Disabled, Disabled+Colored+Checked — followed by a &amp;quot;Change IsChecked&amp;quot; row pairing a Button

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9967, 0.16% pixels differ · Dark: SSIM 0.9967, 0.16% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9996, 0.03% pixels differ · Dark: SSIM 0.9995, 0.03% pixels differ

### 25. Chrome — 🟢/🟢
<sub>chrome</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/chrome_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/chrome_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/chrome_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/chrome_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/chrome_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/chrome_dark.gif" /></td></tr></table>

a self-contained demo page for the W1-11 window-chrome family: page toolbar items (primary + secondary), a menu bar (File menu with items, a separator and a sub-menu), a context flyout (right-click menu) on a button, and a tooltip — all wir

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9984, 0.08% pixels differ · Dark: SSIM 0.9986, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9992, 0.06% pixels differ · Dark: SSIM 0.9987, 0.07% pixels differ

### 26. Clip — 🟢/🟢
<sub>clip</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/clip_light.png" /></td><td><img width="300px" src="captures/ios/cpp/clip_light.png" /></td><td><img width="300px" src="captures/ios/xaml/clip_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/clip_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/clip_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/clip_dark.png" /></td></tr></table>

ports ClipPage.xaml The C# page (Pages/Core/ClipPage.xaml; its .xaml.cs is an empty InitializeComponent) is a ScrollView over a StackLayout that shows the SAME dotnet_bot.png image five times, each successive copy carrying a different geome

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9996, 0.02% pixels differ · Dark: SSIM 0.9996, 0.02% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9985, 0.07% pixels differ · Dark: SSIM 0.9985, 0.07% pixels differ

### 27. Clip Corner Radius — 🟢/🟢
<sub>clip_corner_radius</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/clip_corner_radius_light.png" /></td><td><img width="300px" src="captures/ios/cpp/clip_corner_radius_light.png" /></td><td><img width="300px" src="captures/ios/xaml/clip_corner_radius_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/clip_corner_radius_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/clip_corner_radius_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/clip_corner_radius_dark.png" /></td></tr></table>

ports ClipCornerRadiusGallery.xaml (+ .xaml.cs) The C# page (Pages/Controls/ShapesGalleries/ClipCornerRadiusGallery.xaml) is a StackLayout (Padding=12) that demonstrates DRIVING a RoundRectangleGeometry&amp;#x27;s per-corner CornerRadius from four s

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9996, 0.02% pixels differ · Dark: SSIM 0.9995, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9985, 0.07% pixels differ · Dark: SSIM 0.9984, 0.07% pixels differ

### 28. Clip Gallery — 🟢/🟢
<sub>clip_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/clip_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/clip_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/clip_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/clip_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/clip_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/clip_gallery_dark.png" /></td></tr></table>

ports ClipGallery.xaml The C# page (Pages/Controls/ShapesGalleries/ClipGallery.xaml; its .xaml.cs is an empty InitializeComponent) is a ScrollView over a StackLayout (Padding=12) that shows the SAME &amp;quot;oasis.jpg&amp;quot; image SEVEN times — one bare

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9995, 0.03% pixels differ · Dark: SSIM 0.9995, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9985, 0.07% pixels differ · Dark: SSIM 0.9984, 0.07% pixels differ

### 29. Clip Views — 🟢/🟢
<sub>clip_views</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/clip_views_light.png" /></td><td><img width="300px" src="captures/ios/cpp/clip_views_light.png" /></td><td><img width="300px" src="captures/ios/xaml/clip_views_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/clip_views_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/clip_views_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/clip_views_dark.png" /></td></tr></table>

ports ClipViewsGallery.xaml The C# page (Pages/Controls/ShapesGalleries/ClipViewsGallery.xaml; no code-behind beyond an empty InitializeComponent) is a ScrollView over a StackLayout (Padding=12) that proves the Clip surface (VisualElement.C

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9974, 0.12% pixels differ · Dark: SSIM 0.9973, 0.12% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9974, 0.12% pixels differ · Dark: SSIM 0.9973, 0.12% pixels differ

### 30. Clipping — 🟢/🟢
<sub>clipping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/clipping_light.png" /></td><td><img width="300px" src="captures/ios/cpp/clipping_light.png" /></td><td><img width="300px" src="captures/ios/xaml/clipping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/clipping_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/clipping_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/clipping_dark.png" /></td></tr></table>

compare oracle ~/maui-compare/Pages/ClippingPage.cs (itself written to mirror this gallery page)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9996, 0.03% pixels differ · Dark: SSIM 0.9996, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9949, 0.20% pixels differ · Dark: SSIM 0.9986, 0.07% pixels differ

### 31. Collectionview — 🟢/🟢
<sub>collectionview</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/collectionview_light.png" /></td><td><img width="300px" src="captures/ios/cpp/collectionview_light.png" /></td><td><img width="300px" src="captures/ios/xaml/collectionview_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/collectionview_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/collectionview_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/collectionview_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;collectionview&amp;quot; demo (ComparePages.CollectionViewPage()), the shipped-.NET-MAUI reference for the visual-parity comparison: a CollectionView over 24 captioned items, a string Header (&amp;quot;This is the

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9994, 0.03% pixels differ · Dark: SSIM 0.9993, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9985, 0.07% pixels differ · Dark: SSIM 0.9994, 0.03% pixels differ

### 32. Composition Gallery — 🟢/🟢
<sub>composition_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/composition_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/composition_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/composition_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/composition_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/composition_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/composition_gallery_dark.png" /></td></tr></table>

ports CompositionGallery.xaml A self-contained, code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/CompositionGallery.xaml: a StackLayout holding two Beige 250x250 Grids (Margin 12) that compose multiple overlappi

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9981, 0.09% pixels differ · Dark: SSIM 0.9981, 0.09% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9985, 0.07% pixels differ · Dark: SSIM 0.9991, 0.04% pixels differ

### 33. Containers — 🟢/🟢
<sub>containers</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/containers_light.png" /></td><td><img width="300px" src="captures/ios/cpp/containers_light.png" /></td><td><img width="300px" src="captures/ios/xaml/containers_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/containers_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/containers_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/containers_dark.png" /></td></tr></table>

a self-contained demo page for the W1-07 container set: a scroll_view hosting a vertical stack of content-hosting containers — a border-framed label (stroke + dashed outline + rounded shape), a legacy frame (BorderColor/CornerRadius/HasShad

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9892, 0.42% pixels differ · Dark: SSIM 0.9883, 0.43% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9900, 0.40% pixels differ · Dark: SSIM 0.9897, 0.38% pixels differ

### 34. Content View — 🟢/🟢
<sub>content_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/content_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/content_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/content_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/content_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/content_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/content_view_dark.png" /></td></tr></table>

ports ContentViewPage.xaml (+ ContentViewPage.xaml.cs), code-first

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9990, 0.05% pixels differ · Dark: SSIM 0.9989, 0.05% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9989, 0.05% pixels differ · Dark: SSIM 0.9989, 0.05% pixels differ

### 35. Context Flyout — 🟢/🟢
<sub>context_flyout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/context_flyout_light.png" /></td><td><img width="300px" src="captures/ios/cpp/context_flyout_light.png" /></td><td><img width="300px" src="captures/ios/xaml/context_flyout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/context_flyout_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/context_flyout_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/context_flyout_dark.png" /></td></tr></table>

ports ContextFlyoutPage.xaml (+ ContextFlyoutPage.xaml.cs) The C# page attaches a MenuFlyout as the FlyoutBase.ContextFlyout (right-click / long-press menu) of several controls and wires each menu item to a handler: - a Button (&amp;quot;Increment b

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9990, 0.05% pixels differ · Dark: SSIM 0.9989, 0.05% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9989, 0.05% pixels differ · Dark: SSIM 0.9989, 0.05% pixels differ

### 36. Controls Stack — 🟢/🟢
<sub>controls_stack</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/controls_stack_light.png" /></td><td><img width="300px" src="captures/ios/cpp/controls_stack_light.png" /></td><td><img width="300px" src="captures/ios/xaml/controls_stack_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/controls_stack_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/controls_stack_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/controls_stack_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;controls_stack&amp;quot; demo (ComparePages.ControlsStack()), the shipped-.NET-MAUI reference for the visual-parity comparison: a VerticalStackLayout (Spacing 12, Padding 16) showcasing the basic widgets

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9950, 0.22% pixels differ · Dark: SSIM 0.9945, 0.22% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9950, 0.22% pixels differ · Dark: SSIM 0.9944, 0.22% pixels differ

### 37. Custom Layout — 🟢/🟢
<sub>custom_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/custom_layout_light.png" /></td><td><img width="300px" src="captures/ios/cpp/custom_layout_light.png" /></td><td><img width="300px" src="captures/ios/xaml/custom_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/custom_layout_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/custom_layout_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/custom_layout_dark.png" /></td></tr></table>

ports CustomLayoutPage.xaml (+ CustomLayoutPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9990, 0.05% pixels differ · Dark: SSIM 0.9989, 0.05% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9989, 0.05% pixels differ · Dark: SSIM 0.9989, 0.05% pixels differ

### 38. Custom Size Swipe — 🟢/🟢
<sub>custom_size_swipe</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/custom_size_swipe_light.png" /></td><td><img width="300px" src="captures/ios/cpp/custom_size_swipe_light.png" /></td><td><img width="300px" src="captures/ios/xaml/custom_size_swipe_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/custom_size_swipe_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/custom_size_swipe_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/custom_size_swipe_dark.png" /></td></tr></table>

ports CustomSizeSwipeViewGallery.xaml (+ .xaml.cs) The MAUI CustomSizeSwipeViewGallery is a single SwipeView whose Left / Right / Top item collections each reveal CUSTOM-SIZED content: a SwipeItemView wrapping a Grid/StackLayout with an exp

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9990, 0.05% pixels differ · Dark: SSIM 0.9989, 0.05% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9989, 0.05% pixels differ · Dark: SSIM 0.9989, 0.05% pixels differ

### 39. Custom Swipe Item View — 🟢/🟢
<sub>custom_swipe_item_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/custom_swipe_item_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/custom_swipe_item_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/custom_swipe_item_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/custom_swipe_item_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/custom_swipe_item_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/custom_swipe_item_view_dark.png" /></td></tr></table>

ports CustomSwipeItemViewGallery.xaml A self-contained, code-first port of the .NET MAUI &amp;quot;CustomSwipeItem&amp;quot; gallery: a message-list row whose right swipe reveals a CUSTOM-content swipe item (a swipe_item_view, not a plain swipe_item)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9989, 0.06% pixels differ · Dark: SSIM 0.9990, 0.05% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9985, 0.07% pixels differ · Dark: SSIM 0.9985, 0.07% pixels differ

### 40. Cv Visual States — 🟢/🟢
<sub>cv_visual_states</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/cv_visual_states_light.png" /></td><td><img width="300px" src="captures/ios/cpp/cv_visual_states_light.png" /></td><td><img width="300px" src="captures/ios/xaml/cv_visual_states_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/cv_visual_states_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/cv_visual_states_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/cv_visual_states_dark.png" /></td></tr></table>

ports CollectionViewGalleries/SelectionGalleries/ VisualStatesGallery.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9990, 0.05% pixels differ · Dark: SSIM 0.9990, 0.05% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.07% pixels differ · Dark: SSIM 0.9985, 0.07% pixels differ

### 41. Data Template Selector — 🟢/🟢
<sub>data_template_selector</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/data_template_selector_light.png" /></td><td><img width="300px" src="captures/ios/cpp/data_template_selector_light.png" /></td><td><img width="300px" src="captures/ios/xaml/data_template_selector_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/data_template_selector_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/data_template_selector_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/data_template_selector_dark.png" /></td></tr></table>

ports DataTemplateSelectorGallery.xaml (+ DataTemplateSelectorGallery.xaml.cs, including its WeekendSelector + SearchTermSelector classes)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9984, 0.14% pixels differ · Dark: SSIM 0.9983, 0.14% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9978, 0.16% pixels differ · Dark: SSIM 0.9977, 0.16% pixels differ

### 42. Date Picker — 🟢/🟢
<sub>date_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/date_picker_light.png" /></td><td><img width="300px" src="captures/ios/cpp/date_picker_light.png" /></td><td><img width="300px" src="captures/ios/xaml/date_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/date_picker_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/date_picker_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/date_picker_dark.png" /></td></tr></table>

ports DatePickerPage.xaml (+ DatePickerPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9907, 0.49% pixels differ · Dark: SSIM 0.9899, 0.52% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9926, 0.44% pixels differ · Dark: SSIM 0.9925, 0.44% pixels differ

### 43. Device — 🟢/🟢
<sub>device</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/device_light.png" /></td><td><img width="300px" src="captures/ios/cpp/device_light.png" /></td><td><img width="300px" src="captures/ios/xaml/device_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/device_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/device_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/device_dark.png" /></td></tr></table>

ports DevicePage.xaml

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9985, 0.08% pixels differ · Dark: SSIM 0.9984, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9989, 0.06% pixels differ · Dark: SSIM 0.9988, 0.06% pixels differ

### 44. Dispatcher — 🟢/🟢
<sub>dispatcher</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/dispatcher_light.png" /></td><td><img width="300px" src="captures/ios/cpp/dispatcher_light.png" /></td><td><img width="300px" src="captures/ios/xaml/dispatcher_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/dispatcher_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/dispatcher_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/dispatcher_dark.png" /></td></tr></table>

ports DispatcherPage.xaml (+ .xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9995, 0.03% pixels differ · Dark: SSIM 0.9995, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9989, 0.06% pixels differ · Dark: SSIM 0.9988, 0.06% pixels differ

### 45. Drag Drop — 🟢/🟢
<sub>drag_drop</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/drag_drop_light.png" /></td><td><img width="300px" src="captures/ios/cpp/drag_drop_light.png" /></td><td><img width="300px" src="captures/ios/xaml/drag_drop_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/drag_drop_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/drag_drop_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/drag_drop_dark.png" /></td></tr></table>

ports DragAndDropBetweenLayouts.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.DragAndDropBetweenLayouts)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9995, 0.03% pixels differ · Dark: SSIM 0.9995, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9989, 0.06% pixels differ · Dark: SSIM 0.9988, 0.06% pixels differ

### 46. Editor — 🟢/🟢
<sub>editor</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/editor_light.png" /></td><td><img width="300px" src="captures/ios/cpp/editor_light.png" /></td><td><img width="300px" src="captures/ios/xaml/editor_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/editor_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/editor_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/editor_dark.png" /></td></tr></table>

ports EditorPage.xaml (Microsoft.Maui.Controls sample gallery)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9982, 0.13% pixels differ · Dark: SSIM 0.9979, 0.13% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9974, 0.16% pixels differ · Dark: SSIM 0.9971, 0.16% pixels differ

### 47. Effects — 🟢/🟢
<sub>effects</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/effects_light.png" /></td><td><img width="300px" src="captures/ios/cpp/effects_light.png" /></td><td><img width="300px" src="captures/ios/xaml/effects_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/effects_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/effects_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/effects_dark.png" /></td></tr></table>

ports EffectsPage.xaml (Maui.Controls.Sample.Pages.EffectsPage)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9996, 0.03% pixels differ · Dark: SSIM 0.9996, 0.02% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9985, 0.07% pixels differ

### 48. Ellipse Gallery — 🟢/🟢
<sub>ellipse_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ellipse_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/ellipse_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/ellipse_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ellipse_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/ellipse_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/ellipse_gallery_dark.png" /></td></tr></table>

ports EllipseGallery.xaml A self-contained, code-first port of the MAUI Shapes EllipseGallery (Pages/Controls/ShapesGalleries/EllipseGallery.xaml + .xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9996, 0.02% pixels differ · Dark: SSIM 0.9996, 0.02% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.07% pixels differ · Dark: SSIM 0.9985, 0.07% pixels differ

### 49. Empty View — 🟢/🟢
<sub>empty_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/empty_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/empty_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_dark.png" /></td></tr></table>

ports EmptyViewStringGallery.xaml (+ EmptyViewStringGallery.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9993, 0.05% pixels differ · Dark: SSIM 0.9993, 0.05% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9982, 0.10% pixels differ · Dark: SSIM 0.9982, 0.10% pixels differ

### 50. Empty View Load Simulate — 🟢/🟢
<sub>empty_view_load_simulate</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/empty_view_load_simulate_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_load_simulate_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_load_simulate_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/empty_view_load_simulate_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_load_simulate_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_load_simulate_dark.gif" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewLoadSimulateGallery.xaml (+ EmptyViewLoadSimulateGallery.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9983, 0.10% pixels differ · Dark: SSIM 0.9982, 0.10% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9990, 0.07% pixels differ · Dark: SSIM 0.9989, 0.07% pixels differ

### 51. Empty View Null — 🟢/🟢
<sub>empty_view_null</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/empty_view_null_light.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_null_light.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_null_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/empty_view_null_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_null_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_null_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewNullGallery.xaml (+ EmptyViewNullGallery.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9994, 0.03% pixels differ · Dark: SSIM 0.9993, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9995, 0.03% pixels differ · Dark: SSIM 0.9994, 0.03% pixels differ

### 52. Empty View Rtl — 🟢/🟢
<sub>empty_view_rtl</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/empty_view_rtl_light.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_rtl_light.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_rtl_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/empty_view_rtl_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_rtl_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_rtl_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewRTLGallery.xaml (+ EmptyViewRTLGallery.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9963, 0.18% pixels differ · Dark: SSIM 0.9961, 0.19% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9982, 0.10% pixels differ · Dark: SSIM 0.9981, 0.10% pixels differ

### 53. Empty View Selector — 🟢/🟢
<sub>empty_view_selector</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/empty_view_selector_light.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_selector_light.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_selector_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/empty_view_selector_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_selector_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_selector_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewWithDataTemplateSelector.xaml (+ .xaml.cs, incl

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9988, 0.08% pixels differ · Dark: SSIM 0.9987, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9982, 0.10% pixels differ · Dark: SSIM 0.9981, 0.10% pixels differ

### 54. Empty View Swap — 🟢/🟢
<sub>empty_view_swap</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/empty_view_swap_light.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_swap_light.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_swap_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/empty_view_swap_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_swap_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_swap_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewSwapGallery.xaml (+ .xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9899, 0.41% pixels differ · Dark: SSIM 0.9897, 0.41% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9983, 0.09% pixels differ · Dark: SSIM 0.9983, 0.09% pixels differ

### 55. Empty View Template — 🟢/🟢
<sub>empty_view_template</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/empty_view_template_light.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_template_light.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_template_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/empty_view_template_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_template_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_template_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewTemplateGallery.xaml (+ EmptyViewTemplateGallery.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9986, 0.08% pixels differ · Dark: SSIM 0.9986, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9983, 0.09% pixels differ · Dark: SSIM 0.9983, 0.09% pixels differ

### 56. Empty View View — 🟢/🟢
<sub>empty_view_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/empty_view_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/empty_view_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/empty_view_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/empty_view_view_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewViewGallery.xaml (+ EmptyViewViewGallery.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9986, 0.08% pixels differ · Dark: SSIM 0.9986, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9983, 0.09% pixels differ · Dark: SSIM 0.9983, 0.09% pixels differ

### 57. Entry — 🟢/🟢
<sub>entry</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/entry_light.png" /></td><td><img width="300px" src="captures/ios/cpp/entry_light.png" /></td><td><img width="300px" src="captures/ios/xaml/entry_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/entry_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/entry_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/entry_dark.png" /></td></tr></table>

ports EntryPage.xaml (Microsoft.Maui.Controls sample gallery)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9989, 0.05% pixels differ · Dark: SSIM 0.9989, 0.05% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

### 58. Filter Collection — 🟢/🟢
<sub>filter_collection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/filter_collection_light.png" /></td><td><img width="300px" src="captures/ios/cpp/filter_collection_light.png" /></td><td><img width="300px" src="captures/ios/xaml/filter_collection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/filter_collection_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/filter_collection_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/filter_collection_dark.png" /></td></tr></table>

ports FilterCollectionView.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9938, 0.25% pixels differ · Dark: SSIM 0.9937, 0.25% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9983, 0.09% pixels differ · Dark: SSIM 0.9983, 0.09% pixels differ

### 59. Filter Selection — 🟢/🟢
<sub>filter_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/filter_selection_light.png" /></td><td><img width="300px" src="captures/ios/cpp/filter_selection_light.png" /></td><td><img width="300px" src="captures/ios/xaml/filter_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/filter_selection_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/filter_selection_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/filter_selection_dark.png" /></td></tr></table>

ports FilterSelection.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.FilterSelection)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9986, 0.08% pixels differ · Dark: SSIM 0.9986, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9983, 0.09% pixels differ · Dark: SSIM 0.9985, 0.09% pixels differ

### 60. Flex Layout — 🟢/🟢
<sub>flex_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/flex_layout_light.png" /></td><td><img width="300px" src="captures/ios/cpp/flex_layout_light.png" /></td><td><img width="300px" src="captures/ios/xaml/flex_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/flex_layout_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/flex_layout_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/flex_layout_dark.png" /></td></tr></table>

ports FlexLayoutPage.xaml A self-contained, code-first demo of the FlexLayout control: the classic &amp;quot;holy grail&amp;quot; page layout built from nested flexboxes

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9986, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9988, 0.06% pixels differ

### 61. Focus — 🟢/🟢
<sub>focus</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/focus_light.png" /></td><td><img width="300px" src="captures/ios/cpp/focus_light.png" /></td><td><img width="300px" src="captures/ios/xaml/focus_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/focus_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/focus_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/focus_dark.png" /></td></tr></table>

ports FocusPage.xaml (+ FocusPage.xaml.cs) The C# FocusPage is a focus-subsystem demo: an Entry whose Focused/Unfocused events (OnFocusEntryFocusChanged) append &amp;quot;Focused&amp;quot;/&amp;quot;Unfocused&amp;quot; lines to a scrolling InfoLabel, plus two buttons — &amp;quot;Focus

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9984, 0.07% pixels differ · Dark: SSIM 0.9984, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.05% pixels differ · Dark: SSIM 0.9987, 0.05% pixels differ

### 62. Fonts — 🟢/🟢
<sub>fonts</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/fonts_light.png" /></td><td><img width="300px" src="captures/ios/cpp/fonts_light.png" /></td><td><img width="300px" src="captures/ios/xaml/fonts_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/fonts_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/fonts_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/fonts_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;fonts&amp;quot; demo (ComparePages.Fonts()), the shipped-.NET-MAUI reference for the visual-parity comparison: a ScrollView over a VerticalStackLayout (Spacing 8, Padding 16) of nine Labels — Title/Subtit

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9984, 0.07% pixels differ · Dark: SSIM 0.9984, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.05% pixels differ · Dark: SSIM 0.9987, 0.05% pixels differ

### 63. Footer Only String — 🟢/🟢
<sub>footer_only_string</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/footer_only_string_light.png" /></td><td><img width="300px" src="captures/ios/cpp/footer_only_string_light.png" /></td><td><img width="300px" src="captures/ios/xaml/footer_only_string_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/footer_only_string_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/footer_only_string_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/footer_only_string_dark.png" /></td></tr></table>

ports FooterOnlyString.xaml (+ FooterOnlyString.xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9994, 0.03% pixels differ · Dark: SSIM 0.9993, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9995, 0.03% pixels differ · Dark: SSIM 0.9994, 0.03% pixels differ

### 64. Formatted Text — 🟢/🟢
<sub>formatted_text</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/formatted_text_light.png" /></td><td><img width="300px" src="captures/ios/cpp/formatted_text_light.png" /></td><td><img width="300px" src="captures/ios/xaml/formatted_text_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/formatted_text_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/formatted_text_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/formatted_text_dark.png" /></td></tr></table>

a self-contained demo page for the G1 rich-text slice: a label whose FormattedText is built from several styled spans (bold / italic / colored / underlined / kerned), plus a plain label proving the Text ⇄ FormattedText exclusivity

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9963, 0.15% pixels differ · Dark: SSIM 0.9963, 0.15% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.05% pixels differ · Dark: SSIM 0.9987, 0.05% pixels differ

### 65. Gestures — 🟢/🟢
<sub>gestures</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/gestures_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/gestures_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/gestures_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/gestures_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/gestures_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/gestures_dark.gif" /></td></tr></table>

ports GesturesPage.xaml (+ .xaml.cs) The MAUI GesturesPage.xaml is a *gallery navigation* page: a CollectionView listing gesture-demo sections that the shell navigates into

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9975, 0.11% pixels differ · Dark: SSIM 0.9974, 0.11% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9997, 0.07% pixels differ · Dark: SSIM 0.9997, 0.07% pixels differ

### 66. Gradient — 🟢/🟢
<sub>gradient</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/gradient_light.png" /></td><td><img width="300px" src="captures/ios/cpp/gradient_light.png" /></td><td><img width="300px" src="captures/ios/xaml/gradient_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/gradient_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/gradient_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/gradient_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;gradient&amp;quot; demo (ComparePages.Gradient()), the shipped-.NET-MAUI reference for the visual-parity comparison: a VerticalStackLayout (Spacing 12, Padding 16) of two captioned 60px BoxViews — a Linea

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9984, 0.07% pixels differ · Dark: SSIM 0.9985, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 67. Grid — 🟢/🟢
<sub>grid</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/grid_light.png" /></td><td><img width="300px" src="captures/ios/cpp/grid_light.png" /></td><td><img width="300px" src="captures/ios/xaml/grid_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/grid_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/grid_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/grid_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;grid&amp;quot; demo (ComparePages.GridPage()), the shipped-.NET-MAUI reference for the visual-parity comparison: a Grid (Padding 16, Row/ColumnSpacing 6) with RowDefinitions Auto / 80 / 80 and two Star co

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9986, 0.07% pixels differ · Dark: SSIM 0.9985, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 68. Grid Grouping — 🟢/🟢
<sub>grid_grouping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/grid_grouping_light.png" /></td><td><img width="300px" src="captures/ios/cpp/grid_grouping_light.png" /></td><td><img width="300px" src="captures/ios/xaml/grid_grouping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/grid_grouping_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/grid_grouping_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/grid_grouping_dark.png" /></td></tr></table>

ports GroupingGalleries/GridGrouping.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9985, 0.07% pixels differ · Dark: SSIM 0.9984, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9995, 0.03% pixels differ · Dark: SSIM 0.9994, 0.03% pixels differ

### 69. Grouping No Templates — 🟢/🟢
<sub>grouping_no_templates</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/grouping_no_templates_light.png" /></td><td><img width="300px" src="captures/ios/cpp/grouping_no_templates_light.png" /></td><td><img width="300px" src="captures/ios/xaml/grouping_no_templates_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/grouping_no_templates_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/grouping_no_templates_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/grouping_no_templates_dark.png" /></td></tr></table>

ports GroupingGalleries/GroupingNoTemplates.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9994, 0.03% pixels differ · Dark: SSIM 0.9993, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9995, 0.03% pixels differ · Dark: SSIM 0.9994, 0.03% pixels differ

### 70. Grouping Plus Selection — 🟢/🟢
<sub>grouping_plus_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/grouping_plus_selection_light.png" /></td><td><img width="300px" src="captures/ios/cpp/grouping_plus_selection_light.png" /></td><td><img width="300px" src="captures/ios/xaml/grouping_plus_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/grouping_plus_selection_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/grouping_plus_selection_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/grouping_plus_selection_dark.png" /></td></tr></table>

ports CollectionViewGalleries/GroupingGalleries/ GroupingPlusSelection.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9985, 0.07% pixels differ · Dark: SSIM 0.9984, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9995, 0.03% pixels differ · Dark: SSIM 0.9998, 0.01% pixels differ

### 71. Header Footer — 🟢/🟢
<sub>header_footer</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/header_footer_light.png" /></td><td><img width="300px" src="captures/ios/cpp/header_footer_light.png" /></td><td><img width="300px" src="captures/ios/xaml/header_footer_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/header_footer_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/header_footer_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/header_footer_dark.png" /></td></tr></table>

ports HeaderFooterString.xaml (+ HeaderFooterString.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9983, 0.08% pixels differ · Dark: SSIM 0.9983, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9985, 0.07% pixels differ · Dark: SSIM 0.9998, 0.01% pixels differ

### 72. Header Footer Grid — 🟢/🟢
<sub>header_footer_grid</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/header_footer_grid_light.png" /></td><td><img width="300px" src="captures/ios/cpp/header_footer_grid_light.png" /></td><td><img width="300px" src="captures/ios/xaml/header_footer_grid_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/header_footer_grid_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/header_footer_grid_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/header_footer_grid_dark.png" /></td></tr></table>

ports HeaderFooterGrid.xaml (+ HeaderFooterGrid.xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9930, 0.45% pixels differ · Dark: SSIM 0.9889, 0.52% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9935, 0.43% pixels differ · Dark: SSIM 0.9893, 0.51% pixels differ

### 73. Header Footer Grid Horizontal — 🟡/🟡
<sub>header_footer_grid_horizontal</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/header_footer_grid_horizontal_light.png" /></td><td><img width="300px" src="captures/ios/cpp/header_footer_grid_horizontal_light.png" /></td><td><img width="300px" src="captures/ios/xaml/header_footer_grid_horizontal_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/header_footer_grid_horizontal_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/header_footer_grid_horizontal_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/header_footer_grid_horizontal_dark.png" /></td></tr></table>

ports HeaderFooterGridHorizontal.xaml (+ HeaderFooterGridHorizontal.xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9748, 0.78% pixels differ · Dark: SSIM 0.9745, 0.78% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9753, 0.76% pixels differ · Dark: SSIM 0.9750, 0.76% pixels differ

### 74. Header Footer Template — 🟢/🟢
<sub>header_footer_template</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/header_footer_template_light.png" /></td><td><img width="300px" src="captures/ios/cpp/header_footer_template_light.png" /></td><td><img width="300px" src="captures/ios/xaml/header_footer_template_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/header_footer_template_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/header_footer_template_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/header_footer_template_dark.png" /></td></tr></table>

ports HeaderFooterTemplate.xaml (+ .xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9998, 0.01% pixels differ · Dark: SSIM 0.9998, 0.01% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9984, 0.07% pixels differ · Dark: SSIM 0.9983, 0.07% pixels differ

### 75. Header Footer View — 🟢/🟢
<sub>header_footer_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/header_footer_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/header_footer_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/header_footer_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/header_footer_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/header_footer_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/header_footer_view_dark.png" /></td></tr></table>

ports HeaderFooterView.xaml (+ .xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9982, 0.08% pixels differ · Dark: SSIM 0.9982, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

### 76. Hit Testing — 🟢/🟢
<sub>hit_testing</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/hit_testing_light.png" /></td><td><img width="300px" src="captures/ios/cpp/hit_testing_light.png" /></td><td><img width="300px" src="captures/ios/xaml/hit_testing_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/hit_testing_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/hit_testing_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/hit_testing_dark.png" /></td></tr></table>

ports HitTestingPage.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.HitTestingPage)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9985, 0.11% pixels differ · Dark: SSIM 0.9984, 0.11% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.10% pixels differ · Dark: SSIM 0.9988, 0.09% pixels differ

### 77. Horizontal Stack — 🟢/🟢
<sub>horizontal_stack</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/horizontal_stack_light.png" /></td><td><img width="300px" src="captures/ios/cpp/horizontal_stack_light.png" /></td><td><img width="300px" src="captures/ios/xaml/horizontal_stack_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/horizontal_stack_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/horizontal_stack_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/horizontal_stack_dark.png" /></td></tr></table>

Horizontal Stack

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9992, 0.03% pixels differ · Dark: SSIM 0.9992, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9989, 0.05% pixels differ · Dark: SSIM 0.9989, 0.05% pixels differ

### 78. Hybrid Web View — 🟢/🟢
<sub>hybrid_web_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/hybrid_web_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/hybrid_web_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/hybrid_web_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/hybrid_web_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/hybrid_web_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/hybrid_web_view_dark.png" /></td></tr></table>

ports HybridWebViewPage.xaml (+ HybridWebViewPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9877, 0.53% pixels differ · Dark: SSIM 0.9874, 0.53% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9870, 0.56% pixels differ · Dark: SSIM 0.9867, 0.56% pixels differ

### 79. Image — 🟡/🟡
<sub>image</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/image_light.png" /></td><td><img width="300px" src="captures/ios/cpp/image_light.png" /></td><td><img width="300px" src="captures/ios/xaml/image_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/image_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/image_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/image_dark.png" /></td></tr></table>

ports ImagePage.xaml (+ ImagePage.xaml.cs)

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9565, 3.99% pixels differ · Dark: SSIM 0.9975, 0.17% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9572, 3.96% pixels differ · Dark: SSIM 0.9972, 0.18% pixels differ

### 80. Image Button — 🟢/🟢
<sub>image_button</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/image_button_light.png" /></td><td><img width="300px" src="captures/ios/cpp/image_button_light.png" /></td><td><img width="300px" src="captures/ios/xaml/image_button_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/image_button_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/image_button_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/image_button_dark.png" /></td></tr></table>

ports ImageButtonPage.xaml (+ ImageButtonPage.xaml.cs) A self-contained, code-first demo page for the ImageButton control (the C# gallery-page convention, mirroring the input_controls_page / image_page pattern)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9874, 0.80% pixels differ · Dark: SSIM 0.9882, 0.77% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9875, 0.80% pixels differ · Dark: SSIM 0.9875, 0.80% pixels differ

### 81. Indicator — 🟢/🟢
<sub>indicator</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/indicator_light.png" /></td><td><img width="300px" src="captures/ios/cpp/indicator_light.png" /></td><td><img width="300px" src="captures/ios/xaml/indicator_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/indicator_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/indicator_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/indicator_dark.png" /></td></tr></table>

ports IndicatorPage.xaml A self-contained, code-first demo of the IndicatorView control

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9867, 0.81% pixels differ · Dark: SSIM 0.9829, 0.86% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9882, 0.77% pixels differ · Dark: SSIM 0.9846, 0.82% pixels differ

### 82. Input Controls — 🟢/🟢
<sub>input_controls</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/input_controls_light.png" /></td><td><img width="300px" src="captures/ios/cpp/input_controls_light.png" /></td><td><img width="300px" src="captures/ios/xaml/input_controls_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/input_controls_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/input_controls_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/input_controls_dark.png" /></td></tr></table>

a self-contained demo page for the W1-05 input-control set: editor, search_bar, radio_button (+ the radio_button_group attached grouping) and image_button on one vertical stack, wired together so every input drives a visible output (the C#

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9904, 0.45% pixels differ · Dark: SSIM 0.9904, 0.45% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9909, 0.44% pixels differ · Dark: SSIM 0.9908, 0.44% pixels differ

### 83. Input Transparent — 🟢/🟢
<sub>input_transparent</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/input_transparent_light.png" /></td><td><img width="300px" src="captures/ios/cpp/input_transparent_light.png" /></td><td><img width="300px" src="captures/ios/xaml/input_transparent_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/input_transparent_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/input_transparent_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/input_transparent_dark.png" /></td></tr></table>

ports InputTransparentPage.xaml (Maui.Controls.Sample.Pages.InputTransparentPage)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9986, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.07% pixels differ · Dark: SSIM 0.9986, 0.07% pixels differ

### 84. Invalidate Brush — 🟢/🟢
<sub>invalidate_brush</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/invalidate_brush_light.png" /></td><td><img width="300px" src="captures/ios/cpp/invalidate_brush_light.png" /></td><td><img width="300px" src="captures/ios/xaml/invalidate_brush_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/invalidate_brush_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/invalidate_brush_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/invalidate_brush_dark.png" /></td></tr></table>

ports InvalidateBrushGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/InvalidateBrushGallery.xaml (&amp;quot;Invalidate Brushes Playground&amp;quot;): a VerticalStackLayout (Padding 12) with — - a &amp;quot;Change color&amp;quot; Bu

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9982, 0.08% pixels differ · Dark: SSIM 0.9981, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9988, 0.06% pixels differ

### 85. Invalidate Shadow Host — 🟢/🟢
<sub>invalidate_shadow_host</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/invalidate_shadow_host_light.png" /></td><td><img width="300px" src="captures/ios/cpp/invalidate_shadow_host_light.png" /></td><td><img width="300px" src="captures/ios/xaml/invalidate_shadow_host_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/invalidate_shadow_host_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/invalidate_shadow_host_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/invalidate_shadow_host_dark.png" /></td></tr></table>

ports InvalidateShadowHostPage.xaml A self-contained, code-first demo that a shadow re-applies (invalidates) when its host&amp;#x27;s size changes, mirroring the C# core gallery page (Pages/Core/ShadowGalleries/InvalidateShadowHostPage.xaml + .xaml.

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9832, 0.59% pixels differ · Dark: SSIM 0.9850, 0.59% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9834, 0.58% pixels differ · Dark: SSIM 0.9852, 0.58% pixels differ

### 86. Ios Blur Effect — 🟢/🟢
<sub>ios_blur_effect</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_blur_effect_light.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_blur_effect_light.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_blur_effect_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_blur_effect_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_blur_effect_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_blur_effect_dark.png" /></td></tr></table>

ports iOSBlurEffectPage.xaml The .NET MAUI PlatformSpecifics sample (Pages/PlatformSpecifics/iOS/iOSBlurEffectPage.xaml + .xaml.cs): an Image (Source=&amp;quot;oasis.jpg&amp;quot;) carrying the iOSSpecific VisualElement.BlurEffect knob (XAML seeds it to Extr

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9988, 0.06% pixels differ

### 87. Ios Date Picker — 🟢/🟢
<sub>ios_date_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_date_picker_light.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_date_picker_light.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_date_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_date_picker_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_date_picker_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_date_picker_dark.png" /></td></tr></table>

ports iOSDatePickerPage.xaml (+ iOSDatePickerPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9988, 0.06% pixels differ

### 88. Ios Entry — 🟢/🟢
<sub>ios_entry</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_entry_light.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_entry_light.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_entry_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_entry_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_entry_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_entry_dark.png" /></td></tr></table>

ports iOSEntryPage.xaml (+ iOSEntryPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9844, 0.67% pixels differ · Dark: SSIM 0.9833, 0.70% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9988, 0.06% pixels differ

### 89. Ios First Responder — 🟢/🟢
<sub>ios_first_responder</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_first_responder_light.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_first_responder_light.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_first_responder_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_first_responder_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_first_responder_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_first_responder_dark.png" /></td></tr></table>

ports iOSFirstResponderPage.xaml (+ .xaml.cs) The C# iOSFirstResponderPage is a VisualElement-first-responder demo: a StackLayout with an explanatory Label, a &amp;quot;First Entry&amp;quot; + plain &amp;quot;OK&amp;quot; Button (tapping OK dismisses the keyboard because the

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 90. Ios Pan Gesture — 🟢/🟢
<sub>ios_pan_gesture</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_pan_gesture_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/ios_pan_gesture_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/ios_pan_gesture_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_pan_gesture_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/ios_pan_gesture_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/ios_pan_gesture_dark.gif" /></td></tr></table>

ports iOSPanGestureRecognizerPage.xaml (+ .xaml.cs) The C# iOSPanGestureRecognizerPage is a StackLayout with: a bold message Label (_messageLabel), a &amp;quot;Toggle Simultaneous Gesture Recognition&amp;quot; Button, and a grouped ListView of employees whos

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9983, 0.10% pixels differ · Dark: SSIM 0.9982, 0.10% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9990, 0.07% pixels differ · Dark: SSIM 0.9989, 0.07% pixels differ

### 91. Ios Picker — 🟢/🟢
<sub>ios_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_picker_light.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_picker_light.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_picker_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_picker_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_picker_dark.png" /></td></tr></table>

ports iOSPickerPage.xaml (+ iOSPickerPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

### 92. Ios Safe Area — 🟢/🟢
<sub>ios_safe_area</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_safe_area_light.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_safe_area_light.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_safe_area_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_safe_area_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_safe_area_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_safe_area_dark.png" /></td></tr></table>

ports iOSSafeAreaPage.xaml The .NET MAUI PlatformSpecifics sample (Pages/PlatformSpecifics/iOS/iOSSafeAreaPage.xaml + .xaml.cs): a long Lorem-ipsum Label over a &amp;quot;Disable Use Safe Area&amp;quot; button

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

### 93. Ios Scroll View — 🟢/🟢
<sub>ios_scroll_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_scroll_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_scroll_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_scroll_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_scroll_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_scroll_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_scroll_view_dark.png" /></td></tr></table>

ports iOSScrollViewPage.xaml (+ iOSScrollViewPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9985, 0.07% pixels differ · Dark: SSIM 0.9985, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 94. Ios Search Bar — 🟢/🟢
<sub>ios_search_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_search_bar_light.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_search_bar_light.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_search_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_search_bar_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_search_bar_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_search_bar_dark.png" /></td></tr></table>

ports iOSSearchBarPage.xaml (+ iOSSearchBarPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9977, 0.16% pixels differ · Dark: SSIM 0.9976, 0.16% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9976, 0.16% pixels differ · Dark: SSIM 0.9976, 0.16% pixels differ

### 95. Ios Slider Update On Tap — 🟢/🟢
<sub>ios_slider_update_on_tap</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_slider_update_on_tap_light.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_slider_update_on_tap_light.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_slider_update_on_tap_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_slider_update_on_tap_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_slider_update_on_tap_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_slider_update_on_tap_dark.png" /></td></tr></table>

ports iOSSliderUpdateOnTapPage.xaml (+ iOSSliderUpdateOnTapPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

### 96. Ios Swipe Transition — 🟢/🟢
<sub>ios_swipe_transition</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_swipe_transition_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/ios_swipe_transition_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/ios_swipe_transition_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_swipe_transition_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/ios_swipe_transition_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/ios_swipe_transition_dark.gif" /></td></tr></table>

ports iOSSwipeViewTransitionModePage.xaml (+ .xaml.cs) The C# iOSSwipeViewTransitionModePage is a StackLayout with: a horizontal row holding a &amp;quot;SwipeTransitionMode:&amp;quot; Label + an EnumPicker over the SwipeTransitionMode enum (Reveal / Drag, Se

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 97. Ios Time Picker — 🟢/🟢
<sub>ios_time_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/ios_time_picker_light.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_time_picker_light.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_time_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/ios_time_picker_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/ios_time_picker_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/ios_time_picker_dark.png" /></td></tr></table>

ports iOSTimePickerPage.xaml The .NET MAUI PlatformSpecifics sample (Pages/PlatformSpecifics/iOS/iOSTimePickerPage.xaml + .xaml.cs): a TimePicker carrying the iOSSpecific TimePicker.UpdateMode knob (XAML seeds it to WhenFinished) over a but

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9976, 0.11% pixels differ · Dark: SSIM 0.9975, 0.11% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9982, 0.08% pixels differ · Dark: SSIM 0.9980, 0.08% pixels differ

### 98. Items — 🟢/🟢
<sub>items</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/items_light.png" /></td><td><img width="300px" src="captures/ios/cpp/items_light.png" /></td><td><img width="300px" src="captures/ios/xaml/items_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/items_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/items_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/items_dark.png" /></td></tr></table>

a self-contained demo page for the W2-19 items core: a collection_view over a live observable items source with a templated cell, single selection driving a readout label, and an EmptyView for the cleared state (the C# CollectionView galler

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9985, 0.07% pixels differ · Dark: SSIM 0.9985, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9984, 0.07% pixels differ · Dark: SSIM 0.9989, 0.04% pixels differ

### 99. Items Updating Scroll Mode — 🟢/🟢
<sub>items_updating_scroll_mode</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/items_updating_scroll_mode_light.png" /></td><td><img width="300px" src="captures/ios/cpp/items_updating_scroll_mode_light.png" /></td><td><img width="300px" src="captures/ios/xaml/items_updating_scroll_mode_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/items_updating_scroll_mode_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/items_updating_scroll_mode_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/items_updating_scroll_mode_dark.png" /></td></tr></table>

ports ItemsUpdatingScrollModeGallery.xaml (+ .xaml.cs) of the C# CollectionView gallery (Maui.Controls.Sample.Pages.CollectionViewGalleries.ScrollModeGalleries.ItemsUpdatingScrollModeGallery)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9985, 0.07% pixels differ · Dark: SSIM 0.9985, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9990, 0.04% pixels differ · Dark: SSIM 0.9989, 0.04% pixels differ

### 100. Label — 🟢/🟢
<sub>label</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/label_light.png" /></td><td><img width="300px" src="captures/ios/cpp/label_light.png" /></td><td><img width="300px" src="captures/ios/xaml/label_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/label_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/label_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/label_dark.png" /></td></tr></table>

ports LabelPage.xaml (+ LabelPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9960, 0.16% pixels differ · Dark: SSIM 0.9959, 0.16% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9964, 0.13% pixels differ · Dark: SSIM 0.9964, 0.13% pixels differ

### 101. Layout Is Enabled — 🟢/🟢
<sub>layout_is_enabled</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/layout_is_enabled_light.png" /></td><td><img width="300px" src="captures/ios/cpp/layout_is_enabled_light.png" /></td><td><img width="300px" src="captures/ios/xaml/layout_is_enabled_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/layout_is_enabled_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/layout_is_enabled_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/layout_is_enabled_dark.png" /></td></tr></table>

ports LayoutIsEnabledPage.xaml (+ LayoutIsEnabledPage.xaml.cs) The C# page demonstrates how IsEnabled on a layout cascades to its children: a 2x2 grid whose left column hosts a &amp;quot;MainLayout&amp;quot; full of state-demo sub-stacks (all-enabled / all-d

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9824, 0.65% pixels differ · Dark: SSIM 0.9820, 0.65% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9990, 0.04% pixels differ · Dark: SSIM 0.9989, 0.04% pixels differ

### 102. Line Gallery — 🟢/🟢
<sub>line_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/line_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/line_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/line_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/line_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/line_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/line_gallery_dark.png" /></td></tr></table>

ports LineGallery.xaml A self-contained, code-first port of the MAUI Shapes LineGallery (Pages/Controls/ShapesGalleries/LineGallery.xaml + .xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9985, 0.07% pixels differ · Dark: SSIM 0.9985, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9990, 0.04% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 103. Line Join Gallery — 🟢/🟢
<sub>line_join_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/line_join_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/line_join_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/line_join_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/line_join_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/line_join_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/line_join_gallery_dark.png" /></td></tr></table>

ports LineJoinGallery.xaml A self-contained, code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/LineJoinGallery.xaml: a StackLayout that demonstrates the three StrokeLineJoin variants on an identical open polyline

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9986, 0.06% pixels differ · Dark: SSIM 0.9985, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 104. Measure First Strategy — 🟢/🟢
<sub>measure_first_strategy</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/measure_first_strategy_light.png" /></td><td><img width="300px" src="captures/ios/cpp/measure_first_strategy_light.png" /></td><td><img width="300px" src="captures/ios/xaml/measure_first_strategy_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/measure_first_strategy_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/measure_first_strategy_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/measure_first_strategy_dark.png" /></td></tr></table>

ports MeasureFirstStrategy.xaml (+ .xaml.cs) of the C# CollectionView gallery (Maui.Controls.Sample.Pages.CollectionViewGalleries.GroupingGalleries.MeasureFirstStrategy)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.05% pixels differ · Dark: SSIM 0.9988, 0.05% pixels differ

### 105. Menu Bar — 🟢/🟢
<sub>menu_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/menu_bar_light.png" /></td><td><img width="300px" src="captures/ios/cpp/menu_bar_light.png" /></td><td><img width="300px" src="captures/ios/xaml/menu_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/menu_bar_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/menu_bar_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/menu_bar_dark.png" /></td></tr></table>

ports MenuBarPage.xaml (+ MenuBarPage.cs) The C# page declares three page-level MenuBarItems (Page.MenuBarItems — the app menu bar) and a small visible body: - &amp;quot;Before File&amp;quot; : &amp;quot;Before File Action&amp;quot; (accelerator &amp;quot;b&amp;quot;), &amp;quot;Cool item 1&amp;quot;, a separat

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9934, 0.28% pixels differ · Dark: SSIM 0.9932, 0.28% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.05% pixels differ · Dark: SSIM 0.9988, 0.05% pixels differ

### 106. Modal — 🟢/🟢
<sub>modal</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/modal_light.png" /></td><td><img width="300px" src="captures/ios/cpp/modal_light.png" /></td><td><img width="300px" src="captures/ios/xaml/modal_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/modal_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/modal_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/modal_dark.png" /></td></tr></table>

ports ModalPage.xaml (+ .xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.05% pixels differ · Dark: SSIM 0.9988, 0.05% pixels differ

### 107. Multiple Bound Selection — 🟢/🟢
<sub>multiple_bound_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/multiple_bound_selection_light.png" /></td><td><img width="300px" src="captures/ios/cpp/multiple_bound_selection_light.png" /></td><td><img width="300px" src="captures/ios/xaml/multiple_bound_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/multiple_bound_selection_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/multiple_bound_selection_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/multiple_bound_selection_dark.png" /></td></tr></table>

ports MultipleBoundSelection.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.MultipleBoundSelection)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9980, 0.09% pixels differ · Dark: SSIM 0.9980, 0.09% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 108. Navigation Gallery — 🟢/🟢
<sub>navigation_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/navigation_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/navigation_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/navigation_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/navigation_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/navigation_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/navigation_gallery_dark.png" /></td></tr></table>

ports NavigationGallery.xaml (+ .xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.05% pixels differ · Dark: SSIM 0.9988, 0.05% pixels differ

### 109. Nested Collection — 🟡/🟡
<sub>nested_collection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/nested_collection_light.png" /></td><td><img width="300px" src="captures/ios/cpp/nested_collection_light.png" /></td><td><img width="300px" src="captures/ios/xaml/nested_collection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/nested_collection_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/nested_collection_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/nested_collection_dark.png" /></td></tr></table>

ports NestedGalleries/NestedCollectionViewGallery.xaml (+ NestedCollectionViewGallery.xaml.cs) of the C# CollectionView gallery

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9056, 3.70% pixels differ · Dark: SSIM 0.9242, 3.70% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9062, 3.68% pixels differ · Dark: SSIM 0.9247, 3.68% pixels differ

### 110. Pan Gesture Events — 🟢/🟢
<sub>pan_gesture_events</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/pan_gesture_events_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/pan_gesture_events_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/pan_gesture_events_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/pan_gesture_events_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/pan_gesture_events_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/pan_gesture_events_dark.gif" /></td></tr></table>

ports PanGestureEventsGallery.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.PanGestureEventsGallery)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9983, 0.08% pixels differ · Dark: SSIM 0.9995, 0.05% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9945, 0.25% pixels differ · Dark: SSIM 0.9950, 0.24% pixels differ

### 111. Path Aspect Gallery — 🟢/🟢
<sub>path_aspect_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/path_aspect_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/path_aspect_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/path_aspect_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/path_aspect_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/path_aspect_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/path_aspect_gallery_dark.png" /></td></tr></table>

ports PathAspectGallery.xaml A self-contained, code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PathAspectGallery.xaml: a StackLayout (Padding 12) that demonstrates the four Path Aspect modes on one identical ge

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9986, 0.07% pixels differ · Dark: SSIM 0.9985, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 112. Path Gallery — 🟢/🟢
<sub>path_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/path_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/path_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/path_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/path_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/path_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/path_gallery_dark.png" /></td></tr></table>

ports PathGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PathGallery.xaml: a ScrollView over a StackLayout (Padding 12) that walks eight Path variants (plus two caption-only markup-string Labels

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9986, 0.07% pixels differ · Dark: SSIM 0.9985, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 113. Path Transform String — 🟢/🟢
<sub>path_transform_string</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/path_transform_string_light.png" /></td><td><img width="300px" src="captures/ios/cpp/path_transform_string_light.png" /></td><td><img width="300px" src="captures/ios/xaml/path_transform_string_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/path_transform_string_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/path_transform_string_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/path_transform_string_dark.png" /></td></tr></table>

ports PathTransformStringGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PathTransformStringGallery.xaml: a ScrollView over a StackLayout (Padding 12) that shows the SAME two-figure Path geometry

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9860, 0.63% pixels differ · Dark: SSIM 0.9981, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9994, 0.03% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 114. Picker — 🟢/🟢
<sub>picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/picker_light.png" /></td><td><img width="300px" src="captures/ios/cpp/picker_light.png" /></td><td><img width="300px" src="captures/ios/xaml/picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/picker_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/picker_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/picker_dark.png" /></td></tr></table>

ports PickerPage.xaml (+ PickerPage.xaml.cs) A self-contained, code-first demo page for the Picker control (the C# gallery-page convention, mirroring the value_controls_page / pickers_page pattern)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9986, 0.07% pixels differ · Dark: SSIM 0.9985, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 115. Pickers — 🟢/🟢
<sub>pickers</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/pickers_light.png" /></td><td><img width="300px" src="captures/ios/cpp/pickers_light.png" /></td><td><img width="300px" src="captures/ios/xaml/pickers_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/pickers_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/pickers_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/pickers_dark.png" /></td></tr></table>

a self-contained demo page for the W1-06 picker set: picker, date_picker and time_picker on one vertical stack, wired together so every selection drives a visible output (the C# gallery-page convention, code-first; the value_controls_page p

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9911, 0.36% pixels differ · Dark: SSIM 0.9910, 0.36% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9962, 0.16% pixels differ · Dark: SSIM 0.9961, 0.16% pixels differ

### 116. Pointer Gesture — 🟢/🟢
<sub>pointer_gesture</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/pointer_gesture_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/pointer_gesture_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/pointer_gesture_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/pointer_gesture_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/pointer_gesture_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/pointer_gesture_dark.gif" /></td></tr></table>

ports PointerGestureGalleryPage.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.PointerGestureGalleryPage)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9997, 0.07% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9997, 0.07% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 117. Polygon Gallery — 🟢/🟢
<sub>polygon_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/polygon_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/polygon_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/polygon_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/polygon_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/polygon_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/polygon_gallery_dark.png" /></td></tr></table>

ports PolygonGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PolygonGallery.xaml: a ScrollView over a StackLayout (Padding 12) that walks four Polygon variants, each under a caption Label — - &amp;quot;A

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9990, 0.05% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 118. Polyline Gallery — 🟢/🟢
<sub>polyline_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/polyline_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/polyline_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/polyline_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/polyline_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/polyline_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/polyline_gallery_dark.png" /></td></tr></table>

ports PolylineGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PolylineGallery.xaml: a StackLayout (Padding 12 — no ScrollView in the C# source) holding two Polyline variants, each under a caption

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9837, 0.50% pixels differ · Dark: SSIM 0.9835, 0.50% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 119. Preselected Item — 🟢/🟢
<sub>preselected_item</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/preselected_item_light.png" /></td><td><img width="300px" src="captures/ios/cpp/preselected_item_light.png" /></td><td><img width="300px" src="captures/ios/xaml/preselected_item_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/preselected_item_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/preselected_item_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/preselected_item_dark.png" /></td></tr></table>

ports PreselectedItemGallery.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.PreselectedItemGallery)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9980, 0.09% pixels differ · Dark: SSIM 0.9980, 0.09% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 120. Preselected Items — 🟢/🟢
<sub>preselected_items</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/preselected_items_light.png" /></td><td><img width="300px" src="captures/ios/cpp/preselected_items_light.png" /></td><td><img width="300px" src="captures/ios/xaml/preselected_items_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/preselected_items_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/preselected_items_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/preselected_items_dark.png" /></td></tr></table>

ports PreselectedItemsGallery.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.PreselectedItemsGallery)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9990, 0.05% pixels differ · Dark: SSIM 0.9990, 0.05% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 121. Progress Bar — 🟢/🟢
<sub>progress_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/progress_bar_light.png" /></td><td><img width="300px" src="captures/ios/cpp/progress_bar_light.png" /></td><td><img width="300px" src="captures/ios/xaml/progress_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/progress_bar_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/progress_bar_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/progress_bar_dark.png" /></td></tr></table>

ports ProgressBarPage.xaml (+ ProgressBarPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9990, 0.05% pixels differ · Dark: SSIM 0.9990, 0.05% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 122. Radio Button Border — 🟡/🟡
<sub>radio_button_border</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/radio_button_border_light.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_button_border_light.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_button_border_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/radio_button_border_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_button_border_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_button_border_dark.png" /></td></tr></table>

ports RadioButtonBorder.xaml A self-contained, code-first demo of RadioButton border styling

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9248, 5.79% pixels differ · Dark: SSIM 0.9079, 5.78% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9248, 5.78% pixels differ · Dark: SSIM 0.9079, 5.78% pixels differ

### 123. Radio Button Content — 🟡/🟡
<sub>radio_button_content</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/radio_button_content_light.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_button_content_light.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_button_content_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/radio_button_content_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_button_content_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_button_content_dark.png" /></td></tr></table>

ports RadioButtonContentGallery.xaml A self-contained, code-first demo of the RadioButton.Content surface

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9544, 2.17% pixels differ · Dark: SSIM 0.9561, 2.00% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9544, 2.17% pixels differ · Dark: SSIM 0.9561, 2.00% pixels differ

### 124. Radio Button Group — 🟢/🟢
<sub>radio_button_group</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/radio_button_group_light.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_button_group_light.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_button_group_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/radio_button_group_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_button_group_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_button_group_dark.png" /></td></tr></table>

ports RadioButtonGroupGallery.xaml A self-contained, code-first demo of the RadioButtonGroup ATTACHED-PROPERTY grouping: a vertical StackLayout carries RadioButtonGroup.GroupName=&amp;quot;foo&amp;quot;, so every descendant RadioButton — including one nested

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9821, 0.71% pixels differ · Dark: SSIM 0.9822, 0.71% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9820, 0.72% pixels differ · Dark: SSIM 0.9821, 0.72% pixels differ

### 125. Radio Button Group Binding — 🟢/🟢
<sub>radio_button_group_binding</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/radio_button_group_binding_light.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_button_group_binding_light.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_button_group_binding_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/radio_button_group_binding_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_button_group_binding_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_button_group_binding_dark.png" /></td></tr></table>

ports RadioButtonGroupBindingGallery.xaml A code-first demo of binding the RadioButtonGroup attached properties (GroupName + SelectedValue) to a view-model

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9820, 0.72% pixels differ · Dark: SSIM 0.9822, 0.71% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9820, 0.72% pixels differ · Dark: SSIM 0.9821, 0.72% pixels differ

### 126. Radio Button Group Gallery — 🟡/🟡
<sub>radio_button_group_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/radio_button_group_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_button_group_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_button_group_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/radio_button_group_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_button_group_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_button_group_gallery_dark.png" /></td></tr></table>

ports RadioButtonGroupGalleryPage.xaml A self-contained, code-first demo of RadioButton grouping SCOPE, mirroring the C# controls gallery page (Pages/Controls/RadioButtonGalleries/RadioButtonGroupGalleryPage.xaml)

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9005, 4.06% pixels differ · Dark: SSIM 0.9010, 4.05% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9005, 4.06% pixels differ · Dark: SSIM 0.9010, 4.06% pixels differ

### 127. Radio Content Properties — 🟡/🟡
<sub>radio_content_properties</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/radio_content_properties_light.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_content_properties_light.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_content_properties_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/radio_content_properties_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_content_properties_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_content_properties_dark.png" /></td></tr></table>

ports ContentProperties.xaml A self-contained, code-first demo of how RadioButton propagates the standard Text/Font properties to its Content

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9493, 2.13% pixels differ · Dark: SSIM 0.9509, 2.08% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9492, 2.14% pixels differ · Dark: SSIM 0.9509, 2.08% pixels differ

### 128. Radio Template From Style — 🟢/🟢
<sub>radio_template_from_style</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/radio_template_from_style_light.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_template_from_style_light.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_template_from_style_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/radio_template_from_style_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/radio_template_from_style_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/radio_template_from_style_dark.png" /></td></tr></table>

ports TemplateFromStyle.xaml A self-contained, code-first demo of applying a RadioButton ControlTemplate

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9919, 0.20% pixels differ · Dark: SSIM 0.9881, 0.37% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9962, 0.09% pixels differ · Dark: SSIM 0.9913, 0.32% pixels differ

### 129. Rectangle Gallery — 🟢/🟢
<sub>rectangle_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/rectangle_gallery_light.png" /></td><td><img width="300px" src="captures/ios/cpp/rectangle_gallery_light.png" /></td><td><img width="300px" src="captures/ios/xaml/rectangle_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/rectangle_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/rectangle_gallery_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/rectangle_gallery_dark.png" /></td></tr></table>

ports RectangleGallery.xaml A self-contained, code-first port of the MAUI Shapes RectangleGallery (Pages/Controls/ShapesGalleries/RectangleGallery.xaml + .xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9984, 0.08% pixels differ · Dark: SSIM 0.9983, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 130. Refresh View — 🟢/🟢
<sub>refresh_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/refresh_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/refresh_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/refresh_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/refresh_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/refresh_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/refresh_view_dark.png" /></td></tr></table>

ports RefreshViewPage.xaml (+ RefreshViewPage.xaml.cs + RefreshViewModel.cs) A self-contained, code-first demo page for the RefreshView control (the C# gallery-page convention, mirroring the swipe_refresh_page pattern)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9984, 0.08% pixels differ · Dark: SSIM 0.9983, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 131. Relative Layout — 🟢/🟢
<sub>relative_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/relative_layout_light.png" /></td><td><img width="300px" src="captures/ios/cpp/relative_layout_light.png" /></td><td><img width="300px" src="captures/ios/xaml/relative_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/relative_layout_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/relative_layout_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/relative_layout_dark.png" /></td></tr></table>

ports RelativeLayoutPage.xaml

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9990, 0.05% pixels differ · Dark: SSIM 0.9990, 0.05% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 132. Scattered Radio Button — 🟢/🟢
<sub>scattered_radio_button</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/scattered_radio_button_light.png" /></td><td><img width="300px" src="captures/ios/cpp/scattered_radio_button_light.png" /></td><td><img width="300px" src="captures/ios/xaml/scattered_radio_button_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/scattered_radio_button_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/scattered_radio_button_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/scattered_radio_button_dark.png" /></td></tr></table>

ports ScatteredRadioButtonGallery.xaml A code-first demo that radio buttons DON&amp;#x27;T have to share a container to be grouped: grouping is by GroupName, so buttons scattered across separate containers (and one bare button outside any grouped co

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9818, 0.78% pixels differ · Dark: SSIM 0.9848, 0.60% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9818, 0.78% pixels differ · Dark: SSIM 0.9848, 0.60% pixels differ

### 133. Scroll Mode Test — 🟢/🟢
<sub>scroll_mode_test</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/scroll_mode_test_light.png" /></td><td><img width="300px" src="captures/ios/cpp/scroll_mode_test_light.png" /></td><td><img width="300px" src="captures/ios/xaml/scroll_mode_test_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/scroll_mode_test_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/scroll_mode_test_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/scroll_mode_test_dark.png" /></td></tr></table>

ports ScrollModeTestGallery.xaml (+ .xaml.cs) of the C# CollectionView gallery (Maui.Controls.Sample.Pages.CollectionViewGalleries.ScrollModeGalleries.ScrollModeTestGallery)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9954, 0.17% pixels differ · Dark: SSIM 0.9953, 0.17% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 134. Scroll To Group — 🟢/🟢
<sub>scroll_to_group</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/scroll_to_group_light.png" /></td><td><img width="300px" src="captures/ios/cpp/scroll_to_group_light.png" /></td><td><img width="300px" src="captures/ios/xaml/scroll_to_group_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/scroll_to_group_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/scroll_to_group_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/scroll_to_group_dark.png" /></td></tr></table>

ports ScrollToGalleries/ScrollToGroup.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9990, 0.05% pixels differ · Dark: SSIM 0.9990, 0.05% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 135. Scroll View — 🟢/🟢
<sub>scroll_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/scroll_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/scroll_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/scroll_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/scroll_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/scroll_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/scroll_view_dark.png" /></td></tr></table>

ports ScrollViewPage.xaml (+ the ScrollViewPages sub-demos: ScrollViewOrientationPage / ScrollToEndPage / ScrollToFromConstructorPage), code-first

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9981, 0.08% pixels differ · Dark: SSIM 0.9980, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 136. Search Bar — 🟢/🟢
<sub>search_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/search_bar_light.png" /></td><td><img width="300px" src="captures/ios/cpp/search_bar_light.png" /></td><td><img width="300px" src="captures/ios/xaml/search_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/search_bar_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/search_bar_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/search_bar_dark.png" /></td></tr></table>

ports SearchBarPage.xaml (Microsoft.Maui.Controls sample gallery)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9976, 0.19% pixels differ · Dark: SSIM 0.9973, 0.20% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9971, 0.21% pixels differ · Dark: SSIM 0.9971, 0.20% pixels differ

### 137. Selection Command Param — 🟢/🟢
<sub>selection_command_param</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/selection_command_param_light.png" /></td><td><img width="300px" src="captures/ios/cpp/selection_command_param_light.png" /></td><td><img width="300px" src="captures/ios/xaml/selection_command_param_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/selection_command_param_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/selection_command_param_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/selection_command_param_dark.png" /></td></tr></table>

ports SelectionChangedCommandParameter.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.SelectionChangedCommandParameter)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9984, 0.07% pixels differ · Dark: SSIM 0.9983, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.05% pixels differ · Dark: SSIM 0.9988, 0.05% pixels differ

### 138. Selection Synchronization — 🟢/🟢
<sub>selection_synchronization</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/selection_synchronization_light.png" /></td><td><img width="300px" src="captures/ios/cpp/selection_synchronization_light.png" /></td><td><img width="300px" src="captures/ios/xaml/selection_synchronization_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/selection_synchronization_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/selection_synchronization_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/selection_synchronization_dark.png" /></td></tr></table>

ports SelectionSynchronization.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.SelectionSynchronization)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9993, 0.04% pixels differ · Dark: SSIM 0.9993, 0.04% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.05% pixels differ · Dark: SSIM 0.9988, 0.05% pixels differ

### 139. Semantics — 🟢/🟢
<sub>semantics</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/semantics_light.png" /></td><td><img width="300px" src="captures/ios/cpp/semantics_light.png" /></td><td><img width="300px" src="captures/ios/xaml/semantics_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/semantics_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/semantics_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/semantics_dark.png" /></td></tr></table>

ports SemanticsPage.xaml (+ SemanticsPage.xaml.cs) The C# SemanticsPage is an accessibility showcase: a long VerticalStackLayout where nearly every control carries SemanticProperties.Description / .Hint, plus a block of labels exercising Se

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9993, 0.04% pixels differ · Dark: SSIM 0.9993, 0.04% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.05% pixels differ · Dark: SSIM 0.9988, 0.05% pixels differ

### 140. Shadow Playground — 🟢/🟢
<sub>shadow_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/shadow_playground_light.png" /></td><td><img width="300px" src="captures/ios/cpp/shadow_playground_light.png" /></td><td><img width="300px" src="captures/ios/xaml/shadow_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/shadow_playground_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/shadow_playground_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/shadow_playground_dark.png" /></td></tr></table>

ports ShadowPlaygroundPage.xaml A self-contained, code-first demo of the view Shadow surface, mirroring the C# core gallery page (Pages/Core/ShadowGalleries/ShadowPlaygroundPage.xaml + .xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9993, 0.04% pixels differ · Dark: SSIM 0.9993, 0.04% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.05% pixels differ · Dark: SSIM 0.9988, 0.05% pixels differ

### 141. Shape App Theme — 🟢/🟢
<sub>shape_app_theme</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/shape_app_theme_light.png" /></td><td><img width="300px" src="captures/ios/cpp/shape_app_theme_light.png" /></td><td><img width="300px" src="captures/ios/xaml/shape_app_theme_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/shape_app_theme_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/shape_app_theme_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/shape_app_theme_dark.png" /></td></tr></table>

ports ShapeAppThemeGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/ShapeAppThemeGallery.xaml: a StackLayout (Padding 12) holding a caption Label and a 200x80 Rectangle, all themed via {AppThemeBi

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9993, 0.04% pixels differ · Dark: SSIM 0.9993, 0.04% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.05% pixels differ · Dark: SSIM 0.9988, 0.05% pixels differ

### 142. Shapes — 🟢/🟢
<sub>shapes</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/shapes_light.png" /></td><td><img width="300px" src="captures/ios/cpp/shapes_light.png" /></td><td><img width="300px" src="captures/ios/xaml/shapes_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/shapes_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/shapes_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/shapes_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;shapes&amp;quot; demo (ComparePages.Shapes()), the shipped-.NET-MAUI reference for the visual-parity comparison: a ScrollView over a vertical stack of four LABELLED shapes, each bold-captioned and Start-a

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9993, 0.04% pixels differ · Dark: SSIM 0.9993, 0.04% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.05% pixels differ · Dark: SSIM 0.9988, 0.05% pixels differ

### 143. Single Bound Selection — 🟢/🟢
<sub>single_bound_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/single_bound_selection_light.png" /></td><td><img width="300px" src="captures/ios/cpp/single_bound_selection_light.png" /></td><td><img width="300px" src="captures/ios/xaml/single_bound_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/single_bound_selection_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/single_bound_selection_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/single_bound_selection_dark.png" /></td></tr></table>

ports SingleBoundSelection.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.SingleBoundSelection)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9993, 0.04% pixels differ · Dark: SSIM 0.9988, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.07% pixels differ · Dark: SSIM 0.9986, 0.07% pixels differ

### 144. Slider — 🟢/🟢
<sub>slider</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/slider_light.png" /></td><td><img width="300px" src="captures/ios/cpp/slider_light.png" /></td><td><img width="300px" src="captures/ios/xaml/slider_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/slider_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/slider_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/slider_dark.png" /></td></tr></table>

ports SliderPage.xaml (+ .xaml.cs) Mirrors the MAUI gallery page: a vertical stack of headlined Slider states — Default, BackgroundColor (Blue), Background (yellow→green LinearGradientBrush), Minimum(5)/Maximum(15) with a value readout (Val

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.07% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

### 145. Some Empty Groups — 🟢/🟢
<sub>some_empty_groups</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/some_empty_groups_light.png" /></td><td><img width="300px" src="captures/ios/cpp/some_empty_groups_light.png" /></td><td><img width="300px" src="captures/ios/xaml/some_empty_groups_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/some_empty_groups_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/some_empty_groups_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/some_empty_groups_dark.png" /></td></tr></table>

ports GroupingGalleries/SomeEmptyGroups.xaml (+ .xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9895, 0.51% pixels differ · Dark: SSIM 0.9882, 0.56% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

### 146. Stack Layout — 🟢/🟢
<sub>stack_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/stack_layout_light.png" /></td><td><img width="300px" src="captures/ios/cpp/stack_layout_light.png" /></td><td><img width="300px" src="captures/ios/xaml/stack_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/stack_layout_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/stack_layout_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/stack_layout_dark.png" /></td></tr></table>

ports StackLayoutPage.xaml Demonstrates the generic maui::controls::stack_layout (the orientation-switching sibling of the fixed vertical/horizontal stacks) by nesting two inner stacks inside an outer vertical stack with a 12px margin: a &amp;quot;V

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

### 147. Staggered Layout — 🟢/🟢
<sub>staggered_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/staggered_layout_light.png" /></td><td><img width="300px" src="captures/ios/cpp/staggered_layout_light.png" /></td><td><img width="300px" src="captures/ios/xaml/staggered_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/staggered_layout_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/staggered_layout_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/staggered_layout_dark.png" /></td></tr></table>

ports AlternateLayoutGalleries/StaggeredLayout.xaml (+ StaggeredLayout.xaml.cs) of the C# CollectionView gallery

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9983, 0.08% pixels differ · Dark: SSIM 0.9983, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

### 148. Stepper — 🟢/🟢
<sub>stepper</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/stepper_light.png" /></td><td><img width="300px" src="captures/ios/cpp/stepper_light.png" /></td><td><img width="300px" src="captures/ios/xaml/stepper_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/stepper_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/stepper_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/stepper_dark.png" /></td></tr></table>

ports StepperPage.xaml (+ StepperPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

### 149. Styles — 🟢/🟢
<sub>styles</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/styles_light.png" /></td><td><img width="300px" src="captures/ios/cpp/styles_light.png" /></td><td><img width="300px" src="captures/ios/xaml/styles_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/styles_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/styles_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/styles_dark.png" /></td></tr></table>

ports StylesPage.xaml

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

### 150. Swipe Gesture — 🟢/🟢
<sub>swipe_gesture</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/swipe_gesture_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/swipe_gesture_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/swipe_gesture_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/swipe_gesture_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/swipe_gesture_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/swipe_gesture_dark.gif" /></td></tr></table>

ports SwipeViewGestureRecognizerGallery.xaml (+ .xaml.cs) The MAUI SwipeViewGestureRecognizerGallery is a CollectionView of &amp;quot;message&amp;quot; rows; each row&amp;#x27;s DataTemplate is a SwipeView wired three ways, proving gesture recognizers AND swipe-item

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9983, 0.10% pixels differ · Dark: SSIM 0.9982, 0.10% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9990, 0.07% pixels differ · Dark: SSIM 0.9989, 0.07% pixels differ

### 151. Swipe Item Position — 🟢/🟢
<sub>swipe_item_position</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/swipe_item_position_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/swipe_item_position_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/swipe_item_position_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/swipe_item_position_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/swipe_item_position_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/swipe_item_position_dark.gif" /></td></tr></table>

ports SwipeItemPositionGallery.xaml A code-first port of the MAUI SwipeView sub-gallery Pages/Controls/SwipeViewGalleries/SwipeItemPositionGallery.xaml: a 2-row Grid (Auto / *) with a Picker on top and one SwipeView below that carries TWO S

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9984, 0.08% pixels differ · Dark: SSIM 0.9995, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9947, 0.19% pixels differ · Dark: SSIM 0.9939, 0.20% pixels differ

### 152. Swipe Item Size — 🟢/🟢
<sub>swipe_item_size</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/swipe_item_size_light.png" /></td><td><img width="300px" src="captures/ios/cpp/swipe_item_size_light.png" /></td><td><img width="300px" src="captures/ios/xaml/swipe_item_size_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/swipe_item_size_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/swipe_item_size_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/swipe_item_size_dark.png" /></td></tr></table>

ports SwipeItemSizeGallery.xaml A self-contained, code-first port of the .NET MAUI &amp;quot;SwipeItem Size Gallery&amp;quot;: a scrolling stack of swipe_views demonstrating how a left SwipeItem&amp;#x27;s icon size and the SwipeView content size interact

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9993, 0.03% pixels differ · Dark: SSIM 0.9993, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

### 153. Swipe Refresh — 🟢/🟢
<sub>swipe_refresh</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/swipe_refresh_light.gif" /></td><td><img width="300px" src="captures/ios/cpp/swipe_refresh_light.gif" /></td><td><img width="300px" src="captures/ios/xaml/swipe_refresh_light.gif" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/swipe_refresh_dark.gif" /></td><td><img width="300px" src="captures/ios/cpp/swipe_refresh_dark.gif" /></td><td><img width="300px" src="captures/ios/xaml/swipe_refresh_dark.gif" /></td></tr></table>

a self-contained demo page for the W2-20 swipe + refresh controls: a refresh_view wrapping a swipe_view (which itself wraps a labeled row), with a readout label reflecting the latest interaction

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9948, 0.21% pixels differ · Dark: SSIM 0.9961, 0.15% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9953, 0.19% pixels differ · Dark: SSIM 0.9958, 0.17% pixels differ

### 154. Swipe Threshold — 🟢/🟢
<sub>swipe_threshold</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/swipe_threshold_light.png" /></td><td><img width="300px" src="captures/ios/cpp/swipe_threshold_light.png" /></td><td><img width="300px" src="captures/ios/xaml/swipe_threshold_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/swipe_threshold_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/swipe_threshold_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/swipe_threshold_dark.png" /></td></tr></table>

ports HorizontalSwipeThresholdGallery.xaml (+ .xaml.cs) The MAUI HorizontalSwipeThresholdGallery shows how SwipeView.Threshold (the swipe distance, in DIPs, the user must drag before the items settle open / execute) interacts with SwipeItem

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9993, 0.03% pixels differ · Dark: SSIM 0.9993, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

### 155. Swipe View Margin — 🟢/🟢
<sub>swipe_view_margin</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/swipe_view_margin_light.png" /></td><td><img width="300px" src="captures/ios/cpp/swipe_view_margin_light.png" /></td><td><img width="300px" src="captures/ios/xaml/swipe_view_margin_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/swipe_view_margin_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/swipe_view_margin_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/swipe_view_margin_dark.png" /></td></tr></table>

ports SwipeViewMarginGallery.xaml A self-contained, code-first port of the .NET MAUI &amp;quot;SwipeView Margin Gallery&amp;quot;: two swipe_views whose content&amp;#x27;s Margin + Padding are driven by two sliders, demonstrating that the revealed SwipeItems stay cor

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9993, 0.03% pixels differ · Dark: SSIM 0.9993, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.06% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

### 156. Swipe View Shadow — 🟢/🟢
<sub>swipe_view_shadow</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/swipe_view_shadow_light.png" /></td><td><img width="300px" src="captures/ios/cpp/swipe_view_shadow_light.png" /></td><td><img width="300px" src="captures/ios/xaml/swipe_view_shadow_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/swipe_view_shadow_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/swipe_view_shadow_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/swipe_view_shadow_dark.png" /></td></tr></table>

ports SwipeViewShadowGallery.xaml A code-first port of the MAUI SwipeView sub-gallery Pages/Controls/SwipeViewGalleries/SwipeViewShadowGallery.xaml: a padded vertical StackLayout proving a drop Shadow renders correctly on SwipeView content

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9832, 0.69% pixels differ · Dark: SSIM 0.9993, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9830, 0.72% pixels differ · Dark: SSIM 0.9986, 0.06% pixels differ

### 157. Switch — 🟢/🟢
<sub>switch</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/switch_light.png" /></td><td><img width="300px" src="captures/ios/cpp/switch_light.png" /></td><td><img width="300px" src="captures/ios/xaml/switch_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/switch_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/switch_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/switch_dark.png" /></td></tr></table>

ports SwitchPage.xaml (+ .xaml.cs) Mirrors the MAUI gallery page: a vertical stack of headlined Switch states — Default, BackgroundColor (Blue), Background (a yellow→green LinearGradientBrush), Disabled, OnColor (Red), ThumbColor (Orange)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9979, 0.26% pixels differ · Dark: SSIM 0.9990, 0.26% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9972, 0.28% pixels differ · Dark: SSIM 0.9983, 0.28% pixels differ

### 158. Switch Grouping — 🟢/🟢
<sub>switch_grouping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/switch_grouping_light.png" /></td><td><img width="300px" src="captures/ios/cpp/switch_grouping_light.png" /></td><td><img width="300px" src="captures/ios/xaml/switch_grouping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/switch_grouping_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/switch_grouping_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/switch_grouping_dark.png" /></td></tr></table>

ports CollectionViewGalleries/GroupingGalleries/ SwitchGrouping.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9995, 0.03% pixels differ · Dark: SSIM 0.9995, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 159. Tabbed Flyout — 🟢/🟢
<sub>tabbed_flyout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/tabbed_flyout_light.png" /></td><td><img width="300px" src="captures/ios/cpp/tabbed_flyout_light.png" /></td><td><img width="300px" src="captures/ios/xaml/tabbed_flyout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/tabbed_flyout_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/tabbed_flyout_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/tabbed_flyout_dark.png" /></td></tr></table>

a self-contained demo page for the W1-10 tabbed + flyout vertical: a flyout_page whose FLYOUT pane is a titled menu (two buttons selecting the detail&amp;#x27;s tabs + a &amp;quot;Toggle flyout&amp;quot; presenting/dismissing itself) and whose DETAIL pane is a tabbed

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9995, 0.03% pixels differ · Dark: SSIM 0.9995, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 160. Templated View — 🟢/🟢
<sub>templated_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/templated_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/templated_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/templated_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/templated_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/templated_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/templated_view_dark.png" /></td></tr></table>

ports TemplatedViewPage.xaml The C# page contrasts a standard CardView control with a compact one driven by a ControlTemplate (&amp;quot;CardViewCompressed&amp;quot;) and a custom Rate control built entirely from a ControlTemplate + a heart PathGeometry

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9995, 0.03% pixels differ · Dark: SSIM 0.9995, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 161. Time Picker — 🟢/🟢
<sub>time_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/time_picker_light.png" /></td><td><img width="300px" src="captures/ios/cpp/time_picker_light.png" /></td><td><img width="300px" src="captures/ios/xaml/time_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/time_picker_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/time_picker_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/time_picker_dark.png" /></td></tr></table>

ports TimePickerPage.xaml (+ TimePickerPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9859, 0.67% pixels differ · Dark: SSIM 0.9858, 0.68% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9865, 0.68% pixels differ · Dark: SSIM 0.9864, 0.68% pixels differ

### 162. Title Bar — 🟢/🟢
<sub>title_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/title_bar_light.png" /></td><td><img width="300px" src="captures/ios/cpp/title_bar_light.png" /></td><td><img width="300px" src="captures/ios/xaml/title_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/title_bar_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/title_bar_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/title_bar_dark.png" /></td></tr></table>

ports TitleBarPage.xaml A self-contained, code-first demo of the TitleBar control

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9995, 0.03% pixels differ · Dark: SSIM 0.9995, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9991, 0.04% pixels differ

### 163. Toolbar — 🟢/🟢
<sub>toolbar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/toolbar_light.png" /></td><td><img width="300px" src="captures/ios/cpp/toolbar_light.png" /></td><td><img width="300px" src="captures/ios/xaml/toolbar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/toolbar_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/toolbar_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/toolbar_dark.png" /></td></tr></table>

ports ToolbarPage.xaml (Maui.Controls.Sample.Pages.ToolbarPage)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9992, 0.05% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9992, 0.04% pixels differ · Dark: SSIM 0.9990, 0.04% pixels differ

### 164. Transform Playground — 🟢/🟢
<sub>transform_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/transform_playground_light.png" /></td><td><img width="300px" src="captures/ios/cpp/transform_playground_light.png" /></td><td><img width="300px" src="captures/ios/xaml/transform_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/transform_playground_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/transform_playground_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/transform_playground_dark.png" /></td></tr></table>

ports TransformPlaygroundGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/TransformPlaygroundGallery.xaml: a 50x50 Path rectangle (red fill, blue stroke 4) sits in a 200x200 light-grey panel; belo

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9956, 0.28% pixels differ · Dark: SSIM 0.9956, 0.28% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9990, 0.04% pixels differ · Dark: SSIM 0.9990, 0.04% pixels differ

### 165. Transformations — 🟢/🟢
<sub>transformations</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/transformations_light.png" /></td><td><img width="300px" src="captures/ios/cpp/transformations_light.png" /></td><td><img width="300px" src="captures/ios/xaml/transformations_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/transformations_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/transformations_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/transformations_dark.png" /></td></tr></table>

ports TransformationsPage.xaml (+ .xaml.cs) The MAUI TransformationsPage drives a single target view&amp;#x27;s render transforms from a column of knobs: Sliders for Scale / ScaleX / ScaleY (Maximum 10) and Rotation / RotationX / RotationY (Maximum

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9990, 0.04% pixels differ · Dark: SSIM 0.9990, 0.04% pixels differ

### 166. Triggers — 🟢/🟢
<sub>triggers</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/triggers_light.png" /></td><td><img width="300px" src="captures/ios/cpp/triggers_light.png" /></td><td><img width="300px" src="captures/ios/xaml/triggers_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/triggers_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/triggers_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/triggers_dark.png" /></td></tr></table>

ports TriggersPage.xaml

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9988, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9990, 0.04% pixels differ · Dark: SSIM 0.9990, 0.04% pixels differ

### 167. Update Path Data — 🟢/🟢
<sub>update_path_data</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/update_path_data_light.png" /></td><td><img width="300px" src="captures/ios/cpp/update_path_data_light.png" /></td><td><img width="300px" src="captures/ios/xaml/update_path_data_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/update_path_data_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/update_path_data_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/update_path_data_dark.png" /></td></tr></table>

ports UpdatePathDataGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/UpdatePathDataGallery.xaml: a 2-row Grid (RowSpacing 0) that proves a Path repaints when its Data geometry is replaced at runti

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9985, 0.07% pixels differ · Dark: SSIM 0.9984, 0.07% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.07% pixels differ · Dark: SSIM 0.9986, 0.07% pixels differ

### 168. Varied Size Selector — 🟡/🟡
<sub>varied_size_selector</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/varied_size_selector_light.png" /></td><td><img width="300px" src="captures/ios/cpp/varied_size_selector_light.png" /></td><td><img width="300px" src="captures/ios/xaml/varied_size_selector_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/varied_size_selector_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/varied_size_selector_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/varied_size_selector_dark.png" /></td></tr></table>

ports DataTemplateSelectorGalleries/VariedSizeDataTemplateSelectorGallery.xaml (+ VariedSizeDataTemplateSelectorGallery.xaml.cs) of the C# CollectionView gallery

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9641, 1.58% pixels differ · Dark: SSIM 0.9535, 1.51% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9720, 1.20% pixels differ · Dark: SSIM 0.9593, 1.20% pixels differ

### 169. Vertical Stack — 🟢/🟢
<sub>vertical_stack</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/vertical_stack_light.png" /></td><td><img width="300px" src="captures/ios/cpp/vertical_stack_light.png" /></td><td><img width="300px" src="captures/ios/xaml/vertical_stack_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/vertical_stack_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/vertical_stack_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/vertical_stack_dark.png" /></td></tr></table>

Vertical Stack

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9980, 0.09% pixels differ · Dark: SSIM 0.9981, 0.09% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9986, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 170. Visual States — 🟢/🟢
<sub>visual_states</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/visual_states_light.png" /></td><td><img width="300px" src="captures/ios/cpp/visual_states_light.png" /></td><td><img width="300px" src="captures/ios/xaml/visual_states_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/visual_states_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/visual_states_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/visual_states_dark.png" /></td></tr></table>

ports VisualStatesPage.xaml

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9995, 0.03% pixels differ · Dark: SSIM 0.9994, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 171. Web View — 🟢/🟢
<sub>web_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/web_view_light.png" /></td><td><img width="300px" src="captures/ios/cpp/web_view_light.png" /></td><td><img width="300px" src="captures/ios/xaml/web_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/web_view_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/web_view_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/web_view_dark.png" /></td></tr></table>

a self-contained demo page for the W1-08 web_view vertical: a web_view loading a STATIC html_web_view_source (no network), back/forward/reload buttons over the handler-pushed CanGoBack/CanGoForward read-onlys, an &amp;quot;Eval 1+1&amp;quot; button driving t

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9956, 0.14% pixels differ · Dark: SSIM 0.9955, 0.14% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

### 172. Z Index — 🟢/🟢
<sub>z_index</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/ios/maui/z_index_light.png" /></td><td><img width="300px" src="captures/ios/cpp/z_index_light.png" /></td><td><img width="300px" src="captures/ios/xaml/z_index_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/ios/maui/z_index_dark.png" /></td><td><img width="300px" src="captures/ios/cpp/z_index_dark.png" /></td><td><img width="300px" src="captures/ios/xaml/z_index_dark.png" /></td></tr></table>

ports ZIndexPage.xaml (+ ZIndexPage.xaml.cs), code-first

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9995, 0.03% pixels differ · Dark: SSIM 0.9994, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9987, 0.06% pixels differ · Dark: SSIM 0.9987, 0.06% pixels differ

</details>

<details>
<summary><h2>macOS (172 examples) — click to expand</h2></summary>

.NET MAUI on macOS **is** Mac Catalyst (UIKit) — the MAUI / C++ / C++&amp;XAML columns are the strict parity board. The **AppKit** columns are the native-NSView backend (no MAUI reference; they track completeness, C++ == C++&amp;XAML).

**Discrepancy counts** (MAUI-vs-C++ parity verdicts from the deterministic pixel-perfect score — SSIM + per-pixel diff; AI-based review has been invalidated/removed):

| Classification | Pixel-Perfect Score — C++ (C1/C3) | Pixel-Perfect Score — C++ &amp; XAML (C2/C4) |
| --- | --- | --- |
| 🟢 Match | 162 | 161 |
| 🟡 Minor | 10 | 11 |
| 🔴 Major | 0 | 0 |
| ⬛ Blank | 0 | 0 |
| ⏳ Unreviewed | 0 | 0 |

### 1. Absolute Layout — 🟢/🟢
<sub>absolute_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/absolute_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/absolute_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/absolute_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/absolute_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/absolute_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/absolute_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/absolute_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/absolute_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/absolute_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/absolute_layout_dark.png" /></td></tr></table>

ports AbsoluteLayoutPage.xaml A self-contained, code-first demo of the AbsoluteLayout control

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9947, 0.26% pixels differ · Dark: SSIM 0.9960, 0.27% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 2. Activity Indicator — 🟢/🟢
<sub>activity_indicator</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/activity_indicator_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/activity_indicator_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/activity_indicator_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/activity_indicator_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/activity_indicator_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/activity_indicator_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/activity_indicator_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/activity_indicator_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/activity_indicator_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/activity_indicator_dark.png" /></td></tr></table>

ports ActivityIndicatorPage.xaml (+ ActivityIndicatorPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9975, 0.09% pixels differ · Dark: SSIM 0.9974, 0.11% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9963, 0.13% pixels differ · Dark: SSIM 0.9963, 0.13% pixels differ

### 3. Adaptive Collection — 🟢/🟢
<sub>adaptive_collection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/adaptive_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/adaptive_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/adaptive_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/adaptive_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/adaptive_collection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/adaptive_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/adaptive_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/adaptive_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/adaptive_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/adaptive_collection_dark.png" /></td></tr></table>

ports AdaptiveCollectionView.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.AdaptiveCollectionView)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9970, 0.19% pixels differ · Dark: SSIM 0.9970, 0.18% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 4. Alerts — 🟢/🟢
<sub>alerts</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/alerts_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/alerts_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/alerts_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/alerts_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/alerts_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/alerts_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/alerts_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/alerts_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/alerts_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/alerts_dark.png" /></td></tr></table>

ports AlertsPage.xaml (+ AlertsPage.xaml.cs) The C# AlertsPage drives the three Page dialog services — DisplayAlertAsync (simple OK + Yes/No), DisplayActionSheetAsync (simple + Cancel/Delete), and DisplayPromptAsync (two questions) — from a

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 5. Alignment — 🟢/🟢
<sub>alignment</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/alignment_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/alignment_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/alignment_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/alignment_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/alignment_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/alignment_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/alignment_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/alignment_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/alignment_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/alignment_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;alignment&amp;quot; demo (ComparePages.Alignment()), the shipped-.NET-MAUI reference for the visual-parity comparison

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9960, 0.48% pixels differ · Dark: SSIM 0.9966, 0.46% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9947, 0.52% pixels differ · Dark: SSIM 0.9953, 0.50% pixels differ

### 6. Animation — 🟢/🟡
<sub>animation</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/animation_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/animation_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/animation_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/animation_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/animation_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/animation_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/animation_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/animation_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/animation_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/animation_dark.png" /></td></tr></table>

ports AnimationPage.xaml (+ AnimationPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9770, 0.91% pixels differ · Dark: SSIM 0.9797, 1.03% pixels differ

### 7. App Theme Binding — 🟢/🟢
<sub>app_theme_binding</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/app_theme_binding_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/app_theme_binding_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/app_theme_binding_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/app_theme_binding_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/app_theme_binding_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/app_theme_binding_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/app_theme_binding_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/app_theme_binding_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/app_theme_binding_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/app_theme_binding_dark.png" /></td></tr></table>

ports AppThemeBindingPage.xaml

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 8. Application Control — 🟢/🟢
<sub>application_control</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/application_control_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/application_control_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/application_control_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/application_control_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/application_control_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/application_control_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/application_control_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/application_control_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/application_control_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/application_control_dark.png" /></td></tr></table>

ports ApplicationControlPage.xaml (+ .xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 9. Auto Size Shapes — 🟢/🟢
<sub>auto_size_shapes</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/auto_size_shapes_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/auto_size_shapes_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/auto_size_shapes_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/auto_size_shapes_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/auto_size_shapes_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/auto_size_shapes_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/auto_size_shapes_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/auto_size_shapes_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/auto_size_shapes_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/auto_size_shapes_dark.png" /></td></tr></table>

ports AutoSizeShapesGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/AutoSizeShapesGallery.xaml: a 3-row Grid (RowSpacing 0) that proves a stroked Ellipse auto-sizes to fill exactly half of the av

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9960, 0.71% pixels differ · Dark: SSIM 0.9979, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9966, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 10. Basic Grouping — 🟢/🟢
<sub>basic_grouping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/basic_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/basic_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/basic_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/basic_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/basic_grouping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/basic_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/basic_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/basic_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/basic_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/basic_grouping_dark.png" /></td></tr></table>

ports GroupingGalleries/BasicGrouping.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9977, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 11. Basic Swipe — 🟢/🟢
<sub>basic_swipe</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/basic_swipe_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/basic_swipe_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/basic_swipe_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/basic_swipe_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/basic_swipe_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/basic_swipe_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/basic_swipe_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/basic_swipe_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/basic_swipe_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/basic_swipe_dark.png" /></td></tr></table>

ports BasicSwipeGallery.xaml A code-first port of the MAUI SwipeView sub-gallery Pages/Controls/SwipeViewGalleries/BasicSwipeGallery.xaml: a vertical StackLayout of five SwipeViews, each demonstrating a different revealed-side / SwipeMode c

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 12. Behaviors — 🟢/🟢
<sub>behaviors</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/behaviors_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/behaviors_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/behaviors_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/behaviors_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/behaviors_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/behaviors_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/behaviors_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/behaviors_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/behaviors_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/behaviors_dark.png" /></td></tr></table>

ports BehaviorsPage.xaml (+ .xaml.cs) and its companion Controls.Sample/Behaviors/NumericValidationBehavior.cs

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 13. Border — 🟡/🟡
<sub>border</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/border_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/border_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;border&amp;quot; demo (ComparePages.BorderPage()), the shipped-.NET-MAUI reference for the visual-parity comparison: a single Border centered on the page — red 5pt stroke, a RoundRectangle StrokeShape (Co

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9776, 1.34% pixels differ · Dark: SSIM 0.9818, 1.24% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9763, 1.37% pixels differ · Dark: SSIM 0.9805, 1.27% pixels differ

### 14. Border Clip Playground — 🟢/🟢
<sub>border_clip_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/border_clip_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_clip_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_clip_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_clip_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_clip_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/border_clip_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_clip_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_clip_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_clip_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_clip_playground_dark.png" /></td></tr></table>

ports BorderClipPlayground.xaml (+ .xaml.cs) The C# page is an interactive Border-shape playground: a 100x100 Border (red stroke) clips an AspectFill Image (oasis.jpg) into the currently selected StrokeShape, while controls below mutate the

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9888, 0.57% pixels differ · Dark: SSIM 0.9889, 0.60% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9904, 0.45% pixels differ · Dark: SSIM 0.9908, 0.46% pixels differ

### 15. Border Layout — 🟢/🟢
<sub>border_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/border_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/border_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_layout_dark.png" /></td></tr></table>

ports BorderLayout.xaml (+ BorderLayout.xaml.cs) The C# page demonstrates driving Border.StrokeThickness from a Slider: a Slider (0..40, set to 5 in OnAppearing) is bound to the Border&amp;#x27;s StrokeThickness; the Border (Silver stroke, White bac

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9956, 0.56% pixels differ · Dark: SSIM 0.9951, 0.80% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9944, 0.60% pixels differ · Dark: SSIM 0.9938, 0.84% pixels differ

### 16. Border Playground — 🟡/🟡
<sub>border_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/border_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/border_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_playground_dark.png" /></td></tr></table>

ports BorderPlayground.xaml (+ BorderPlayground.xaml.cs) A self-contained, code-first interactive Border playground

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9871, 1.24% pixels differ · Dark: SSIM 0.9848, 1.35% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9903, 1.09% pixels differ · Dark: SSIM 0.9888, 1.15% pixels differ

### 17. Border Resize Content — 🟢/🟢
<sub>border_resize_content</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/border_resize_content_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_resize_content_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_resize_content_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_resize_content_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_resize_content_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/border_resize_content_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_resize_content_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_resize_content_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_resize_content_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_resize_content_dark.png" /></td></tr></table>

ports BorderResizeContent.xaml A self-contained, code-first demo that resizes a Border&amp;#x27;s CONTENT and watches the Border track it

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9930, 0.55% pixels differ · Dark: SSIM 0.9934, 0.61% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9926, 0.44% pixels differ · Dark: SSIM 0.9931, 0.48% pixels differ

### 18. Border Stroke — 🟡/🟡
<sub>border_stroke</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/border_stroke_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_stroke_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_stroke_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_stroke_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_stroke_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/border_stroke_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/border_stroke_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/border_stroke_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/border_stroke_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/border_stroke_dark.png" /></td></tr></table>

ports BorderStroke.xaml (+ BorderStroke.xaml.cs) A self-contained, code-first demo of Border StrokeThickness and how a Border tracks the height of its content

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9747, 3.60% pixels differ · Dark: SSIM 0.9748, 3.60% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9734, 3.63% pixels differ · Dark: SSIM 0.9735, 3.64% pixels differ

### 19. Borderless — 🟢/🟢
<sub>borderless</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/borderless_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/borderless_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/borderless_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/borderless_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/borderless_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/borderless_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/borderless_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/borderless_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/borderless_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/borderless_dark.png" /></td></tr></table>

ports Borderless.xaml A self-contained, code-first demo of a stroke-less Border

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9992, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9966, 0.12% pixels differ · Dark: SSIM 0.9989, 0.11% pixels differ

### 20. Box View — 🟢/🟢
<sub>box_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/box_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/box_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/box_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/box_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/box_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/box_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/box_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/box_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/box_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/box_view_dark.png" /></td></tr></table>

ports BoxViewPage.xaml (+ BoxViewPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 21. Button — 🟢/🟢
<sub>button</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/button_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/button_dark.png" /></td></tr></table>

ports ButtonPage.xaml (+ ButtonPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9904, 0.37% pixels differ · Dark: SSIM 0.9906, 0.37% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9892, 0.40% pixels differ · Dark: SSIM 0.9893, 0.40% pixels differ

### 22. Carousel Page — 🟡/🟡
<sub>carousel_page</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/carousel_page_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/carousel_page_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/carousel_page_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/carousel_page_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/carousel_page_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/carousel_page_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/carousel_page_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/carousel_page_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/carousel_page_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/carousel_page_dark.png" /></td></tr></table>

Carousel Page

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9884, 1.05% pixels differ · Dark: SSIM 0.9944, 0.73% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9871, 1.08% pixels differ · Dark: SSIM 0.9932, 0.76% pixels differ

### 23. Chat Example — 🟢/🟢
<sub>chat_example</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/chat_example_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/chat_example_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/chat_example_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/chat_example_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/chat_example_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/chat_example_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/chat_example_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/chat_example_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/chat_example_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/chat_example_dark.png" /></td></tr></table>

ports ChatExample.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.ItemSizeGalleries.ChatExample), tracking the maui-compare reference demo ~/maui-compare/Pages/ChatExamplePage.cs (the visual-parity oracle)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 24. Check Box — 🟢/🟢
<sub>check_box</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/check_box_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/check_box_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/check_box_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/check_box_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/check_box_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/check_box_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/check_box_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/check_box_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/check_box_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/check_box_dark.png" /></td></tr></table>

ports CheckBoxPage.xaml (+ .xaml.cs) Mirrors the MAUI gallery page: a vertical stack of headlined CheckBox states — Default, Colored (Color=Purple), Disabled, Disabled+Colored+Checked — followed by a &amp;quot;Change IsChecked&amp;quot; row pairing a Button

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9972, 0.11% pixels differ · Dark: SSIM 0.9973, 0.11% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 25. Chrome — 🟢/🟢
<sub>chrome</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/chrome_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/chrome_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/chrome_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/chrome_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/chrome_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/chrome_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/chrome_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/chrome_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/chrome_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/chrome_dark.png" /></td></tr></table>

a self-contained demo page for the W1-11 window-chrome family: page toolbar items (primary + secondary), a menu bar (File menu with items, a separator and a sub-menu), a context flyout (right-click menu) on a button, and a tooltip — all wir

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 26. Clip — 🟢/🟢
<sub>clip</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/clip_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/clip_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/clip_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/clip_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/clip_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/clip_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/clip_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/clip_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/clip_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/clip_dark.png" /></td></tr></table>

ports ClipPage.xaml The C# page (Pages/Core/ClipPage.xaml; its .xaml.cs is an empty InitializeComponent) is a ScrollView over a StackLayout that shows the SAME dotnet_bot.png image five times, each successive copy carrying a different geome

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 27. Clip Corner Radius — 🟢/🟢
<sub>clip_corner_radius</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/clip_corner_radius_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/clip_corner_radius_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/clip_corner_radius_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/clip_corner_radius_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/clip_corner_radius_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/clip_corner_radius_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/clip_corner_radius_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/clip_corner_radius_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/clip_corner_radius_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/clip_corner_radius_dark.png" /></td></tr></table>

ports ClipCornerRadiusGallery.xaml (+ .xaml.cs) The C# page (Pages/Controls/ShapesGalleries/ClipCornerRadiusGallery.xaml) is a StackLayout (Padding=12) that demonstrates DRIVING a RoundRectangleGeometry&amp;#x27;s per-corner CornerRadius from four s

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 28. Clip Gallery — 🟢/🟢
<sub>clip_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/clip_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/clip_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/clip_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/clip_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/clip_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/clip_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/clip_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/clip_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/clip_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/clip_gallery_dark.png" /></td></tr></table>

ports ClipGallery.xaml The C# page (Pages/Controls/ShapesGalleries/ClipGallery.xaml; its .xaml.cs is an empty InitializeComponent) is a ScrollView over a StackLayout (Padding=12) that shows the SAME &amp;quot;oasis.jpg&amp;quot; image SEVEN times — one bare

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 29. Clip Views — 🟢/🟢
<sub>clip_views</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/clip_views_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/clip_views_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/clip_views_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/clip_views_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/clip_views_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/clip_views_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/clip_views_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/clip_views_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/clip_views_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/clip_views_dark.png" /></td></tr></table>

ports ClipViewsGallery.xaml The C# page (Pages/Controls/ShapesGalleries/ClipViewsGallery.xaml; no code-behind beyond an empty InitializeComponent) is a ScrollView over a StackLayout (Padding=12) that proves the Clip surface (VisualElement.C

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9921, 0.92% pixels differ · Dark: SSIM 0.9932, 0.95% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9908, 0.95% pixels differ · Dark: SSIM 0.9919, 0.98% pixels differ

### 30. Clipping — 🟢/🟢
<sub>clipping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/clipping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/clipping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/clipping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/clipping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/clipping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/clipping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/clipping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/clipping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/clipping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/clipping_dark.png" /></td></tr></table>

compare oracle ~/maui-compare/Pages/ClippingPage.cs (itself written to mirror this gallery page)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9963, 0.15% pixels differ · Dark: SSIM 0.9965, 0.15% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9951, 0.18% pixels differ · Dark: SSIM 0.9954, 0.18% pixels differ

### 31. Collectionview — 🟢/🟢
<sub>collectionview</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/collectionview_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/collectionview_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/collectionview_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/collectionview_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/collectionview_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/collectionview_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/collectionview_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/collectionview_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/collectionview_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/collectionview_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;collectionview&amp;quot; demo (ComparePages.CollectionViewPage()), the shipped-.NET-MAUI reference for the visual-parity comparison: a CollectionView over 24 captioned items, a string Header (&amp;quot;This is the

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 32. Composition Gallery — 🟢/🟢
<sub>composition_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/composition_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/composition_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/composition_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/composition_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/composition_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/composition_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/composition_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/composition_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/composition_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/composition_gallery_dark.png" /></td></tr></table>

ports CompositionGallery.xaml A self-contained, code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/CompositionGallery.xaml: a StackLayout holding two Beige 250x250 Grids (Margin 12) that compose multiple overlappi

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9972, 0.39% pixels differ · Dark: SSIM 0.9973, 0.39% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 33. Containers — 🟢/🟢
<sub>containers</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/containers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/containers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/containers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/containers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/containers_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/containers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/containers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/containers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/containers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/containers_dark.png" /></td></tr></table>

a self-contained demo page for the W1-07 container set: a scroll_view hosting a vertical stack of content-hosting containers — a border-framed label (stroke + dashed outline + rounded shape), a legacy frame (BorderColor/CornerRadius/HasShad

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9877, 0.54% pixels differ · Dark: SSIM 0.9899, 0.66% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9869, 0.56% pixels differ · Dark: SSIM 0.9891, 0.68% pixels differ

### 34. Content View — 🟢/🟢
<sub>content_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/content_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/content_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/content_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/content_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/content_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/content_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/content_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/content_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/content_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/content_view_dark.png" /></td></tr></table>

ports ContentViewPage.xaml (+ ContentViewPage.xaml.cs), code-first

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 35. Context Flyout — 🟡/🟡
<sub>context_flyout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/context_flyout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/context_flyout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/context_flyout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/context_flyout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/context_flyout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/context_flyout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/context_flyout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/context_flyout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/context_flyout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/context_flyout_dark.png" /></td></tr></table>

ports ContextFlyoutPage.xaml (+ ContextFlyoutPage.xaml.cs) The C# page attaches a MenuFlyout as the FlyoutBase.ContextFlyout (right-click / long-press menu) of several controls and wires each menu item to a handler: - a Button (&amp;quot;Increment b

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9183, 3.34% pixels differ · Dark: SSIM 0.9186, 3.34% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9157, 3.42% pixels differ · Dark: SSIM 0.9160, 3.42% pixels differ

### 36. Controls Stack — 🟢/🟢
<sub>controls_stack</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/controls_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/controls_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/controls_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/controls_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/controls_stack_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/controls_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/controls_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/controls_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/controls_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/controls_stack_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;controls_stack&amp;quot; demo (ComparePages.ControlsStack()), the shipped-.NET-MAUI reference for the visual-parity comparison: a VerticalStackLayout (Spacing 12, Padding 16) showcasing the basic widgets

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9962, 0.12% pixels differ · Dark: SSIM 0.9962, 0.12% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9949, 0.15% pixels differ · Dark: SSIM 0.9949, 0.16% pixels differ

### 37. Custom Layout — 🟢/🟢
<sub>custom_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/custom_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/custom_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/custom_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/custom_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/custom_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/custom_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/custom_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/custom_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/custom_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/custom_layout_dark.png" /></td></tr></table>

ports CustomLayoutPage.xaml (+ CustomLayoutPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 38. Custom Size Swipe — 🟢/🟢
<sub>custom_size_swipe</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/custom_size_swipe_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/custom_size_swipe_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/custom_size_swipe_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/custom_size_swipe_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/custom_size_swipe_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/custom_size_swipe_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/custom_size_swipe_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/custom_size_swipe_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/custom_size_swipe_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/custom_size_swipe_dark.png" /></td></tr></table>

ports CustomSizeSwipeViewGallery.xaml (+ .xaml.cs) The MAUI CustomSizeSwipeViewGallery is a single SwipeView whose Left / Right / Top item collections each reveal CUSTOM-SIZED content: a SwipeItemView wrapping a Grid/StackLayout with an exp

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9979, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9966, 0.12% pixels differ · Dark: SSIM 0.9967, 0.11% pixels differ

### 39. Custom Swipe Item View — 🟢/🟢
<sub>custom_swipe_item_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/custom_swipe_item_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/custom_swipe_item_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/custom_swipe_item_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/custom_swipe_item_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/custom_swipe_item_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/custom_swipe_item_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/custom_swipe_item_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/custom_swipe_item_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/custom_swipe_item_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/custom_swipe_item_view_dark.png" /></td></tr></table>

ports CustomSwipeItemViewGallery.xaml A self-contained, code-first port of the .NET MAUI &amp;quot;CustomSwipeItem&amp;quot; gallery: a message-list row whose right swipe reveals a CUSTOM-content swipe item (a swipe_item_view, not a plain swipe_item)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.12% pixels differ

### 40. Cv Visual States — 🟢/🟢
<sub>cv_visual_states</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/cv_visual_states_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/cv_visual_states_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/cv_visual_states_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/cv_visual_states_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/cv_visual_states_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/cv_visual_states_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/cv_visual_states_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/cv_visual_states_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/cv_visual_states_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/cv_visual_states_dark.png" /></td></tr></table>

ports CollectionViewGalleries/SelectionGalleries/ VisualStatesGallery.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 41. Data Template Selector — 🟢/🟢
<sub>data_template_selector</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/data_template_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/data_template_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/data_template_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/data_template_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/data_template_selector_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/data_template_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/data_template_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/data_template_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/data_template_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/data_template_selector_dark.png" /></td></tr></table>

ports DataTemplateSelectorGallery.xaml (+ DataTemplateSelectorGallery.xaml.cs, including its WeekendSelector + SearchTermSelector classes)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 42. Date Picker — 🟢/🟢
<sub>date_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/date_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/date_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/date_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/date_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/date_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/date_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/date_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/date_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/date_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/date_picker_dark.png" /></td></tr></table>

ports DatePickerPage.xaml (+ DatePickerPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9974, 0.13% pixels differ · Dark: SSIM 0.9975, 0.13% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9961, 0.16% pixels differ · Dark: SSIM 0.9963, 0.16% pixels differ

### 43. Device — 🟢/🟢
<sub>device</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/device_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/device_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/device_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/device_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/device_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/device_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/device_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/device_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/device_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/device_dark.png" /></td></tr></table>

ports DevicePage.xaml

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9925, 0.28% pixels differ · Dark: SSIM 0.9923, 0.31% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 44. Dispatcher — 🟢/🟢
<sub>dispatcher</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/dispatcher_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/dispatcher_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/dispatcher_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/dispatcher_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/dispatcher_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/dispatcher_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/dispatcher_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/dispatcher_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/dispatcher_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/dispatcher_dark.png" /></td></tr></table>

ports DispatcherPage.xaml (+ .xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 45. Drag Drop — 🟢/🟢
<sub>drag_drop</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/drag_drop_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/drag_drop_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/drag_drop_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/drag_drop_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/drag_drop_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/drag_drop_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/drag_drop_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/drag_drop_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/drag_drop_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/drag_drop_dark.png" /></td></tr></table>

ports DragAndDropBetweenLayouts.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.DragAndDropBetweenLayouts)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 46. Editor — 🟢/🟢
<sub>editor</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/editor_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/editor_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/editor_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/editor_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/editor_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/editor_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/editor_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/editor_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/editor_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/editor_dark.png" /></td></tr></table>

ports EditorPage.xaml (Microsoft.Maui.Controls sample gallery)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 47. Effects — 🟢/🟢
<sub>effects</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/effects_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/effects_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/effects_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/effects_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/effects_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/effects_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/effects_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/effects_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/effects_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/effects_dark.png" /></td></tr></table>

ports EffectsPage.xaml (Maui.Controls.Sample.Pages.EffectsPage)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 48. Ellipse Gallery — 🟢/🟢
<sub>ellipse_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/ellipse_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ellipse_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ellipse_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ellipse_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ellipse_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/ellipse_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ellipse_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ellipse_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ellipse_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ellipse_gallery_dark.png" /></td></tr></table>

ports EllipseGallery.xaml A self-contained, code-first port of the MAUI Shapes EllipseGallery (Pages/Controls/ShapesGalleries/EllipseGallery.xaml + .xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 49. Empty View — 🟢/🟢
<sub>empty_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/empty_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/empty_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_dark.png" /></td></tr></table>

ports EmptyViewStringGallery.xaml (+ EmptyViewStringGallery.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 50. Empty View Load Simulate — 🟢/🟢
<sub>empty_view_load_simulate</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/empty_view_load_simulate_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_load_simulate_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_load_simulate_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_load_simulate_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_load_simulate_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/empty_view_load_simulate_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_load_simulate_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_load_simulate_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_load_simulate_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_load_simulate_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewLoadSimulateGallery.xaml (+ EmptyViewLoadSimulateGallery.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 51. Empty View Null — 🟢/🟢
<sub>empty_view_null</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/empty_view_null_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_null_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_null_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_null_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_null_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/empty_view_null_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_null_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_null_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_null_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_null_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewNullGallery.xaml (+ EmptyViewNullGallery.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 52. Empty View Rtl — 🟢/🟢
<sub>empty_view_rtl</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/empty_view_rtl_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_rtl_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_rtl_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_rtl_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_rtl_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/empty_view_rtl_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_rtl_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_rtl_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_rtl_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_rtl_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewRTLGallery.xaml (+ EmptyViewRTLGallery.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 53. Empty View Selector — 🟢/🟢
<sub>empty_view_selector</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/empty_view_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_selector_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/empty_view_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_selector_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewWithDataTemplateSelector.xaml (+ .xaml.cs, incl

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 54. Empty View Swap — 🟢/🟢
<sub>empty_view_swap</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/empty_view_swap_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_swap_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_swap_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_swap_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_swap_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/empty_view_swap_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_swap_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_swap_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_swap_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_swap_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewSwapGallery.xaml (+ .xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 55. Empty View Template — 🟢/🟢
<sub>empty_view_template</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/empty_view_template_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_template_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_template_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_template_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_template_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/empty_view_template_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_template_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_template_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_template_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_template_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewTemplateGallery.xaml (+ EmptyViewTemplateGallery.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 56. Empty View View — 🟢/🟢
<sub>empty_view_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/empty_view_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/empty_view_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/empty_view_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/empty_view_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/empty_view_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/empty_view_view_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewViewGallery.xaml (+ EmptyViewViewGallery.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 57. Entry — 🟢/🟢
<sub>entry</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/entry_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/entry_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/entry_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/entry_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/entry_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/entry_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/entry_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/entry_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/entry_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/entry_dark.png" /></td></tr></table>

ports EntryPage.xaml (Microsoft.Maui.Controls sample gallery)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9977, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 58. Filter Collection — 🟢/🟢
<sub>filter_collection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/filter_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/filter_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/filter_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/filter_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/filter_collection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/filter_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/filter_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/filter_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/filter_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/filter_collection_dark.png" /></td></tr></table>

ports FilterCollectionView.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9955, 0.16% pixels differ · Dark: SSIM 0.9955, 0.18% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 59. Filter Selection — 🟢/🟢
<sub>filter_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/filter_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/filter_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/filter_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/filter_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/filter_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/filter_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/filter_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/filter_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/filter_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/filter_selection_dark.png" /></td></tr></table>

ports FilterSelection.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.FilterSelection)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 60. Flex Layout — 🟢/🟢
<sub>flex_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/flex_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/flex_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/flex_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/flex_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/flex_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/flex_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/flex_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/flex_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/flex_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/flex_layout_dark.png" /></td></tr></table>

ports FlexLayoutPage.xaml A self-contained, code-first demo of the FlexLayout control: the classic &amp;quot;holy grail&amp;quot; page layout built from nested flexboxes

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9979, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9966, 0.12% pixels differ · Dark: SSIM 0.9967, 0.11% pixels differ

### 61. Focus — 🟢/🟢
<sub>focus</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/focus_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/focus_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/focus_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/focus_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/focus_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/focus_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/focus_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/focus_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/focus_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/focus_dark.png" /></td></tr></table>

ports FocusPage.xaml (+ FocusPage.xaml.cs) The C# FocusPage is a focus-subsystem demo: an Entry whose Focused/Unfocused events (OnFocusEntryFocusChanged) append &amp;quot;Focused&amp;quot;/&amp;quot;Unfocused&amp;quot; lines to a scrolling InfoLabel, plus two buttons — &amp;quot;Focus

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 62. Fonts — 🟢/🟢
<sub>fonts</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/fonts_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/fonts_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/fonts_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/fonts_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/fonts_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/fonts_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/fonts_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/fonts_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/fonts_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/fonts_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;fonts&amp;quot; demo (ComparePages.Fonts()), the shipped-.NET-MAUI reference for the visual-parity comparison: a ScrollView over a VerticalStackLayout (Spacing 8, Padding 16) of nine Labels — Title/Subtit

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 63. Footer Only String — 🟢/🟢
<sub>footer_only_string</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/footer_only_string_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/footer_only_string_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/footer_only_string_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/footer_only_string_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/footer_only_string_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/footer_only_string_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/footer_only_string_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/footer_only_string_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/footer_only_string_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/footer_only_string_dark.png" /></td></tr></table>

ports FooterOnlyString.xaml (+ FooterOnlyString.xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 64. Formatted Text — 🟢/🟢
<sub>formatted_text</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/formatted_text_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/formatted_text_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/formatted_text_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/formatted_text_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/formatted_text_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/formatted_text_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/formatted_text_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/formatted_text_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/formatted_text_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/formatted_text_dark.png" /></td></tr></table>

a self-contained demo page for the G1 rich-text slice: a label whose FormattedText is built from several styled spans (bold / italic / colored / underlined / kerned), plus a plain label proving the Text ⇄ FormattedText exclusivity

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9967, 0.12% pixels differ · Dark: SSIM 0.9967, 0.12% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 65. Gestures — 🟢/🟢
<sub>gestures</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/gestures_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/gestures_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/gestures_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/gestures_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/gestures_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/gestures_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/gestures_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/gestures_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/gestures_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/gestures_dark.png" /></td></tr></table>

ports GesturesPage.xaml (+ .xaml.cs) The MAUI GesturesPage.xaml is a *gallery navigation* page: a CollectionView listing gesture-demo sections that the shell navigates into

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 66. Gradient — 🟢/🟢
<sub>gradient</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/gradient_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/gradient_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/gradient_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/gradient_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/gradient_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/gradient_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/gradient_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/gradient_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/gradient_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/gradient_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;gradient&amp;quot; demo (ComparePages.Gradient()), the shipped-.NET-MAUI reference for the visual-parity comparison: a VerticalStackLayout (Spacing 12, Padding 16) of two captioned 60px BoxViews — a Linea

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 67. Grid — 🟢/🟢
<sub>grid</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/grid_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/grid_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/grid_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/grid_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/grid_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/grid_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/grid_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/grid_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/grid_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/grid_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;grid&amp;quot; demo (ComparePages.GridPage()), the shipped-.NET-MAUI reference for the visual-parity comparison: a Grid (Padding 16, Row/ColumnSpacing 6) with RowDefinitions Auto / 80 / 80 and two Star co

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 68. Grid Grouping — 🟢/🟢
<sub>grid_grouping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/grid_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/grid_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/grid_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/grid_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/grid_grouping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/grid_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/grid_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/grid_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/grid_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/grid_grouping_dark.png" /></td></tr></table>

ports GroupingGalleries/GridGrouping.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9979, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9966, 0.12% pixels differ · Dark: SSIM 0.9967, 0.11% pixels differ

### 69. Grouping No Templates — 🟢/🟢
<sub>grouping_no_templates</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/grouping_no_templates_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/grouping_no_templates_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/grouping_no_templates_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/grouping_no_templates_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/grouping_no_templates_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/grouping_no_templates_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/grouping_no_templates_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/grouping_no_templates_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/grouping_no_templates_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/grouping_no_templates_dark.png" /></td></tr></table>

ports GroupingGalleries/GroupingNoTemplates.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 70. Grouping Plus Selection — 🟢/🟢
<sub>grouping_plus_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/grouping_plus_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/grouping_plus_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/grouping_plus_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/grouping_plus_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/grouping_plus_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/grouping_plus_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/grouping_plus_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/grouping_plus_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/grouping_plus_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/grouping_plus_selection_dark.png" /></td></tr></table>

ports CollectionViewGalleries/GroupingGalleries/ GroupingPlusSelection.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9979, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9966, 0.12% pixels differ · Dark: SSIM 0.9967, 0.11% pixels differ

### 71. Header Footer — 🟢/🟢
<sub>header_footer</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/header_footer_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/header_footer_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/header_footer_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/header_footer_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/header_footer_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/header_footer_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/header_footer_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/header_footer_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/header_footer_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/header_footer_dark.png" /></td></tr></table>

ports HeaderFooterString.xaml (+ HeaderFooterString.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 72. Header Footer Grid — 🟢/🟢
<sub>header_footer_grid</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/header_footer_grid_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/header_footer_grid_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/header_footer_grid_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/header_footer_grid_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/header_footer_grid_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/header_footer_grid_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/header_footer_grid_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/header_footer_grid_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/header_footer_grid_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/header_footer_grid_dark.png" /></td></tr></table>

ports HeaderFooterGrid.xaml (+ HeaderFooterGrid.xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9951, 0.24% pixels differ · Dark: SSIM 0.9917, 0.31% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9938, 0.27% pixels differ · Dark: SSIM 0.9904, 0.34% pixels differ

### 73. Header Footer Grid Horizontal — 🟡/🟡
<sub>header_footer_grid_horizontal</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/header_footer_grid_horizontal_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/header_footer_grid_horizontal_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/header_footer_grid_horizontal_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/header_footer_grid_horizontal_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/header_footer_grid_horizontal_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/header_footer_grid_horizontal_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/header_footer_grid_horizontal_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/header_footer_grid_horizontal_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/header_footer_grid_horizontal_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/header_footer_grid_horizontal_dark.png" /></td></tr></table>

ports HeaderFooterGridHorizontal.xaml (+ HeaderFooterGridHorizontal.xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9786, 0.53% pixels differ · Dark: SSIM 0.9774, 0.72% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9773, 0.56% pixels differ · Dark: SSIM 0.9762, 0.76% pixels differ

### 74. Header Footer Template — 🟢/🟢
<sub>header_footer_template</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/header_footer_template_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/header_footer_template_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/header_footer_template_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/header_footer_template_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/header_footer_template_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/header_footer_template_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/header_footer_template_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/header_footer_template_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/header_footer_template_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/header_footer_template_dark.png" /></td></tr></table>

ports HeaderFooterTemplate.xaml (+ .xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9979, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9966, 0.12% pixels differ · Dark: SSIM 0.9967, 0.11% pixels differ

### 75. Header Footer View — 🟢/🟢
<sub>header_footer_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/header_footer_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/header_footer_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/header_footer_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/header_footer_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/header_footer_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/header_footer_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/header_footer_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/header_footer_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/header_footer_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/header_footer_view_dark.png" /></td></tr></table>

ports HeaderFooterView.xaml (+ .xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9952, 0.23% pixels differ · Dark: SSIM 0.9932, 0.28% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9940, 0.26% pixels differ · Dark: SSIM 0.9920, 0.31% pixels differ

### 76. Hit Testing — 🟢/🟢
<sub>hit_testing</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/hit_testing_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/hit_testing_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/hit_testing_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/hit_testing_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/hit_testing_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/hit_testing_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/hit_testing_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/hit_testing_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/hit_testing_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/hit_testing_dark.png" /></td></tr></table>

ports HitTestingPage.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.HitTestingPage)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9977, 0.11% pixels differ · Dark: SSIM 0.9977, 0.11% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9964, 0.14% pixels differ · Dark: SSIM 0.9965, 0.14% pixels differ

### 77. Horizontal Stack — 🟢/🟢
<sub>horizontal_stack</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/horizontal_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/horizontal_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/horizontal_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/horizontal_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/horizontal_stack_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/horizontal_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/horizontal_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/horizontal_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/horizontal_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/horizontal_stack_dark.png" /></td></tr></table>

Horizontal Stack

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 78. Hybrid Web View — 🟢/🟢
<sub>hybrid_web_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/hybrid_web_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/hybrid_web_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/hybrid_web_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/hybrid_web_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/hybrid_web_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/hybrid_web_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/hybrid_web_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/hybrid_web_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/hybrid_web_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/hybrid_web_view_dark.png" /></td></tr></table>

ports HybridWebViewPage.xaml (+ HybridWebViewPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 79. Image — 🟢/🟢
<sub>image</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/image_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/image_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/image_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/image_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/image_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/image_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/image_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/image_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/image_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/image_dark.png" /></td></tr></table>

ports ImagePage.xaml (+ ImagePage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 80. Image Button — 🟢/🟢
<sub>image_button</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/image_button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/image_button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/image_button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/image_button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/image_button_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/image_button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/image_button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/image_button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/image_button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/image_button_dark.png" /></td></tr></table>

ports ImageButtonPage.xaml (+ ImageButtonPage.xaml.cs) A self-contained, code-first demo page for the ImageButton control (the C# gallery-page convention, mirroring the input_controls_page / image_page pattern)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9947, 0.24% pixels differ · Dark: SSIM 0.9947, 0.23% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9935, 0.27% pixels differ · Dark: SSIM 0.9935, 0.26% pixels differ

### 81. Indicator — 🟢/🟢
<sub>indicator</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/indicator_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/indicator_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/indicator_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/indicator_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/indicator_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/indicator_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/indicator_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/indicator_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/indicator_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/indicator_dark.png" /></td></tr></table>

ports IndicatorPage.xaml A self-contained, code-first demo of the IndicatorView control

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 82. Input Controls — 🟢/🟢
<sub>input_controls</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/input_controls_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/input_controls_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/input_controls_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/input_controls_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/input_controls_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/input_controls_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/input_controls_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/input_controls_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/input_controls_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/input_controls_dark.png" /></td></tr></table>

a self-contained demo page for the W1-05 input-control set: editor, search_bar, radio_button (+ the radio_button_group attached grouping) and image_button on one vertical stack, wired together so every input drives a visible output (the C#

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9950, 0.18% pixels differ · Dark: SSIM 0.9950, 0.20% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9938, 0.22% pixels differ · Dark: SSIM 0.9937, 0.23% pixels differ

### 83. Input Transparent — 🟢/🟢
<sub>input_transparent</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/input_transparent_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/input_transparent_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/input_transparent_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/input_transparent_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/input_transparent_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/input_transparent_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/input_transparent_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/input_transparent_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/input_transparent_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/input_transparent_dark.png" /></td></tr></table>

ports InputTransparentPage.xaml (Maui.Controls.Sample.Pages.InputTransparentPage)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 84. Invalidate Brush — 🟢/🟢
<sub>invalidate_brush</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/invalidate_brush_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/invalidate_brush_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/invalidate_brush_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/invalidate_brush_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/invalidate_brush_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/invalidate_brush_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/invalidate_brush_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/invalidate_brush_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/invalidate_brush_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/invalidate_brush_dark.png" /></td></tr></table>

ports InvalidateBrushGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/InvalidateBrushGallery.xaml (&amp;quot;Invalidate Brushes Playground&amp;quot;): a VerticalStackLayout (Padding 12) with — - a &amp;quot;Change color&amp;quot; Bu

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 85. Invalidate Shadow Host — 🟢/🟢
<sub>invalidate_shadow_host</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/invalidate_shadow_host_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/invalidate_shadow_host_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/invalidate_shadow_host_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/invalidate_shadow_host_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/invalidate_shadow_host_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/invalidate_shadow_host_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/invalidate_shadow_host_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/invalidate_shadow_host_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/invalidate_shadow_host_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/invalidate_shadow_host_dark.png" /></td></tr></table>

ports InvalidateShadowHostPage.xaml A self-contained, code-first demo that a shadow re-applies (invalidates) when its host&amp;#x27;s size changes, mirroring the C# core gallery page (Pages/Core/ShadowGalleries/InvalidateShadowHostPage.xaml + .xaml.

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 86. Ios Blur Effect — 🟢/🟢
<sub>ios_blur_effect</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/ios_blur_effect_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_blur_effect_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_blur_effect_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_blur_effect_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_blur_effect_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/ios_blur_effect_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_blur_effect_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_blur_effect_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_blur_effect_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_blur_effect_dark.png" /></td></tr></table>

ports iOSBlurEffectPage.xaml The .NET MAUI PlatformSpecifics sample (Pages/PlatformSpecifics/iOS/iOSBlurEffectPage.xaml + .xaml.cs): an Image (Source=&amp;quot;oasis.jpg&amp;quot;) carrying the iOSSpecific VisualElement.BlurEffect knob (XAML seeds it to Extr

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 87. Ios Date Picker — 🟢/🟢
<sub>ios_date_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/ios_date_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_date_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_date_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_date_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_date_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/ios_date_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_date_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_date_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_date_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_date_picker_dark.png" /></td></tr></table>

ports iOSDatePickerPage.xaml (+ iOSDatePickerPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 88. Ios Entry — 🟢/🟢
<sub>ios_entry</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/ios_entry_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_entry_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_entry_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_entry_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_entry_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/ios_entry_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_entry_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_entry_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_entry_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_entry_dark.png" /></td></tr></table>

ports iOSEntryPage.xaml (+ iOSEntryPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 89. Ios First Responder — 🟢/🟢
<sub>ios_first_responder</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/ios_first_responder_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_first_responder_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_first_responder_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_first_responder_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_first_responder_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/ios_first_responder_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_first_responder_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_first_responder_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_first_responder_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_first_responder_dark.png" /></td></tr></table>

ports iOSFirstResponderPage.xaml (+ .xaml.cs) The C# iOSFirstResponderPage is a VisualElement-first-responder demo: a StackLayout with an explanatory Label, a &amp;quot;First Entry&amp;quot; + plain &amp;quot;OK&amp;quot; Button (tapping OK dismisses the keyboard because the

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 90. Ios Pan Gesture — 🟢/🟢
<sub>ios_pan_gesture</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/ios_pan_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_pan_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_pan_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_pan_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_pan_gesture_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/ios_pan_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_pan_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_pan_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_pan_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_pan_gesture_dark.png" /></td></tr></table>

ports iOSPanGestureRecognizerPage.xaml (+ .xaml.cs) The C# iOSPanGestureRecognizerPage is a StackLayout with: a bold message Label (_messageLabel), a &amp;quot;Toggle Simultaneous Gesture Recognition&amp;quot; Button, and a grouped ListView of employees whos

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 91. Ios Picker — 🟢/🟢
<sub>ios_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/ios_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/ios_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_picker_dark.png" /></td></tr></table>

ports iOSPickerPage.xaml (+ iOSPickerPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 92. Ios Safe Area — 🟢/🟢
<sub>ios_safe_area</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/ios_safe_area_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_safe_area_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_safe_area_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_safe_area_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_safe_area_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/ios_safe_area_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_safe_area_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_safe_area_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_safe_area_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_safe_area_dark.png" /></td></tr></table>

ports iOSSafeAreaPage.xaml The .NET MAUI PlatformSpecifics sample (Pages/PlatformSpecifics/iOS/iOSSafeAreaPage.xaml + .xaml.cs): a long Lorem-ipsum Label over a &amp;quot;Disable Use Safe Area&amp;quot; button

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 93. Ios Scroll View — 🟢/🟢
<sub>ios_scroll_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/ios_scroll_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_scroll_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_scroll_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_scroll_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_scroll_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/ios_scroll_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_scroll_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_scroll_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_scroll_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_scroll_view_dark.png" /></td></tr></table>

ports iOSScrollViewPage.xaml (+ iOSScrollViewPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 94. Ios Search Bar — 🟢/🟢
<sub>ios_search_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/ios_search_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_search_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_search_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_search_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_search_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/ios_search_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_search_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_search_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_search_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_search_bar_dark.png" /></td></tr></table>

ports iOSSearchBarPage.xaml (+ iOSSearchBarPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 95. Ios Slider Update On Tap — 🟢/🟢
<sub>ios_slider_update_on_tap</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/ios_slider_update_on_tap_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_slider_update_on_tap_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_slider_update_on_tap_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_slider_update_on_tap_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_slider_update_on_tap_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/ios_slider_update_on_tap_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_slider_update_on_tap_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_slider_update_on_tap_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_slider_update_on_tap_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_slider_update_on_tap_dark.png" /></td></tr></table>

ports iOSSliderUpdateOnTapPage.xaml (+ iOSSliderUpdateOnTapPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 96. Ios Swipe Transition — 🟢/🟢
<sub>ios_swipe_transition</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/ios_swipe_transition_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_swipe_transition_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_swipe_transition_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_swipe_transition_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_swipe_transition_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/ios_swipe_transition_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_swipe_transition_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_swipe_transition_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_swipe_transition_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_swipe_transition_dark.png" /></td></tr></table>

ports iOSSwipeViewTransitionModePage.xaml (+ .xaml.cs) The C# iOSSwipeViewTransitionModePage is a StackLayout with: a horizontal row holding a &amp;quot;SwipeTransitionMode:&amp;quot; Label + an EnumPicker over the SwipeTransitionMode enum (Reveal / Drag, Se

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 97. Ios Time Picker — 🟢/🟢
<sub>ios_time_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/ios_time_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_time_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_time_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_time_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_time_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/ios_time_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/ios_time_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/ios_time_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/ios_time_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/ios_time_picker_dark.png" /></td></tr></table>

ports iOSTimePickerPage.xaml The .NET MAUI PlatformSpecifics sample (Pages/PlatformSpecifics/iOS/iOSTimePickerPage.xaml + .xaml.cs): a TimePicker carrying the iOSSpecific TimePicker.UpdateMode knob (XAML seeds it to WhenFinished) over a but

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 98. Items — 🟢/🟢
<sub>items</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/items_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/items_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/items_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/items_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/items_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/items_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/items_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/items_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/items_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/items_dark.png" /></td></tr></table>

a self-contained demo page for the W2-19 items core: a collection_view over a live observable items source with a templated cell, single selection driving a readout label, and an EmptyView for the cleared state (the C# CollectionView galler

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 99. Items Updating Scroll Mode — 🟢/🟢
<sub>items_updating_scroll_mode</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/items_updating_scroll_mode_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/items_updating_scroll_mode_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/items_updating_scroll_mode_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/items_updating_scroll_mode_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/items_updating_scroll_mode_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/items_updating_scroll_mode_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/items_updating_scroll_mode_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/items_updating_scroll_mode_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/items_updating_scroll_mode_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/items_updating_scroll_mode_dark.png" /></td></tr></table>

ports ItemsUpdatingScrollModeGallery.xaml (+ .xaml.cs) of the C# CollectionView gallery (Maui.Controls.Sample.Pages.CollectionViewGalleries.ScrollModeGalleries.ItemsUpdatingScrollModeGallery)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 100. Label — 🟢/🟢
<sub>label</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/label_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/label_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/label_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/label_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/label_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/label_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/label_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/label_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/label_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/label_dark.png" /></td></tr></table>

ports LabelPage.xaml (+ LabelPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 101. Layout Is Enabled — 🟢/🟢
<sub>layout_is_enabled</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/layout_is_enabled_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/layout_is_enabled_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/layout_is_enabled_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/layout_is_enabled_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/layout_is_enabled_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/layout_is_enabled_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/layout_is_enabled_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/layout_is_enabled_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/layout_is_enabled_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/layout_is_enabled_dark.png" /></td></tr></table>

ports LayoutIsEnabledPage.xaml (+ LayoutIsEnabledPage.xaml.cs) The C# page demonstrates how IsEnabled on a layout cascades to its children: a 2x2 grid whose left column hosts a &amp;quot;MainLayout&amp;quot; full of state-demo sub-stacks (all-enabled / all-d

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 102. Line Gallery — 🟢/🟢
<sub>line_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/line_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/line_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/line_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/line_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/line_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/line_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/line_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/line_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/line_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/line_gallery_dark.png" /></td></tr></table>

ports LineGallery.xaml A self-contained, code-first port of the MAUI Shapes LineGallery (Pages/Controls/ShapesGalleries/LineGallery.xaml + .xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 103. Line Join Gallery — 🟢/🟢
<sub>line_join_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/line_join_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/line_join_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/line_join_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/line_join_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/line_join_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/line_join_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/line_join_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/line_join_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/line_join_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/line_join_gallery_dark.png" /></td></tr></table>

ports LineJoinGallery.xaml A self-contained, code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/LineJoinGallery.xaml: a StackLayout that demonstrates the three StrokeLineJoin variants on an identical open polyline

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 104. Measure First Strategy — 🟢/🟢
<sub>measure_first_strategy</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/measure_first_strategy_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/measure_first_strategy_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/measure_first_strategy_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/measure_first_strategy_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/measure_first_strategy_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/measure_first_strategy_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/measure_first_strategy_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/measure_first_strategy_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/measure_first_strategy_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/measure_first_strategy_dark.png" /></td></tr></table>

ports MeasureFirstStrategy.xaml (+ .xaml.cs) of the C# CollectionView gallery (Maui.Controls.Sample.Pages.CollectionViewGalleries.GroupingGalleries.MeasureFirstStrategy)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 105. Menu Bar — 🟢/🟢
<sub>menu_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/menu_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/menu_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/menu_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/menu_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/menu_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/menu_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/menu_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/menu_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/menu_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/menu_bar_dark.png" /></td></tr></table>

ports MenuBarPage.xaml (+ MenuBarPage.cs) The C# page declares three page-level MenuBarItems (Page.MenuBarItems — the app menu bar) and a small visible body: - &amp;quot;Before File&amp;quot; : &amp;quot;Before File Action&amp;quot; (accelerator &amp;quot;b&amp;quot;), &amp;quot;Cool item 1&amp;quot;, a separat

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 106. Modal — 🟢/🟢
<sub>modal</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/modal_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/modal_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/modal_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/modal_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/modal_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/modal_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/modal_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/modal_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/modal_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/modal_dark.png" /></td></tr></table>

ports ModalPage.xaml (+ .xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 107. Multiple Bound Selection — 🟢/🟢
<sub>multiple_bound_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/multiple_bound_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/multiple_bound_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/multiple_bound_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/multiple_bound_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/multiple_bound_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/multiple_bound_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/multiple_bound_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/multiple_bound_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/multiple_bound_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/multiple_bound_selection_dark.png" /></td></tr></table>

ports MultipleBoundSelection.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.MultipleBoundSelection)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 108. Navigation Gallery — 🟢/🟢
<sub>navigation_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/navigation_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/navigation_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/navigation_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/navigation_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/navigation_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/navigation_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/navigation_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/navigation_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/navigation_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/navigation_gallery_dark.png" /></td></tr></table>

ports NavigationGallery.xaml (+ .xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 109. Nested Collection — 🟡/🟡
<sub>nested_collection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/nested_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/nested_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/nested_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/nested_collection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/nested_collection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/nested_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/nested_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/nested_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/nested_collection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/nested_collection_dark.png" /></td></tr></table>

ports NestedGalleries/NestedCollectionViewGallery.xaml (+ NestedCollectionViewGallery.xaml.cs) of the C# CollectionView gallery

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9166, 3.18% pixels differ · Dark: SSIM 0.9834, 3.76% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9154, 3.21% pixels differ · Dark: SSIM 0.9822, 3.79% pixels differ

### 110. Pan Gesture Events — 🟢/🟢
<sub>pan_gesture_events</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/pan_gesture_events_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/pan_gesture_events_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/pan_gesture_events_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/pan_gesture_events_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/pan_gesture_events_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/pan_gesture_events_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/pan_gesture_events_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/pan_gesture_events_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/pan_gesture_events_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/pan_gesture_events_dark.png" /></td></tr></table>

ports PanGestureEventsGallery.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.PanGestureEventsGallery)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 111. Path Aspect Gallery — 🟢/🟢
<sub>path_aspect_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/path_aspect_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/path_aspect_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/path_aspect_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/path_aspect_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/path_aspect_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/path_aspect_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/path_aspect_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/path_aspect_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/path_aspect_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/path_aspect_gallery_dark.png" /></td></tr></table>

ports PathAspectGallery.xaml A self-contained, code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PathAspectGallery.xaml: a StackLayout (Padding 12) that demonstrates the four Path Aspect modes on one identical ge

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 112. Path Gallery — 🟢/🟢
<sub>path_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/path_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/path_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/path_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/path_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/path_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/path_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/path_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/path_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/path_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/path_gallery_dark.png" /></td></tr></table>

ports PathGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PathGallery.xaml: a ScrollView over a StackLayout (Padding 12) that walks eight Path variants (plus two caption-only markup-string Labels

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 113. Path Transform String — 🟢/🟢
<sub>path_transform_string</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/path_transform_string_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/path_transform_string_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/path_transform_string_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/path_transform_string_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/path_transform_string_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/path_transform_string_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/path_transform_string_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/path_transform_string_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/path_transform_string_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/path_transform_string_dark.png" /></td></tr></table>

ports PathTransformStringGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PathTransformStringGallery.xaml: a ScrollView over a StackLayout (Padding 12) that shows the SAME two-figure Path geometry

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 114. Picker — 🟢/🟢
<sub>picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/picker_dark.png" /></td></tr></table>

ports PickerPage.xaml (+ PickerPage.xaml.cs) A self-contained, code-first demo page for the Picker control (the C# gallery-page convention, mirroring the value_controls_page / pickers_page pattern)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 115. Pickers — 🟢/🟢
<sub>pickers</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/pickers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/pickers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/pickers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/pickers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/pickers_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/pickers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/pickers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/pickers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/pickers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/pickers_dark.png" /></td></tr></table>

a self-contained demo page for the W1-06 picker set: picker, date_picker and time_picker on one vertical stack, wired together so every selection drives a visible output (the C# gallery-page convention, code-first; the value_controls_page p

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 116. Pointer Gesture — 🟢/🟢
<sub>pointer_gesture</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/pointer_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/pointer_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/pointer_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/pointer_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/pointer_gesture_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/pointer_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/pointer_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/pointer_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/pointer_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/pointer_gesture_dark.png" /></td></tr></table>

ports PointerGestureGalleryPage.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.PointerGestureGalleryPage)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 117. Polygon Gallery — 🟢/🟢
<sub>polygon_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/polygon_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/polygon_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/polygon_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/polygon_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/polygon_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/polygon_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/polygon_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/polygon_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/polygon_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/polygon_gallery_dark.png" /></td></tr></table>

ports PolygonGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PolygonGallery.xaml: a ScrollView over a StackLayout (Padding 12) that walks four Polygon variants, each under a caption Label — - &amp;quot;A

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 118. Polyline Gallery — 🟢/🟢
<sub>polyline_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/polyline_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/polyline_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/polyline_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/polyline_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/polyline_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/polyline_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/polyline_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/polyline_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/polyline_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/polyline_gallery_dark.png" /></td></tr></table>

ports PolylineGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PolylineGallery.xaml: a StackLayout (Padding 12 — no ScrollView in the C# source) holding two Polyline variants, each under a caption

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 119. Preselected Item — 🟢/🟢
<sub>preselected_item</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/preselected_item_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/preselected_item_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/preselected_item_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/preselected_item_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/preselected_item_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/preselected_item_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/preselected_item_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/preselected_item_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/preselected_item_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/preselected_item_dark.png" /></td></tr></table>

ports PreselectedItemGallery.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.PreselectedItemGallery)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 120. Preselected Items — 🟢/🟢
<sub>preselected_items</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/preselected_items_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/preselected_items_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/preselected_items_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/preselected_items_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/preselected_items_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/preselected_items_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/preselected_items_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/preselected_items_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/preselected_items_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/preselected_items_dark.png" /></td></tr></table>

ports PreselectedItemsGallery.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.PreselectedItemsGallery)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 121. Progress Bar — 🟢/🟢
<sub>progress_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/progress_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/progress_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/progress_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/progress_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/progress_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/progress_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/progress_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/progress_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/progress_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/progress_bar_dark.png" /></td></tr></table>

ports ProgressBarPage.xaml (+ ProgressBarPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 122. Radio Button Border — 🟢/🟢
<sub>radio_button_border</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/radio_button_border_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_button_border_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_button_border_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_button_border_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_button_border_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/radio_button_border_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_button_border_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_button_border_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_button_border_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_button_border_dark.png" /></td></tr></table>

ports RadioButtonBorder.xaml A self-contained, code-first demo of RadioButton border styling

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 123. Radio Button Content — 🟢/🟢
<sub>radio_button_content</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/radio_button_content_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_button_content_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_button_content_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_button_content_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_button_content_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/radio_button_content_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_button_content_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_button_content_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_button_content_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_button_content_dark.png" /></td></tr></table>

ports RadioButtonContentGallery.xaml A self-contained, code-first demo of the RadioButton.Content surface

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 124. Radio Button Group — 🟢/🟢
<sub>radio_button_group</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/radio_button_group_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_button_group_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_button_group_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_button_group_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_button_group_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/radio_button_group_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_button_group_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_button_group_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_button_group_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_button_group_dark.png" /></td></tr></table>

ports RadioButtonGroupGallery.xaml A self-contained, code-first demo of the RadioButtonGroup ATTACHED-PROPERTY grouping: a vertical StackLayout carries RadioButtonGroup.GroupName=&amp;quot;foo&amp;quot;, so every descendant RadioButton — including one nested

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 125. Radio Button Group Binding — 🟢/🟢
<sub>radio_button_group_binding</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/radio_button_group_binding_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_button_group_binding_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_button_group_binding_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_button_group_binding_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_button_group_binding_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/radio_button_group_binding_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_button_group_binding_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_button_group_binding_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_button_group_binding_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_button_group_binding_dark.png" /></td></tr></table>

ports RadioButtonGroupBindingGallery.xaml A code-first demo of binding the RadioButtonGroup attached properties (GroupName + SelectedValue) to a view-model

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 126. Radio Button Group Gallery — 🟢/🟢
<sub>radio_button_group_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/radio_button_group_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_button_group_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_button_group_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_button_group_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_button_group_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/radio_button_group_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_button_group_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_button_group_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_button_group_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_button_group_gallery_dark.png" /></td></tr></table>

ports RadioButtonGroupGalleryPage.xaml A self-contained, code-first demo of RadioButton grouping SCOPE, mirroring the C# controls gallery page (Pages/Controls/RadioButtonGalleries/RadioButtonGroupGalleryPage.xaml)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 127. Radio Content Properties — 🟢/🟢
<sub>radio_content_properties</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/radio_content_properties_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_content_properties_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_content_properties_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_content_properties_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_content_properties_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/radio_content_properties_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_content_properties_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_content_properties_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_content_properties_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_content_properties_dark.png" /></td></tr></table>

ports ContentProperties.xaml A self-contained, code-first demo of how RadioButton propagates the standard Text/Font properties to its Content

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 128. Radio Template From Style — 🟢/🟢
<sub>radio_template_from_style</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/radio_template_from_style_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_template_from_style_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_template_from_style_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_template_from_style_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_template_from_style_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/radio_template_from_style_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/radio_template_from_style_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/radio_template_from_style_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/radio_template_from_style_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/radio_template_from_style_dark.png" /></td></tr></table>

ports TemplateFromStyle.xaml A self-contained, code-first demo of applying a RadioButton ControlTemplate

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 129. Rectangle Gallery — 🟢/🟢
<sub>rectangle_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/rectangle_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/rectangle_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/rectangle_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/rectangle_gallery_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/rectangle_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/rectangle_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/rectangle_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/rectangle_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/rectangle_gallery_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/rectangle_gallery_dark.png" /></td></tr></table>

ports RectangleGallery.xaml A self-contained, code-first port of the MAUI Shapes RectangleGallery (Pages/Controls/ShapesGalleries/RectangleGallery.xaml + .xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 130. Refresh View — 🟢/🟢
<sub>refresh_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/refresh_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/refresh_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/refresh_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/refresh_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/refresh_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/refresh_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/refresh_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/refresh_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/refresh_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/refresh_view_dark.png" /></td></tr></table>

ports RefreshViewPage.xaml (+ RefreshViewPage.xaml.cs + RefreshViewModel.cs) A self-contained, code-first demo page for the RefreshView control (the C# gallery-page convention, mirroring the swipe_refresh_page pattern)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 131. Relative Layout — 🟢/🟢
<sub>relative_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/relative_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/relative_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/relative_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/relative_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/relative_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/relative_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/relative_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/relative_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/relative_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/relative_layout_dark.png" /></td></tr></table>

ports RelativeLayoutPage.xaml

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 132. Scattered Radio Button — 🟢/🟢
<sub>scattered_radio_button</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/scattered_radio_button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/scattered_radio_button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/scattered_radio_button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/scattered_radio_button_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/scattered_radio_button_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/scattered_radio_button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/scattered_radio_button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/scattered_radio_button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/scattered_radio_button_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/scattered_radio_button_dark.png" /></td></tr></table>

ports ScatteredRadioButtonGallery.xaml A code-first demo that radio buttons DON&amp;#x27;T have to share a container to be grouped: grouping is by GroupName, so buttons scattered across separate containers (and one bare button outside any grouped co

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 133. Scroll Mode Test — 🟢/🟢
<sub>scroll_mode_test</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/scroll_mode_test_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/scroll_mode_test_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/scroll_mode_test_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/scroll_mode_test_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/scroll_mode_test_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/scroll_mode_test_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/scroll_mode_test_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/scroll_mode_test_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/scroll_mode_test_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/scroll_mode_test_dark.png" /></td></tr></table>

ports ScrollModeTestGallery.xaml (+ .xaml.cs) of the C# CollectionView gallery (Maui.Controls.Sample.Pages.CollectionViewGalleries.ScrollModeGalleries.ScrollModeTestGallery)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 134. Scroll To Group — 🟢/🟢
<sub>scroll_to_group</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/scroll_to_group_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/scroll_to_group_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/scroll_to_group_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/scroll_to_group_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/scroll_to_group_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/scroll_to_group_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/scroll_to_group_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/scroll_to_group_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/scroll_to_group_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/scroll_to_group_dark.png" /></td></tr></table>

ports ScrollToGalleries/ScrollToGroup.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 135. Scroll View — 🟡/🟡
<sub>scroll_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/scroll_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/scroll_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/scroll_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/scroll_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/scroll_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/scroll_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/scroll_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/scroll_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/scroll_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/scroll_view_dark.png" /></td></tr></table>

ports ScrollViewPage.xaml (+ the ScrollViewPages sub-demos: ScrollViewOrientationPage / ScrollToEndPage / ScrollToFromConstructorPage), code-first

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9483, 1.28% pixels differ · Dark: SSIM 0.9973, 0.10% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9472, 1.31% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 136. Search Bar — 🟢/🟢
<sub>search_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/search_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/search_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/search_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/search_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/search_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/search_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/search_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/search_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/search_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/search_bar_dark.png" /></td></tr></table>

ports SearchBarPage.xaml (Microsoft.Maui.Controls sample gallery)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9977, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9965, 0.11% pixels differ

### 137. Selection Command Param — 🟢/🟢
<sub>selection_command_param</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/selection_command_param_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/selection_command_param_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/selection_command_param_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/selection_command_param_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/selection_command_param_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/selection_command_param_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/selection_command_param_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/selection_command_param_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/selection_command_param_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/selection_command_param_dark.png" /></td></tr></table>

ports SelectionChangedCommandParameter.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.SelectionChangedCommandParameter)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 138. Selection Synchronization — 🟢/🟢
<sub>selection_synchronization</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/selection_synchronization_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/selection_synchronization_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/selection_synchronization_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/selection_synchronization_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/selection_synchronization_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/selection_synchronization_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/selection_synchronization_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/selection_synchronization_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/selection_synchronization_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/selection_synchronization_dark.png" /></td></tr></table>

ports SelectionSynchronization.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.SelectionSynchronization)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 139. Semantics — 🟢/🟢
<sub>semantics</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/semantics_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/semantics_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/semantics_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/semantics_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/semantics_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/semantics_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/semantics_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/semantics_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/semantics_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/semantics_dark.png" /></td></tr></table>

ports SemanticsPage.xaml (+ SemanticsPage.xaml.cs) The C# SemanticsPage is an accessibility showcase: a long VerticalStackLayout where nearly every control carries SemanticProperties.Description / .Hint, plus a block of labels exercising Se

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 140. Shadow Playground — 🟢/🟢
<sub>shadow_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/shadow_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/shadow_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/shadow_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/shadow_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/shadow_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/shadow_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/shadow_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/shadow_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/shadow_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/shadow_playground_dark.png" /></td></tr></table>

ports ShadowPlaygroundPage.xaml A self-contained, code-first demo of the view Shadow surface, mirroring the C# core gallery page (Pages/Core/ShadowGalleries/ShadowPlaygroundPage.xaml + .xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 141. Shape App Theme — 🟡/🟡
<sub>shape_app_theme</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/shape_app_theme_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/shape_app_theme_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/shape_app_theme_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/shape_app_theme_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/shape_app_theme_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/shape_app_theme_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/shape_app_theme_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/shape_app_theme_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/shape_app_theme_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/shape_app_theme_dark.png" /></td></tr></table>

ports ShapeAppThemeGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/ShapeAppThemeGallery.xaml: a StackLayout (Padding 12) holding a caption Label and a 200x80 Rectangle, all themed via {AppThemeBi

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9576, 3.78% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9576, 3.77% pixels differ

### 142. Shapes — 🟢/🟢
<sub>shapes</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/shapes_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/shapes_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/shapes_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/shapes_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/shapes_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/shapes_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/shapes_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/shapes_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/shapes_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/shapes_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;shapes&amp;quot; demo (ComparePages.Shapes()), the shipped-.NET-MAUI reference for the visual-parity comparison: a ScrollView over a vertical stack of four LABELLED shapes, each bold-captioned and Start-a

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 143. Single Bound Selection — 🟢/🟢
<sub>single_bound_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/single_bound_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/single_bound_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/single_bound_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/single_bound_selection_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/single_bound_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/single_bound_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/single_bound_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/single_bound_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/single_bound_selection_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/single_bound_selection_dark.png" /></td></tr></table>

ports SingleBoundSelection.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.SingleBoundSelection)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 144. Slider — 🟢/🟢
<sub>slider</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/slider_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/slider_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/slider_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/slider_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/slider_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/slider_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/slider_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/slider_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/slider_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/slider_dark.png" /></td></tr></table>

ports SliderPage.xaml (+ .xaml.cs) Mirrors the MAUI gallery page: a vertical stack of headlined Slider states — Default, BackgroundColor (Blue), Background (yellow→green LinearGradientBrush), Minimum(5)/Maximum(15) with a value readout (Val

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 145. Some Empty Groups — 🟢/🟢
<sub>some_empty_groups</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/some_empty_groups_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/some_empty_groups_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/some_empty_groups_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/some_empty_groups_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/some_empty_groups_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/some_empty_groups_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/some_empty_groups_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/some_empty_groups_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/some_empty_groups_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/some_empty_groups_dark.png" /></td></tr></table>

ports GroupingGalleries/SomeEmptyGroups.xaml (+ .xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9950, 0.25% pixels differ · Dark: SSIM 0.9944, 0.32% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 146. Stack Layout — 🟢/🟢
<sub>stack_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/stack_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/stack_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/stack_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/stack_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/stack_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/stack_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/stack_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/stack_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/stack_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/stack_layout_dark.png" /></td></tr></table>

ports StackLayoutPage.xaml Demonstrates the generic maui::controls::stack_layout (the orientation-switching sibling of the fixed vertical/horizontal stacks) by nesting two inner stacks inside an outer vertical stack with a 12px margin: a &amp;quot;V

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 147. Staggered Layout — 🟢/🟢
<sub>staggered_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/staggered_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/staggered_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/staggered_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/staggered_layout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/staggered_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/staggered_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/staggered_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/staggered_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/staggered_layout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/staggered_layout_dark.png" /></td></tr></table>

ports AlternateLayoutGalleries/StaggeredLayout.xaml (+ StaggeredLayout.xaml.cs) of the C# CollectionView gallery

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 148. Stepper — 🟢/🟢
<sub>stepper</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/stepper_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/stepper_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/stepper_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/stepper_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/stepper_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/stepper_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/stepper_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/stepper_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/stepper_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/stepper_dark.png" /></td></tr></table>

ports StepperPage.xaml (+ StepperPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 149. Styles — 🟢/🟢
<sub>styles</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/styles_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/styles_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/styles_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/styles_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/styles_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/styles_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/styles_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/styles_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/styles_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/styles_dark.png" /></td></tr></table>

ports StylesPage.xaml

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 150. Swipe Gesture — 🟢/🟢
<sub>swipe_gesture</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/swipe_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_gesture_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_gesture_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/swipe_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_gesture_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_gesture_dark.png" /></td></tr></table>

ports SwipeViewGestureRecognizerGallery.xaml (+ .xaml.cs) The MAUI SwipeViewGestureRecognizerGallery is a CollectionView of &amp;quot;message&amp;quot; rows; each row&amp;#x27;s DataTemplate is a SwipeView wired three ways, proving gesture recognizers AND swipe-item

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9979, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9966, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 151. Swipe Item Position — 🟢/🟢
<sub>swipe_item_position</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/swipe_item_position_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_item_position_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_item_position_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_item_position_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_item_position_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/swipe_item_position_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_item_position_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_item_position_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_item_position_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_item_position_dark.png" /></td></tr></table>

ports SwipeItemPositionGallery.xaml A code-first port of the MAUI SwipeView sub-gallery Pages/Controls/SwipeViewGalleries/SwipeItemPositionGallery.xaml: a 2-row Grid (Auto / *) with a Picker on top and one SwipeView below that carries TWO S

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9979, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 152. Swipe Item Size — 🟢/🟢
<sub>swipe_item_size</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/swipe_item_size_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_item_size_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_item_size_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_item_size_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_item_size_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/swipe_item_size_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_item_size_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_item_size_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_item_size_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_item_size_dark.png" /></td></tr></table>

ports SwipeItemSizeGallery.xaml A self-contained, code-first port of the .NET MAUI &amp;quot;SwipeItem Size Gallery&amp;quot;: a scrolling stack of swipe_views demonstrating how a left SwipeItem&amp;#x27;s icon size and the SwipeView content size interact

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 153. Swipe Refresh — 🟢/🟢
<sub>swipe_refresh</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/swipe_refresh_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_refresh_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_refresh_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_refresh_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_refresh_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/swipe_refresh_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_refresh_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_refresh_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_refresh_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_refresh_dark.png" /></td></tr></table>

a self-contained demo page for the W2-20 swipe + refresh controls: a refresh_view wrapping a swipe_view (which itself wraps a labeled row), with a readout label reflecting the latest interaction

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9906, 0.32% pixels differ · Dark: SSIM 0.9901, 0.40% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9893, 0.35% pixels differ · Dark: SSIM 0.9888, 0.43% pixels differ

### 154. Swipe Threshold — 🟢/🟢
<sub>swipe_threshold</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/swipe_threshold_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_threshold_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_threshold_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_threshold_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_threshold_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/swipe_threshold_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_threshold_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_threshold_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_threshold_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_threshold_dark.png" /></td></tr></table>

ports HorizontalSwipeThresholdGallery.xaml (+ .xaml.cs) The MAUI HorizontalSwipeThresholdGallery shows how SwipeView.Threshold (the swipe distance, in DIPs, the user must drag before the items settle open / execute) interacts with SwipeItem

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9979, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9966, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 155. Swipe View Margin — 🟢/🟢
<sub>swipe_view_margin</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/swipe_view_margin_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_view_margin_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_view_margin_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_view_margin_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_view_margin_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/swipe_view_margin_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_view_margin_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_view_margin_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_view_margin_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_view_margin_dark.png" /></td></tr></table>

ports SwipeViewMarginGallery.xaml A self-contained, code-first port of the .NET MAUI &amp;quot;SwipeView Margin Gallery&amp;quot;: two swipe_views whose content&amp;#x27;s Margin + Padding are driven by two sliders, demonstrating that the revealed SwipeItems stay cor

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 156. Swipe View Shadow — 🟡/🟡
<sub>swipe_view_shadow</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/swipe_view_shadow_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_view_shadow_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_view_shadow_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_view_shadow_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_view_shadow_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/swipe_view_shadow_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/swipe_view_shadow_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/swipe_view_shadow_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/swipe_view_shadow_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/swipe_view_shadow_dark.png" /></td></tr></table>

ports SwipeViewShadowGallery.xaml A code-first port of the MAUI SwipeView sub-gallery Pages/Controls/SwipeViewGalleries/SwipeViewShadowGallery.xaml: a padded vertical StackLayout proving a drop Shadow renders correctly on SwipeView content

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9867, 1.36% pixels differ · Dark: SSIM 0.9930, 0.08% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9869, 1.39% pixels differ · Dark: SSIM 0.9916, 0.11% pixels differ

### 157. Switch — 🟢/🟢
<sub>switch</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/switch_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/switch_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/switch_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/switch_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/switch_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/switch_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/switch_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/switch_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/switch_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/switch_dark.png" /></td></tr></table>

ports SwitchPage.xaml (+ .xaml.cs) Mirrors the MAUI gallery page: a vertical stack of headlined Switch states — Default, BackgroundColor (Blue), Background (a yellow→green LinearGradientBrush), Disabled, OnColor (Red), ThumbColor (Orange)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9969, 0.15% pixels differ · Dark: SSIM 0.9977, 0.14% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9957, 0.18% pixels differ · Dark: SSIM 0.9964, 0.17% pixels differ

### 158. Switch Grouping — 🟢/🟢
<sub>switch_grouping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/switch_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/switch_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/switch_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/switch_grouping_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/switch_grouping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/switch_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/switch_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/switch_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/switch_grouping_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/switch_grouping_dark.png" /></td></tr></table>

ports CollectionViewGalleries/GroupingGalleries/ SwitchGrouping.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 159. Tabbed Flyout — 🟢/🟢
<sub>tabbed_flyout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/tabbed_flyout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/tabbed_flyout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/tabbed_flyout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/tabbed_flyout_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/tabbed_flyout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/tabbed_flyout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/tabbed_flyout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/tabbed_flyout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/tabbed_flyout_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/tabbed_flyout_dark.png" /></td></tr></table>

a self-contained demo page for the W1-10 tabbed + flyout vertical: a flyout_page whose FLYOUT pane is a titled menu (two buttons selecting the detail&amp;#x27;s tabs + a &amp;quot;Toggle flyout&amp;quot; presenting/dismissing itself) and whose DETAIL pane is a tabbed

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 160. Templated View — 🟢/🟢
<sub>templated_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/templated_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/templated_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/templated_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/templated_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/templated_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/templated_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/templated_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/templated_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/templated_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/templated_view_dark.png" /></td></tr></table>

ports TemplatedViewPage.xaml The C# page contrasts a standard CardView control with a compact one driven by a ControlTemplate (&amp;quot;CardViewCompressed&amp;quot;) and a custom Rate control built entirely from a ControlTemplate + a heart PathGeometry

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 161. Time Picker — 🟢/🟢
<sub>time_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/time_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/time_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/time_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/time_picker_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/time_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/time_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/time_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/time_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/time_picker_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/time_picker_dark.png" /></td></tr></table>

ports TimePickerPage.xaml (+ TimePickerPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 162. Title Bar — 🟢/🟢
<sub>title_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/title_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/title_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/title_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/title_bar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/title_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/title_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/title_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/title_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/title_bar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/title_bar_dark.png" /></td></tr></table>

ports TitleBarPage.xaml A self-contained, code-first demo of the TitleBar control

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 163. Toolbar — 🟢/🟢
<sub>toolbar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/toolbar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/toolbar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/toolbar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/toolbar_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/toolbar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/toolbar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/toolbar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/toolbar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/toolbar_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/toolbar_dark.png" /></td></tr></table>

ports ToolbarPage.xaml (Maui.Controls.Sample.Pages.ToolbarPage)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 164. Transform Playground — 🟢/🟢
<sub>transform_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/transform_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/transform_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/transform_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/transform_playground_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/transform_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/transform_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/transform_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/transform_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/transform_playground_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/transform_playground_dark.png" /></td></tr></table>

ports TransformPlaygroundGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/TransformPlaygroundGallery.xaml: a 50x50 Path rectangle (red fill, blue stroke 4) sits in a 200x200 light-grey panel; belo

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 165. Transformations — 🟢/🟢
<sub>transformations</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/transformations_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/transformations_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/transformations_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/transformations_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/transformations_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/transformations_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/transformations_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/transformations_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/transformations_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/transformations_dark.png" /></td></tr></table>

ports TransformationsPage.xaml (+ .xaml.cs) The MAUI TransformationsPage drives a single target view&amp;#x27;s render transforms from a column of knobs: Sliders for Scale / ScaleX / ScaleY (Maximum 10) and Rotation / RotationX / RotationY (Maximum

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 166. Triggers — 🟢/🟢
<sub>triggers</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/triggers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/triggers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/triggers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/triggers_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/triggers_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/triggers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/triggers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/triggers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/triggers_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/triggers_dark.png" /></td></tr></table>

ports TriggersPage.xaml

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 167. Update Path Data — 🟢/🟢
<sub>update_path_data</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/update_path_data_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/update_path_data_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/update_path_data_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/update_path_data_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/update_path_data_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/update_path_data_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/update_path_data_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/update_path_data_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/update_path_data_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/update_path_data_dark.png" /></td></tr></table>

ports UpdatePathDataGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/UpdatePathDataGallery.xaml: a 2-row Grid (RowSpacing 0) that proves a Path repaints when its Data geometry is replaced at runti

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 168. Varied Size Selector — 🟢/🟢
<sub>varied_size_selector</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/varied_size_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/varied_size_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/varied_size_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/varied_size_selector_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/varied_size_selector_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/varied_size_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/varied_size_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/varied_size_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/varied_size_selector_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/varied_size_selector_dark.png" /></td></tr></table>

ports DataTemplateSelectorGalleries/VariedSizeDataTemplateSelectorGallery.xaml (+ VariedSizeDataTemplateSelectorGallery.xaml.cs) of the C# CollectionView gallery

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 169. Vertical Stack — 🟢/🟢
<sub>vertical_stack</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/vertical_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/vertical_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/vertical_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/vertical_stack_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/vertical_stack_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/vertical_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/vertical_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/vertical_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/vertical_stack_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/vertical_stack_dark.png" /></td></tr></table>

Vertical Stack

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9978, 0.09% pixels differ · Dark: SSIM 0.9978, 0.08% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.12% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 170. Visual States — 🟢/🟢
<sub>visual_states</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/visual_states_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/visual_states_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/visual_states_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/visual_states_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/visual_states_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/visual_states_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/visual_states_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/visual_states_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/visual_states_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/visual_states_dark.png" /></td></tr></table>

ports VisualStatesPage.xaml

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 171. Web View — 🟢/🟢
<sub>web_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/web_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/web_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/web_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/web_view_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/web_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/web_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/web_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/web_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/web_view_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/web_view_dark.png" /></td></tr></table>

a self-contained demo page for the W1-08 web_view vertical: a web_view loading a STATIC html_web_view_source (no network), back/forward/reload buttons over the handler-pushed CanGoBack/CanGoForward read-onlys, an &amp;quot;Eval 1+1&amp;quot; button driving t

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 172. Z Index — 🟢/🟢
<sub>z_index</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th><th>AppKit / C++</th><th>AppKit / C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/maccatalyst/maui/z_index_light.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/z_index_light.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/z_index_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/z_index_light.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/z_index_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/maccatalyst/maui/z_index_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/cpp/z_index_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/xaml/z_index_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_cpp/z_index_dark.png" /></td><td><img width="300px" src="captures/maccatalyst/appkit_xaml/z_index_dark.png" /></td></tr></table>

ports ZIndexPage.xaml (+ ZIndexPage.xaml.cs), code-first

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

</details>

<details>
<summary><h2>Android (172 examples) — click to expand</h2></summary>

Real .NET MAUI vs the C++ port vs the compile-time-XAML gallery, captured on the same Android emulator in light and dark. MAUI is the content ground truth.

**Discrepancy counts** (MAUI-vs-C++ parity verdicts from the deterministic pixel-perfect score — SSIM + per-pixel diff; AI-based review has been invalidated/removed):

| Classification | Pixel-Perfect Score — C++ (C1/C3) | Pixel-Perfect Score — C++ &amp; XAML (C2/C4) |
| --- | --- | --- |
| 🟢 Match | 143 | 143 |
| 🟡 Minor | 25 | 24 |
| 🔴 Major | 4 | 5 |
| ⬛ Blank | 0 | 0 |
| ⏳ Unreviewed | 0 | 0 |

### 1. Absolute Layout — 🟢/🟢
<sub>absolute_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/absolute_layout_light.png" /></td><td><img width="300px" src="captures/android/cpp/absolute_layout_light.png" /></td><td><img width="300px" src="captures/android/xaml/absolute_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/absolute_layout_dark.png" /></td><td><img width="300px" src="captures/android/cpp/absolute_layout_dark.png" /></td><td><img width="300px" src="captures/android/xaml/absolute_layout_dark.png" /></td></tr></table>

ports AbsoluteLayoutPage.xaml A self-contained, code-first demo of the AbsoluteLayout control

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9903, 0.63% pixels differ · Dark: SSIM 0.9948, 0.64% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9999, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 2. Activity Indicator — 🟡/🟡
<sub>activity_indicator</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/activity_indicator_light.png" /></td><td><img width="300px" src="captures/android/cpp/activity_indicator_light.png" /></td><td><img width="300px" src="captures/android/xaml/activity_indicator_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/activity_indicator_dark.png" /></td><td><img width="300px" src="captures/android/cpp/activity_indicator_dark.png" /></td><td><img width="300px" src="captures/android/xaml/activity_indicator_dark.png" /></td></tr></table>

ports ActivityIndicatorPage.xaml (+ ActivityIndicatorPage.xaml.cs)

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9897, 1.39% pixels differ · Dark: SSIM 0.9803, 1.47% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9897, 1.39% pixels differ · Dark: SSIM 0.9803, 1.47% pixels differ

### 3. Adaptive Collection — 🟢/🟢
<sub>adaptive_collection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/adaptive_collection_light.png" /></td><td><img width="300px" src="captures/android/cpp/adaptive_collection_light.png" /></td><td><img width="300px" src="captures/android/xaml/adaptive_collection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/adaptive_collection_dark.png" /></td><td><img width="300px" src="captures/android/cpp/adaptive_collection_dark.png" /></td><td><img width="300px" src="captures/android/xaml/adaptive_collection_dark.png" /></td></tr></table>

ports AdaptiveCollectionView.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.AdaptiveCollectionView)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9976, 0.14% pixels differ · Dark: SSIM 0.9976, 0.16% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 4. Alerts — 🟢/🟢
<sub>alerts</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/alerts_light.png" /></td><td><img width="300px" src="captures/android/cpp/alerts_light.png" /></td><td><img width="300px" src="captures/android/xaml/alerts_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/alerts_dark.png" /></td><td><img width="300px" src="captures/android/cpp/alerts_dark.png" /></td><td><img width="300px" src="captures/android/xaml/alerts_dark.png" /></td></tr></table>

ports AlertsPage.xaml (+ AlertsPage.xaml.cs) The C# AlertsPage drives the three Page dialog services — DisplayAlertAsync (simple OK + Yes/No), DisplayActionSheetAsync (simple + Cancel/Delete), and DisplayPromptAsync (two questions) — from a

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 5. Alignment — 🟡/🟡
<sub>alignment</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/alignment_light.png" /></td><td><img width="300px" src="captures/android/cpp/alignment_light.png" /></td><td><img width="300px" src="captures/android/xaml/alignment_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/alignment_dark.png" /></td><td><img width="300px" src="captures/android/cpp/alignment_dark.png" /></td><td><img width="300px" src="captures/android/xaml/alignment_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;alignment&amp;quot; demo (ComparePages.Alignment()), the shipped-.NET-MAUI reference for the visual-parity comparison

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9579, 1.75% pixels differ · Dark: SSIM 0.9820, 1.75% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9789, 1.01% pixels differ · Dark: SSIM 0.9840, 1.01% pixels differ

### 6. Animation — 🟡/🟡
<sub>animation</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/animation_light.png" /></td><td><img width="300px" src="captures/android/cpp/animation_light.png" /></td><td><img width="300px" src="captures/android/xaml/animation_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/animation_dark.png" /></td><td><img width="300px" src="captures/android/cpp/animation_dark.png" /></td><td><img width="300px" src="captures/android/xaml/animation_dark.png" /></td></tr></table>

ports AnimationPage.xaml (+ AnimationPage.xaml.cs)

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9748, 4.26% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9748, 4.26% pixels differ

### 7. App Theme Binding — 🟢/🟡
<sub>app_theme_binding</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/app_theme_binding_light.png" /></td><td><img width="300px" src="captures/android/cpp/app_theme_binding_light.png" /></td><td><img width="300px" src="captures/android/xaml/app_theme_binding_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/app_theme_binding_dark.png" /></td><td><img width="300px" src="captures/android/cpp/app_theme_binding_dark.png" /></td><td><img width="300px" src="captures/android/xaml/app_theme_binding_dark.png" /></td></tr></table>

ports AppThemeBindingPage.xaml

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9928, 1.10% pixels differ

### 8. Application Control — 🟢/🟢
<sub>application_control</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/application_control_light.png" /></td><td><img width="300px" src="captures/android/cpp/application_control_light.png" /></td><td><img width="300px" src="captures/android/xaml/application_control_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/application_control_dark.png" /></td><td><img width="300px" src="captures/android/cpp/application_control_dark.png" /></td><td><img width="300px" src="captures/android/xaml/application_control_dark.png" /></td></tr></table>

ports ApplicationControlPage.xaml (+ .xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 9. Auto Size Shapes — 🟢/🟢
<sub>auto_size_shapes</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/auto_size_shapes_light.png" /></td><td><img width="300px" src="captures/android/cpp/auto_size_shapes_light.png" /></td><td><img width="300px" src="captures/android/xaml/auto_size_shapes_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/auto_size_shapes_dark.png" /></td><td><img width="300px" src="captures/android/cpp/auto_size_shapes_dark.png" /></td><td><img width="300px" src="captures/android/xaml/auto_size_shapes_dark.png" /></td></tr></table>

ports AutoSizeShapesGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/AutoSizeShapesGallery.xaml: a 3-row Grid (RowSpacing 0) that proves a stroked Ellipse auto-sizes to fill exactly half of the av

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 10. Basic Grouping — 🟢/🟢
<sub>basic_grouping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/basic_grouping_light.png" /></td><td><img width="300px" src="captures/android/cpp/basic_grouping_light.png" /></td><td><img width="300px" src="captures/android/xaml/basic_grouping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/basic_grouping_dark.png" /></td><td><img width="300px" src="captures/android/cpp/basic_grouping_dark.png" /></td><td><img width="300px" src="captures/android/xaml/basic_grouping_dark.png" /></td></tr></table>

ports GroupingGalleries/BasicGrouping.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9930, 0.18% pixels differ · Dark: SSIM 0.9924, 0.18% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9999, 0.00% pixels differ

### 11. Basic Swipe — 🟢/🟢
<sub>basic_swipe</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/basic_swipe_light.png" /></td><td><img width="300px" src="captures/android/cpp/basic_swipe_light.png" /></td><td><img width="300px" src="captures/android/xaml/basic_swipe_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/basic_swipe_dark.png" /></td><td><img width="300px" src="captures/android/cpp/basic_swipe_dark.png" /></td><td><img width="300px" src="captures/android/xaml/basic_swipe_dark.png" /></td></tr></table>

ports BasicSwipeGallery.xaml A code-first port of the MAUI SwipeView sub-gallery Pages/Controls/SwipeViewGalleries/BasicSwipeGallery.xaml: a vertical StackLayout of five SwipeViews, each demonstrating a different revealed-side / SwipeMode c

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 12. Behaviors — 🟢/🟢
<sub>behaviors</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/behaviors_light.png" /></td><td><img width="300px" src="captures/android/cpp/behaviors_light.png" /></td><td><img width="300px" src="captures/android/xaml/behaviors_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/behaviors_dark.png" /></td><td><img width="300px" src="captures/android/cpp/behaviors_dark.png" /></td><td><img width="300px" src="captures/android/xaml/behaviors_dark.png" /></td></tr></table>

ports BehaviorsPage.xaml (+ .xaml.cs) and its companion Controls.Sample/Behaviors/NumericValidationBehavior.cs

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 13. Border — 🟡/🟡
<sub>border</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/border_light.png" /></td><td><img width="300px" src="captures/android/cpp/border_light.png" /></td><td><img width="300px" src="captures/android/xaml/border_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/border_dark.png" /></td><td><img width="300px" src="captures/android/cpp/border_dark.png" /></td><td><img width="300px" src="captures/android/xaml/border_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;border&amp;quot; demo (ComparePages.BorderPage()), the shipped-.NET-MAUI reference for the visual-parity comparison: a single Border centered on the page — red 5pt stroke, a RoundRectangle StrokeShape (Co

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9592, 3.55% pixels differ · Dark: SSIM 0.9626, 3.31% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9592, 3.55% pixels differ · Dark: SSIM 0.9626, 3.31% pixels differ

### 14. Border Clip Playground — 🟡/🟡
<sub>border_clip_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/border_clip_playground_light.png" /></td><td><img width="300px" src="captures/android/cpp/border_clip_playground_light.png" /></td><td><img width="300px" src="captures/android/xaml/border_clip_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/border_clip_playground_dark.png" /></td><td><img width="300px" src="captures/android/cpp/border_clip_playground_dark.png" /></td><td><img width="300px" src="captures/android/xaml/border_clip_playground_dark.png" /></td></tr></table>

ports BorderClipPlayground.xaml (+ .xaml.cs) The C# page is an interactive Border-shape playground: a 100x100 Border (red stroke) clips an AspectFill Image (oasis.jpg) into the currently selected StrokeShape, while controls below mutate the

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9541, 2.89% pixels differ · Dark: SSIM 0.9675, 2.02% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9588, 2.31% pixels differ · Dark: SSIM 0.9723, 1.43% pixels differ

### 15. Border Layout — 🟢/🟢
<sub>border_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/border_layout_light.png" /></td><td><img width="300px" src="captures/android/cpp/border_layout_light.png" /></td><td><img width="300px" src="captures/android/xaml/border_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/border_layout_dark.png" /></td><td><img width="300px" src="captures/android/cpp/border_layout_dark.png" /></td><td><img width="300px" src="captures/android/xaml/border_layout_dark.png" /></td></tr></table>

ports BorderLayout.xaml (+ BorderLayout.xaml.cs) The C# page demonstrates driving Border.StrokeThickness from a Slider: a Slider (0..40, set to 5 in OnAppearing) is bound to the Border&amp;#x27;s StrokeThickness; the Border (Silver stroke, White bac

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9948, 0.20% pixels differ · Dark: SSIM 0.9941, 0.28% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9948, 0.20% pixels differ · Dark: SSIM 0.9941, 0.28% pixels differ

### 16. Border Playground — 🟡/🟡
<sub>border_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/border_playground_light.png" /></td><td><img width="300px" src="captures/android/cpp/border_playground_light.png" /></td><td><img width="300px" src="captures/android/xaml/border_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/border_playground_dark.png" /></td><td><img width="300px" src="captures/android/cpp/border_playground_dark.png" /></td><td><img width="300px" src="captures/android/xaml/border_playground_dark.png" /></td></tr></table>

ports BorderPlayground.xaml (+ BorderPlayground.xaml.cs) A self-contained, code-first interactive Border playground

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9660, 2.45% pixels differ · Dark: SSIM 0.9641, 1.73% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9657, 2.20% pixels differ · Dark: SSIM 0.9646, 1.45% pixels differ

### 17. Border Resize Content — 🟡/🟡
<sub>border_resize_content</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/border_resize_content_light.png" /></td><td><img width="300px" src="captures/android/cpp/border_resize_content_light.png" /></td><td><img width="300px" src="captures/android/xaml/border_resize_content_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/border_resize_content_dark.png" /></td><td><img width="300px" src="captures/android/cpp/border_resize_content_dark.png" /></td><td><img width="300px" src="captures/android/xaml/border_resize_content_dark.png" /></td></tr></table>

ports BorderResizeContent.xaml A self-contained, code-first demo that resizes a Border&amp;#x27;s CONTENT and watches the Border track it

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9762, 1.75% pixels differ · Dark: SSIM 0.9731, 1.81% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9789, 3.32% pixels differ · Dark: SSIM 0.9595, 3.37% pixels differ

### 18. Border Stroke — 🟡/🟡
<sub>border_stroke</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/border_stroke_light.png" /></td><td><img width="300px" src="captures/android/cpp/border_stroke_light.png" /></td><td><img width="300px" src="captures/android/xaml/border_stroke_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/border_stroke_dark.png" /></td><td><img width="300px" src="captures/android/cpp/border_stroke_dark.png" /></td><td><img width="300px" src="captures/android/xaml/border_stroke_dark.png" /></td></tr></table>

ports BorderStroke.xaml (+ BorderStroke.xaml.cs) A self-contained, code-first demo of Border StrokeThickness and how a Border tracks the height of its content

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9235, 3.37% pixels differ · Dark: SSIM 0.9351, 3.43% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9235, 3.37% pixels differ · Dark: SSIM 0.9351, 3.43% pixels differ

### 19. Borderless — 🟢/🟢
<sub>borderless</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/borderless_light.png" /></td><td><img width="300px" src="captures/android/cpp/borderless_light.png" /></td><td><img width="300px" src="captures/android/xaml/borderless_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/borderless_dark.png" /></td><td><img width="300px" src="captures/android/cpp/borderless_dark.png" /></td><td><img width="300px" src="captures/android/xaml/borderless_dark.png" /></td></tr></table>

ports Borderless.xaml A self-contained, code-first demo of a stroke-less Border

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 20. Box View — 🟢/🟢
<sub>box_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/box_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/box_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/box_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/box_view_dark.png" /></td><td><img width="300px" src="captures/android/cpp/box_view_dark.png" /></td><td><img width="300px" src="captures/android/xaml/box_view_dark.png" /></td></tr></table>

ports BoxViewPage.xaml (+ BoxViewPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9957, 0.53% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9957, 0.53% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 21. Button — 🟡/🟡
<sub>button</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/button_light.png" /></td><td><img width="300px" src="captures/android/cpp/button_light.png" /></td><td><img width="300px" src="captures/android/xaml/button_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/button_dark.png" /></td><td><img width="300px" src="captures/android/cpp/button_dark.png" /></td><td><img width="300px" src="captures/android/xaml/button_dark.png" /></td></tr></table>

ports ButtonPage.xaml (+ ButtonPage.xaml.cs)

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9394, 3.73% pixels differ · Dark: SSIM 0.9441, 1.33% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9394, 3.73% pixels differ · Dark: SSIM 0.9441, 1.33% pixels differ

### 22. Carousel Page — 🟢/🟢
<sub>carousel_page</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/carousel_page_light.png" /></td><td><img width="300px" src="captures/android/cpp/carousel_page_light.png" /></td><td><img width="300px" src="captures/android/xaml/carousel_page_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/carousel_page_dark.png" /></td><td><img width="300px" src="captures/android/cpp/carousel_page_dark.png" /></td><td><img width="300px" src="captures/android/xaml/carousel_page_dark.png" /></td></tr></table>

Carousel Page

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9845, 0.72% pixels differ · Dark: SSIM 0.9892, 0.72% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9845, 0.72% pixels differ · Dark: SSIM 0.9892, 0.72% pixels differ

### 23. Chat Example — 🟢/🟢
<sub>chat_example</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/chat_example_light.png" /></td><td><img width="300px" src="captures/android/cpp/chat_example_light.png" /></td><td><img width="300px" src="captures/android/xaml/chat_example_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/chat_example_dark.png" /></td><td><img width="300px" src="captures/android/cpp/chat_example_dark.png" /></td><td><img width="300px" src="captures/android/xaml/chat_example_dark.png" /></td></tr></table>

ports ChatExample.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.ItemSizeGalleries.ChatExample), tracking the maui-compare reference demo ~/maui-compare/Pages/ChatExamplePage.cs (the visual-parity oracle)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 24. Check Box — 🟢/🟢
<sub>check_box</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/check_box_light.png" /></td><td><img width="300px" src="captures/android/cpp/check_box_light.png" /></td><td><img width="300px" src="captures/android/xaml/check_box_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/check_box_dark.png" /></td><td><img width="300px" src="captures/android/cpp/check_box_dark.png" /></td><td><img width="300px" src="captures/android/xaml/check_box_dark.png" /></td></tr></table>

ports CheckBoxPage.xaml (+ .xaml.cs) Mirrors the MAUI gallery page: a vertical stack of headlined CheckBox states — Default, Colored (Color=Purple), Disabled, Disabled+Colored+Checked — followed by a &amp;quot;Change IsChecked&amp;quot; row pairing a Button

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9984, 0.09% pixels differ · Dark: SSIM 0.9986, 0.09% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 25. Chrome — 🟢/🟢
<sub>chrome</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/chrome_light.png" /></td><td><img width="300px" src="captures/android/cpp/chrome_light.png" /></td><td><img width="300px" src="captures/android/xaml/chrome_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/chrome_dark.png" /></td><td><img width="300px" src="captures/android/cpp/chrome_dark.png" /></td><td><img width="300px" src="captures/android/xaml/chrome_dark.png" /></td></tr></table>

a self-contained demo page for the W1-11 window-chrome family: page toolbar items (primary + secondary), a menu bar (File menu with items, a separator and a sub-menu), a context flyout (right-click menu) on a button, and a tooltip — all wir

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 26. Clip — 🟡/🟡
<sub>clip</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/clip_light.png" /></td><td><img width="300px" src="captures/android/cpp/clip_light.png" /></td><td><img width="300px" src="captures/android/xaml/clip_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/clip_dark.png" /></td><td><img width="300px" src="captures/android/cpp/clip_dark.png" /></td><td><img width="300px" src="captures/android/xaml/clip_dark.png" /></td></tr></table>

ports ClipPage.xaml The C# page (Pages/Core/ClipPage.xaml; its .xaml.cs is an empty InitializeComponent) is a ScrollView over a StackLayout that shows the SAME dotnet_bot.png image five times, each successive copy carrying a different geome

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9879, 1.01% pixels differ · Dark: SSIM 0.9883, 0.31% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9879, 1.01% pixels differ · Dark: SSIM 0.9883, 0.31% pixels differ

### 27. Clip Corner Radius — 🟢/🟢
<sub>clip_corner_radius</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/clip_corner_radius_light.png" /></td><td><img width="300px" src="captures/android/cpp/clip_corner_radius_light.png" /></td><td><img width="300px" src="captures/android/xaml/clip_corner_radius_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/clip_corner_radius_dark.png" /></td><td><img width="300px" src="captures/android/cpp/clip_corner_radius_dark.png" /></td><td><img width="300px" src="captures/android/xaml/clip_corner_radius_dark.png" /></td></tr></table>

ports ClipCornerRadiusGallery.xaml (+ .xaml.cs) The C# page (Pages/Controls/ShapesGalleries/ClipCornerRadiusGallery.xaml) is a StackLayout (Padding=12) that demonstrates DRIVING a RoundRectangleGeometry&amp;#x27;s per-corner CornerRadius from four s

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9999, 0.00% pixels differ · Dark: SSIM 0.9997, 0.02% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9998, 0.00% pixels differ · Dark: SSIM 0.9997, 0.02% pixels differ

### 28. Clip Gallery — 🟢/🟢
<sub>clip_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/clip_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/clip_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/clip_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/clip_gallery_dark.png" /></td><td><img width="300px" src="captures/android/cpp/clip_gallery_dark.png" /></td><td><img width="300px" src="captures/android/xaml/clip_gallery_dark.png" /></td></tr></table>

ports ClipGallery.xaml The C# page (Pages/Controls/ShapesGalleries/ClipGallery.xaml; its .xaml.cs is an empty InitializeComponent) is a ScrollView over a StackLayout (Padding=12) that shows the SAME &amp;quot;oasis.jpg&amp;quot; image SEVEN times — one bare

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9954, 0.58% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9954, 0.58% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 29. Clip Views — 🟡/🟡
<sub>clip_views</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/clip_views_light.png" /></td><td><img width="300px" src="captures/android/cpp/clip_views_light.png" /></td><td><img width="300px" src="captures/android/xaml/clip_views_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/clip_views_dark.png" /></td><td><img width="300px" src="captures/android/cpp/clip_views_dark.png" /></td><td><img width="300px" src="captures/android/xaml/clip_views_dark.png" /></td></tr></table>

ports ClipViewsGallery.xaml The C# page (Pages/Controls/ShapesGalleries/ClipViewsGallery.xaml; no code-behind beyond an empty InitializeComponent) is a ScrollView over a StackLayout (Padding=12) that proves the Clip surface (VisualElement.C

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9694, 4.31% pixels differ · Dark: SSIM 0.9754, 4.27% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9694, 4.31% pixels differ · Dark: SSIM 0.9754, 4.27% pixels differ

### 30. Clipping — 🟢/🟢
<sub>clipping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/clipping_light.png" /></td><td><img width="300px" src="captures/android/cpp/clipping_light.png" /></td><td><img width="300px" src="captures/android/xaml/clipping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/clipping_dark.png" /></td><td><img width="300px" src="captures/android/cpp/clipping_dark.png" /></td><td><img width="300px" src="captures/android/xaml/clipping_dark.png" /></td></tr></table>

compare oracle ~/maui-compare/Pages/ClippingPage.cs (itself written to mirror this gallery page)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9877, 0.54% pixels differ · Dark: SSIM 0.9884, 0.56% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9877, 0.54% pixels differ · Dark: SSIM 0.9884, 0.56% pixels differ

### 31. Collectionview — 🟢/🟢
<sub>collectionview</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/collectionview_light.png" /></td><td><img width="300px" src="captures/android/cpp/collectionview_light.png" /></td><td><img width="300px" src="captures/android/xaml/collectionview_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/collectionview_dark.png" /></td><td><img width="300px" src="captures/android/cpp/collectionview_dark.png" /></td><td><img width="300px" src="captures/android/xaml/collectionview_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;collectionview&amp;quot; demo (ComparePages.CollectionViewPage()), the shipped-.NET-MAUI reference for the visual-parity comparison: a CollectionView over 24 captioned items, a string Header (&amp;quot;This is the

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9999, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9999, 0.00% pixels differ

### 32. Composition Gallery — 🟢/🟢
<sub>composition_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/composition_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/composition_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/composition_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/composition_gallery_dark.png" /></td><td><img width="300px" src="captures/android/cpp/composition_gallery_dark.png" /></td><td><img width="300px" src="captures/android/xaml/composition_gallery_dark.png" /></td></tr></table>

ports CompositionGallery.xaml A self-contained, code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/CompositionGallery.xaml: a StackLayout holding two Beige 250x250 Grids (Margin 12) that compose multiple overlappi

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 33. Containers — 🟢/🟢
<sub>containers</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/containers_light.png" /></td><td><img width="300px" src="captures/android/cpp/containers_light.png" /></td><td><img width="300px" src="captures/android/xaml/containers_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/containers_dark.png" /></td><td><img width="300px" src="captures/android/cpp/containers_dark.png" /></td><td><img width="300px" src="captures/android/xaml/containers_dark.png" /></td></tr></table>

a self-contained demo page for the W1-07 container set: a scroll_view hosting a vertical stack of content-hosting containers — a border-framed label (stroke + dashed outline + rounded shape), a legacy frame (BorderColor/CornerRadius/HasShad

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9882, 0.46% pixels differ · Dark: SSIM 0.9895, 0.41% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9882, 0.46% pixels differ · Dark: SSIM 0.9895, 0.41% pixels differ

### 34. Content View — 🟢/🟢
<sub>content_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/content_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/content_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/content_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/content_view_dark.png" /></td><td><img width="300px" src="captures/android/cpp/content_view_dark.png" /></td><td><img width="300px" src="captures/android/xaml/content_view_dark.png" /></td></tr></table>

ports ContentViewPage.xaml (+ ContentViewPage.xaml.cs), code-first

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 35. Context Flyout — 🔴/🔴
<sub>context_flyout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/context_flyout_light.png" /></td><td><img width="300px" src="captures/android/cpp/context_flyout_light.png" /></td><td><img width="300px" src="captures/android/xaml/context_flyout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/context_flyout_dark.png" /></td><td><img width="300px" src="captures/android/cpp/context_flyout_dark.png" /></td><td><img width="300px" src="captures/android/xaml/context_flyout_dark.png" /></td></tr></table>

ports ContextFlyoutPage.xaml (+ ContextFlyoutPage.xaml.cs) The C# page attaches a MenuFlyout as the FlyoutBase.ContextFlyout (right-click / long-press menu) of several controls and wires each menu item to a handler: - a Button (&amp;quot;Increment b

#### 🔴 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.8823, 8.55% pixels differ · Dark: SSIM 0.1808, 95.79% pixels differ

#### 🔴 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.8823, 8.55% pixels differ · Dark: SSIM 0.1808, 95.79% pixels differ

### 36. Controls Stack — 🟢/🟢
<sub>controls_stack</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/controls_stack_light.png" /></td><td><img width="300px" src="captures/android/cpp/controls_stack_light.png" /></td><td><img width="300px" src="captures/android/xaml/controls_stack_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/controls_stack_dark.png" /></td><td><img width="300px" src="captures/android/cpp/controls_stack_dark.png" /></td><td><img width="300px" src="captures/android/xaml/controls_stack_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;controls_stack&amp;quot; demo (ComparePages.ControlsStack()), the shipped-.NET-MAUI reference for the visual-parity comparison: a VerticalStackLayout (Spacing 12, Padding 16) showcasing the basic widgets

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9856, 0.43% pixels differ · Dark: SSIM 0.9837, 0.53% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9856, 0.43% pixels differ · Dark: SSIM 0.9837, 0.53% pixels differ

### 37. Custom Layout — 🟢/🟢
<sub>custom_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/custom_layout_light.png" /></td><td><img width="300px" src="captures/android/cpp/custom_layout_light.png" /></td><td><img width="300px" src="captures/android/xaml/custom_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/custom_layout_dark.png" /></td><td><img width="300px" src="captures/android/cpp/custom_layout_dark.png" /></td><td><img width="300px" src="captures/android/xaml/custom_layout_dark.png" /></td></tr></table>

ports CustomLayoutPage.xaml (+ CustomLayoutPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 38. Custom Size Swipe — 🟢/🟢
<sub>custom_size_swipe</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/custom_size_swipe_light.png" /></td><td><img width="300px" src="captures/android/cpp/custom_size_swipe_light.png" /></td><td><img width="300px" src="captures/android/xaml/custom_size_swipe_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/custom_size_swipe_dark.png" /></td><td><img width="300px" src="captures/android/cpp/custom_size_swipe_dark.png" /></td><td><img width="300px" src="captures/android/xaml/custom_size_swipe_dark.png" /></td></tr></table>

ports CustomSizeSwipeViewGallery.xaml (+ .xaml.cs) The MAUI CustomSizeSwipeViewGallery is a single SwipeView whose Left / Right / Top item collections each reveal CUSTOM-SIZED content: a SwipeItemView wrapping a Grid/StackLayout with an exp

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 39. Custom Swipe Item View — 🟢/🟢
<sub>custom_swipe_item_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/custom_swipe_item_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/custom_swipe_item_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/custom_swipe_item_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/custom_swipe_item_view_dark.png" /></td><td><img width="300px" src="captures/android/cpp/custom_swipe_item_view_dark.png" /></td><td><img width="300px" src="captures/android/xaml/custom_swipe_item_view_dark.png" /></td></tr></table>

ports CustomSwipeItemViewGallery.xaml A self-contained, code-first port of the .NET MAUI &amp;quot;CustomSwipeItem&amp;quot; gallery: a message-list row whose right swipe reveals a CUSTOM-content swipe item (a swipe_item_view, not a plain swipe_item)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 40. Cv Visual States — 🟢/🟢
<sub>cv_visual_states</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/cv_visual_states_light.png" /></td><td><img width="300px" src="captures/android/cpp/cv_visual_states_light.png" /></td><td><img width="300px" src="captures/android/xaml/cv_visual_states_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/cv_visual_states_dark.png" /></td><td><img width="300px" src="captures/android/cpp/cv_visual_states_dark.png" /></td><td><img width="300px" src="captures/android/xaml/cv_visual_states_dark.png" /></td></tr></table>

ports CollectionViewGalleries/SelectionGalleries/ VisualStatesGallery.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9834, 0.70% pixels differ · Dark: SSIM 0.9809, 0.69% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9834, 0.70% pixels differ · Dark: SSIM 0.9809, 0.69% pixels differ

### 41. Data Template Selector — 🟢/🟢
<sub>data_template_selector</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/data_template_selector_light.png" /></td><td><img width="300px" src="captures/android/cpp/data_template_selector_light.png" /></td><td><img width="300px" src="captures/android/xaml/data_template_selector_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/data_template_selector_dark.png" /></td><td><img width="300px" src="captures/android/cpp/data_template_selector_dark.png" /></td><td><img width="300px" src="captures/android/xaml/data_template_selector_dark.png" /></td></tr></table>

ports DataTemplateSelectorGallery.xaml (+ DataTemplateSelectorGallery.xaml.cs, including its WeekendSelector + SearchTermSelector classes)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9865, 0.37% pixels differ · Dark: SSIM 0.9854, 0.41% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9865, 0.37% pixels differ · Dark: SSIM 0.9854, 0.41% pixels differ

### 42. Date Picker — 🟡/🟡
<sub>date_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/date_picker_light.png" /></td><td><img width="300px" src="captures/android/cpp/date_picker_light.png" /></td><td><img width="300px" src="captures/android/xaml/date_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/date_picker_dark.png" /></td><td><img width="300px" src="captures/android/cpp/date_picker_dark.png" /></td><td><img width="300px" src="captures/android/xaml/date_picker_dark.png" /></td></tr></table>

ports DatePickerPage.xaml (+ DatePickerPage.xaml.cs)

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9848, 1.61% pixels differ · Dark: SSIM 0.9697, 0.75% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9848, 1.61% pixels differ · Dark: SSIM 0.9697, 0.75% pixels differ

### 43. Device — 🟢/🟢
<sub>device</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/device_light.png" /></td><td><img width="300px" src="captures/android/cpp/device_light.png" /></td><td><img width="300px" src="captures/android/xaml/device_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/device_dark.png" /></td><td><img width="300px" src="captures/android/cpp/device_dark.png" /></td><td><img width="300px" src="captures/android/xaml/device_dark.png" /></td></tr></table>

ports DevicePage.xaml

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9884, 0.43% pixels differ · Dark: SSIM 0.9881, 0.47% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 44. Dispatcher — 🟢/🟢
<sub>dispatcher</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/dispatcher_light.png" /></td><td><img width="300px" src="captures/android/cpp/dispatcher_light.png" /></td><td><img width="300px" src="captures/android/xaml/dispatcher_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/dispatcher_dark.png" /></td><td><img width="300px" src="captures/android/cpp/dispatcher_dark.png" /></td><td><img width="300px" src="captures/android/xaml/dispatcher_dark.png" /></td></tr></table>

ports DispatcherPage.xaml (+ .xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9999, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9999, 0.00% pixels differ

### 45. Drag Drop — 🟢/🟢
<sub>drag_drop</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/drag_drop_light.png" /></td><td><img width="300px" src="captures/android/cpp/drag_drop_light.png" /></td><td><img width="300px" src="captures/android/xaml/drag_drop_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/drag_drop_dark.png" /></td><td><img width="300px" src="captures/android/cpp/drag_drop_dark.png" /></td><td><img width="300px" src="captures/android/xaml/drag_drop_dark.png" /></td></tr></table>

ports DragAndDropBetweenLayouts.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.DragAndDropBetweenLayouts)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 46. Editor — 🟢/🟢
<sub>editor</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/editor_light.png" /></td><td><img width="300px" src="captures/android/cpp/editor_light.png" /></td><td><img width="300px" src="captures/android/xaml/editor_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/editor_dark.png" /></td><td><img width="300px" src="captures/android/cpp/editor_dark.png" /></td><td><img width="300px" src="captures/android/xaml/editor_dark.png" /></td></tr></table>

ports EditorPage.xaml (Microsoft.Maui.Controls sample gallery)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9997, 0.47% pixels differ · Dark: SSIM 0.9981, 0.21% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9997, 0.47% pixels differ · Dark: SSIM 0.9981, 0.21% pixels differ

### 47. Effects — 🟢/🟢
<sub>effects</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/effects_light.png" /></td><td><img width="300px" src="captures/android/cpp/effects_light.png" /></td><td><img width="300px" src="captures/android/xaml/effects_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/effects_dark.png" /></td><td><img width="300px" src="captures/android/cpp/effects_dark.png" /></td><td><img width="300px" src="captures/android/xaml/effects_dark.png" /></td></tr></table>

ports EffectsPage.xaml (Maui.Controls.Sample.Pages.EffectsPage)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9999, 0.12% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9999, 0.12% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 48. Ellipse Gallery — 🟢/🟢
<sub>ellipse_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ellipse_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/ellipse_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/ellipse_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/ellipse_gallery_dark.png" /></td><td><img width="300px" src="captures/android/cpp/ellipse_gallery_dark.png" /></td><td><img width="300px" src="captures/android/xaml/ellipse_gallery_dark.png" /></td></tr></table>

ports EllipseGallery.xaml A self-contained, code-first port of the MAUI Shapes EllipseGallery (Pages/Controls/ShapesGalleries/EllipseGallery.xaml + .xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9877, 0.62% pixels differ · Dark: SSIM 0.9895, 0.63% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9877, 0.62% pixels differ · Dark: SSIM 0.9895, 0.63% pixels differ

### 49. Empty View — 🟢/🟢
<sub>empty_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/empty_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/empty_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/empty_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/empty_view_dark.png" /></td><td><img width="300px" src="captures/android/cpp/empty_view_dark.png" /></td><td><img width="300px" src="captures/android/xaml/empty_view_dark.png" /></td></tr></table>

ports EmptyViewStringGallery.xaml (+ EmptyViewStringGallery.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9890, 0.25% pixels differ · Dark: SSIM 0.9879, 0.29% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9890, 0.25% pixels differ · Dark: SSIM 0.9879, 0.29% pixels differ

### 50. Empty View Load Simulate — 🟢/🟢
<sub>empty_view_load_simulate</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/empty_view_load_simulate_light.png" /></td><td><img width="300px" src="captures/android/cpp/empty_view_load_simulate_light.png" /></td><td><img width="300px" src="captures/android/xaml/empty_view_load_simulate_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/empty_view_load_simulate_dark.png" /></td><td><img width="300px" src="captures/android/cpp/empty_view_load_simulate_dark.png" /></td><td><img width="300px" src="captures/android/xaml/empty_view_load_simulate_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewLoadSimulateGallery.xaml (+ EmptyViewLoadSimulateGallery.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9954, 0.20% pixels differ · Dark: SSIM 0.9954, 0.22% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9954, 0.20% pixels differ · Dark: SSIM 0.9954, 0.22% pixels differ

### 51. Empty View Null — 🟢/🟢
<sub>empty_view_null</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/empty_view_null_light.png" /></td><td><img width="300px" src="captures/android/cpp/empty_view_null_light.png" /></td><td><img width="300px" src="captures/android/xaml/empty_view_null_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/empty_view_null_dark.png" /></td><td><img width="300px" src="captures/android/cpp/empty_view_null_dark.png" /></td><td><img width="300px" src="captures/android/xaml/empty_view_null_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewNullGallery.xaml (+ EmptyViewNullGallery.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9960, 0.18% pixels differ · Dark: SSIM 0.9960, 0.19% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9960, 0.18% pixels differ · Dark: SSIM 0.9960, 0.19% pixels differ

### 52. Empty View Rtl — 🟢/🟢
<sub>empty_view_rtl</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/empty_view_rtl_light.png" /></td><td><img width="300px" src="captures/android/cpp/empty_view_rtl_light.png" /></td><td><img width="300px" src="captures/android/xaml/empty_view_rtl_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/empty_view_rtl_dark.png" /></td><td><img width="300px" src="captures/android/cpp/empty_view_rtl_dark.png" /></td><td><img width="300px" src="captures/android/xaml/empty_view_rtl_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewRTLGallery.xaml (+ EmptyViewRTLGallery.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9859, 0.48% pixels differ · Dark: SSIM 0.9848, 0.44% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9889, 0.34% pixels differ · Dark: SSIM 0.9879, 0.29% pixels differ

### 53. Empty View Selector — 🟢/🟢
<sub>empty_view_selector</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/empty_view_selector_light.png" /></td><td><img width="300px" src="captures/android/cpp/empty_view_selector_light.png" /></td><td><img width="300px" src="captures/android/xaml/empty_view_selector_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/empty_view_selector_dark.png" /></td><td><img width="300px" src="captures/android/cpp/empty_view_selector_dark.png" /></td><td><img width="300px" src="captures/android/xaml/empty_view_selector_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewWithDataTemplateSelector.xaml (+ .xaml.cs, incl

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9895, 0.24% pixels differ · Dark: SSIM 0.9884, 0.28% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9895, 0.24% pixels differ · Dark: SSIM 0.9885, 0.28% pixels differ

### 54. Empty View Swap — 🟡/🟢
<sub>empty_view_swap</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/empty_view_swap_light.png" /></td><td><img width="300px" src="captures/android/cpp/empty_view_swap_light.png" /></td><td><img width="300px" src="captures/android/xaml/empty_view_swap_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/empty_view_swap_dark.png" /></td><td><img width="300px" src="captures/android/cpp/empty_view_swap_dark.png" /></td><td><img width="300px" src="captures/android/xaml/empty_view_swap_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewSwapGallery.xaml (+ .xaml.cs)

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9754, 0.71% pixels differ · Dark: SSIM 0.9738, 0.90% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9887, 0.30% pixels differ · Dark: SSIM 0.9875, 0.44% pixels differ

### 55. Empty View Template — 🟢/🟢
<sub>empty_view_template</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/empty_view_template_light.png" /></td><td><img width="300px" src="captures/android/cpp/empty_view_template_light.png" /></td><td><img width="300px" src="captures/android/xaml/empty_view_template_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/empty_view_template_dark.png" /></td><td><img width="300px" src="captures/android/cpp/empty_view_template_dark.png" /></td><td><img width="300px" src="captures/android/xaml/empty_view_template_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewTemplateGallery.xaml (+ EmptyViewTemplateGallery.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9895, 0.24% pixels differ · Dark: SSIM 0.9885, 0.28% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9895, 0.24% pixels differ · Dark: SSIM 0.9885, 0.28% pixels differ

### 56. Empty View View — 🟢/🟢
<sub>empty_view_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/empty_view_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/empty_view_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/empty_view_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/empty_view_view_dark.png" /></td><td><img width="300px" src="captures/android/cpp/empty_view_view_dark.png" /></td><td><img width="300px" src="captures/android/xaml/empty_view_view_dark.png" /></td></tr></table>

ports EmptyViewGalleries/EmptyViewViewGallery.xaml (+ EmptyViewViewGallery.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9895, 0.24% pixels differ · Dark: SSIM 0.9885, 0.28% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9895, 0.24% pixels differ · Dark: SSIM 0.9885, 0.28% pixels differ

### 57. Entry — 🟢/🟢
<sub>entry</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/entry_light.png" /></td><td><img width="300px" src="captures/android/cpp/entry_light.png" /></td><td><img width="300px" src="captures/android/xaml/entry_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/entry_dark.png" /></td><td><img width="300px" src="captures/android/cpp/entry_dark.png" /></td><td><img width="300px" src="captures/android/xaml/entry_dark.png" /></td></tr></table>

ports EntryPage.xaml (Microsoft.Maui.Controls sample gallery)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9872, 0.50% pixels differ · Dark: SSIM 0.9871, 0.42% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9872, 0.50% pixels differ · Dark: SSIM 0.9871, 0.42% pixels differ

### 58. Filter Collection — 🟢/🟢
<sub>filter_collection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/filter_collection_light.png" /></td><td><img width="300px" src="captures/android/cpp/filter_collection_light.png" /></td><td><img width="300px" src="captures/android/xaml/filter_collection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/filter_collection_dark.png" /></td><td><img width="300px" src="captures/android/cpp/filter_collection_dark.png" /></td><td><img width="300px" src="captures/android/xaml/filter_collection_dark.png" /></td></tr></table>

ports FilterCollectionView.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9816, 0.47% pixels differ · Dark: SSIM 0.9800, 0.55% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9889, 0.25% pixels differ · Dark: SSIM 0.9875, 0.31% pixels differ

### 59. Filter Selection — 🟢/🟢
<sub>filter_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/filter_selection_light.png" /></td><td><img width="300px" src="captures/android/cpp/filter_selection_light.png" /></td><td><img width="300px" src="captures/android/xaml/filter_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/filter_selection_dark.png" /></td><td><img width="300px" src="captures/android/cpp/filter_selection_dark.png" /></td><td><img width="300px" src="captures/android/xaml/filter_selection_dark.png" /></td></tr></table>

ports FilterSelection.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.FilterSelection)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9895, 0.24% pixels differ · Dark: SSIM 0.9884, 0.28% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9895, 0.24% pixels differ · Dark: SSIM 0.9884, 0.28% pixels differ

### 60. Flex Layout — 🟢/🟢
<sub>flex_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/flex_layout_light.png" /></td><td><img width="300px" src="captures/android/cpp/flex_layout_light.png" /></td><td><img width="300px" src="captures/android/xaml/flex_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/flex_layout_dark.png" /></td><td><img width="300px" src="captures/android/cpp/flex_layout_dark.png" /></td><td><img width="300px" src="captures/android/xaml/flex_layout_dark.png" /></td></tr></table>

ports FlexLayoutPage.xaml A self-contained, code-first demo of the FlexLayout control: the classic &amp;quot;holy grail&amp;quot; page layout built from nested flexboxes

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 61. Focus — 🟢/🟢
<sub>focus</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/focus_light.png" /></td><td><img width="300px" src="captures/android/cpp/focus_light.png" /></td><td><img width="300px" src="captures/android/xaml/focus_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/focus_dark.png" /></td><td><img width="300px" src="captures/android/cpp/focus_dark.png" /></td><td><img width="300px" src="captures/android/xaml/focus_dark.png" /></td></tr></table>

ports FocusPage.xaml (+ FocusPage.xaml.cs) The C# FocusPage is a focus-subsystem demo: an Entry whose Focused/Unfocused events (OnFocusEntryFocusChanged) append &amp;quot;Focused&amp;quot;/&amp;quot;Unfocused&amp;quot; lines to a scrolling InfoLabel, plus two buttons — &amp;quot;Focus

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 62. Fonts — 🟢/🟢
<sub>fonts</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/fonts_light.png" /></td><td><img width="300px" src="captures/android/cpp/fonts_light.png" /></td><td><img width="300px" src="captures/android/xaml/fonts_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/fonts_dark.png" /></td><td><img width="300px" src="captures/android/cpp/fonts_dark.png" /></td><td><img width="300px" src="captures/android/xaml/fonts_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;fonts&amp;quot; demo (ComparePages.Fonts()), the shipped-.NET-MAUI reference for the visual-parity comparison: a ScrollView over a VerticalStackLayout (Spacing 8, Padding 16) of nine Labels — Title/Subtit

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 63. Footer Only String — 🟢/🟢
<sub>footer_only_string</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/footer_only_string_light.png" /></td><td><img width="300px" src="captures/android/cpp/footer_only_string_light.png" /></td><td><img width="300px" src="captures/android/xaml/footer_only_string_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/footer_only_string_dark.png" /></td><td><img width="300px" src="captures/android/cpp/footer_only_string_dark.png" /></td><td><img width="300px" src="captures/android/xaml/footer_only_string_dark.png" /></td></tr></table>

ports FooterOnlyString.xaml (+ FooterOnlyString.xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9999, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9999, 0.00% pixels differ

### 64. Formatted Text — 🟢/🟢
<sub>formatted_text</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/formatted_text_light.png" /></td><td><img width="300px" src="captures/android/cpp/formatted_text_light.png" /></td><td><img width="300px" src="captures/android/xaml/formatted_text_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/formatted_text_dark.png" /></td><td><img width="300px" src="captures/android/cpp/formatted_text_dark.png" /></td><td><img width="300px" src="captures/android/xaml/formatted_text_dark.png" /></td></tr></table>

a self-contained demo page for the G1 rich-text slice: a label whose FormattedText is built from several styled spans (bold / italic / colored / underlined / kerned), plus a plain label proving the Text ⇄ FormattedText exclusivity

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9966, 0.11% pixels differ · Dark: SSIM 0.9966, 0.12% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9966, 0.11% pixels differ · Dark: SSIM 0.9966, 0.12% pixels differ

### 65. Gestures — 🟢/🟢
<sub>gestures</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/gestures_light.png" /></td><td><img width="300px" src="captures/android/cpp/gestures_light.png" /></td><td><img width="300px" src="captures/android/xaml/gestures_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/gestures_dark.png" /></td><td><img width="300px" src="captures/android/cpp/gestures_dark.png" /></td><td><img width="300px" src="captures/android/xaml/gestures_dark.png" /></td></tr></table>

ports GesturesPage.xaml (+ .xaml.cs) The MAUI GesturesPage.xaml is a *gallery navigation* page: a CollectionView listing gesture-demo sections that the shell navigates into

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 66. Gradient — 🟢/🟢
<sub>gradient</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/gradient_light.png" /></td><td><img width="300px" src="captures/android/cpp/gradient_light.png" /></td><td><img width="300px" src="captures/android/xaml/gradient_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/gradient_dark.png" /></td><td><img width="300px" src="captures/android/cpp/gradient_dark.png" /></td><td><img width="300px" src="captures/android/xaml/gradient_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;gradient&amp;quot; demo (ComparePages.Gradient()), the shipped-.NET-MAUI reference for the visual-parity comparison: a VerticalStackLayout (Spacing 12, Padding 16) of two captioned 60px BoxViews — a Linea

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 67. Grid — 🟢/🟢
<sub>grid</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/grid_light.png" /></td><td><img width="300px" src="captures/android/cpp/grid_light.png" /></td><td><img width="300px" src="captures/android/xaml/grid_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/grid_dark.png" /></td><td><img width="300px" src="captures/android/cpp/grid_dark.png" /></td><td><img width="300px" src="captures/android/xaml/grid_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;grid&amp;quot; demo (ComparePages.GridPage()), the shipped-.NET-MAUI reference for the visual-parity comparison: a Grid (Padding 16, Row/ColumnSpacing 6) with RowDefinitions Auto / 80 / 80 and two Star co

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9993, 0.02% pixels differ · Dark: SSIM 0.9993, 0.02% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9993, 0.02% pixels differ · Dark: SSIM 0.9993, 0.02% pixels differ

### 68. Grid Grouping — 🟢/🟢
<sub>grid_grouping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/grid_grouping_light.png" /></td><td><img width="300px" src="captures/android/cpp/grid_grouping_light.png" /></td><td><img width="300px" src="captures/android/xaml/grid_grouping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/grid_grouping_dark.png" /></td><td><img width="300px" src="captures/android/cpp/grid_grouping_dark.png" /></td><td><img width="300px" src="captures/android/xaml/grid_grouping_dark.png" /></td></tr></table>

ports GroupingGalleries/GridGrouping.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9895, 0.27% pixels differ · Dark: SSIM 0.9886, 0.27% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9999, 0.00% pixels differ

### 69. Grouping No Templates — 🟢/🟢
<sub>grouping_no_templates</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/grouping_no_templates_light.png" /></td><td><img width="300px" src="captures/android/cpp/grouping_no_templates_light.png" /></td><td><img width="300px" src="captures/android/xaml/grouping_no_templates_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/grouping_no_templates_dark.png" /></td><td><img width="300px" src="captures/android/cpp/grouping_no_templates_dark.png" /></td><td><img width="300px" src="captures/android/xaml/grouping_no_templates_dark.png" /></td></tr></table>

ports GroupingGalleries/GroupingNoTemplates.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9997, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9997, 0.00% pixels differ

### 70. Grouping Plus Selection — 🟢/🟢
<sub>grouping_plus_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/grouping_plus_selection_light.png" /></td><td><img width="300px" src="captures/android/cpp/grouping_plus_selection_light.png" /></td><td><img width="300px" src="captures/android/xaml/grouping_plus_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/grouping_plus_selection_dark.png" /></td><td><img width="300px" src="captures/android/cpp/grouping_plus_selection_dark.png" /></td><td><img width="300px" src="captures/android/xaml/grouping_plus_selection_dark.png" /></td></tr></table>

ports CollectionViewGalleries/GroupingGalleries/ GroupingPlusSelection.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9930, 0.18% pixels differ · Dark: SSIM 0.9924, 0.18% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9999, 0.00% pixels differ

### 71. Header Footer — 🟢/🟢
<sub>header_footer</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/header_footer_light.png" /></td><td><img width="300px" src="captures/android/cpp/header_footer_light.png" /></td><td><img width="300px" src="captures/android/xaml/header_footer_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/header_footer_dark.png" /></td><td><img width="300px" src="captures/android/cpp/header_footer_dark.png" /></td><td><img width="300px" src="captures/android/xaml/header_footer_dark.png" /></td></tr></table>

ports HeaderFooterString.xaml (+ HeaderFooterString.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9999, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 72. Header Footer Grid — 🟢/🟢
<sub>header_footer_grid</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/header_footer_grid_light.png" /></td><td><img width="300px" src="captures/android/cpp/header_footer_grid_light.png" /></td><td><img width="300px" src="captures/android/xaml/header_footer_grid_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/header_footer_grid_dark.png" /></td><td><img width="300px" src="captures/android/cpp/header_footer_grid_dark.png" /></td><td><img width="300px" src="captures/android/xaml/header_footer_grid_dark.png" /></td></tr></table>

ports HeaderFooterGrid.xaml (+ HeaderFooterGrid.xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 73. Header Footer Grid Horizontal — 🟢/🟢
<sub>header_footer_grid_horizontal</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/header_footer_grid_horizontal_light.png" /></td><td><img width="300px" src="captures/android/cpp/header_footer_grid_horizontal_light.png" /></td><td><img width="300px" src="captures/android/xaml/header_footer_grid_horizontal_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/header_footer_grid_horizontal_dark.png" /></td><td><img width="300px" src="captures/android/cpp/header_footer_grid_horizontal_dark.png" /></td><td><img width="300px" src="captures/android/xaml/header_footer_grid_horizontal_dark.png" /></td></tr></table>

ports HeaderFooterGridHorizontal.xaml (+ HeaderFooterGridHorizontal.xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9969, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9969, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 74. Header Footer Template — 🟢/🟢
<sub>header_footer_template</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/header_footer_template_light.png" /></td><td><img width="300px" src="captures/android/cpp/header_footer_template_light.png" /></td><td><img width="300px" src="captures/android/xaml/header_footer_template_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/header_footer_template_dark.png" /></td><td><img width="300px" src="captures/android/cpp/header_footer_template_dark.png" /></td><td><img width="300px" src="captures/android/xaml/header_footer_template_dark.png" /></td></tr></table>

ports HeaderFooterTemplate.xaml (+ .xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 75. Header Footer View — 🟢/🟢
<sub>header_footer_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/header_footer_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/header_footer_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/header_footer_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/header_footer_view_dark.png" /></td><td><img width="300px" src="captures/android/cpp/header_footer_view_dark.png" /></td><td><img width="300px" src="captures/android/xaml/header_footer_view_dark.png" /></td></tr></table>

ports HeaderFooterView.xaml (+ .xaml.cs) of the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 76. Hit Testing — 🟡/🟡
<sub>hit_testing</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/hit_testing_light.png" /></td><td><img width="300px" src="captures/android/cpp/hit_testing_light.png" /></td><td><img width="300px" src="captures/android/xaml/hit_testing_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/hit_testing_dark.png" /></td><td><img width="300px" src="captures/android/cpp/hit_testing_dark.png" /></td><td><img width="300px" src="captures/android/xaml/hit_testing_dark.png" /></td></tr></table>

ports HitTestingPage.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.HitTestingPage)

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9882, 1.12% pixels differ · Dark: SSIM 0.9956, 0.23% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9882, 1.12% pixels differ · Dark: SSIM 0.9956, 0.23% pixels differ

### 77. Horizontal Stack — 🟢/🟢
<sub>horizontal_stack</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/horizontal_stack_light.png" /></td><td><img width="300px" src="captures/android/cpp/horizontal_stack_light.png" /></td><td><img width="300px" src="captures/android/xaml/horizontal_stack_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/horizontal_stack_dark.png" /></td><td><img width="300px" src="captures/android/cpp/horizontal_stack_dark.png" /></td><td><img width="300px" src="captures/android/xaml/horizontal_stack_dark.png" /></td></tr></table>

Horizontal Stack

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 78. Hybrid Web View — 🔴/🔴
<sub>hybrid_web_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/hybrid_web_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/hybrid_web_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/hybrid_web_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/hybrid_web_view_dark.png" /></td><td><img width="300px" src="captures/android/cpp/hybrid_web_view_dark.png" /></td><td><img width="300px" src="captures/android/xaml/hybrid_web_view_dark.png" /></td></tr></table>

ports HybridWebViewPage.xaml (+ HybridWebViewPage.xaml.cs)

#### 🔴 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.8864, 10.31% pixels differ · Dark: SSIM 0.6412, 38.14% pixels differ

#### 🔴 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.8864, 10.31% pixels differ · Dark: SSIM 0.6412, 38.01% pixels differ

### 79. Image — 🟡/🟡
<sub>image</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/image_light.png" /></td><td><img width="300px" src="captures/android/cpp/image_light.png" /></td><td><img width="300px" src="captures/android/xaml/image_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/image_dark.png" /></td><td><img width="300px" src="captures/android/cpp/image_dark.png" /></td><td><img width="300px" src="captures/android/xaml/image_dark.png" /></td></tr></table>

ports ImagePage.xaml (+ ImagePage.xaml.cs)

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9493, 2.40% pixels differ · Dark: SSIM 0.9541, 1.50% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9494, 2.28% pixels differ · Dark: SSIM 0.9541, 1.50% pixels differ

### 80. Image Button — 🟡/🟡
<sub>image_button</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/image_button_light.png" /></td><td><img width="300px" src="captures/android/cpp/image_button_light.png" /></td><td><img width="300px" src="captures/android/xaml/image_button_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/image_button_dark.png" /></td><td><img width="300px" src="captures/android/cpp/image_button_dark.png" /></td><td><img width="300px" src="captures/android/xaml/image_button_dark.png" /></td></tr></table>

ports ImageButtonPage.xaml (+ ImageButtonPage.xaml.cs) A self-contained, code-first demo page for the ImageButton control (the C# gallery-page convention, mirroring the input_controls_page / image_page pattern)

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9307, 5.12% pixels differ · Dark: SSIM 0.9264, 5.73% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9307, 5.12% pixels differ · Dark: SSIM 0.9264, 5.73% pixels differ

### 81. Indicator — 🟢/🟢
<sub>indicator</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/indicator_light.png" /></td><td><img width="300px" src="captures/android/cpp/indicator_light.png" /></td><td><img width="300px" src="captures/android/xaml/indicator_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/indicator_dark.png" /></td><td><img width="300px" src="captures/android/cpp/indicator_dark.png" /></td><td><img width="300px" src="captures/android/xaml/indicator_dark.png" /></td></tr></table>

ports IndicatorPage.xaml A self-contained, code-first demo of the IndicatorView control

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9860, 0.76% pixels differ · Dark: SSIM 0.9838, 0.71% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9874, 0.73% pixels differ · Dark: SSIM 0.9851, 0.66% pixels differ

### 82. Input Controls — 🟢/🟢
<sub>input_controls</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/input_controls_light.png" /></td><td><img width="300px" src="captures/android/cpp/input_controls_light.png" /></td><td><img width="300px" src="captures/android/xaml/input_controls_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/input_controls_dark.png" /></td><td><img width="300px" src="captures/android/cpp/input_controls_dark.png" /></td><td><img width="300px" src="captures/android/xaml/input_controls_dark.png" /></td></tr></table>

a self-contained demo page for the W1-05 input-control set: editor, search_bar, radio_button (+ the radio_button_group attached grouping) and image_button on one vertical stack, wired together so every input drives a visible output (the C#

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9870, 0.34% pixels differ · Dark: SSIM 0.9852, 0.50% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9870, 0.34% pixels differ · Dark: SSIM 0.9852, 0.50% pixels differ

### 83. Input Transparent — 🟢/🟢
<sub>input_transparent</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/input_transparent_light.png" /></td><td><img width="300px" src="captures/android/cpp/input_transparent_light.png" /></td><td><img width="300px" src="captures/android/xaml/input_transparent_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/input_transparent_dark.png" /></td><td><img width="300px" src="captures/android/cpp/input_transparent_dark.png" /></td><td><img width="300px" src="captures/android/xaml/input_transparent_dark.png" /></td></tr></table>

ports InputTransparentPage.xaml (Maui.Controls.Sample.Pages.InputTransparentPage)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9996, 0.05% pixels differ · Dark: SSIM 0.9998, 0.15% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9996, 0.05% pixels differ · Dark: SSIM 0.9997, 0.15% pixels differ

### 84. Invalidate Brush — 🟢/🟢
<sub>invalidate_brush</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/invalidate_brush_light.png" /></td><td><img width="300px" src="captures/android/cpp/invalidate_brush_light.png" /></td><td><img width="300px" src="captures/android/xaml/invalidate_brush_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/invalidate_brush_dark.png" /></td><td><img width="300px" src="captures/android/cpp/invalidate_brush_dark.png" /></td><td><img width="300px" src="captures/android/xaml/invalidate_brush_dark.png" /></td></tr></table>

ports InvalidateBrushGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/InvalidateBrushGallery.xaml (&amp;quot;Invalidate Brushes Playground&amp;quot;): a VerticalStackLayout (Padding 12) with — - a &amp;quot;Change color&amp;quot; Bu

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9999, 0.00% pixels differ · Dark: SSIM 0.9999, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9999, 0.00% pixels differ · Dark: SSIM 0.9999, 0.00% pixels differ

### 85. Invalidate Shadow Host — 🟢/🟢
<sub>invalidate_shadow_host</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/invalidate_shadow_host_light.png" /></td><td><img width="300px" src="captures/android/cpp/invalidate_shadow_host_light.png" /></td><td><img width="300px" src="captures/android/xaml/invalidate_shadow_host_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/invalidate_shadow_host_dark.png" /></td><td><img width="300px" src="captures/android/cpp/invalidate_shadow_host_dark.png" /></td><td><img width="300px" src="captures/android/xaml/invalidate_shadow_host_dark.png" /></td></tr></table>

ports InvalidateShadowHostPage.xaml A self-contained, code-first demo that a shadow re-applies (invalidates) when its host&amp;#x27;s size changes, mirroring the C# core gallery page (Pages/Core/ShadowGalleries/InvalidateShadowHostPage.xaml + .xaml.

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9829, 0.63% pixels differ · Dark: SSIM 0.9843, 0.65% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9829, 0.63% pixels differ · Dark: SSIM 0.9843, 0.65% pixels differ

### 86. Ios Blur Effect — 🟡/🟡
<sub>ios_blur_effect</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_blur_effect_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_blur_effect_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_blur_effect_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/ios_blur_effect_dark.png" /></td><td><img width="300px" src="captures/android/cpp/ios_blur_effect_dark.png" /></td><td><img width="300px" src="captures/android/xaml/ios_blur_effect_dark.png" /></td></tr></table>

ports iOSBlurEffectPage.xaml The .NET MAUI PlatformSpecifics sample (Pages/PlatformSpecifics/iOS/iOSBlurEffectPage.xaml + .xaml.cs): an Image (Source=&amp;quot;oasis.jpg&amp;quot;) carrying the iOSSpecific VisualElement.BlurEffect knob (XAML seeds it to Extr

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9758, 0.48% pixels differ · Dark: SSIM 0.9709, 0.60% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9758, 0.48% pixels differ · Dark: SSIM 0.9709, 0.60% pixels differ

### 87. Ios Date Picker — 🟢/🟢
<sub>ios_date_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_date_picker_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_date_picker_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_date_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/ios_date_picker_dark.png" /></td><td><img width="300px" src="captures/android/cpp/ios_date_picker_dark.png" /></td><td><img width="300px" src="captures/android/xaml/ios_date_picker_dark.png" /></td></tr></table>

ports iOSDatePickerPage.xaml (+ iOSDatePickerPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 88. Ios Entry — 🟢/🟢
<sub>ios_entry</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_entry_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_entry_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_entry_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/ios_entry_dark.png" /></td><td><img width="300px" src="captures/android/cpp/ios_entry_dark.png" /></td><td><img width="300px" src="captures/android/xaml/ios_entry_dark.png" /></td></tr></table>

ports iOSEntryPage.xaml (+ iOSEntryPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9999, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9999, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 89. Ios First Responder — 🟢/🟢
<sub>ios_first_responder</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_first_responder_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_first_responder_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_first_responder_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/ios_first_responder_dark.png" /></td><td><img width="300px" src="captures/android/cpp/ios_first_responder_dark.png" /></td><td><img width="300px" src="captures/android/xaml/ios_first_responder_dark.png" /></td></tr></table>

ports iOSFirstResponderPage.xaml (+ .xaml.cs) The C# iOSFirstResponderPage is a VisualElement-first-responder demo: a StackLayout with an explanatory Label, a &amp;quot;First Entry&amp;quot; + plain &amp;quot;OK&amp;quot; Button (tapping OK dismisses the keyboard because the

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9999, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9999, 0.00% pixels differ

### 90. Ios Pan Gesture — 🟢/🟢
<sub>ios_pan_gesture</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_pan_gesture_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_pan_gesture_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_pan_gesture_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/ios_pan_gesture_dark.png" /></td><td><img width="300px" src="captures/android/cpp/ios_pan_gesture_dark.png" /></td><td><img width="300px" src="captures/android/xaml/ios_pan_gesture_dark.png" /></td></tr></table>

ports iOSPanGestureRecognizerPage.xaml (+ .xaml.cs) The C# iOSPanGestureRecognizerPage is a StackLayout with: a bold message Label (_messageLabel), a &amp;quot;Toggle Simultaneous Gesture Recognition&amp;quot; Button, and a grouped ListView of employees whos

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 91. Ios Picker — 🟢/🟢
<sub>ios_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_picker_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_picker_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/ios_picker_dark.png" /></td><td><img width="300px" src="captures/android/cpp/ios_picker_dark.png" /></td><td><img width="300px" src="captures/android/xaml/ios_picker_dark.png" /></td></tr></table>

ports iOSPickerPage.xaml (+ iOSPickerPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9999, 0.09% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9999, 0.09% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 92. Ios Safe Area — 🟢/🟢
<sub>ios_safe_area</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_safe_area_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_safe_area_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_safe_area_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/ios_safe_area_dark.png" /></td><td><img width="300px" src="captures/android/cpp/ios_safe_area_dark.png" /></td><td><img width="300px" src="captures/android/xaml/ios_safe_area_dark.png" /></td></tr></table>

ports iOSSafeAreaPage.xaml The .NET MAUI PlatformSpecifics sample (Pages/PlatformSpecifics/iOS/iOSSafeAreaPage.xaml + .xaml.cs): a long Lorem-ipsum Label over a &amp;quot;Disable Use Safe Area&amp;quot; button

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9999, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9999, 0.00% pixels differ

### 93. Ios Scroll View — 🟢/🟢
<sub>ios_scroll_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_scroll_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_scroll_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_scroll_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/ios_scroll_view_dark.png" /></td><td><img width="300px" src="captures/android/cpp/ios_scroll_view_dark.png" /></td><td><img width="300px" src="captures/android/xaml/ios_scroll_view_dark.png" /></td></tr></table>

ports iOSScrollViewPage.xaml (+ iOSScrollViewPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9999, 0.00% pixels differ · Dark: SSIM 0.9999, 0.01% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9999, 0.00% pixels differ · Dark: SSIM 0.9999, 0.01% pixels differ

### 94. Ios Search Bar — 🟢/🟢
<sub>ios_search_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_search_bar_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_search_bar_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_search_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/ios_search_bar_dark.png" /></td><td><img width="300px" src="captures/android/cpp/ios_search_bar_dark.png" /></td><td><img width="300px" src="captures/android/xaml/ios_search_bar_dark.png" /></td></tr></table>

ports iOSSearchBarPage.xaml (+ iOSSearchBarPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9862, 0.37% pixels differ · Dark: SSIM 0.9850, 0.42% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9862, 0.37% pixels differ · Dark: SSIM 0.9850, 0.42% pixels differ

### 95. Ios Slider Update On Tap — 🟢/🟢
<sub>ios_slider_update_on_tap</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_slider_update_on_tap_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_slider_update_on_tap_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_slider_update_on_tap_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/ios_slider_update_on_tap_dark.png" /></td><td><img width="300px" src="captures/android/cpp/ios_slider_update_on_tap_dark.png" /></td><td><img width="300px" src="captures/android/xaml/ios_slider_update_on_tap_dark.png" /></td></tr></table>

ports iOSSliderUpdateOnTapPage.xaml (+ iOSSliderUpdateOnTapPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9999, 0.01% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9999, 0.01% pixels differ

### 96. Ios Swipe Transition — 🟢/🟢
<sub>ios_swipe_transition</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_swipe_transition_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_swipe_transition_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_swipe_transition_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/ios_swipe_transition_dark.png" /></td><td><img width="300px" src="captures/android/cpp/ios_swipe_transition_dark.png" /></td><td><img width="300px" src="captures/android/xaml/ios_swipe_transition_dark.png" /></td></tr></table>

ports iOSSwipeViewTransitionModePage.xaml (+ .xaml.cs) The C# iOSSwipeViewTransitionModePage is a StackLayout with: a horizontal row holding a &amp;quot;SwipeTransitionMode:&amp;quot; Label + an EnumPicker over the SwipeTransitionMode enum (Reveal / Drag, Se

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 97. Ios Time Picker — 🟢/🟢
<sub>ios_time_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/ios_time_picker_light.png" /></td><td><img width="300px" src="captures/android/cpp/ios_time_picker_light.png" /></td><td><img width="300px" src="captures/android/xaml/ios_time_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/ios_time_picker_dark.png" /></td><td><img width="300px" src="captures/android/cpp/ios_time_picker_dark.png" /></td><td><img width="300px" src="captures/android/xaml/ios_time_picker_dark.png" /></td></tr></table>

ports iOSTimePickerPage.xaml The .NET MAUI PlatformSpecifics sample (Pages/PlatformSpecifics/iOS/iOSTimePickerPage.xaml + .xaml.cs): a TimePicker carrying the iOSSpecific TimePicker.UpdateMode knob (XAML seeds it to WhenFinished) over a but

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 98. Items — 🟢/🟢
<sub>items</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/items_light.png" /></td><td><img width="300px" src="captures/android/cpp/items_light.png" /></td><td><img width="300px" src="captures/android/xaml/items_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/items_dark.png" /></td><td><img width="300px" src="captures/android/cpp/items_dark.png" /></td><td><img width="300px" src="captures/android/xaml/items_dark.png" /></td></tr></table>

a self-contained demo page for the W2-19 items core: a collection_view over a live observable items source with a templated cell, single selection driving a readout label, and an EmptyView for the cleared state (the C# CollectionView galler

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9948, 0.17% pixels differ · Dark: SSIM 0.9946, 0.19% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9948, 0.17% pixels differ · Dark: SSIM 0.9946, 0.19% pixels differ

### 99. Items Updating Scroll Mode — 🟢/🟢
<sub>items_updating_scroll_mode</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/items_updating_scroll_mode_light.png" /></td><td><img width="300px" src="captures/android/cpp/items_updating_scroll_mode_light.png" /></td><td><img width="300px" src="captures/android/xaml/items_updating_scroll_mode_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/items_updating_scroll_mode_dark.png" /></td><td><img width="300px" src="captures/android/cpp/items_updating_scroll_mode_dark.png" /></td><td><img width="300px" src="captures/android/xaml/items_updating_scroll_mode_dark.png" /></td></tr></table>

ports ItemsUpdatingScrollModeGallery.xaml (+ .xaml.cs) of the C# CollectionView gallery (Maui.Controls.Sample.Pages.CollectionViewGalleries.ScrollModeGalleries.ItemsUpdatingScrollModeGallery)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9999, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9999, 0.00% pixels differ

### 100. Label — 🟡/🟡
<sub>label</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/label_light.png" /></td><td><img width="300px" src="captures/android/cpp/label_light.png" /></td><td><img width="300px" src="captures/android/xaml/label_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/label_dark.png" /></td><td><img width="300px" src="captures/android/cpp/label_dark.png" /></td><td><img width="300px" src="captures/android/xaml/label_dark.png" /></td></tr></table>

ports LabelPage.xaml (+ LabelPage.xaml.cs)

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9374, 2.50% pixels differ · Dark: SSIM 0.9331, 2.75% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9374, 2.50% pixels differ · Dark: SSIM 0.9331, 2.75% pixels differ

### 101. Layout Is Enabled — 🟡/🟡
<sub>layout_is_enabled</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/layout_is_enabled_light.png" /></td><td><img width="300px" src="captures/android/cpp/layout_is_enabled_light.png" /></td><td><img width="300px" src="captures/android/xaml/layout_is_enabled_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/layout_is_enabled_dark.png" /></td><td><img width="300px" src="captures/android/cpp/layout_is_enabled_dark.png" /></td><td><img width="300px" src="captures/android/xaml/layout_is_enabled_dark.png" /></td></tr></table>

ports LayoutIsEnabledPage.xaml (+ LayoutIsEnabledPage.xaml.cs) The C# page demonstrates how IsEnabled on a layout cascades to its children: a 2x2 grid whose left column hosts a &amp;quot;MainLayout&amp;quot; full of state-demo sub-stacks (all-enabled / all-d

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9718, 2.38% pixels differ · Dark: SSIM 0.9846, 1.39% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9718, 2.38% pixels differ · Dark: SSIM 0.9846, 1.39% pixels differ

### 102. Line Gallery — 🟢/🟢
<sub>line_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/line_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/line_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/line_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/line_gallery_dark.png" /></td><td><img width="300px" src="captures/android/cpp/line_gallery_dark.png" /></td><td><img width="300px" src="captures/android/xaml/line_gallery_dark.png" /></td></tr></table>

ports LineGallery.xaml A self-contained, code-first port of the MAUI Shapes LineGallery (Pages/Controls/ShapesGalleries/LineGallery.xaml + .xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9996, 0.02% pixels differ · Dark: SSIM 0.9998, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9996, 0.02% pixels differ · Dark: SSIM 0.9998, 0.00% pixels differ

### 103. Line Join Gallery — 🟢/🟢
<sub>line_join_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/line_join_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/line_join_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/line_join_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/line_join_gallery_dark.png" /></td><td><img width="300px" src="captures/android/cpp/line_join_gallery_dark.png" /></td><td><img width="300px" src="captures/android/xaml/line_join_gallery_dark.png" /></td></tr></table>

ports LineJoinGallery.xaml A self-contained, code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/LineJoinGallery.xaml: a StackLayout that demonstrates the three StrokeLineJoin variants on an identical open polyline

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 104. Measure First Strategy — 🟢/🟢
<sub>measure_first_strategy</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/measure_first_strategy_light.png" /></td><td><img width="300px" src="captures/android/cpp/measure_first_strategy_light.png" /></td><td><img width="300px" src="captures/android/xaml/measure_first_strategy_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/measure_first_strategy_dark.png" /></td><td><img width="300px" src="captures/android/cpp/measure_first_strategy_dark.png" /></td><td><img width="300px" src="captures/android/xaml/measure_first_strategy_dark.png" /></td></tr></table>

ports MeasureFirstStrategy.xaml (+ .xaml.cs) of the C# CollectionView gallery (Maui.Controls.Sample.Pages.CollectionViewGalleries.GroupingGalleries.MeasureFirstStrategy)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9999, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9999, 0.00% pixels differ

### 105. Menu Bar — 🟡/🟢
<sub>menu_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/menu_bar_light.png" /></td><td><img width="300px" src="captures/android/cpp/menu_bar_light.png" /></td><td><img width="300px" src="captures/android/xaml/menu_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/menu_bar_dark.png" /></td><td><img width="300px" src="captures/android/cpp/menu_bar_dark.png" /></td><td><img width="300px" src="captures/android/xaml/menu_bar_dark.png" /></td></tr></table>

ports MenuBarPage.xaml (+ MenuBarPage.cs) The C# page declares three page-level MenuBarItems (Page.MenuBarItems — the app menu bar) and a small visible body: - &amp;quot;Before File&amp;quot; : &amp;quot;Before File Action&amp;quot; (accelerator &amp;quot;b&amp;quot;), &amp;quot;Cool item 1&amp;quot;, a separat

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9703, 3.45% pixels differ · Dark: SSIM 0.9485, 3.61% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 106. Modal — 🟢/🟢
<sub>modal</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/modal_light.png" /></td><td><img width="300px" src="captures/android/cpp/modal_light.png" /></td><td><img width="300px" src="captures/android/xaml/modal_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/modal_dark.png" /></td><td><img width="300px" src="captures/android/cpp/modal_dark.png" /></td><td><img width="300px" src="captures/android/xaml/modal_dark.png" /></td></tr></table>

ports ModalPage.xaml (+ .xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 107. Multiple Bound Selection — 🟡/🟡
<sub>multiple_bound_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/multiple_bound_selection_light.png" /></td><td><img width="300px" src="captures/android/cpp/multiple_bound_selection_light.png" /></td><td><img width="300px" src="captures/android/xaml/multiple_bound_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/multiple_bound_selection_dark.png" /></td><td><img width="300px" src="captures/android/cpp/multiple_bound_selection_dark.png" /></td><td><img width="300px" src="captures/android/xaml/multiple_bound_selection_dark.png" /></td></tr></table>

ports MultipleBoundSelection.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.MultipleBoundSelection)

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9556, 7.26% pixels differ · Dark: SSIM 0.9002, 7.44% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9556, 7.26% pixels differ · Dark: SSIM 0.9002, 7.44% pixels differ

### 108. Navigation Gallery — 🟢/🟢
<sub>navigation_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/navigation_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/navigation_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/navigation_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/navigation_gallery_dark.png" /></td><td><img width="300px" src="captures/android/cpp/navigation_gallery_dark.png" /></td><td><img width="300px" src="captures/android/xaml/navigation_gallery_dark.png" /></td></tr></table>

ports NavigationGallery.xaml (+ .xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 109. Nested Collection — 🟢/🟢
<sub>nested_collection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/nested_collection_light.png" /></td><td><img width="300px" src="captures/android/cpp/nested_collection_light.png" /></td><td><img width="300px" src="captures/android/xaml/nested_collection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/nested_collection_dark.png" /></td><td><img width="300px" src="captures/android/cpp/nested_collection_dark.png" /></td><td><img width="300px" src="captures/android/xaml/nested_collection_dark.png" /></td></tr></table>

ports NestedGalleries/NestedCollectionViewGallery.xaml (+ NestedCollectionViewGallery.xaml.cs) of the C# CollectionView gallery

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 110. Pan Gesture Events — 🟢/🟢
<sub>pan_gesture_events</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/pan_gesture_events_light.png" /></td><td><img width="300px" src="captures/android/cpp/pan_gesture_events_light.png" /></td><td><img width="300px" src="captures/android/xaml/pan_gesture_events_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/pan_gesture_events_dark.png" /></td><td><img width="300px" src="captures/android/cpp/pan_gesture_events_dark.png" /></td><td><img width="300px" src="captures/android/xaml/pan_gesture_events_dark.png" /></td></tr></table>

ports PanGestureEventsGallery.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.PanGestureEventsGallery)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9967, 0.13% pixels differ · Dark: SSIM 0.9967, 0.13% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 111. Path Aspect Gallery — 🟢/🟢
<sub>path_aspect_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/path_aspect_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/path_aspect_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/path_aspect_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/path_aspect_gallery_dark.png" /></td><td><img width="300px" src="captures/android/cpp/path_aspect_gallery_dark.png" /></td><td><img width="300px" src="captures/android/xaml/path_aspect_gallery_dark.png" /></td></tr></table>

ports PathAspectGallery.xaml A self-contained, code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PathAspectGallery.xaml: a StackLayout (Padding 12) that demonstrates the four Path Aspect modes on one identical ge

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 112. Path Gallery — 🟢/🟢
<sub>path_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/path_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/path_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/path_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/path_gallery_dark.png" /></td><td><img width="300px" src="captures/android/cpp/path_gallery_dark.png" /></td><td><img width="300px" src="captures/android/xaml/path_gallery_dark.png" /></td></tr></table>

ports PathGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PathGallery.xaml: a ScrollView over a StackLayout (Padding 12) that walks eight Path variants (plus two caption-only markup-string Labels

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9952, 0.60% pixels differ · Dark: SSIM 0.9985, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9952, 0.60% pixels differ · Dark: SSIM 0.9985, 0.00% pixels differ

### 113. Path Transform String — 🟢/🟢
<sub>path_transform_string</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/path_transform_string_light.png" /></td><td><img width="300px" src="captures/android/cpp/path_transform_string_light.png" /></td><td><img width="300px" src="captures/android/xaml/path_transform_string_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/path_transform_string_dark.png" /></td><td><img width="300px" src="captures/android/cpp/path_transform_string_dark.png" /></td><td><img width="300px" src="captures/android/xaml/path_transform_string_dark.png" /></td></tr></table>

ports PathTransformStringGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PathTransformStringGallery.xaml: a ScrollView over a StackLayout (Padding 12) that shows the SAME two-figure Path geometry

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9858, 0.64% pixels differ · Dark: SSIM 0.9912, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9999, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 114. Picker — 🟢/🟢
<sub>picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/picker_light.png" /></td><td><img width="300px" src="captures/android/cpp/picker_light.png" /></td><td><img width="300px" src="captures/android/xaml/picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/picker_dark.png" /></td><td><img width="300px" src="captures/android/cpp/picker_dark.png" /></td><td><img width="300px" src="captures/android/xaml/picker_dark.png" /></td></tr></table>

ports PickerPage.xaml (+ PickerPage.xaml.cs) A self-contained, code-first demo page for the Picker control (the C# gallery-page convention, mirroring the value_controls_page / pickers_page pattern)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9894, 0.79% pixels differ · Dark: SSIM 0.9871, 0.42% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9894, 0.79% pixels differ · Dark: SSIM 0.9871, 0.42% pixels differ

### 115. Pickers — 🟢/🟢
<sub>pickers</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/pickers_light.png" /></td><td><img width="300px" src="captures/android/cpp/pickers_light.png" /></td><td><img width="300px" src="captures/android/xaml/pickers_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/pickers_dark.png" /></td><td><img width="300px" src="captures/android/cpp/pickers_dark.png" /></td><td><img width="300px" src="captures/android/xaml/pickers_dark.png" /></td></tr></table>

a self-contained demo page for the W1-06 picker set: picker, date_picker and time_picker on one vertical stack, wired together so every selection drives a visible output (the C# gallery-page convention, code-first; the value_controls_page p

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9938, 0.32% pixels differ · Dark: SSIM 0.9944, 0.22% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9993, 0.11% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 116. Pointer Gesture — 🟢/🟢
<sub>pointer_gesture</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/pointer_gesture_light.png" /></td><td><img width="300px" src="captures/android/cpp/pointer_gesture_light.png" /></td><td><img width="300px" src="captures/android/xaml/pointer_gesture_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/pointer_gesture_dark.png" /></td><td><img width="300px" src="captures/android/cpp/pointer_gesture_dark.png" /></td><td><img width="300px" src="captures/android/xaml/pointer_gesture_dark.png" /></td></tr></table>

ports PointerGestureGalleryPage.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.PointerGestureGalleryPage)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 117. Polygon Gallery — 🟢/🟢
<sub>polygon_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/polygon_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/polygon_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/polygon_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/polygon_gallery_dark.png" /></td><td><img width="300px" src="captures/android/cpp/polygon_gallery_dark.png" /></td><td><img width="300px" src="captures/android/xaml/polygon_gallery_dark.png" /></td></tr></table>

ports PolygonGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PolygonGallery.xaml: a ScrollView over a StackLayout (Padding 12) that walks four Polygon variants, each under a caption Label — - &amp;quot;A

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9989, 0.04% pixels differ · Dark: SSIM 0.9988, 0.04% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9989, 0.04% pixels differ · Dark: SSIM 0.9988, 0.04% pixels differ

### 118. Polyline Gallery — 🟢/🟢
<sub>polyline_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/polyline_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/polyline_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/polyline_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/polyline_gallery_dark.png" /></td><td><img width="300px" src="captures/android/cpp/polyline_gallery_dark.png" /></td><td><img width="300px" src="captures/android/xaml/polyline_gallery_dark.png" /></td></tr></table>

ports PolylineGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PolylineGallery.xaml: a StackLayout (Padding 12 — no ScrollView in the C# source) holding two Polyline variants, each under a caption

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9829, 0.51% pixels differ · Dark: SSIM 0.9830, 0.55% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 119. Preselected Item — 🟢/🟢
<sub>preselected_item</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/preselected_item_light.png" /></td><td><img width="300px" src="captures/android/cpp/preselected_item_light.png" /></td><td><img width="300px" src="captures/android/xaml/preselected_item_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/preselected_item_dark.png" /></td><td><img width="300px" src="captures/android/cpp/preselected_item_dark.png" /></td><td><img width="300px" src="captures/android/xaml/preselected_item_dark.png" /></td></tr></table>

ports PreselectedItemGallery.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.PreselectedItemGallery)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9972, 0.43% pixels differ · Dark: SSIM 0.9999, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9972, 0.43% pixels differ · Dark: SSIM 0.9999, 0.00% pixels differ

### 120. Preselected Items — 🟢/🟢
<sub>preselected_items</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/preselected_items_light.png" /></td><td><img width="300px" src="captures/android/cpp/preselected_items_light.png" /></td><td><img width="300px" src="captures/android/xaml/preselected_items_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/preselected_items_dark.png" /></td><td><img width="300px" src="captures/android/cpp/preselected_items_dark.png" /></td><td><img width="300px" src="captures/android/xaml/preselected_items_dark.png" /></td></tr></table>

ports PreselectedItemsGallery.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.PreselectedItemsGallery)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9999, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9999, 0.00% pixels differ

### 121. Progress Bar — 🟢/🟢
<sub>progress_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/progress_bar_light.png" /></td><td><img width="300px" src="captures/android/cpp/progress_bar_light.png" /></td><td><img width="300px" src="captures/android/xaml/progress_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/progress_bar_dark.png" /></td><td><img width="300px" src="captures/android/cpp/progress_bar_dark.png" /></td><td><img width="300px" src="captures/android/xaml/progress_bar_dark.png" /></td></tr></table>

ports ProgressBarPage.xaml (+ ProgressBarPage.xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9987, 0.01% pixels differ · Dark: SSIM 0.9984, 0.24% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9988, 0.01% pixels differ · Dark: SSIM 0.9984, 0.24% pixels differ

### 122. Radio Button Border — 🟢/🟢
<sub>radio_button_border</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/radio_button_border_light.png" /></td><td><img width="300px" src="captures/android/cpp/radio_button_border_light.png" /></td><td><img width="300px" src="captures/android/xaml/radio_button_border_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/radio_button_border_dark.png" /></td><td><img width="300px" src="captures/android/cpp/radio_button_border_dark.png" /></td><td><img width="300px" src="captures/android/xaml/radio_button_border_dark.png" /></td></tr></table>

ports RadioButtonBorder.xaml A self-contained, code-first demo of RadioButton border styling

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9999, 0.10% pixels differ · Dark: SSIM 0.9969, 0.13% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9999, 0.10% pixels differ · Dark: SSIM 0.9969, 0.13% pixels differ

### 123. Radio Button Content — 🟢/🟢
<sub>radio_button_content</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/radio_button_content_light.png" /></td><td><img width="300px" src="captures/android/cpp/radio_button_content_light.png" /></td><td><img width="300px" src="captures/android/xaml/radio_button_content_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/radio_button_content_dark.png" /></td><td><img width="300px" src="captures/android/cpp/radio_button_content_dark.png" /></td><td><img width="300px" src="captures/android/xaml/radio_button_content_dark.png" /></td></tr></table>

ports RadioButtonContentGallery.xaml A self-contained, code-first demo of the RadioButton.Content surface

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9911, 0.27% pixels differ · Dark: SSIM 0.9971, 0.18% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9911, 0.27% pixels differ · Dark: SSIM 0.9971, 0.18% pixels differ

### 124. Radio Button Group — 🟢/🟢
<sub>radio_button_group</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/radio_button_group_light.png" /></td><td><img width="300px" src="captures/android/cpp/radio_button_group_light.png" /></td><td><img width="300px" src="captures/android/xaml/radio_button_group_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/radio_button_group_dark.png" /></td><td><img width="300px" src="captures/android/cpp/radio_button_group_dark.png" /></td><td><img width="300px" src="captures/android/xaml/radio_button_group_dark.png" /></td></tr></table>

ports RadioButtonGroupGallery.xaml A self-contained, code-first demo of the RadioButtonGroup ATTACHED-PROPERTY grouping: a vertical StackLayout carries RadioButtonGroup.GroupName=&amp;quot;foo&amp;quot;, so every descendant RadioButton — including one nested

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9987, 0.15% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9987, 0.15% pixels differ

### 125. Radio Button Group Binding — 🟢/🟢
<sub>radio_button_group_binding</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/radio_button_group_binding_light.png" /></td><td><img width="300px" src="captures/android/cpp/radio_button_group_binding_light.png" /></td><td><img width="300px" src="captures/android/xaml/radio_button_group_binding_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/radio_button_group_binding_dark.png" /></td><td><img width="300px" src="captures/android/cpp/radio_button_group_binding_dark.png" /></td><td><img width="300px" src="captures/android/xaml/radio_button_group_binding_dark.png" /></td></tr></table>

ports RadioButtonGroupBindingGallery.xaml A code-first demo of binding the RadioButtonGroup attached properties (GroupName + SelectedValue) to a view-model

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9987, 0.15% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9987, 0.15% pixels differ

### 126. Radio Button Group Gallery — 🟢/🟢
<sub>radio_button_group_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/radio_button_group_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/radio_button_group_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/radio_button_group_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/radio_button_group_gallery_dark.png" /></td><td><img width="300px" src="captures/android/cpp/radio_button_group_gallery_dark.png" /></td><td><img width="300px" src="captures/android/xaml/radio_button_group_gallery_dark.png" /></td></tr></table>

ports RadioButtonGroupGalleryPage.xaml A self-contained, code-first demo of RadioButton grouping SCOPE, mirroring the C# controls gallery page (Pages/Controls/RadioButtonGalleries/RadioButtonGroupGalleryPage.xaml)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9966, 0.42% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9966, 0.42% pixels differ

### 127. Radio Content Properties — 🟢/🟢
<sub>radio_content_properties</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/radio_content_properties_light.png" /></td><td><img width="300px" src="captures/android/cpp/radio_content_properties_light.png" /></td><td><img width="300px" src="captures/android/xaml/radio_content_properties_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/radio_content_properties_dark.png" /></td><td><img width="300px" src="captures/android/cpp/radio_content_properties_dark.png" /></td><td><img width="300px" src="captures/android/xaml/radio_content_properties_dark.png" /></td></tr></table>

ports ContentProperties.xaml A self-contained, code-first demo of how RadioButton propagates the standard Text/Font properties to its Content

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9999, 0.00% pixels differ · Dark: SSIM 0.9977, 0.26% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9999, 0.00% pixels differ · Dark: SSIM 0.9977, 0.26% pixels differ

### 128. Radio Template From Style — 🟢/🟢
<sub>radio_template_from_style</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/radio_template_from_style_light.png" /></td><td><img width="300px" src="captures/android/cpp/radio_template_from_style_light.png" /></td><td><img width="300px" src="captures/android/xaml/radio_template_from_style_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/radio_template_from_style_dark.png" /></td><td><img width="300px" src="captures/android/cpp/radio_template_from_style_dark.png" /></td><td><img width="300px" src="captures/android/xaml/radio_template_from_style_dark.png" /></td></tr></table>

ports TemplateFromStyle.xaml A self-contained, code-first demo of applying a RadioButton ControlTemplate

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9928, 0.12% pixels differ · Dark: SSIM 0.9886, 0.34% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9979, 0.00% pixels differ · Dark: SSIM 0.9923, 0.28% pixels differ

### 129. Rectangle Gallery — 🟢/🟢
<sub>rectangle_gallery</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/rectangle_gallery_light.png" /></td><td><img width="300px" src="captures/android/cpp/rectangle_gallery_light.png" /></td><td><img width="300px" src="captures/android/xaml/rectangle_gallery_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/rectangle_gallery_dark.png" /></td><td><img width="300px" src="captures/android/cpp/rectangle_gallery_dark.png" /></td><td><img width="300px" src="captures/android/xaml/rectangle_gallery_dark.png" /></td></tr></table>

ports RectangleGallery.xaml A self-contained, code-first port of the MAUI Shapes RectangleGallery (Pages/Controls/ShapesGalleries/RectangleGallery.xaml + .xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9813, 0.76% pixels differ · Dark: SSIM 0.9851, 0.79% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9813, 0.76% pixels differ · Dark: SSIM 0.9851, 0.79% pixels differ

### 130. Refresh View — 🟢/🟢
<sub>refresh_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/refresh_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/refresh_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/refresh_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/refresh_view_dark.png" /></td><td><img width="300px" src="captures/android/cpp/refresh_view_dark.png" /></td><td><img width="300px" src="captures/android/xaml/refresh_view_dark.png" /></td></tr></table>

ports RefreshViewPage.xaml (+ RefreshViewPage.xaml.cs + RefreshViewModel.cs) A self-contained, code-first demo page for the RefreshView control (the C# gallery-page convention, mirroring the swipe_refresh_page pattern)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 131. Relative Layout — 🟢/🟢
<sub>relative_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/relative_layout_light.png" /></td><td><img width="300px" src="captures/android/cpp/relative_layout_light.png" /></td><td><img width="300px" src="captures/android/xaml/relative_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/relative_layout_dark.png" /></td><td><img width="300px" src="captures/android/cpp/relative_layout_dark.png" /></td><td><img width="300px" src="captures/android/xaml/relative_layout_dark.png" /></td></tr></table>

ports RelativeLayoutPage.xaml

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9995, 0.01% pixels differ · Dark: SSIM 0.9995, 0.01% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9996, 0.01% pixels differ · Dark: SSIM 0.9996, 0.01% pixels differ

### 132. Scattered Radio Button — 🟢/🟢
<sub>scattered_radio_button</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/scattered_radio_button_light.png" /></td><td><img width="300px" src="captures/android/cpp/scattered_radio_button_light.png" /></td><td><img width="300px" src="captures/android/xaml/scattered_radio_button_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/scattered_radio_button_dark.png" /></td><td><img width="300px" src="captures/android/cpp/scattered_radio_button_dark.png" /></td><td><img width="300px" src="captures/android/xaml/scattered_radio_button_dark.png" /></td></tr></table>

ports ScatteredRadioButtonGallery.xaml A code-first demo that radio buttons DON&amp;#x27;T have to share a container to be grouped: grouping is by GroupName, so buttons scattered across separate containers (and one bare button outside any grouped co

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9947, 0.19% pixels differ · Dark: SSIM 0.9923, 0.47% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9947, 0.19% pixels differ · Dark: SSIM 0.9923, 0.47% pixels differ

### 133. Scroll Mode Test — 🟢/🟢
<sub>scroll_mode_test</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/scroll_mode_test_light.png" /></td><td><img width="300px" src="captures/android/cpp/scroll_mode_test_light.png" /></td><td><img width="300px" src="captures/android/xaml/scroll_mode_test_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/scroll_mode_test_dark.png" /></td><td><img width="300px" src="captures/android/cpp/scroll_mode_test_dark.png" /></td><td><img width="300px" src="captures/android/xaml/scroll_mode_test_dark.png" /></td></tr></table>

ports ScrollModeTestGallery.xaml (+ .xaml.cs) of the C# CollectionView gallery (Maui.Controls.Sample.Pages.CollectionViewGalleries.ScrollModeGalleries.ScrollModeTestGallery)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9958, 0.18% pixels differ · Dark: SSIM 0.9957, 0.14% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9999, 0.04% pixels differ · Dark: SSIM 0.9999, 0.00% pixels differ

### 134. Scroll To Group — 🟢/🟢
<sub>scroll_to_group</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/scroll_to_group_light.png" /></td><td><img width="300px" src="captures/android/cpp/scroll_to_group_light.png" /></td><td><img width="300px" src="captures/android/xaml/scroll_to_group_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/scroll_to_group_dark.png" /></td><td><img width="300px" src="captures/android/cpp/scroll_to_group_dark.png" /></td><td><img width="300px" src="captures/android/xaml/scroll_to_group_dark.png" /></td></tr></table>

ports ScrollToGalleries/ScrollToGroup.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9999, 0.01% pixels differ · Dark: SSIM 0.9999, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.01% pixels differ · Dark: SSIM 0.9999, 0.00% pixels differ

### 135. Scroll View — 🟢/🟢
<sub>scroll_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/scroll_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/scroll_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/scroll_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/scroll_view_dark.png" /></td><td><img width="300px" src="captures/android/cpp/scroll_view_dark.png" /></td><td><img width="300px" src="captures/android/xaml/scroll_view_dark.png" /></td></tr></table>

ports ScrollViewPage.xaml (+ the ScrollViewPages sub-demos: ScrollViewOrientationPage / ScrollToEndPage / ScrollToFromConstructorPage), code-first

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9965, 0.41% pixels differ · Dark: SSIM 0.9994, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9965, 0.41% pixels differ · Dark: SSIM 0.9995, 0.00% pixels differ

### 136. Search Bar — 🔴/🔴
<sub>search_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/search_bar_light.png" /></td><td><img width="300px" src="captures/android/cpp/search_bar_light.png" /></td><td><img width="300px" src="captures/android/xaml/search_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/search_bar_dark.png" /></td><td><img width="300px" src="captures/android/cpp/search_bar_dark.png" /></td><td><img width="300px" src="captures/android/xaml/search_bar_dark.png" /></td></tr></table>

ports SearchBarPage.xaml (Microsoft.Maui.Controls sample gallery)

#### 🔴 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9053, 2.68% pixels differ · Dark: SSIM 0.8988, 2.95% pixels differ

#### 🔴 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9053, 2.68% pixels differ · Dark: SSIM 0.8988, 2.95% pixels differ

### 137. Selection Command Param — 🟢/🟢
<sub>selection_command_param</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/selection_command_param_light.png" /></td><td><img width="300px" src="captures/android/cpp/selection_command_param_light.png" /></td><td><img width="300px" src="captures/android/xaml/selection_command_param_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/selection_command_param_dark.png" /></td><td><img width="300px" src="captures/android/cpp/selection_command_param_dark.png" /></td><td><img width="300px" src="captures/android/xaml/selection_command_param_dark.png" /></td></tr></table>

ports SelectionChangedCommandParameter.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.SelectionChangedCommandParameter)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9999, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.9999, 0.00% pixels differ

### 138. Selection Synchronization — 🟢/🟢
<sub>selection_synchronization</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/selection_synchronization_light.png" /></td><td><img width="300px" src="captures/android/cpp/selection_synchronization_light.png" /></td><td><img width="300px" src="captures/android/xaml/selection_synchronization_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/selection_synchronization_dark.png" /></td><td><img width="300px" src="captures/android/cpp/selection_synchronization_dark.png" /></td><td><img width="300px" src="captures/android/xaml/selection_synchronization_dark.png" /></td></tr></table>

ports SelectionSynchronization.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.SelectionSynchronization)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9875, 0.73% pixels differ · Dark: SSIM 0.9873, 0.80% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9875, 0.73% pixels differ · Dark: SSIM 0.9873, 0.80% pixels differ

### 139. Semantics — 🟢/🟢
<sub>semantics</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/semantics_light.png" /></td><td><img width="300px" src="captures/android/cpp/semantics_light.png" /></td><td><img width="300px" src="captures/android/xaml/semantics_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/semantics_dark.png" /></td><td><img width="300px" src="captures/android/cpp/semantics_dark.png" /></td><td><img width="300px" src="captures/android/xaml/semantics_dark.png" /></td></tr></table>

ports SemanticsPage.xaml (+ SemanticsPage.xaml.cs) The C# SemanticsPage is an accessibility showcase: a long VerticalStackLayout where nearly every control carries SemanticProperties.Description / .Hint, plus a block of labels exercising Se

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9843, 0.60% pixels differ · Dark: SSIM 0.9830, 0.49% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9843, 0.60% pixels differ · Dark: SSIM 0.9830, 0.49% pixels differ

### 140. Shadow Playground — 🟢/🟢
<sub>shadow_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/shadow_playground_light.png" /></td><td><img width="300px" src="captures/android/cpp/shadow_playground_light.png" /></td><td><img width="300px" src="captures/android/xaml/shadow_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/shadow_playground_dark.png" /></td><td><img width="300px" src="captures/android/cpp/shadow_playground_dark.png" /></td><td><img width="300px" src="captures/android/xaml/shadow_playground_dark.png" /></td></tr></table>

ports ShadowPlaygroundPage.xaml A self-contained, code-first demo of the view Shadow surface, mirroring the C# core gallery page (Pages/Core/ShadowGalleries/ShadowPlaygroundPage.xaml + .xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9993, 0.10% pixels differ · Dark: SSIM 0.9997, 0.02% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9993, 0.10% pixels differ · Dark: SSIM 0.9996, 0.02% pixels differ

### 141. Shape App Theme — 🟢/🔴
<sub>shape_app_theme</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/shape_app_theme_light.png" /></td><td><img width="300px" src="captures/android/cpp/shape_app_theme_light.png" /></td><td><img width="300px" src="captures/android/xaml/shape_app_theme_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/shape_app_theme_dark.png" /></td><td><img width="300px" src="captures/android/cpp/shape_app_theme_dark.png" /></td><td><img width="300px" src="captures/android/xaml/shape_app_theme_dark.png" /></td></tr></table>

ports ShapeAppThemeGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/ShapeAppThemeGallery.xaml: a StackLayout (Padding 12) holding a caption Label and a 200x80 Rectangle, all themed via {AppThemeBi

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🔴 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 0.0726, 97.00% pixels differ

### 142. Shapes — 🟢/🟢
<sub>shapes</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/shapes_light.png" /></td><td><img width="300px" src="captures/android/cpp/shapes_light.png" /></td><td><img width="300px" src="captures/android/xaml/shapes_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/shapes_dark.png" /></td><td><img width="300px" src="captures/android/cpp/shapes_dark.png" /></td><td><img width="300px" src="captures/android/xaml/shapes_dark.png" /></td></tr></table>

a faithful reproduction of the maui-compare &amp;quot;shapes&amp;quot; demo (ComparePages.Shapes()), the shipped-.NET-MAUI reference for the visual-parity comparison: a ScrollView over a vertical stack of four LABELLED shapes, each bold-captioned and Start-a

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9996, 0.01% pixels differ · Dark: SSIM 0.9997, 0.01% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9996, 0.01% pixels differ · Dark: SSIM 0.9997, 0.01% pixels differ

### 143. Single Bound Selection — 🟢/🟢
<sub>single_bound_selection</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/single_bound_selection_light.png" /></td><td><img width="300px" src="captures/android/cpp/single_bound_selection_light.png" /></td><td><img width="300px" src="captures/android/xaml/single_bound_selection_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/single_bound_selection_dark.png" /></td><td><img width="300px" src="captures/android/cpp/single_bound_selection_dark.png" /></td><td><img width="300px" src="captures/android/xaml/single_bound_selection_dark.png" /></td></tr></table>

ports SingleBoundSelection.xaml (+ .xaml.cs) (Maui.Controls.Sample.Pages.CollectionViewGalleries.SelectionGalleries.SingleBoundSelection)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 144. Slider — 🟡/🟡
<sub>slider</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/slider_light.png" /></td><td><img width="300px" src="captures/android/cpp/slider_light.png" /></td><td><img width="300px" src="captures/android/xaml/slider_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/slider_dark.png" /></td><td><img width="300px" src="captures/android/cpp/slider_dark.png" /></td><td><img width="300px" src="captures/android/xaml/slider_dark.png" /></td></tr></table>

ports SliderPage.xaml (+ .xaml.cs) Mirrors the MAUI gallery page: a vertical stack of headlined Slider states — Default, BackgroundColor (Blue), Background (yellow→green LinearGradientBrush), Minimum(5)/Maximum(15) with a value readout (Val

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9862, 0.85% pixels differ · Dark: SSIM 0.9894, 1.14% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9862, 0.85% pixels differ · Dark: SSIM 0.9894, 1.14% pixels differ

### 145. Some Empty Groups — 🟢/🟢
<sub>some_empty_groups</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/some_empty_groups_light.png" /></td><td><img width="300px" src="captures/android/cpp/some_empty_groups_light.png" /></td><td><img width="300px" src="captures/android/xaml/some_empty_groups_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/some_empty_groups_dark.png" /></td><td><img width="300px" src="captures/android/cpp/some_empty_groups_dark.png" /></td><td><img width="300px" src="captures/android/xaml/some_empty_groups_dark.png" /></td></tr></table>

ports GroupingGalleries/SomeEmptyGroups.xaml (+ .xaml.cs)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9946, 0.36% pixels differ · Dark: SSIM 0.9937, 0.40% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 146. Stack Layout — 🟢/🟢
<sub>stack_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/stack_layout_light.png" /></td><td><img width="300px" src="captures/android/cpp/stack_layout_light.png" /></td><td><img width="300px" src="captures/android/xaml/stack_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/stack_layout_dark.png" /></td><td><img width="300px" src="captures/android/cpp/stack_layout_dark.png" /></td><td><img width="300px" src="captures/android/xaml/stack_layout_dark.png" /></td></tr></table>

ports StackLayoutPage.xaml Demonstrates the generic maui::controls::stack_layout (the orientation-switching sibling of the fixed vertical/horizontal stacks) by nesting two inner stacks inside an outer vertical stack with a 12px margin: a &amp;quot;V

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 147. Staggered Layout — 🟢/🟢
<sub>staggered_layout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/staggered_layout_light.png" /></td><td><img width="300px" src="captures/android/cpp/staggered_layout_light.png" /></td><td><img width="300px" src="captures/android/xaml/staggered_layout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/staggered_layout_dark.png" /></td><td><img width="300px" src="captures/android/cpp/staggered_layout_dark.png" /></td><td><img width="300px" src="captures/android/xaml/staggered_layout_dark.png" /></td></tr></table>

ports AlternateLayoutGalleries/StaggeredLayout.xaml (+ StaggeredLayout.xaml.cs) of the C# CollectionView gallery

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 148. Stepper — 🟡/🟡
<sub>stepper</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/stepper_light.png" /></td><td><img width="300px" src="captures/android/cpp/stepper_light.png" /></td><td><img width="300px" src="captures/android/xaml/stepper_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/stepper_dark.png" /></td><td><img width="300px" src="captures/android/cpp/stepper_dark.png" /></td><td><img width="300px" src="captures/android/xaml/stepper_dark.png" /></td></tr></table>

ports StepperPage.xaml (+ StepperPage.xaml.cs)

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9873, 7.37% pixels differ · Dark: SSIM 0.9976, 0.08% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9873, 7.37% pixels differ · Dark: SSIM 0.9976, 0.08% pixels differ

### 149. Styles — 🟢/🟢
<sub>styles</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/styles_light.png" /></td><td><img width="300px" src="captures/android/cpp/styles_light.png" /></td><td><img width="300px" src="captures/android/xaml/styles_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/styles_dark.png" /></td><td><img width="300px" src="captures/android/cpp/styles_dark.png" /></td><td><img width="300px" src="captures/android/xaml/styles_dark.png" /></td></tr></table>

ports StylesPage.xaml

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9998, 0.01% pixels differ · Dark: SSIM 0.9998, 0.01% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9998, 0.01% pixels differ · Dark: SSIM 0.9998, 0.01% pixels differ

### 150. Swipe Gesture — 🟢/🟢
<sub>swipe_gesture</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/swipe_gesture_light.png" /></td><td><img width="300px" src="captures/android/cpp/swipe_gesture_light.png" /></td><td><img width="300px" src="captures/android/xaml/swipe_gesture_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/swipe_gesture_dark.png" /></td><td><img width="300px" src="captures/android/cpp/swipe_gesture_dark.png" /></td><td><img width="300px" src="captures/android/xaml/swipe_gesture_dark.png" /></td></tr></table>

ports SwipeViewGestureRecognizerGallery.xaml (+ .xaml.cs) The MAUI SwipeViewGestureRecognizerGallery is a CollectionView of &amp;quot;message&amp;quot; rows; each row&amp;#x27;s DataTemplate is a SwipeView wired three ways, proving gesture recognizers AND swipe-item

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 151. Swipe Item Position — 🟢/🟢
<sub>swipe_item_position</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/swipe_item_position_light.png" /></td><td><img width="300px" src="captures/android/cpp/swipe_item_position_light.png" /></td><td><img width="300px" src="captures/android/xaml/swipe_item_position_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/swipe_item_position_dark.png" /></td><td><img width="300px" src="captures/android/cpp/swipe_item_position_dark.png" /></td><td><img width="300px" src="captures/android/xaml/swipe_item_position_dark.png" /></td></tr></table>

ports SwipeItemPositionGallery.xaml A code-first port of the MAUI SwipeView sub-gallery Pages/Controls/SwipeViewGalleries/SwipeItemPositionGallery.xaml: a 2-row Grid (Auto / *) with a Picker on top and one SwipeView below that carries TWO S

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9999, 0.09% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9999, 0.09% pixels differ · Dark: SSIM 0.9966, 0.11% pixels differ

### 152. Swipe Item Size — 🟢/🟢
<sub>swipe_item_size</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/swipe_item_size_light.png" /></td><td><img width="300px" src="captures/android/cpp/swipe_item_size_light.png" /></td><td><img width="300px" src="captures/android/xaml/swipe_item_size_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/swipe_item_size_dark.png" /></td><td><img width="300px" src="captures/android/cpp/swipe_item_size_dark.png" /></td><td><img width="300px" src="captures/android/xaml/swipe_item_size_dark.png" /></td></tr></table>

ports SwipeItemSizeGallery.xaml A self-contained, code-first port of the .NET MAUI &amp;quot;SwipeItem Size Gallery&amp;quot;: a scrolling stack of swipe_views demonstrating how a left SwipeItem&amp;#x27;s icon size and the SwipeView content size interact

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9948, 0.61% pixels differ · Dark: SSIM 0.9942, 0.61% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9948, 0.61% pixels differ · Dark: SSIM 0.9942, 0.61% pixels differ

### 153. Swipe Refresh — 🟢/🟢
<sub>swipe_refresh</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/swipe_refresh_light.png" /></td><td><img width="300px" src="captures/android/cpp/swipe_refresh_light.png" /></td><td><img width="300px" src="captures/android/xaml/swipe_refresh_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/swipe_refresh_dark.png" /></td><td><img width="300px" src="captures/android/cpp/swipe_refresh_dark.png" /></td><td><img width="300px" src="captures/android/xaml/swipe_refresh_dark.png" /></td></tr></table>

a self-contained demo page for the W2-20 swipe + refresh controls: a refresh_view wrapping a swipe_view (which itself wraps a labeled row), with a readout label reflecting the latest interaction

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 154. Swipe Threshold — 🟢/🟢
<sub>swipe_threshold</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/swipe_threshold_light.png" /></td><td><img width="300px" src="captures/android/cpp/swipe_threshold_light.png" /></td><td><img width="300px" src="captures/android/xaml/swipe_threshold_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/swipe_threshold_dark.png" /></td><td><img width="300px" src="captures/android/cpp/swipe_threshold_dark.png" /></td><td><img width="300px" src="captures/android/xaml/swipe_threshold_dark.png" /></td></tr></table>

ports HorizontalSwipeThresholdGallery.xaml (+ .xaml.cs) The MAUI HorizontalSwipeThresholdGallery shows how SwipeView.Threshold (the swipe distance, in DIPs, the user must drag before the items settle open / execute) interacts with SwipeItem

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9997, 0.01% pixels differ · Dark: SSIM 0.9997, 0.01% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9997, 0.01% pixels differ · Dark: SSIM 0.9997, 0.01% pixels differ

### 155. Swipe View Margin — 🟢/🟢
<sub>swipe_view_margin</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/swipe_view_margin_light.png" /></td><td><img width="300px" src="captures/android/cpp/swipe_view_margin_light.png" /></td><td><img width="300px" src="captures/android/xaml/swipe_view_margin_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/swipe_view_margin_dark.png" /></td><td><img width="300px" src="captures/android/cpp/swipe_view_margin_dark.png" /></td><td><img width="300px" src="captures/android/xaml/swipe_view_margin_dark.png" /></td></tr></table>

ports SwipeViewMarginGallery.xaml A self-contained, code-first port of the .NET MAUI &amp;quot;SwipeView Margin Gallery&amp;quot;: two swipe_views whose content&amp;#x27;s Margin + Padding are driven by two sliders, demonstrating that the revealed SwipeItems stay cor

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9999, 0.01% pixels differ · Dark: SSIM 0.9997, 0.01% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9999, 0.01% pixels differ · Dark: SSIM 0.9997, 0.01% pixels differ

### 156. Swipe View Shadow — 🟢/🟢
<sub>swipe_view_shadow</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/swipe_view_shadow_light.png" /></td><td><img width="300px" src="captures/android/cpp/swipe_view_shadow_light.png" /></td><td><img width="300px" src="captures/android/xaml/swipe_view_shadow_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/swipe_view_shadow_dark.png" /></td><td><img width="300px" src="captures/android/cpp/swipe_view_shadow_dark.png" /></td><td><img width="300px" src="captures/android/xaml/swipe_view_shadow_dark.png" /></td></tr></table>

ports SwipeViewShadowGallery.xaml A code-first port of the MAUI SwipeView sub-gallery Pages/Controls/SwipeViewGalleries/SwipeViewShadowGallery.xaml: a padded vertical StackLayout proving a drop Shadow renders correctly on SwipeView content

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9867, 0.67% pixels differ · Dark: SSIM 0.9924, 0.09% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9867, 0.67% pixels differ · Dark: SSIM 0.9924, 0.09% pixels differ

### 157. Switch — 🟢/🟢
<sub>switch</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/switch_light.png" /></td><td><img width="300px" src="captures/android/cpp/switch_light.png" /></td><td><img width="300px" src="captures/android/xaml/switch_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/switch_dark.png" /></td><td><img width="300px" src="captures/android/cpp/switch_dark.png" /></td><td><img width="300px" src="captures/android/xaml/switch_dark.png" /></td></tr></table>

ports SwitchPage.xaml (+ .xaml.cs) Mirrors the MAUI gallery page: a vertical stack of headlined Switch states — Default, BackgroundColor (Blue), Background (a yellow→green LinearGradientBrush), Disabled, OnColor (Red), ThumbColor (Orange)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9974, 0.35% pixels differ · Dark: SSIM 0.9984, 0.74% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9974, 0.35% pixels differ · Dark: SSIM 0.9984, 0.74% pixels differ

### 158. Switch Grouping — 🟢/🟢
<sub>switch_grouping</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/switch_grouping_light.png" /></td><td><img width="300px" src="captures/android/cpp/switch_grouping_light.png" /></td><td><img width="300px" src="captures/android/xaml/switch_grouping_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/switch_grouping_dark.png" /></td><td><img width="300px" src="captures/android/cpp/switch_grouping_dark.png" /></td><td><img width="300px" src="captures/android/xaml/switch_grouping_dark.png" /></td></tr></table>

ports CollectionViewGalleries/GroupingGalleries/ SwitchGrouping.xaml (+ .xaml.cs) of the C# CollectionView gallery

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9994, 0.01% pixels differ · Dark: SSIM 0.9990, 0.03% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9994, 0.01% pixels differ · Dark: SSIM 0.9990, 0.03% pixels differ

### 159. Tabbed Flyout — 🟢/🟢
<sub>tabbed_flyout</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/tabbed_flyout_light.png" /></td><td><img width="300px" src="captures/android/cpp/tabbed_flyout_light.png" /></td><td><img width="300px" src="captures/android/xaml/tabbed_flyout_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/tabbed_flyout_dark.png" /></td><td><img width="300px" src="captures/android/cpp/tabbed_flyout_dark.png" /></td><td><img width="300px" src="captures/android/xaml/tabbed_flyout_dark.png" /></td></tr></table>

a self-contained demo page for the W1-10 tabbed + flyout vertical: a flyout_page whose FLYOUT pane is a titled menu (two buttons selecting the detail&amp;#x27;s tabs + a &amp;quot;Toggle flyout&amp;quot; presenting/dismissing itself) and whose DETAIL pane is a tabbed

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 160. Templated View — 🟢/🟢
<sub>templated_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/templated_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/templated_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/templated_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/templated_view_dark.png" /></td><td><img width="300px" src="captures/android/cpp/templated_view_dark.png" /></td><td><img width="300px" src="captures/android/xaml/templated_view_dark.png" /></td></tr></table>

ports TemplatedViewPage.xaml The C# page contrasts a standard CardView control with a compact one driven by a ControlTemplate (&amp;quot;CardViewCompressed&amp;quot;) and a custom Rate control built entirely from a ControlTemplate + a heart PathGeometry

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9916, 0.50% pixels differ · Dark: SSIM 0.9912, 0.54% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9916, 0.50% pixels differ · Dark: SSIM 0.9912, 0.54% pixels differ

### 161. Time Picker — 🟡/🟡
<sub>time_picker</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/time_picker_light.png" /></td><td><img width="300px" src="captures/android/cpp/time_picker_light.png" /></td><td><img width="300px" src="captures/android/xaml/time_picker_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/time_picker_dark.png" /></td><td><img width="300px" src="captures/android/cpp/time_picker_dark.png" /></td><td><img width="300px" src="captures/android/xaml/time_picker_dark.png" /></td></tr></table>

ports TimePickerPage.xaml (+ TimePickerPage.xaml.cs)

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9891, 1.40% pixels differ · Dark: SSIM 0.9653, 0.72% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9891, 1.40% pixels differ · Dark: SSIM 0.9653, 0.72% pixels differ

### 162. Title Bar — 🟢/🟢
<sub>title_bar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/title_bar_light.png" /></td><td><img width="300px" src="captures/android/cpp/title_bar_light.png" /></td><td><img width="300px" src="captures/android/xaml/title_bar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/title_bar_dark.png" /></td><td><img width="300px" src="captures/android/cpp/title_bar_dark.png" /></td><td><img width="300px" src="captures/android/xaml/title_bar_dark.png" /></td></tr></table>

ports TitleBarPage.xaml A self-contained, code-first demo of the TitleBar control

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.06% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9999, 0.06% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 163. Toolbar — 🟢/🟢
<sub>toolbar</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/toolbar_light.png" /></td><td><img width="300px" src="captures/android/cpp/toolbar_light.png" /></td><td><img width="300px" src="captures/android/xaml/toolbar_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/toolbar_dark.png" /></td><td><img width="300px" src="captures/android/cpp/toolbar_dark.png" /></td><td><img width="300px" src="captures/android/xaml/toolbar_dark.png" /></td></tr></table>

ports ToolbarPage.xaml (Maui.Controls.Sample.Pages.ToolbarPage)

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 164. Transform Playground — 🟢/🟢
<sub>transform_playground</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/transform_playground_light.png" /></td><td><img width="300px" src="captures/android/cpp/transform_playground_light.png" /></td><td><img width="300px" src="captures/android/xaml/transform_playground_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/transform_playground_dark.png" /></td><td><img width="300px" src="captures/android/cpp/transform_playground_dark.png" /></td><td><img width="300px" src="captures/android/xaml/transform_playground_dark.png" /></td></tr></table>

ports TransformPlaygroundGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/TransformPlaygroundGallery.xaml: a 50x50 Path rectangle (red fill, blue stroke 4) sits in a 200x200 light-grey panel; belo

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9959, 0.24% pixels differ · Dark: SSIM 0.9957, 0.29% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9996, 0.00% pixels differ · Dark: SSIM 0.9994, 0.05% pixels differ

### 165. Transformations — 🟢/🟢
<sub>transformations</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/transformations_light.png" /></td><td><img width="300px" src="captures/android/cpp/transformations_light.png" /></td><td><img width="300px" src="captures/android/xaml/transformations_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/transformations_dark.png" /></td><td><img width="300px" src="captures/android/cpp/transformations_dark.png" /></td><td><img width="300px" src="captures/android/xaml/transformations_dark.png" /></td></tr></table>

ports TransformationsPage.xaml (+ .xaml.cs) The MAUI TransformationsPage drives a single target view&amp;#x27;s render transforms from a column of knobs: Sliders for Scale / ScaleX / ScaleY (Maximum 10) and Rotation / RotationX / RotationY (Maximum

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9993, 0.01% pixels differ · Dark: SSIM 0.9989, 0.06% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9993, 0.01% pixels differ · Dark: SSIM 0.9989, 0.06% pixels differ

### 166. Triggers — 🟢/🟢
<sub>triggers</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/triggers_light.png" /></td><td><img width="300px" src="captures/android/cpp/triggers_light.png" /></td><td><img width="300px" src="captures/android/xaml/triggers_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/triggers_dark.png" /></td><td><img width="300px" src="captures/android/cpp/triggers_dark.png" /></td><td><img width="300px" src="captures/android/xaml/triggers_dark.png" /></td></tr></table>

ports TriggersPage.xaml

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 167. Update Path Data — 🟢/🟢
<sub>update_path_data</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/update_path_data_light.png" /></td><td><img width="300px" src="captures/android/cpp/update_path_data_light.png" /></td><td><img width="300px" src="captures/android/xaml/update_path_data_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/update_path_data_dark.png" /></td><td><img width="300px" src="captures/android/cpp/update_path_data_dark.png" /></td><td><img width="300px" src="captures/android/xaml/update_path_data_dark.png" /></td></tr></table>

ports UpdatePathDataGallery.xaml A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/UpdatePathDataGallery.xaml: a 2-row Grid (RowSpacing 0) that proves a Path repaints when its Data geometry is replaced at runti

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 168. Varied Size Selector — 🟡/🟡
<sub>varied_size_selector</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/varied_size_selector_light.png" /></td><td><img width="300px" src="captures/android/cpp/varied_size_selector_light.png" /></td><td><img width="300px" src="captures/android/xaml/varied_size_selector_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/varied_size_selector_dark.png" /></td><td><img width="300px" src="captures/android/cpp/varied_size_selector_dark.png" /></td><td><img width="300px" src="captures/android/xaml/varied_size_selector_dark.png" /></td></tr></table>

ports DataTemplateSelectorGalleries/VariedSizeDataTemplateSelectorGallery.xaml (+ VariedSizeDataTemplateSelectorGallery.xaml.cs) of the C# CollectionView gallery

#### 🟡 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9631, 1.81% pixels differ · Dark: SSIM 0.9506, 1.65% pixels differ

#### 🟡 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9738, 1.37% pixels differ · Dark: SSIM 0.9574, 1.27% pixels differ

### 169. Vertical Stack — 🟢/🟢
<sub>vertical_stack</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/vertical_stack_light.png" /></td><td><img width="300px" src="captures/android/cpp/vertical_stack_light.png" /></td><td><img width="300px" src="captures/android/xaml/vertical_stack_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/vertical_stack_dark.png" /></td><td><img width="300px" src="captures/android/cpp/vertical_stack_dark.png" /></td><td><img width="300px" src="captures/android/xaml/vertical_stack_dark.png" /></td></tr></table>

Vertical Stack

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 1.0000, 0.00% pixels differ · Dark: SSIM 1.0000, 0.00% pixels differ

### 170. Visual States — 🟢/🟢
<sub>visual_states</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/visual_states_light.png" /></td><td><img width="300px" src="captures/android/cpp/visual_states_light.png" /></td><td><img width="300px" src="captures/android/xaml/visual_states_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/visual_states_dark.png" /></td><td><img width="300px" src="captures/android/cpp/visual_states_dark.png" /></td><td><img width="300px" src="captures/android/xaml/visual_states_dark.png" /></td></tr></table>

ports VisualStatesPage.xaml

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9944, 0.18% pixels differ · Dark: SSIM 0.9945, 0.18% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9943, 0.18% pixels differ · Dark: SSIM 0.9945, 0.18% pixels differ

### 171. Web View — 🔴/🔴
<sub>web_view</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/web_view_light.png" /></td><td><img width="300px" src="captures/android/cpp/web_view_light.png" /></td><td><img width="300px" src="captures/android/xaml/web_view_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/web_view_dark.png" /></td><td><img width="300px" src="captures/android/cpp/web_view_dark.png" /></td><td><img width="300px" src="captures/android/xaml/web_view_dark.png" /></td></tr></table>

a self-contained demo page for the W1-08 web_view vertical: a web_view loading a STATIC html_web_view_source (no network), back/forward/reload buttons over the handler-pushed CanGoBack/CanGoForward read-onlys, an &amp;quot;Eval 1+1&amp;quot; button driving t

#### 🔴 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9625, 1.51% pixels differ · Dark: SSIM 0.5700, 41.45% pixels differ

#### 🔴 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9703, 1.14% pixels differ · Dark: SSIM 0.5868, 40.77% pixels differ

### 172. Z Index — 🟢/🟢
<sub>z_index</sub>

<table><tr><th></th><th>MAUI</th><th>C++</th><th>C++ &amp; XAML</th></tr><tr><th>Light</th><td><img width="300px" src="captures/android/maui/z_index_light.png" /></td><td><img width="300px" src="captures/android/cpp/z_index_light.png" /></td><td><img width="300px" src="captures/android/xaml/z_index_light.png" /></td></tr><tr><th>Dark</th><td><img width="300px" src="captures/android/maui/z_index_dark.png" /></td><td><img width="300px" src="captures/android/cpp/z_index_dark.png" /></td><td><img width="300px" src="captures/android/xaml/z_index_dark.png" /></td></tr></table>

ports ZIndexPage.xaml (+ ZIndexPage.xaml.cs), code-first

#### 🟢 Pixel-Perfect Score — C++ (C1/C3)

Light: SSIM 0.9999, 0.00% pixels differ · Dark: SSIM 0.9997, 0.01% pixels differ

#### 🟢 Pixel-Perfect Score — C++ &amp; XAML (C2/C4)

Light: SSIM 0.9999, 0.00% pixels differ · Dark: SSIM 0.9997, 0.01% pixels differ

</details>
