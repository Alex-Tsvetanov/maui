---
title: "ContextExtensions.ToPixels"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.ContextExtensions.ToPixels"
declaring_type: "ContextExtensions"
member_kind: method
---

# ContextExtensions.ToPixels

> [!abstract] Method of [[ContextExtensions|ContextExtensions]]
> Namespace: `Microsoft.Maui.Platform`

Converts the specified device-independent values to pixels using the context's display density.

## Signatures

```csharp
(int left, int top, int right, int bottom) static ToPixels(this Android.Content.Context! context, Microsoft.Maui.Graphics.Rect rectangle)
Microsoft.Maui.Thickness static ToPixels(this Android.Content.Context! context, Microsoft.Maui.Thickness thickness)
float static ToPixels(this Android.Content.Context? self, double dp)
```

## See also

- Declaring type: [[ContextExtensions|ContextExtensions]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
