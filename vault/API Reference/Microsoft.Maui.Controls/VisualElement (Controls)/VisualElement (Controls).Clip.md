---
title: "VisualElement (Controls).Clip"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.VisualElement.Clip"
declaring_type: "VisualElement (Controls)"
member_kind: property
---

# VisualElement (Controls).Clip

> [!abstract] Property of [[VisualElement (Controls)|VisualElement (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Specifies the clipping region for an element. This is a bindable property.

## Signature

```csharp
Microsoft.Maui.Controls.Shapes.Geometry Clip { get; set; }
```

## Remarks

When an element is rendered, only the portion of the element that falls inside the clipping `Geometry` is displayed, while any content that extends outside the clipping region is clipped (that is, not displayed).

## See also

- Declaring type: [[VisualElement (Controls)|VisualElement (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
