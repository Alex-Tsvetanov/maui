---
title: "ICanvas"
tags:
  - api
  - kind/interface
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.ICanvas"
namespace: "Microsoft.Maui.Graphics"
kind: interface
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

# ICanvas

> [!abstract] Interface in `Microsoft.Maui.Graphics`
> Full name: `Microsoft.Maui.Graphics.ICanvas`

Represents a platform-agnostic canvas on which 2D graphics can be drawn using types from the `Graphics` namespace.

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


## Properties

| Name | Summary |
|---|---|
| [[ICanvas.Alpha\|Alpha]] | Sets the opacity of am object. |
| [[ICanvas.Antialias\|Antialias]] | Sets a value that indicates whether to use anti-aliasing is enabled. |
| [[ICanvas.BlendMode\|BlendMode]] | Sets the blend mode, which determines what happens when an object is rendered on top of an existing object. |
| [[ICanvas.DisplayScale\|DisplayScale]] | Gets or sets a value that represents the scaling factor to scale the UI by. |
| [[ICanvas.FillColor\|FillColor]] | Sets the color used to paint an object's interior. |
| [[ICanvas.Font\|Font]] | Sets the font used when drawing text. |
| [[ICanvas.FontColor\|FontColor]] | Sets the font color when drawing text. |
| [[ICanvas.FontSize\|FontSize]] | Sets the size of the font used when drawing text. |
| [[ICanvas.MiterLimit\|MiterLimit]] | Sets the limit of the miter length of line joins in an object. |
| [[ICanvas.StrokeColor\|StrokeColor]] | Sets the `Color` used to paint an object's outline. |
| [[ICanvas.StrokeDashOffset\|StrokeDashOffset]] | Sets the distance within the dash pattern where a dash begins. |
| [[ICanvas.StrokeDashPattern\|StrokeDashPattern]] | Sets the pattern of dashes and gaps that are used to outline an object. |
| [[ICanvas.StrokeLineCap\|StrokeLineCap]] | Sets the shape at the start and end of a line. |
| [[ICanvas.StrokeLineJoin\|StrokeLineJoin]] | Sets the type of join used at the vertices of a shape. |
| [[ICanvas.StrokeSize\|StrokeSize]] | Sets the width of the stroke used to draw an object's outline. |

## Methods

| Name | Summary |
|---|---|
| [[ICanvas.ClipPath\|ClipPath]] | Clips an object so that only the area that's within the region of a `PathF` object will be visible. |
| [[ICanvas.ClipRectangle\|ClipRectangle]] | Clips an object so that only the area that's within the region of the rectangle will be visible. |
| [[ICanvas.ConcatenateTransform\|ConcatenateTransform]] | Applies transformation specified by `transform` to a graphical object. |
| [[ICanvas.DrawArc\|DrawArc]] | Draws an arc onto the canvas. |
| [[ICanvas.DrawEllipse\|DrawEllipse]] | Draws an ellipse onto the canvas. |
| [[ICanvas.DrawImage\|DrawImage]] | Draws an image onto the canvas. |
| [[ICanvas.DrawLine\|DrawLine]] | Draws a line between two points onto the canvas. |
| [[ICanvas.DrawPath\|DrawPath]] | Draws the specified `path` onto the canvas. |
| [[ICanvas.DrawRectangle\|DrawRectangle]] | Draws a rectangle onto the canvas. |
| [[ICanvas.DrawRoundedRectangle\|DrawRoundedRectangle]] | Draws a rectangle with rounded corners onto the canvas. |
| [[ICanvas.DrawString\|DrawString]] | Draws a text string onto the canvas. |
| [[ICanvas.DrawText\|DrawText]] | Draws attributed text within a bounding box onto the canvas. |
| [[ICanvas.FillArc\|FillArc]] | Draws a filled arc onto the canvas. |
| [[ICanvas.FillEllipse\|FillEllipse]] | Draws a filled ellipse onto the canvas. |
| [[ICanvas.FillPath\|FillPath]] | Draws and fills the specified `path` onto the canvas. |
| [[ICanvas.FillRectangle\|FillRectangle]] | Draws a filled rectangle onto the canvas. |
| [[ICanvas.FillRoundedRectangle\|FillRoundedRectangle]] | Draws a filled rectangle with rounded corners onto the canvas. |
| [[ICanvas.GetStringSize\|GetStringSize]] | Calculates the area a string would occupy if drawn on the canvas. |
| [[ICanvas.ResetState\|ResetState]] | Resets the graphics state to its default values. |
| [[ICanvas.RestoreState\|RestoreState]] | Restores the graphics state to the most recently saved state. |
| [[ICanvas.Rotate\|Rotate]] | Rotates a graphical object around a point. |
| [[ICanvas.SaveState\|SaveState]] | Saves the current graphics state. |
| [[ICanvas.Scale\|Scale]] | Changes the size of a graphical object by scaling it. |
| [[ICanvas.SetFillPaint\|SetFillPaint]] | Sets `paint` as the fill of a graphical object. |
| [[ICanvas.SetShadow\|SetShadow]] | Adds a shadow to a graphical object. |
| [[ICanvas.SubtractFromClip\|SubtractFromClip]] | Clips an object so that only the area outside the of a rectangle will be visible. |
| [[ICanvas.Translate\|Translate]] | Shifts a graphical object in horizontal and vertical directions. |

## See also

- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.graphics.icanvas)
