---
title: "NSImageExtensions.ScaleImage"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Platform
aliases:
  - "Microsoft.Maui.Graphics.Platform.NSImageExtensions.ScaleImage"
declaring_type: "NSImageExtensions"
member_kind: method
---

# NSImageExtensions.ScaleImage

> [!abstract] Method of [[NSImageExtensions|NSImageExtensions]]
> Namespace: `Microsoft.Maui.Graphics.Platform`

Returns a scaled copy of the specified NSImage at the given size or maximum dimensions.

## Signatures

```csharp
AppKit.NSImage static ScaleImage(this AppKit.NSImage target, CoreGraphics.CGSize size, bool disposeOriginal = false)
AppKit.NSImage static ScaleImage(this AppKit.NSImage target, float maxWidth, float maxHeight, bool disposeOriginal = false)
```

## See also

- Declaring type: [[NSImageExtensions|NSImageExtensions]]
- [[_Microsoft.Maui.Graphics.Platform|Microsoft.Maui.Graphics.Platform namespace]]
