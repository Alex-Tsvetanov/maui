---
title: "Microsoft.Maui.Controls.Compatibility"
tags:
  - api
  - namespace
  - ns/Microsoft-Maui-Controls-Compatibility
---

# Microsoft.Maui.Controls.Compatibility

> [!info] Namespace
> `Microsoft.Maui.Controls.Compatibility` — 14 public types.

- [Online namespace docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.compatibility)

## Overview

`Microsoft.Maui.Controls.Compatibility` provides the Xamarin.Forms-era layout controls and supporting types, carried forward into .NET MAUI to ease porting of existing applications. Where .NET MAUI introduces streamlined, single-pass layout primitives, this namespace preserves the original arrangement controls and their measurement behavior so legacy XAML and layout logic continue to work with minimal changes.

The core of the namespace is its set of layout containers. [[Grid (Compatibility)|Grid]], [[StackLayout (Compatibility)|StackLayout]], [[AbsoluteLayout (Compatibility)|AbsoluteLayout]], [[FlexLayout (Compatibility)|FlexLayout]], and [[RelativeLayout|RelativeLayout]] each arrange and group child controls using the compatibility measurement model. These build on the shared base [[Layout{T}|Layout&lt;T&gt;]], the base class for layouts that allow you to arrange and group UI controls in your application.

Positioning in a [[RelativeLayout|RelativeLayout]] is expressed through [[Constraint|Constraint]] and [[ConstraintExpression|ConstraintExpression]], with [[ConstraintTypeConverter|ConstraintTypeConverter]] enabling these constraints to be authored in XAML. Strongly typed child collections are exposed through the [[IGridList{T}|IGridList&lt;T&gt;]], [[IAbsoluteList{T}|IAbsoluteList&lt;T&gt;]], and [[IRelativeList{T}|IRelativeList&lt;T&gt;]] interfaces, while [[INativeElementView|INativeElementView]] supports hosting native platform elements within the compatibility layout tree.

> [!tip] Prefer the modern equivalents (such as the non-compatibility `Grid`, `StackLayout`, and `FlexLayout`) for new code; reach for this namespace primarily when migrating Xamarin.Forms layouts.

## Key types

- [[Grid (Compatibility)|Grid]] — arranges child controls in rows and columns using the compatibility layout model.
- [[StackLayout (Compatibility)|StackLayout]] — stacks child controls in a single horizontal or vertical line.
- [[AbsoluteLayout (Compatibility)|AbsoluteLayout]] — positions and sizes children using absolute or proportional bounds.
- [[FlexLayout (Compatibility)|FlexLayout]] — arranges children with flexbox-style layout rules.
- [[RelativeLayout|RelativeLayout]] — positions children relative to one another or to the parent via constraints.
- [[Layout{T}|Layout&lt;T&gt;]] — base class for layouts that arrange and group UI controls in your application.
- [[Constraint|Constraint]] — describes a positional or sizing constraint used by relative layouts.
- [[ConstraintExpression|ConstraintExpression]] — declares a constraint relative to another element or property.
- [[ConstraintTypeConverter|ConstraintTypeConverter]] — converts string values into constraints for XAML authoring.
- [[IGridList{T}|IGridList&lt;T&gt;]] — strongly typed child collection for a grid layout.
- [[IAbsoluteList{T}|IAbsoluteList&lt;T&gt;]] — strongly typed child collection for an absolute layout.
- [[INativeElementView|INativeElementView]] — exposes a hosted native platform element within the layout tree.


## Classes

| Type | Summary |
|---|---|
| [[AbsoluteLayout (Compatibility)\|AbsoluteLayout (Compatibility)]] |  |
| [[Constraint\|Constraint]] |  |
| [[ConstraintExpression\|ConstraintExpression]] |  |
| [[ConstraintTypeConverter\|ConstraintTypeConverter]] |  |
| [[FlexLayout (Compatibility)\|FlexLayout (Compatibility)]] |  |
| [[Grid (Compatibility)\|Grid (Compatibility)]] |  |
| [[Layout (Compatibility)\|Layout (Compatibility)]] | Base class for layouts that allow you to arrange and group UI controls in your application. |
| [[Layout{T}\|Layout<T>]] | Base class for layouts that allow you to arrange and group UI controls in your application. |
| [[RelativeLayout\|RelativeLayout]] |  |
| [[StackLayout (Compatibility)\|StackLayout (Compatibility)]] |  |

## Interfaces

| Type | Summary |
|---|---|
| [[IAbsoluteList{T}\|IAbsoluteList<T>]] |  |
| [[IGridList{T}\|IGridList<T>]] |  |
| [[INativeElementView\|INativeElementView]] |  |
| [[IRelativeList{T}\|IRelativeList<T>]] |  |

## See also

- [[_API Reference]]
