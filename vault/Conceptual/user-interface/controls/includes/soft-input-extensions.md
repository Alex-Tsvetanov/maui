---
title: "Soft Input Extensions"
tags:
  - conceptual
  - area/user-interface
ms_date: "10/19/2023"
source: "https://learn.microsoft.com/dotnet/maui/user-interface/controls/includes/soft-input-extensions?view=net-maui-10.0"
---

## Hide and show the soft input keyboard

The `SoftInputExtensions` class, in the `Microsoft.Maui` namespace, provides a series of extension methods that support interacting with the soft input keyboard on controls that support text input. The class defines the following methods:

- `IsSoftInputShowing`, which checks to see if the device is currently showing the soft input keyboard.
- `HideSoftInputAsync`, which will attempt to hide the soft input keyboard if it's currently showing.
- `ShowSoftInputAsync`, which will attempt to show the soft input keyboard if it's currently hidden.
