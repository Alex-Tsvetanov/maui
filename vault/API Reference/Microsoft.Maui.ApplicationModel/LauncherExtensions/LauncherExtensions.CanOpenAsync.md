---
title: "LauncherExtensions.CanOpenAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.LauncherExtensions.CanOpenAsync"
declaring_type: "LauncherExtensions"
member_kind: method
---

# LauncherExtensions.CanOpenAsync

> [!abstract] Method of [[LauncherExtensions|LauncherExtensions]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Queries if the device supports opening the given URI scheme.

## Signature

```csharp
System.Threading.Tasks.Task<bool>! static CanOpenAsync(this Microsoft.Maui.ApplicationModel.ILauncher! launcher, string! uri)
```

## Returns

`true` if opening is supported, otherwise `false`.

## Parameters

| Parameter | Description |
|---|---|
| `launcher` | The object this method is invoked on. |
| `uri` | URI scheme to query. |

## See also

- Declaring type: [[LauncherExtensions|LauncherExtensions]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
