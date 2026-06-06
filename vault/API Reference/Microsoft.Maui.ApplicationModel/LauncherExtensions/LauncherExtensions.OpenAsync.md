---
title: "LauncherExtensions.OpenAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.LauncherExtensions.OpenAsync"
declaring_type: "LauncherExtensions"
member_kind: method
---

# LauncherExtensions.OpenAsync

> [!abstract] Method of [[LauncherExtensions|LauncherExtensions]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Opens the app specified by the URI scheme.

## Signature

```csharp
System.Threading.Tasks.Task<bool>! static OpenAsync(this Microsoft.Maui.ApplicationModel.ILauncher! launcher, string! uri)
```

## Returns

`true` if the URI was opened, otherwise `false`.

## Parameters

| Parameter | Description |
|---|---|
| `launcher` | The object this method is invoked on. |
| `uri` | URI to open. |

## See also

- Declaring type: [[LauncherExtensions|LauncherExtensions]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
