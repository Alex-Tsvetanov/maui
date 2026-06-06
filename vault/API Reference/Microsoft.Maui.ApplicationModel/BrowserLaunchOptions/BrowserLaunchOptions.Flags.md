---
title: "BrowserLaunchOptions.Flags"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.BrowserLaunchOptions.Flags"
declaring_type: "BrowserLaunchOptions"
member_kind: property
---

# BrowserLaunchOptions.Flags

> [!abstract] Property of [[BrowserLaunchOptions|BrowserLaunchOptions]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Gets or sets additional launch flags that may or may not take effect based on the device and `LaunchMode`.

## Signature

```csharp
Microsoft.Maui.ApplicationModel.BrowserLaunchFlags Flags { get; set; }
```

## Remarks

The default value is `None`. Not all flags work on all platforms, check the flag descriptions.

## See also

- Declaring type: [[BrowserLaunchOptions|BrowserLaunchOptions]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
