---
title: "ColorExtensions (Platform).ToColor"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.ColorExtensions.ToColor"
declaring_type: "ColorExtensions (Platform)"
member_kind: method
---

# ColorExtensions (Platform).ToColor

> [!abstract] Method of [[ColorExtensions (Platform)|ColorExtensions (Platform)]]
> Namespace: `Microsoft.Maui.Platform`

Converts the specified platform color value to a cross-platform Color.

## Signatures

```csharp
Microsoft.Maui.Graphics.Color! static ToColor(this Android.Graphics.Color color)
Microsoft.Maui.Graphics.Color! static ToColor(this uint color)
Microsoft.Maui.Graphics.Color? static ToColor(this UIKit.UIColor! color)
Microsoft.Maui.Graphics.Color! static ToColor(this Microsoft.UI.Xaml.Media.SolidColorBrush! solidColorBrush)
Microsoft.Maui.Graphics.Color! static ToColor(this Windows.UI.Color color)
```

## See also

- Declaring type: [[ColorExtensions (Platform)|ColorExtensions (Platform)]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
