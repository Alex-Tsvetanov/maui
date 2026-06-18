# MAUI C++ port — example gallery

These are **runnable, code-first demos** of the C++23 .NET MAUI port, each exercising a family of
controls on the *real* native backends — **macOS (AppKit)** and **iOS (UIKit)**. Every screenshot and
recording below was captured from the actual `maui_macos_gallery` / `maui_ios_gallery` apps driving the
ported controls through the virtual-view ⇄ handler ⇄ native seam (no mock-ups).

> **Scope.** This is a *representative* gallery — one page per control family — not (yet) a 1:1 port of
> .NET MAUI's ~282 `Controls.Sample` pages. A full 1:1 example port is the roadmap; this set proves each
> control family renders and reacts on macOS + iOS today.

## Examples

| Example | What it shows | macOS | iOS |
| --- | --- | :---: | :---: |
| [Value controls](value_controls/) | switch · checkbox · slider · stepper · progress · activity indicator | ✅ | ✅ |
| [Input controls](input_controls/) | editor · search bar · radio group · image button | ✅ | ✅ |
| [Pickers](pickers/) | picker · date picker · time picker | ✅ | ✅ |
| [Formatted text](formatted_text/) | rich `FormattedText` spans (bold / italic / colour / underline / kern) | ✅ | ✅ |
| [Items (CollectionView)](items/) | observable items source · templated cell · selection | ✅ | ✅ |
| [Shapes & drawing](shapes/) | graphics view · box view · rectangle / ellipse / line / polygon / path | ✅ | ✅ |
| [Containers](containers/) | scroll view · border · frame · content view | ✅ | ✅ |
| [Swipe & refresh](swipe_refresh/) | refresh view · swipe view (interaction-driven) | ✅ | ⚠️ |
| [WebView](web_view/) | WKWebView · navigation · `EvaluateJavaScript` | ✅ | ⚠️ |
| [Window chrome](chrome/) | toolbar · menu bar · context flyout · tooltip | ✅ | ⚠️ |
| [Tabbed + flyout](tabbed_flyout/) | flyout page · tabbed page (multi-page composition) | ✅ | ❌ |

Legend: ✅ renders natively · ⚠️ partial (see the page's README — usually interaction-only or a faithful
iOS no-op) · ❌ known gallery-host limitation (see the page's README).

## Platform status

| Platform | Status |
| --- | --- |
| macOS (AppKit) | ✅ demo |
| iOS (UIKit) | ✅ demo |
| Windows | ⬜ TODO |
| Linux | ⬜ TODO |
| Android | ⬜ TODO |

## Build & run

```sh
export VCPKG_ROOT="$HOME/vcpkg"

# macOS
cmake --preset apple && cmake --build --preset apple --target maui_macos_gallery
MAUI_SAMPLE_PAGE=value_controls ./build/apple/maui_macos_gallery     # any page key from the table

# iOS simulator
cmake --preset ios && cmake --build --preset ios --target maui_ios_gallery
xcrun simctl boot "iPhone 17"; xcrun simctl install booted build/ios/maui_ios_gallery.app
SIMCTL_CHILD_MAUI_SAMPLE_PAGE=value_controls xcrun simctl launch booted dev.maui-cpp.ios-gallery
```

The page is chosen by the `MAUI_SAMPLE_PAGE` environment variable (one of the keys in the table; default
`value_controls`). Each page is a self-contained `maui::samples::*_page` in
[`src/samples/pages/`](../../src/samples/pages/); the host (`src/samples/macos_gallery.mm` /
`src/samples/ios_gallery.mm`) builds a `maui_app`, attaches the page's handlers bottom-up, opens the
window, and lays the page out.

> Recordings are short loops of each page running on the simulator (captured with `simctl io
> recordVideo`, encoded to GIF with ffmpeg's `palettegen`/`paletteuse`). The demo pages are static at
> rest, so the loops show the rendered page rather than motion — interaction (slider drag, swipe, etc.)
> is manual. Screenshots are real `screencapture` / `simctl io` captures, not composited.
