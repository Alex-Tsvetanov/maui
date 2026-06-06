---
title: "CanvasState"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.CanvasState"
namespace: "Microsoft.Maui.Graphics"
kind: class
platforms:
  - All platforms (.NET)
  - Android
  - iOS
  - Mac Catalyst
  - macOS
  - Windows
  - Tizen
  - .NET Standard
assemblies:
  - Graphics
---

# CanvasState

> [!abstract] Class in `Microsoft.Maui.Graphics`
> Full name: `Microsoft.Maui.Graphics.CanvasState`

Represents the state of a canvas, including transformation and stroke properties.

## Platforms

| Platform | Available |
|---|---|
| All platforms (.NET) | ✅ |
| Android | ✅ |
| iOS | ✅ |
| Mac Catalyst | ✅ |
| macOS | ✅ |
| Windows | ✅ |
| Tizen | ✅ |
| .NET Standard | ✅ |


## Constructors

| Name | Summary |
|---|---|
| [[CanvasState.CanvasState\|CanvasState]] | Initializes a new instance of the `CanvasState` class. |

## Properties

| Name | Summary |
|---|---|
| [[CanvasState.Scale\|Scale]] | Gets or sets the transformation matrix for the canvas. |
| [[CanvasState.ScaleX\|ScaleX]] | Gets the horizontal scale factor derived from the transformation matrix. |
| [[CanvasState.ScaleY\|ScaleY]] | Gets the vertical scale factor derived from the transformation matrix. |
| [[CanvasState.StrokeDashOffset\|StrokeDashOffset]] | Gets or sets the distance into the dash pattern to start the dash. |
| [[CanvasState.StrokeDashPattern\|StrokeDashPattern]] | Gets or sets the pattern of dashes and gaps used to stroke paths. |
| [[CanvasState.StrokeSize\|StrokeSize]] | Gets or sets the width of the stroke used to draw an object's outline. |
| [[CanvasState.Transform\|Transform]] |  |

## Methods

| Name | Summary |
|---|---|
| [[CanvasState.Dispose\|Dispose]] | Releases resources used by the canvas state. |
| [[CanvasState.GetLengthScale\|GetLengthScale]] | Gets the scale factor for lengths based on the provided transformation matrix. |
| [[CanvasState.TransformChanged\|TransformChanged]] | Called when the transformation matrix changes. |

## See also

- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.graphics.canvasstate)
