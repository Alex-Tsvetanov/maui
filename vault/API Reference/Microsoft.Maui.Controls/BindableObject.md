---
title: "BindableObject"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.BindableObject"
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

# BindableObject

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.BindableObject`

Provides a mechanism to propagate data changes from one object to another. Enables validation, type coercion, and an event system.

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
| [[BindableObject.BindableObject\|BindableObject]] | Initializes a new instance of the `BindableObject` class. |

## Properties

| Name | Summary |
|---|---|
| [[BindableObject.BindingContext\|BindingContext]] |  |
| [[BindableObject.Dispatcher\|Dispatcher]] | Gets the dispatcher that was available when this bindable object was created, otherwise tries to find the nearest available dispatcher (probably the window's… |

## Methods

| Name | Summary |
|---|---|
| [[BindableObject.ApplyBindings\|ApplyBindings]] | Applies all the current bindings to `BindingContext`. |
| [[BindableObject.ClearValue\|ClearValue]] | Clears any value that is previously set for a bindable property. |
| [[BindableObject.CoerceValue\|CoerceValue]] | Coerces the value of the specified bindable property. This is done by invoking `CoerceValueDelegate` of the specified bindable property. |
| [[BindableObject.GetValue\|GetValue]] | Returns the value that is contained in the given bindable property. |
| [[BindableObject.IsSet\|IsSet]] | Determines whether or not a bindable property exists and has a value set. |
| [[BindableObject.OnBindingContextChanged\|OnBindingContextChanged]] | Raises the `BindingContextChanged` event. |
| [[BindableObject.OnPropertyChanged\|OnPropertyChanged]] | Raises the `PropertyChanged` event. |
| [[BindableObject.OnPropertyChanging\|OnPropertyChanging]] | Raises the `PropertyChanging` event. |
| [[BindableObject.RemoveBinding\|RemoveBinding]] | Removes a previously set binding from a bindable property. |
| [[BindableObject.SetBinding\|SetBinding]] | Assigns a binding to a bindable property. |
| [[BindableObject.SetInheritedBindingContext\|SetInheritedBindingContext]] | Sets the inherited context to a nested element. |
| [[BindableObject.SetValue\|SetValue]] | Sets the value of the specified bindable property. |
| [[BindableObject.UnapplyBindings\|UnapplyBindings]] | Removes all current bindings from the current context. |

## Events

| Name | Summary |
|---|---|
| [[BindableObject.BindingContextChanged\|BindingContextChanged]] | Occurs when the value of the `BindingContext` property changes. |
| [[BindableObject.PropertyChanged\|PropertyChanged]] | Gets or sets an object that contains the properties that will be targeted by the bound properties that belong to this `BindableObject`. This is a bindable pr… |
| [[BindableObject.PropertyChanging\|PropertyChanging]] | Occurs when a property value is changing. |

## Fields

| Name | Summary |
|---|---|
| [[BindableObject.BindingContextProperty\|BindingContextProperty]] | Bindable property for `BindingContext`. |

## Remarks

The `BindableObject` class provides a data storage mechanism that enables the application developer to synchronize data between objects in response to changes, for example, between the View and View Model in the MVVM design pattern. All of the visual elements in the Microsoft.Maui.Controls namespace inherit from `BindableObject` class, so they can all be used to bind the data behind their user interface.

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.bindableobject)
