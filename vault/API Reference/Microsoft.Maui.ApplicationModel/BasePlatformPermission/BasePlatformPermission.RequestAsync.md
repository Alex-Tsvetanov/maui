---
title: "BasePlatformPermission.RequestAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.Permissions.BasePlatformPermission.RequestAsync"
declaring_type: "BasePlatformPermission"
member_kind: method
---

# BasePlatformPermission.RequestAsync

> [!abstract] Method of [[BasePlatformPermission|BasePlatformPermission]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Requests the permission from the user for this application.

## Signature

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.ApplicationModel.PermissionStatus> override RequestAsync()
```

## Remarks

Will throw `PermissionException` if a required entry was not found in the application manifest. Not all permissions require a manifest entry.

## Returns

A `PermissionStatus` value indicating the result of this permission request.

## See also

- Declaring type: [[BasePlatformPermission|BasePlatformPermission]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
