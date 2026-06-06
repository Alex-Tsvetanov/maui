---
title: "PlatformCanvas.DrawAttributedText"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Platform
aliases:
  - "Microsoft.Maui.Graphics.Platform.PlatformCanvas.DrawAttributedText"
declaring_type: "PlatformCanvas"
member_kind: method
---

# PlatformCanvas.DrawAttributedText

> [!abstract] Method of [[PlatformCanvas|PlatformCanvas]]
> Namespace: `Microsoft.Maui.Graphics.Platform`

Draws the specified attributed text into the given native context along the specified path.

## Signatures

```csharp
void static DrawAttributedText(CoreGraphics.CGContext context, Microsoft.Maui.Graphics.Text.IAttributedText text, CoreGraphics.CGPath path, Microsoft.Maui.Graphics.IFont font, float fontSize, Microsoft.Maui.Graphics.Color fontColor, Microsoft.Maui.Graphics.TextFlow textFlow = Microsoft.Maui.Graphics.TextFlow.ClipBounds, float ix = 0, float iy = 0)
void static DrawAttributedText(CoreGraphics.CGContext context, Microsoft.Maui.Graphics.Text.IAttributedText text, CoreGraphics.CGRect rect, Microsoft.Maui.Graphics.IFont font, float fontSize, Microsoft.Maui.Graphics.Color fontColor, Microsoft.Maui.Graphics.TextFlow textFlow = Microsoft.Maui.Graphics.TextFlow.ClipBounds, float ix = 0, float iy = 0)
```

## See also

- Declaring type: [[PlatformCanvas|PlatformCanvas]]
- [[_Microsoft.Maui.Graphics.Platform|Microsoft.Maui.Graphics.Platform namespace]]
