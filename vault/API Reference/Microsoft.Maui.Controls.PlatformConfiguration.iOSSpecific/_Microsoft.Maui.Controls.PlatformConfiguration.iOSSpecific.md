---
title: "Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific"
tags:
  - api
  - namespace
  - ns/Microsoft-Maui-Controls-PlatformConfiguration-iOSSpecific
---

# Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific

> [!info] Namespace
> `Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific` — 28 public types.

- [Online namespace docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.platformconfiguration.iosspecific)

## Overview

`Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific` holds the iOS branch of .NET MAUI's *platform-specifics* system: opt-in APIs that expose iOS-only behavior on otherwise cross-platform controls without forcing custom renderers or `#if` branching. Each type here adds fluent, attached-property-style configuration that is silently ignored on non-iOS platforms, so you can tune native iOS presentation while keeping a single shared UI definition.

The classes are organized one-per-control, mirroring the MAUI control they extend. Page-level types such as [[NavigationPage (iOSSpecific)|NavigationPage]], [[Page (iOSSpecific)|Page]], and [[TabbedPage (iOSSpecific)|TabbedPage]] handle navigation chrome — status bar visibility and color, large-title display, and tab-bar translucency. Visual and effect surfaces like [[VisualElement (iOSSpecific)|VisualElement]] and [[ShadowEffect]] expose blur and shadow treatments. Input controls including [[DatePicker (iOSSpecific)|DatePicker]], [[TimePicker (iOSSpecific)|TimePicker]], [[Picker (iOSSpecific)|Picker]], [[ListView (iOSSpecific)|ListView]], and [[SearchBar (iOSSpecific)|SearchBar]] adjust update timing and native visual style.

Behavioral choices are expressed through the accompanying enumerations — for example [[UpdateMode]], [[BlurEffectStyle]], [[SeparatorStyle]], [[StatusBarHiddenMode]], [[StatusBarTextColorMode]], [[TranslucencyMode]], and [[LargeTitleDisplayMode]] — which map directly onto the underlying UIKit options. Together these pieces let you reach iOS-native presentation details through a consistent, discoverable, cross-platform-safe API.

> [!info]
> These configurations apply only when running on iOS; on other platforms the calls are no-ops, so they are safe to set unconditionally in shared code.

## Key types

- [[NavigationPage (iOSSpecific)|NavigationPage]] — iOS navigation chrome: status bar, large titles, and translucency settings.
- [[Page (iOSSpecific)|Page]] — Page-level iOS settings such as status bar and modal presentation behavior.
- [[TabbedPage (iOSSpecific)|TabbedPage]] — Controls iOS tab bar translucency for tabbed pages.
- [[VisualElement (iOSSpecific)|VisualElement]] — Access to the blur effect, shadow effect, and legacy color mode on iOS.
- [[ShadowEffect]] — Represents a shadow effect that can be applied to iOS controls.
- [[ListView (iOSSpecific)|ListView]] — Access to the separator style for list views on iOS.
- [[SearchBar (iOSSpecific)|SearchBar]] — iOS-specific configuration for SearchBar visual style.
- [[Picker (iOSSpecific)|Picker]] — iOS picker configuration, including update behavior.
- [[DatePicker (iOSSpecific)|DatePicker]] — iOS-specific configuration for DatePicker update behavior.
- [[FlyoutPage (iOSSpecific)|FlyoutPage]] — iOS-specific configuration for FlyoutPage shadow effects.
- [[UpdateMode]] — Specifies when picker controls update their value during user interaction on iOS.
- [[BlurEffectStyle]] — Enumerates the available iOS blur effect styles.


## Classes

| Type | Summary |
|---|---|
| [[Application (iOSSpecific)\|Application (iOSSpecific)]] | Provides control over simultaneous recognition for pan gesture recognizers. |
| [[Cell (iOSSpecific)\|Cell (iOSSpecific)]] | Provides iOS-specific configuration for Cell background color in ListView. |
| [[DatePicker (iOSSpecific)\|DatePicker (iOSSpecific)]] | Provides iOS-specific configuration for DatePicker update behavior. |
| [[Entry (iOSSpecific)\|Entry (iOSSpecific)]] | The entry instance that Microsoft.Maui.Controls created on the iOS platform. |
| [[FlyoutPage (iOSSpecific)\|FlyoutPage (iOSSpecific)]] | Provides iOS-specific configuration for FlyoutPage shadow effects. |
| [[ListView (iOSSpecific)\|ListView (iOSSpecific)]] | Provides access to the separator style for list views on the iOS platform. |
| [[NavigationPage (iOSSpecific)\|NavigationPage (iOSSpecific)]] | The navigation page instance that Microsoft.Maui.Controls created on the iOS platform. |
| [[Page (iOSSpecific)\|Page (iOSSpecific)]] | The page instance that Microsoft.Maui.Controls created on the iOS platform. |
| [[Picker (iOSSpecific)\|Picker (iOSSpecific)]] | The picker instance that Microsoft.Maui.Controls created on the iOS platform. |
| [[ScrollView (iOSSpecific)\|ScrollView (iOSSpecific)]] | The scroll view instance that Microsoft.Maui.Controls created on the iOS platform. |
| [[SearchBar (iOSSpecific)\|SearchBar (iOSSpecific)]] | Provides iOS-specific configuration for SearchBar visual style. |
| [[ShadowEffect\|ShadowEffect]] | Represents a shadow effect that can be applied to iOS controls. |
| [[Slider (iOSSpecific)\|Slider (iOSSpecific)]] | Platform-specific functionality for sliders the iOS platform. |
| [[SwipeView (iOSSpecific)\|SwipeView (iOSSpecific)]] | Provides iOS-specific configuration for SwipeView transition animations. |
| [[TabbedPage (iOSSpecific)\|TabbedPage (iOSSpecific)]] | Provides iOS-specific configuration for TabbedPage tab bar translucency. |
| [[TimePicker (iOSSpecific)\|TimePicker (iOSSpecific)]] | Provides iOS-specific configuration for TimePicker update behavior. |
| [[VisualElement (iOSSpecific)\|VisualElement (iOSSpecific)]] | Provides access to the blur effect, shadow effect, and legacy color mode on the iOS platform. |

## Enums

| Type | Summary |
|---|---|
| [[BlurEffectStyle\|BlurEffectStyle]] | Enumerates blur effect styles. |
| [[GroupHeaderStyle\|GroupHeaderStyle]] | Specifies the iOS UITableViewStyle for grouped ListView headers. |
| [[LargeTitleDisplayMode\|LargeTitleDisplayMode]] | Enumerates preferences for displaying large titles. |
| [[SeparatorStyle\|SeparatorStyle]] | Enumerates list view separator styles. |
| [[StatusBarHiddenMode\|StatusBarHiddenMode]] | Enumerates status bar hiding behavior preferences. |
| [[StatusBarTextColorMode\|StatusBarTextColorMode]] | Specifies how iOS status bar text color adjusts based on navigation bar color. |
| [[TranslucencyMode\|TranslucencyMode]] | Specifies the iOS tab bar translucency mode. |
| [[UIModalPresentationStyle\|UIModalPresentationStyle]] | Enumerates valid modal presentation styles for iOS. |
| [[UISearchBarStyle\|UISearchBarStyle]] | Specifies the iOS UISearchBarStyle visual appearance. |
| [[UIStatusBarAnimation\|UIStatusBarAnimation]] | Specifies the iOS status bar hide/show animation style. |
| [[UpdateMode\|UpdateMode]] | Specifies when picker controls update their selected value during user interaction on iOS. |

## See also

- [[_API Reference]]
