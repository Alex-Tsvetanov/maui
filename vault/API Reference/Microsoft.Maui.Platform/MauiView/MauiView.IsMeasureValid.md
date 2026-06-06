---
title: "MauiView.IsMeasureValid"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.MauiView.IsMeasureValid"
declaring_type: "MauiView"
member_kind: method
---

# MauiView.IsMeasureValid

> [!abstract] Method of [[MauiView|MauiView]]
> Namespace: `Microsoft.Maui.Platform`

Checks if the current measure information is still valid for the given constraints. This optimization avoids redundant measure operations when constraints haven't changed.

## Signature

```csharp
bool IsMeasureValid(double widthConstraint, double heightConstraint)
```

## Parameters

| Parameter | Description |
|---|---|
| `widthConstraint` | The width constraint to check |
| `heightConstraint` | The height constraint to check |

## Returns

True if the cached measure is still valid, false otherwise

## See also

- Declaring type: [[MauiView|MauiView]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
