---
title: "Microsoft.Maui.Controls.PlatformConfiguration.AndroidSpecific"
tags:
  - api
  - namespace
  - ns/Microsoft-Maui-Controls-PlatformConfiguration-AndroidSpecific
---

# Microsoft.Maui.Controls.PlatformConfiguration.AndroidSpecific

> [!info] Namespace
> `Microsoft.Maui.Controls.PlatformConfiguration.AndroidSpecific` — 15 public types.

- [Online namespace docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.platformconfiguration.androidspecific)

## Overview

`Microsoft.Maui.Controls.PlatformConfiguration.AndroidSpecific` is part of .NET MAUI's **platform-specifics** system — a way to opt into platform-unique behavior on Android without abandoning the cross-platform control model. Each type in this namespace acts as a configuration extension over a standard MAUI control, exposing Android-only tweaks (padding, shadows, elevation, toolbar placement, soft-keyboard handling, and similar) as fluent or bindable settings that have no effect on other platforms.

The namespace pairs control-specific configurators with the enums that parameterize them. Controls such as [[Button (AndroidSpecific)|Button]], [[ImageButton (AndroidSpecific)|ImageButton]], and [[VisualElement (AndroidSpecific)|VisualElement]] expose shadow, padding, and elevation behavior, while [[Entry (AndroidSpecific)|Entry]] and [[Application (AndroidSpecific)|Application]] address text-input and on-screen keyboard accommodation. Page- and shell-level types like [[TabbedPage (AndroidSpecific)|TabbedPage]], [[ShellItem (AndroidSpecific)|ShellItem]], [[ListView (AndroidSpecific)|ListView]], [[SwipeView (AndroidSpecific)|SwipeView]], and [[ViewCell (AndroidSpecific)|ViewCell]] cover navigation chrome and list interaction details, and [[WebView (AndroidSpecific)|WebView]] governs mixed secure/insecure content loading.

Enumerations supply the allowed values for these settings: [[ImeFlags]] for input method editor options, [[MixedContentHandling]] for web view content policy, [[ToolbarPlacement (AndroidSpecific)|ToolbarPlacement]] for tab/toolbar position, and [[WindowSoftInputModeAdjust]] for how the layout reacts to the soft keyboard. Together they let you progressively enhance an Android build from shared MAUI code.

> [!tip]
> Platform-specifics are additive: applying an Android-specific setting is ignored on iOS, Windows, and other targets, so you can keep a single shared UI definition.

## Key types

- [[Application (AndroidSpecific)|Application]] — controls window soft-input mode so content pans to accommodate the on-screen keyboard.
- [[Entry (AndroidSpecific)|Entry]] — configures input method editor (IME) options for entry fields.
- [[Button (AndroidSpecific)|Button]] — controls padding and shadow rendering for buttons.
- [[VisualElement (AndroidSpecific)|VisualElement]] — controls legacy color mode and elevation for visual elements.
- [[TabbedPage (AndroidSpecific)|TabbedPage]] — Android tabbed-page configuration, including toolbar placement.
- [[ShellItem (AndroidSpecific)|ShellItem]] — Android-specific platform configuration for Shell items.
- [[SwipeView (AndroidSpecific)|SwipeView]] — controls the swipe transition animation mode.
- [[WebView (AndroidSpecific)|WebView]] — controls handling of mixed secure and insecure content.
- [[ImeFlags]] — enumerates IME options for entry fields.
- [[MixedContentHandling]] — enumerates web view behaviors when handling mixed content.
- [[ToolbarPlacement (AndroidSpecific)|ToolbarPlacement]] — enumerates toolbar positions.
- [[WindowSoftInputModeAdjust]] — enumerates how an on-screen input interface is visually accommodated (bindable).


## Classes

| Type | Summary |
|---|---|
| [[Application (AndroidSpecific)\|Application (AndroidSpecific)]] | Indicates that the content of the control will pan, possibly off of the screen, to accommodate the input interface. |
| [[Button (AndroidSpecific)\|Button (AndroidSpecific)]] | Controls padding and shadows for buttons on the Android platform. |
| [[Entry (AndroidSpecific)\|Entry (AndroidSpecific)]] | Controls input method editor (IME) options for entry fields on the Android platform. |
| [[ImageButton (AndroidSpecific)\|ImageButton (AndroidSpecific)]] | Android-specific shadow effects for ImageButton controls. |
| [[ListView (AndroidSpecific)\|ListView (AndroidSpecific)]] | The list view instance that Microsoft.Maui.Controls created on the Android platform. |
| [[ShellItem (AndroidSpecific)\|ShellItem (AndroidSpecific)]] | Android-specific platform configuration for ShellItem. |
| [[SwipeView (AndroidSpecific)\|SwipeView (AndroidSpecific)]] | Controls the swipe transition animation mode for SwipeView on Android. |
| [[TabbedPage (AndroidSpecific)\|TabbedPage (AndroidSpecific)]] | The tabbed page instance that Microsoft.Maui.Controls created on the Android platform. |
| [[ViewCell (AndroidSpecific)\|ViewCell (AndroidSpecific)]] | Android-specific context actions behavior for ViewCell in ListView. |
| [[VisualElement (AndroidSpecific)\|VisualElement (AndroidSpecific)]] | Controls the legacy color mode and elevation for visual elements on the Android platform. |
| [[WebView (AndroidSpecific)\|WebView (AndroidSpecific)]] | Allow all content, whether secure or insecure. |

## Enums

| Type | Summary |
|---|---|
| [[ImeFlags\|ImeFlags]] | Enumerates input method editor (IME) options for entry fields on the Android platform. |
| [[MixedContentHandling\|MixedContentHandling]] | Enumerates web view behaviors when handling mixed content. |
| [[ToolbarPlacement (AndroidSpecific)\|ToolbarPlacement (AndroidSpecific)]] | Enumerates toolbar positions. |
| [[WindowSoftInputModeAdjust\|WindowSoftInputModeAdjust]] | Enumerates values that control how an on-screen input interface is visually accommodated. This is a bindable property. |

## See also

- [[_API Reference]]
