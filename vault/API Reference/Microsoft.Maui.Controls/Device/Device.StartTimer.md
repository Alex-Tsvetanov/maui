---
title: "Device.StartTimer"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Device.StartTimer"
declaring_type: "Device"
member_kind: method
---

# Device.StartTimer

> [!abstract] Method of [[Device|Device]]
> Namespace: `Microsoft.Maui.Controls`

Starts a recurring timer using the device clock capabilities.

## Signature

```csharp
void static StartTimer(System.TimeSpan interval, System.Func<bool> callback)
```

## Parameters

| Parameter | Description |
|---|---|
| `interval` | The interval between invocations of the callback. |
| `callback` | The action to run when the timer elapses. |

## Remarks

While the callback returns If you want the code inside the timer to interact on the UI thread (e.g. setting text of a Label or showing an alert), it should be done within a

## See also

- Declaring type: [[Device|Device]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
