---
title: "VisualElement (Controls).TranslationY"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.VisualElement.TranslationY"
declaring_type: "VisualElement (Controls)"
member_kind: property
---

# VisualElement (Controls).TranslationY

> [!abstract] Property of [[VisualElement (Controls)|VisualElement (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the Y translation delta of the element in device-independent units. This is a bindable property.

## Signature

```csharp
double TranslationY { get; set; }
```

## Remarks

Translation is applied post layout. It is particularly good for applying animations. Translating an element outside the bounds of its parent container may prevent inputs from working. Device-independent units (DIUs) provide a consistent unit of measurement across different screen densities. One device-independent unit equals one pixel on a 96-DPI display.

## See also

- Declaring type: [[VisualElement (Controls)|VisualElement (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
