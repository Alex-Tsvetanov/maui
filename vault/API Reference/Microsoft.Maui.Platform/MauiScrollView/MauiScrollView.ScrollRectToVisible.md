---
title: "MauiScrollView.ScrollRectToVisible"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.MauiScrollView.ScrollRectToVisible"
declaring_type: "MauiScrollView"
member_kind: method
---

# MauiScrollView.ScrollRectToVisible

> [!abstract] Method of [[MauiScrollView|MauiScrollView]]
> Namespace: `Microsoft.Maui.Platform`

Invalidates the cached constraint values, forcing a re-measurement and re-arrangement on the next layout pass.

## Signature

```csharp
void override ScrollRectToVisible(CoreGraphics.CGRect rect, bool animated)
```

## Parameters

| Parameter | Description |
|---|---|
| `widthConstraint` | The width constraint to cache. |
| `heightConstraint` | The height constraint to cache. |
| `rect` | The rectangle to scroll to. |
| `animated` | Whether the scrolling should be animated. |

## See also

- Declaring type: [[MauiScrollView|MauiScrollView]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
