---
title: "VisualElement (Controls).WidthRequest"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.VisualElement.WidthRequest"
declaring_type: "VisualElement (Controls)"
member_kind: property
---

# VisualElement (Controls).WidthRequest

> [!abstract] Property of [[VisualElement (Controls)|VisualElement (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the desired width override of this element in device-independent units. This is a bindable property.

## Signature

```csharp
double WidthRequest { get; set; }
```

## Remarks

The default value is -1, which means the value is unset and a width will be determined automatically. `WidthRequest` does not immediately change the `Bounds` of an element; setting the `WidthRequest` will change the resulting width of the element during the next layout pass. Device-independent units (DIUs) provide a consistent unit of measurement across different screen densities. One device-independent unit equals one pixel on a 96-DPI display.

## See also

- Declaring type: [[VisualElement (Controls)|VisualElement (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
