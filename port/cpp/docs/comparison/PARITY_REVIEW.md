# C++ port vs .NET MAUI — parity REVIEW (Gemini-judged, for verification)

> **Not authoritative.** Independent Google-Gemini vision pass. These verdicts are NOT in the tracked board (`parity_status.json` / `README.md`) — verify them here first.
>
> Each page's differences are split into two buckets: **🛠 Port diffs** (genuine C++ issues to fix) and **🧩 MAUI quirks** (MAUI-side imperfections — *subject to your ruling*; they do NOT drive the verdict). Rule on the quirk categories below; rulings get recorded in the comparison policy so the loop stops re-litigating them.

**Pages judged: 172** · 115 with port diffs · 191 MAUI-quirk notes across 3 categories.

_Judged by: gemini-3.1-flash-lite ×161, gemini-2.5-flash ×10, gemini-3.5-flash ×1. Full-flash verdicts are more reliable than flash-lite; weight your review accordingly._

## MAUI imperfection categories seen — RULE ON EACH

For each category decide: **ignore** (MAUI imperfection, port need not match) · **match** (port should replicate it) · **case-by-case**. Rulings → comparison policy in `port/CLAUDE.md` / `port/PROJECT.md`.

### Whole-screen padding / margins  (166 page(s)) — _ruling: TBD_
- **Label**: MAUI insets the page inside a card container, cropping the bottom content; C++ port shows the full scrollable content.
- **Button**: MAUI insets the whole page significantly from screen edges; port uses less padding.
- **Entry**: MAUI insets whole page ~60px; port uses ~16px
- **Editor**: MAUI insets the whole page more significantly than the C++ port.
- **Search Bar**: MAUI insets the whole page significantly; the port uses less outer padding.
- **Picker**: MAUI insets whole page ~60px; port uses ~16px
- **Date Picker**: MAUI insets the whole page significantly and crops the bottom content; the C++ port uses less outer padding and shows more content at the bottom.
- **Time Picker**: MAUI insets the whole page significantly (~60px top/bottom/sides); port uses less padding (~16px sides, ~0px top/bottom) and shows more content.
- **Pickers**: MAUI insets whole page with significant padding; port uses less outer padding.
- **Slider**: MAUI insets whole page ~60px; port uses ~16px
- **Stepper**: MAUI insets whole page ~60px; port uses less padding.
- **Switch**: MAUI wraps the page in a card with significant top and side margins; the C++ port renders the page flush to the screen edges.
- **Check Box**: MAUI wraps content in a card with significant top/side padding; the C++ port renders flush to the screen edges.
- **Progress Bar**: MAUI insets the entire page within a container card, while the C++ port renders the page edge-to-edge.
- **Activity Indicator**: MAUI wraps the page in a card with significant top/bottom/side padding; the C++ port renders the page full-screen.
- **Indicator**: MAUI wraps the page in a card with large outer margins and top/bottom cropping; the C++ port renders full-screen.
- **Image**: MAUI wraps the page in a card with significant top/side margins; the C++ port uses a full-screen layout.
- **Image Button**: MAUI wraps the page in a card with significant outer margins and top/bottom cropping; the C++ port renders full-screen.
- **Box View**: MAUI wraps the page in a card with significant top and side padding, while the C++ port renders the page flush to the screen edges.
- **Content View**: MAUI wraps content in a card with significant top/side padding; port renders full-screen
- **Containers**: MAUI wraps the page in a card with significant top/bottom/side margins; the port renders full-screen.
- **Control stack**: MAUI wraps the page in a card with significant top and side margins; the C++ port uses the full screen width.
- **Input Controls**: MAUI insets the entire page content within a card container, while the C++ port renders full-screen.
- **Fonts**: MAUI wraps the page in a card with significant top and side padding; the C++ port renders the page flush to the screen edges.
- **Formatted Text**: MAUI insets the page content within a card with significant top and side margins; the C++ port renders the content flush to the screen edges.
- **Styles**: MAUI wraps the page in a card with large margins and top/bottom cropping; the C++ port renders full-screen.
- **Triggers**: MAUI renders the page inside a card with significant top and side padding; the C++ port renders full-screen.
- **Behaviors**: MAUI wraps the page in a card with significant top and side margins; the C++ port renders the page flush to the screen edges.
- **Semantics**: MAUI wraps the page in a card with significant top/bottom/side margins; the C++ port renders the page full-width/height.
- **App Theme Binding**: MAUI wraps content in a card with significant top/side padding; port uses full-screen layout
- **Stack Layout**: MAUI wraps the page in a card with significant top and side padding; the C++ port renders the page flush to the screen edges.
- **Vertical Stack**: MAUI renders with a large top margin and status bar area, while the C++ port renders closer to the top edge.
- **Horizontal Stack**: MAUI wraps the page in a container with significant top and side padding; the C++ port uses minimal padding.
- **Grid**: MAUI insets the entire page within a card container; the C++ port renders the page full-width.
- **Absolute Layout**: MAUI renders the page inside a gray container card with significant padding, while the C++ port renders full-screen.
- **Flex Layout**: MAUI wraps the page in a card with significant top/bottom/side padding; the C++ port renders full-screen.
- **Relative Layout**: MAUI renders the page inside a container with significant top and side padding, while the C++ port renders closer to the screen edges.
- **Layout alignment (Start/Center/End/Fill)**: MAUI insets the entire page content within a card container with significant padding, while the C++ port renders the content flush to the screen edges.
- **Z Index**: MAUI wraps the page in a card with significant top and side padding; the C++ port uses a different layout container with less padding.
- **Layout Is Enabled**: MAUI wraps the page in a card with significant top and side padding; the C++ port renders the page flush to the screen edges.
- … and 126 more

### Top/bottom cropping  (9 page(s)) — _ruling: TBD_
- **Button**: MAUI crops the top and bottom of the page content; port shows more content.
- **Entry**: MAUI page is cropped top/bottom by harness; port shows more content
- **Search Bar**: MAUI crops the bottom of the page; the port shows additional search bars.
- **Picker**: MAUI crops the bottom of the page; the port shows more content
- **Pickers**: MAUI's harness crops the top/bottom of the page; port shows more content.
- **Slider**: MAUI crops the bottom of the page, hiding several controls visible in the port
- **Stepper**: MAUI crops the bottom of the page, hiding 'ValueChangedEventArgs' stepper and 'Value: 0' label.
- **Empty View Null**: MAUI includes a top navigation bar and status bar area; the C++ port shows a full-screen view with a different status bar layout
- **Empty View Rtl**: MAUI crops content at the top and bottom; the C++ port displays more vertical content.

### Harness chrome (card / nav / status bar)  (16 page(s)) — _ruling: TBD_
- **Button**: MAUI's status bar area is white in light theme, C++ port's is black.
- **Editor**: MAUI's status bar shows 'maui_ios_gallery' and a different time; C++ shows a different time and a black notch.
- **Picker**: MAUI has a navigation bar and status bar chrome; the port has a simpler status bar
- **Grid**: MAUI status bar and navigation area are part of the harness; the C++ port shows the native iOS status bar.
- **Selection Command Param**: MAUI status bar area is blacked out by the harness; the C++ port shows the native status bar
- **Staggered Layout**: MAUI version includes a navigation bar and status bar area, while the C++ port shows a full-screen layout.
- **Empty View Load Simulate**: MAUI displays a status bar and navigation header while the C++ port shows a full-screen view
- **Empty View Load Simulate**: MAUI uses a different status bar layout with a smaller notch area compared to the C++ port
- **Radio Button Group Binding**: MAUI renders a blank screen in the gallery harness while the C++ port renders the actual page content.
- **Radio Template From Style**: MAUI renders a blank screen in both light and dark modes, likely due to a failure to load or render the content within the harness.
- **Basic Swipe**: MAUI status bar and navigation area are obscured by the harness; the C++ port shows standard iOS status bar.
- **Ios Picker**: MAUI uses a navigation bar with a title and back button, while the C++ port shows a bare page
- **Ios Picker**: MAUI has a status bar clock/battery/notch area, while the C++ port has a different status bar layout
- **Ios Search Bar**: MAUI status bar and navigation area are part of the harness; the C++ port uses the native iOS status bar area.
- **Ios Pan Gesture**: MAUI includes a status bar and navigation header; the C++ port shows a different status bar layout and lacks the navigation header.
- **Ios Safe Area**: MAUI status bar and navigation area are part of the harness; the C++ port shows native iOS status bar.

## Per-page review

### 1. Label — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.5-flash</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI insets the page inside a card container, cropping the bottom content; C++ port shows the full scrollable content.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/label.png) | ![](cpp_ios_light/label.png) | ![](csharp_ios_dark/label.png) | ![](cpp_ios_dark/label.png) |

### 2. Button — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-2.5-flash</sub>
**🛠 Port diffs (fix):**
- The text 'Button' (pink, below 'CornerRadius' button) has extra spaces in the C++ port ('B u t t o n').

**🧩 MAUI quirks (discuss):**
- MAUI insets the whole page significantly from screen edges; port uses less padding.
- MAUI crops the top and bottom of the page content; port shows more content.
- MAUI's status bar area is white in light theme, C++ port's is black.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/button.png) | ![](cpp_ios_light/button.png) | ![](csharp_ios_dark/button.png) | ![](cpp_ios_dark/button.png) |

### 3. Entry — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-2.5-flash</sub>
**🛠 Port diffs (fix):**
- C++ port includes a Slider and an Entry with 'Cursor' text not present in MAUI.

**🧩 MAUI quirks (discuss):**
- MAUI insets whole page ~60px; port uses ~16px
- MAUI page is cropped top/bottom by harness; port shows more content

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/entry.png) | ![](cpp_ios_light/entry.png) | ![](csharp_ios_dark/entry.png) | ![](cpp_ios_dark/entry.png) |

### 4. Editor — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-2.5-flash</sub>
**🛠 Port diffs (fix):**
- Placeholder text color differs in both light and dark themes.

**🧩 MAUI quirks (discuss):**
- MAUI insets the whole page more significantly than the C++ port.
- MAUI's status bar shows 'maui_ios_gallery' and a different time; C++ shows a different time and a black notch.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/editor.png) | ![](cpp_ios_light/editor.png) | ![](csharp_ios_dark/editor.png) | ![](cpp_ios_dark/editor.png) |

### 5. Search Bar — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-2.5-flash</sub>
**🛠 Port diffs (fix):**
- The text 'Italic 24' in MAUI is 'Italic 24pt' in the C++ port (extra 'pt').
- The search bar containing 'Italic 24/24pt' is wider in MAUI, causing the adjacent 'x' button to be partially clipped; in the C++ port, this search bar is narrower, and the 'x' button is fully visible and slightly different in size/shape.
- The search icon (magnifying glass) appears slightly bolder/thicker in the C++ port.
- The 'Search...' placeholder text appears slightly bolder in the C++ port.

**🧩 MAUI quirks (discuss):**
- MAUI insets the whole page significantly; the port uses less outer padding.
- MAUI crops the bottom of the page; the port shows additional search bars.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/search_bar.png) | ![](cpp_ios_light/search_bar.png) | ![](csharp_ios_dark/search_bar.png) | ![](cpp_ios_dark/search_bar.png) |

### 6. Picker — 🟢 L:match · 🟢 D:match  <sub>· gemini-2.5-flash</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI insets whole page ~60px; port uses ~16px
- MAUI crops the bottom of the page; the port shows more content
- MAUI has a navigation bar and status bar chrome; the port has a simpler status bar

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/picker.png) | ![](cpp_ios_light/picker.png) | ![](csharp_ios_dark/picker.png) | ![](cpp_ios_dark/picker.png) |

### 7. Date Picker — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-2.5-flash</sub>
**🛠 Port diffs (fix):**
- The date value in the "Default" entry is '21.06.2026' in C++ vs '19.06.2026' in MAUI.
- The date value in the "BackgroundColor" entry is '21.06.2026' in C++ vs '19.06.2026' in MAUI.
- The date value in the green "Background" entry is '21.06.2026' in C++ vs '19.06.2026' in MAUI.
- The date value in the third "Background" entry is '21.06.2026' in C++ vs '19.06.2026' in MAUI.
- The background of the third "Background" entry is a solid pink/purple in C++ vs a blue-green gradient in MAUI.
- The date value in the "Disabled" entry is '21.06.2026' in C++ vs '19.06.2026' in MAUI.

**🧩 MAUI quirks (discuss):**
- MAUI insets the whole page significantly and crops the bottom content; the C++ port uses less outer padding and shows more content at the bottom.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/date_picker.png) | ![](cpp_ios_light/date_picker.png) | ![](csharp_ios_dark/date_picker.png) | ![](cpp_ios_dark/date_picker.png) |

### 8. Time Picker — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-2.5-flash</sub>
**🛠 Port diffs (fix):**
- C++ port's 'BackgroundColor' (blue) is a slightly different shade than MAUI's.
- C++ port's 'Background' (green) is a slightly different shade than MAUI's.
- C++ port's last 'Background' entry has a solid pink/magenta background instead of MAUI's blue gradient.
- C++ port includes 'Disabled' entry, 'TextColor' entry, 'Format' entry, 'IsFocused' entry, 'IsFocused False' label, and 'Set to null' label which are not present in MAUI.

**🧩 MAUI quirks (discuss):**
- MAUI insets the whole page significantly (~60px top/bottom/sides); port uses less padding (~16px sides, ~0px top/bottom) and shows more content.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/time_picker.png) | ![](cpp_ios_light/time_picker.png) | ![](csharp_ios_dark/time_picker.png) | ![](cpp_ios_dark/time_picker.png) |

### 9. Pickers — 🟢 L:match · 🟢 D:match  <sub>· gemini-2.5-flash</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI insets whole page with significant padding; port uses less outer padding.
- MAUI's harness crops the top/bottom of the page; port shows more content.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/pickers.png) | ![](cpp_ios_light/pickers.png) | ![](csharp_ios_dark/pickers.png) | ![](cpp_ios_dark/pickers.png) |

### 10. Slider — 🟢 L:match · 🟢 D:match  <sub>· gemini-2.5-flash</sub>
**🛠 Port diffs (fix):** _none — identical, only MAUI quirks_

**🧩 MAUI quirks (discuss):**
- MAUI insets whole page ~60px; port uses ~16px
- MAUI crops the bottom of the page, hiding several controls visible in the port

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/slider.png) | ![](cpp_ios_light/slider.png) | ![](csharp_ios_dark/slider.png) | ![](cpp_ios_dark/slider.png) |

### 11. Stepper — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-2.5-flash</sub>
**🛠 Port diffs (fix):**
- BackgroundColor stepper: MAUI's red background covers the stepper buttons; C++'s red background is behind the buttons but does not cover them.

**🧩 MAUI quirks (discuss):**
- MAUI insets whole page ~60px; port uses less padding.
- MAUI crops the bottom of the page, hiding 'ValueChangedEventArgs' stepper and 'Value: 0' label.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/stepper.png) | ![](cpp_ios_light/stepper.png) | ![](csharp_ios_dark/stepper.png) | ![](cpp_ios_dark/stepper.png) |

### 12. Switch — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The ThumbColor switch indicator color is incorrectly set to orange in the C++ port instead of the neutral gray seen in MAUI.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side margins; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/switch.png) | ![](cpp_ios_light/switch.png) | ![](csharp_ios_dark/switch.png) | ![](cpp_ios_dark/switch.png) |

### 13. Check Box — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI wraps content in a card with significant top/side padding; the C++ port renders flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/check_box.png) | ![](cpp_ios_light/check_box.png) | ![](csharp_ios_dark/check_box.png) | ![](cpp_ios_dark/check_box.png) |

### 14. Progress Bar — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI insets the entire page within a container card, while the C++ port renders the page edge-to-edge.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/progress_bar.png) | ![](cpp_ios_light/progress_bar.png) | ![](csharp_ios_dark/progress_bar.png) | ![](cpp_ios_dark/progress_bar.png) |

### 15. Activity Indicator — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port renders two additional controls ('Not Running' and '- End of page -') that are missing from the MAUI source.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/bottom/side padding; the C++ port renders the page full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/activity_indicator.png) | ![](cpp_ios_light/activity_indicator.png) | ![](csharp_ios_dark/activity_indicator.png) | ![](cpp_ios_dark/activity_indicator.png) |

### 16. Indicator — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Indicator Size row dots are rendered much larger in the C++ port than in the MAUI reference.
- Indicator MaximumVisible row text is truncated to '7 of 10' in the C++ port, whereas MAUI shows '7 of' followed by a line break.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with large outer margins and top/bottom cropping; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/indicator.png) | ![](cpp_ios_light/indicator.png) | ![](csharp_ios_dark/indicator.png) | ![](cpp_ios_dark/indicator.png) |

### 17. Image — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The 'Font Image Source' label and its corresponding image are completely missing in the C++ port.
- The vertical spacing between the 'UriSource' and 'FileSource' sections is inconsistent with the MAUI reference.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/side margins; the C++ port uses a full-screen layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/image.png) | ![](cpp_ios_light/image.png) | ![](csharp_ios_dark/image.png) | ![](cpp_ios_dark/image.png) |

### 18. Image Button — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- C++ port contains additional UI elements (sliders, extra image examples) not present in the MAUI source.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant outer margins and top/bottom cropping; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/image_button.png) | ![](cpp_ios_light/image_button.png) | ![](csharp_ios_dark/image_button.png) | ![](cpp_ios_dark/image_button.png) |

### 19. Box View — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port renders an additional 'Using CornerRadius' control at the bottom of the page which is absent in the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding, while the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/box_view.png) | ![](cpp_ios_light/box_view.png) | ![](csharp_ios_dark/box_view.png) | ![](cpp_ios_dark/box_view.png) |

### 20. Content View — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI wraps content in a card with significant top/side padding; port renders full-screen

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/content_view.png) | ![](cpp_ios_light/content_view.png) | ![](csharp_ios_dark/content_view.png) | ![](cpp_ios_dark/content_view.png) |

### 21. Containers — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The frame control has a slightly different border thickness and internal padding compared to the MAUI reference.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/bottom/side margins; the port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/containers.png) | ![](cpp_ios_light/containers.png) | ![](csharp_ios_dark/containers.png) | ![](cpp_ios_dark/containers.png) |

### 22. Control stack — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port renders an additional progress bar at the bottom of the page which is absent in the MAUI reference.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side margins; the C++ port uses the full screen width.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/controls_stack.png) | ![](cpp_ios_light/controls_stack.png) | ![](csharp_ios_dark/controls_stack.png) | ![](cpp_ios_dark/controls_stack.png) |

### 23. Input Controls — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI insets the entire page content within a card container, while the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/input_controls.png) | ![](cpp_ios_light/input_controls.png) | ![](csharp_ios_dark/input_controls.png) | ![](cpp_ios_dark/input_controls.png) |

### 24. Fonts — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The layout, font styling, and spacing are identical once the harness-wrapper padding is accounted for._

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/fonts.png) | ![](cpp_ios_light/fonts.png) | ![](csharp_ios_dark/fonts.png) | ![](cpp_ios_dark/fonts.png) |

### 25. Formatted Text — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI insets the page content within a card with significant top and side margins; the C++ port renders the content flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/formatted_text.png) | ![](cpp_ios_light/formatted_text.png) | ![](csharp_ios_dark/formatted_text.png) | ![](cpp_ios_dark/formatted_text.png) |

### 26. Styles — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Button height is significantly smaller in the C++ port.
- Button lacks the rounded corner radius present in the MAUI version.
- Button border thickness and visual style differ from the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with large margins and top/bottom cropping; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/styles.png) | ![](cpp_ios_light/styles.png) | ![](csharp_ios_dark/styles.png) | ![](cpp_ios_dark/styles.png) |

### 27. Triggers — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The layout and styling are identical once the harness wrapper differences are accounted for._

**🧩 MAUI quirks (discuss):**
- MAUI renders the page inside a card with significant top and side padding; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/triggers.png) | ![](cpp_ios_light/triggers.png) | ![](csharp_ios_dark/triggers.png) | ![](cpp_ios_dark/triggers.png) |

### 28. Behaviors — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The text entry field height and internal vertical padding are slightly inconsistent between the two implementations.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side margins; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/behaviors.png) | ![](cpp_ios_light/behaviors.png) | ![](csharp_ios_dark/behaviors.png) | ![](cpp_ios_dark/behaviors.png) |

### 29. Semantics — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port fails to render the bottom section of the page containing the heading level list and subsequent labels.
- The search bar in the C++ port has a different internal layout and spacing compared to the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/bottom/side margins; the C++ port renders the page full-width/height.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/semantics.png) | ![](cpp_ios_light/semantics.png) | ![](csharp_ios_dark/semantics.png) | ![](cpp_ios_dark/semantics.png) |

### 30. App Theme Binding — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI wraps content in a card with significant top/side padding; port uses full-screen layout

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/app_theme_binding.png) | ![](cpp_ios_light/app_theme_binding.png) | ![](csharp_ios_dark/app_theme_binding.png) | ![](cpp_ios_dark/app_theme_binding.png) |

### 31. Stack Layout — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/stack_layout.png) | ![](cpp_ios_light/stack_layout.png) | ![](csharp_ios_dark/stack_layout.png) | ![](cpp_ios_dark/stack_layout.png) |

### 32. Vertical Stack — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI renders with a large top margin and status bar area, while the C++ port renders closer to the top edge.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/vertical_stack.png) | ![](cpp_ios_light/vertical_stack.png) | ![](csharp_ios_dark/vertical_stack.png) | ![](cpp_ios_dark/vertical_stack.png) |

### 33. Horizontal Stack — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port displays six colored squares, whereas the MAUI version only displays four.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a container with significant top and side padding; the C++ port uses minimal padding.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/horizontal_stack.png) | ![](cpp_ios_light/horizontal_stack.png) | ![](csharp_ios_dark/horizontal_stack.png) | ![](cpp_ios_dark/horizontal_stack.png) |

### 34. Grid — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI insets the entire page within a card container; the C++ port renders the page full-width.
- MAUI status bar and navigation area are part of the harness; the C++ port shows the native iOS status bar.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/grid.png) | ![](cpp_ios_light/grid.png) | ![](csharp_ios_dark/grid.png) | ![](cpp_ios_dark/grid.png) |

### 35. Absolute Layout — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The blue rectangle at the top is significantly smaller in the C++ port.
- The green and red side rectangles are missing in the C++ port.
- The 'AutoSized' text block has different dimensions and internal padding.
- The bottom black rectangle is significantly smaller in the C++ port.

**🧩 MAUI quirks (discuss):**
- MAUI renders the page inside a gray container card with significant padding, while the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/absolute_layout.png) | ![](cpp_ios_light/absolute_layout.png) | ![](csharp_ios_dark/absolute_layout.png) | ![](cpp_ios_dark/absolute_layout.png) |

### 36. Flex Layout — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Vertical spacing between the header, content, and footer elements is not perfectly aligned with the MAUI reference.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/bottom/side padding; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/flex_layout.png) | ![](cpp_ios_light/flex_layout.png) | ![](csharp_ios_dark/flex_layout.png) | ![](cpp_ios_dark/flex_layout.png) |

### 37. Relative Layout — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The central gray rectangle has incorrect aspect ratio and dimensions in the C++ port.
- The black inner square within the gray rectangle is incorrectly sized and positioned in the C++ port.

**🧩 MAUI quirks (discuss):**
- MAUI renders the page inside a container with significant top and side padding, while the C++ port renders closer to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/relative_layout.png) | ![](cpp_ios_light/relative_layout.png) | ![](csharp_ios_dark/relative_layout.png) | ![](cpp_ios_dark/relative_layout.png) |

### 38. Layout alignment (Start/Center/End/Fill) — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI insets the entire page content within a card container with significant padding, while the C++ port renders the content flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/alignment.png) | ![](cpp_ios_light/alignment.png) | ![](csharp_ios_dark/alignment.png) | ![](cpp_ios_dark/alignment.png) |

### 39. Z Index — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The vertical spacing between the colored labels is slightly smaller in the C++ port than in the MAUI reference.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port uses a different layout container with less padding.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/z_index.png) | ![](cpp_ios_light/z_index.png) | ![](csharp_ios_dark/z_index.png) | ![](cpp_ios_dark/z_index.png) |

### 40. Layout Is Enabled — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port renders extra layout sections at the bottom of the page that are completely missing from the MAUI source.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/layout_is_enabled.png) | ![](cpp_ios_light/layout_is_enabled.png) | ![](csharp_ios_dark/layout_is_enabled.png) | ![](cpp_ios_dark/layout_is_enabled.png) |

### 41. Shapes — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The Line control has a much shorter length in the C++ port compared to the MAUI reference.

**🧩 MAUI quirks (discuss):**
- MAUI uses a large outer container margin and status bar area; the C++ port uses a full-screen layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/shapes.png) | ![](cpp_ios_light/shapes.png) | ![](csharp_ios_dark/shapes.png) | ![](cpp_ios_dark/shapes.png) |

### 42. Ellipse Gallery — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The layout and rendering of all shapes and text are identical to MAUI once the harness padding is accounted for._

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a container with significant top and side padding; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/ellipse_gallery.png) | ![](cpp_ios_light/ellipse_gallery.png) | ![](csharp_ios_dark/ellipse_gallery.png) | ![](cpp_ios_dark/ellipse_gallery.png) |

### 43. Rectangle Gallery — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port renders an additional 'A Rectangle with curved corners' control at the bottom of the page that is missing in the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI insets the entire page content with significant padding and top/bottom cropping compared to the C++ port.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/rectangle_gallery.png) | ![](cpp_ios_light/rectangle_gallery.png) | ![](csharp_ios_dark/rectangle_gallery.png) | ![](cpp_ios_dark/rectangle_gallery.png) |

### 44. Line Gallery — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The third line (the thick black line) is completely missing in the C++ port in both light and dark themes.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/line_gallery.png) | ![](cpp_ios_light/line_gallery.png) | ![](csharp_ios_dark/line_gallery.png) | ![](cpp_ios_dark/line_gallery.png) |

### 45. Line Join Gallery — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The rendering of the stroke caps and joins is identical between MAUI and the C++ port._

**🧩 MAUI quirks (discuss):**
- MAUI wraps content in a card with significant top and side padding; the C++ port renders content flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/line_join_gallery.png) | ![](cpp_ios_light/line_join_gallery.png) | ![](csharp_ios_dark/line_join_gallery.png) | ![](cpp_ios_dark/line_join_gallery.png) |

### 46. Polygon Gallery — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The NonZero Polygon shape is rendered as a star in the C++ port, whereas MAUI renders a triangle on a horizontal line.

**🧩 MAUI quirks (discuss):**
- MAUI uses a large outer container margin and status bar padding not present in the C++ port.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/polygon_gallery.png) | ![](cpp_ios_light/polygon_gallery.png) | ![](csharp_ios_dark/polygon_gallery.png) | ![](cpp_ios_dark/polygon_gallery.png) |

### 47. Polyline Gallery — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The dashed polyline stroke pattern is incorrect, showing a much higher density of dashes than the MAUI reference.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the content in a card with significant top and side padding; the C++ port renders content flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/polyline_gallery.png) | ![](cpp_ios_light/polyline_gallery.png) | ![](csharp_ios_dark/polyline_gallery.png) | ![](cpp_ios_dark/polyline_gallery.png) |

### 48. Path Gallery — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port is missing the LineSegment, PathGeometry shape, and Cubic Bezier Path controls.
- The C++ port incorrectly includes Overlapping Rectangles and EllipseGeometry controls that are not present in the MAUI version.
- The Composite shape in the C++ port is rendered as a full circle, whereas the MAUI version shows only a partial arc.

**🧩 MAUI quirks (discuss):**
- MAUI insets the entire page content within a container card, while the C++ port renders directly to the screen bounds.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/path_gallery.png) | ![](cpp_ios_light/path_gallery.png) | ![](csharp_ios_dark/path_gallery.png) | ![](cpp_ios_dark/path_gallery.png) |

### 49. Path Aspect Gallery — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port is missing the gray background container rectangles for each image item.
- The C++ port has different internal spacing between the labels and the images compared to MAUI.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the content in a card with significant padding and rounded corners; the C++ port renders the content directly on the screen background.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/path_aspect_gallery.png) | ![](cpp_ios_light/path_aspect_gallery.png) | ![](csharp_ios_dark/path_aspect_gallery.png) | ![](cpp_ios_dark/path_aspect_gallery.png) |

### 50. Path Transform String — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI insets the page content within a container, while the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/path_transform_string.png) | ![](cpp_ios_light/path_transform_string.png) | ![](csharp_ios_dark/path_transform_string.png) | ![](cpp_ios_dark/path_transform_string.png) |

### 51. Composition Gallery — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port scales down all graphical elements (shapes and lines) relative to the MAUI source.
- The internal spacing and relative positioning of the shapes are compressed due to the incorrect scaling.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the content in a card with large margins and top/bottom cropping; the C++ port renders the page full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/composition_gallery.png) | ![](cpp_ios_light/composition_gallery.png) | ![](csharp_ios_dark/composition_gallery.png) | ![](cpp_ios_dark/composition_gallery.png) |

### 52. Transform Playground — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port displays additional ScaleY and SkewX sliders that are absent in the MAUI implementation.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port uses a full-screen layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/transform_playground.png) | ![](cpp_ios_light/transform_playground.png) | ![](csharp_ios_dark/transform_playground.png) | ![](cpp_ios_dark/transform_playground.png) |

### 53. Transformations — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port displays four additional controls (AnchorX, AnchorY, TranslationX, TranslationY) that are missing from the MAUI reference.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/side padding and rounded corners; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/transformations.png) | ![](cpp_ios_light/transformations.png) | ![](csharp_ios_dark/transformations.png) | ![](cpp_ios_dark/transformations.png) |

### 54. Update Path Data — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The path rendering and text layout are identical once accounting for the harness wrapper differences._

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a container with significant top and side padding; the C++ port renders the page edge-to-edge.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/update_path_data.png) | ![](cpp_ios_light/update_path_data.png) | ![](csharp_ios_dark/update_path_data.png) | ![](cpp_ios_dark/update_path_data.png) |

### 55. Auto Size Shapes — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The ellipse shape is incorrect; the C++ port renders a circle instead of the wide ellipse shown in MAUI.

**🧩 MAUI quirks (discuss):**
- MAUI renders the page inside a container with significant top and side padding; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/auto_size_shapes.png) | ![](cpp_ios_light/auto_size_shapes.png) | ![](csharp_ios_dark/auto_size_shapes.png) | ![](cpp_ios_dark/auto_size_shapes.png) |

### 56. Shape App Theme — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The content layout, colors, and sizing are identical once the harness wrapper differences are accounted for._

**🧩 MAUI quirks (discuss):**
- MAUI renders the page inside a large inset card with significant top/bottom cropping; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/shape_app_theme.png) | ![](cpp_ios_light/shape_app_theme.png) | ![](csharp_ios_dark/shape_app_theme.png) | ![](cpp_ios_dark/shape_app_theme.png) |

### 57. Invalidate Brush — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The 'Change color' button is significantly smaller in the C++ port.
- The progress bar width is significantly smaller in the C++ port.
- The vertical spacing between the button and the text is inconsistent with MAUI.

**🧩 MAUI quirks (discuss):**
- MAUI insets the entire page inside a container card with large margins; the C++ port uses minimal padding.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/invalidate_brush.png) | ![](cpp_ios_light/invalidate_brush.png) | ![](csharp_ios_dark/invalidate_brush.png) | ![](cpp_ios_dark/invalidate_brush.png) |

### 58. Gradient brushes — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI wraps content in a card with significant top/side padding; C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/gradient.png) | ![](cpp_ios_light/gradient.png) | ![](csharp_ios_dark/gradient.png) | ![](cpp_ios_dark/gradient.png) |

### 59. Border — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The bordered content container and text are identical in size, color, and placement._

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/bottom margins and rounded corners, while the C++ port renders the page full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/border.png) | ![](cpp_ios_light/border.png) | ![](csharp_ios_dark/border.png) | ![](cpp_ios_dark/border.png) |

### 60. Border Stroke — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The vertical spacing between the slider and the group of three buttons is slightly larger in the C++ port than in MAUI.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/side padding and rounded corners; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/border_stroke.png) | ![](cpp_ios_light/border_stroke.png) | ![](csharp_ios_dark/border_stroke.png) | ![](cpp_ios_dark/border_stroke.png) |

### 61. Border Layout — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Slider track height is thinner in the C++ port.
- Slider thumb diameter is smaller in the C++ port.
- Slider track length is shorter in the C++ port.

**🧩 MAUI quirks (discuss):**
- MAUI wraps content in a card with large margins and top-inset; C++ port uses full-width layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/border_layout.png) | ![](cpp_ios_light/border_layout.png) | ![](csharp_ios_dark/border_layout.png) | ![](cpp_ios_dark/border_layout.png) |

### 62. Border Playground — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port is missing approximately half of the UI controls present in the MAUI version.
- The C++ port layout is truncated vertically, failing to render the full list of controls.

**🧩 MAUI quirks (discuss):**
- MAUI insets the entire page inside a container card with significant top/side margins; the C++ port uses full-width layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/border_playground.png) | ![](cpp_ios_light/border_playground.png) | ![](csharp_ios_dark/border_playground.png) | ![](cpp_ios_dark/border_playground.png) |

### 63. Border Clip Playground — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port renders a dog image inside the border area instead of the red shape graphic shown in MAUI.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a container with significant top and side padding; the C++ port uses a full-screen layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/border_clip_playground.png) | ![](cpp_ios_light/border_clip_playground.png) | ![](csharp_ios_dark/border_clip_playground.png) | ![](cpp_ios_dark/border_clip_playground.png) |

### 64. Border Resize Content — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port renders images inside the shapes instead of the solid colors shown in MAUI.
- The shapes in the C++ port are incorrectly sized and positioned relative to the MAUI layout.
- The C++ port includes extra UI controls (sliders) at the bottom that are not present in the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI uses a large outer container margin and top/bottom cropping; the C++ port uses full-screen layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/border_resize_content.png) | ![](cpp_ios_light/border_resize_content.png) | ![](csharp_ios_dark/border_resize_content.png) | ![](cpp_ios_dark/border_resize_content.png) |

### 65. Borderless — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI renders inside a container with top/bottom padding and status bar overlap, while the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/borderless.png) | ![](cpp_ios_light/borderless.png) | ![](csharp_ios_dark/borderless.png) | ![](cpp_ios_dark/borderless.png) |

### 66. Clip — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port displays two extra image examples (EllipseGeometry and GeometryGroup) that are missing from the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port uses a full-screen layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/clip.png) | ![](cpp_ios_light/clip.png) | ![](csharp_ios_dark/clip.png) | ![](cpp_ios_dark/clip.png) |

### 67. Clip Views — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Search bar background color is lighter/different than the MAUI reference.
- Search bar internal icon alignment and padding are inconsistent with MAUI.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/side margins; the port uses a full-width layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/clip_views.png) | ![](cpp_ios_light/clip_views.png) | ![](csharp_ios_dark/clip_views.png) | ![](cpp_ios_dark/clip_views.png) |

### 68. Clip Corner Radius — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port displays a different image than the MAUI version.
- The C++ port includes an extra 'Bottom Right Corner' slider control not present in the MAUI version.
- The slider track and thumb styling in the C++ port do not match the MAUI design.
- The vertical spacing between the image and the first slider is inconsistent between the two versions.

**🧩 MAUI quirks (discuss):**
- MAUI renders the page inside a card with significant top/side margins; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/clip_corner_radius.png) | ![](cpp_ios_light/clip_corner_radius.png) | ![](csharp_ios_dark/clip_corner_radius.png) | ![](cpp_ios_dark/clip_corner_radius.png) |

### 69. Clip Gallery — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- MAUI fails to load or render the image assets, showing only gray rectangles; the C++ port correctly displays the images.
- The C++ port includes an additional 'Clipped Image using RoundRectangleGeometry' section not present in the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant outer padding and top/bottom cropping; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/clip_gallery.png) | ![](cpp_ios_light/clip_gallery.png) | ![](csharp_ios_dark/clip_gallery.png) | ![](cpp_ios_dark/clip_gallery.png) |

### 70. Clipping — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- C++ port renders extra coffee cup icons at the bottom that are not present in the MAUI version.
- C++ port renders the sequence of numbers as a single concatenated string '12345678' instead of individual spaced elements.
- The internal spacing and alignment of the number sequence differ significantly from the MAUI reference.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/bottom/side margins; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/clipping.png) | ![](cpp_ios_light/clipping.png) | ![](csharp_ios_dark/clipping.png) | ![](cpp_ios_dark/clipping.png) |

### 71. Shadow Playground — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The rendering is identical once the harness-wrapper padding differences are accounted for._

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side margins; the C++ port uses a full-screen layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/shadow_playground.png) | ![](cpp_ios_light/shadow_playground.png) | ![](csharp_ios_dark/shadow_playground.png) | ![](cpp_ios_dark/shadow_playground.png) |

### 72. Invalidate Shadow Host — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The layout and styling are identical once the harness wrapper differences are accounted for._

**🧩 MAUI quirks (discuss):**
- MAUI renders the page inside a card with significant top and side padding; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/invalidate_shadow_host.png) | ![](cpp_ios_light/invalidate_shadow_host.png) | ![](csharp_ios_dark/invalidate_shadow_host.png) | ![](cpp_ios_dark/invalidate_shadow_host.png) |

### 73. CollectionView — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Slightly inconsistent vertical spacing between the header text and the start of the grid items compared to MAUI.

**🧩 MAUI quirks (discuss):**
- MAUI uses a large top margin/inset for the page content; the C++ port places content closer to the top edge.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/collectionview.png) | ![](cpp_ios_light/collectionview.png) | ![](csharp_ios_dark/collectionview.png) | ![](cpp_ios_dark/collectionview.png) |

### 74. Items — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical content and layout once harness wrapper differences are set aside_

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port renders the page flush to the screen edges

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/items.png) | ![](cpp_ios_light/items.png) | ![](csharp_ios_dark/items.png) | ![](cpp_ios_dark/items.png) |

### 75. Single Bound Selection — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The layout and content are identical once the harness-wrapper padding differences are accounted for._

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a container with significant top and side padding; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/single_bound_selection.png) | ![](cpp_ios_light/single_bound_selection.png) | ![](csharp_ios_dark/single_bound_selection.png) | ![](cpp_ios_dark/single_bound_selection.png) |

### 76. Multiple Bound Selection — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Selection highlight background color does not match MAUI in either theme.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side margins; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/multiple_bound_selection.png) | ![](cpp_ios_light/multiple_bound_selection.png) | ![](csharp_ios_dark/multiple_bound_selection.png) | ![](cpp_ios_dark/multiple_bound_selection.png) |

### 77. Preselected Item — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The vertical spacing between list items is slightly tighter in the C++ port than in the MAUI reference.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the content in a card with significant top and side margins; the C++ port renders the list flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/preselected_item.png) | ![](cpp_ios_light/preselected_item.png) | ![](csharp_ios_dark/preselected_item.png) | ![](cpp_ios_dark/preselected_item.png) |

### 78. Preselected Items — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port renders smaller collection view items compared to MAUI.
- The C++ port has tighter internal spacing between collection view items than MAUI.
- The C++ port displays more items per row and more rows total due to the smaller item size and spacing.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the content in a card with large outer margins and top/bottom cropping; the C++ port renders the content edge-to-edge.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/preselected_items.png) | ![](cpp_ios_light/preselected_items.png) | ![](csharp_ios_dark/preselected_items.png) | ![](cpp_ios_dark/preselected_items.png) |

### 79. Selection Command Param — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical once MAUI harness padding and status bar layout are accounted for_

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port renders the page edge-to-edge
- MAUI status bar area is blacked out by the harness; the C++ port shows the native status bar

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/selection_command_param.png) | ![](cpp_ios_light/selection_command_param.png) | ![](csharp_ios_dark/selection_command_param.png) | ![](cpp_ios_dark/selection_command_param.png) |

### 80. Selection Synchronization — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Selection highlight background color is incorrect in both light and dark themes.

**🧩 MAUI quirks (discuss):**
- MAUI renders the page inside a container with significant top and side margins; the C++ port renders full-width.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/selection_synchronization.png) | ![](cpp_ios_light/selection_synchronization.png) | ![](csharp_ios_dark/selection_synchronization.png) | ![](cpp_ios_dark/selection_synchronization.png) |

### 81. Filter Collection — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Slightly different vertical spacing between list items.
- Text font weight and rendering appear slightly different compared to MAUI.
- The search bar height and internal padding are slightly inconsistent with MAUI.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/bottom/side margins; the C++ port renders the page edge-to-edge.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/filter_collection.png) | ![](cpp_ios_light/filter_collection.png) | ![](csharp_ios_dark/filter_collection.png) | ![](cpp_ios_dark/filter_collection.png) |

### 82. Filter Selection — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port renders an additional list item (index 13) at the bottom of the collection view.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side margins; the C++ port uses full-screen width.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/filter_selection.png) | ![](cpp_ios_light/filter_selection.png) | ![](csharp_ios_dark/filter_selection.png) | ![](cpp_ios_dark/filter_selection.png) |

### 83. Header Footer — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical once MAUI harness quirks are set aside_

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/side padding and rounded corners; the C++ port renders the page content directly to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/header_footer.png) | ![](cpp_ios_light/header_footer.png) | ![](csharp_ios_dark/header_footer.png) | ![](cpp_ios_dark/header_footer.png) |

### 84. Header Footer Grid — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Missing 'This Is A Header' text element.
- Missing 'This Is A Footer' text element.
- Missing 'Add Content' buttons in both header and footer sections.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant padding and top/bottom cropping; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/header_footer_grid.png) | ![](cpp_ios_light/header_footer_grid.png) | ![](csharp_ios_dark/header_footer_grid.png) | ![](cpp_ios_dark/header_footer_grid.png) |

### 85. Header Footer Grid Horizontal — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Missing large vertical text 'This Is A Header' in the C++ port.
- The C++ port uses a different layout for the image labels, resulting in a different number of items per row and different spacing.
- The C++ port displays additional image labels not present in the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant padding and top/bottom cropping; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/header_footer_grid_horizontal.png) | ![](cpp_ios_light/header_footer_grid_horizontal.png) | ![](csharp_ios_dark/header_footer_grid_horizontal.png) | ![](cpp_ios_dark/header_footer_grid_horizontal.png) |

### 86. Header Footer Template — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Missing 'This Is A Header' text element.
- Missing 'This Is A Footer' text element.
- The blue boxes are stacked without the vertical spacing present in the MAUI version.
- The date/time text is positioned differently relative to the blue boxes.

**🧩 MAUI quirks (discuss):**
- MAUI insets the entire page inside a container with significant padding and top/bottom cropping; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/header_footer_template.png) | ![](cpp_ios_light/header_footer_template.png) | ![](csharp_ios_dark/header_footer_template.png) | ![](cpp_ios_dark/header_footer_template.png) |

### 87. Header Footer View — ⬛ L:cpp_blank · ⬛ D:cpp_blank  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port fails to render any content, resulting in a blank screen.

**🧩 MAUI quirks (discuss):**
- MAUI insets the entire page within a card container; the C++ port does not replicate this container.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/header_footer_view.png) | ![](cpp_ios_light/header_footer_view.png) | ![](csharp_ios_dark/header_footer_view.png) | ![](cpp_ios_dark/header_footer_view.png) |

### 88. Footer Only String — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The footer element 'This is a footer' is completely missing from the C++ port in both light and dark themes.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the content in a card with significant top and side margins; the C++ port renders flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/footer_only_string.png) | ![](cpp_ios_light/footer_only_string.png) | ![](csharp_ios_dark/footer_only_string.png) | ![](cpp_ios_dark/footer_only_string.png) |

### 89. Basic Grouping — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port fails to render the bottom portion of the list, specifically missing the 'Total members: 4' label and the entire 'Defenders' section.

**🧩 MAUI quirks (discuss):**
- MAUI insets the entire page content within a container card with significant padding, while the C++ port renders closer to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/basic_grouping.png) | ![](cpp_ios_light/basic_grouping.png) | ![](csharp_ios_dark/basic_grouping.png) | ![](cpp_ios_dark/basic_grouping.png) |

### 90. Grid Grouping — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port includes an extra 'This is a header' text element at the top of the list that is missing in the MAUI source.

**🧩 MAUI quirks (discuss):**
- MAUI insets the entire page content within a container card, while the C++ port renders flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/grid_grouping.png) | ![](cpp_ios_light/grid_grouping.png) | ![](csharp_ios_dark/grid_grouping.png) | ![](cpp_ios_dark/grid_grouping.png) |

### 91. Grouping No Templates — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Vertical spacing between list items is inconsistent with the MAUI reference.
- The C++ port list is truncated earlier than the MAUI reference.

**🧩 MAUI quirks (discuss):**
- MAUI renders the list inside a container with significant top and side padding; the C++ port renders closer to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/grouping_no_templates.png) | ![](cpp_ios_light/grouping_no_templates.png) | ![](csharp_ios_dark/grouping_no_templates.png) | ![](cpp_ios_dark/grouping_no_templates.png) |

### 92. Grouping Plus Selection — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port list is truncated and missing the 'Defenders' section and all items following it.
- The 'Total members: 4' label and the 'Defenders' section header are missing in the C++ port.

**🧩 MAUI quirks (discuss):**
- MAUI insets the entire page within a card container; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/grouping_plus_selection.png) | ![](cpp_ios_light/grouping_plus_selection.png) | ![](csharp_ios_dark/grouping_plus_selection.png) | ![](cpp_ios_dark/grouping_plus_selection.png) |

### 93. Switch Grouping — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Vertical spacing between list items and labels is slightly inconsistent with MAUI's layout.
- The font weight or rendering appears slightly thinner in the C++ port compared to MAUI.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port uses a full-screen layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/switch_grouping.png) | ![](cpp_ios_light/switch_grouping.png) | ![](csharp_ios_dark/switch_grouping.png) | ![](cpp_ios_dark/switch_grouping.png) |

### 94. Some Empty Groups — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical content and layout once harness padding is accounted for_

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port renders the page edge-to-edge

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/some_empty_groups.png) | ![](cpp_ios_light/some_empty_groups.png) | ![](csharp_ios_dark/some_empty_groups.png) | ![](cpp_ios_dark/some_empty_groups.png) |

### 95. Scroll To Group — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port list view contains extra items that are not present in the MAUI source.

**🧩 MAUI quirks (discuss):**
- MAUI renders the page within a container with significant top and side padding; the C++ port uses a full-screen layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/scroll_to_group.png) | ![](cpp_ios_light/scroll_to_group.png) | ![](csharp_ios_dark/scroll_to_group.png) | ![](cpp_ios_dark/scroll_to_group.png) |

### 96. Scroll Mode Test — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The 'ItemsUpdatingScrollMode' picker control has a distinct border/background box in the C++ port, whereas it is text-only in MAUI.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the entire page in a card with significant top and side margins; the C++ port uses a full-width layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/scroll_mode_test.png) | ![](cpp_ios_light/scroll_mode_test.png) | ![](csharp_ios_dark/scroll_mode_test.png) | ![](cpp_ios_dark/scroll_mode_test.png) |

### 97. Adaptive Collection — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The port displays 8 items while MAUI displays 7 items.
- The vertical spacing between items is inconsistent between the two implementations.
- The font weight and text rendering appear slightly different.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/side margins; the port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/adaptive_collection.png) | ![](cpp_ios_light/adaptive_collection.png) | ![](csharp_ios_dark/adaptive_collection.png) | ![](cpp_ios_dark/adaptive_collection.png) |

### 98. Staggered Layout — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port displays twice as many items as the MAUI version, suggesting incorrect item sizing or layout spacing.
- The vertical spacing between items is inconsistent between the two implementations.

**🧩 MAUI quirks (discuss):**
- MAUI version includes a navigation bar and status bar area, while the C++ port shows a full-screen layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/staggered_layout.png) | ![](cpp_ios_light/staggered_layout.png) | ![](csharp_ios_dark/staggered_layout.png) | ![](cpp_ios_dark/staggered_layout.png) |

### 99. Varied Size Selector — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port list contains extra items (Milk4, Coffee5) not present in the MAUI source.
- The vertical spacing between list items in the C++ port is tighter than in the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a container with significant top and side margins; the C++ port renders closer to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/varied_size_selector.png) | ![](cpp_ios_light/varied_size_selector.png) | ![](csharp_ios_dark/varied_size_selector.png) | ![](cpp_ios_dark/varied_size_selector.png) |

### 100. Nested Collection — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The port fails to render the nested CollectionView structure, showing only a single flat list of items.
- The port is missing the red text labels (Source 0-3) and the blue caption text associated with each nested item.
- The port uses a different font and layout for the list items compared to the MAUI implementation.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a gray card with significant padding and top/bottom cropping; the port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/nested_collection.png) | ![](cpp_ios_light/nested_collection.png) | ![](csharp_ios_dark/nested_collection.png) | ![](cpp_ios_dark/nested_collection.png) |

### 101. Data Template Selector — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Search bar height is slightly taller in the C++ port compared to MAUI.
- Internal padding within the search bar differs slightly between the two implementations.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side margins; the C++ port uses full-width layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/data_template_selector.png) | ![](cpp_ios_light/data_template_selector.png) | ![](csharp_ios_dark/data_template_selector.png) | ![](cpp_ios_dark/data_template_selector.png) |

### 102. Cv Visual States — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The content layout, spacing, and text rendering are identical once the harness wrapper differences are accounted for._

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/cv_visual_states.png) | ![](cpp_ios_light/cv_visual_states.png) | ![](csharp_ios_dark/cv_visual_states.png) | ![](cpp_ios_dark/cv_visual_states.png) |

### 103. Empty View — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The search bar height is slightly taller in the C++ port compared to the MAUI version.
- The vertical spacing between the search bar and the list items is slightly different.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side margins; the C++ port uses full-screen width.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/empty_view.png) | ![](cpp_ios_light/empty_view.png) | ![](csharp_ios_dark/empty_view.png) | ![](cpp_ios_dark/empty_view.png) |

### 104. Empty View Null — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical content and layout once harness wrapper differences are accounted for_

**🧩 MAUI quirks (discuss):**
- MAUI includes a top navigation bar and status bar area; the C++ port shows a full-screen view with a different status bar layout
- MAUI renders the page within a container with significant top/bottom margins; the C++ port renders the content flush to the screen edges

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/empty_view_null.png) | ![](cpp_ios_light/empty_view_null.png) | ![](csharp_ios_dark/empty_view_null.png) | ![](cpp_ios_dark/empty_view_null.png) |

### 105. Empty View Rtl — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port uses smaller font sizes for the list items, causing more items to fit on the screen.
- The C++ port has tighter vertical and horizontal spacing between list items compared to MAUI.
- The search bar height and internal padding are inconsistent with the MAUI reference.

**🧩 MAUI quirks (discuss):**
- MAUI insets the entire page within a card with large margins; the C++ port uses full-width layout.
- MAUI crops content at the top and bottom; the C++ port displays more vertical content.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/empty_view_rtl.png) | ![](cpp_ios_light/empty_view_rtl.png) | ![](csharp_ios_dark/empty_view_rtl.png) | ![](cpp_ios_dark/empty_view_rtl.png) |

### 106. Empty View Selector — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The layout and styling are identical once the harness-wrapper padding differences are accounted for._

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side margins; the C++ port renders the page edge-to-edge.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/empty_view_selector.png) | ![](cpp_ios_light/empty_view_selector.png) | ![](csharp_ios_dark/empty_view_selector.png) | ![](cpp_ios_dark/empty_view_selector.png) |

### 107. Empty View Swap — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port displays a much larger number of list items compared to the MAUI version, suggesting incorrect item height or container constraints.
- The search bar and toggle control have different vertical spacing and alignment relative to the top of the screen compared to MAUI.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side margins; the C++ port uses the full screen width.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/empty_view_swap.png) | ![](cpp_ios_light/empty_view_swap.png) | ![](csharp_ios_dark/empty_view_swap.png) | ![](cpp_ios_dark/empty_view_swap.png) |

### 108. Empty View Template — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port displays more list items on screen than the MAUI version, suggesting incorrect item height or spacing calculations.
- The search bar and list items in the C++ port appear to have different vertical spacing and sizing compared to the MAUI reference.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the content in a card with significant top and side margins; the C++ port uses the full screen width.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/empty_view_template.png) | ![](cpp_ios_light/empty_view_template.png) | ![](csharp_ios_dark/empty_view_template.png) | ![](cpp_ios_dark/empty_view_template.png) |

### 109. Empty View View — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port renders more list items on screen than MAUI, suggesting the item height or padding is smaller in the port.
- The search bar height and internal padding appear slightly different in the C++ port compared to MAUI.

**🧩 MAUI quirks (discuss):**
- MAUI insets the entire page inside a container with significant top/bottom and side margins; the C++ port uses the full screen width.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/empty_view_view.png) | ![](cpp_ios_light/empty_view_view.png) | ![](csharp_ios_dark/empty_view_view.png) | ![](cpp_ios_dark/empty_view_view.png) |

### 110. Empty View Load Simulate — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI displays a status bar and navigation header while the C++ port shows a full-screen view
- MAUI uses a different status bar layout with a smaller notch area compared to the C++ port

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/empty_view_load_simulate.png) | ![](cpp_ios_light/empty_view_load_simulate.png) | ![](csharp_ios_dark/empty_view_load_simulate.png) | ![](cpp_ios_dark/empty_view_load_simulate.png) |

### 111. Carousel Page — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- C++ port includes additional UI elements (Prev/Next buttons and position label) that are absent in the MAUI reference.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/bottom/side margins; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/carousel_page.png) | ![](cpp_ios_light/carousel_page.png) | ![](csharp_ios_dark/carousel_page.png) | ![](cpp_ios_dark/carousel_page.png) |

### 112. Chat Example — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Bubble background colors do not match MAUI's palette.
- Internal padding and bubble sizing are inconsistent with MAUI's layout.
- Text alignment and bubble positioning relative to each other differ from MAUI.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with large margins and top/bottom cropping; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/chat_example.png) | ![](cpp_ios_light/chat_example.png) | ![](csharp_ios_dark/chat_example.png) | ![](cpp_ios_dark/chat_example.png) |

### 113. Items Updating Scroll Mode — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI insets the entire page content within a card, while the C++ port renders flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/items_updating_scroll_mode.png) | ![](cpp_ios_light/items_updating_scroll_mode.png) | ![](csharp_ios_dark/items_updating_scroll_mode.png) | ![](cpp_ios_dark/items_updating_scroll_mode.png) |

### 114. Radio Button Group — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port uses a different font family and smaller font size for all text labels.
- The vertical spacing between the RadioButton options is significantly tighter in the C++ port.
- The RadioButton control size and hit area appear smaller in the C++ port compared to the MAUI implementation.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with large outer margins and top/bottom cropping; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/radio_button_group.png) | ![](cpp_ios_light/radio_button_group.png) | ![](csharp_ios_dark/radio_button_group.png) | ![](cpp_ios_dark/radio_button_group.png) |

### 115. Radio Button Group Binding — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- MAUI is completely blank in both light and dark modes, failing to render the page content present in the C++ port.

**🧩 MAUI quirks (discuss):**
- MAUI renders a blank screen in the gallery harness while the C++ port renders the actual page content.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/radio_button_group_binding.png) | ![](cpp_ios_light/radio_button_group_binding.png) | ![](csharp_ios_dark/radio_button_group_binding.png) | ![](cpp_ios_dark/radio_button_group_binding.png) |

### 116. Radio Button Group Gallery — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port displays an additional 'Test: mixed group names' section with five extra radio buttons that are completely absent in the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI insets the entire page content within a card with significant padding, while the C++ port uses the full screen width.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/radio_button_group_gallery.png) | ![](cpp_ios_light/radio_button_group_gallery.png) | ![](csharp_ios_dark/radio_button_group_gallery.png) | ![](cpp_ios_dark/radio_button_group_gallery.png) |

### 117. Radio Button Border — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Radio button container height is significantly smaller in the C++ port.
- Internal padding and alignment of the radio button text and icon differ from MAUI.
- The border thickness and corner radius of the radio button containers do not match MAUI.

**🧩 MAUI quirks (discuss):**
- MAUI renders the page inside a card with large outer margins and top/bottom cropping; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/radio_button_border.png) | ![](cpp_ios_light/radio_button_border.png) | ![](csharp_ios_dark/radio_button_border.png) | ![](cpp_ios_dark/radio_button_border.png) |

### 118. Radio Button Content — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- RadioButton content views (the box containing text) are missing in the C++ port.
- The coffee cup icons and their associated layout are missing in the C++ port.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant padding and top/bottom cropping; the port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/radio_button_content.png) | ![](cpp_ios_light/radio_button_content.png) | ![](csharp_ios_dark/radio_button_content.png) | ![](cpp_ios_dark/radio_button_content.png) |

### 119. Radio Content Properties — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port displays extra RadioButton instances at the bottom that are not present in the MAUI version.
- The C++ port shows additional text blocks describing semantic properties that are absent in the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/side padding; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/radio_content_properties.png) | ![](cpp_ios_light/radio_content_properties.png) | ![](csharp_ios_dark/radio_content_properties.png) | ![](cpp_ios_dark/radio_content_properties.png) |

### 120. Radio Template From Style — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port displays three list items (A, B, C) that are entirely missing from the MAUI source of truth.

**🧩 MAUI quirks (discuss):**
- MAUI renders a blank screen in both light and dark modes, likely due to a failure to load or render the content within the harness.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/radio_template_from_style.png) | ![](cpp_ios_light/radio_template_from_style.png) | ![](csharp_ios_dark/radio_template_from_style.png) | ![](cpp_ios_dark/radio_template_from_style.png) |

### 121. Scattered Radio Button — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The horizontal spacing between the radio button circles and their text labels is slightly smaller in the C++ port.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a container with significant top and side padding; the C++ port renders flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/scattered_radio_button.png) | ![](cpp_ios_light/scattered_radio_button.png) | ![](csharp_ios_dark/scattered_radio_button.png) | ![](cpp_ios_dark/scattered_radio_button.png) |

### 122. Swipe Gesture — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Missing text content: 'Welcome to .NET MAUI!' and 'A SwipeView with gesture recognizers' are absent in the C++ port.

**🧩 MAUI quirks (discuss):**
- MAUI renders content inside a card with significant top/side padding; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/swipe_gesture.png) | ![](cpp_ios_light/swipe_gesture.png) | ![](csharp_ios_dark/swipe_gesture.png) | ![](cpp_ios_dark/swipe_gesture.png) |

### 123. Swipe Item Position — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The layout and styling are identical once the harness wrapper differences are accounted for._

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port renders the page edge-to-edge.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/swipe_item_position.png) | ![](cpp_ios_light/swipe_item_position.png) | ![](csharp_ios_dark/swipe_item_position.png) | ![](cpp_ios_dark/swipe_item_position.png) |

### 124. Swipe Item Size — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port renders an additional 'SwipeView 256 Height' row at the bottom which is absent in the MAUI source.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/bottom/side margins; the C++ port renders full-width/height.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/swipe_item_size.png) | ![](cpp_ios_light/swipe_item_size.png) | ![](csharp_ios_dark/swipe_item_size.png) | ![](cpp_ios_dark/swipe_item_size.png) |

### 125. Swipe Threshold — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port renders additional UI elements (Execute Mode section and threshold label) that are missing in the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/bottom/side padding; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/swipe_threshold.png) | ![](cpp_ios_light/swipe_threshold.png) | ![](csharp_ios_dark/swipe_threshold.png) | ![](cpp_ios_dark/swipe_threshold.png) |

### 126. Swipe View Margin — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port includes an additional 'Vertical Swipeltems' control at the bottom of the list that is missing in the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant outer margins and top/bottom cropping; the C++ port uses a full-screen layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/swipe_view_margin.png) | ![](cpp_ios_light/swipe_view_margin.png) | ![](csharp_ios_dark/swipe_view_margin.png) | ![](cpp_ios_dark/swipe_view_margin.png) |

### 127. Swipe View Shadow — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Light theme: SwipeView containers are missing the drop shadow effect present in MAUI.
- Dark theme: SwipeView containers are completely missing; only the text labels are rendered.

**🧩 MAUI quirks (discuss):**
- MAUI insets the entire page content within a card-like container with significant margins, while the C++ port renders content closer to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/swipe_view_shadow.png) | ![](cpp_ios_light/swipe_view_shadow.png) | ![](csharp_ios_dark/swipe_view_shadow.png) | ![](cpp_ios_dark/swipe_view_shadow.png) |

### 128. Swipe Refresh — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI wraps content in a card with significant top/side padding; port renders full-screen

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/swipe_refresh.png) | ![](cpp_ios_light/swipe_refresh.png) | ![](csharp_ios_dark/swipe_refresh.png) | ![](cpp_ios_dark/swipe_refresh.png) |

### 129. Refresh View — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Vertical spacing between the button rows and the surrounding text labels is slightly compressed in the C++ port.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page content in a card with significant top and side padding; the C++ port renders the content flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/refresh_view.png) | ![](cpp_ios_light/refresh_view.png) | ![](csharp_ios_dark/refresh_view.png) | ![](cpp_ios_dark/refresh_view.png) |

### 130. Custom Size Swipe — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port renders the page edge-to-edge.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/custom_size_swipe.png) | ![](cpp_ios_light/custom_size_swipe.png) | ![](csharp_ios_dark/custom_size_swipe.png) | ![](cpp_ios_dark/custom_size_swipe.png) |

### 131. Custom Swipe Item View — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI wraps content in a card with significant top/side padding; port uses full-width layout

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/custom_swipe_item_view.png) | ![](cpp_ios_light/custom_swipe_item_view.png) | ![](csharp_ios_dark/custom_swipe_item_view.png) | ![](cpp_ios_dark/custom_swipe_item_view.png) |

### 132. Basic Swipe — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Vertical spacing between the list items and the bottom text is inconsistent with the MAUI reference.
- The font weight or rendering style of the text labels appears slightly thinner in the C++ port.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port uses a full-screen layout.
- MAUI status bar and navigation area are obscured by the harness; the C++ port shows standard iOS status bar.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/basic_swipe.png) | ![](cpp_ios_light/basic_swipe.png) | ![](csharp_ios_dark/basic_swipe.png) | ![](cpp_ios_dark/basic_swipe.png) |

### 133. Gestures — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The 'Last gesture' text content differs between the two implementations.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/gestures.png) | ![](cpp_ios_light/gestures.png) | ![](csharp_ios_dark/gestures.png) | ![](cpp_ios_dark/gestures.png) |

### 134. Pan Gesture Events — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/bottom/side padding; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/pan_gesture_events.png) | ![](cpp_ios_light/pan_gesture_events.png) | ![](csharp_ios_dark/pan_gesture_events.png) | ![](cpp_ios_dark/pan_gesture_events.png) |

### 135. Pointer Gesture — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI insets the entire page content within a card container with significant top and side margins, while the C++ port renders the content flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/pointer_gesture.png) | ![](cpp_ios_light/pointer_gesture.png) | ![](csharp_ios_dark/pointer_gesture.png) | ![](cpp_ios_dark/pointer_gesture.png) |

### 136. Drag Drop — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- C++ port displays additional debug text lines (Drag position, Drop position, Move status) not present in the MAUI version.
- C++ port is missing the 'Drag start position relative to...' header text.
- C++ port is missing the 'Self X:0, Y:0 (Red)' text line.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/bottom/side padding; the C++ port renders the page full-width/height.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/drag_drop.png) | ![](cpp_ios_light/drag_drop.png) | ![](csharp_ios_dark/drag_drop.png) | ![](cpp_ios_dark/drag_drop.png) |

### 137. Hit Testing — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port includes a large green rectangle and a red rectangle at the bottom of the page that are missing in the MAUI version.
- The 'Selected' text value differs ('-' in MAUI vs 'Image' in C++ port).

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/bottom/side padding; the C++ port renders the page full-width.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/hit_testing.png) | ![](cpp_ios_light/hit_testing.png) | ![](csharp_ios_dark/hit_testing.png) | ![](cpp_ios_dark/hit_testing.png) |

### 138. Input Transparent — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port renders an additional toggle switch and text label at the bottom of the page which are not present in the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/side padding; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/input_transparent.png) | ![](cpp_ios_light/input_transparent.png) | ![](csharp_ios_dark/input_transparent.png) | ![](cpp_ios_dark/input_transparent.png) |

### 139. Focus — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/focus.png) | ![](cpp_ios_light/focus.png) | ![](csharp_ios_dark/focus.png) | ![](cpp_ios_dark/focus.png) |

### 140. Dispatcher — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- C++ port displays extra UI elements (3 Second Timer, tick count, and Device.StartTimer text) missing from MAUI.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port renders full-width.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/dispatcher.png) | ![](cpp_ios_light/dispatcher.png) | ![](csharp_ios_dark/dispatcher.png) | ![](cpp_ios_dark/dispatcher.png) |

### 141. Device — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Text alignment is incorrect; MAUI centers the text block, while the C++ port aligns it to the top-left.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the content in a centered card with significant padding; the C++ port renders content flush to the top-left screen area.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/device.png) | ![](cpp_ios_light/device.png) | ![](csharp_ios_dark/device.png) | ![](cpp_ios_dark/device.png) |

### 142. Effects — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Missing Entry controls in both light and dark themes.
- Missing labels 'Entry With Focus Routing Effect' and 'Entry With Focus Platform Effect' in both themes.
- The C++ port background color is pure black in dark mode, whereas MAUI uses a slightly lighter shade.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/side padding; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/effects.png) | ![](cpp_ios_light/effects.png) | ![](csharp_ios_dark/effects.png) | ![](cpp_ios_dark/effects.png) |

### 143. Measure First Strategy — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Vertical spacing between list items is slightly tighter in the C++ port than in the MAUI reference.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the content in a card with significant top/side margins and rounded corners; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/measure_first_strategy.png) | ![](cpp_ios_light/measure_first_strategy.png) | ![](csharp_ios_dark/measure_first_strategy.png) | ![](cpp_ios_dark/measure_first_strategy.png) |

### 144. Scroll View — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port includes an extra 'Scrolled to: 0 / 0 (done)' text element at the top of the list that is not present in the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI insets the entire page content within a container card with significant top/bottom/side margins; the C++ port renders flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/scroll_view.png) | ![](cpp_ios_light/scroll_view.png) | ![](csharp_ios_dark/scroll_view.png) | ![](cpp_ios_dark/scroll_view.png) |

### 145. Web View — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- C++ port includes extra header text ('Welcome', 'Served from...') that is absent in the MAUI reference.
- The C++ port fails to render the long file path string present in the MAUI dark mode view.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/side margins; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/web_view.png) | ![](cpp_ios_light/web_view.png) | ![](csharp_ios_dark/web_view.png) | ![](cpp_ios_dark/web_view.png) |

### 146. Hybrid Web View — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Text labels in the C++ port are truncated with ellipses, while MAUI labels are fully visible.
- The C++ port uses a different font weight or rendering style for the labels compared to MAUI.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/side margins; the C++ port uses the full screen width.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/hybrid_web_view.png) | ![](cpp_ios_light/hybrid_web_view.png) | ![](csharp_ios_dark/hybrid_web_view.png) | ![](cpp_ios_dark/hybrid_web_view.png) |

### 147. Alerts — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The layout and styling are identical once the harness-wrapper padding differences are accounted for._

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant outer padding and top/bottom cropping; the C++ port renders the page edge-to-edge.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/alerts.png) | ![](cpp_ios_light/alerts.png) | ![](csharp_ios_dark/alerts.png) | ![](cpp_ios_dark/alerts.png) |

### 148. Animation — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The text label 't (animation target)' is rotated 90 degrees in MAUI but appears horizontal in the C++ port.

**🧩 MAUI quirks (discuss):**
- MAUI renders the page inside a card with significant top/bottom and side margins; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/animation.png) | ![](cpp_ios_light/animation.png) | ![](csharp_ios_dark/animation.png) | ![](cpp_ios_dark/animation.png) |

### 149. Application Control — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The status text 'main window: (untitled)' in MAUI is replaced by 'main window: MAUI C++ — gallery' in the port.

**🧩 MAUI quirks (discuss):**
- MAUI uses a large top-margin/inset for the page content, while the C++ port renders closer to the top screen edge.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/application_control.png) | ![](cpp_ios_light/application_control.png) | ![](csharp_ios_dark/application_control.png) | ![](cpp_ios_dark/application_control.png) |

### 150. Ios Entry — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The layout and styling are identical once the harness wrapper differences are accounted for._

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port renders the page edge-to-edge.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/ios_entry.png) | ![](cpp_ios_light/ios_entry.png) | ![](csharp_ios_dark/ios_entry.png) | ![](cpp_ios_dark/ios_entry.png) |

### 151. Ios Date Picker — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI displays the page within a card with significant top and side padding, while the C++ port renders the page edge-to-edge.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/ios_date_picker.png) | ![](cpp_ios_light/ios_date_picker.png) | ![](csharp_ios_dark/ios_date_picker.png) | ![](cpp_ios_dark/ios_date_picker.png) |

### 152. Ios Time Picker — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The layout and styling are identical once the harness wrapper differences are accounted for._

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port renders the page edge-to-edge.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/ios_time_picker.png) | ![](cpp_ios_light/ios_time_picker.png) | ![](csharp_ios_dark/ios_time_picker.png) | ![](cpp_ios_dark/ios_time_picker.png) |

### 153. Ios Picker — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI uses a navigation bar with a title and back button, while the C++ port shows a bare page
- MAUI has a status bar clock/battery/notch area, while the C++ port has a different status bar layout

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/ios_picker.png) | ![](cpp_ios_light/ios_picker.png) | ![](csharp_ios_dark/ios_picker.png) | ![](cpp_ios_dark/ios_picker.png) |

### 154. Ios Search Bar — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The layout and styling are identical once the harness wrapper differences are accounted for._

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side margins; the C++ port renders the page edge-to-edge.
- MAUI status bar and navigation area are part of the harness; the C++ port uses the native iOS status bar area.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/ios_search_bar.png) | ![](cpp_ios_light/ios_search_bar.png) | ![](csharp_ios_dark/ios_search_bar.png) | ![](cpp_ios_dark/ios_search_bar.png) |

### 155. Ios Scroll View — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port displays a back button in the top-left corner which is not present in the MAUI layout.

**🧩 MAUI quirks (discuss):**
- MAUI uses a large top-margin/harness area that is absent in the C++ port.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/ios_scroll_view.png) | ![](cpp_ios_light/ios_scroll_view.png) | ![](csharp_ios_dark/ios_scroll_view.png) | ![](cpp_ios_dark/ios_scroll_view.png) |

### 156. Ios Slider Update On Tap — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Slider thumb and track height are slightly smaller in the C++ port.
- Vertical spacing between the text and the slider is slightly tighter in the C++ port.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side margins; the C++ port uses full-screen layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/ios_slider_update_on_tap.png) | ![](cpp_ios_light/ios_slider_update_on_tap.png) | ![](csharp_ios_dark/ios_slider_update_on_tap.png) | ![](cpp_ios_dark/ios_slider_update_on_tap.png) |

### 157. Ios First Responder — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI wraps content in a card with significant top/bottom/side padding; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/ios_first_responder.png) | ![](cpp_ios_light/ios_first_responder.png) | ![](csharp_ios_dark/ios_first_responder.png) | ![](cpp_ios_dark/ios_first_responder.png) |

### 158. Ios Pan Gesture — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI includes a status bar and navigation header; the C++ port shows a different status bar layout and lacks the navigation header.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/ios_pan_gesture.png) | ![](cpp_ios_light/ios_pan_gesture.png) | ![](csharp_ios_dark/ios_pan_gesture.png) | ![](cpp_ios_dark/ios_pan_gesture.png) |

### 159. Ios Safe Area — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The content layout, text, and colors are identical once the harness wrapper differences are accounted for._

**🧩 MAUI quirks (discuss):**
- MAUI renders the page inside a card with significant top and side margins; the C++ port renders full-screen.
- MAUI status bar and navigation area are part of the harness; the C++ port shows native iOS status bar.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/ios_safe_area.png) | ![](cpp_ios_light/ios_safe_area.png) | ![](csharp_ios_dark/ios_safe_area.png) | ![](cpp_ios_dark/ios_safe_area.png) |

### 160. Ios Swipe Transition — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The font size for the labels and text is noticeably smaller in the C++ port.
- The internal spacing between the 'SwipeTransitionMode:' label and the 'Reveal'/'Drag' buttons is inconsistent with MAUI.
- The 'Swipe right' box has different dimensions and internal text alignment compared to the MAUI reference.
- The vertical spacing between the text blocks is compressed in the C++ port.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a container with large margins and top-inset, while the C++ port renders closer to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/ios_swipe_transition.png) | ![](cpp_ios_light/ios_swipe_transition.png) | ![](csharp_ios_dark/ios_swipe_transition.png) | ![](cpp_ios_dark/ios_swipe_transition.png) |

### 161. Ios Blur Effect — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port displays an image of a dog at the top of the page which is completely absent in the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/bottom/side padding; the C++ port renders the page content directly to the screen bounds.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/ios_blur_effect.png) | ![](cpp_ios_light/ios_blur_effect.png) | ![](csharp_ios_dark/ios_blur_effect.png) | ![](cpp_ios_dark/ios_blur_effect.png) |

### 162. Navigation Gallery — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side margins; the C++ port renders the page full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/navigation_gallery.png) | ![](cpp_ios_light/navigation_gallery.png) | ![](csharp_ios_dark/navigation_gallery.png) | ![](cpp_ios_dark/navigation_gallery.png) |

### 163. Modal — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Vertical spacing between the text labels and buttons is slightly inconsistent with the MAUI reference.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/bottom/side margins; the C++ port renders the page full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/modal.png) | ![](cpp_ios_light/modal.png) | ![](csharp_ios_dark/modal.png) | ![](cpp_ios_dark/modal.png) |

### 164. Tabbed Flyout — ⬛ L:cpp_blank · ⬛ D:cpp_blank  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port fails to render any content, resulting in a blank screen.

**🧩 MAUI quirks (discuss):**
- MAUI renders the page inside a card with significant top and side padding, while the C++ port is full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/tabbed_flyout.png) | ![](cpp_ios_light/tabbed_flyout.png) | ![](csharp_ios_dark/tabbed_flyout.png) | ![](cpp_ios_dark/tabbed_flyout.png) |

### 165. Toolbar — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The layout and content are identical once the harness-wrapper padding differences are accounted for._

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/toolbar.png) | ![](cpp_ios_light/toolbar.png) | ![](csharp_ios_dark/toolbar.png) | ![](cpp_ios_dark/toolbar.png) |

### 166. Menu Bar — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI wraps content in a card with significant top/bottom/side padding; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/menu_bar.png) | ![](cpp_ios_light/menu_bar.png) | ![](csharp_ios_dark/menu_bar.png) | ![](cpp_ios_dark/menu_bar.png) |

### 167. Title Bar — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port uses a thinner font weight for labels compared to the bold/medium weight in MAUI.
- The C++ port's input fields and checkboxes have smaller heights and different internal padding than the MAUI reference.
- The C++ port's layout lacks the consistent vertical spacing between the 'Content Options' and 'Color Options' sections seen in MAUI.
- The C++ port's checkbox alignment and sizing relative to the text labels are inconsistent with the MAUI implementation.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant outer padding and top/bottom cropping; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/title_bar.png) | ![](cpp_ios_light/title_bar.png) | ![](csharp_ios_dark/title_bar.png) | ![](cpp_ios_dark/title_bar.png) |

### 168. Chrome — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI renders content within a card with significant top/side padding; C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/chrome.png) | ![](cpp_ios_light/chrome.png) | ![](csharp_ios_dark/chrome.png) | ![](cpp_ios_dark/chrome.png) |

### 169. Context Flyout — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- C++ port includes extraneous Bing search bar, image/video/text creator buttons, and footer content not present in the MAUI source.
- C++ port includes an extraneous cookie consent modal in the dark theme.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a gray container card with significant top/bottom/side margins; the C++ port renders the page full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/context_flyout.png) | ![](cpp_ios_light/context_flyout.png) | ![](csharp_ios_dark/context_flyout.png) | ![](cpp_ios_dark/context_flyout.png) |

### 170. Templated View — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The vertical spacing between the individual card items is slightly smaller in the C++ port than in the MAUI reference.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a container card with significant outer margins and top/bottom cropping; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/templated_view.png) | ![](cpp_ios_light/templated_view.png) | ![](csharp_ios_dark/templated_view.png) | ![](cpp_ios_dark/templated_view.png) |

### 171. Custom Layout — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI uses a large top-inset for the navigation bar and status bar area, while the C++ port renders the content closer to the top edge.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/custom_layout.png) | ![](cpp_ios_light/custom_layout.png) | ![](csharp_ios_dark/custom_layout.png) | ![](cpp_ios_dark/custom_layout.png) |

### 172. Visual States — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The first Entry control in the C++ port lacks the green background color present in MAUI.
- The C++ port Entry controls have different border styling and internal padding compared to MAUI.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port uses full-width layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/visual_states.png) | ![](cpp_ios_light/visual_states.png) | ![](csharp_ios_dark/visual_states.png) | ![](cpp_ios_dark/visual_states.png) |
