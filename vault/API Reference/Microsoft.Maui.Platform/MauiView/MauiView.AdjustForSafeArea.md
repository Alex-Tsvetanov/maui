---
title: "MauiView.AdjustForSafeArea"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.MauiView.AdjustForSafeArea"
declaring_type: "MauiView"
member_kind: method
---

# MauiView.AdjustForSafeArea

> [!abstract] Method of [[MauiView|MauiView]]
> Namespace: `Microsoft.Maui.Platform`

Adjusts the given bounds rectangle to account for safe area insets. This method subtracts the safe area padding from the bounds to ensure content doesn't overlap with system UI elements.

## Signature

```csharp
CoreGraphics.CGRect AdjustForSafeArea(CoreGraphics.CGRect bounds)
```

## Parameters

| Parameter | Description |
|---|---|
| `bounds` | The original bounds rectangle |

## Returns

The bounds rectangle adjusted for safe area insets

## See also

- Declaring type: [[MauiView|MauiView]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
