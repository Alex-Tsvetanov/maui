---
title: "VisualElement (Controls).MinimumHeightRequest"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.VisualElement.MinimumHeightRequest"
declaring_type: "VisualElement (Controls)"
member_kind: property
---

# VisualElement (Controls).MinimumHeightRequest

> [!abstract] Property of [[VisualElement (Controls)|VisualElement (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the minimum height the element will request during layout in device-independent units. This is a bindable property.

## Signature

```csharp
double MinimumHeightRequest { get; set; }
```

## Remarks

The default value is -1, which means the value is unset and a height will be determined automatically. `MinimumHeightRequest` is used to ensure that the element has at least the specified height during layout. Device-independent units (DIUs) provide a consistent unit of measurement across different screen densities. One device-independent unit equals one pixel on a 96-DPI display.

## See also

- Declaring type: [[VisualElement (Controls)|VisualElement (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
