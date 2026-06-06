---
title: "IPhoneDialer.Open"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel-Communication
aliases:
  - "Microsoft.Maui.ApplicationModel.Communication.IPhoneDialer.Open"
declaring_type: "IPhoneDialer"
member_kind: method
---

# IPhoneDialer.Open

> [!abstract] Method of [[IPhoneDialer|IPhoneDialer]]
> Namespace: `Microsoft.Maui.ApplicationModel.Communication`

Open the phone dialer to a specific phone number.

## Signature

```csharp
void Open(string! number)
```

## Remarks

Will throw `ArgumentNullException` if `number` is not valid. Will throw `FeatureNotSupportedException` if making phone calls is not supported on the device.

## Parameters

| Parameter | Description |
|---|---|
| `number` | Phone number to initialize the dialer with. |

## See also

- Declaring type: [[IPhoneDialer|IPhoneDialer]]
- [[_Microsoft.Maui.ApplicationModel.Communication|Microsoft.Maui.ApplicationModel.Communication namespace]]
