---
title: "Android platform-specifics in .NET MAUI"
description: "Learn how to consume Android platform-specifics in .NET MAUI apps."
tags:
  - conceptual
  - area/android
ms_date: "04/05/2022"
source: "https://learn.microsoft.com/dotnet/maui/android/platform-specifics?view=net-maui-10.0"
---

# Android platform-specifics

.NET Multi-platform App UI (.NET MAUI) platform-specifics allow you to consume functionality that's only available on a specific platform, without customizing handlers.

The following platform-specific functionality is provided for .NET MAUI views on Android:

- Setting the input method editor options for the soft keyboard for an [[Entry (Controls)|Entry]]. For more information, see [[entry-ime-options|Entry input method editor options on Android]].
- Enabling fast scrolling in a [[ListView (Controls)|ListView]]. For more information, see [[listview-fast-scrolling|ListView fast scrolling on Android]].
- Controlling the transition that's used when opening a [[SwipeView (Controls)|SwipeView]]. For more information, see [[swipeview-swipetransitionmode|SwipeView swipe transition Mode]].
- Controlling whether a [[WebView (Controls)|WebView]] can display mixed content. For more information, see [[webview-mixed-content|WebView mixed content on Android]].
- Enabling zoom on a [[WebView (Controls)|WebView]]. For more information, see [[webview-zoom-controls|WebView zoom on Android]].

The following platform-specific functionality is provided for .NET MAUI pages on Android:

- Disabling transition animations when navigating through pages in a [[TabbedPage (Controls)|TabbedPage]]. For more information, see [[tabbedpage-transition-animations|TabbedPage page transition animations on Android]].
- Enabling swiping between pages in a [[TabbedPage (Controls)|TabbedPage]]. For more information, see [[tabbedpage-page-swiping|TabbedPage page swiping on Android]].
- Setting the toolbar placement and color on a [[TabbedPage (Controls)|TabbedPage]]. For more information, see [[tabbedpage-toolbar-placement|TabbedPage toolbar placement on android]].

The following platform-specific functionality is provided for the .NET MAUI `Application` class on Android:

- Setting the operating mode of a soft keyboard. For more information, see [[soft-keyboard-input-mode|Soft keyboard input mode on Android]].
