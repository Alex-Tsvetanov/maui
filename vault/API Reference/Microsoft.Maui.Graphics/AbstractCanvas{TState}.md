---
title: "AbstractCanvas<TState>"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.AbstractCanvas<TState>"
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

# AbstractCanvas<TState>

> [!abstract] Class in `Microsoft.Maui.Graphics`
> Full name: `Microsoft.Maui.Graphics.AbstractCanvas<TState>`

Provides an abstract base implementation of the `ICanvas` interface.

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
| [[AbstractCanvas{TState}.AbstractCanvas\|AbstractCanvas]] |  |

## Properties

| Name | Summary |
|---|---|
| [[AbstractCanvas{TState}.Alpha\|Alpha]] |  |
| [[AbstractCanvas{TState}.Antialias\|Antialias]] |  |
| [[AbstractCanvas{TState}.AssignedStrokeLimit\|AssignedStrokeLimit]] |  |
| [[AbstractCanvas{TState}.BlendMode\|BlendMode]] |  |
| [[AbstractCanvas{TState}.CurrentState\|CurrentState]] |  |
| [[AbstractCanvas{TState}.DisplayScale\|DisplayScale]] |  |
| [[AbstractCanvas{TState}.FillColor\|FillColor]] |  |
| [[AbstractCanvas{TState}.Font\|Font]] |  |
| [[AbstractCanvas{TState}.FontColor\|FontColor]] |  |
| [[AbstractCanvas{TState}.FontSize\|FontSize]] |  |
| [[AbstractCanvas{TState}.LimitStrokeScaling\|LimitStrokeScaling]] |  |
| [[AbstractCanvas{TState}.LimitStrokeScalingEnabled\|LimitStrokeScalingEnabled]] |  |
| [[AbstractCanvas{TState}.MiterLimit\|MiterLimit]] |  |
| [[AbstractCanvas{TState}.PlatformStrokeSize\|PlatformStrokeSize]] |  |
| [[AbstractCanvas{TState}.StrokeColor\|StrokeColor]] |  |
| [[AbstractCanvas{TState}.StrokeDashOffset\|StrokeDashOffset]] |  |
| [[AbstractCanvas{TState}.StrokeDashPattern\|StrokeDashPattern]] |  |
| [[AbstractCanvas{TState}.StrokeLimit\|StrokeLimit]] |  |
| [[AbstractCanvas{TState}.StrokeLineCap\|StrokeLineCap]] |  |
| [[AbstractCanvas{TState}.StrokeLineJoin\|StrokeLineJoin]] |  |
| [[AbstractCanvas{TState}.StrokeSize\|StrokeSize]] |  |

## Methods

| Name | Summary |
|---|---|
| [[AbstractCanvas{TState}.ClipPath\|ClipPath]] |  |
| [[AbstractCanvas{TState}.ClipRectangle\|ClipRectangle]] |  |
| [[AbstractCanvas{TState}.ConcatenateTransform\|ConcatenateTransform]] |  |
| [[AbstractCanvas{TState}.Dispose\|Dispose]] |  |
| [[AbstractCanvas{TState}.DrawArc\|DrawArc]] |  |
| [[AbstractCanvas{TState}.DrawEllipse\|DrawEllipse]] |  |
| [[AbstractCanvas{TState}.DrawImage\|DrawImage]] |  |
| [[AbstractCanvas{TState}.DrawLine\|DrawLine]] |  |
| [[AbstractCanvas{TState}.DrawPath\|DrawPath]] |  |
| [[AbstractCanvas{TState}.DrawRectangle\|DrawRectangle]] |  |
| [[AbstractCanvas{TState}.DrawRoundedRectangle\|DrawRoundedRectangle]] |  |
| [[AbstractCanvas{TState}.DrawString\|DrawString]] |  |
| [[AbstractCanvas{TState}.DrawText\|DrawText]] |  |
| [[AbstractCanvas{TState}.FillArc\|FillArc]] |  |
| [[AbstractCanvas{TState}.FillEllipse\|FillEllipse]] |  |
| [[AbstractCanvas{TState}.FillPath\|FillPath]] |  |
| [[AbstractCanvas{TState}.FillRectangle\|FillRectangle]] |  |
| [[AbstractCanvas{TState}.FillRoundedRectangle\|FillRoundedRectangle]] |  |
| [[AbstractCanvas{TState}.GetStringSize\|GetStringSize]] |  |
| [[AbstractCanvas{TState}.PlatformConcatenateTransform\|PlatformConcatenateTransform]] |  |
| [[AbstractCanvas{TState}.PlatformDrawArc\|PlatformDrawArc]] |  |
| [[AbstractCanvas{TState}.PlatformDrawEllipse\|PlatformDrawEllipse]] |  |
| [[AbstractCanvas{TState}.PlatformDrawLine\|PlatformDrawLine]] |  |
| [[AbstractCanvas{TState}.PlatformDrawPath\|PlatformDrawPath]] |  |
| [[AbstractCanvas{TState}.PlatformDrawRectangle\|PlatformDrawRectangle]] |  |
| [[AbstractCanvas{TState}.PlatformDrawRoundedRectangle\|PlatformDrawRoundedRectangle]] |  |
| [[AbstractCanvas{TState}.PlatformRotate\|PlatformRotate]] |  |
| [[AbstractCanvas{TState}.PlatformScale\|PlatformScale]] |  |
| [[AbstractCanvas{TState}.PlatformSetStrokeDashPattern\|PlatformSetStrokeDashPattern]] |  |
| [[AbstractCanvas{TState}.PlatformTranslate\|PlatformTranslate]] |  |
| [[AbstractCanvas{TState}.ResetState\|ResetState]] |  |
| [[AbstractCanvas{TState}.RestoreState\|RestoreState]] |  |
| [[AbstractCanvas{TState}.Rotate\|Rotate]] |  |
| [[AbstractCanvas{TState}.SaveState\|SaveState]] |  |
| [[AbstractCanvas{TState}.Scale\|Scale]] |  |
| [[AbstractCanvas{TState}.SetFillPaint\|SetFillPaint]] |  |
| [[AbstractCanvas{TState}.SetShadow\|SetShadow]] |  |
| [[AbstractCanvas{TState}.StateRestored\|StateRestored]] |  |
| [[AbstractCanvas{TState}.SubtractFromClip\|SubtractFromClip]] |  |
| [[AbstractCanvas{TState}.Translate\|Translate]] |  |

## Remarks

This class handles state management, coordinate transformation, and common drawing operations, while delegating platform-specific rendering to derived classes.

## See also

- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.graphics.abstractcanvas)
