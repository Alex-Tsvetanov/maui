---
title: "ILauncher.OpenAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.ILauncher.OpenAsync"
declaring_type: "ILauncher"
member_kind: method
---

# ILauncher.OpenAsync

> [!abstract] Method of [[ILauncher|ILauncher]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Opens the app specified by the URI scheme.

## Signatures

```csharp
System.Threading.Tasks.Task<bool>! OpenAsync(Microsoft.Maui.ApplicationModel.OpenFileRequest! request)
System.Threading.Tasks.Task<bool>! OpenAsync(System.Uri! uri)
```

## Returns

`true` if the URI was opened, otherwise `false`.

## Parameters

| Parameter | Description |
|---|---|
| `uri` | URI to open. |

## See also

- Declaring type: [[ILauncher|ILauncher]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
