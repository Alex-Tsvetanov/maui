---
title: "GridLength"
tags:
  - api
  - kind/struct
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.GridLength"
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

# GridLength

> [!abstract] Struct in `Microsoft.Maui`
> Full name: `Microsoft.Maui.GridLength`

Used to define the size (width/height) of Grid ColumnDefinition and RowDefinition.

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
| [[GridLength.GridLength\|GridLength]] | Gets a value indicating whether this GridLength uses Absolute units. |
| [[GridLength.implicit operator Microsoft.Maui.GridLength\|implicit operator Microsoft.Maui.GridLength]] |  |

## Properties

| Name | Summary |
|---|---|
| [[GridLength.GridUnitType\|GridUnitType]] | Gets the unit type that indicates how the GridLength is interpreted. |
| [[GridLength.IsAbsolute\|IsAbsolute]] |  |
| [[GridLength.IsAuto\|IsAuto]] |  |
| [[GridLength.IsStar\|IsStar]] |  |
| [[GridLength.Value\|Value]] | Gets the numeric value of the GridLength. Represents an absolute size or weight; ignored for Auto. |

## Methods

| Name | Summary |
|---|---|
| [[GridLength.Equals\|Equals]] | Determines whether the specified object is equal to the current GridLength. |
| [[GridLength.GetHashCode\|GetHashCode]] | Returns the hash code for this GridLength. |
| [[GridLength.ToString\|ToString]] | Returns a string that represents this GridLength. |

## Fields

| Name | Summary |
|---|---|
| [[GridLength.Auto\|Auto]] | A ready-to-use GridLength of `Auto`. Value is ignored. |
| [[GridLength.Star\|Star]] | A ready-to-use GridLength of `Star`. Distributes available space proportionally. |

## Operators

| Name | Summary |
|---|---|
| [[GridLength.operator !=\|operator !=]] |  |
| [[GridLength.operator ==\|operator ==]] |  |

## Remarks

GridLength of type `Absolute` represents exact size. GridLength of type `Auto` adapts to fit content size. GridLength of type `Star` distributes remaining space proportionally. This value type is readonly.

## See also

- [[_Microsoft.Maui|Microsoft.Maui namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.gridlength)
