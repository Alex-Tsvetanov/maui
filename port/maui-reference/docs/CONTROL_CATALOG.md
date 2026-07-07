# CONTROL_CATALOG — MAUI control coverage in the C++ port

_Generated 2026-07-08 · 79 MAUI controls · 77 with a C++ twin · 50 handler-registered · 62 XAML-registered_

## Coverage
| MAUI control | C++ twin | Handler | XAML | Pages |
|---|---|---|---|---|
| ListView | — | — | — | — |
| RoundRectangle | — (only round_rectangle_geometry) | — | — | 8 |
| TemplatedView | templated_view | — | — | — |
| TitleBar | title_bar | — | — | 1 |
| EntryCell | entry_cell | — | EntryCell | 1 |
| SwitchCell | switch_cell | — | SwitchCell | 1 |
| TextCell | text_cell | — | TextCell | 1 |
| ViewCell | view_cell | — | ViewCell | 1 |
| ImageCell | image_cell | — | ImageCell | — |
| SwipeItem | swipe_item | — | SwipeItem | 1 |
| ToolbarItem | toolbar_item | — | — | 1 |
| MenuItem | menu_item | — | — | — |
| MenuBar | menu_bar | — | — | — |
| MenuBarItem | menu_bar_item | — | — | 1 |
| MenuFlyoutItem | menu_flyout_item | — | — | 2 |
| MenuFlyoutSubItem | menu_flyout_sub_item | — | — | 1 |
| ShellItem | shell_item | — | ShellItem | — |
| ShellSection | shell_section | — | ShellSection | — |
| ShellContent | shell_content | — | ShellContent | — |
| FlyoutItem | flyout_item | — | FlyoutItem | — |
| Tab | tab (shell_section) | — | Tab | — |
| TabBar | tab_bar (shell_item) | — | TabBar | — |
| TapGestureRecognizer | tap_gesture_recognizer | — | — | — |
| PanGestureRecognizer | pan_gesture_recognizer | — | — | — |
| PinchGestureRecognizer | pinch_gesture_recognizer | — | — | — |
| SwipeGestureRecognizer | swipe_gesture_recognizer | — | — | — |
| PointerGestureRecognizer | pointer_gesture_recognizer | — | — | — |
| DragGestureRecognizer | drag_gesture_recognizer | — | — | — |
| DropGestureRecognizer | drop_gesture_recognizer | — | — | — |
| GraphicsView | graphics_view | graphics_view_handler | GraphicsView | — |
| SwipeItemView | swipe_item_view | content_page_handler | SwipeItemView | — |
| FlyoutPage | flyout_page | flyout_page_handler | FlyoutPage | — |
| NavigationPage | navigation_page | navigation_page_handler | NavigationPage | — |
| TabbedPage | tabbed_page | tabbed_page_handler | TabbedPage | — |
| Shell | shell | shell_handler | Shell | — |
| ContentPage | content_page | content_page_handler | ContentPage | 195 |
| Label | label | label_handler | Label | 181 |
| VerticalStackLayout | vertical_stack_layout | layout_handler | VerticalStackLayout | 123 |
| Button | button | button_handler | Button | 75 |
| Grid | grid | layout_handler | Grid | 63 |
| StackLayout | stack_layout | layout_handler | StackLayout | 44 |
| CollectionView | collection_view | collection_view_handler | CollectionView | 43 |
| ScrollView | scroll_view | scroll_view_handler | ScrollView | 38 |
| HorizontalStackLayout | horizontal_stack_layout | layout_handler | HorizontalStackLayout | 23 |
| Slider | slider | slider_handler | Slider | 20 |
| BoxView | box_view | shape_view_handler | BoxView | 19 |
| Entry | entry | entry_handler | Entry | 18 |
| Border | border | border_handler | Border | 17 |
| Image | image | image_handler | Image | 17 |
| SearchBar | search_bar | search_bar_handler | SearchBar | 15 |
| SwipeView | swipe_view | swipe_view_handler | SwipeView | 12 |
| Picker | picker | picker_handler | Picker | 10 |
| Switch | toggle_switch | switch_handler | Switch | 9 |
| CheckBox | check_box | check_box_handler | CheckBox | 8 |
| Ellipse | ellipse | shape_view_handler | Ellipse | 8 |
| RadioButton | radio_button | radio_button_handler | RadioButton | 8 |
| Polygon | polygon | shape_view_handler | Polygon | 7 |
| Editor | editor | editor_handler | Editor | 6 |
| Rectangle | rectangle | shape_view_handler | Rectangle | 6 |
| ContentView | content_view | content_page_handler | ContentView | 5 |
| Frame | frame | border_handler | Frame | 5 |
| Line | line | shape_view_handler | Line | 5 |
| Stepper | stepper | stepper_handler | Stepper | 5 |
| DatePicker | date_picker | date_picker_handler | DatePicker | 4 |
| Polyline | polyline | shape_view_handler | Polyline | 4 |
| TimePicker | time_picker | time_picker_handler | TimePicker | 4 |
| ActivityIndicator | activity_indicator | activity_indicator_handler | ActivityIndicator | 3 |
| CarouselView | carousel_view | collection_view_handler | CarouselView | 3 |
| ProgressBar | progress_bar | progress_bar_handler | ProgressBar | 3 |
| AbsoluteLayout | absolute_layout | layout_handler | AbsoluteLayout | 2 |
| ImageButton | image_button | image_button_handler | ImageButton | 2 |
| Path | path | shape_view_handler | Path | 2 |
| RefreshView | refresh_view | refresh_view_handler | RefreshView | 2 |
| WebView | web_view | web_view_handler | WebView | 2 |
| ContentPresenter | content_presenter | content_page_handler | ContentPresenter | 1 |
| FlexLayout | flex_layout | layout_handler | FlexLayout | 1 |
| HybridWebView | hybrid_web_view | hybrid_web_view_handler | HybridWebView | 1 |
| IndicatorView | indicator_view | indicator_view_handler | IndicatorView | 1 |
| TableView | table_view | table_view_handler | TableView | 1 |

## Gaps (ranked, actionable)
- **ListView** — missing: twin, handler, xaml, page. No `list_view` type exists at all; the legacy list control is fully unported — blocks every ListView-based page.
- **RoundRectangle** — missing: twin, handler, xaml. Only `round_rectangle_geometry` (a clip Geometry) exists, not the RoundRectangle *Shape*; 8 Border `StrokeShape` pages reference it (see Notes — uncertain, may be handled inside border corner-radius instead).
- **TemplatedView** — missing: handler, xaml, page. Base templated container has no handler; almost always subclassed (Border/ContentView do the real work), so low practical impact.
- **TitleBar** — missing: handler, xaml. `title_bar` twin exists but no handler or XAML registration, so the `gap_title_bar` reference page can't actually render it.
- **ImageCell** — missing: handler, page. Cell renders via its parent table, not a view handler (expected), but nothing exercises it either.
- **EntryCell** — missing: handler. Rendered by `table_view`, not an own view handler (expected for cells); xaml + page present.
- **SwitchCell** — missing: handler. Same cell-rendering pattern; xaml + page present.
- **TextCell** — missing: handler. Same; xaml + page present.
- **ViewCell** — missing: handler. Same; xaml + page present.
- **SwipeItem** — missing: handler. Data element hosted by SwipeView (no own handler by design); xaml + page present.
- **ToolbarItem** — missing: handler, xaml. Hosted by a page toolbar, not a standalone view; page present.
- **MenuBarItem** — missing: handler, xaml. Menu structural element (handler N/A); page present.
- **MenuFlyoutItem** — missing: handler, xaml. Menu element (handler N/A); 2 pages present.
- **MenuFlyoutSubItem** — missing: handler, xaml. Menu element (handler N/A); page present.
- **MenuItem** — missing: handler, xaml, page. Base menu element; no XAML registration and no reference page.
- **MenuBar** — missing: handler, xaml, page. Container menu element; no XAML registration and no reference page.
- **ShellItem** — missing: handler, page. Shell structural node (handler N/A); xaml present, no page.
- **ShellSection** — missing: handler, page. Same.
- **ShellContent** — missing: handler, page. Same.
- **FlyoutItem** — missing: handler, page. Same.
- **Tab** — missing: handler, page. Same.
- **TabBar** — missing: handler, page. Same.
- **TapGestureRecognizer** — missing: handler, xaml, page. Gesture recognizer, not a view — no view handler by design; not registered for markup and no page-tag usage.
- **PanGestureRecognizer** — missing: handler, xaml, page. Same gesture-recognizer pattern.
- **PinchGestureRecognizer** — missing: handler, xaml, page. Same.
- **SwipeGestureRecognizer** — missing: handler, xaml, page. Same.
- **PointerGestureRecognizer** — missing: handler, xaml, page. Same.
- **DragGestureRecognizer** — missing: handler, xaml, page. Same.
- **DropGestureRecognizer** — missing: handler, xaml, page. Same.
- **GraphicsView** — missing: page. Fully wired (twin + handler + xaml); just no reference page draws it — untested only.
- **SwipeItemView** — missing: page. Fully wired; no reference page uses it as a root tag (custom_swipe_item_view page nests it under SwipeItems).
- **FlyoutPage** — missing: page. Fully wired; no reference page (used as a host shell, not a page-tagged element).
- **NavigationPage** — missing: page. Fully wired; used as a host wrapper, not a page-tagged element.
- **TabbedPage** — missing: page. Fully wired; no reference page.
- **Shell** — missing: page. Fully wired; no reference page.

## Notes
- **Handler `—` is not always a defect.** Cells (Entry/Switch/Text/View/ImageCell), gesture recognizers, menu/toolbar items, SwipeItem and the Shell structural nodes are *not* view-handler-driven in MAUI itself — they render via a parent (TableView/ListView, SwipeView, page toolbar, Shell). Their blank handler cell is expected architecture, not a "renders blank" bug; only genuinely-visual missing handlers (TitleBar, TemplatedView) mean nothing paints.
- **RoundRectangle is the one uncertain match.** The port ships `round_rectangle_geometry` (a Geometry, for clips) and registers `RoundRectangleGeometry` in XAML, but no RoundRectangle *Shape* twin. The 8 pages that list it use it as `Border.StrokeShape`. This could be a real gap or the port may realize rounded strokes directly through border corner-radius — verify against `border.hpp`/`border.cpp` before treating it as a hard capability gap.
- **`Switch` → `toggle_switch` is a deliberate rename, not a gap.** Fully covered (switch_handler + XAML "Switch"). Frame reuses `border_handler`; BoxView and all six shapes reuse `shape_view_handler`; the six layouts reuse `layout_handler`; ContentView/ContentPresenter/SwipeItemView reuse `content_page_handler` — all intentional shared handlers.
- **Scan confidence / what each column may have missed:**
  - *Twin:* provenance-grep of `<= Microsoft…` comments only. ListView is a true absence; RoundRectangle-shape and a handful of untagged headers (round_rectangle_geometry etc.) are excluded from the twin set, so a genuinely-present-but-untagged twin could read as missing.
  - *Handler:* scanned only `maui_controls_handlers.cpp` (the builder-boot seed). Parity with the global `MAUI_REGISTER_HANDLER` table in `src/controls/*.cpp` + `src/core/*.cpp` was **not** verified — a control could be globally registered (passing headless tests) yet absent here (rendering blank in the gallery), the exact two-list trap noted in project memory.
  - *XAML:* single deterministic grep over `register_xaml_*.cpp`; no dynamic/macro registration paths observed. Twin+handler existence was not cross-checked per XAML entry.
  - *Pages:* page-*presence* by opening-tag regex over 195 reference XAML files — counts mean "at least one page mentions this tag," not per-instance counts, and are **not** render-verified. Attached-property child elements share a control's tag prefix, so a page counted for e.g. Grid definitely uses Grid but the count is presence-based. NavigationPage/FlyoutPage/Shell/TabbedPage show `—` mostly because they wrap pages as hosts rather than appearing as page-root tags, so "untested" here likely understates real exercise.
