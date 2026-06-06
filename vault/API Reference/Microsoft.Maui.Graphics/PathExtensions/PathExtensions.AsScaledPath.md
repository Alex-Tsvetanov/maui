---
title: "PathExtensions.AsScaledPath"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.PathExtensions.AsScaledPath"
declaring_type: "PathExtensions"
member_kind: method
---

# PathExtensions.AsScaledPath

> [!abstract] Method of [[PathExtensions|PathExtensions]]
> Namespace: `Microsoft.Maui.Graphics`

Creates a new path by scaling the target path uniformly.

## Signatures

```csharp
Microsoft.Maui.Graphics.PathF static AsScaledPath(this Microsoft.Maui.Graphics.PathF target, float scale)
Microsoft.Maui.Graphics.PathF static AsScaledPath(this Microsoft.Maui.Graphics.PathF target, float xScale, float yScale)
```

## Parameters

| Parameter | Description |
|---|---|
| `target` | The path to scale. |
| `scale` | The uniform scale factor to apply to both x and y dimensions. |

## Returns

A new scaled path.

## See also

- Declaring type: [[PathExtensions|PathExtensions]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
