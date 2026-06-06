---
title: "VibrationExtensions.Vibrate"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Devices
aliases:
  - "Microsoft.Maui.Devices.VibrationExtensions.Vibrate"
declaring_type: "VibrationExtensions"
member_kind: method
---

# VibrationExtensions.Vibrate

> [!abstract] Method of [[VibrationExtensions|VibrationExtensions]]
> Namespace: `Microsoft.Maui.Devices`

Vibrates the device for the specified time in the range [0, 5000]ms.

## Signature

```csharp
void static Vibrate(this Microsoft.Maui.Devices.IVibration! vibration, double duration)
```

## Parameters

| Parameter | Description |
|---|---|
| `vibration` | The object this method is invoked on. |
| `duration` | The time to vibrate for. This value will be ignored on iOS as it only supports a vibration of 500ms. |

## See also

- Declaring type: [[VibrationExtensions|VibrationExtensions]]
- [[_Microsoft.Maui.Devices|Microsoft.Maui.Devices namespace]]
