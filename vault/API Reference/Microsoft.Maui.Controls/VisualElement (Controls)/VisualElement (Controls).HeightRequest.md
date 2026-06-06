---
title: "VisualElement (Controls).HeightRequest"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.VisualElement.HeightRequest"
declaring_type: "VisualElement (Controls)"
member_kind: property
---

# VisualElement (Controls).HeightRequest

> [!abstract] Property of [[VisualElement (Controls)|VisualElement (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the desired height override of this element in device-independent units. This is a bindable property.

## Signature

```csharp
double HeightRequest { get; set; }
```

## Remarks

The default value is -1, which means the value is unset; the effective minimum height will be zero. `HeightRequest` does not immediately change the `Bounds` of an element; setting the `HeightRequest` will change the resulting height of the element during the next layout pass. Device-independent units (DIUs) provide a consistent unit of measurement across different screen densities. One device-independent unit equals one pixel on a 96-DPI display.

## See also

- Declaring type: [[VisualElement (Controls)|VisualElement (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
