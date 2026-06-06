---
title: "PhoneDialer.Open"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel-Communication
aliases:
  - "Microsoft.Maui.ApplicationModel.Communication.PhoneDialer.Open"
declaring_type: "PhoneDialer"
member_kind: method
---

# PhoneDialer.Open

> [!abstract] Method of [[PhoneDialer|PhoneDialer]]
> Namespace: `Microsoft.Maui.ApplicationModel.Communication`

Open the phone dialer to a specific phone number.

## Signature

```csharp
void static Open(string! number)
```

## Parameters

| Parameter | Description |
|---|---|
| `number` | Phone number to initialize the dialer with. |

## Remarks

Will throw `ArgumentNullException` if `number` is not valid. Will throw `FeatureNotSupportedException` if making phone calls is not supported on the device.

## See also

- Declaring type: [[PhoneDialer|PhoneDialer]]
- [[_Microsoft.Maui.ApplicationModel.Communication|Microsoft.Maui.ApplicationModel.Communication namespace]]
