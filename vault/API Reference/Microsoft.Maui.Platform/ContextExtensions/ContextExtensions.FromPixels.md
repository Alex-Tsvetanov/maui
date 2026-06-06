---
title: "ContextExtensions.FromPixels"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.ContextExtensions.FromPixels"
declaring_type: "ContextExtensions"
member_kind: method
---

# ContextExtensions.FromPixels

> [!abstract] Method of [[ContextExtensions|ContextExtensions]]
> Namespace: `Microsoft.Maui.Platform`

Converts the specified pixel values to device-independent units using the context's display density.

## Signatures

```csharp
Microsoft.Maui.Graphics.Size static FromPixels(this Android.Content.Context! context, double width, double height)
Microsoft.Maui.Graphics.Rect static FromPixels(this Android.Content.Context! context, Microsoft.Maui.Graphics.Rect rect)
Microsoft.Maui.Thickness static FromPixels(this Android.Content.Context! context, Microsoft.Maui.Thickness thickness)
double static FromPixels(this Android.Content.Context? self, double pixels)
```

## See also

- Declaring type: [[ContextExtensions|ContextExtensions]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
