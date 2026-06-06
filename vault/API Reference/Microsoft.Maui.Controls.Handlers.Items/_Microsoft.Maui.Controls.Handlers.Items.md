---
title: "Microsoft.Maui.Controls.Handlers.Items"
tags:
  - api
  - namespace
  - ns/Microsoft-Maui-Controls-Handlers-Items
---

# Microsoft.Maui.Controls.Handlers.Items

> [!info] Namespace
> `Microsoft.Maui.Controls.Handlers.Items` — 64 public types.

- [Online namespace docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.handlers.items)

## Overview

`Microsoft.Maui.Controls.Handlers.Items` contains the platform-side handler and infrastructure types that render .NET MAUI's collection-oriented controls — primarily `CollectionView`, `CarouselView`, and their grouped, selectable, and reorderable variants. These handlers bridge the cross-platform control definitions to the underlying native virtualizing list/collection widgets (such as Android's `RecyclerView` and the iOS collection view stack), so that large, data-bound item lists scroll smoothly and recycle item views efficiently.

The namespace is organized around a few repeating roles. **Handlers** like [[CollectionViewHandler|CollectionViewHandler]] and [[CarouselViewHandler|CarouselViewHandler]] are the entry points that map a MAUI control to its native realization. **Sources** (e.g. [[IItemsViewSource|IItemsViewSource]] and its grouped/observable derivatives) wrap the bound collection and notify the UI of changes. **Adaptors/adapters** such as [[ItemTemplateAdaptor|ItemTemplateAdaptor]] and the `ItemsViewAdapter` family materialize data items into templated cells, while **controllers**, **delegators**, **cells**, and **layouts** coordinate scrolling, selection, spacing, and measurement.

Because many of these types are generic over a `TItemsView` and are split per concern (handler, controller, adapter, source, layout), they compose into a layered pipeline: a handler creates a native collection, a source feeds it data, an adaptor turns each item into a recycled view via its `DataTemplate`, and layout/decoration types control how those views are arranged. Most of these types are framework internals you rarely touch directly, but they are the machinery behind the high-level items controls.

> [!info] Internals
> The majority of these types are platform-implementation details for the items controls. Application code normally interacts with `CollectionView`/`CarouselView` rather than these handlers directly.

## Key types

- [[CollectionViewHandler|CollectionViewHandler]] — handler that renders `CollectionView` on each native platform.
- [[CarouselViewHandler|CarouselViewHandler]] — handler that renders the swipeable `CarouselView`.
- [[ItemsViewHandler{TItemsView}|ItemsViewHandler<TItemsView>]] — generic base handler shared by the items-view controls.
- [[StructuredItemsViewHandler{TItemsView}|StructuredItemsViewHandler<TItemsView>]] — handler adding header/footer and item layout structure.
- [[GroupableItemsViewHandler{TItemsView}|GroupableItemsViewHandler<TItemsView>]] — handler supporting grouped item sources.
- [[SelectableItemsViewHandler{TItemsView}|SelectableItemsViewHandler<TItemsView>]] — handler adding single/multiple selection behavior.
- [[ReorderableItemsViewHandler{TItemsView}|ReorderableItemsViewHandler<TItemsView>]] — handler supporting drag-to-reorder items.
- [[IItemsViewSource|IItemsViewSource]] — abstraction over the bound item collection consumed by the handlers.
- [[IObservableItemsViewSource|IObservableItemsViewSource]] — item source that propagates collection-change notifications to the UI.
- [[IGroupableItemsViewSource|IGroupableItemsViewSource]] — item source that exposes grouped data to the collection.
- [[ItemTemplateAdaptor|ItemTemplateAdaptor]] — adaptor that materializes data items into templated, recyclable views.
- [[CollectionViewSelectionChangedEventArgs|CollectionViewSelectionChangedEventArgs]] — event data describing selection changes in a collection view.


## Classes

| Type | Summary |
|---|---|
| [[CarouselTemplatedCell\|CarouselTemplatedCell]] |  |
| [[CarouselViewAdapter{TItemsView, TItemsViewSource}\|CarouselViewAdapter<TItemsView, TItemsViewSource>]] |  |
| [[CarouselViewController\|CarouselViewController]] |  |
| [[CarouselViewDelegator\|CarouselViewDelegator]] |  |
| [[CarouselViewHandler\|CarouselViewHandler]] |  |
| [[CarouselViewItemTemplateAdaptor\|CarouselViewItemTemplateAdaptor]] |  |
| [[CarouselViewLayout\|CarouselViewLayout]] |  |
| [[CollectionViewHandler\|CollectionViewHandler]] |  |
| [[CollectionViewSelectionChangedEventArgs\|CollectionViewSelectionChangedEventArgs]] |  |
| [[DefaultCell\|DefaultCell]] |  |
| [[EmptyItemAdaptor\|EmptyItemAdaptor]] |  |
| [[EmptyViewAdapter\|EmptyViewAdapter]] |  |
| [[GridViewLayout\|GridViewLayout]] |  |
| [[GroupItemSource\|GroupItemSource]] |  |
| [[GroupItemTemplateAdaptor\|GroupItemTemplateAdaptor]] |  |
| [[GroupableItemsViewAdapter{TItemsView, TItemsViewSource}\|GroupableItemsViewAdapter<TItemsView, TItemsViewSource>]] |  |
| [[GroupableItemsViewController{TItemsView}\|GroupableItemsViewController<TItemsView>]] |  |
| [[GroupableItemsViewDelegator{TItemsView, TViewController}\|GroupableItemsViewDelegator<TItemsView, TViewController>]] |  |
| [[GroupableItemsViewHandler{TItemsView}\|GroupableItemsViewHandler<TItemsView>]] |  |
| [[IndexPathHelpers\|IndexPathHelpers]] |  |
| [[ItemContentView\|ItemContentView]] |  |
| [[ItemTemplateAdaptor\|ItemTemplateAdaptor]] |  |
| [[ItemsViewAdapter{TItemsView, TItemsViewSource}\|ItemsViewAdapter<TItemsView, TItemsViewSource>]] |  |
| [[ItemsViewCell\|ItemsViewCell]] |  |
| [[ItemsViewController{TItemsView}\|ItemsViewController<TItemsView>]] |  |
| [[ItemsViewDelegator{TItemsView, TViewController}\|ItemsViewDelegator<TItemsView, TViewController>]] |  |
| [[ItemsViewHandler{TItemsView}\|ItemsViewHandler<TItemsView>]] |  |
| [[ItemsViewLayout\|ItemsViewLayout]] |  |
| [[LayoutAttributesChangedEventArgs\|LayoutAttributesChangedEventArgs]] |  |
| [[ListViewLayout\|ListViewLayout]] |  |
| [[MauiCarouselRecyclerView\|MauiCarouselRecyclerView]] |  |
| [[MauiCarouselView\|MauiCarouselView]] |  |
| [[MauiCollectionView\|MauiCollectionView]] |  |
| [[MauiCollectionView{TItemsView}\|MauiCollectionView<TItemsView>]] |  |
| [[MauiGroupableItemsView{TItemsView}\|MauiGroupableItemsView<TItemsView>]] |  |
| [[MauiRecyclerView{TItemsView, TAdapter, TItemsViewSource}\|MauiRecyclerView<TItemsView, TAdapter, TItemsViewSource>]] |  |
| [[MauiSelectableItemsView{TItemsView}\|MauiSelectableItemsView<TItemsView>]] |  |
| [[MauiStructuredItemsView{TItemsView}\|MauiStructuredItemsView<TItemsView>]] |  |
| [[RecyclerViewScrollListener{TItemsView, TItemsViewSource}\|RecyclerViewScrollListener<TItemsView, TItemsViewSource>]] |  |
| [[ReorderableItemsViewAdapter{TItemsView, TItemsViewSource}\|ReorderableItemsViewAdapter<TItemsView, TItemsViewSource>]] |  |
| [[ReorderableItemsViewController{TItemsView}\|ReorderableItemsViewController<TItemsView>]] |  |
| [[ReorderableItemsViewDelegator{TItemsView, TViewController}\|ReorderableItemsViewDelegator<TItemsView, TViewController>]] |  |
| [[ReorderableItemsViewHandler{TItemsView}\|ReorderableItemsViewHandler<TItemsView>]] |  |
| [[ScrollToPositionExtensions\|ScrollToPositionExtensions]] |  |
| [[SelectableItemsViewAdapter{TItemsView, TItemsSource}\|SelectableItemsViewAdapter<TItemsView, TItemsSource>]] |  |
| [[SelectableItemsViewController{TItemsView}\|SelectableItemsViewController<TItemsView>]] |  |
| [[SelectableItemsViewDelegator{TItemsView, TViewController}\|SelectableItemsViewDelegator<TItemsView, TViewController>]] |  |
| [[SelectableItemsViewHandler{TItemsView}\|SelectableItemsViewHandler<TItemsView>]] |  |
| [[SelectableViewHolder\|SelectableViewHolder]] |  |
| [[SimpleItemTouchHelperCallback\|SimpleItemTouchHelperCallback]] |  |
| [[SnapManager\|SnapManager]] |  |
| [[SpacingItemDecoration\|SpacingItemDecoration]] |  |
| [[StructuredItemsViewAdapter{TItemsView, TItemsViewSource}\|StructuredItemsViewAdapter<TItemsView, TItemsViewSource>]] |  |
| [[StructuredItemsViewController{TItemsView}\|StructuredItemsViewController<TItemsView>]] |  |
| [[StructuredItemsViewHandler{TItemsView}\|StructuredItemsViewHandler<TItemsView>]] |  |
| [[TemplatedCell\|TemplatedCell]] |  |
| [[TemplatedItemViewHolder\|TemplatedItemViewHolder]] |  |

## Interfaces

| Type | Summary |
|---|---|
| [[IGroupableItemsViewSource\|IGroupableItemsViewSource]] |  |
| [[IItemTouchHelperAdapter\|IItemTouchHelperAdapter]] |  |
| [[IItemsViewSource\|IItemsViewSource]] |  |
| [[ILoopItemsViewSource\|ILoopItemsViewSource]] |  |
| [[IMauiCarouselRecyclerView\|IMauiCarouselRecyclerView]] |  |
| [[IMauiRecyclerView{TItemsView}\|IMauiRecyclerView<TItemsView>]] |  |
| [[IObservableItemsViewSource\|IObservableItemsViewSource]] |  |

## See also

- [[_API Reference]]
