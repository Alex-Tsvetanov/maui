---
title: "Microsoft.Maui.Media"
tags:
  - api
  - namespace
  - ns/Microsoft-Maui-Media
---

# Microsoft.Maui.Media

> [!info] Namespace
> `Microsoft.Maui.Media` — 14 public types.

- [Online namespace docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.media)

## Overview

`Microsoft.Maui.Media` provides cross-platform access to a device's media and audio capabilities, letting a single .NET MAUI codebase pick photos and videos, capture the screen, and read text aloud without writing platform-specific code. It groups several otherwise-unrelated device features behind small, focused contracts so apps can rely on the same API surface across Android, iOS, macOS, and Windows.

The media-capture side centers on [[IMediaPicker|IMediaPicker]] and its default implementation [[MediaPicker|MediaPicker]], which let a user pick an existing photo or video from the gallery or capture a new one with the camera. [[MediaPickerOptions|MediaPickerOptions]] tailors that interaction — for example, supplying a title for the picker UI.

Screen capture is handled by [[IScreenshot|IScreenshot]] and [[Screenshot|Screenshot]], which grab the currently displayed UI and return an [[IScreenshotResult|IScreenshotResult]] that can be saved or streamed in a chosen [[ScreenshotFormat|ScreenshotFormat]]. [[IPlatformScreenshot|IPlatformScreenshot]] exposes the underlying platform hook, and [[ScreenshotExtensions|ScreenshotExtensions]] adds convenience helpers over `IScreenshot`.

Accessibility and audio output come from [[ITextToSpeech|ITextToSpeech]] and [[TextToSpeech|TextToSpeech]], which speak text using the device's built-in engines and enumerate available [[Locale|Locale]] voices, with [[SpeechOptions|SpeechOptions]] controlling pitch, volume, and locale of the spoken output.

## Key types

- [[IMediaPicker|IMediaPicker]] — lets a user pick or take a photo or video on the device.
- [[MediaPicker|MediaPicker]] — default media picker; reports whether media capture is supported.
- [[MediaPickerOptions|MediaPickerOptions]] — options that customize how media is picked from the device.
- [[IScreenshot|IScreenshot]] — captures the app's currently displayed screen.
- [[IScreenshotResult|IScreenshotResult]] — the result of a screen capture, ready to save or stream.
- [[ScreenshotFormat|ScreenshotFormat]] — the possible image formats for reading screenshots.
- [[ScreenshotExtensions|ScreenshotExtensions]] — static extension helpers for use with `IScreenshot`.
- [[ITextToSpeech|ITextToSpeech]] — speaks back text and queries available languages via built-in engines.
- [[TextToSpeech|TextToSpeech]] — default text-to-speech implementation and supported-language list.
- [[SpeechOptions|SpeechOptions]] — options that influence text-to-speech behavior such as locale and pitch.
- [[Locale|Locale]] — a geographical, political, or cultural region used to select a speech voice.


## Classes

| Type | Summary |
|---|---|
| [[Locale\|Locale]] | Represents a specific geographical, political, or cultural region. |
| [[MediaPicker\|MediaPicker]] | Gets a value indicating whether capturing media is supported on this device. |
| [[MediaPickerOptions\|MediaPickerOptions]] | Pick options for picking media from the device. |
| [[Screenshot\|Screenshot]] | The width of this screenshot in pixels. |
| [[ScreenshotExtensions\|ScreenshotExtensions]] | This class contains static extension methods for use with `IScreenshot`. |
| [[SpeechOptions\|SpeechOptions]] | Represents options that can be used to influence the `ITextToSpeech` behavior. |
| [[TextToSpeech\|TextToSpeech]] | Gets a list of languages supported by text-to-speech. |
| [[UnitConverters\|UnitConverters]] | Static class with built-in unit converters. |

## Interfaces

| Type | Summary |
|---|---|
| [[IMediaPicker\|IMediaPicker]] | The MediaPicker API lets a user pick or take a photo or video on the device. |
| [[IPlatformScreenshot\|IPlatformScreenshot]] | Gets a value indicating whether capturing screenshots is supported on this device. |
| [[IScreenshot\|IScreenshot]] | The Screenshot API lets you take a capture of the current displayed screen of the app. |
| [[IScreenshotResult\|IScreenshotResult]] | Captures a screenshot of the specified activity. |
| [[ITextToSpeech\|ITextToSpeech]] | The TextToSpeech API enables an application to utilize the built-in text-to-speech engines to speak back text from the device and also to query available lan… |

## Enums

| Type | Summary |
|---|---|
| [[ScreenshotFormat\|ScreenshotFormat]] | The possible formats for reading screenshot images. |

## See also

- [[_API Reference]]
