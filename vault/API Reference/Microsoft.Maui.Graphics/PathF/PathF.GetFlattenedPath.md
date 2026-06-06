---
title: "PathF.GetFlattenedPath"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.PathF.GetFlattenedPath"
declaring_type: "PathF"
member_kind: method
---

# PathF.GetFlattenedPath

> [!abstract] Method of [[PathF|PathF]]
> Namespace: `Microsoft.Maui.Graphics`

Creates a new path consisting only of line segments approximating all curves and arcs.

## Signature

```csharp
Microsoft.Maui.Graphics.PathF GetFlattenedPath(float flatness = 0.001, bool includeSubPaths = false)
```

## Parameters

| Parameter | Description |
|---|---|
| `flatness` | Maximum allowed deviation per segment (smaller = more segments). |
| `includeSubPaths` | If true , flattens all sub-paths; otherwise stops after the first closed one. |

## Returns

A flattened path.

## See also

- Declaring type: [[PathF|PathF]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
