---
title: "Photos.RequestAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.Permissions.Photos.RequestAsync"
declaring_type: "Photos"
member_kind: method
---

# Photos.RequestAsync

> [!abstract] Method of [[Photos|Photos]]
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

- Declaring type: [[Photos|Photos]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
