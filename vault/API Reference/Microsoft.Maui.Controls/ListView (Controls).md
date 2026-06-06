---
title: "ListView (Controls)"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ListView"
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

# ListView (Controls)

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.ListView`

An `ItemsView{T}` that displays a collection of data as a vertical list.

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
| [[ListView (Controls).ListView\|ListView]] | Controls whether anything happens in BeginRefresh(), is set based on RefreshCommand.CanExecute |

## Properties

| Name | Summary |
|---|---|
| [[ListView (Controls).CachingStrategy\|CachingStrategy]] | Gets or sets the binding to use for displaying the group header. |
| [[ListView (Controls).Footer\|Footer]] |  |
| [[ListView (Controls).FooterElement\|FooterElement]] |  |
| [[ListView (Controls).FooterTemplate\|FooterTemplate]] |  |
| [[ListView (Controls).GroupDisplayBinding\|GroupDisplayBinding]] |  |
| [[ListView (Controls).GroupHeaderTemplate\|GroupHeaderTemplate]] |  |
| [[ListView (Controls).GroupShortNameBinding\|GroupShortNameBinding]] |  |
| [[ListView (Controls).HasUnevenRows\|HasUnevenRows]] |  |
| [[ListView (Controls).Header\|Header]] |  |
| [[ListView (Controls).HeaderElement\|HeaderElement]] |  |
| [[ListView (Controls).HeaderTemplate\|HeaderTemplate]] |  |
| [[ListView (Controls).HorizontalScrollBarVisibility\|HorizontalScrollBarVisibility]] |  |
| [[ListView (Controls).IsGroupingEnabled\|IsGroupingEnabled]] |  |
| [[ListView (Controls).IsPullToRefreshEnabled\|IsPullToRefreshEnabled]] |  |
| [[ListView (Controls).IsRefreshing\|IsRefreshing]] |  |
| [[ListView (Controls).RefreshAllowed\|RefreshAllowed]] |  |
| [[ListView (Controls).RefreshCommand\|RefreshCommand]] |  |
| [[ListView (Controls).RefreshControlColor\|RefreshControlColor]] |  |
| [[ListView (Controls).RowHeight\|RowHeight]] |  |
| [[ListView (Controls).SelectedItem\|SelectedItem]] |  |
| [[ListView (Controls).SelectionMode\|SelectionMode]] |  |
| [[ListView (Controls).SeparatorColor\|SeparatorColor]] |  |
| [[ListView (Controls).SeparatorVisibility\|SeparatorVisibility]] |  |
| [[ListView (Controls).VerticalScrollBarVisibility\|VerticalScrollBarVisibility]] |  |

## Methods

| Name | Summary |
|---|---|
| [[ListView (Controls).BeginRefresh\|BeginRefresh]] | Enters the refreshing state by setting the `IsRefreshing` property to `true`. |
| [[ListView (Controls).CreateDefault\|CreateDefault]] |  |
| [[ListView (Controls).CreateDefaultCell\|CreateDefaultCell]] | Internal API for Microsoft.Maui.Controls platform use. |
| [[ListView (Controls).EndRefresh\|EndRefresh]] | Exits the refreshing state by setting the `IsRefreshing` property to `false`. |
| [[ListView (Controls).GetDisplayTextFromGroup\|GetDisplayTextFromGroup]] | Internal API for Microsoft.Maui.Controls platform use. |
| [[ListView (Controls).NotifyRowTapped\|NotifyRowTapped]] | Notifies that a row was tapped at the specified group and item index. |
| [[ListView (Controls).On{T}\|On<T>]] |  |
| [[ListView (Controls).OnBindingContextChanged\|OnBindingContextChanged]] | Gets or sets the string, binding, or view that will be displayed at the bottom of the list view. This is a bindable property. |
| [[ListView (Controls).OnMeasure\|OnMeasure]] |  |
| [[ListView (Controls).ScrollTo\|ScrollTo]] | Scrolls to the specified item with the specified scroll position and animation setting. |
| [[ListView (Controls).SendCellAppearing\|SendCellAppearing]] | Internal API for Microsoft.Maui.Controls platform use. |
| [[ListView (Controls).SendCellDisappearing\|SendCellDisappearing]] | Internal API for Microsoft.Maui.Controls platform use. |
| [[ListView (Controls).SendRefreshing\|SendRefreshing]] | Internal API for Microsoft.Maui.Controls platform use. |
| [[ListView (Controls).SendScrolled\|SendScrolled]] |  |
| [[ListView (Controls).SetupContent\|SetupContent]] |  |
| [[ListView (Controls).UnhookContent\|UnhookContent]] |  |
| [[ListView (Controls).ValidateItemTemplate\|ValidateItemTemplate]] |  |

## Events

| Name | Summary |
|---|---|
| [[ListView (Controls).ItemAppearing\|ItemAppearing]] |  |
| [[ListView (Controls).ItemDisappearing\|ItemDisappearing]] |  |
| [[ListView (Controls).ItemSelected\|ItemSelected]] |  |
| [[ListView (Controls).ItemTapped\|ItemTapped]] |  |
| [[ListView (Controls).Refreshing\|Refreshing]] |  |
| [[ListView (Controls).ScrollToRequested\|ScrollToRequested]] |  |
| [[ListView (Controls).Scrolled\|Scrolled]] |  |

## Fields

| Name | Summary |
|---|---|
| [[ListView (Controls).FooterProperty\|FooterProperty]] | Bindable property for `Footer`. |
| [[ListView (Controls).FooterTemplateProperty\|FooterTemplateProperty]] | Bindable property for `FooterTemplate`. |
| [[ListView (Controls).GroupHeaderTemplateProperty\|GroupHeaderTemplateProperty]] | Bindable property for `GroupHeaderTemplate`. |
| [[ListView (Controls).HasUnevenRowsProperty\|HasUnevenRowsProperty]] | Bindable property for `HasUnevenRows`. |
| [[ListView (Controls).HeaderProperty\|HeaderProperty]] | Bindable property for `Header`. |
| [[ListView (Controls).HeaderTemplateProperty\|HeaderTemplateProperty]] | Bindable property for `HeaderTemplate`. |
| [[ListView (Controls).HorizontalScrollBarVisibilityProperty\|HorizontalScrollBarVisibilityProperty]] | Bindable property for `HorizontalScrollBarVisibility`. |
| [[ListView (Controls).IsGroupingEnabledProperty\|IsGroupingEnabledProperty]] | Bindable property for `IsGroupingEnabled`. |
| [[ListView (Controls).IsPullToRefreshEnabledProperty\|IsPullToRefreshEnabledProperty]] | Bindable property for `IsPullToRefreshEnabled`. |
| [[ListView (Controls).IsRefreshingProperty\|IsRefreshingProperty]] | Bindable property for `IsRefreshing`. |
| [[ListView (Controls).RefreshCommandProperty\|RefreshCommandProperty]] | Bindable property for `RefreshCommand`. |
| [[ListView (Controls).RefreshControlColorProperty\|RefreshControlColorProperty]] | Bindable property for `RefreshControlColor`. |
| [[ListView (Controls).RowHeightProperty\|RowHeightProperty]] | Bindable property for `RowHeight`. |
| [[ListView (Controls).SelectedItemProperty\|SelectedItemProperty]] | Bindable property for `SelectedItem`. |
| [[ListView (Controls).SelectionModeProperty\|SelectionModeProperty]] | Bindable property for `SelectionMode`. |
| [[ListView (Controls).SeparatorColorProperty\|SeparatorColorProperty]] | Bindable property for `SeparatorColor`. |
| [[ListView (Controls).SeparatorVisibilityProperty\|SeparatorVisibilityProperty]] | Bindable property for `SeparatorVisibility`. |
| [[ListView (Controls).VerticalScrollBarVisibilityProperty\|VerticalScrollBarVisibilityProperty]] | Bindable property for `VerticalScrollBarVisibility`. |

## Guide

- 📖 Conceptual: [[listview]]

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.listview)
