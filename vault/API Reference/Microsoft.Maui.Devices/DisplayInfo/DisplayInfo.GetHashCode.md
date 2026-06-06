---
title: "DisplayInfo.GetHashCode"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Devices
aliases:
  - "Microsoft.Maui.Devices.DisplayInfo.GetHashCode"
declaring_type: "DisplayInfo"
member_kind: method
---

# DisplayInfo.GetHashCode

> [!abstract] Method of [[DisplayInfo|DisplayInfo]]
> Namespace: `Microsoft.Maui.Devices`

Gets the hash code for this display info instance.

## Signature

```csharp
int override GetHashCode()
```

## Returns

The computed hash code for this device idiom or 0 when the device platform is `null`.

## Remarks

The hash code is computed from `Height`, `Width`, `Density`, `Orientation` and `Rotation`.

## See also

- Declaring type: [[DisplayInfo|DisplayInfo]]
- [[_Microsoft.Maui.Devices|Microsoft.Maui.Devices namespace]]
