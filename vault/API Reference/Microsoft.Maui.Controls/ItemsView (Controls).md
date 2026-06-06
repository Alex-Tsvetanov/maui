---
title: "ItemsView (Controls)"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ItemsView"
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

# ItemsView (Controls)

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.ItemsView`

A `View` that serves as a base class for views that contain a templated list of items.

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
| [[ItemsView (Controls).ItemsView\|ItemsView]] |  |

## Properties

| Name | Summary |
|---|---|
| [[ItemsView (Controls).EmptyView\|EmptyView]] |  |
| [[ItemsView (Controls).EmptyViewTemplate\|EmptyViewTemplate]] |  |
| [[ItemsView (Controls).HorizontalScrollBarVisibility\|HorizontalScrollBarVisibility]] |  |
| [[ItemsView (Controls).InternalItemsLayout\|InternalItemsLayout]] |  |
| [[ItemsView (Controls).ItemTemplate\|ItemTemplate]] |  |
| [[ItemsView (Controls).ItemsSource\|ItemsSource]] |  |
| [[ItemsView (Controls).ItemsUpdatingScrollMode\|ItemsUpdatingScrollMode]] |  |
| [[ItemsView (Controls).RemainingItemsThreshold\|RemainingItemsThreshold]] |  |
| [[ItemsView (Controls).RemainingItemsThresholdReachedCommand\|RemainingItemsThresholdReachedCommand]] |  |
| [[ItemsView (Controls).RemainingItemsThresholdReachedCommandParameter\|RemainingItemsThresholdReachedCommandParameter]] |  |
| [[ItemsView (Controls).VerticalScrollBarVisibility\|VerticalScrollBarVisibility]] |  |

## Methods

| Name | Summary |
|---|---|
| [[ItemsView (Controls).OnBindingContextChanged\|OnBindingContextChanged]] |  |
| [[ItemsView (Controls).OnMeasure\|OnMeasure]] |  |
| [[ItemsView (Controls).OnRemainingItemsThresholdReached\|OnRemainingItemsThresholdReached]] |  |
| [[ItemsView (Controls).OnScrollToRequested\|OnScrollToRequested]] |  |
| [[ItemsView (Controls).OnScrolled\|OnScrolled]] |  |
| [[ItemsView (Controls).ScrollTo\|ScrollTo]] | Gets or sets the scroll behavior when items are added, removed, or updated in the collection. |
| [[ItemsView (Controls).SendRemainingItemsThresholdReached\|SendRemainingItemsThresholdReached]] | Manually triggers the remaining items threshold behavior. |
| [[ItemsView (Controls).SendScrolled\|SendScrolled]] |  |

## Events

| Name | Summary |
|---|---|
| [[ItemsView (Controls).RemainingItemsThresholdReached\|RemainingItemsThresholdReached]] |  |
| [[ItemsView (Controls).ScrollToRequested\|ScrollToRequested]] |  |
| [[ItemsView (Controls).Scrolled\|Scrolled]] |  |

## Fields

| Name | Summary |
|---|---|
| [[ItemsView (Controls).EmptyViewProperty\|EmptyViewProperty]] | Bindable property for `EmptyView`. |
| [[ItemsView (Controls).EmptyViewTemplateProperty\|EmptyViewTemplateProperty]] | Gets or sets the view or object to display when the `ItemsSource` is empty or `null`. |
| [[ItemsView (Controls).HorizontalScrollBarVisibilityProperty\|HorizontalScrollBarVisibilityProperty]] | Gets or sets the parameter to pass to the `RemainingItemsThresholdReachedCommand`. |
| [[ItemsView (Controls).ItemTemplateProperty\|ItemTemplateProperty]] | Bindable property for `ItemTemplate`. |
| [[ItemsView (Controls).ItemsSourceProperty\|ItemsSourceProperty]] | Bindable property for `ItemsSource`. |
| [[ItemsView (Controls).ItemsUpdatingScrollModeProperty\|ItemsUpdatingScrollModeProperty]] | Gets or sets the `DataTemplate` used to render each item in the collection. |
| [[ItemsView (Controls).RemainingItemsThresholdProperty\|RemainingItemsThresholdProperty]] | Gets or sets the visibility of the vertical scroll bar. |
| [[ItemsView (Controls).RemainingItemsThresholdReachedCommandParameterProperty\|RemainingItemsThresholdReachedCommandParameterProperty]] | Gets or sets the command to execute when the remaining items threshold is reached during scrolling. |
| [[ItemsView (Controls).RemainingItemsThresholdReachedCommandProperty\|RemainingItemsThresholdReachedCommandProperty]] | Gets or sets the collection of items to be displayed. |
| [[ItemsView (Controls).VerticalScrollBarVisibilityProperty\|VerticalScrollBarVisibilityProperty]] | Gets or sets the visibility of the horizontal scroll bar. |

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.itemsview)
