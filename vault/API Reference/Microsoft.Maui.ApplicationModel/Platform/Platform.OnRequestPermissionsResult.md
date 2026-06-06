---
title: "Platform.OnRequestPermissionsResult"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.Platform.OnRequestPermissionsResult"
declaring_type: "Platform"
member_kind: method
---

# Platform.OnRequestPermissionsResult

> [!abstract] Method of [[Platform|Platform]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Pass permission request results from an activity's overridden method to the library for handling internal permission requests.

## Signature

```csharp
void static OnRequestPermissionsResult(int requestCode, string![]! permissions, Android.Content.PM.Permission[]! grantResults)
```

## Parameters

| Parameter | Description |
|---|---|
| `requestCode` | The requestCode from the corresponding overridden method in an activity. |
| `permissions` | The permissions from the corresponding overridden method in an activity. |
| `grantResults` | The grantResults from the corresponding overridden method in an activity. |

## See also

- Declaring type: [[Platform|Platform]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
