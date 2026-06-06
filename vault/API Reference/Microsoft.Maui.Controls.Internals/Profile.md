---
title: "Profile"
tags:
  - api
  - kind/struct
  - ns/Microsoft-Maui-Controls-Internals
aliases:
  - "Microsoft.Maui.Controls.Internals.Profile"
namespace: "Microsoft.Maui.Controls.Internals"
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

# Profile

> [!abstract] Struct in `Microsoft.Maui.Controls.Internals`
> Full name: `Microsoft.Maui.Controls.Internals.Profile`

A disposable struct for profiling code execution.

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
| [[Profile.Profile\|Profile]] |  |

## Properties

| Name | Summary |
|---|---|
| [[Profile.IsEnabled\|IsEnabled]] | Gets whether profiling is enabled. |

## Methods

| Name | Summary |
|---|---|
| [[Profile.Dispose\|Dispose]] | Disposes the profile and records the elapsed time. |
| [[Profile.Enable\|Enable]] | Enables profiling and initializes data structures. |
| [[Profile.FrameBegin\|FrameBegin]] | Begins a new profiling frame. |
| [[Profile.FrameEnd\|FrameEnd]] | Ends the current profiling frame. |
| [[Profile.FramePartition\|FramePartition]] | Creates a partition within the current frame. |
| [[Profile.Start\|Start]] | Starts profiling. |
| [[Profile.Stop\|Stop]] | Stops profiling and unwinds the stack. |

## Fields

| Name | Summary |
|---|---|
| [[Profile.Data\|Data]] |  |

## See also

- [[_Microsoft.Maui.Controls.Internals|Microsoft.Maui.Controls.Internals namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.internals.profile)
