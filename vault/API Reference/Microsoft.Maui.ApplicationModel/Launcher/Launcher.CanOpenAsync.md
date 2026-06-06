---
title: "Launcher.CanOpenAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.Launcher.CanOpenAsync"
declaring_type: "Launcher"
member_kind: method
---

# Launcher.CanOpenAsync

> [!abstract] Method of [[Launcher|Launcher]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Queries if the device supports opening the given URI scheme.

## Signatures

```csharp
System.Threading.Tasks.Task<bool>! static CanOpenAsync(string! uri)
System.Threading.Tasks.Task<bool>! static CanOpenAsync(System.Uri! uri)
```

## Returns

`true` if opening is supported, otherwise `false`.

## Parameters

| Parameter | Description |
|---|---|
| `uri` | URI scheme to query. |

## See also

- Declaring type: [[Launcher|Launcher]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
