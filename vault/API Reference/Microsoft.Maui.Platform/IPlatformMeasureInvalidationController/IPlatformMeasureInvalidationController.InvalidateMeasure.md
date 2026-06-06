---
title: "IPlatformMeasureInvalidationController.InvalidateMeasure"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.IPlatformMeasureInvalidationController.InvalidateMeasure"
declaring_type: "IPlatformMeasureInvalidationController"
member_kind: method
---

# IPlatformMeasureInvalidationController.InvalidateMeasure

> [!abstract] Method of [[IPlatformMeasureInvalidationController|IPlatformMeasureInvalidationController]]
> Namespace: `Microsoft.Maui.Platform`

Invalidates the current view via SetNeedsLayout and returns whether to continue propagating the invalidation to ancestors or not.

## Signature

```csharp
bool InvalidateMeasure(bool isPropagating = false)
```

## Returns

True to continue propagating invalidation to ancestor views, false to stop propagation.

## Parameters

| Parameter | Description |
|---|---|
| `isPropagating` | True if this invalidation is being propagated from a descendant view, false if this is the initial view that triggered the invalidation. |

## See also

- Declaring type: [[IPlatformMeasureInvalidationController|IPlatformMeasureInvalidationController]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
