---
title: "VisualElement (Controls).MaximumHeightRequest"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.VisualElement.MaximumHeightRequest"
declaring_type: "VisualElement (Controls)"
member_kind: property
---

# VisualElement (Controls).MaximumHeightRequest

> [!abstract] Property of [[VisualElement (Controls)|VisualElement (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the maximum height the element will request during layout in device-independent units. This is a bindable property.

## Signature

```csharp
double MaximumHeightRequest { get; set; }
```

## Remarks

The default value is `PositiveInfinity`. `MaximumHeightRequest` is used to ensure that the element has no more than the specified height during layout. Device-independent units (DIUs) provide a consistent unit of measurement across different screen densities. One device-independent unit equals one pixel on a 96-DPI display.

## See also

- Declaring type: [[VisualElement (Controls)|VisualElement (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
