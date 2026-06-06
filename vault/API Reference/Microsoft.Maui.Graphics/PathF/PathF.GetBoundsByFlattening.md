---
title: "PathF.GetBoundsByFlattening"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.PathF.GetBoundsByFlattening"
declaring_type: "PathF"
member_kind: method
---

# PathF.GetBoundsByFlattening

> [!abstract] Method of [[PathF|PathF]]
> Namespace: `Microsoft.Maui.Graphics`

Gets the axis-aligned bounding box of the path (cached until modified).

## Signature

```csharp
Microsoft.Maui.Graphics.RectF GetBoundsByFlattening(float flatness = 0.001)
```

## Parameters

| Parameter | Description |
|---|---|
| `flatness` | Maximum allowed deviation when flattening (smaller = more points). |

## Returns

The bounding rectangle.

## See also

- Declaring type: [[PathF|PathF]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
