---
title: "iOS platform-specifics in .NET MAUI"
description: "Learn how to consume iOS platform-specifics in .NET MAUI apps."
tags:
  - conceptual
  - area/ios
ms_date: "03/14/2025"
source: "https://learn.microsoft.com/dotnet/maui/ios/platform-specifics?view=net-maui-10.0"
---

# iOS platform-specifics

.NET Multi-platform App UI (.NET MAUI) platform-specifics allow you to consume functionality that's only available on a specific platform, without customizing handlers.

The following platform-specific functionality is provided for .NET MAUI views on iOS:

- Setting the [[Cell (Controls)|Cell]] background color. For more information, see [[cell-background-color|Cell background color on iOS]].
- Controlling when item selection occurs in a [[DatePicker (Controls)|DatePicker]]. For more information, see [[datepicker-selection|DatePicker item selection on iOS]].
- Ensuring that inputted text fits into an [[Entry (Controls)|Entry]] by adjusting the font size. For more information, see [[entry-font-size|Entry font size on iOS]].
- Setting the cursor color in a [[Entry (Controls)|Entry]]. For more information, see [[entry-cursor-color|Entry cursor color on iOS]].
- Controlling whether [[ListView (Controls)|ListView]] header cells float during scrolling. For more information, see [[listview-group-header-style|ListView group header style on iOS]].
- Controlling whether row animations are disabled when the [[ListView (Controls)|ListView]] items collection is being updated. For more information, see [[listview-row-animations|ListView row animations on iOS]].
- Setting the separator style on a [[ListView (Controls)|ListView]]. For more information, see [[listview-separator-style|ListView separator style on iOS]].
- Controlling when item selection occurs in a [[Picker (Controls)|Picker]]. For more information, see [[picker-selection|Picker item selection on iOS]].
- Controlling whether a [[SearchBar (Controls)|SearchBar]] has a background. For more information, see [[searchbar-style|SearchBar style on iOS]].
- Enabling the `Slider.Value` property to be set by tapping on a position on the [[Slider (Controls)|Slider]] bar, rather than by having to drag the [[Slider (Controls)|Slider]] thumb. For more information, see [[slider-thumb|Slider thumb tap on iOS]].
- Controlling the transition that's used when opening a [[SwipeView (Controls)|SwipeView]]. For more information, see [[swipeview-swipetransitionmode|SwipeView swipe transition mode on iOS]].
- Controlling when item selection occurs in a [[TimePicker (Controls)|TimePicker]]. For more information, see [[timepicker-selection|TimePicker item selection on iOS]].

The following platform-specific functionality is provided for .NET MAUI pages on iOS:

- Controlling whether the detail page of a [[FlyoutPage (Controls)|FlyoutPage]] has shadow applied to it, when revealing the flyout page. For more information, see [[flyoutpage-shadow|FlyoutPage shadow on iOS]].
- Controlling whether the navigation bar is translucent. For more information, see [[navigation-bar-translucent|Navigation bar translucency on iOS]].
- Controlling whether the page title is displayed as a large title in the page navigation bar. For more information, see [[page-large-title|Large page titles on iOS]].
- Disabling the safe area layout guide, which ensures that page content is positioned on an area of the screen that is safe for all iOS devices. For more information, see [[page-safe-area-layout|Disable the safe area layout guide on iOS]].
- Displaying a modal page as a popover. For more information, see [[page-popover|Display a modal page as a popover on iOS and Mac Catalyst]].
- Setting the visibility of the homage indicator on a [[Page (Controls)|Page]]. For more information, see [[page-home-indicator|Home indicator visibility on iOS]].
- Setting the status bar visibility on a [[Page (Controls)|Page]]. For more information, see [[page-status-bar-visibility|Page status bar visibility on iOS]].
- Setting the presentation style of modal pages. For more information, see [[page-presentation-style|Modal page presentation style on iOS]].
- Setting the translucency mode of the tab bar on a [[TabbedPage (Controls)|TabbedPage]]. For more information, see [[tabbedpage-translucent-tabbar|TabbedPage translucent TabBar on iOS]].

The following platform-specific functionality is provided for .NET MAUI layouts on iOS:

- Controlling whether a [[ScrollView (Controls)|ScrollView]] handles a touch gesture or passes it to its content. For more information, see [[scrollview-content-touches|ScrollView content touches on iOS]].

The following platform-specific functionality is provided for the .NET MAUI [[Application (Controls)|Application]] class on iOS:

- Enabling a [[PanGestureRecognizer|PanGestureRecognizer]] in a scrolling view to capture and share the pan gesture with the scrolling view. For more information, see [[application-pan-gesture|Simultaneous pan gesture recognition on iOS]].
