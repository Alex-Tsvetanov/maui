---
title: "VisualElement (Controls).MinimumWidthRequest"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.VisualElement.MinimumWidthRequest"
declaring_type: "VisualElement (Controls)"
member_kind: property
---

# VisualElement (Controls).MinimumWidthRequest

> [!abstract] Property of [[VisualElement (Controls)|VisualElement (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the minimum width the element will request during layout in device-independent units. This is a bindable property.

## Signature

```csharp
double MinimumWidthRequest { get; set; }
```

## Remarks

The default value is -1, which means the value is unset; the effective minimum width will be zero. `MinimumWidthRequest` is used to ensure that the element has at least the specified width during layout. Device-independent units (DIUs) provide a consistent unit of measurement across different screen densities. One device-independent unit equals one pixel on a 96-DPI display.

## See also

- Declaring type: [[VisualElement (Controls)|VisualElement (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
