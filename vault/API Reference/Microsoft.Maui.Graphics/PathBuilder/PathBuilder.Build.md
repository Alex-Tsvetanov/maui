---
title: "PathBuilder.Build"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.PathBuilder.Build"
declaring_type: "PathBuilder"
member_kind: method
---

# PathBuilder.Build

> [!abstract] Method of [[PathBuilder|PathBuilder]]
> Namespace: `Microsoft.Maui.Graphics`

Builds a path from a string definition.

## Signature

```csharp
Microsoft.Maui.Graphics.PathF static Build(string definition)
```

## Parameters

| Parameter | Description |
|---|---|
| `definition` | The string definition of the path using SVG-like path commands. |

## Returns

A new `PathF` object representing the defined path.

## Remarks

Returns an empty path if the definition is null or empty.

## See also

- Declaring type: [[PathBuilder|PathBuilder]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
