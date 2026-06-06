---
title: "Microsoft.Maui.Controls.Handlers.Compatibility"
tags:
  - api
  - namespace
  - ns/Microsoft-Maui-Controls-Handlers-Compatibility
---

# Microsoft.Maui.Controls.Handlers.Compatibility

> [!info] Namespace
> `Microsoft.Maui.Controls.Handlers.Compatibility` — 37 public types.

- [Online namespace docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.handlers.compatibility)

## Overview

`Microsoft.Maui.Controls.Handlers.Compatibility` is the bridge that lets .NET MAUI reuse the legacy Xamarin.Forms-style *renderer* model inside the modern handler architecture. Where MAUI normally maps a cross-platform control to a platform view through a handler, this namespace preserves the older pattern in which each control is paired with a platform-specific `Renderer` that builds and manages the native view directly. This keeps existing renderer-based code and third-party customizations working on MAUI while the rest of the framework moves to handlers.

The bulk of the namespace is renderer types for the controls that historically relied on this model — list and table presentation, navigation containers, cells, and a handful of structural controls. [[ListViewRenderer|ListViewRenderer]], [[TableViewRenderer|TableViewRenderer]], [[NavigationRenderer|NavigationRenderer]], [[TabbedRenderer|TabbedRenderer]], [[ShellRenderer|ShellRenderer]], [[FrameRenderer|FrameRenderer]], and [[PhoneFlyoutPageRenderer|PhoneFlyoutPageRenderer]] each adapt a cross-platform control to its native representation. Cell-oriented controls such as `TextCell`, `EntryCell`, `SwitchCell`, `ImageCell`, and `ViewCell` are served by dedicated renderers including [[TextCellRenderer|TextCellRenderer]], [[EntryCellRenderer|EntryCellRenderer]], [[ViewCellRenderer|ViewCellRenderer]], and the [[ICellRenderer|ICellRenderer]] contract they implement.

Underpinning these are the generic base renderers [[ViewRenderer{TElement, TNativeView}|ViewRenderer<TElement, TNativeView>]] and [[VisualElementRenderer{TElement, TPlatformElement}|VisualElementRenderer<TElement, TPlatformElement>]], which provide the shared element-to-native-view wiring that concrete renderers extend. Supporting cell views, adapters, and factories (`CellFactory`, `CellAdapter`, `ListViewAdaptor`, `TableViewAdaptor`) complete the plumbing that translates MAUI's data and templating into native list and table content.

> [!info] These are compatibility shims for the renderer-based model. New MAUI controls should use handlers rather than renderers; prefer these types only when migrating renderer-based code.

## Key types

- [[ViewRenderer{TElement, TNativeView}|ViewRenderer<TElement, TNativeView>]] — generic base renderer that wires a cross-platform element to its native view.
- [[VisualElementRenderer{TElement, TPlatformElement}|VisualElementRenderer<TElement, TPlatformElement>]] — base renderer providing shared visual-element to platform-element behavior.
- [[ListViewRenderer|ListViewRenderer]] — renders a `ListView` using the compatibility renderer model.
- [[TableViewRenderer|TableViewRenderer]] — renders a `TableView` to its native equivalent.
- [[NavigationRenderer|NavigationRenderer]] — renders a `NavigationPage` and its navigation stack.
- [[TabbedRenderer|TabbedRenderer]] — renders a `TabbedPage` with native tabs.
- [[ShellRenderer|ShellRenderer]] — renders `Shell` navigation through the compatibility path.
- [[FrameRenderer|FrameRenderer]] — renders a `Frame` control.
- [[ICellRenderer|ICellRenderer]] — contract implemented by cell renderers that produce native cell content.
- [[TextCellRenderer|TextCellRenderer]] — renders a `TextCell` into native list/table content.
- [[ViewCellRenderer|ViewCellRenderer]] — renders a `ViewCell` hosting an arbitrary view.
- [[CellFactory|CellFactory]] — produces the appropriate native cell for a given cell type.


## Classes

| Type | Summary |
|---|---|
| [[BaseCellView\|BaseCellView]] |  |
| [[CellAdapter\|CellAdapter]] |  |
| [[CellContentFactory\|CellContentFactory]] |  |
| [[CellFactory\|CellFactory]] |  |
| [[CellRenderer\|CellRenderer]] |  |
| [[CellTableViewCell\|CellTableViewCell]] |  |
| [[CellWrapperTemplate\|CellWrapperTemplate]] |  |
| [[CellWrapperTemplateSelector\|CellWrapperTemplateSelector]] |  |
| [[EntryCellEditText\|EntryCellEditText]] |  |
| [[EntryCellRenderer\|EntryCellRenderer]] |  |
| [[EntryCellRendererCompleted\|EntryCellRendererCompleted]] |  |
| [[EntryCellTableViewCell\|EntryCellTableViewCell]] |  |
| [[EntryCellView\|EntryCellView]] |  |
| [[FormsRefreshControl\|FormsRefreshControl]] |  |
| [[FrameRenderer\|FrameRenderer]] |  |
| [[ImageCellRenderer\|ImageCellRenderer]] |  |
| [[ListViewAdaptor\|ListViewAdaptor]] |  |
| [[ListViewRenderer\|ListViewRenderer]] |  |
| [[NavigationRenderer\|NavigationRenderer]] |  |
| [[PhoneFlyoutPageRenderer\|PhoneFlyoutPageRenderer]] |  |
| [[SectionCell\|SectionCell]] |  |
| [[ShellRenderer\|ShellRenderer]] |  |
| [[SwitchCellRenderer\|SwitchCellRenderer]] |  |
| [[SwitchCellView\|SwitchCellView]] |  |
| [[TabbedRenderer\|TabbedRenderer]] |  |
| [[TableViewAdaptor\|TableViewAdaptor]] |  |
| [[TableViewModelRenderer\|TableViewModelRenderer]] |  |
| [[TableViewRenderer\|TableViewRenderer]] |  |
| [[TextCellRenderer\|TextCellRenderer]] |  |
| [[UnEvenTableViewModelRenderer\|UnEvenTableViewModelRenderer]] |  |
| [[ViewCellRenderer\|ViewCellRenderer]] |  |
| [[ViewRenderer\|ViewRenderer]] |  |
| [[ViewRenderer{TElement, TNativeView}\|ViewRenderer<TElement, TNativeView>]] |  |
| [[ViewRenderer{TElement, TPlatformView}\|ViewRenderer<TElement, TPlatformView>]] |  |
| [[VisualElementRenderer{TElement, TPlatformElement}\|VisualElementRenderer<TElement, TPlatformElement>]] |  |
| [[VisualElementRenderer{TElement}\|VisualElementRenderer<TElement>]] |  |

## Interfaces

| Type | Summary |
|---|---|
| [[ICellRenderer\|ICellRenderer]] |  |

## See also

- [[_API Reference]]
