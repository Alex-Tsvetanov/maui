---
title: "GtkSkiaDirectRenderer.Invalidate"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Skia
aliases:
  - "Microsoft.Maui.Graphics.Skia.GtkSkiaDirectRenderer.Invalidate"
declaring_type: "GtkSkiaDirectRenderer"
member_kind: method
---

# GtkSkiaDirectRenderer.Invalidate

> [!abstract] Method of [[GtkSkiaDirectRenderer|GtkSkiaDirectRenderer]]
> Namespace: `Microsoft.Maui.Graphics.Skia`

Invalidates the entire drawing surface, causing a redraw.

## Signatures

```csharp
void Invalidate()
void Invalidate(float x, float y, float w, float h)
```

## Remarks

Calls `QueueDraw` on the associated `GtkSkiaGraphicsView` to request a redraw.

## See also

- Declaring type: [[GtkSkiaDirectRenderer|GtkSkiaDirectRenderer]]
- [[_Microsoft.Maui.Graphics.Skia|Microsoft.Maui.Graphics.Skia namespace]]
