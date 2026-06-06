---
title: "ICanvas.ClipPath"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.ICanvas.ClipPath"
declaring_type: "ICanvas"
member_kind: method
---

# ICanvas.ClipPath

> [!abstract] Method of [[ICanvas|ICanvas]]
> Namespace: `Microsoft.Maui.Graphics`

Clips an object so that only the area that's within the region of a `PathF` object will be visible.

## Signature

```csharp
void ClipPath(Microsoft.Maui.Graphics.PathF path, Microsoft.Maui.Graphics.WindingMode windingMode = Microsoft.Maui.Graphics.WindingMode.NonZero)
```

## Parameters

| Parameter | Description |
|---|---|
| `path` | The path used to clip the object |
| `windingMode` | Fill algorithm used for the path. Default is `NonZero`. |

## See also

- Declaring type: [[ICanvas|ICanvas]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
