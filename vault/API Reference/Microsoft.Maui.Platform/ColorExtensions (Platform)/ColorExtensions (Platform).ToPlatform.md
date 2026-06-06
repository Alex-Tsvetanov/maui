---
title: "ColorExtensions (Platform).ToPlatform"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.ColorExtensions.ToPlatform"
declaring_type: "ColorExtensions (Platform)"
member_kind: method
---

# ColorExtensions (Platform).ToPlatform

> [!abstract] Method of [[ColorExtensions (Platform)|ColorExtensions (Platform)]]
> Namespace: `Microsoft.Maui.Platform`

Converts the specified cross-platform color to its native platform color representation.

## Signatures

```csharp
Android.Graphics.Color static ToPlatform(this Microsoft.Maui.Graphics.Color! self, int defaultColorResourceId, Android.Content.Context! context)
Android.Graphics.Color static ToPlatform(this Microsoft.Maui.Graphics.Color! self)
Android.Graphics.Color static ToPlatform(this Microsoft.Maui.Graphics.Color? self, Microsoft.Maui.Graphics.Color! defaultColor)
UIKit.UIColor! static ToPlatform(this Microsoft.Maui.Graphics.Color! color)
UIKit.UIColor? static ToPlatform(this Microsoft.Maui.Graphics.Color? color, Microsoft.Maui.Graphics.Color? defaultColor)
UIKit.UIColor! static ToPlatform(this Microsoft.Maui.Graphics.Color? color, UIKit.UIColor! defaultColor)
Tizen.UIExtensions.Common.Color static ToPlatform(this Microsoft.Maui.Graphics.Color! c)
```

## See also

- Declaring type: [[ColorExtensions (Platform)|ColorExtensions (Platform)]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
