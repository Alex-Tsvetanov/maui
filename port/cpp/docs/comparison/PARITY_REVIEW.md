# C++ port vs .NET MAUI — parity REVIEW (Gemini-judged, for verification)

> **Not authoritative.** Independent Google-Gemini vision pass. These verdicts are NOT in the tracked board (`parity_status.json` / `README.md`) — verify them here first.
>
> Each page's differences are split into two buckets: **🛠 Port diffs** (genuine C++ issues to fix) and **🧩 MAUI quirks** (MAUI-side imperfections — *subject to your ruling*; they do NOT drive the verdict). Rule on the quirk categories below; rulings get recorded in the comparison policy so the loop stops re-litigating them.

**Pages judged: 170** · 109 with port diffs · 197 MAUI-quirk notes across 4 categories.

_Judged by: gemini-3.1-flash-lite ×140, gemini-3.5-flash ×20, gemini-2.5-flash ×10. Full-flash verdicts are more reliable than flash-lite; weight your review accordingly._

**Deferred to Claude fallback (2):** containers, styles

## MAUI imperfection categories seen — RULE ON EACH

For each category decide: **ignore** (MAUI imperfection, port need not match) · **match** (port should replicate it) · **case-by-case**. Rulings → comparison policy in `port/CLAUDE.md` / `port/PROJECT.md`.

### Whole-screen padding / margins  (163 page(s)) — _ruling: TBD_
- **Label**: MAUI insets the page inside a card container, while the C++ port is full screen.
- **Button**: MAUI insets the page inside a container card and crops the bottom, while the C++ port shows the full page including extra buttons at the bottom.
- **Entry**: MAUI insets the page inside a card with large margins and crops the bottom content (Slider and 'Cursor' Entry); C++ port displays full screen and shows this extra content.
- **Editor**: MAUI insets the page inside a rounded card container with top/bottom cropping; C++ port is full-screen.
- **Date Picker**: MAUI insets the page inside a card container with top/bottom cropping, while the C++ port displays full screen showing more controls at the bottom.
- **Time Picker**: MAUI insets the page and crops the bottom, hiding several controls that are fully visible in the C++ port.
- **Pickers**: MAUI insets the page inside a card with a black background at the top; C++ port is full-screen.
- **Slider**: MAUI insets whole page; C++ port is full screen and shows more items at the bottom.
- **Stepper**: MAUI insets the page inside a card container, cropping the bottom content (ValueChanged stepper and Value label).
- **Switch**: MAUI insets the page inside a card container with large margins, while the C++ port is full-screen.
- **Check Box**: MAUI insets the page content inside a rounded card container, while the C++ port fills the screen.
- **Progress Bar**: MAUI insets the page content inside a rounded card container, while the C++ port is full-screen.
- **Activity Indicator**: MAUI insets the page inside a card container, cropping the bottom content ('Not Running' and '- End of page -').
- **Indicator**: MAUI insets the page inside a card container with rounded corners, while C++ port fills the screen
- **Image**: MAUI insets the page inside a card, cropping the bottom content (Font Image Source icons and Animating a gif switch); C++ port shows the full page.
- **Image Button**: MAUI insets the page inside a card container, whereas the C++ port is full-screen
- **Image Button**: MAUI crops the page at the bottom, hiding several controls (sliders, CornerRadius, Custom Size, Padding) that are visible in the C++ port
- **Box View**: MAUI insets the page inside a card container, cropping the bottom content so the gradient is cut off and the CornerRadius element is missing
- **Content View**: MAUI insets the page inside a card container with a black background border, while the C++ port is full screen
- **Control stack**: MAUI insets the whole page inside a harness card with large padding; port uses less outer padding.
- **Input Controls**: MAUI insets whole page ~60px; port uses ~16px
- **Fonts**: MAUI insets the whole page significantly from the screen edges; the C++ port uses less outer padding.
- **Fonts**: The C++ port shows the device notch, which is not visible in MAUI due to its larger outer padding.
- **Formatted Text**: MAUI insets whole page ~60px; port uses ~16px
- **Triggers**: MAUI has larger top padding/inset, pushing content down; port has less.
- **Triggers**: MAUI has a larger overall page inset from screen edges.
- **Behaviors**: MAUI insets the whole page inside a harness card with large outer padding; port uses less outer padding.
- **Semantics**: MAUI insets the whole page inside a card with large padding; port uses less outer padding.
- **App Theme Binding**: MAUI insets the entire page content within a white card, leaving large margins to the screen edges; the C++ port uses less outer padding.
- **Stack Layout**: MAUI insets the whole page within a card with rounded corners; the port uses a full-screen background.
- **Vertical Stack**: MAUI insets whole page significantly; port uses less outer padding.
- **Horizontal Stack**: MAUI insets the entire page within a container card; the C++ port renders full-screen.
- **Grid**: MAUI insets the entire page inside a container with significant padding and rounded corners, while the C++ port renders the grid flush to the screen edges.
- **Absolute Layout**: MAUI renders the page inside a gray container card with significant outer padding and top/bottom cropping; the C++ port renders full-screen.
- **Flex Layout**: MAUI wraps the page in a card with significant top/bottom/side margins; the C++ port renders full-screen.
- **Relative Layout**: MAUI renders the page inside a container with significant top and side padding; the C++ port renders full-screen.
- **Layout alignment (Start/Center/End/Fill)**: MAUI insets the entire page content within a card with significant padding, while the C++ port renders the content flush to the screen edges.
- **Z Index**: MAUI uses a large outer container card with significant padding and top/bottom cropping; the C++ port renders the page content directly with minimal padding.
- **Layout Is Enabled**: MAUI wraps the page in a card with significant top and side padding; the C++ port uses a full-screen layout.
- **Shapes**: MAUI insets the entire page within a card container with significant padding, while the C++ port renders flush to the screen edges.
- … and 123 more

### Top/bottom cropping  (16 page(s)) — _ruling: TBD_
- **Label**: MAUI crops the bottom of the page, hiding several elements that are visible in the C++ port.
- **Search Bar**: MAUI crops the bottom of the page, hiding several search bars that are visible in the C++ port
- **Picker**: MAUI crops the bottom of the page, hiding several picker test cases and buttons that are visible in the C++ port
- **Stepper**: MAUI has a top status bar area with different background/notch presentation.
- **Indicator**: MAUI's card container crops the top and bottom of the page
- **Control stack**: MAUI's harness card crops the page top/bottom; port shows more of the page.
- **Input Controls**: MAUI crops the top of the page due to the harness card
- **Fonts**: MAUI shows 'maui_ios_gallery' in the top left navigation bar, which is part of the harness.
- **Semantics**: MAUI's card crops the bottom of the page, hiding additional content (HeadingLevel labels, StackLayout labels, 'Click to set semantic focus' button).
- **Semantics**: MAUI's card crops the rightmost 'x' button next to the search bar.
- **App Theme Binding**: MAUI's card is shorter than the screen, cropping the top/bottom slightly.
- **Clipping**: MAUI clips content at the top and bottom due to the container card; the C++ port shows more vertical content.
- **Items**: MAUI includes a navigation bar and status bar area; the C++ port uses a different top-level layout structure.
- **Preselected Item**: MAUI clips the bottom of the list; the C++ port shows more list items.
- **Footer Only String**: MAUI includes a top status bar and navigation header; the C++ port shows only the page content
- **Navigation Gallery**: MAUI status bar and top-level navigation chrome are present; the C++ port uses a different status bar style.

### Harness chrome (card / nav / status bar)  (17 page(s)) — _ruling: TBD_
- **Image**: Harness chrome (status bar, navigation bar/notch area) differs between MAUI and C++ port.
- **Box View**: Harness chrome including status bar, notch, and card background
- **Input Controls**: MAUI includes a navigation bar; port does not
- **Formatted Text**: MAUI shows harness navigation bar
- **Triggers**: MAUI includes a navigation bar with 'maui_ios_gallery' and back arrow; port does not.
- **Behaviors**: MAUI's status bar shows '18:42' and 'maui_ios_gallery'; port shows '23:06' and a black notch.
- **Semantics**: MAUI shows iOS status bar/notch; port shows a black bar for the notch area.
- **Stack Layout**: MAUI status bar shows '3:18' and standard icons; C++ port status bar shows '23:06' and a dynamic island.
- **Vertical Stack**: Status bar differences (time, dynamic island).
- **Update Path Data**: MAUI uses a navigation bar and status bar area, while the C++ port shows a full-screen view with a different status bar layout.
- **Empty View Null**: MAUI shows status bar clock and battery, while the C++ port shows the dynamic island area.
- **Empty View Load Simulate**: MAUI shows status bar clock and battery; C++ port shows dynamic island and status bar icons
- **Radio Template From Style**: MAUI harness renders a blank screen while the C++ port renders the actual page content.
- **Ios Pan Gesture**: MAUI includes a navigation bar and status bar area; the C++ port shows the page content directly.
- **Ios Safe Area**: MAUI status bar and navigation area are part of the harness; the C++ port shows the native system status bar.
- **Modal**: MAUI displays status bar and notch area, while the C++ port shows a different status bar layout and dynamic island area.
- **Menu Bar**: MAUI includes a status bar and navigation header; the C++ port shows a different status bar layout and lacks the navigation header.

### Other / uncategorised  (1 page(s)) — _ruling: TBD_
- **Radio Button Group Binding**: MAUI reference is blank in both light and dark modes.

## Per-page review

### 1. Label — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.5-flash</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI insets the page inside a card container, while the C++ port is full screen.
- MAUI crops the bottom of the page, hiding several elements that are visible in the C++ port.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/label.png) | ![](cpp_ios_light/label.png) | ![](csharp_ios_dark/label.png) | ![](cpp_ios_dark/label.png) |

### 2. Button — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.5-flash</sub>
**🛠 Port diffs (fix):**
- The 'CornerRadius' button has sharp corners in the C++ port but is rounded in MAUI.
- The pink 'Button' has wide character spacing ('B u t t o n') in the C++ port but normal spacing in MAUI.

**🧩 MAUI quirks (discuss):**
- MAUI insets the page inside a container card and crops the bottom, while the C++ port shows the full page including extra buttons at the bottom.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/button.png) | ![](cpp_ios_light/button.png) | ![](csharp_ios_dark/button.png) | ![](cpp_ios_dark/button.png) |

### 3. Entry — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.5-flash</sub>
**🛠 Port diffs (fix):**
- Entry controls in C++ port have smaller height than in MAUI.
- Vertical spacing between controls is tighter in C++ port than in MAUI.
- Entry borders in C++ port (dark) are much darker and less visible than in MAUI (dark).

**🧩 MAUI quirks (discuss):**
- MAUI insets the page inside a card with large margins and crops the bottom content (Slider and 'Cursor' Entry); C++ port displays full screen and shows this extra content.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/entry.png) | ![](cpp_ios_light/entry.png) | ![](csharp_ios_dark/entry.png) | ![](cpp_ios_dark/entry.png) |

### 4. Editor — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.5-flash</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI insets the page inside a rounded card container with top/bottom cropping; C++ port is full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/editor.png) | ![](cpp_ios_light/editor.png) | ![](csharp_ios_dark/editor.png) | ![](cpp_ios_dark/editor.png) |

### 5. Search Bar — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.5-flash</sub>
**🛠 Port diffs (fix):**
- Fourth search bar text and coloring differs ('Italic 24' with gray '24' in MAUI vs 'Italic 24pt' in C++)

**🧩 MAUI quirks (discuss):**
- MAUI crops the bottom of the page, hiding several search bars that are visible in the C++ port

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/search_bar.png) | ![](cpp_ios_light/search_bar.png) | ![](csharp_ios_dark/search_bar.png) | ![](cpp_ios_dark/search_bar.png) |

### 6. Picker — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.5-flash</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI crops the bottom of the page, hiding several picker test cases and buttons that are visible in the C++ port

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/picker.png) | ![](cpp_ios_light/picker.png) | ![](csharp_ios_dark/picker.png) | ![](cpp_ios_dark/picker.png) |

### 7. Date Picker — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.5-flash</sub>
**🛠 Port diffs (fix):**
- The second Background field displays a solid magenta background in the C++ port instead of the blue-to-teal gradient shown in MAUI.
- Dates in several fields differ (e.g., 19.06.2026 in MAUI vs 22.06.2026 in C++ port).

**🧩 MAUI quirks (discuss):**
- MAUI insets the page inside a card container with top/bottom cropping, while the C++ port displays full screen showing more controls at the bottom.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/date_picker.png) | ![](cpp_ios_light/date_picker.png) | ![](csharp_ios_dark/date_picker.png) | ![](cpp_ios_dark/date_picker.png) |

### 8. Time Picker — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.5-flash</sub>
**🛠 Port diffs (fix):**
- The second 'Background' TimePicker displays a solid pink/magenta background in the C++ port instead of the blue-to-cyan gradient shown in MAUI.

**🧩 MAUI quirks (discuss):**
- MAUI insets the page and crops the bottom, hiding several controls that are fully visible in the C++ port.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/time_picker.png) | ![](cpp_ios_light/time_picker.png) | ![](csharp_ios_dark/time_picker.png) | ![](cpp_ios_dark/time_picker.png) |

### 9. Pickers — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.5-flash</sub>
**🛠 Port diffs (fix):**
- The date picker displays a different date (19.06.2026 in MAUI vs 22.06.2026 in C++), which also updates the label text.

**🧩 MAUI quirks (discuss):**
- MAUI insets the page inside a card with a black background at the top; C++ port is full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/pickers.png) | ![](cpp_ios_light/pickers.png) | ![](csharp_ios_dark/pickers.png) | ![](cpp_ios_dark/pickers.png) |

### 10. Slider — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.5-flash</sub>
**🛠 Port diffs (fix):**
- Header labels in C++ port are smaller and have a lighter font weight than MAUI's bold headers.
- The 'Background' slider in C++ port is missing its thumb.
- Vertical spacing between labels and sliders is tighter in the C++ port.

**🧩 MAUI quirks (discuss):**
- MAUI insets whole page; C++ port is full screen and shows more items at the bottom.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/slider.png) | ![](cpp_ios_light/slider.png) | ![](csharp_ios_dark/slider.png) | ![](cpp_ios_dark/slider.png) |

### 11. Stepper — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.5-flash</sub>
**🛠 Port diffs (fix):**
- BackgroundColor red background spans full width in MAUI, but is restricted to the Stepper control in the C++ port.
- Label fonts in MAUI are larger and bolder than in the C++ port.

**🧩 MAUI quirks (discuss):**
- MAUI insets the page inside a card container, cropping the bottom content (ValueChanged stepper and Value label).
- MAUI has a top status bar area with different background/notch presentation.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/stepper.png) | ![](cpp_ios_light/stepper.png) | ![](csharp_ios_dark/stepper.png) | ![](cpp_ios_dark/stepper.png) |

### 12. Switch — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.5-flash</sub>
**🛠 Port diffs (fix):**
- Header labels ('Default', 'BackgroundColor', etc.) have smaller font size and lighter font weight in C++ port.
- The 'Background' switch is missing its thumb entirely in the C++ port.
- The 'ThumbColor' switch has an orange thumb in the C++ port, whereas it is white in MAUI.
- Vertical spacing between elements is tighter in the C++ port.

**🧩 MAUI quirks (discuss):**
- MAUI insets the page inside a card container with large margins, while the C++ port is full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/switch.png) | ![](cpp_ios_light/switch.png) | ![](csharp_ios_dark/switch.png) | ![](cpp_ios_dark/switch.png) |

### 13. Check Box — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.5-flash</sub>
**🛠 Port diffs (fix):**
- Labels ('Default', 'Colored', etc.) in C++ port are rendered with regular font weight and smaller font size compared to MAUI's bold, larger text.

**🧩 MAUI quirks (discuss):**
- MAUI insets the page content inside a rounded card container, while the C++ port fills the screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/check_box.png) | ![](cpp_ios_light/check_box.png) | ![](csharp_ios_dark/check_box.png) | ![](cpp_ios_dark/check_box.png) |

### 14. Progress Bar — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.5-flash</sub>
**🛠 Port diffs (fix):**
- Label text ('Default', 'ProgressColor', etc.) is smaller and regular weight in the C++ port, whereas MAUI uses larger, bold fonts.

**🧩 MAUI quirks (discuss):**
- MAUI insets the page content inside a rounded card container, while the C++ port is full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/progress_bar.png) | ![](cpp_ios_light/progress_bar.png) | ![](csharp_ios_dark/progress_bar.png) | ![](cpp_ios_dark/progress_bar.png) |

### 15. Activity Indicator — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.5-flash</sub>
**🛠 Port diffs (fix):**
- Label fonts in C++ port are smaller and regular weight, whereas MAUI uses larger, bold fonts.

**🧩 MAUI quirks (discuss):**
- MAUI insets the page inside a card container, cropping the bottom content ('Not Running' and '- End of page -').

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/activity_indicator.png) | ![](cpp_ios_light/activity_indicator.png) | ![](csharp_ios_dark/activity_indicator.png) | ![](cpp_ios_dark/activity_indicator.png) |

### 16. Indicator — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.5-flash</sub>
**🛠 Port diffs (fix):**
- Indicator Size row has much larger indicator dots in C++ port than in MAUI
- CarouselView 'Item 1' and its indicators are pushed to the bottom of the screen in C++ port, creating a large vertical gap
- Text wrapping and content mismatch for 'Indicator MaximumVisible - 7 of 10' (MAUI cuts off '10' and wraps differently)

**🧩 MAUI quirks (discuss):**
- MAUI insets the page inside a card container with rounded corners, while C++ port fills the screen
- MAUI's card container crops the top and bottom of the page

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/indicator.png) | ![](cpp_ios_light/indicator.png) | ![](csharp_ios_dark/indicator.png) | ![](cpp_ios_dark/indicator.png) |

### 17. Image — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.5-flash</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI insets the page inside a card, cropping the bottom content (Font Image Source icons and Animating a gif switch); C++ port shows the full page.
- Harness chrome (status bar, navigation bar/notch area) differs between MAUI and C++ port.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/image.png) | ![](cpp_ios_light/image.png) | ![](csharp_ios_dark/image.png) | ![](cpp_ios_dark/image.png) |

### 18. Image Button — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.5-flash</sub>
**🛠 Port diffs (fix):** _none — only MAUI quirks_

**🧩 MAUI quirks (discuss):**
- MAUI insets the page inside a card container, whereas the C++ port is full-screen
- MAUI crops the page at the bottom, hiding several controls (sliders, CornerRadius, Custom Size, Padding) that are visible in the C++ port

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/image_button.png) | ![](cpp_ios_light/image_button.png) | ![](csharp_ios_dark/image_button.png) | ![](cpp_ios_dark/image_button.png) |

### 19. Box View — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.5-flash</sub>
**🛠 Port diffs (fix):** _none — only MAUI quirks_

**🧩 MAUI quirks (discuss):**
- MAUI insets the page inside a card container, cropping the bottom content so the gradient is cut off and the CornerRadius element is missing
- Harness chrome including status bar, notch, and card background

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/box_view.png) | ![](cpp_ios_light/box_view.png) | ![](csharp_ios_dark/box_view.png) | ![](cpp_ios_dark/box_view.png) |

### 20. Content View — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.5-flash</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI insets the page inside a card container with a black background border, while the C++ port is full screen

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/content_view.png) | ![](cpp_ios_light/content_view.png) | ![](csharp_ios_dark/content_view.png) | ![](cpp_ios_dark/content_view.png) |

### 21. Control stack — 🟢 L:match · 🟢 D:match  <sub>· gemini-2.5-flash</sub>
**🛠 Port diffs (fix):** _none — Identical, only MAUI harness quirks._

**🧩 MAUI quirks (discuss):**
- MAUI insets the whole page inside a harness card with large padding; port uses less outer padding.
- MAUI's harness card crops the page top/bottom; port shows more of the page.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/controls_stack.png) | ![](cpp_ios_light/controls_stack.png) | ![](csharp_ios_dark/controls_stack.png) | ![](cpp_ios_dark/controls_stack.png) |

### 22. Input Controls — 🟢 L:match · 🟢 D:match  <sub>· gemini-2.5-flash</sub>
**🛠 Port diffs (fix):** _none — Only MAUI harness wrapper differences._

**🧩 MAUI quirks (discuss):**
- MAUI insets whole page ~60px; port uses ~16px
- MAUI includes a navigation bar; port does not
- MAUI crops the top of the page due to the harness card

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/input_controls.png) | ![](cpp_ios_light/input_controls.png) | ![](csharp_ios_dark/input_controls.png) | ![](cpp_ios_dark/input_controls.png) |

### 23. Fonts — 🟢 L:match · 🟢 D:match  <sub>· gemini-2.5-flash</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI insets the whole page significantly from the screen edges; the C++ port uses less outer padding.
- MAUI shows 'maui_ios_gallery' in the top left navigation bar, which is part of the harness.
- The C++ port shows the device notch, which is not visible in MAUI due to its larger outer padding.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/fonts.png) | ![](cpp_ios_light/fonts.png) | ![](csharp_ios_dark/fonts.png) | ![](cpp_ios_dark/fonts.png) |

### 24. Formatted Text — 🟢 L:match · 🟢 D:match  <sub>· gemini-2.5-flash</sub>
**🛠 Port diffs (fix):** _none — Identical content and layout once MAUI harness quirks are set aside._

**🧩 MAUI quirks (discuss):**
- MAUI insets whole page ~60px; port uses ~16px
- MAUI shows harness navigation bar

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/formatted_text.png) | ![](cpp_ios_light/formatted_text.png) | ![](csharp_ios_dark/formatted_text.png) | ![](cpp_ios_dark/formatted_text.png) |

### 25. Triggers — 🟢 L:match · 🟢 D:match  <sub>· gemini-2.5-flash</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI includes a navigation bar with 'maui_ios_gallery' and back arrow; port does not.
- MAUI has larger top padding/inset, pushing content down; port has less.
- MAUI has a larger overall page inset from screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/triggers.png) | ![](cpp_ios_light/triggers.png) | ![](csharp_ios_dark/triggers.png) | ![](cpp_ios_dark/triggers.png) |

### 26. Behaviors — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-2.5-flash</sub>
**🛠 Port diffs (fix):**
- Light theme: Main text 'Red when the number isn't valid' has different font size/weight and vertical position.
- Dark theme: Main text 'Red when the number isn't valid' has different font size/weight and vertical position.
- Dark theme: Input field border color/prominence differs.

**🧩 MAUI quirks (discuss):**
- MAUI insets the whole page inside a harness card with large outer padding; port uses less outer padding.
- MAUI's status bar shows '18:42' and 'maui_ios_gallery'; port shows '23:06' and a black notch.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/behaviors.png) | ![](cpp_ios_light/behaviors.png) | ![](csharp_ios_dark/behaviors.png) | ![](cpp_ios_dark/behaviors.png) |

### 27. Semantics — 🟢 L:match · 🟢 D:match  <sub>· gemini-2.5-flash</sub>
**🛠 Port diffs (fix):** _none — identical, only MAUI harness wrapper differences_

**🧩 MAUI quirks (discuss):**
- MAUI insets the whole page inside a card with large padding; port uses less outer padding.
- MAUI's card crops the bottom of the page, hiding additional content (HeadingLevel labels, StackLayout labels, 'Click to set semantic focus' button).
- MAUI's card crops the rightmost 'x' button next to the search bar.
- MAUI shows iOS status bar/notch; port shows a black bar for the notch area.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/semantics.png) | ![](cpp_ios_light/semantics.png) | ![](csharp_ios_dark/semantics.png) | ![](cpp_ios_dark/semantics.png) |

### 28. App Theme Binding — 🟢 L:match · 🟢 D:match  <sub>· gemini-2.5-flash</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI insets the entire page content within a white card, leaving large margins to the screen edges; the C++ port uses less outer padding.
- MAUI's card is shorter than the screen, cropping the top/bottom slightly.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/app_theme_binding.png) | ![](cpp_ios_light/app_theme_binding.png) | ![](csharp_ios_dark/app_theme_binding.png) | ![](cpp_ios_dark/app_theme_binding.png) |

### 29. Stack Layout — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-2.5-flash</sub>
**🛠 Port diffs (fix):**
- The C++ port renders the colored rectangles in both vertical and horizontal stacks without any spacing between them, whereas MAUI renders them with a small gap.

**🧩 MAUI quirks (discuss):**
- MAUI insets the whole page within a card with rounded corners; the port uses a full-screen background.
- MAUI status bar shows '3:18' and standard icons; C++ port status bar shows '23:06' and a dynamic island.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/stack_layout.png) | ![](cpp_ios_light/stack_layout.png) | ![](csharp_ios_dark/stack_layout.png) | ![](cpp_ios_dark/stack_layout.png) |

### 30. Vertical Stack — 🟢 L:match · 🟢 D:match  <sub>· gemini-2.5-flash</sub>
**🛠 Port diffs (fix):** _none — Identical content and internal layout; only MAUI harness wrapper differences._

**🧩 MAUI quirks (discuss):**
- MAUI insets whole page significantly; port uses less outer padding.
- Status bar differences (time, dynamic island).

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/vertical_stack.png) | ![](cpp_ios_light/vertical_stack.png) | ![](csharp_ios_dark/vertical_stack.png) | ![](cpp_ios_dark/vertical_stack.png) |

### 31. Horizontal Stack — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port displays six colored squares in the HorizontalStackLayout, whereas the MAUI version only displays four.

**🧩 MAUI quirks (discuss):**
- MAUI insets the entire page within a container card; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/horizontal_stack.png) | ![](cpp_ios_light/horizontal_stack.png) | ![](csharp_ios_dark/horizontal_stack.png) | ![](cpp_ios_dark/horizontal_stack.png) |

### 32. Grid — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI insets the entire page inside a container with significant padding and rounded corners, while the C++ port renders the grid flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/grid.png) | ![](cpp_ios_light/grid.png) | ![](csharp_ios_dark/grid.png) | ![](cpp_ios_dark/grid.png) |

### 33. Absolute Layout — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The vertical positioning of the top blue rectangle and bottom black rectangle is slightly offset in the C++ port compared to the MAUI reference.
- The internal spacing between the central text and the surrounding colored rectangles shows minor discrepancies in the C++ port.

**🧩 MAUI quirks (discuss):**
- MAUI renders the page inside a gray container card with significant outer padding and top/bottom cropping; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/absolute_layout.png) | ![](cpp_ios_light/absolute_layout.png) | ![](csharp_ios_dark/absolute_layout.png) | ![](cpp_ios_dark/absolute_layout.png) |

### 34. Flex Layout — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The header and footer heights and vertical positioning relative to the content area are slightly inconsistent with the MAUI reference.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/bottom/side margins; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/flex_layout.png) | ![](cpp_ios_light/flex_layout.png) | ![](csharp_ios_dark/flex_layout.png) | ![](cpp_ios_dark/flex_layout.png) |

### 35. Relative Layout — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The central gray rectangle has a different aspect ratio and width in the C++ port.

**🧩 MAUI quirks (discuss):**
- MAUI renders the page inside a container with significant top and side padding; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/relative_layout.png) | ![](cpp_ios_light/relative_layout.png) | ![](csharp_ios_dark/relative_layout.png) | ![](cpp_ios_dark/relative_layout.png) |

### 36. Layout alignment (Start/Center/End/Fill) — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI insets the entire page content within a card with significant padding, while the C++ port renders the content flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/alignment.png) | ![](cpp_ios_light/alignment.png) | ![](csharp_ios_dark/alignment.png) | ![](cpp_ios_dark/alignment.png) |

### 37. Z Index — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The vertical spacing between the stacked colored labels is slightly tighter in the C++ port than in the MAUI reference.

**🧩 MAUI quirks (discuss):**
- MAUI uses a large outer container card with significant padding and top/bottom cropping; the C++ port renders the page content directly with minimal padding.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/z_index.png) | ![](cpp_ios_light/z_index.png) | ![](csharp_ios_dark/z_index.png) | ![](cpp_ios_dark/z_index.png) |

### 38. Layout Is Enabled — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port renders extra content sections ('Children have commands attached' and 'Nested layouts') that are missing from the MAUI source.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port uses a full-screen layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/layout_is_enabled.png) | ![](cpp_ios_light/layout_is_enabled.png) | ![](csharp_ios_dark/layout_is_enabled.png) | ![](cpp_ios_dark/layout_is_enabled.png) |

### 39. Shapes — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The line control has a different length and vertical position in the C++ port compared to the MAUI reference.

**🧩 MAUI quirks (discuss):**
- MAUI insets the entire page within a card container with significant padding, while the C++ port renders flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/shapes.png) | ![](cpp_ios_light/shapes.png) | ![](csharp_ios_dark/shapes.png) | ![](cpp_ios_dark/shapes.png) |

### 40. Ellipse Gallery — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The shapes in the C++ port are consistently smaller in width and height than the MAUI originals.
- The vertical spacing between the text labels and the shapes is slightly tighter in the C++ port.

**🧩 MAUI quirks (discuss):**
- MAUI renders the page with a large top margin and status bar area, while the C++ port uses a different layout container.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/ellipse_gallery.png) | ![](cpp_ios_light/ellipse_gallery.png) | ![](csharp_ios_dark/ellipse_gallery.png) | ![](cpp_ios_dark/ellipse_gallery.png) |

### 41. Rectangle Gallery — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port renders an additional 'A Rectangle with curved corners' control at the bottom of the page that is missing in the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI uses a large outer container margin and status bar padding; the C++ port uses a different layout container with less padding.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/rectangle_gallery.png) | ![](cpp_ios_light/rectangle_gallery.png) | ![](csharp_ios_dark/rectangle_gallery.png) | ![](cpp_ios_dark/rectangle_gallery.png) |

### 42. Line Gallery — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The thick black line at the bottom has a different length and slightly different positioning in the C++ port.

**🧩 MAUI quirks (discuss):**
- MAUI uses a large top-margin/inset for the page content; the C++ port uses a smaller top-margin.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/line_gallery.png) | ![](cpp_ios_light/line_gallery.png) | ![](csharp_ios_dark/line_gallery.png) | ![](cpp_ios_dark/line_gallery.png) |

### 43. Line Join Gallery — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI insets the entire page content within a card container, while the C++ port renders the content flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/line_join_gallery.png) | ![](cpp_ios_light/line_join_gallery.png) | ![](csharp_ios_dark/line_join_gallery.png) | ![](cpp_ios_dark/line_join_gallery.png) |

### 44. Polygon Gallery — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- NonZero Polygon fill and stroke logic differs significantly from MAUI in both light and dark themes.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/polygon_gallery.png) | ![](cpp_ios_light/polygon_gallery.png) | ![](csharp_ios_dark/polygon_gallery.png) | ![](cpp_ios_dark/polygon_gallery.png) |

### 45. Polyline Gallery — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The dash polyline pattern is incorrectly rendered with too many segments and incorrect spacing.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with large margins and top/bottom cropping; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/polyline_gallery.png) | ![](cpp_ios_light/polyline_gallery.png) | ![](csharp_ios_dark/polyline_gallery.png) | ![](cpp_ios_dark/polyline_gallery.png) |

### 46. Path Gallery — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port is missing the 'Overlapping Rectangles' and 'EllipseGeometry' sections entirely.
- The 'Composite shape' in the C++ port is rendered as a full circle, whereas the MAUI version shows only a partial arc.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/path_gallery.png) | ![](cpp_ios_light/path_gallery.png) | ![](csharp_ios_dark/path_gallery.png) | ![](cpp_ios_dark/path_gallery.png) |

### 47. Path Aspect Gallery — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port is missing the gray background rectangles behind each image.
- The vertical spacing between the text labels and the images is inconsistent with MAUI.
- The image sizes and aspect ratios appear to differ from the MAUI implementation.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with large margins and top/bottom cropping; the C++ port uses a full-screen layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/path_aspect_gallery.png) | ![](cpp_ios_light/path_aspect_gallery.png) | ![](csharp_ios_dark/path_aspect_gallery.png) | ![](cpp_ios_dark/path_aspect_gallery.png) |

### 48. Path Transform String — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The rendering of the shapes and text is identical once the harness wrapper differences are accounted for._

**🧩 MAUI quirks (discuss):**
- MAUI includes a large top-margin harness wrapper that shifts content down, while the C++ port starts content near the top of the screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/path_transform_string.png) | ![](cpp_ios_light/path_transform_string.png) | ![](csharp_ios_dark/path_transform_string.png) | ![](cpp_ios_dark/path_transform_string.png) |

### 49. Composition Gallery — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The entire layout in the C++ port is scaled down, resulting in smaller shapes and different internal spacing compared to MAUI.
- The relative positioning of the shapes and lines is inconsistent with the MAUI reference due to the scaling issue.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with large margins and top/bottom cropping; the C++ port renders the page full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/composition_gallery.png) | ![](cpp_ios_light/composition_gallery.png) | ![](csharp_ios_dark/composition_gallery.png) | ![](cpp_ios_dark/composition_gallery.png) |

### 50. Transform Playground — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port displays additional controls (ScaleY, SkewTransform, SkewX) that are missing from the MAUI implementation.

**🧩 MAUI quirks (discuss):**
- MAUI renders the page inside a card with significant top and side padding; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/transform_playground.png) | ![](cpp_ios_light/transform_playground.png) | ![](csharp_ios_dark/transform_playground.png) | ![](cpp_ios_dark/transform_playground.png) |

### 51. Transformations — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- C++ port includes four additional controls (AnchorX, AnchorY, TranslationX, TranslationY) that are missing from the MAUI source.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/bottom and side margins; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/transformations.png) | ![](cpp_ios_light/transformations.png) | ![](csharp_ios_dark/transformations.png) | ![](cpp_ios_dark/transformations.png) |

### 52. Update Path Data — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI uses a navigation bar and status bar area, while the C++ port shows a full-screen view with a different status bar layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/update_path_data.png) | ![](cpp_ios_light/update_path_data.png) | ![](csharp_ios_dark/update_path_data.png) | ![](cpp_ios_dark/update_path_data.png) |

### 53. Auto Size Shapes — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The ellipse shape is incorrectly rendered as a circle in the C++ port.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a container with significant top/bottom padding and rounded corners, while the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/auto_size_shapes.png) | ![](cpp_ios_light/auto_size_shapes.png) | ![](csharp_ios_dark/auto_size_shapes.png) | ![](cpp_ios_dark/auto_size_shapes.png) |

### 54. Shape App Theme — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI renders content within a card with significant top and side padding; the C++ port renders content flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/shape_app_theme.png) | ![](cpp_ios_light/shape_app_theme.png) | ![](csharp_ios_dark/shape_app_theme.png) | ![](cpp_ios_dark/shape_app_theme.png) |

### 55. Invalidate Brush — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The vertical spacing between the button and the text label is slightly inconsistent with the MAUI reference.

**🧩 MAUI quirks (discuss):**
- MAUI renders the page inside a card with significant top and side padding; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/invalidate_brush.png) | ![](cpp_ios_light/invalidate_brush.png) | ![](csharp_ios_dark/invalidate_brush.png) | ![](cpp_ios_dark/invalidate_brush.png) |

### 56. Gradient brushes — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI wraps content in a card with significant top/side padding; C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/gradient.png) | ![](cpp_ios_light/gradient.png) | ![](csharp_ios_dark/gradient.png) | ![](cpp_ios_dark/gradient.png) |

### 57. Border — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The bordered content container and text are identical in size, color, and placement._

**🧩 MAUI quirks (discuss):**
- MAUI renders the page within a card with significant top and side margins; the C++ port renders the page full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/border.png) | ![](cpp_ios_light/border.png) | ![](csharp_ios_dark/border.png) | ![](cpp_ios_dark/border.png) |

### 58. Border Stroke — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The layout and styling are identical once the harness-wrapper padding differences are accounted for._

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/bottom/side padding; the C++ port uses full-screen width with minimal padding.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/border_stroke.png) | ![](cpp_ios_light/border_stroke.png) | ![](csharp_ios_dark/border_stroke.png) | ![](cpp_ios_dark/border_stroke.png) |

### 59. Border Layout — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Slider track height is noticeably thinner in the C++ port.
- Slider thumb diameter is smaller in the C++ port.
- The rounded rectangle container for the colored bars has a different stroke thickness and corner radius in the C++ port.

**🧩 MAUI quirks (discuss):**
- MAUI renders the page inside a container with significant top and side margins; the C++ port renders flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/border_layout.png) | ![](cpp_ios_light/border_layout.png) | ![](csharp_ios_dark/border_layout.png) | ![](cpp_ios_dark/border_layout.png) |

### 60. Border Playground — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- C++ port is missing several UI sections (Background End Color, Content Background, etc.) that are present in MAUI.

**🧩 MAUI quirks (discuss):**
- MAUI uses a large top margin and rounded container card; the C++ port uses a full-screen layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/border_playground.png) | ![](cpp_ios_light/border_playground.png) | ![](csharp_ios_dark/border_playground.png) | ![](cpp_ios_dark/border_playground.png) |

### 61. Border Clip Playground — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port renders a dog image inside the border shape area instead of the red-outlined shape shown in MAUI.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a container with significant top and side padding; the C++ port uses a different layout container.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/border_clip_playground.png) | ![](cpp_ios_light/border_clip_playground.png) | ![](csharp_ios_dark/border_clip_playground.png) | ![](cpp_ios_dark/border_clip_playground.png) |

### 62. Border Resize Content — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- C++ port displays images inside the shapes instead of solid colors.
- C++ port includes additional UI controls (sliders for Font Size and Image Scale) not present in the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI uses a large outer container margin and top/bottom cropping; the C++ port uses full-screen layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/border_resize_content.png) | ![](cpp_ios_light/border_resize_content.png) | ![](csharp_ios_dark/border_resize_content.png) | ![](cpp_ios_dark/border_resize_content.png) |

### 63. Borderless — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI wraps content in a card with large top/side margins; port renders full-screen with minimal padding.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/borderless.png) | ![](cpp_ios_light/borderless.png) | ![](csharp_ios_dark/borderless.png) | ![](cpp_ios_dark/borderless.png) |

### 64. Clip — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port is missing two image clipping examples present in the MAUI source.
- The C++ port displays the images at a different scale compared to the MAUI source.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/clip.png) | ![](cpp_ios_light/clip.png) | ![](csharp_ios_dark/clip.png) | ![](cpp_ios_dark/clip.png) |

### 65. Clip Views — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The search bar background color does not match the MAUI reference in either light or dark mode.
- The search bar corner radius and internal padding appear inconsistent with the MAUI reference.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/side padding and rounded corners; the C++ port uses a full-width layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/clip_views.png) | ![](cpp_ios_light/clip_views.png) | ![](csharp_ios_dark/clip_views.png) | ![](cpp_ios_dark/clip_views.png) |

### 66. Clip Corner Radius — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port is missing the 'Bottom Right Corner' slider control.
- The image content is different (MAUI shows a purple robot, C++ port shows a dog).

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side margins; the C++ port uses a full-screen layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/clip_corner_radius.png) | ![](cpp_ios_light/clip_corner_radius.png) | ![](csharp_ios_dark/clip_corner_radius.png) | ![](cpp_ios_dark/clip_corner_radius.png) |

### 67. Clip Gallery — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- MAUI fails to render the actual images, showing only gray rectangles instead of the dog photos present in the C++ port.
- The C++ port includes an additional 'Clipped Image using RoundRectangleGeometry' section and a bottom label that are completely missing from the MAUI render.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant outer padding and top/bottom cropping; the C++ port renders the page content directly to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/clip_gallery.png) | ![](cpp_ios_light/clip_gallery.png) | ![](csharp_ios_dark/clip_gallery.png) | ![](cpp_ios_dark/clip_gallery.png) |

### 68. Clipping — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port renders two coffee cup icons at the bottom left which are completely absent in the MAUI version.
- The C++ port displays an extra number '1' at the start of the horizontal list which is not present in the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/bottom/side margins; the C++ port renders full-screen.
- MAUI clips content at the top and bottom due to the container card; the C++ port shows more vertical content.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/clipping.png) | ![](cpp_ios_light/clipping.png) | ![](csharp_ios_dark/clipping.png) | ![](cpp_ios_dark/clipping.png) |

### 69. Shadow Playground — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The layout and rendering are identical once the harness wrapper differences are accounted for._

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port uses a full-screen layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/shadow_playground.png) | ![](cpp_ios_light/shadow_playground.png) | ![](csharp_ios_dark/shadow_playground.png) | ![](cpp_ios_dark/shadow_playground.png) |

### 70. Invalidate Shadow Host — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical content and layout once harness wrapper differences are accounted for_

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/bottom/side margins; the C++ port renders the page full-screen

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/invalidate_shadow_host.png) | ![](cpp_ios_light/invalidate_shadow_host.png) | ![](csharp_ios_dark/invalidate_shadow_host.png) | ![](cpp_ios_dark/invalidate_shadow_host.png) |

### 71. CollectionView — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Slight variation in internal grid item spacing and alignment compared to the MAUI reference.

**🧩 MAUI quirks (discuss):**
- MAUI uses a large top margin and status bar area, while the C++ port uses a different header layout and padding.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/collectionview.png) | ![](cpp_ios_light/collectionview.png) | ![](csharp_ios_dark/collectionview.png) | ![](cpp_ios_dark/collectionview.png) |

### 72. Items — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The vertical spacing between the list items is slightly tighter in the C++ port than in the MAUI reference.

**🧩 MAUI quirks (discuss):**
- MAUI includes a navigation bar and status bar area; the C++ port uses a different top-level layout structure.
- MAUI has significant top and side padding around the content; the C++ port uses different screen-edge margins.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/items.png) | ![](cpp_ios_light/items.png) | ![](csharp_ios_dark/items.png) | ![](cpp_ios_dark/items.png) |

### 73. Single Bound Selection — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The content layout, text, and spacing are identical once the harness wrapper differences are accounted for._

**🧩 MAUI quirks (discuss):**
- MAUI uses a large top-margin/inset for the page content, while the C++ port renders content closer to the top of the screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/single_bound_selection.png) | ![](cpp_ios_light/single_bound_selection.png) | ![](csharp_ios_dark/single_bound_selection.png) | ![](cpp_ios_dark/single_bound_selection.png) |

### 74. Multiple Bound Selection — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Selection highlight background color is lighter in the C++ port than in MAUI.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/side padding; the C++ port uses full-screen layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/multiple_bound_selection.png) | ![](cpp_ios_light/multiple_bound_selection.png) | ![](csharp_ios_dark/multiple_bound_selection.png) | ![](cpp_ios_dark/multiple_bound_selection.png) |

### 75. Preselected Item — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The vertical spacing between list items is tighter in the C++ port, allowing more items to fit on screen.
- The status bar area in the C++ port has a different background container/padding compared to the MAUI harness.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/side margins; the C++ port uses the full screen width.
- MAUI clips the bottom of the list; the C++ port shows more list items.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/preselected_item.png) | ![](cpp_ios_light/preselected_item.png) | ![](csharp_ios_dark/preselected_item.png) | ![](cpp_ios_dark/preselected_item.png) |

### 76. Preselected Items — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port renders smaller collection view items compared to MAUI.
- The C++ port has tighter internal spacing between collection view items.
- The C++ port displays more items per row and more rows overall due to the smaller item size.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the content in a card with large outer margins and top/bottom cropping; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/preselected_items.png) | ![](cpp_ios_light/preselected_items.png) | ![](csharp_ios_dark/preselected_items.png) | ![](cpp_ios_dark/preselected_items.png) |

### 77. Selection Command Param — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI renders content inside a card with significant top and side padding; the C++ port renders flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/selection_command_param.png) | ![](cpp_ios_light/selection_command_param.png) | ![](csharp_ios_dark/selection_command_param.png) | ![](cpp_ios_dark/selection_command_param.png) |

### 78. Selection Synchronization — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Selection highlight background color is lighter in the C++ port than in MAUI for both light and dark themes.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a container with significant top and side margins; the C++ port renders flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/selection_synchronization.png) | ![](cpp_ios_light/selection_synchronization.png) | ![](csharp_ios_dark/selection_synchronization.png) | ![](cpp_ios_dark/selection_synchronization.png) |

### 79. Filter Collection — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The list item height in the C++ port is smaller than in MAUI, causing more items to fit on the screen.
- The search bar height and internal padding appear slightly different between the two implementations.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side margins; the C++ port uses the full screen width.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/filter_collection.png) | ![](cpp_ios_light/filter_collection.png) | ![](csharp_ios_dark/filter_collection.png) | ![](cpp_ios_dark/filter_collection.png) |

### 80. Filter Selection — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port displays a much longer list of items than the MAUI version, suggesting incorrect item height or layout constraints.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with large margins and top/bottom cropping; the C++ port uses full-screen layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/filter_selection.png) | ![](cpp_ios_light/filter_selection.png) | ![](csharp_ios_dark/filter_selection.png) | ![](cpp_ios_dark/filter_selection.png) |

### 81. Header Footer — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The content layout, text, and spacing are identical once the harness wrapper differences are accounted for._

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side margins; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/header_footer.png) | ![](cpp_ios_light/header_footer.png) | ![](csharp_ios_dark/header_footer.png) | ![](cpp_ios_dark/header_footer.png) |

### 82. Header Footer Grid — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port displays a large image header and footer that do not exist in the MAUI source.
- The text 'This Is A Header' and 'This Is A Footer' have different font weights and colors compared to MAUI.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/bottom/side margins; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/header_footer_grid.png) | ![](cpp_ios_light/header_footer_grid.png) | ![](csharp_ios_dark/header_footer_grid.png) | ![](cpp_ios_dark/header_footer_grid.png) |

### 83. Header Footer Grid Horizontal — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port renders extra content rows that are not present in the MAUI version.
- The C++ port displays an image thumbnail in the top-left corner that is missing in the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI renders the page inside a card with significant top/side padding and cropping, while the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/header_footer_grid_horizontal.png) | ![](cpp_ios_light/header_footer_grid_horizontal.png) | ![](csharp_ios_dark/header_footer_grid_horizontal.png) | ![](cpp_ios_dark/header_footer_grid_horizontal.png) |

### 84. Header Footer Template — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port fails to render individual image containers with spacing, merging them into a single block.
- The C++ port displays the full date and time string in the header and footer, whereas MAUI only shows the date in the header and date/time in the footer.
- The C++ port header and footer text alignment and font weight differ from the MAUI reference.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/bottom/side padding; the C++ port uses the full screen width.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/header_footer_template.png) | ![](cpp_ios_light/header_footer_template.png) | ![](csharp_ios_dark/header_footer_template.png) | ![](cpp_ios_dark/header_footer_template.png) |

### 85. Header Footer View — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Header and footer backgrounds are images in the C++ port but solid colors in MAUI.
- Text color in the header and footer is white in the C++ port but a light beige/off-white in MAUI.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/bottom/side padding; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/header_footer_view.png) | ![](cpp_ios_light/header_footer_view.png) | ![](csharp_ios_dark/header_footer_view.png) | ![](cpp_ios_dark/header_footer_view.png) |

### 86. Footer Only String — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI includes a top status bar and navigation header; the C++ port shows only the page content
- MAUI has a slight top margin/padding for the list content; the C++ port starts at the very top of the screen

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/footer_only_string.png) | ![](cpp_ios_light/footer_only_string.png) | ![](csharp_ios_dark/footer_only_string.png) | ![](cpp_ios_dark/footer_only_string.png) |

### 87. Basic Grouping — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port renders additional list items (Yellowjacket and the entire 'Heroes for Hire' section) that are missing from the MAUI source.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the content in a card with significant top and side padding; the C++ port uses a full-screen layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/basic_grouping.png) | ![](cpp_ios_light/basic_grouping.png) | ![](csharp_ios_dark/basic_grouping.png) | ![](cpp_ios_dark/basic_grouping.png) |

### 88. Grid Grouping — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- C++ port adds 'This is a header' at the top.
- C++ port adds 'This is a footer.' at the bottom.
- C++ port adds an extra section 'Great Lakes Avengers' with members not present in the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI renders the page inside a card with significant top/bottom and side margins; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/grid_grouping.png) | ![](cpp_ios_light/grid_grouping.png) | ![](csharp_ios_dark/grid_grouping.png) | ![](cpp_ios_dark/grid_grouping.png) |

### 89. Grouping No Templates — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The content layout, font, and spacing are identical once the harness wrapper differences are accounted for._

**🧩 MAUI quirks (discuss):**
- MAUI uses a large outer container inset and top/bottom cropping; the C++ port uses full-screen layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/grouping_no_templates.png) | ![](cpp_ios_light/grouping_no_templates.png) | ![](csharp_ios_dark/grouping_no_templates.png) | ![](cpp_ios_dark/grouping_no_templates.png) |

### 90. Grouping Plus Selection — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port displays an additional 'Heroes for Hire' section and 'Total members: 5' label that are missing in the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI renders content within a card with significant top/bottom/side padding; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/grouping_plus_selection.png) | ![](cpp_ios_light/grouping_plus_selection.png) | ![](csharp_ios_dark/grouping_plus_selection.png) | ![](cpp_ios_dark/grouping_plus_selection.png) |

### 91. Switch Grouping — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port displays additional list groups (Defenders, Heroes for Hire) and items that are not present in the MAUI version.
- The C++ port list content extends beyond the visible area shown in the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a container with significant top and side margins; the C++ port renders flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/switch_grouping.png) | ![](cpp_ios_light/switch_grouping.png) | ![](csharp_ios_dark/switch_grouping.png) | ![](cpp_ios_dark/switch_grouping.png) |

### 92. Some Empty Groups — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI insets the entire page content within a card-like container with significant padding, while the C++ port renders the content flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/some_empty_groups.png) | ![](cpp_ios_light/some_empty_groups.png) | ![](csharp_ios_dark/some_empty_groups.png) | ![](cpp_ios_dark/some_empty_groups.png) |

### 93. Scroll To Group — 🔴 L:diff · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port renders extra list items (Doctor Druid, She-Hulk, Mockingbird, Fantastic Four group, and Defenders group) not present in the MAUI light mode capture.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port uses the full screen width.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/scroll_to_group.png) | ![](cpp_ios_light/scroll_to_group.png) | ![](csharp_ios_dark/scroll_to_group.png) | ![](cpp_ios_dark/scroll_to_group.png) |

### 94. Scroll Mode Test — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The list view in the C++ port renders more items on screen than the MAUI version, suggesting incorrect item height or spacing calculations.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side margins; the C++ port renders flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/scroll_mode_test.png) | ![](cpp_ios_light/scroll_mode_test.png) | ![](csharp_ios_dark/scroll_mode_test.png) | ![](cpp_ios_dark/scroll_mode_test.png) |

### 95. Adaptive Collection — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The port renders 8 items on screen while MAUI renders 7, indicating incorrect vertical spacing or item sizing.
- The port lacks the vertical spacing between list items present in the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side margins; the port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/adaptive_collection.png) | ![](cpp_ios_light/adaptive_collection.png) | ![](csharp_ios_dark/adaptive_collection.png) | ![](cpp_ios_dark/adaptive_collection.png) |

### 96. Staggered Layout — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port renders more items on screen than the MAUI version, suggesting the item size or spacing is not identical.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the content in a container with significant top and side margins; the C++ port renders closer to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/staggered_layout.png) | ![](cpp_ios_light/staggered_layout.png) | ![](csharp_ios_dark/staggered_layout.png) | ![](cpp_ios_dark/staggered_layout.png) |

### 97. Varied Size Selector — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port list contains extra items (Milk4, Coffee5) not present in the MAUI source.
- The C++ port list items have different vertical spacing and height compared to the MAUI source.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/bottom/side margins; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/varied_size_selector.png) | ![](cpp_ios_light/varied_size_selector.png) | ![](csharp_ios_dark/varied_size_selector.png) | ![](cpp_ios_dark/varied_size_selector.png) |

### 98. Nested Collection — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port renders many more list items than the MAUI version.
- The text color for the 'Source' headers and 'Caption' items is incorrect in the C++ port (black/blue vs red in MAUI).
- The layout of the items is vertical in the C++ port, whereas MAUI displays them in a horizontal scrolling collection.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the content in a gray card with significant padding and top/bottom cropping; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/nested_collection.png) | ![](cpp_ios_light/nested_collection.png) | ![](csharp_ios_dark/nested_collection.png) | ![](cpp_ios_dark/nested_collection.png) |

### 99. Data Template Selector — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The layout and content are identical once the harness-wrapper padding differences are accounted for._

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port renders the page edge-to-edge.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/data_template_selector.png) | ![](cpp_ios_light/data_template_selector.png) | ![](csharp_ios_dark/data_template_selector.png) | ![](cpp_ios_dark/data_template_selector.png) |

### 100. Cv Visual States — 🟢 L:match · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Dark theme list items are missing in the C++ port.

**🧩 MAUI quirks (discuss):**
- MAUI insets the entire page within a card container; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/cv_visual_states.png) | ![](cpp_ios_light/cv_visual_states.png) | ![](csharp_ios_dark/cv_visual_states.png) | ![](cpp_ios_dark/cv_visual_states.png) |

### 101. Empty View — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port renders more list items on screen than the MAUI version, suggesting the list item height or padding is smaller in the port.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a container with significant top and side margins; the C++ port uses the full screen width and height.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/empty_view.png) | ![](cpp_ios_light/empty_view.png) | ![](csharp_ios_dark/empty_view.png) | ![](cpp_ios_dark/empty_view.png) |

### 102. Empty View Null — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI shows status bar clock and battery, while the C++ port shows the dynamic island area.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/empty_view_null.png) | ![](cpp_ios_light/empty_view_null.png) | ![](csharp_ios_dark/empty_view_null.png) | ![](cpp_ios_dark/empty_view_null.png) |

### 103. Empty View Rtl — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port displays many more list items than the MAUI version, suggesting incorrect layout constraints or item count logic.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the content in a card with significant top/bottom and side margins; the C++ port renders closer to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/empty_view_rtl.png) | ![](cpp_ios_light/empty_view_rtl.png) | ![](csharp_ios_dark/empty_view_rtl.png) | ![](cpp_ios_dark/empty_view_rtl.png) |

### 104. Empty View Selector — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI insets the entire page content within a card container, while the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/empty_view_selector.png) | ![](cpp_ios_light/empty_view_selector.png) | ![](csharp_ios_dark/empty_view_selector.png) | ![](cpp_ios_dark/empty_view_selector.png) |

### 105. Empty View Swap — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port displays many more list items than the MAUI version, suggesting the scroll view or layout engine is not respecting the same bounds or item count.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with large top and side margins; the C++ port uses the full screen width.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/empty_view_swap.png) | ![](cpp_ios_light/empty_view_swap.png) | ![](csharp_ios_dark/empty_view_swap.png) | ![](cpp_ios_dark/empty_view_swap.png) |

### 106. Empty View Template — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port displays more items in the list than the MAUI version, suggesting incorrect item sizing or layout constraints.
- The search bar and list items appear to have different vertical spacing and density compared to the MAUI reference.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the content in a card with large outer margins and top/bottom cropping; the C++ port uses full-screen layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/empty_view_template.png) | ![](cpp_ios_light/empty_view_template.png) | ![](csharp_ios_dark/empty_view_template.png) | ![](cpp_ios_dark/empty_view_template.png) |

### 107. Empty View View — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port displays more list items than the MAUI version, suggesting incorrect item sizing or spacing calculations.
- The search bar and list item layout density differ significantly between the two implementations.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the content in a card with large outer margins and top/bottom cropping; the C++ port uses a full-screen layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/empty_view_view.png) | ![](cpp_ios_light/empty_view_view.png) | ![](csharp_ios_dark/empty_view_view.png) | ![](cpp_ios_dark/empty_view_view.png) |

### 108. Empty View Load Simulate — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI shows status bar clock and battery; C++ port shows dynamic island and status bar icons

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/empty_view_load_simulate.png) | ![](cpp_ios_light/empty_view_load_simulate.png) | ![](csharp_ios_dark/empty_view_load_simulate.png) | ![](cpp_ios_dark/empty_view_load_simulate.png) |

### 109. Carousel Page — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port includes additional UI elements (Prev/Next buttons and position status text) that are completely absent in the MAUI reference.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a container with significant top and side margins; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/carousel_page.png) | ![](cpp_ios_light/carousel_page.png) | ![](csharp_ios_dark/carousel_page.png) | ![](cpp_ios_dark/carousel_page.png) |

### 110. Chat Example — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Chat bubble corner radius is significantly different (more rounded in MAUI, less rounded in C++).
- Chat bubble background colors do not match the MAUI reference colors.
- The vertical spacing between the two chat bubbles is inconsistent with the MAUI reference.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/bottom/side padding; the C++ port renders the page full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/chat_example.png) | ![](cpp_ios_light/chat_example.png) | ![](csharp_ios_dark/chat_example.png) | ![](cpp_ios_dark/chat_example.png) |

### 111. Items Updating Scroll Mode — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The list item height or vertical spacing is smaller in the C++ port, causing more items to be visible in the same viewport area.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a container with significant top and side margins; the C++ port renders closer to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/items_updating_scroll_mode.png) | ![](cpp_ios_light/items_updating_scroll_mode.png) | ![](csharp_ios_dark/items_updating_scroll_mode.png) | ![](cpp_ios_dark/items_updating_scroll_mode.png) |

### 112. Radio Button Group — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The internal padding/spacing between the radio button circles and their associated text labels is slightly inconsistent with the MAUI reference.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side margins; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/radio_button_group.png) | ![](cpp_ios_light/radio_button_group.png) | ![](csharp_ios_dark/radio_button_group.png) | ![](cpp_ios_dark/radio_button_group.png) |

### 113. Radio Button Group Binding — ⬛ L:cpp_blank · ⬛ D:cpp_blank  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- MAUI reference is blank, making it impossible to verify content parity.

**🧩 MAUI quirks (discuss):**
- MAUI reference is blank in both light and dark modes.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/radio_button_group_binding.png) | ![](cpp_ios_light/radio_button_group_binding.png) | ![](csharp_ios_dark/radio_button_group_binding.png) | ![](cpp_ios_dark/radio_button_group_binding.png) |

### 114. Radio Button Group Gallery — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port renders an additional 'Test: mixed group names' section with five radio buttons that is completely absent in the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port renders the page content flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/radio_button_group_gallery.png) | ![](cpp_ios_light/radio_button_group_gallery.png) | ![](csharp_ios_dark/radio_button_group_gallery.png) | ![](cpp_ios_dark/radio_button_group_gallery.png) |

### 115. Radio Button Border — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Radio button containers in the C++ port are much thinner and lack the consistent padding/height of the MAUI controls.
- The border thickness and corner radius of the radio button containers do not match the MAUI implementation.
- The radio button selection indicator (the inner circle) is visually smaller and differently positioned in the C++ port.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/radio_button_border.png) | ![](cpp_ios_light/radio_button_border.png) | ![](csharp_ios_dark/radio_button_border.png) | ![](cpp_ios_dark/radio_button_border.png) |

### 116. Radio Button Content — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The custom template section at the bottom is missing the coffee cup icons in the C++ port.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant padding and top/bottom cropping; the port displays the page edge-to-edge.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/radio_button_content.png) | ![](cpp_ios_light/radio_button_content.png) | ![](csharp_ios_dark/radio_button_content.png) | ![](cpp_ios_dark/radio_button_content.png) |

### 117. Radio Content Properties — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port displays four additional RadioButton controls at the bottom of the page that are missing in the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI uses a large outer container margin and top/bottom cropping; the C++ port uses minimal padding and shows more content.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/radio_content_properties.png) | ![](cpp_ios_light/radio_content_properties.png) | ![](csharp_ios_dark/radio_content_properties.png) | ![](cpp_ios_dark/radio_content_properties.png) |

### 118. Radio Template From Style — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- MAUI is blank in both light and dark modes, failing to render the page content present in the C++ port.

**🧩 MAUI quirks (discuss):**
- MAUI harness renders a blank screen while the C++ port renders the actual page content.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/radio_template_from_style.png) | ![](cpp_ios_light/radio_template_from_style.png) | ![](csharp_ios_dark/radio_template_from_style.png) | ![](cpp_ios_dark/radio_template_from_style.png) |

### 119. Scattered Radio Button — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The horizontal spacing between the radio button circles and their corresponding text labels is narrower in the C++ port.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a container with significant top and side padding; the C++ port uses a different layout container with less padding.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/scattered_radio_button.png) | ![](cpp_ios_light/scattered_radio_button.png) | ![](csharp_ios_dark/scattered_radio_button.png) | ![](cpp_ios_dark/scattered_radio_button.png) |

### 120. Swipe Gesture — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The SwipeView component is completely missing in the C++ port, leaving a large empty gap between the header and footer text.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/side padding; the C++ port uses a full-screen layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/swipe_gesture.png) | ![](cpp_ios_light/swipe_gesture.png) | ![](csharp_ios_dark/swipe_gesture.png) | ![](cpp_ios_dark/swipe_gesture.png) |

### 121. Swipe Item Position — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The layout and styling are identical once the harness wrapper differences are accounted for._

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side margins; the C++ port renders the page edge-to-edge.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/swipe_item_position.png) | ![](cpp_ios_light/swipe_item_position.png) | ![](csharp_ios_dark/swipe_item_position.png) | ![](cpp_ios_dark/swipe_item_position.png) |

### 122. Swipe Item Size — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port renders an additional 'SwipeView 256 Height' row at the bottom that is missing in the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side margins; the C++ port uses full-width layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/swipe_item_size.png) | ![](cpp_ios_light/swipe_item_size.png) | ![](csharp_ios_dark/swipe_item_size.png) | ![](cpp_ios_dark/swipe_item_size.png) |

### 123. Swipe Threshold — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port displays two additional sections ('Custom Threshold (Execute Mode)' and 'Reveal threshold=80 / Execute threshold=80') that are missing in the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/bottom/side padding; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/swipe_threshold.png) | ![](cpp_ios_light/swipe_threshold.png) | ![](csharp_ios_dark/swipe_threshold.png) | ![](cpp_ios_dark/swipe_threshold.png) |

### 124. Swipe View Margin — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port includes an additional 'Vertical Swipeltems' control at the bottom of the list that is missing in the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant outer margins and top/bottom cropping; the C++ port uses full-screen layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/swipe_view_margin.png) | ![](cpp_ios_light/swipe_view_margin.png) | ![](csharp_ios_dark/swipe_view_margin.png) | ![](cpp_ios_dark/swipe_view_margin.png) |

### 125. Swipe View Shadow — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The layout, sizing, and styling of the SwipeView elements are identical to MAUI._

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side margins; the C++ port renders the page full-width.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/swipe_view_shadow.png) | ![](cpp_ios_light/swipe_view_shadow.png) | ![](csharp_ios_dark/swipe_view_shadow.png) | ![](cpp_ios_dark/swipe_view_shadow.png) |

### 126. Swipe Refresh — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI insets the page content within a card container; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/swipe_refresh.png) | ![](cpp_ios_light/swipe_refresh.png) | ![](csharp_ios_dark/swipe_refresh.png) | ![](cpp_ios_dark/swipe_refresh.png) |

### 127. Refresh View — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The content layout, spacing, and colors are identical once the harness wrapper differences are accounted for._

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/refresh_view.png) | ![](cpp_ios_light/refresh_view.png) | ![](csharp_ios_dark/refresh_view.png) | ![](cpp_ios_dark/refresh_view.png) |

### 128. Custom Size Swipe — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port renders the page edge-to-edge.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/custom_size_swipe.png) | ![](cpp_ios_light/custom_size_swipe.png) | ![](csharp_ios_dark/custom_size_swipe.png) | ![](cpp_ios_dark/custom_size_swipe.png) |

### 129. Custom Swipe Item View — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/custom_swipe_item_view.png) | ![](cpp_ios_light/custom_swipe_item_view.png) | ![](csharp_ios_dark/custom_swipe_item_view.png) | ![](cpp_ios_dark/custom_swipe_item_view.png) |

### 130. Basic Swipe — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The vertical spacing between the list items is slightly tighter in the C++ port than in the MAUI reference.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the content in a card with significant top and side margins; the C++ port uses a full-width layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/basic_swipe.png) | ![](cpp_ios_light/basic_swipe.png) | ![](csharp_ios_dark/basic_swipe.png) | ![](cpp_ios_dark/basic_swipe.png) |

### 131. Gestures — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The 'Last gesture' text content differs between the MAUI and C++ implementations.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/bottom/side padding; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/gestures.png) | ![](cpp_ios_light/gestures.png) | ![](csharp_ios_dark/gestures.png) | ![](cpp_ios_dark/gestures.png) |

### 132. Pan Gesture Events — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI wraps the content in a card with significant top/bottom/side padding; the C++ port renders the content full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/pan_gesture_events.png) | ![](cpp_ios_light/pan_gesture_events.png) | ![](csharp_ios_dark/pan_gesture_events.png) | ![](cpp_ios_dark/pan_gesture_events.png) |

### 133. Pointer Gesture — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI insets the entire page content within a container, while the C++ port renders content flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/pointer_gesture.png) | ![](cpp_ios_light/pointer_gesture.png) | ![](csharp_ios_dark/pointer_gesture.png) | ![](cpp_ios_dark/pointer_gesture.png) |

### 134. Drag Drop — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port displays extra text lines ('Drag position relative to...', 'Drop position relative to...', 'Move: swatch dropped into Rainbow') not present in the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/bottom and side margins; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/drag_drop.png) | ![](cpp_ios_light/drag_drop.png) | ![](csharp_ios_dark/drag_drop.png) | ![](cpp_ios_dark/drag_drop.png) |

### 135. Hit Testing — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port renders an additional large green rounded rectangle at the bottom of the screen not present in the MAUI version.
- The 'Selected' text label content differs ('-' in MAUI vs 'Image' in C++ port).

**🧩 MAUI quirks (discuss):**
- MAUI version has a large top inset and status bar area, while the C++ port uses the full screen height.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/hit_testing.png) | ![](cpp_ios_light/hit_testing.png) | ![](csharp_ios_dark/hit_testing.png) | ![](cpp_ios_dark/hit_testing.png) |

### 136. Input Transparent — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port renders an additional toggle switch and associated label at the bottom of the page which are not present in the MAUI reference.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/bottom/side margins; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/input_transparent.png) | ![](cpp_ios_light/input_transparent.png) | ![](csharp_ios_dark/input_transparent.png) | ![](cpp_ios_dark/input_transparent.png) |

### 137. Focus — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI wraps content in a card with significant top/side padding; port uses full-screen layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/focus.png) | ![](cpp_ios_light/focus.png) | ![](csharp_ios_dark/focus.png) | ![](cpp_ios_dark/focus.png) |

### 138. Dispatcher — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port displays extra UI elements (3 Second Timer, Device.StartTimer) that are missing from the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port uses full-screen layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/dispatcher.png) | ![](cpp_ios_light/dispatcher.png) | ![](csharp_ios_dark/dispatcher.png) | ![](cpp_ios_dark/dispatcher.png) |

### 139. Device — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Text block is anchored to the top-left in the C++ port instead of being vertically and horizontally centered as in MAUI.

**🧩 MAUI quirks (discuss):**
- MAUI wraps content in a card with significant top/side padding; C++ port renders content flush to the top-left.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/device.png) | ![](cpp_ios_light/device.png) | ![](csharp_ios_dark/device.png) | ![](cpp_ios_dark/device.png) |

### 140. Effects — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The C++ port matches the MAUI layout and styling perfectly, ignoring the harness wrapper differences._

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a container with significant top and side padding; the C++ port uses standard screen-edge padding.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/effects.png) | ![](cpp_ios_light/effects.png) | ![](csharp_ios_dark/effects.png) | ![](cpp_ios_dark/effects.png) |

### 141. Measure First Strategy — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port displays extra list content (Fantastic Four, Defenders, Heroes for Hire) that is missing from the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side margins; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/measure_first_strategy.png) | ![](cpp_ios_light/measure_first_strategy.png) | ![](csharp_ios_dark/measure_first_strategy.png) | ![](cpp_ios_dark/measure_first_strategy.png) |

### 142. Scroll View — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port renders a 'Scrolled to: 0 / 0 (done)' header that is completely absent in the MAUI version.
- The list content in the C++ port starts at 'Row 0', whereas the MAUI version starts at 'Row 12', indicating a different scroll position or data source.

**🧩 MAUI quirks (discuss):**
- MAUI insets the entire page inside a container card with significant padding, while the C++ port renders flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/scroll_view.png) | ![](cpp_ios_light/scroll_view.png) | ![](csharp_ios_dark/scroll_view.png) | ![](cpp_ios_dark/scroll_view.png) |

### 143. Web View — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- C++ port includes extra header text and navigation logs not present in the MAUI version.
- MAUI dark mode shows a long file path in the navigation log, whereas the C++ port shows a URL.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side margins; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/web_view.png) | ![](cpp_ios_light/web_view.png) | ![](csharp_ios_dark/web_view.png) | ![](cpp_ios_dark/web_view.png) |

### 144. Hybrid Web View — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Text labels in the C++ port are truncated with ellipses, whereas MAUI displays the full text strings.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/side padding; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/hybrid_web_view.png) | ![](cpp_ios_light/hybrid_web_view.png) | ![](csharp_ios_dark/hybrid_web_view.png) | ![](cpp_ios_dark/hybrid_web_view.png) |

### 145. Alerts — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI insets the entire page content within a card-like container with significant padding, while the C++ port renders the content flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/alerts.png) | ![](cpp_ios_light/alerts.png) | ![](csharp_ios_dark/alerts.png) | ![](cpp_ios_dark/alerts.png) |

### 146. Animation — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The text label 't (animation target)' is rotated 90 degrees in MAUI but appears horizontal in the C++ port.
- The text label content differs slightly ('t (animation target)' vs '.NET bot (animation target)').

**🧩 MAUI quirks (discuss):**
- MAUI renders the page inside a card with significant top/bottom padding and clipping; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/animation.png) | ![](cpp_ios_light/animation.png) | ![](csharp_ios_dark/animation.png) | ![](cpp_ios_dark/animation.png) |

### 147. Application Control — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The status text at the bottom of the page contains different content ('MAUI C++ — gallery' in the port vs 'untitled' in MAUI).

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/side padding; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/application_control.png) | ![](cpp_ios_light/application_control.png) | ![](csharp_ios_dark/application_control.png) | ![](cpp_ios_dark/application_control.png) |

### 148. Ios Entry — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI insets the page content within a container card while the C++ port renders full-screen

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/ios_entry.png) | ![](cpp_ios_light/ios_entry.png) | ![](csharp_ios_dark/ios_entry.png) | ![](cpp_ios_dark/ios_entry.png) |

### 149. Ios Date Picker — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI renders the page inside a card with significant top and side padding; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/ios_date_picker.png) | ![](cpp_ios_light/ios_date_picker.png) | ![](csharp_ios_dark/ios_date_picker.png) | ![](cpp_ios_dark/ios_date_picker.png) |

### 150. Ios Time Picker — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- TimePicker control height and internal text alignment/padding differ slightly from the MAUI reference.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side margins; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/ios_time_picker.png) | ![](cpp_ios_light/ios_time_picker.png) | ![](csharp_ios_dark/ios_time_picker.png) | ![](cpp_ios_dark/ios_time_picker.png) |

### 151. Ios Picker — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI wraps content in a card with top/bottom padding and rounded corners; port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/ios_picker.png) | ![](cpp_ios_light/ios_picker.png) | ![](csharp_ios_dark/ios_picker.png) | ![](cpp_ios_dark/ios_picker.png) |

### 152. Ios Search Bar — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI wraps content in a card with significant top/bottom/side margins; port renders full-screen

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/ios_search_bar.png) | ![](cpp_ios_light/ios_search_bar.png) | ![](csharp_ios_dark/ios_search_bar.png) | ![](cpp_ios_dark/ios_search_bar.png) |

### 153. Ios Scroll View — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port includes an extra back button in the top-left corner not present in the MAUI source.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/side padding; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/ios_scroll_view.png) | ![](cpp_ios_light/ios_scroll_view.png) | ![](csharp_ios_dark/ios_scroll_view.png) | ![](cpp_ios_dark/ios_scroll_view.png) |

### 154. Ios Slider Update On Tap — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The slider thumb size and visual styling (shadow/border) do not perfectly match the MAUI implementation.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side margins; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/ios_slider_update_on_tap.png) | ![](cpp_ios_light/ios_slider_update_on_tap.png) | ![](csharp_ios_dark/ios_slider_update_on_tap.png) | ![](cpp_ios_dark/ios_slider_update_on_tap.png) |

### 155. Ios First Responder — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/ios_first_responder.png) | ![](cpp_ios_light/ios_first_responder.png) | ![](csharp_ios_dark/ios_first_responder.png) | ![](cpp_ios_dark/ios_first_responder.png) |

### 156. Ios Pan Gesture — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI includes a navigation bar and status bar area; the C++ port shows the page content directly.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/ios_pan_gesture.png) | ![](cpp_ios_light/ios_pan_gesture.png) | ![](csharp_ios_dark/ios_pan_gesture.png) | ![](cpp_ios_dark/ios_pan_gesture.png) |

### 157. Ios Safe Area — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The content layout, text, and colors are identical once the harness wrapper differences are accounted for._

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/bottom/side padding; the C++ port renders the page edge-to-edge.
- MAUI status bar and navigation area are part of the harness; the C++ port shows the native system status bar.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/ios_safe_area.png) | ![](cpp_ios_light/ios_safe_area.png) | ![](csharp_ios_dark/ios_safe_area.png) | ![](cpp_ios_dark/ios_safe_area.png) |

### 158. Ios Swipe Transition — 🟡 L:minor · 🟡 D:minor  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Slightly inconsistent vertical spacing between the header text and the swipeable container compared to MAUI.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side margins; the C++ port uses a full-screen layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/ios_swipe_transition.png) | ![](cpp_ios_light/ios_swipe_transition.png) | ![](csharp_ios_dark/ios_swipe_transition.png) | ![](cpp_ios_dark/ios_swipe_transition.png) |

### 159. Ios Blur Effect — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port displays an image at the top of the page which is not present in the MAUI version.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page content in a card with significant top and side margins; the C++ port renders the page content directly to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/ios_blur_effect.png) | ![](cpp_ios_light/ios_blur_effect.png) | ![](csharp_ios_dark/ios_blur_effect.png) | ![](cpp_ios_dark/ios_blur_effect.png) |

### 160. Navigation Gallery — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The content layout, spacing, and colors are identical once the harness wrapper differences are accounted for._

**🧩 MAUI quirks (discuss):**
- MAUI renders the page inside a card with significant top and side padding; the C++ port renders full-screen.
- MAUI status bar and top-level navigation chrome are present; the C++ port uses a different status bar style.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/navigation_gallery.png) | ![](cpp_ios_light/navigation_gallery.png) | ![](csharp_ios_dark/navigation_gallery.png) | ![](cpp_ios_dark/navigation_gallery.png) |

### 161. Modal — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI displays status bar and notch area, while the C++ port shows a different status bar layout and dynamic island area.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/modal.png) | ![](cpp_ios_light/modal.png) | ![](csharp_ios_dark/modal.png) | ![](cpp_ios_dark/modal.png) |

### 162. Tabbed Flyout — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port implements a bottom tab bar navigation pattern, whereas MAUI uses a list-based menu.
- The C++ port includes a back button in the top-left corner which is absent in the MAUI view.
- The C++ port displays the page title at the top of the screen, which is missing in the MAUI view.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/side margins; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/tabbed_flyout.png) | ![](cpp_ios_light/tabbed_flyout.png) | ![](csharp_ios_dark/tabbed_flyout.png) | ![](cpp_ios_dark/tabbed_flyout.png) |

### 163. Toolbar — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The layout, spacing, and text rendering are identical once the harness wrapper differences are accounted for._

**🧩 MAUI quirks (discuss):**
- MAUI renders the page inside a container with significant top and side padding, while the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/toolbar.png) | ![](cpp_ios_light/toolbar.png) | ![](csharp_ios_dark/toolbar.png) | ![](cpp_ios_dark/toolbar.png) |

### 164. Menu Bar — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI includes a status bar and navigation header; the C++ port shows a different status bar layout and lacks the navigation header.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/menu_bar.png) | ![](cpp_ios_light/menu_bar.png) | ![](csharp_ios_dark/menu_bar.png) | ![](cpp_ios_dark/menu_bar.png) |

### 165. Title Bar — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port has incorrect internal spacing between the 'Content Options' and 'Color Options' sections.
- The text labels and input fields in the C++ port have different font sizes and vertical alignment compared to MAUI.
- The checkbox and radio button controls in the C++ port are sized and positioned differently than in MAUI.
- The 'Set Color' and 'Set Foreground' labels in the C++ port are truncated or misaligned compared to the MAUI layout.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the entire page in a card with large margins and top/bottom cropping; the C++ port uses a full-screen layout.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/title_bar.png) | ![](cpp_ios_light/title_bar.png) | ![](csharp_ios_dark/title_bar.png) | ![](cpp_ios_dark/title_bar.png) |

### 166. Chrome — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — identical_

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/chrome.png) | ![](cpp_ios_light/chrome.png) | ![](csharp_ios_dark/chrome.png) | ![](cpp_ios_dark/chrome.png) |

### 167. Context Flyout — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port renders an unexpected Microsoft Bing cookie consent modal overlay at the bottom of the screen.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top/bottom/side margins; the C++ port renders full-screen.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/context_flyout.png) | ![](cpp_ios_light/context_flyout.png) | ![](csharp_ios_dark/context_flyout.png) | ![](cpp_ios_dark/context_flyout.png) |

### 168. Templated View — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- The C++ port renders fewer cards on screen than MAUI, suggesting incorrect internal padding or control height calculations.
- The text in the C++ port appears slightly more condensed or differently aligned compared to the MAUI source.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a large white card with significant outer margins; the C++ port renders the page full-width.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/templated_view.png) | ![](cpp_ios_light/templated_view.png) | ![](csharp_ios_dark/templated_view.png) | ![](cpp_ios_dark/templated_view.png) |

### 169. Custom Layout — 🟢 L:match · 🟢 D:match  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):** _none — The layout and element positioning are identical once the harness wrapper differences are accounted for._

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant top and side padding; the C++ port renders the page edge-to-edge.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/custom_layout.png) | ![](cpp_ios_light/custom_layout.png) | ![](csharp_ios_dark/custom_layout.png) | ![](cpp_ios_dark/custom_layout.png) |

### 170. Visual States — 🔴 L:diff · 🔴 D:diff  <sub>· gemini-3.1-flash-lite</sub>
**🛠 Port diffs (fix):**
- Entry fields in the C++ port lack the distinct border/background styling present in MAUI.
- The vertical spacing between elements is inconsistent compared to the MAUI reference.
- The C++ port is missing the container card background color, resulting in a stark white/black background instead of the MAUI card-based layout.

**🧩 MAUI quirks (discuss):**
- MAUI wraps the page in a card with significant outer padding and rounded corners; the C++ port renders the page flush to the screen edges.

| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |
| --- | --- | --- | --- |
| ![](csharp_ios_light/visual_states.png) | ![](cpp_ios_light/visual_states.png) | ![](csharp_ios_dark/visual_states.png) | ![](cpp_ios_dark/visual_states.png) |
