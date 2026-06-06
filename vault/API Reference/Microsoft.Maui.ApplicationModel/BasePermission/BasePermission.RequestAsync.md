---
title: "BasePermission.RequestAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.Permissions.BasePermission.RequestAsync"
declaring_type: "BasePermission"
member_kind: method
---

# BasePermission.RequestAsync

> [!abstract] Method of [[BasePermission|BasePermission]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Requests this permission from the user for this application.

## Signature

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.ApplicationModel.PermissionStatus> abstract RequestAsync()
```

## Remarks

Will throw `PermissionException` if a required entry was not found in the application manifest. Not all permissions require a manifest entry.

## Returns

A `PermissionStatus` value indicating the result of this permission request.

## See also

- Declaring type: [[BasePermission|BasePermission]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
