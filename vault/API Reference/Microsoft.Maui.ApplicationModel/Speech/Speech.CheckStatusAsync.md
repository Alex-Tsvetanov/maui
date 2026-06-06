---
title: "Speech.CheckStatusAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.Permissions.Speech.CheckStatusAsync"
declaring_type: "Speech"
member_kind: method
---

# Speech.CheckStatusAsync

> [!abstract] Method of [[Speech|Speech]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Retrieves the current status of the permission.

## Signature

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.ApplicationModel.PermissionStatus> override CheckStatusAsync()
```

## Remarks

Will throw `PermissionException` if a required entry was not found in the application manifest. Not all permissions require a manifest entry.

## Returns

A `PermissionStatus` value indicating the current status of the permission.

## See also

- Declaring type: [[Speech|Speech]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
