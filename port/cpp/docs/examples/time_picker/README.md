# TimePicker

Ports .NET MAUI's `TimePickerPage` ([oracle](../../../../../src/Controls/samples/Controls.Sample/Pages/Controls/TimePickerPage.xaml)) as a code-first `maui::samples::time_picker_page`. TimePicker variants: Default, BackgroundColor=Blue, gradient Background, randomizable Background (Update/Clear), Time=4:15:26, Disabled, TextColor=Green, Format hh:mm, Set-null/now, and IsOpen Open/Close with Opened/Closed events — plus a live picker whose selection drives a readout.

| macOS (AppKit) | iOS (UIKit) |
| --- | --- |
| ![macOS](macos.png) | ![iOS](ios.png) |

**Running on iOS:**

![iOS demo](ios.gif)

**Platforms:** macOS ✅ demo · iOS ✅ demo · Windows ⬜ TODO · Linux ⬜ TODO · Android ⬜ TODO

**Run it:**
- macOS — `MAUI_SAMPLE_PAGE=time_picker ./build/apple/maui_macos_gallery`
- iOS sim — `SIMCTL_CHILD_MAUI_SAMPLE_PAGE=time_picker xcrun simctl launch booted dev.maui-cpp.ios-gallery`

> Notes: same IsFocused-echo deferral + fixed-seed random background as the date page.
