---
title: "Matrix"
tags:
  - api
  - kind/struct
  - ns/Microsoft-Maui-Controls-Shapes
aliases:
  - "Microsoft.Maui.Controls.Shapes.Matrix"
namespace: "Microsoft.Maui.Controls.Shapes"
kind: struct
platforms:
  - All platforms (.NET)
  - Android
  - iOS
  - Mac Catalyst
  - Windows
  - Tizen
  - .NET Standard
assemblies:
  - Controls
---

# Matrix

> [!abstract] Struct in `Microsoft.Maui.Controls.Shapes`
> Full name: `Microsoft.Maui.Controls.Shapes.Matrix`

Represents a 3x3 affine transformation matrix used for 2D transformations such as rotation, scaling, skewing, and translation.

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


## Constructors

| Name | Summary |
|---|---|
| [[Matrix.Matrix\|Matrix]] | Initializes a new `Matrix` with the specified values. |

## Properties

| Name | Summary |
|---|---|
| [[Matrix.Determinant\|Determinant]] |  |
| [[Matrix.HasInverse\|HasInverse]] | Gets the determinant of this matrix. |
| [[Matrix.Identity\|Identity]] | Gets the identity matrix. |
| [[Matrix.IsIdentity\|IsIdentity]] |  |
| [[Matrix.M11\|M11]] |  |
| [[Matrix.M12\|M12]] |  |
| [[Matrix.M21\|M21]] |  |
| [[Matrix.M22\|M22]] |  |
| [[Matrix.OffsetX\|OffsetX]] |  |
| [[Matrix.OffsetY\|OffsetY]] |  |

## Methods

| Name | Summary |
|---|---|
| [[Matrix.Append\|Append]] | Appends the specified matrix to this matrix. |
| [[Matrix.Equals\|Equals]] | Gets or sets the value at row 1, column 1 of the matrix (horizontal scale component). |
| [[Matrix.GetHashCode\|GetHashCode]] |  |
| [[Matrix.Invert\|Invert]] | Inverts this matrix if it is invertible. |
| [[Matrix.Multiply\|Multiply]] | Gets a value indicating whether this matrix is the identity matrix. |
| [[Matrix.Prepend\|Prepend]] | Prepends the specified matrix to this matrix. |
| [[Matrix.Rotate\|Rotate]] | Appends a rotation transformation to this matrix. |
| [[Matrix.RotateAt\|RotateAt]] | Appends a rotation transformation around the specified point to this matrix. |
| [[Matrix.RotateAtPrepend\|RotateAtPrepend]] | Prepends a rotation transformation around the specified point to this matrix. |
| [[Matrix.RotatePrepend\|RotatePrepend]] | Prepends a rotation transformation to this matrix. |
| [[Matrix.Scale\|Scale]] | Appends a scale transformation to this matrix. |
| [[Matrix.ScaleAt\|ScaleAt]] | Appends a scale transformation around the specified point to this matrix. |
| [[Matrix.ScaleAtPrepend\|ScaleAtPrepend]] | Prepends a scale transformation around the specified point to this matrix. |
| [[Matrix.ScalePrepend\|ScalePrepend]] | Prepends a scale transformation to this matrix. |
| [[Matrix.SetIdentity\|SetIdentity]] | Sets this matrix to the identity matrix. |
| [[Matrix.Skew\|Skew]] | Appends a skew transformation to this matrix. |
| [[Matrix.SkewPrepend\|SkewPrepend]] | Prepends a skew transformation to this matrix. |
| [[Matrix.Transform\|Transform]] | Transforms the specified point by this matrix and returns the result. |
| [[Matrix.Translate\|Translate]] | Appends a translation transformation to this matrix. |
| [[Matrix.TranslatePrepend\|TranslatePrepend]] | Prepends a translation transformation to this matrix. |

## Operators

| Name | Summary |
|---|---|
| [[Matrix.operator !=\|operator !=]] |  |
| [[Matrix.operator _\|operator *]] |  |
| [[Matrix.operator ==\|operator ==]] |  |

## See also

- [[_Microsoft.Maui.Controls.Shapes|Microsoft.Maui.Controls.Shapes namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.shapes.matrix)
