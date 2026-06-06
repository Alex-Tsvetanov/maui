---
title: "PlatformWebViewInitializingEventArgs.ProfileName"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.PlatformWebViewInitializingEventArgs.ProfileName"
declaring_type: "PlatformWebViewInitializingEventArgs"
member_kind: property
---

# PlatformWebViewInitializingEventArgs.ProfileName

> [!abstract] Property of [[PlatformWebViewInitializingEventArgs|PlatformWebViewInitializingEventArgs]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the name of the controller profile.

## Signature

```csharp
string? ProfileName { get; set; }
```

## Remarks

Profile names are only allowed to contain the following ASCII characters: * alphabet characters: a-z and A-Z * digit characters: 0-9 * and '#', '@', '$', '(', ')', '+', '-', '_', '~', '.', ' ' (space). It has a maximum length of 64 characters excluding the null-terminator. It is ASCII case insensitive.

## See also

- Declaring type: [[PlatformWebViewInitializingEventArgs|PlatformWebViewInitializingEventArgs]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
