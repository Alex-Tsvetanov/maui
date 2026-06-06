---
title: "DispatcherProvider.SetCurrent"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Dispatching
aliases:
  - "Microsoft.Maui.Dispatching.DispatcherProvider.SetCurrent"
declaring_type: "DispatcherProvider"
member_kind: method
---

# DispatcherProvider.SetCurrent

> [!abstract] Method of [[DispatcherProvider|DispatcherProvider]]
> Namespace: `Microsoft.Maui.Dispatching`

Sets the current dispatcher provider.

## Signature

```csharp
bool static SetCurrent(Microsoft.Maui.Dispatching.IDispatcherProvider? provider)
```

## Parameters

| Parameter | Description |
|---|---|
| `provider` | The `IDispatcherProvider` object to set as the current dispatcher provider. |

## Returns

`true` if the current dispatcher was actually updated, otherwise `false`.

## See also

- Declaring type: [[DispatcherProvider|DispatcherProvider]]
- [[_Microsoft.Maui.Dispatching|Microsoft.Maui.Dispatching namespace]]
