---
title: "WebViewInitializationStartedEventArgs.ProfileName"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.WebViewInitializationStartedEventArgs.ProfileName"
declaring_type: "WebViewInitializationStartedEventArgs"
member_kind: property
---

# WebViewInitializationStartedEventArgs.ProfileName

> [!abstract] Property of [[WebViewInitializationStartedEventArgs|WebViewInitializationStartedEventArgs]]
> Namespace: `Microsoft.Maui`

Gets or sets the name of the controller profile.

## Signature

```csharp
string? ProfileName { get; set; }
```

## Remarks

Profile names are only allowed to contain the following ASCII characters: * alphabet characters: a-z and A-Z * digit characters: 0-9 * and '#', '@', '$', '(', ')', '+', '-', '_', '~', '.', ' ' (space). It has a maximum length of 64 characters excluding the null-terminator. It is ASCII case insensitive.

## See also

- Declaring type: [[WebViewInitializationStartedEventArgs|WebViewInitializationStartedEventArgs]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
