---
title: "Sensors.CheckStatusAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.Permissions.Sensors.CheckStatusAsync"
declaring_type: "Sensors"
member_kind: method
---

# Sensors.CheckStatusAsync

> [!abstract] Method of [[Sensors|Sensors]]
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

- Declaring type: [[Sensors|Sensors]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
