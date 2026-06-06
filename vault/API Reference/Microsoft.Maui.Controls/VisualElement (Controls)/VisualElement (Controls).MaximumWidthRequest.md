---
title: "VisualElement (Controls).MaximumWidthRequest"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.VisualElement.MaximumWidthRequest"
declaring_type: "VisualElement (Controls)"
member_kind: property
---

# VisualElement (Controls).MaximumWidthRequest

> [!abstract] Property of [[VisualElement (Controls)|VisualElement (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the maximum width the element will request during layout in device-independent units. This is a bindable property.

## Signature

```csharp
double MaximumWidthRequest { get; set; }
```

## Remarks

The default value is `PositiveInfinity`. `MaximumWidthRequest` is used to ensure the element has no more than the specified width during layout. Device-independent units (DIUs) provide a consistent unit of measurement across different screen densities. One device-independent unit equals one pixel on a 96-DPI display.

## See also

- Declaring type: [[VisualElement (Controls)|VisualElement (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
