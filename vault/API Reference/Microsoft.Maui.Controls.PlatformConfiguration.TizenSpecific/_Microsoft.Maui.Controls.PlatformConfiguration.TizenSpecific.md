---
title: "Microsoft.Maui.Controls.PlatformConfiguration.TizenSpecific"
tags:
  - api
  - namespace
  - ns/Microsoft-Maui-Controls-PlatformConfiguration-TizenSpecific
---

# Microsoft.Maui.Controls.PlatformConfiguration.TizenSpecific

> [!info] Namespace
> `Microsoft.Maui.Controls.PlatformConfiguration.TizenSpecific` — 17 public types.

- [Online namespace docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.platformconfiguration.tizenspecific)

## Overview

`Microsoft.Maui.Controls.PlatformConfiguration.TizenSpecific` exposes the **Tizen** branch of .NET MAUI's Platform-Specifics API. Platform-specifics let you opt into behavior or appearance that exists only on a single platform — here, Tizen — without dropping out of shared cross-platform code or writing custom renderers. Each type in this namespace wraps a familiar MAUI control or element and adds Tizen-only attached properties and fluent extension methods that tune how that control looks or behaves on Tizen devices.

The configuration types map one-to-one onto core MAUI controls: [[Application (TizenSpecific)|Application]], [[VisualElement (TizenSpecific)|VisualElement]], [[Entry (TizenSpecific)|Entry]], [[Label (TizenSpecific)|Label]], [[Image (TizenSpecific)|Image]], [[Switch (TizenSpecific)|Switch]], [[ProgressBar (TizenSpecific)|ProgressBar]], [[Page (TizenSpecific)|Page]], and [[NavigationPage (TizenSpecific)|NavigationPage]], among others. Through them you can adjust things such as font weight on text controls, blend color on images, the pulsing state of progress bars, the bread crumb bar on pages, and focus order, styles, and tooltips on visual elements.

Alongside the per-control configuration classes, the namespace ships a set of constant/enumeration helper types that supply the allowed values: [[FontWeight (TizenSpecific)|FontWeight]] and [[FocusDirection]] define constant values, while [[ButtonStyle]], [[SwitchStyle]], [[ProgressBarStyle]], and [[TabbedPageStyle]] enumerate the visual styles available for their respective controls. Together these pieces give Tizen apps fine-grained, type-safe access to platform features while keeping the rest of the UI definition portable.

## Key types

- [[VisualElement (TizenSpecific)|VisualElement]] — Provides access to focus order, styles, and tooltips for visual elements on the Tizen platform.
- [[Application (TizenSpecific)|Application]] — Provides Tizen-specific platform configuration for application-level features.
- [[Entry (TizenSpecific)|Entry]] — Provides access to the font weight for entry controls on the Tizen platform.
- [[Label (TizenSpecific)|Label]] — Provides access to the font weight for labels on the Tizen platform.
- [[Image (TizenSpecific)|Image]] — Provides access to the blend color for images on the Tizen platform.
- [[Switch (TizenSpecific)|Switch]] — Provides Tizen-specific platform configuration for switch controls.
- [[ProgressBar (TizenSpecific)|ProgressBar]] — Provides access to the pulsing status for progress bars.
- [[Page (TizenSpecific)|Page]] — Provides access to the bread crumb bar for pages on the Tizen platform.
- [[NavigationPage (TizenSpecific)|NavigationPage]] — Provides access to the bread crumb bar for navigation pages on the Tizen platform.
- [[FontWeight (TizenSpecific)|FontWeight]] — Contains constants for font weights.
- [[FocusDirection]] — Contains constants for describing focus directions.
- [[ProgressBarStyle]] — Enumerates visual styles for progress bars.


## Classes

| Type | Summary |
|---|---|
| [[Application (TizenSpecific)\|Application (TizenSpecific)]] | Provides Tizen-specific platform configuration for application-level features. |
| [[ButtonStyle\|ButtonStyle]] | Enumerates button styles |
| [[Entry (TizenSpecific)\|Entry (TizenSpecific)]] | Provides access to the font weight for entry controls on the Tizen platform. |
| [[FocusDirection\|FocusDirection]] | Contains constants for describing focus directions. |
| [[FontWeight (TizenSpecific)\|FontWeight (TizenSpecific)]] | Contains constants for font weights. |
| [[Image (TizenSpecific)\|Image (TizenSpecific)]] | Provides access to the blend color for images on the Tizen platform. |
| [[ItemsView (TizenSpecific)\|ItemsView (TizenSpecific)]] |  |
| [[Label (TizenSpecific)\|Label (TizenSpecific)]] | Provides access to the font weight for labels on the Tizen platform. |
| [[NavigationPage (TizenSpecific)\|NavigationPage (TizenSpecific)]] | Provides access to the bread crumb bar for navigation pages on the Tizen platform. |
| [[Page (TizenSpecific)\|Page (TizenSpecific)]] | Provides access to the bread crumb bar for pages on the Tizen platform. |
| [[ProgressBar (TizenSpecific)\|ProgressBar (TizenSpecific)]] | Provides access to the pulsing status for progress bars. |
| [[ProgressBarStyle\|ProgressBarStyle]] | Enumerates visual styles for progress bars. |
| [[ScrollView (TizenSpecific)\|ScrollView (TizenSpecific)]] |  |
| [[Switch (TizenSpecific)\|Switch (TizenSpecific)]] | Provides Tizen-specific platform configuration for switch controls. |
| [[SwitchStyle\|SwitchStyle]] | Enumerates visual styles for switches. |
| [[TabbedPageStyle\|TabbedPageStyle]] | Enumerates tab bar styles for a tabbed page. |
| [[VisualElement (TizenSpecific)\|VisualElement (TizenSpecific)]] | Provides access to focus order, styles, and tooltips for visual elements on the Tizen platform. |

## See also

- [[_API Reference]]
