---
title: "Element"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Element"
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

# Element

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.Element`

Provides the base class for all Microsoft.Maui.Controls hierarchal elements. This class contains all the methods and properties required to represent an element in the Microsoft.Maui.Controls hierarchy.

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
| [[Element.Element\|Element]] |  |

## Properties

| Name | Summary |
|---|---|
| [[Element.AutomationId\|AutomationId]] |  |
| [[Element.ClassId\|ClassId]] |  |
| [[Element.EffectControlProvider\|EffectControlProvider]] |  |
| [[Element.Effects\|Effects]] |  |
| [[Element.Handler\|Handler]] |  |
| [[Element.Id\|Id]] |  |
| [[Element.LogicalChildren\|LogicalChildren]] |  |
| [[Element.Parent\|Parent]] |  |
| [[Element.RealParent\|RealParent]] |  |
| [[Element.StyleId\|StyleId]] |  |

## Methods

| Name | Summary |
|---|---|
| [[Element.AddLogicalChild\|AddLogicalChild]] | Adds an `Element` to the logical children. |
| [[Element.ClearLogicalChildren\|ClearLogicalChildren]] | Removes all child `Element`s. |
| [[Element.EffectIsAttached\|EffectIsAttached]] | For internal use by .NET MAUI. |
| [[Element.FindByName\|FindByName]] | Returns the element that has the specified name. |
| [[Element.InsertLogicalChild\|InsertLogicalChild]] | Inserts an `Element` to the logical children at the specified index. |
| [[Element.MapAutomationPropertiesExcludedWithChildren\|MapAutomationPropertiesExcludedWithChildren]] | Maps the abstract `ExcludedWithChildrenProperty` property to the platform-specific implementations. |
| [[Element.MapAutomationPropertiesHelpText\|MapAutomationPropertiesHelpText]] |  |
| [[Element.MapAutomationPropertiesIsInAccessibleTree\|MapAutomationPropertiesIsInAccessibleTree]] | Maps the abstract `IsInAccessibleTreeProperty` property to the platform-specific implementations. |
| [[Element.MapAutomationPropertiesLabeledBy\|MapAutomationPropertiesLabeledBy]] |  |
| [[Element.MapAutomationPropertiesName\|MapAutomationPropertiesName]] |  |
| [[Element.OnBindingContextChanged\|OnBindingContextChanged]] | Invoked whenever the binding context of the element changes. Implement this method to add class handling for this event. |
| [[Element.OnChildAdded\|OnChildAdded]] | Raises the `ChildAdded` event. Implement this method to add class handling for this event. |
| [[Element.OnChildRemoved\|OnChildRemoved]] | Raises the `ChildRemoved` event. Implement this method to add class handling for this event |
| [[Element.OnHandlerChanged\|OnHandlerChanged]] | When overridden in a derived class, should raise the `HandlerChanged` event. |
| [[Element.OnHandlerChanging\|OnHandlerChanging]] | When overridden in a derived class, should raise the `HandlerChanging` event. |
| [[Element.OnParentChanged\|OnParentChanged]] | When overridden in a derived class, should raise the `ParentChanged` event. |
| [[Element.OnParentChanging\|OnParentChanging]] | When overridden in a derived class, should raise the `ParentChanging` event. |
| [[Element.OnParentSet\|OnParentSet]] | Raises the (internal) ParentSet event. Implement this method in order to add behavior when the element is added to a parent. |
| [[Element.OnPropertyChanged\|OnPropertyChanged]] | Method that is called when a bound property is changed. |
| [[Element.RemoveDynamicResource\|RemoveDynamicResource]] | Removes a previously set dynamic resource. |
| [[Element.RemoveLogicalChild\|RemoveLogicalChild]] | Removes the first occurrence of a specific `Element` from the logical children. |
| [[Element.SetDynamicResource\|SetDynamicResource]] | Sets the `BindableProperty` property of this element to be updated via the DynamicResource with the provided key. |
| [[Element.SetValueFromRenderer\|SetValueFromRenderer]] | For internal use by .NET MAUI. |

## Events

| Name | Summary |
|---|---|
| [[Element.ChildAdded\|ChildAdded]] |  |
| [[Element.ChildRemoved\|ChildRemoved]] |  |
| [[Element.DescendantAdded\|DescendantAdded]] |  |
| [[Element.DescendantRemoved\|DescendantRemoved]] |  |
| [[Element.HandlerChanged\|HandlerChanged]] | Raised whenever the element's handler has changed. |
| [[Element.HandlerChanging\|HandlerChanging]] |  |
| [[Element.ParentChanged\|ParentChanged]] | Raised whenever the element's parent has changed. |
| [[Element.ParentChanging\|ParentChanging]] |  |

## Fields

| Name | Summary |
|---|---|
| [[Element.AutomationIdProperty\|AutomationIdProperty]] | Bindable property for `AutomationId`. |
| [[Element.ClassIdProperty\|ClassIdProperty]] | Bindable property for `ClassId`. |
| [[Element.transientNamescope\|transientNamescope]] |  |

## Remarks

Important categories of visual elements are noted in the following table: Class Description `VisualElement` An `Element` that occupies an area on the screen, has a visual appearance, and can obtain touch input. Cells are elements meant to be added to `ListView` or `TableView`. A that occupies most or all of the screen and contains a single child. have a single child of type `View`, while subclasses of have a collection of multiple children views, including other layouts. Controls and specialized `View` The lower part of the diagram shows the Microsoft.Maui.Controls classes for universally-available controls, such as and `TableView`.

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.element)
