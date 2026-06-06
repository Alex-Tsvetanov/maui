---
title: "Easing"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.Easing"
namespace: "Microsoft.Maui"
kind: class
platforms:
  - All platforms (.NET)
  - Android
  - iOS
  - Mac Catalyst
  - Windows
  - Tizen
  - .NET Standard
  - .NET Standard 2.0
assemblies:
  - src
---

# Easing

> [!abstract] Class in `Microsoft.Maui`
> Full name: `Microsoft.Maui.Easing`

Functions that modify values non-linearly, generally used for animations.

## Platforms

| Platform | Available |
|---|---|
| All platforms (.NET) | ✅ |
| Android | ✅ |
| iOS | ✅ |
| Mac Catalyst | ✅ |
| Windows | ✅ |
| Tizen | ✅ |
| .NET Standard | ✅ |
| .NET Standard 2.0 | ✅ |


## Constructors

| Name | Summary |
|---|---|
| [[Easing.Easing\|Easing]] | Creates a new `Easing` object with the `easingFunc` function. |

## Properties

| Name | Summary |
|---|---|
| [[Easing.Default\|Default]] | The default easing function that is used. Defaults to `CubicInOut`. |

## Methods

| Name | Summary |
|---|---|
| [[Easing.Ease\|Ease]] | Applies the easing function to the specified value `v`. |

## Fields

| Name | Summary |
|---|---|
| [[Easing.BounceIn\|BounceIn]] | Jumps towards, and then bounces as it settles at the final value. |
| [[Easing.BounceOut\|BounceOut]] | Leaps to final values, bounces 3 times, and settles. |
| [[Easing.CubicIn\|CubicIn]] | Starts slowly and accelerates. |
| [[Easing.CubicInOut\|CubicInOut]] | Accelerates and decelerates. Often a natural-looking choice. |
| [[Easing.CubicOut\|CubicOut]] | Starts quickly and the decelerates. |
| [[Easing.Linear\|Linear]] | Linear transformation. |
| [[Easing.SinIn\|SinIn]] | Smoothly accelerates. |
| [[Easing.SinInOut\|SinInOut]] | Accelerates in and decelerates out. |
| [[Easing.SinOut\|SinOut]] | Smoothly decelerates. |
| [[Easing.SpringIn\|SpringIn]] | Moves away and then leaps toward the final value. |
| [[Easing.SpringOut\|SpringOut]] | Overshoots and then returns. |

## Operators

| Name | Summary |
|---|---|
| [[Easing.implicit operator Microsoft.Maui.Easing!\|implicit operator Microsoft.Maui.Easing!]] |  |

## Remarks

Easing functions are applied to input values in the range [0,1]. The cubic easing functions are often considered to look most natural. If developers wish to use their own easing functions, they should return a value of 0 for an input of 0 and a value of 1 for an input of 1 or the animation will have a jump.

## Guide

- 📖 Conceptual: [[easing]]

## See also

- [[_Microsoft.Maui|Microsoft.Maui namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.easing)
