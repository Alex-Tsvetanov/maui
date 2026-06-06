---
title: "BasePermission.CheckStatusAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.Permissions.BasePermission.CheckStatusAsync"
declaring_type: "BasePermission"
member_kind: method
---

# BasePermission.CheckStatusAsync

> [!abstract] Method of [[BasePermission|BasePermission]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Retrieves the current status of this permission.

## Signature

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.ApplicationModel.PermissionStatus> abstract CheckStatusAsync()
```

## Remarks

Will throw `PermissionException` if a required entry was not found in the application manifest. Not all permissions require a manifest entry.

## Returns

A `PermissionStatus` value indicating the current status of this permission.

## See also

- Declaring type: [[BasePermission|BasePermission]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
