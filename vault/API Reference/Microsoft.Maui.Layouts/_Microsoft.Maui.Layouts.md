---
title: "Microsoft.Maui.Layouts"
tags:
  - api
  - namespace
  - ns/Microsoft-Maui-Layouts
---

# Microsoft.Maui.Layouts

> [!info] Namespace
> `Microsoft.Maui.Layouts` — 18 public types.

- [Online namespace docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.layouts)

## Overview

`Microsoft.Maui.Layouts` provides the measurement and arrangement engine that sits behind .NET MAUI's layout panels. Where the public layout views (such as the stack, grid, absolute, and flex layouts) describe *what* should be arranged, the types in this namespace implement *how* those children are measured and positioned within the available space. The contract is centered on [[ILayoutManager|ILayoutManager]] and its shared base [[LayoutManager|LayoutManager]], which expose the two-pass `Measure`/`ArrangeChildren` model used throughout the framework.

Each panel type pairs with a dedicated manager that encodes its specific positioning rules: [[GridLayoutManager|GridLayoutManager]] for row/column grids, [[AbsoluteLayoutManager|AbsoluteLayoutManager]] for coordinate- and proportion-based placement, [[HorizontalStackLayoutManager|HorizontalStackLayoutManager]] and [[VerticalStackLayoutManager|VerticalStackLayoutManager]] for single-direction stacking (with [[StackLayoutManager (Layouts)|StackLayoutManager]] as the shared stacking base), and [[FlexLayoutManager|FlexLayoutManager]] for flexbox-style layout.

The Flex-prefixed types ([[FlexDirection|FlexDirection]], [[FlexWrap|FlexWrap]], [[FlexJustify|FlexJustify]], [[FlexAlignItems|FlexAlignItems]], [[FlexAlignContent|FlexAlignContent]], [[FlexAlignSelf|FlexAlignSelf]], [[FlexBasis|FlexBasis]], and [[FlexPosition|FlexPosition]]) supply the flexbox configuration values that drive `FlexLayoutManager`. Supporting helpers such as [[LayoutExtensions|LayoutExtensions]] and the [[AbsoluteLayoutFlags|AbsoluteLayoutFlags]] options round out the namespace. In most apps you consume these indirectly through the layout views; you interact with the managers directly when authoring a custom layout or handheld measurement logic.

## Key types

- [[ILayoutManager|ILayoutManager]] — Contract defining the measure and arrange passes a layout manager must implement.
- [[LayoutManager|LayoutManager]] — Shared base class providing common measurement/arrangement behavior for layout managers.
- [[GridLayoutManager|GridLayoutManager]] — Measures and arranges children using row and column definitions.
- [[AbsoluteLayoutManager|AbsoluteLayoutManager]] — Positions children by absolute or proportional bounds.
- [[FlexLayoutManager|FlexLayoutManager]] — Arranges children using flexbox layout rules.
- [[HorizontalStackLayoutManager|HorizontalStackLayoutManager]] — Stacks children left-to-right in a single row.
- [[VerticalStackLayoutManager|VerticalStackLayoutManager]] — Stacks children top-to-bottom in a single column.
- [[StackLayoutManager (Layouts)|StackLayoutManager]] — Shared base for the horizontal and vertical stack managers.
- [[AbsoluteLayoutFlags|AbsoluteLayoutFlags]] — Flags controlling how absolute-layout bounds are interpreted (positional vs. proportional).
- [[FlexDirection|FlexDirection]] — Configures the main-axis direction for flex layout.
- [[FlexJustify|FlexJustify]] — Controls alignment of flex items along the main axis.
- [[LayoutExtensions|LayoutExtensions]] — Helper extension methods used by layout managers.


## Classes

| Type | Summary |
|---|---|
| [[AbsoluteLayoutFlags\|AbsoluteLayoutFlags]] |  |
| [[AbsoluteLayoutManager\|AbsoluteLayoutManager]] |  |
| [[FlexAlignContent\|FlexAlignContent]] |  |
| [[FlexAlignItems\|FlexAlignItems]] |  |
| [[FlexAlignSelf\|FlexAlignSelf]] |  |
| [[FlexBasis\|FlexBasis]] |  |
| [[FlexDirection\|FlexDirection]] |  |
| [[FlexJustify\|FlexJustify]] |  |
| [[FlexLayoutManager\|FlexLayoutManager]] |  |
| [[FlexPosition\|FlexPosition]] |  |
| [[FlexWrap\|FlexWrap]] |  |
| [[GridLayoutManager\|GridLayoutManager]] |  |
| [[HorizontalStackLayoutManager\|HorizontalStackLayoutManager]] |  |
| [[LayoutExtensions\|LayoutExtensions]] |  |
| [[LayoutManager\|LayoutManager]] |  |
| [[StackLayoutManager (Layouts)\|StackLayoutManager (Layouts)]] |  |
| [[VerticalStackLayoutManager\|VerticalStackLayoutManager]] |  |

## Interfaces

| Type | Summary |
|---|---|
| [[ILayoutManager\|ILayoutManager]] |  |

## See also

- [[_API Reference]]
