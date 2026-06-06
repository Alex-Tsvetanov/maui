---
title: ".NET MAUI Shell lifecycle"
description: "Learn about Shell apps and the .NET MAUI page lifecycle."
tags:
  - conceptual
  - area/fundamentals
ms_date: "08/30/2024"
source: "https://learn.microsoft.com/dotnet/maui/fundamentals/shell/lifecycle?view=net-maui-10.0"
---

# .NET MAUI Shell lifecycle

[![Browse sample.](~/media/code-sample.png) Browse the sample](/samples/dotnet/maui-samples/fundamentals-shell)

Shell apps respect the .NET Multi-platform App UI (.NET MAUI) lifecycle, and additionally fire an [[BaseShellItem.Appearing|Appearing]] event when a page is about to appear on the screen, and a [[BaseShellItem.Disappearing|Disappearing]] event when a page is about to disappear from the screen. These events are propagated to pages, and can be handled by overriding the [[Page (Controls).OnAppearing|OnAppearing]] or [[Page (Controls).OnDisappearing|OnDisappearing]] methods on the page.

> [!NOTE]
> In a Shell app, the [[BaseShellItem.Appearing|Appearing]] and [[BaseShellItem.Disappearing|Disappearing]] events are raised from cross-platform code, prior to platform code making a page visible, or removing a page from the screen.

## Modeless navigation

In a Shell app, pushing a page onto the navigation stack will result in the currently visible [[ShellContent|ShellContent]] object, and its page content, raising the [[BaseShellItem.Disappearing|Disappearing]] event. Similarly, popping the last page from the navigation stack will result in the newly visible [[ShellContent|ShellContent]] object, and its page content, raising the [[BaseShellItem.Appearing|Appearing]] event.

For more information about modeless navigation, see [[navigationpage#perform-modeless-navigation|Perform modeless navigation]].

## Modal navigation

In a Shell app, pushing a modal page onto the modal navigation stack will result in all visible Shell objects raising the [[BaseShellItem.Disappearing|Disappearing]] event. Similarly, popping the last modal page from the modal navigation stack will result in all visible Shell objects raising the [[BaseShellItem.Appearing|Appearing]] event.

For more information about modal navigation, see [[navigationpage#perform-modal-navigation|Perform modal navigation]].
