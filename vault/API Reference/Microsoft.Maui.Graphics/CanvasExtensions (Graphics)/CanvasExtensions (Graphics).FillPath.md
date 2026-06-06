---
title: "CanvasExtensions (Graphics).FillPath"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.CanvasExtensions.FillPath"
declaring_type: "CanvasExtensions (Graphics)"
member_kind: method
---

# CanvasExtensions (Graphics).FillPath

> [!abstract] Method of [[CanvasExtensions (Graphics)|CanvasExtensions (Graphics)]]
> Namespace: `Microsoft.Maui.Graphics`

Fills the specified path using the current fill color and the non-zero winding rule.

## Signatures

```csharp
void static FillPath(this Microsoft.Maui.Graphics.ICanvas target, Microsoft.Maui.Graphics.PathF path, Microsoft.Maui.Graphics.WindingMode windingMode)
void static FillPath(this Microsoft.Maui.Graphics.ICanvas target, Microsoft.Maui.Graphics.PathF path)
```

## Parameters

| Parameter | Description |
|---|---|
| `target` | The canvas to draw on. |
| `path` | The path to fill. |

## Remarks

For best results with fill operations, ensure the path represents a closed shape by calling `Close` or manually connecting the end point back to the start point. Unclosed paths may produce unexpected results or exceptions depending on the graphics backend.

## See also

- Declaring type: [[CanvasExtensions (Graphics)|CanvasExtensions (Graphics)]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
