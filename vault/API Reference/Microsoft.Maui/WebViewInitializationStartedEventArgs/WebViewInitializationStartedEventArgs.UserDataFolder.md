---
title: "WebViewInitializationStartedEventArgs.UserDataFolder"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.WebViewInitializationStartedEventArgs.UserDataFolder"
declaring_type: "WebViewInitializationStartedEventArgs"
member_kind: property
---

# WebViewInitializationStartedEventArgs.UserDataFolder

> [!abstract] Property of [[WebViewInitializationStartedEventArgs|WebViewInitializationStartedEventArgs]]
> Namespace: `Microsoft.Maui`

Gets or sets the user data folder location for WebView2.

## Signature

```csharp
string? UserDataFolder { get; set; }
```

## Remarks

The default user data folder {Executable File Name}.WebView2 is created in the same directory next to the compiled code for the app. WebView2 creation fails if the compiled code is running in a directory in which the process does not have permission to create a new directory. The app is responsible to clean up the associated user data folder when it is done.

## See also

- Declaring type: [[WebViewInitializationStartedEventArgs|WebViewInitializationStartedEventArgs]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
