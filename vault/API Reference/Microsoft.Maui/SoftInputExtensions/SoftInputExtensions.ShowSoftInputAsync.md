---
title: "SoftInputExtensions.ShowSoftInputAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.SoftInputExtensions.ShowSoftInputAsync"
declaring_type: "SoftInputExtensions"
member_kind: method
---

# SoftInputExtensions.ShowSoftInputAsync

> [!abstract] Method of [[SoftInputExtensions|SoftInputExtensions]]
> Namespace: `Microsoft.Maui`

If a soft input pane is currently hiding, this will attempt to show it.

## Signature

```csharp
System.Threading.Tasks.Task<bool>! static ShowSoftInputAsync(this Microsoft.Maui.ITextInput! targetView, System.Threading.CancellationToken token)
```

## Returns

Returns true if the platform was able to show the soft input pane.

## Parameters

| Parameter | Description |
|---|---|
| `targetView` |  |
| `token` | Cancellation token |

## See also

- Declaring type: [[SoftInputExtensions|SoftInputExtensions]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
