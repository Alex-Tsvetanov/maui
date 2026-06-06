---
title: "ILauncher.TryOpenAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.ILauncher.TryOpenAsync"
declaring_type: "ILauncher"
member_kind: method
---

# ILauncher.TryOpenAsync

> [!abstract] Method of [[ILauncher|ILauncher]]
> Namespace: `Microsoft.Maui.ApplicationModel`

First checks if the provided URI is supported, then opens the app specified by the URI.

## Signature

```csharp
System.Threading.Tasks.Task<bool>! TryOpenAsync(System.Uri! uri)
```

## Returns

`true` if the URI was opened, otherwise `false`.

## Parameters

| Parameter | Description |
|---|---|
| `uri` | URI to try and open. |

## See also

- Declaring type: [[ILauncher|ILauncher]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
