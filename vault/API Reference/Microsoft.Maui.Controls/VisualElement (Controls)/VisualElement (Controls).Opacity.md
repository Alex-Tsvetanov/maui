---
title: "VisualElement (Controls).Opacity"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.VisualElement.Opacity"
declaring_type: "VisualElement (Controls)"
member_kind: property
---

# VisualElement (Controls).Opacity

> [!abstract] Property of [[VisualElement (Controls)|VisualElement (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the opacity value applied to the element when it is rendered. The range of this value is 0 to 1; values outside this range will be set to the nearest valid value. This is a bindable property.

## Signature

```csharp
double Opacity { get; set; }
```

## Remarks

The default value is 1.0. The opacity value has no effect unless `IsVisible` is `true`. The effective opacity of an element is the value of `Opacity` multiplied by the opacity of the element's Parent . If a parent has 0.5 opacity, and a child has 0.5 opacity, the child will render with an effective 0.25 opacity.

## See also

- Declaring type: [[VisualElement (Controls)|VisualElement (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
