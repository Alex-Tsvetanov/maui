# HybridWebView

Ports .NET MAUI's `HybridWebViewPage` ([oracle](../../../../../src/Controls/samples/Controls.Sample/Pages/Controls/HybridWebViewPage.xaml)) as a code-first `maui::samples::hybrid_web_view_page`. A hybrid_web_view beside a status editor + button column: send_raw_message + raw_message_received are fully wired (host↔page raw channel), and Invoke buttons drive invoke_js writing results into the status editor.

| macOS (AppKit) | iOS (UIKit) |
| --- | --- |
| ![macOS](macos.png) | ![iOS](ios.png) |

**Running on iOS:**

![iOS demo](ios.gif)

**Platforms:** macOS ✅ demo · iOS ✅ demo · Windows ⬜ TODO · Linux ⬜ TODO · Android ⬜ TODO

**Run it:** `MAUI_SAMPLE_PAGE=hybrid_web_view ./build/apple/maui_macos_gallery` (macOS) · `SIMCTL_CHILD_MAUI_SAMPLE_PAGE=hybrid_web_view xcrun simctl launch booted dev.maui-cpp.ios-gallery` (iOS)

> Notes: SetInvokeJavaScriptTarget / InvokeDotNet (JS→.NET by reflection) and typed InvokeJavaScriptAsync<T> are deferred (no reflection per the port doctrine).
