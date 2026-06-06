---
title: "MauiView.SizeThatFits"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.MauiView.SizeThatFits"
declaring_type: "MauiView"
member_kind: method
---

# MauiView.SizeThatFits

> [!abstract] Method of [[MauiView|MauiView]]
> Namespace: `Microsoft.Maui.Platform`

Performs cross-platform arrange operation, optionally adjusting for safe area. This method positions and sizes child elements within the available bounds.

## Signature

```csharp
CoreGraphics.CGSize override SizeThatFits(CoreGraphics.CGSize size)
```

## Parameters

| Parameter | Description |
|---|---|
| `bounds` | The bounds rectangle to arrange within |
| `size` | The size constraints |

## Returns

The size that fits within the constraints

## See also

- Declaring type: [[MauiView|MauiView]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
