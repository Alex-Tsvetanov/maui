---
title: "Windows platform-specifics in .NET MAUI"
description: "Learn how to consume Windows platform-specifics in .NET MAUI apps."
tags:
  - conceptual
  - area/windows
ms_date: "04/06/2022"
source: "https://learn.microsoft.com/dotnet/maui/windows/platform-specifics?view=net-maui-10.0"
---

# Windows platform-specifics

.NET Multi-platform App UI (.NET MAUI) platform-specifics allow you to consume functionality that's only available on a specific platform, without customizing handlers.

The following platform-specific functionality is provided for .NET MAUI views, pages, and layouts on Windows:

- Setting an access key for a [[VisualElement (Controls)|VisualElement]]. For more information, see [[visualelement-access-keys|VisualElement Access Keys on Windows]].

The following platform-specific functionality is provided for .NET MAUI views on Windows:

- Detecting reading order from text content in [[Entry (Controls)|Entry]], [[Editor|Editor]], and [[Label (Controls)|Label]] instances. For more information, see [[inputview-reading-order|InputView Reading Order on Windows]].
- Enabling tap gesture support in a [[ListView (Controls)|ListView]]. For more information, see [[listview-selectionmode|ListView SelectionMode on Windows]].
- Enabling the pull direction of a [[RefreshView (Controls)|RefreshView]] to be changed. For more information, see [[refreshview-pulldirection|RefreshView Pull Direction on Windows]].
- Enabling a [[SearchBar (Controls)|SearchBar]] to interact with the spell check engine. For more information, see [[searchbar-spell-check|SearchBar Spell Check on Windows]].

The following platform-specific functionality is provided for the .NET MAUI `Application` class on Windows:

- Specifying the directory in the project that image assets will be loaded from. For more information, see [[default-image-directory|Default Image Directory on Windows]].
