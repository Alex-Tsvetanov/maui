---
title: "BrushExtensions.UpdateBackground"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-Platform
aliases:
  - "Microsoft.Maui.Controls.Platform.BrushExtensions.UpdateBackground"
declaring_type: "BrushExtensions"
member_kind: method
---

# BrushExtensions.UpdateBackground

> [!abstract] Method of [[BrushExtensions|BrushExtensions]]
> Namespace: `Microsoft.Maui.Controls.Platform`

Updates the background of the native drawable or view to match the specified cross-platform brush.

## Signatures

```csharp
void static UpdateBackground(this Android.Graphics.Drawables.GradientDrawable gradientDrawable, Microsoft.Maui.Controls.Brush brush, int height, int width)
void static UpdateBackground(this Android.Graphics.Paint paint, Microsoft.Maui.Controls.Brush brush, int height, int width)
void static UpdateBackground(this Android.Views.View view, Microsoft.Maui.Controls.Brush brush)
void static UpdateBackground(this UIKit.UIView control, Microsoft.Maui.Controls.Brush brush)
```

## See also

- Declaring type: [[BrushExtensions|BrushExtensions]]
- [[_Microsoft.Maui.Controls.Platform|Microsoft.Maui.Controls.Platform namespace]]
