---
title: "Microsoft.Maui.Controls.Platform.Compatibility"
tags:
  - api
  - namespace
  - ns/Microsoft-Maui-Controls-Platform-Compatibility
---

# Microsoft.Maui.Controls.Platform.Compatibility

> [!info] Namespace
> `Microsoft.Maui.Controls.Platform.Compatibility` — 65 public types.

- [Online namespace docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.platform.compatibility)

## Overview

`Microsoft.Maui.Controls.Platform.Compatibility` houses the platform-specific plumbing that lets .NET MAUI's cross-platform controls run on top of the legacy Xamarin.Forms compatibility renderer pipeline. Rather than being part of the modern handler architecture, these types are the renderers, appearance trackers, view containers, and value converters that bridge MAUI's abstract controls to the underlying native UI primitives. Most members are infrastructure: you rarely instantiate them directly, but they are public so that custom renderers and platform-specific customizations can extend or replace the default behavior.

The bulk of the namespace is devoted to **Shell**. Renderers such as [[ShellFlyoutRenderer|ShellFlyoutRenderer]], [[ShellItemRenderer|ShellItemRenderer]], and [[ShellSectionRenderer|ShellSectionRenderer]] turn Shell's logical structure (items, sections, the flyout, and search) into concrete native views, while a family of appearance trackers — exposed through interfaces like [[IShellNavBarAppearanceTracker|IShellNavBarAppearanceTracker]], [[IShellTabBarAppearanceTracker|IShellTabBarAppearanceTracker]], and [[IShellToolbarAppearanceTracker|IShellToolbarAppearanceTracker]] — keep colors, bars, and chrome in sync with the active Shell appearance. The central [[IShellContext|IShellContext]] interface ties these renderers together and exposes the shared Shell rendering context.

Beyond Shell, the namespace supplies general compatibility helpers: cell and container views such as [[CellControl|CellControl]] and [[ContainerView (Compatibility)|ContainerView]], and a set of `IValueConverter`-style converters — including [[CaseConverter|CaseConverter]], [[HeightConverter|HeightConverter]], and [[KeyboardConverter|KeyboardConverter]] — used by templated compatibility renderers to adapt MAUI property values to native expectations.

> [!info] Infrastructure namespace
> These types support the Xamarin.Forms compatibility renderer pipeline. Prefer the modern handler-based APIs for new work; reach for these only when customizing or migrating legacy renderer behavior.

## Key types

- [[IShellContext|IShellContext]] — Shared Shell rendering context that ties the compatibility renderers together.
- [[ShellFlyoutRenderer|ShellFlyoutRenderer]] — Renders the Shell flyout into a native view.
- [[ShellItemRenderer|ShellItemRenderer]] — Renders a Shell item (and its tab structure) on the platform.
- [[ShellSectionRenderer|ShellSectionRenderer]] — Renders the content of a Shell section.
- [[ShellSearchResultsRenderer|ShellSearchResultsRenderer]] — Renders Shell search results for the search handler.
- [[IShellNavBarAppearanceTracker|IShellNavBarAppearanceTracker]] — Tracks and applies appearance to the Shell navigation bar.
- [[IShellTabBarAppearanceTracker|IShellTabBarAppearanceTracker]] — Tracks and applies appearance to the Shell tab bar.
- [[IShellToolbarAppearanceTracker|IShellToolbarAppearanceTracker]] — Tracks and applies appearance to the Shell toolbar.
- [[IShellFlyoutTransition|IShellFlyoutTransition]] — Defines the animated transition used when opening/closing the flyout.
- [[CellControl|CellControl]] — Compatibility control that hosts a MAUI cell on the platform.
- [[CaseConverter|CaseConverter]] — Value converter that adjusts text casing for compatibility renderers.
- [[KeyboardConverter|KeyboardConverter]] — Value converter that maps MAUI keyboard types to native keyboard settings.

## Related guides

- [[_API Reference|API Reference]]


## Classes

| Type | Summary |
|---|---|
| [[AdapterListItem\|AdapterListItem]] |  |
| [[CaseConverter\|CaseConverter]] |  |
| [[CellControl\|CellControl]] |  |
| [[CollapseWhenEmptyConverter\|CollapseWhenEmptyConverter]] |  |
| [[ColorConverter (Compatibility)\|ColorConverter (Compatibility)]] |  |
| [[ContainerView (Compatibility)\|ContainerView (Compatibility)]] |  |
| [[CustomFrameLayout\|CustomFrameLayout]] |  |
| [[ElementViewHolder\|ElementViewHolder]] |  |
| [[EntryCellTextBox\|EntryCellTextBox]] |  |
| [[HeaderContainer\|HeaderContainer]] |  |
| [[HeightConverter\|HeightConverter]] |  |
| [[HorizontalTextAlignmentConverter\|HorizontalTextAlignmentConverter]] |  |
| [[KeyboardConverter\|KeyboardConverter]] |  |
| [[ListGroupHeaderPresenter\|ListGroupHeaderPresenter]] |  |
| [[ListViewGroupStyleSelector\|ListViewGroupStyleSelector]] |  |
| [[SafeShellNavBarAppearanceTracker\|SafeShellNavBarAppearanceTracker]] |  |
| [[SafeShellTabBarAppearanceTracker\|SafeShellTabBarAppearanceTracker]] |  |
| [[SearchHandlerAppearanceTracker\|SearchHandlerAppearanceTracker]] |  |
| [[ShellBottomNavViewAppearanceTracker\|ShellBottomNavViewAppearanceTracker]] |  |
| [[ShellContentFragment\|ShellContentFragment]] |  |
| [[ShellFlyoutContentRenderer\|ShellFlyoutContentRenderer]] |  |
| [[ShellFlyoutRecyclerAdapter\|ShellFlyoutRecyclerAdapter]] |  |
| [[ShellFlyoutRenderer\|ShellFlyoutRenderer]] |  |
| [[ShellFlyoutTemplatedContentRenderer\|ShellFlyoutTemplatedContentRenderer]] |  |
| [[ShellItemRenderer\|ShellItemRenderer]] |  |
| [[ShellItemRendererBase\|ShellItemRendererBase]] |  |
| [[ShellItemTransition\|ShellItemTransition]] |  |
| [[ShellNavBarAppearanceTracker\|ShellNavBarAppearanceTracker]] |  |
| [[ShellPageRendererTracker\|ShellPageRendererTracker]] |  |
| [[ShellSearchResultsRenderer\|ShellSearchResultsRenderer]] |  |
| [[ShellSearchView (Compatibility)\|ShellSearchView (Compatibility)]] |  |
| [[ShellSearchViewAdapter\|ShellSearchViewAdapter]] |  |
| [[ShellSectionHeaderCell\|ShellSectionHeaderCell]] |  |
| [[ShellSectionRenderer\|ShellSectionRenderer]] |  |
| [[ShellSectionRootHeader\|ShellSectionRootHeader]] |  |
| [[ShellSectionRootRenderer\|ShellSectionRootRenderer]] |  |
| [[ShellTabBarAppearanceTracker\|ShellTabBarAppearanceTracker]] |  |
| [[ShellTabLayoutAppearanceTracker\|ShellTabLayoutAppearanceTracker]] |  |
| [[ShellTableViewController\|ShellTableViewController]] |  |
| [[ShellTableViewSource\|ShellTableViewSource]] |  |
| [[ShellToolbarAppearanceTracker\|ShellToolbarAppearanceTracker]] |  |
| [[ShellToolbarTracker\|ShellToolbarTracker]] |  |
| [[SlideFlyoutTransition\|SlideFlyoutTransition]] |  |
| [[TitleViewContainer\|TitleViewContainer]] |  |
| [[UIContainerCell\|UIContainerCell]] |  |
| [[UIContainerView\|UIContainerView]] |  |

## Interfaces

| Type | Summary |
|---|---|
| [[IShellBottomNavViewAppearanceTracker\|IShellBottomNavViewAppearanceTracker]] |  |
| [[IShellContext\|IShellContext]] |  |
| [[IShellFlyoutContentRenderer\|IShellFlyoutContentRenderer]] |  |
| [[IShellFlyoutRenderer\|IShellFlyoutRenderer]] |  |
| [[IShellFlyoutTransition\|IShellFlyoutTransition]] |  |
| [[IShellItemRenderer\|IShellItemRenderer]] |  |
| [[IShellItemTransition\|IShellItemTransition]] |  |
| [[IShellNavBarAppearanceTracker\|IShellNavBarAppearanceTracker]] |  |
| [[IShellObservableFragment\|IShellObservableFragment]] |  |
| [[IShellPageRendererTracker\|IShellPageRendererTracker]] |  |
| [[IShellSearchResultsRenderer\|IShellSearchResultsRenderer]] |  |
| [[IShellSearchView\|IShellSearchView]] |  |
| [[IShellSectionRenderer\|IShellSectionRenderer]] |  |
| [[IShellSectionRootHeader\|IShellSectionRootHeader]] |  |
| [[IShellSectionRootRenderer\|IShellSectionRootRenderer]] |  |
| [[IShellTabBarAppearanceTracker\|IShellTabBarAppearanceTracker]] |  |
| [[IShellTabLayoutAppearanceTracker\|IShellTabLayoutAppearanceTracker]] |  |
| [[IShellToolbarAppearanceTracker\|IShellToolbarAppearanceTracker]] |  |
| [[IShellToolbarTracker\|IShellToolbarTracker]] |  |

## See also

- [[_API Reference]]
