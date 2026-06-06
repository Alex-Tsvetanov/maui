---
title: "PathExtensions.ToDefinitionString"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.PathExtensions.ToDefinitionString"
declaring_type: "PathExtensions"
member_kind: method
---

# PathExtensions.ToDefinitionString

> [!abstract] Method of [[PathExtensions|PathExtensions]]
> Namespace: `Microsoft.Maui.Graphics`

Converts a path to an SVG-style path definition string.

## Signature

```csharp
string static ToDefinitionString(this Microsoft.Maui.Graphics.PathF path, float ppu = 1)
```

## Parameters

| Parameter | Description |
|---|---|
| `path` | The path to convert. |
| `ppu` | Points per unit scaling factor (default is 1). |

## Returns

A string representation of the path using SVG path commands.

## See also

- Declaring type: [[PathExtensions|PathExtensions]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
