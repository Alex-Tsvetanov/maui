---
title: "Permissions.IsCapabilityDeclared"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.Permissions.IsCapabilityDeclared"
declaring_type: "Permissions"
member_kind: method
---

# Permissions.IsCapabilityDeclared

> [!abstract] Method of [[Permissions|Permissions]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Checks if the capability specified in `capabilityName` is declared in the application's AppxManifest.xml file.

## Signature

```csharp
bool static IsCapabilityDeclared(string capabilityName)
```

## Parameters

| Parameter | Description |
|---|---|
| `capabilityName` | The capability to check for specification in the AppxManifest.xml file. |

## Returns

`true` when the capability is specified, otherwise `false`.

## See also

- Declaring type: [[Permissions|Permissions]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
