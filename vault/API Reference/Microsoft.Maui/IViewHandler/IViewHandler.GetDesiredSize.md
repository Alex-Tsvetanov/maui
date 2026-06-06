---
title: "IViewHandler.GetDesiredSize"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.IViewHandler.GetDesiredSize"
declaring_type: "IViewHandler"
member_kind: method
---

# IViewHandler.GetDesiredSize

> [!abstract] Method of [[IViewHandler|IViewHandler]]
> Namespace: `Microsoft.Maui`

Computes the actual size of a view based on the desired size and constraints.

## Signature

```csharp
Microsoft.Maui.Graphics.Size GetDesiredSize(double widthConstraint, double heightConstraint)
```

## Returns

The computed size for the view associated to this handler.

## Parameters

| Parameter | Description |
|---|---|
| `widthConstraint` | The constraint on the width of the view. |
| `heightConstraint` | The constraint on the height of the view. |

## See also

- Declaring type: [[IViewHandler|IViewHandler]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
