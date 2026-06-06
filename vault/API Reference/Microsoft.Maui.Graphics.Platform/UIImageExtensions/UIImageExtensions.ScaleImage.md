---
title: "UIImageExtensions.ScaleImage"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Platform
aliases:
  - "Microsoft.Maui.Graphics.Platform.UIImageExtensions.ScaleImage"
declaring_type: "UIImageExtensions"
member_kind: method
---

# UIImageExtensions.ScaleImage

> [!abstract] Method of [[UIImageExtensions|UIImageExtensions]]
> Namespace: `Microsoft.Maui.Graphics.Platform`

Returns a copy of the specified image scaled to the given size or constraints, optionally disposing the original.

## Signatures

```csharp
UIKit.UIImage static ScaleImage(this UIKit.UIImage target, CoreGraphics.CGSize size, bool disposeOriginal = false)
UIKit.UIImage static ScaleImage(this UIKit.UIImage target, float maxWidth, float maxHeight, bool disposeOriginal = false)
```

## See also

- Declaring type: [[UIImageExtensions|UIImageExtensions]]
- [[_Microsoft.Maui.Graphics.Platform|Microsoft.Maui.Graphics.Platform namespace]]
