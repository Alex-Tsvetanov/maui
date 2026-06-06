---
title: "SoftInputExtensions.HideSoftInputAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.SoftInputExtensions.HideSoftInputAsync"
declaring_type: "SoftInputExtensions"
member_kind: method
---

# SoftInputExtensions.HideSoftInputAsync

> [!abstract] Method of [[SoftInputExtensions|SoftInputExtensions]]
> Namespace: `Microsoft.Maui`

If a soft input pane is currently showing, this will attempt to hide it.

## Signature

```csharp
System.Threading.Tasks.Task<bool>! static HideSoftInputAsync(this Microsoft.Maui.ITextInput! targetView, System.Threading.CancellationToken token)
```

## Returns

Returns true if the platform was able to hide the soft input pane.

## Parameters

| Parameter | Description |
|---|---|
| `targetView` |  |
| `token` | Cancellation token |

## See also

- Declaring type: [[SoftInputExtensions|SoftInputExtensions]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
