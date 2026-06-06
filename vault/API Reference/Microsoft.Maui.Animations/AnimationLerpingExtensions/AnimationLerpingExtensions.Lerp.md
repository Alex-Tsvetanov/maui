---
title: "AnimationLerpingExtensions.Lerp"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Animations
aliases:
  - "Microsoft.Maui.Animations.AnimationLerpingExtensions.Lerp"
declaring_type: "AnimationLerpingExtensions"
member_kind: method
---

# AnimationLerpingExtensions.Lerp

> [!abstract] Method of [[AnimationLerpingExtensions|AnimationLerpingExtensions]]
> Namespace: `Microsoft.Maui.Animations`

Linearly interpolates between a start and end value based on the given progress, with overloads for numeric, nullable, and color types.

## Signatures

```csharp
double static Lerp(this double start, double end, double progress)
float static Lerp(this float start, float end, double progress)
float? static Lerp(this float? start, float? end, double progress)
Microsoft.Maui.Graphics.Color! static Lerp(this Microsoft.Maui.Graphics.Color! color, Microsoft.Maui.Graphics.Color! endColor, double progress)
Microsoft.Maui.Graphics.Point static Lerp(this Microsoft.Maui.Graphics.Point start, Microsoft.Maui.Graphics.Point end, double progress)
Microsoft.Maui.Graphics.PointF static Lerp(this Microsoft.Maui.Graphics.PointF start, Microsoft.Maui.Graphics.PointF end, double progress)
Microsoft.Maui.Graphics.Rect static Lerp(this Microsoft.Maui.Graphics.Rect start, Microsoft.Maui.Graphics.Rect end, double progress)
Microsoft.Maui.Graphics.RectF static Lerp(this Microsoft.Maui.Graphics.RectF start, Microsoft.Maui.Graphics.RectF end, double progress)
Microsoft.Maui.Graphics.Size static Lerp(this Microsoft.Maui.Graphics.Size start, Microsoft.Maui.Graphics.Size end, double progress)
Microsoft.Maui.Graphics.SizeF static Lerp(this Microsoft.Maui.Graphics.SizeF start, Microsoft.Maui.Graphics.SizeF end, double progress)
Microsoft.Maui.Graphics.SolidPaint! static Lerp(this Microsoft.Maui.Graphics.SolidPaint! paint, Microsoft.Maui.Graphics.SolidPaint! endPaint, double progress)
Microsoft.Maui.Thickness static Lerp(this Microsoft.Maui.Thickness start, Microsoft.Maui.Thickness end, double progress)
```

## See also

- Declaring type: [[AnimationLerpingExtensions|AnimationLerpingExtensions]]
- [[_Microsoft.Maui.Animations|Microsoft.Maui.Animations namespace]]
