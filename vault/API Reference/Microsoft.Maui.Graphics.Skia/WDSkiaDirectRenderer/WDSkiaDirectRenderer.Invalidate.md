---
title: "WDSkiaDirectRenderer.Invalidate"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Skia
aliases:
  - "Microsoft.Maui.Graphics.Skia.WDSkiaDirectRenderer.Invalidate"
declaring_type: "WDSkiaDirectRenderer"
member_kind: method
---

# WDSkiaDirectRenderer.Invalidate

> [!abstract] Method of [[WDSkiaDirectRenderer|WDSkiaDirectRenderer]]
> Namespace: `Microsoft.Maui.Graphics.Skia`

Invalidates the entire drawing surface, causing a redraw.

## Signatures

```csharp
void Invalidate()
void Invalidate(float x, float y, float w, float h)
```

## Remarks

This method delegates to the associated graphics view's Invalidate method.

## See also

- Declaring type: [[WDSkiaDirectRenderer|WDSkiaDirectRenderer]]
- [[_Microsoft.Maui.Graphics.Skia|Microsoft.Maui.Graphics.Skia namespace]]
