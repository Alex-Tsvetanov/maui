---
title: "View"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.View"
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

# View

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.View`

A visual element that is used to place layouts and controls on the screen.

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
| [[View.View\|View]] | Initializes a new instance of the `View` class |

## Properties

| Name | Summary |
|---|---|
| [[View.GestureController\|GestureController]] |  |
| [[View.GestureRecognizers\|GestureRecognizers]] |  |
| [[View.HorizontalOptions\|HorizontalOptions]] |  |
| [[View.Margin\|Margin]] |  |
| [[View.VerticalOptions\|VerticalOptions]] |  |

## Methods

| Name | Summary |
|---|---|
| [[View.ChangeVisualState\|ChangeVisualState]] |  |
| [[View.GetChildElements\|GetChildElements]] |  |
| [[View.GetRendererOverrides{T}\|GetRendererOverrides<T>]] |  |
| [[View.OnBindingContextChanged\|OnBindingContextChanged]] | Gets or sets the `LayoutOptions` that define how the element gets arranged in a layout cycle. This is a bindable property. |

## Fields

| Name | Summary |
|---|---|
| [[View.HorizontalOptionsProperty\|HorizontalOptionsProperty]] | Bindable property for `HorizontalOptions`. |
| [[View.MarginProperty\|MarginProperty]] | Bindable property for `Margin`. |
| [[View.VerticalOptionsProperty\|VerticalOptionsProperty]] | Bindable property for `VerticalOptions`. |
| [[View.propertyMapper\|propertyMapper]] | Represents the view's internal `PropertyMapper`. |

## Remarks

This is the base class for `Layout` and most of the controls. Because `View` ultimately inherits from `BindableObject`, application developers can use the Model-View-ViewModel architecture, as well as XAML, to develop portable user interfaces.

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.view)
