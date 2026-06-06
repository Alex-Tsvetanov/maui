---
title: "WDSkiaGraphicsView.Renderer"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Graphics-Skia
aliases:
  - "Microsoft.Maui.Graphics.Skia.WDSkiaGraphicsView.Renderer"
declaring_type: "WDSkiaGraphicsView"
member_kind: property
---

# WDSkiaGraphicsView.Renderer

> [!abstract] Property of [[WDSkiaGraphicsView|WDSkiaGraphicsView]]
> Namespace: `Microsoft.Maui.Graphics.Skia`

Gets or sets the renderer used to handle drawing operations.

## Signature

```csharp
Microsoft.Maui.Graphics.Skia.ISkiaGraphicsRenderer Renderer { get; set; }
```

## Remarks

When a new renderer is set, the previous renderer is disposed, and the new renderer is configured with the current drawable and view dimensions.

## See also

- Declaring type: [[WDSkiaGraphicsView|WDSkiaGraphicsView]]
- [[_Microsoft.Maui.Graphics.Skia|Microsoft.Maui.Graphics.Skia namespace]]
