---
title: "Launcher.TryOpenAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.Launcher.TryOpenAsync"
declaring_type: "Launcher"
member_kind: method
---

# Launcher.TryOpenAsync

> [!abstract] Method of [[Launcher|Launcher]]
> Namespace: `Microsoft.Maui.ApplicationModel`

First checks if the provided URI is supported, then opens the app specified by the URI.

## Signatures

```csharp
System.Threading.Tasks.Task<bool>! static TryOpenAsync(string! uri)
System.Threading.Tasks.Task<bool>! static TryOpenAsync(System.Uri! uri)
```

## Returns

`true` if the URI was opened, otherwise `false`.

## Parameters

| Parameter | Description |
|---|---|
| `uri` | URI to try and open. |

## See also

- Declaring type: [[Launcher|Launcher]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
