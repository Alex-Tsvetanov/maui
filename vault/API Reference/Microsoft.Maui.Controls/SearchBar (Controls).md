---
title: "SearchBar (Controls)"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.SearchBar"
namespace: "Microsoft.Maui.Controls"
kind: class
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

# SearchBar (Controls)

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.SearchBar`

Represents a specialized input control for entering search text with a built-in search button and cancel button.

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
| [[SearchBar (Controls).SearchBar\|SearchBar]] | Initializes a new instance of the `SearchBar` class. |

## Properties

| Name | Summary |
|---|---|
| [[SearchBar (Controls).CancelButtonColor\|CancelButtonColor]] |  |
| [[SearchBar (Controls).HorizontalTextAlignment\|HorizontalTextAlignment]] |  |
| [[SearchBar (Controls).IsEnabledCore\|IsEnabledCore]] |  |
| [[SearchBar (Controls).ReturnType\|ReturnType]] |  |
| [[SearchBar (Controls).SearchCommand\|SearchCommand]] |  |
| [[SearchBar (Controls).SearchCommandParameter\|SearchCommandParameter]] |  |
| [[SearchBar (Controls).SearchIconColor\|SearchIconColor]] |  |
| [[SearchBar (Controls).VerticalTextAlignment\|VerticalTextAlignment]] |  |

## Methods

| Name | Summary |
|---|---|
| [[SearchBar (Controls).MapIsSpellCheckEnabled\|MapIsSpellCheckEnabled]] |  |
| [[SearchBar (Controls).MapSearchBarStyle\|MapSearchBarStyle]] |  |
| [[SearchBar (Controls).MapText\|MapText]] |  |
| [[SearchBar (Controls).On{T}\|On<T>]] |  |
| [[SearchBar (Controls).OnSearchButtonPressed\|OnSearchButtonPressed]] | For internal use by the .NET MAUI platform. Raises the `SearchButtonPressed` event and executes the `SearchCommand`. |

## Events

| Name | Summary |
|---|---|
| [[SearchBar (Controls).SearchButtonPressed\|SearchButtonPressed]] | Determines what the return key on the on-screen keyboard should look like. This is a bindable property. |

## Fields

| Name | Summary |
|---|---|
| [[SearchBar (Controls).CancelButtonColorProperty\|CancelButtonColorProperty]] | Bindable property for `CancelButtonColor`. This is a bindable property. |
| [[SearchBar (Controls).CharacterSpacingProperty\|CharacterSpacingProperty]] | Bindable property for the spacing between characters in the text. This is a bindable property. |
| [[SearchBar (Controls).CursorPositionProperty\|CursorPositionProperty]] |  |
| [[SearchBar (Controls).FontAttributesProperty\|FontAttributesProperty]] |  |
| [[SearchBar (Controls).FontAutoScalingEnabledProperty\|FontAutoScalingEnabledProperty]] |  |
| [[SearchBar (Controls).FontFamilyProperty\|FontFamilyProperty]] |  |
| [[SearchBar (Controls).FontSizeProperty\|FontSizeProperty]] |  |
| [[SearchBar (Controls).HorizontalTextAlignmentProperty\|HorizontalTextAlignmentProperty]] | Bindable property for `HorizontalTextAlignment`. This is a bindable property. |
| [[SearchBar (Controls).IsTextPredictionEnabledProperty\|IsTextPredictionEnabledProperty]] | Backing store for the `IsTextPredictionEnabled` property. |
| [[SearchBar (Controls).PlaceholderColorProperty\|PlaceholderColorProperty]] | Bindable property for the color of the placeholder text. This is a bindable property. |
| [[SearchBar (Controls).PlaceholderProperty\|PlaceholderProperty]] | Bindable property for the placeholder text displayed when the search bar is empty. This is a bindable property. |
| [[SearchBar (Controls).ReturnTypeProperty\|ReturnTypeProperty]] | Bindable property for `ReturnType`. This is a bindable property. |
| [[SearchBar (Controls).SearchCommandParameterProperty\|SearchCommandParameterProperty]] | Bindable property for `SearchCommandParameter`. This is a bindable property. |
| [[SearchBar (Controls).SearchCommandProperty\|SearchCommandProperty]] | Bindable property for `SearchCommand`. This is a bindable property. |
| [[SearchBar (Controls).SearchIconColorProperty\|SearchIconColorProperty]] | Bindable property for `SearchIconColor`. This is a bindable property. |
| [[SearchBar (Controls).SelectionLengthProperty\|SelectionLengthProperty]] |  |
| [[SearchBar (Controls).TextColorProperty\|TextColorProperty]] | Bindable property for the color of the search text. This is a bindable property. |
| [[SearchBar (Controls).TextProperty\|TextProperty]] | Bindable property for the text displayed in the search bar. This is a bindable property. |
| [[SearchBar (Controls).VerticalTextAlignmentProperty\|VerticalTextAlignmentProperty]] | Bindable property for `VerticalTextAlignment`. This is a bindable property. |

## Remarks

The `SearchBar` provides a user interface optimized for text searches, including a search icon, placeholder text, and optionally a cancel button. Use the `SearchCommand` to respond to search requests.

## Guide

- 📖 Conceptual: [[searchbar]]

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.searchbar)
