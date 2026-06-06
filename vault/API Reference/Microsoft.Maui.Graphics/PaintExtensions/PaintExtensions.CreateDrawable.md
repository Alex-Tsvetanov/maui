---
title: "PaintExtensions.CreateDrawable"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.PaintExtensions.CreateDrawable"
declaring_type: "PaintExtensions"
member_kind: method
---

# PaintExtensions.CreateDrawable

> [!abstract] Method of [[PaintExtensions|PaintExtensions]]
> Namespace: `Microsoft.Maui.Graphics`

Creates a native drawable from the specified cross-platform paint.

## Signatures

```csharp
Android.Graphics.Drawables.Drawable? static CreateDrawable(this Microsoft.Maui.Graphics.ImagePaint! imagePaint, Android.Content.Context? context)
Android.Graphics.Drawables.Drawable? static CreateDrawable(this Microsoft.Maui.Graphics.LinearGradientPaint! linearGradientPaint, Android.Content.Context? context)
Android.Graphics.Drawables.Drawable? static CreateDrawable(this Microsoft.Maui.Graphics.PatternPaint! patternPaint, Android.Content.Context? context)
Android.Graphics.Drawables.Drawable? static CreateDrawable(this Microsoft.Maui.Graphics.RadialGradientPaint! radialGradientPaint, Android.Content.Context? context)
Android.Graphics.Drawables.Drawable? static CreateDrawable(this Microsoft.Maui.Graphics.SolidPaint! solidPaint, Android.Content.Context? context)
Microsoft.Maui.MauiDrawable? static CreateDrawable(this Microsoft.Maui.Graphics.ImagePaint! imagePaint)
Microsoft.Maui.MauiDrawable? static CreateDrawable(this Microsoft.Maui.Graphics.LinearGradientPaint! linearGradientPaint)
Microsoft.Maui.MauiDrawable? static CreateDrawable(this Microsoft.Maui.Graphics.PatternPaint! patternPaint)
Microsoft.Maui.MauiDrawable? static CreateDrawable(this Microsoft.Maui.Graphics.RadialGradientPaint! radialGradientPaint)
Microsoft.Maui.MauiDrawable? static CreateDrawable(this Microsoft.Maui.Graphics.SolidPaint! solidPaint)
```

## See also

- Declaring type: [[PaintExtensions|PaintExtensions]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
