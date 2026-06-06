---
title: "CarouselView"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.CarouselView"
namespace: "Microsoft.Maui.Controls"
kind: class
platforms:
  - All platforms (.NET)
  - Android
  - iOS
  - Mac Catalyst
  - Windows
  - Tizen
  - .NET Standard
assemblies:
  - Controls
---

# CarouselView

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.CarouselView`

A view that presents a scrollable collection of items where each item 'snaps' into place after scrolling.

## Platforms

| Platform | Available |
|---|---|
| All platforms (.NET) | ✅ |
| Android | ✅ |
| iOS | ✅ |
| Mac Catalyst | ✅ |
| Windows | ✅ |
| Tizen | ✅ |
| .NET Standard | ✅ |


## Constructors

| Name | Summary |
|---|---|
| [[CarouselView.CarouselView\|CarouselView]] | Initializes a new instance of the `CarouselView` class. |

## Properties

| Name | Summary |
|---|---|
| [[CarouselView.AnimateCurrentItemChanges\|AnimateCurrentItemChanges]] | Gets a value indicating whether current item changes should be animated. For internal use by platform renderers. |
| [[CarouselView.AnimatePositionChanges\|AnimatePositionChanges]] | Gets a value indicating whether position changes should be animated. For internal use by platform renderers. |
| [[CarouselView.CurrentItem\|CurrentItem]] |  |
| [[CarouselView.CurrentItemChangedCommand\|CurrentItemChangedCommand]] |  |
| [[CarouselView.CurrentItemChangedCommandParameter\|CurrentItemChangedCommandParameter]] |  |
| [[CarouselView.IndicatorView\|IndicatorView]] |  |
| [[CarouselView.IsBounceEnabled\|IsBounceEnabled]] |  |
| [[CarouselView.IsDragging\|IsDragging]] | Gets a value indicating whether the user is currently dragging the carousel. |
| [[CarouselView.IsScrollAnimated\|IsScrollAnimated]] |  |
| [[CarouselView.IsScrolling\|IsScrolling]] | Gets or sets the layout used to arrange items in the carousel. |
| [[CarouselView.IsSwipeEnabled\|IsSwipeEnabled]] |  |
| [[CarouselView.ItemsLayout\|ItemsLayout]] |  |
| [[CarouselView.Loop\|Loop]] |  |
| [[CarouselView.PeekAreaInsets\|PeekAreaInsets]] |  |
| [[CarouselView.Position\|Position]] |  |
| [[CarouselView.PositionChangedCommand\|PositionChangedCommand]] |  |
| [[CarouselView.PositionChangedCommandParameter\|PositionChangedCommandParameter]] |  |
| [[CarouselView.VisibleViews\|VisibleViews]] |  |

## Methods

| Name | Summary |
|---|---|
| [[CarouselView.OnCurrentItemChanged\|OnCurrentItemChanged]] | Called when the current item changes. Override this method to add custom logic when the current item changes. |
| [[CarouselView.OnPositionChanged\|OnPositionChanged]] | Called when the position changes. Override this method to add custom logic when the carousel position changes. |
| [[CarouselView.SetIsDragging\|SetIsDragging]] | Sets the dragging state of the carousel. For internal use by platform renderers. |

## Events

| Name | Summary |
|---|---|
| [[CarouselView.CurrentItemChanged\|CurrentItemChanged]] |  |
| [[CarouselView.PositionChanged\|PositionChanged]] |  |

## Fields

| Name | Summary |
|---|---|
| [[CarouselView.CurrentItemChangedCommandParameterProperty\|CurrentItemChangedCommandParameterProperty]] | Bindable property for `CurrentItemChangedCommandParameter`. |
| [[CarouselView.CurrentItemChangedCommandProperty\|CurrentItemChangedCommandProperty]] | Bindable property for `CurrentItemChangedCommand`. |
| [[CarouselView.CurrentItemProperty\|CurrentItemProperty]] | Gets or sets a value indicating whether scrolling between items is animated. |
| [[CarouselView.IsBounceEnabledProperty\|IsBounceEnabledProperty]] | Bindable property for `IsBounceEnabled`. |
| [[CarouselView.IsDraggingProperty\|IsDraggingProperty]] | Bindable property for `IsDragging`. |
| [[CarouselView.IsScrollAnimatedProperty\|IsScrollAnimatedProperty]] | Gets or sets a value indicating whether swipe gestures are enabled for navigation. |
| [[CarouselView.IsSwipeEnabledProperty\|IsSwipeEnabledProperty]] | Gets or sets a value indicating whether bounce effects are enabled when scrolling reaches the end of the carousel. |
| [[CarouselView.ItemsLayoutProperty\|ItemsLayoutProperty]] | Gets or sets the index of the currently displayed item in the carousel. |
| [[CarouselView.LoopProperty\|LoopProperty]] | Bindable property for `Loop`. |
| [[CarouselView.PeekAreaInsetsProperty\|PeekAreaInsetsProperty]] | Gets or sets a value indicating whether the carousel loops back to the first item after reaching the last item. |
| [[CarouselView.PositionChangedCommandParameterProperty\|PositionChangedCommandParameterProperty]] | Bindable property for `PositionChangedCommandParameter`. |
| [[CarouselView.PositionChangedCommandProperty\|PositionChangedCommandProperty]] | Bindable property for `PositionChangedCommand`. |
| [[CarouselView.PositionProperty\|PositionProperty]] | Gets or sets the currently displayed item in the carousel. |
| [[CarouselView.VisibleViewsProperty\|VisibleViewsProperty]] | Gets or sets the amount of space to reserve on each side of the current item to show a peek of adjacent items. |

## Constants

| Name | Summary |
|---|---|
| [[CarouselView.CurrentItemVisualState\|CurrentItemVisualState]] | Visual state name for the current item in the carousel. |
| [[CarouselView.DefaultItemVisualState\|DefaultItemVisualState]] | Visual state name for items that are neither current, next, nor previous. |
| [[CarouselView.NextItemVisualState\|NextItemVisualState]] | Visual state name for the next item in the carousel. |
| [[CarouselView.PreviousItemVisualState\|PreviousItemVisualState]] | Visual state name for the previous item in the carousel. |

## Remarks

`CarouselView` is useful for displaying a horizontal or vertical carousel of items, where the user can swipe through items and each item snaps into view. Unlike `CollectionView`, `CarouselView` enforces single-item snap points and provides additional features like looping and position tracking.

## Guide

- 📖 Conceptual: [[carouselview]]

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.carouselview)
