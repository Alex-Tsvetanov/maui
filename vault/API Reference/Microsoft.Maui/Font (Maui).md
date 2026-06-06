---
title: "Font (Maui)"
tags:
  - api
  - kind/struct
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.Font"
namespace: "Microsoft.Maui"
kind: struct
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

# Font (Maui)

> [!abstract] Struct in `Microsoft.Maui`
> Full name: `Microsoft.Maui.Font`

Represents a font, including family, size, weight, slant, and auto-scaling settings.

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
| [[Font (Maui).Font\|Font]] |  |

## Properties

| Name | Summary |
|---|---|
| [[Font (Maui).AutoScalingEnabled\|AutoScalingEnabled]] |  |
| [[Font (Maui).Default\|Default]] | Gets the default font for the platform. |
| [[Font (Maui).Family\|Family]] | Gets the font family name, or null for the default system font. |
| [[Font (Maui).IsDefault\|IsDefault]] | Gets a value indicating whether this font is the default font (no family, non-positive size, default slant, and regular weight). |
| [[Font (Maui).Size\|Size]] | Gets the size of the font. |
| [[Font (Maui).Slant\|Slant]] | Gets the slant (style) of the font (e.g., default or italic). |
| [[Font (Maui).Weight\|Weight]] |  |

## Methods

| Name | Summary |
|---|---|
| [[Font (Maui).Equals\|Equals]] | Determines whether the specified object is equal to this font. |
| [[Font (Maui).GetHashCode\|GetHashCode]] | Returns the hash code for this font. |
| [[Font (Maui).OfSize\|OfSize]] | Creates a font with the specified family, size, weight, slant, and auto-scaling. |
| [[Font (Maui).SystemFontOfSize\|SystemFontOfSize]] | Returns a system font of the specified size, weight, slant, and auto-scaling. |
| [[Font (Maui).SystemFontOfWeight\|SystemFontOfWeight]] | Returns a font instance with the specified weight, slant, and auto-scaling with a default size. |
| [[Font (Maui).ToString\|ToString]] | Determines whether two fonts are not equal. |
| [[Font (Maui).WithAutoScaling\|WithAutoScaling]] | Gets the weight of the font, defaulting to `Regular` if unspecified. |
| [[Font (Maui).WithSize\|WithSize]] | Returns a new `Font` with the specified size. |
| [[Font (Maui).WithSlant\|WithSlant]] | Returns a new `Font` with the specified slant (style). |
| [[Font (Maui).WithWeight\|WithWeight]] | Returns a new `Font` with the specified weight. |

## Operators

| Name | Summary |
|---|---|
| [[Font (Maui).operator !=\|operator !=]] |  |
| [[Font (Maui).operator ==\|operator ==]] |  |

## Remarks

The font used to display text. Supported fonts and their rendering depend on the platform.

## See also

- [[_Microsoft.Maui|Microsoft.Maui namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.font)
