---
title: "IVersionTracking.IsFirstLaunchForBuild"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.IVersionTracking.IsFirstLaunchForBuild"
declaring_type: "IVersionTracking"
member_kind: method
---

# IVersionTracking.IsFirstLaunchForBuild

> [!abstract] Method of [[IVersionTracking|IVersionTracking]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Determines if this is the first launch of the app for a specified build number.

## Signature

```csharp
bool IsFirstLaunchForBuild(string! build)
```

## Returns

`true` if this is the first launch of the app for the specified build number; otherwise `false`.

## Parameters

| Parameter | Description |
|---|---|
| `build` | The build number. |

## See also

- Declaring type: [[IVersionTracking|IVersionTracking]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
