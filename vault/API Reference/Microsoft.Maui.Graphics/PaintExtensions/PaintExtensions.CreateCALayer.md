---
title: "PaintExtensions.CreateCALayer"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.PaintExtensions.CreateCALayer"
declaring_type: "PaintExtensions"
member_kind: method
---

# PaintExtensions.CreateCALayer

> [!abstract] Method of [[PaintExtensions|PaintExtensions]]
> Namespace: `Microsoft.Maui.Graphics`

Creates a native Core Animation layer from the specified cross-platform gradient paint for the given frame.

## Signatures

```csharp
CoreAnimation.CALayer? static CreateCALayer(this Microsoft.Maui.Graphics.GradientPaint! gradientPaint, CoreGraphics.CGRect frame = default(CoreGraphics.CGRect))
CoreAnimation.CALayer? static CreateCALayer(this Microsoft.Maui.Graphics.ImagePaint! imagePaint, CoreGraphics.CGRect frame = default(CoreGraphics.CGRect))
CoreAnimation.CALayer? static CreateCALayer(this Microsoft.Maui.Graphics.LinearGradientPaint! linearGradientPaint, CoreGraphics.CGRect frame = default(CoreGraphics.CGRect))
CoreAnimation.CALayer? static CreateCALayer(this Microsoft.Maui.Graphics.PatternPaint! patternPaint, CoreGraphics.CGRect frame = default(CoreGraphics.CGRect))
CoreAnimation.CALayer? static CreateCALayer(this Microsoft.Maui.Graphics.RadialGradientPaint! radialGradientPaint, CoreGraphics.CGRect frame = default(CoreGraphics.CGRect))
CoreAnimation.CALayer? static CreateCALayer(this Microsoft.Maui.Graphics.SolidPaint! solidPaint, CoreGraphics.CGRect frame = default(CoreGraphics.CGRect))
```

## See also

- Declaring type: [[PaintExtensions|PaintExtensions]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
