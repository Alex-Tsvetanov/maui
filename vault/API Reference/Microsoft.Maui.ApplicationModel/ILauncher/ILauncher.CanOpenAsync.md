---
title: "ILauncher.CanOpenAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.ILauncher.CanOpenAsync"
declaring_type: "ILauncher"
member_kind: method
---

# ILauncher.CanOpenAsync

> [!abstract] Method of [[ILauncher|ILauncher]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Queries if the device supports opening the given URI scheme.

## Signature

```csharp
System.Threading.Tasks.Task<bool>! CanOpenAsync(System.Uri! uri)
```

## Returns

`true` if opening is supported, otherwise `false`.

## Parameters

| Parameter | Description |
|---|---|
| `uri` | URI scheme to query. |

## See also

- Declaring type: [[ILauncher|ILauncher]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
