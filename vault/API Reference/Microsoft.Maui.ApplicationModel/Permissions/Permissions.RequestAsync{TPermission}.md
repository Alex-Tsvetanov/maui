---
title: "Permissions.RequestAsync<TPermission>"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.Permissions.RequestAsync<TPermission>"
declaring_type: "Permissions"
member_kind: method
---

# Permissions.RequestAsync<TPermission>

> [!abstract] Method of [[Permissions|Permissions]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Requests the permission from the user for this application.

## Signature

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.ApplicationModel.PermissionStatus> static RequestAsync<TPermission>()
```

## Remarks

Will throw `PermissionException` if a required entry was not found in the application manifest. Not all permissions require a manifest entry.

## Returns

A `PermissionStatus` value indicating the result of this permission request.

## See also

- Declaring type: [[Permissions|Permissions]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
