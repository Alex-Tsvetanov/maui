---
title: "VisualElement (Controls)"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.VisualElement"
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

# VisualElement (Controls)

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.VisualElement`

Provides the base class for all visual elements in .NET MAUI.

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
| [[VisualElement (Controls).VisualElement\|VisualElement]] | Initializes a new instance of the `VisualElement` class. |

## Properties

| Name | Summary |
|---|---|
| [[VisualElement (Controls).AnchorX\|AnchorX]] |  |
| [[VisualElement (Controls).AnchorY\|AnchorY]] |  |
| [[VisualElement (Controls).Background\|Background]] |  |
| [[VisualElement (Controls).BackgroundColor\|BackgroundColor]] |  |
| [[VisualElement (Controls).Batched\|Batched]] | Gets the current width of this element in device-independent units. This is a read-only bindable property. |
| [[VisualElement (Controls).Behaviors\|Behaviors]] |  |
| [[VisualElement (Controls).Bounds\|Bounds]] |  |
| [[VisualElement (Controls).Clip\|Clip]] |  |
| [[VisualElement (Controls).DesiredSize\|DesiredSize]] | Gets or sets the front-to-back z-index of an element within a layout. This is a bindable property. |
| [[VisualElement (Controls).DisableLayout\|DisableLayout]] | Gets a value that indicates that layout for this element is disabled. |
| [[VisualElement (Controls).FlowDirection\|FlowDirection]] |  |
| [[VisualElement (Controls).Frame\|Frame]] |  |
| [[VisualElement (Controls).Handler\|Handler]] |  |
| [[VisualElement (Controls).Height\|Height]] |  |
| [[VisualElement (Controls).HeightRequest\|HeightRequest]] |  |
| [[VisualElement (Controls).InputTransparent\|InputTransparent]] |  |
| [[VisualElement (Controls).IsEnabled\|IsEnabled]] |  |
| [[VisualElement (Controls).IsEnabledCore\|IsEnabledCore]] |  |
| [[VisualElement (Controls).IsFocused\|IsFocused]] | Gets a value indicating whether this element is focused currently. This is a bindable property. |
| [[VisualElement (Controls).IsInPlatformLayout\|IsInPlatformLayout]] |  |
| [[VisualElement (Controls).IsLoaded\|IsLoaded]] |  |
| [[VisualElement (Controls).IsPlatformEnabled\|IsPlatformEnabled]] |  |
| [[VisualElement (Controls).IsPlatformStateConsistent\|IsPlatformStateConsistent]] |  |
| [[VisualElement (Controls).IsVisible\|IsVisible]] |  |
| [[VisualElement (Controls).IsVisibleCore\|IsVisibleCore]] |  |
| [[VisualElement (Controls).MaximumHeightRequest\|MaximumHeightRequest]] |  |
| [[VisualElement (Controls).MaximumWidthRequest\|MaximumWidthRequest]] |  |
| [[VisualElement (Controls).MinimumHeightRequest\|MinimumHeightRequest]] |  |
| [[VisualElement (Controls).MinimumWidthRequest\|MinimumWidthRequest]] |  |
| [[VisualElement (Controls).Opacity\|Opacity]] |  |
| [[VisualElement (Controls).Resources\|Resources]] |  |
| [[VisualElement (Controls).Rotation\|Rotation]] |  |
| [[VisualElement (Controls).RotationX\|RotationX]] |  |
| [[VisualElement (Controls).RotationY\|RotationY]] |  |
| [[VisualElement (Controls).Scale\|Scale]] |  |
| [[VisualElement (Controls).ScaleX\|ScaleX]] |  |
| [[VisualElement (Controls).ScaleY\|ScaleY]] |  |
| [[VisualElement (Controls).Shadow\|Shadow]] |  |
| [[VisualElement (Controls).TranslationX\|TranslationX]] |  |
| [[VisualElement (Controls).TranslationY\|TranslationY]] |  |
| [[VisualElement (Controls).Triggers\|Triggers]] |  |
| [[VisualElement (Controls).Visual\|Visual]] |  |
| [[VisualElement (Controls).Width\|Width]] |  |
| [[VisualElement (Controls).WidthRequest\|WidthRequest]] |  |
| [[VisualElement (Controls).Window\|Window]] | Gets the `Window` that is associated with an element. This is a read-only bindable property. |
| [[VisualElement (Controls).X\|X]] |  |
| [[VisualElement (Controls).Y\|Y]] |  |
| [[VisualElement (Controls).ZIndex\|ZIndex]] |  |

## Methods

| Name | Summary |
|---|---|
| [[VisualElement (Controls).Arrange\|Arrange]] | Positions child objects and determines a size for an element. |
| [[VisualElement (Controls).ArrangeOverride\|ArrangeOverride]] | Allows subclasses to implement custom Arrange logic during a controls layout pass. |
| [[VisualElement (Controls).BatchBegin\|BatchBegin]] | Gets or sets a value that indicates that this element is currently going through the platform layout cycle. |
| [[VisualElement (Controls).BatchCommit\|BatchCommit]] | Signals the end of a batch of commands to the element and that those commands should now be committed. |
| [[VisualElement (Controls).ChangeVisualState\|ChangeVisualState]] | Changes the current visual state based on this elements current property values. |
| [[VisualElement (Controls).ComputeConstraintForView\|ComputeConstraintForView]] |  |
| [[VisualElement (Controls).Focus\|Focus]] | Attempts to set focus to this element. |
| [[VisualElement (Controls).InvalidateMeasure\|InvalidateMeasure]] | Marks the current measure of an element as invalidated. |
| [[VisualElement (Controls).InvalidateMeasureNonVirtual\|InvalidateMeasureNonVirtual]] | Invalidates the measure of an element. |
| [[VisualElement (Controls).InvalidateMeasureOverride\|InvalidateMeasureOverride]] | Provides a way to allow subclasses (e.g., Layout) to override `InvalidateMeasure` |
| [[VisualElement (Controls).Layout\|Layout]] | Updates the bounds of the element during the layout cycle. |
| [[VisualElement (Controls).MapAccessKey\|MapAccessKey]] |  |
| [[VisualElement (Controls).MapAccessKeyHorizontalOffset\|MapAccessKeyHorizontalOffset]] |  |
| [[VisualElement (Controls).MapAccessKeyPlacement\|MapAccessKeyPlacement]] |  |
| [[VisualElement (Controls).MapAccessKeyVerticalOffset\|MapAccessKeyVerticalOffset]] |  |
| [[VisualElement (Controls).MapBackgroundColor\|MapBackgroundColor]] |  |
| [[VisualElement (Controls).MapBackgroundImageSource\|MapBackgroundImageSource]] |  |
| [[VisualElement (Controls).Measure\|Measure]] | Returns the minimum size that an element needs in order to be displayed on the device. Margins are excluded from the measurement, but returned with the size.… |
| [[VisualElement (Controls).MeasureOverride\|MeasureOverride]] | Allows subclasses to implement custom Measure logic during a controls measure pass. |
| [[VisualElement (Controls).OnBindingContextChanged\|OnBindingContextChanged]] | Invoked when the binding context of this element has changed. |
| [[VisualElement (Controls).OnChildAdded\|OnChildAdded]] | Invoked when a child object is added to this element. |
| [[VisualElement (Controls).OnChildRemoved\|OnChildRemoved]] | Invoked when a child object is removed from this element. |
| [[VisualElement (Controls).OnChildrenReordered\|OnChildrenReordered]] | Raises the `ChildrenReordered` event. |
| [[VisualElement (Controls).OnMeasure\|OnMeasure]] | Method that is called when a layout measurement happens. |
| [[VisualElement (Controls).OnSizeAllocated\|OnSizeAllocated]] | Method that is called when the size of the element is set during a layout cycle. Implement this method to add class handling for this event. |
| [[VisualElement (Controls).PlatformSizeChanged\|PlatformSizeChanged]] | Gets or sets the local resource dictionary. |
| [[VisualElement (Controls).RefreshIsEnabledProperty\|RefreshIsEnabledProperty]] | This method must always be called if some event occurs and the value of the `IsEnabledCore` property will change. |
| [[VisualElement (Controls).RefreshIsVisibleProperty\|RefreshIsVisibleProperty]] |  |
| [[VisualElement (Controls).SizeAllocated\|SizeAllocated]] | Method that is called during a layout cycle to signal the start of a sub-tree layout. |
| [[VisualElement (Controls).Unfocus\|Unfocus]] | Unsets keyboard focus on this element. |
| [[VisualElement (Controls).~VisualElement\|~VisualElement]] |  |

## Events

| Name | Summary |
|---|---|
| [[VisualElement (Controls).BatchCommitted\|BatchCommitted]] |  |
| [[VisualElement (Controls).ChildrenReordered\|ChildrenReordered]] | Occurs when the order of this element's children changes. |
| [[VisualElement (Controls).FocusChangeRequested\|FocusChangeRequested]] |  |
| [[VisualElement (Controls).Focused\|Focused]] |  |
| [[VisualElement (Controls).Loaded\|Loaded]] |  |
| [[VisualElement (Controls).MeasureInvalidated\|MeasureInvalidated]] | Occurs when the current measure of an element has been invalidated. |
| [[VisualElement (Controls).SizeChanged\|SizeChanged]] | Occurs when the size of an element changed. |
| [[VisualElement (Controls).Unfocused\|Unfocused]] |  |
| [[VisualElement (Controls).Unloaded\|Unloaded]] |  |

## Fields

| Name | Summary |
|---|---|
| [[VisualElement (Controls).AnchorXProperty\|AnchorXProperty]] | Bindable property for `AnchorX`. |
| [[VisualElement (Controls).AnchorYProperty\|AnchorYProperty]] | Bindable property for `AnchorY`. |
| [[VisualElement (Controls).BackgroundColorProperty\|BackgroundColorProperty]] | Bindable property for `BackgroundColor`. |
| [[VisualElement (Controls).BackgroundProperty\|BackgroundProperty]] | Bindable property for `Background`. |
| [[VisualElement (Controls).BehaviorsProperty\|BehaviorsProperty]] | Bindable property for `Behaviors`. |
| [[VisualElement (Controls).ClipProperty\|ClipProperty]] | Bindable property for `Clip`. |
| [[VisualElement (Controls).FlowDirectionProperty\|FlowDirectionProperty]] | Bindable property for `FlowDirection`. |
| [[VisualElement (Controls).HeightProperty\|HeightProperty]] | Bindable property for `Height`. |
| [[VisualElement (Controls).HeightRequestProperty\|HeightRequestProperty]] | Bindable property for `HeightRequest`. |
| [[VisualElement (Controls).InputTransparentProperty\|InputTransparentProperty]] | Bindable property for `InputTransparent`. |
| [[VisualElement (Controls).IsEnabledProperty\|IsEnabledProperty]] | Gets the explicit value of `IsEnabled` set directly on this element, before coercion by `IsEnabledCore` which factors in parent state. |
| [[VisualElement (Controls).IsFocusedProperty\|IsFocusedProperty]] | Bindable property for `IsFocused`. |
| [[VisualElement (Controls).IsFocusedPropertyKey\|IsFocusedPropertyKey]] | Bindable property for `IsFocused`. |
| [[VisualElement (Controls).IsVisibleProperty\|IsVisibleProperty]] | Gets or sets a `IVisual` implementation that overrides the visual appearance of an element. This is a bindable property. |
| [[VisualElement (Controls).MaximumHeightRequestProperty\|MaximumHeightRequestProperty]] | Bindable property for `MaximumHeightRequest`. |
| [[VisualElement (Controls).MaximumWidthRequestProperty\|MaximumWidthRequestProperty]] | Bindable property for `MaximumWidthRequest`. |
| [[VisualElement (Controls).MinimumHeightRequestProperty\|MinimumHeightRequestProperty]] | Bindable property for `MinimumHeightRequest`. |
| [[VisualElement (Controls).MinimumWidthRequestProperty\|MinimumWidthRequestProperty]] | Bindable property for `MinimumWidthRequest`. |
| [[VisualElement (Controls).NavigationProperty\|NavigationProperty]] | Bindable property for `Navigation`. |
| [[VisualElement (Controls).OpacityProperty\|OpacityProperty]] | Bindable property for `Opacity`. |
| [[VisualElement (Controls).RotationProperty\|RotationProperty]] | Bindable property for `Rotation`. |
| [[VisualElement (Controls).RotationXProperty\|RotationXProperty]] | Bindable property for `RotationX`. |
| [[VisualElement (Controls).RotationYProperty\|RotationYProperty]] | Bindable property for `RotationY`. |
| [[VisualElement (Controls).ScaleProperty\|ScaleProperty]] | Bindable property for `Scale`. |
| [[VisualElement (Controls).ScaleXProperty\|ScaleXProperty]] | Bindable property for `ScaleX`. |
| [[VisualElement (Controls).ScaleYProperty\|ScaleYProperty]] | Bindable property for `ScaleY`. |
| [[VisualElement (Controls).ShadowProperty\|ShadowProperty]] | Bindable property for `Shadow`. |
| [[VisualElement (Controls).StyleProperty\|StyleProperty]] |  |
| [[VisualElement (Controls).TranslationXProperty\|TranslationXProperty]] | Bindable property for `TranslationX`. |
| [[VisualElement (Controls).TranslationYProperty\|TranslationYProperty]] | Bindable property for `TranslationY`. |
| [[VisualElement (Controls).TriggersProperty\|TriggersProperty]] | Bindable property for `Triggers`. |
| [[VisualElement (Controls).VisualProperty\|VisualProperty]] | Bindable property for `Visual`. |
| [[VisualElement (Controls).WidthProperty\|WidthProperty]] | Bindable property for `Width`. |
| [[VisualElement (Controls).WidthRequestProperty\|WidthRequestProperty]] | Bindable property for `WidthRequest`. |
| [[VisualElement (Controls).WindowProperty\|WindowProperty]] | Gets or sets the layout flow direction. This is a bindable property. |
| [[VisualElement (Controls).XProperty\|XProperty]] | Bindable property for `X`. |
| [[VisualElement (Controls).YProperty\|YProperty]] | Bindable property for `Y`. |
| [[VisualElement (Controls).ZIndexProperty\|ZIndexProperty]] | Gets or sets the shadow effect cast by the element. This is a bindable property. |

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.visualelement)
