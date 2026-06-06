---
title: "MauiScrollView.AdjustedContentInsetDidChange"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.MauiScrollView.AdjustedContentInsetDidChange"
declaring_type: "MauiScrollView"
member_kind: method
---

# MauiScrollView.AdjustedContentInsetDidChange

> [!abstract] Method of [[MauiScrollView|MauiScrollView]]
> Namespace: `Microsoft.Maui.Platform`

Determines whether this scroll view should respond to safe area changes. Returns false if this scroll view is nested within another scroll view, as nested scroll views should not apply their own safe area adjustments.

## Signature

```csharp
void override AdjustedContentInsetDidChange()
```

## Returns

True if this scroll view should apply safe area adjustments, false otherwise.

## See also

- Declaring type: [[MauiScrollView|MauiScrollView]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
