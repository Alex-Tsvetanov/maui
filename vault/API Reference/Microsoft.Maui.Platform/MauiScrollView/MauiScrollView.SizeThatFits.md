---
title: "MauiScrollView.SizeThatFits"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.MauiScrollView.SizeThatFits"
declaring_type: "MauiScrollView"
member_kind: method
---

# MauiScrollView.SizeThatFits

> [!abstract] Method of [[MauiScrollView|MauiScrollView]]
> Namespace: `Microsoft.Maui.Platform`

Calculates the size that fits within the given constraints. This method is called by iOS when the system needs to determine the natural size of the scroll view.

## Signature

```csharp
CoreGraphics.CGSize override SizeThatFits(CoreGraphics.CGSize size)
```

## Parameters

| Parameter | Description |
|---|---|
| `size` | The available size constraints. |

## Returns

The size that fits within the constraints.

## See also

- Declaring type: [[MauiScrollView|MauiScrollView]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
