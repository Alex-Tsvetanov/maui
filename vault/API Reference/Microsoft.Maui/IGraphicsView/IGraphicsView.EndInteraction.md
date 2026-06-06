---
title: "IGraphicsView.EndInteraction"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.IGraphicsView.EndInteraction"
declaring_type: "IGraphicsView"
member_kind: method
---

# IGraphicsView.EndInteraction

> [!abstract] Method of [[IGraphicsView|IGraphicsView]]
> Namespace: `Microsoft.Maui`

Raised when the press that raised the StartInteraction event is released.

## Signature

```csharp
void EndInteraction(Microsoft.Maui.Graphics.PointF[]! points, bool isInsideBounds)
```

## Parameters

| Parameter | Description |
|---|---|
| `points` | The set of positions where there has been interaction. |
| `isInsideBounds` | a boolean that indicates if the interaction takes place within the bounds. |

## See also

- Declaring type: [[IGraphicsView|IGraphicsView]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
