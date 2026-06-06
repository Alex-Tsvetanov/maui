---
title: "VisualTreeElementExtensions.GetVisualTreeElements"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.VisualTreeElementExtensions.GetVisualTreeElements"
declaring_type: "VisualTreeElementExtensions"
member_kind: method
---

# VisualTreeElementExtensions.GetVisualTreeElements

> [!abstract] Method of [[VisualTreeElementExtensions|VisualTreeElementExtensions]]
> Namespace: `Microsoft.Maui`

Gets list of a Visual Tree Elements children based off of a rectangle defined by its coordinates which are specified in platform units, not pixels.

## Signatures

```csharp
System.Collections.Generic.IReadOnlyList<Microsoft.Maui.IVisualTreeElement!>! static GetVisualTreeElements(this Microsoft.Maui.IVisualTreeElement! visualElement, double x, double y)
System.Collections.Generic.IReadOnlyList<Microsoft.Maui.IVisualTreeElement!>! static GetVisualTreeElements(this Microsoft.Maui.IVisualTreeElement! visualElement, double x1, double y1, double x2, double y2)
System.Collections.Generic.IReadOnlyList<Microsoft.Maui.IVisualTreeElement!>! static GetVisualTreeElements(this Microsoft.Maui.IVisualTreeElement! visualElement, Microsoft.Maui.Graphics.Point point)
System.Collections.Generic.IReadOnlyList<Microsoft.Maui.IVisualTreeElement!>! static GetVisualTreeElements(this Microsoft.Maui.IVisualTreeElement! visualElement, Microsoft.Maui.Graphics.Rect rectangle)
```

## Returns

List of Children Elements.

## Parameters

| Parameter | Description |
|---|---|
| `visualElement` | `IVisualTreeElement` to scan. |
| `x1` | The X coordinate of the top left point. |
| `y1` | The Y coordinate of the top left point. |
| `x2` | The X coordinate of the bottom right point. |
| `y2` | The Y coordinate of the bottom right point. |

## See also

- Declaring type: [[VisualTreeElementExtensions|VisualTreeElementExtensions]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
