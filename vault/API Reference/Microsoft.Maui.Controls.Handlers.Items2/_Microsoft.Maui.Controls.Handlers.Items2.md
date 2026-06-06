---
title: "Microsoft.Maui.Controls.Handlers.Items2"
tags:
  - api
  - namespace
  - ns/Microsoft-Maui-Controls-Handlers-Items2
---

# Microsoft.Maui.Controls.Handlers.Items2

> [!info] Namespace
> `Microsoft.Maui.Controls.Handlers.Items2` — 18 public types.

- [Online namespace docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.handlers.items2)

## Overview

`Microsoft.Maui.Controls.Handlers.Items2` contains the platform handler infrastructure that backs .NET MAUI's items-based controls — `CollectionView`, `CarouselView`, and related virtualized list surfaces — on Apple platforms. The `2` suffix throughout the namespace marks this as a second-generation implementation that maps MAUI items views onto the native iOS/Mac Catalyst `UICollectionView` stack. It exists alongside the original items handlers as an updated rendering path with revised layout and recycling behavior.

The namespace is organized around the standard UIKit collection pattern. Handler types such as [[CollectionViewHandler2|CollectionViewHandler2]] and [[CarouselViewHandler2|CarouselViewHandler2]] connect a cross-platform control to its native view, while *controller* types like [[ItemsViewController2{TItemsView}|ItemsViewController2]] own the underlying collection view and drive data population. *Delegator* types route the native collection view's delegate callbacks back into the controllers, and *cell* types provide the reusable native cells that render each item.

Layered specialization adds capabilities on top of the base items controller: structured (header/footer/spacing) layout, grouping, selection, and reordering each have their own controller and delegator pair. Together these pieces translate MAUI data templates, layout, and interaction into native virtualized scrolling.

> [!info] Platform internals
> These are platform handler types used by MAUI's items controls under the hood. Application code normally works with the cross-platform `CollectionView`/`CarouselView` controls rather than calling these types directly.

## Key types

- [[CollectionViewHandler2|CollectionViewHandler2]] — Handler that maps `CollectionView` to the native iOS collection view stack.
- [[CarouselViewHandler2|CarouselViewHandler2]] — Handler that maps `CarouselView` to its native swipeable implementation.
- [[ItemsViewController2{TItemsView}|ItemsViewController2<TItemsView>]] — Base controller owning the native collection view and item data source.
- [[StructuredItemsViewController2{TItemsView}|StructuredItemsViewController2<TItemsView>]] — Adds header, footer, and item-layout (spacing/sizing) support.
- [[GroupableItemsViewController2{TItemsView}|GroupableItemsViewController2<TItemsView>]] — Adds grouped-data sectioning support.
- [[SelectableItemsViewController2{TItemsView}|SelectableItemsViewController2<TItemsView>]] — Adds single/multiple item selection support.
- [[ReorderableItemsViewController2{TItemsView}|ReorderableItemsViewController2<TItemsView>]] — Adds drag-to-reorder support.
- [[CarouselViewController2|CarouselViewController2]] — Controller specialized for carousel paging behavior.
- [[ItemsViewDelegator2{TItemsView, TViewController}|ItemsViewDelegator2<TItemsView, TViewController>]] — Base delegate bridge routing native collection-view callbacks to a controller.
- [[TemplatedCell2|TemplatedCell2]] — Reusable native cell that renders an item from a MAUI `DataTemplate`.
- [[DefaultCell2|DefaultCell2]] — Default cell used when no data template is supplied.
- [[LayoutAttributesChangedEventArgs2|LayoutAttributesChangedEventArgs2]] — Event data raised when a cell's layout attributes change.


## Classes

| Type | Summary |
|---|---|
| [[CarouselViewController2\|CarouselViewController2]] |  |
| [[CarouselViewDelegator2\|CarouselViewDelegator2]] |  |
| [[CarouselViewHandler2\|CarouselViewHandler2]] |  |
| [[CollectionViewHandler2\|CollectionViewHandler2]] |  |
| [[DefaultCell2\|DefaultCell2]] |  |
| [[GroupableItemsViewController2{TItemsView}\|GroupableItemsViewController2<TItemsView>]] |  |
| [[GroupableItemsViewDelegator2{TItemsView, TViewController}\|GroupableItemsViewDelegator2<TItemsView, TViewController>]] |  |
| [[ItemsViewCell2\|ItemsViewCell2]] |  |
| [[ItemsViewController2{TItemsView}\|ItemsViewController2<TItemsView>]] |  |
| [[ItemsViewDelegator2{TItemsView, TViewController}\|ItemsViewDelegator2<TItemsView, TViewController>]] |  |
| [[ItemsViewHandler2{TItemsView}\|ItemsViewHandler2<TItemsView>]] |  |
| [[LayoutAttributesChangedEventArgs2\|LayoutAttributesChangedEventArgs2]] |  |
| [[ReorderableItemsViewController2{TItemsView}\|ReorderableItemsViewController2<TItemsView>]] |  |
| [[ReorderableItemsViewDelegator2{TItemsView, TViewController}\|ReorderableItemsViewDelegator2<TItemsView, TViewController>]] |  |
| [[SelectableItemsViewController2{TItemsView}\|SelectableItemsViewController2<TItemsView>]] |  |
| [[SelectableItemsViewDelegator2{TItemsView, TViewController}\|SelectableItemsViewDelegator2<TItemsView, TViewController>]] |  |
| [[StructuredItemsViewController2{TItemsView}\|StructuredItemsViewController2<TItemsView>]] |  |
| [[TemplatedCell2\|TemplatedCell2]] |  |

## See also

- [[_API Reference]]
