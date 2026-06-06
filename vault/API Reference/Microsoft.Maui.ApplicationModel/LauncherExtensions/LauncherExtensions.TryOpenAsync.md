---
title: "LauncherExtensions.TryOpenAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.LauncherExtensions.TryOpenAsync"
declaring_type: "LauncherExtensions"
member_kind: method
---

# LauncherExtensions.TryOpenAsync

> [!abstract] Method of [[LauncherExtensions|LauncherExtensions]]
> Namespace: `Microsoft.Maui.ApplicationModel`

First checks if the provided URI is supported, then opens the app specified by the URI.

## Signature

```csharp
System.Threading.Tasks.Task<bool>! static TryOpenAsync(this Microsoft.Maui.ApplicationModel.ILauncher! launcher, string! uri)
```

## Returns

`true` if the URI was opened, otherwise `false`.

## Parameters

| Parameter | Description |
|---|---|
| `launcher` | The object this method is invoked on. |
| `uri` | URI to try and open. |

## See also

- Declaring type: [[LauncherExtensions|LauncherExtensions]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
