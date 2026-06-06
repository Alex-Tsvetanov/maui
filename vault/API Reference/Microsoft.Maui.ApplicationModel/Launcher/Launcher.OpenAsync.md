---
title: "Launcher.OpenAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.Launcher.OpenAsync"
declaring_type: "Launcher"
member_kind: method
---

# Launcher.OpenAsync

> [!abstract] Method of [[Launcher|Launcher]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Opens the app specified by the URI scheme.

## Signatures

```csharp
System.Threading.Tasks.Task<bool>! static OpenAsync(Microsoft.Maui.ApplicationModel.OpenFileRequest! request)
System.Threading.Tasks.Task<bool>! static OpenAsync(string! uri)
System.Threading.Tasks.Task<bool>! static OpenAsync(System.Uri! uri)
```

## Returns

`true` if the URI was opened, otherwise `false`.

## Parameters

| Parameter | Description |
|---|---|
| `uri` | URI to open. |

## See also

- Declaring type: [[Launcher|Launcher]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
