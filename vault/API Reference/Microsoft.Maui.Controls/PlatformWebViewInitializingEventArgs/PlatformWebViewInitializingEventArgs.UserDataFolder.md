---
title: "PlatformWebViewInitializingEventArgs.UserDataFolder"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.PlatformWebViewInitializingEventArgs.UserDataFolder"
declaring_type: "PlatformWebViewInitializingEventArgs"
member_kind: property
---

# PlatformWebViewInitializingEventArgs.UserDataFolder

> [!abstract] Property of [[PlatformWebViewInitializingEventArgs|PlatformWebViewInitializingEventArgs]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the user data folder location for WebView2.

## Signature

```csharp
string? UserDataFolder { get; set; }
```

## Remarks

The default user data folder {Executable File Name}.WebView2 is created in the same directory next to the compiled code for the app. WebView2 creation fails if the compiled code is running in a directory in which the process does not have permission to create a new directory. The app is responsible to clean up the associated user data folder when it is done.

## See also

- Declaring type: [[PlatformWebViewInitializingEventArgs|PlatformWebViewInitializingEventArgs]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
