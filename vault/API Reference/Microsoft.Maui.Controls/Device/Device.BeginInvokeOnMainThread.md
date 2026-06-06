---
title: "Device.BeginInvokeOnMainThread"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Device.BeginInvokeOnMainThread"
declaring_type: "Device"
member_kind: method
---

# Device.BeginInvokeOnMainThread

> [!abstract] Method of [[Device|Device]]
> Namespace: `Microsoft.Maui.Controls`

Invokes an Action on the device main (UI) thread.

## Signature

```csharp
void static BeginInvokeOnMainThread(System.Action action)
```

## Parameters

| Parameter | Description |
|---|---|
| `action` | The Action to invoke |

## Remarks

This example shows how to set the Text of Label on the main thread, e.g. in response to an async event.

## See also

- Declaring type: [[Device|Device]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
