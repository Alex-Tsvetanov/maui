---
title: "Microsoft.Maui.Controls.PlatformConfiguration.WindowsSpecific"
tags:
  - api
  - namespace
  - ns/Microsoft-Maui-Controls-PlatformConfiguration-WindowsSpecific
---

# Microsoft.Maui.Controls.PlatformConfiguration.WindowsSpecific

> [!info] Namespace
> `Microsoft.Maui.Controls.PlatformConfiguration.WindowsSpecific` — 16 public types.

- [Online namespace docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.platformconfiguration.windowsspecific)

## Overview

`Microsoft.Maui.Controls.PlatformConfiguration.WindowsSpecific` exposes the Windows (WinUI/UWP) slice of .NET MAUI's platform-specifics system. Platform-specifics let you opt into behavior that exists only on a single platform without dropping down to a custom handler or renderer. The types here act as fluent configuration entry points, each one extending a familiar cross-platform control and surfacing the Windows-only knobs that control wraps. You reach them through a control's `On<Windows>()` configuration, then read or set the platform property.

The namespace covers a broad range of surfaces. Page-level and shell-level layout is handled by [[Page (WindowsSpecific)|Page]], [[FlyoutPage (WindowsSpecific)|FlyoutPage]], and [[TabbedPage (WindowsSpecific)|TabbedPage]], which manage flyout collapse behavior, pane width, toolbar placement, and header icons. Text and input controls — [[InputView (WindowsSpecific)|InputView]], [[Label (WindowsSpecific)|Label]], and [[SearchBar (WindowsSpecific)|SearchBar]] — expose reading-order detection and spellchecker control. Collection and refresh controls are covered by [[ListView (WindowsSpecific)|ListView]] and [[RefreshView (WindowsSpecific)|RefreshView]].

Several supporting enums and value types parameterize these features, including [[CollapseStyle]], [[ToolbarPlacement (WindowsSpecific)|ToolbarPlacement]], [[ListViewSelectionMode (WindowsSpecific)|ListViewSelectionMode]], [[RefreshPullDirection]], and [[WebViewExecutionMode]]. Together the types give Windows app authors fine-grained control over UWP-specific presentation while keeping the rest of the UI definition cross-platform.

## Key types

- [[Page (WindowsSpecific)|Page]] — Provides the Page Windows platform-specific functionality, including toolbar placement.
- [[FlyoutPage (WindowsSpecific)|FlyoutPage]] — Windows-specific configuration for flyout page collapse behavior and pane width.
- [[TabbedPage (WindowsSpecific)|TabbedPage]] — Controls header icons for tabbed pages on Windows.
- [[Application (WindowsSpecific)|Application]] — Windows-specific configuration for the application's image directory.
- [[VisualElement (WindowsSpecific)|VisualElement]] — Access to platform-specific features of visual elements on Windows.
- [[ListView (WindowsSpecific)|ListView]] — Platform-specific properties for list view controls on UWP.
- [[RefreshView (WindowsSpecific)|RefreshView]] — Windows-specific configuration for the pull-to-refresh gesture direction.
- [[SearchBar (WindowsSpecific)|SearchBar]] — Control over the spellchecker on search bars.
- [[InputView (WindowsSpecific)|InputView]] — Access to reading order detection on Windows.
- [[Label (WindowsSpecific)|Label]] — Access to reading order detection on Windows.
- [[WebView (WindowsSpecific)|WebView]] — Controls whether JavaScript alerts are enabled for a web view.
- [[ToolbarPlacement (WindowsSpecific)|ToolbarPlacement]] — Enumerates toolbar positions for pages on Windows.


## Classes

| Type | Summary |
|---|---|
| [[Application (WindowsSpecific)\|Application (WindowsSpecific)]] | Provides Windows-specific configuration for the application's image directory. |
| [[FlyoutPage (WindowsSpecific)\|FlyoutPage (WindowsSpecific)]] | Provides Windows-specific configuration for flyout page collapse behavior and pane width. |
| [[InputView (WindowsSpecific)\|InputView (WindowsSpecific)]] | Provides access to reading order detection on the Windows platform. |
| [[Label (WindowsSpecific)\|Label (WindowsSpecific)]] | Provides access to reading order detection on the Windows platform. |
| [[ListView (WindowsSpecific)\|ListView (WindowsSpecific)]] | Platform-specific properties for list view controls on UWP. |
| [[Page (WindowsSpecific)\|Page (WindowsSpecific)]] | Provides the Page Windows Platform-Specific Functionality. |
| [[RefreshPullDirection\|RefreshPullDirection]] |  |
| [[RefreshView (WindowsSpecific)\|RefreshView (WindowsSpecific)]] | Provides Windows-specific configuration for the pull-to-refresh gesture direction. |
| [[SearchBar (WindowsSpecific)\|SearchBar (WindowsSpecific)]] | Provides control over the spellchecker on search bars. |
| [[TabbedPage (WindowsSpecific)\|TabbedPage (WindowsSpecific)]] | Provides control over header icons on the Windows platform. |
| [[VisualElement (WindowsSpecific)\|VisualElement (WindowsSpecific)]] | Provides access to platform-specific features of visual elements on the Windows platform. |
| [[WebView (WindowsSpecific)\|WebView (WindowsSpecific)]] | Controls whether JavaScript alerts are enabled for a web view. |
| [[WebViewExecutionMode\|WebViewExecutionMode]] |  |

## Enums

| Type | Summary |
|---|---|
| [[CollapseStyle\|CollapseStyle]] | Enumerates collapse styles for master-detail pages. |
| [[ListViewSelectionMode (WindowsSpecific)\|ListViewSelectionMode (WindowsSpecific)]] | Selection modes for list view controls on UWP. |
| [[ToolbarPlacement (WindowsSpecific)\|ToolbarPlacement (WindowsSpecific)]] | Enumerates toolbar positions for pages on the Windows platform. |

## See also

- [[_API Reference]]
