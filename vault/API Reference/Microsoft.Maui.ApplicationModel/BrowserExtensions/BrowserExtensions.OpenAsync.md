---
title: "BrowserExtensions.OpenAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.BrowserExtensions.OpenAsync"
declaring_type: "BrowserExtensions"
member_kind: method
---

# BrowserExtensions.OpenAsync

> [!abstract] Method of [[BrowserExtensions|BrowserExtensions]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Open the browser to specified URI.

## Signatures

```csharp
System.Threading.Tasks.Task<bool>! static OpenAsync(this Microsoft.Maui.ApplicationModel.IBrowser! browser, string! uri, Microsoft.Maui.ApplicationModel.BrowserLaunchMode launchMode)
System.Threading.Tasks.Task<bool>! static OpenAsync(this Microsoft.Maui.ApplicationModel.IBrowser! browser, string! uri, Microsoft.Maui.ApplicationModel.BrowserLaunchOptions! options)
System.Threading.Tasks.Task<bool>! static OpenAsync(this Microsoft.Maui.ApplicationModel.IBrowser! browser, string! uri)
System.Threading.Tasks.Task<bool>! static OpenAsync(this Microsoft.Maui.ApplicationModel.IBrowser! browser, System.Uri! uri, Microsoft.Maui.ApplicationModel.BrowserLaunchMode launchMode)
System.Threading.Tasks.Task<bool>! static OpenAsync(this Microsoft.Maui.ApplicationModel.IBrowser! browser, System.Uri! uri)
```

## Returns

Completed task when browser is launched, but not necessarily closed. Result indicates if launching was successful or not.

## Parameters

| Parameter | Description |
|---|---|
| `browser` | The `IBrowser` instance to invoke this method on. |
| `uri` | URI to open. |

## See also

- Declaring type: [[BrowserExtensions|BrowserExtensions]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
