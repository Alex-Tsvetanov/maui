---
title: "Browser.OpenAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.Browser.OpenAsync"
declaring_type: "Browser"
member_kind: method
---

# Browser.OpenAsync

> [!abstract] Method of [[Browser|Browser]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Open the browser to specified URI.

## Signatures

```csharp
System.Threading.Tasks.Task<bool>! static OpenAsync(string! uri, Microsoft.Maui.ApplicationModel.BrowserLaunchMode launchMode)
System.Threading.Tasks.Task<bool>! static OpenAsync(string! uri, Microsoft.Maui.ApplicationModel.BrowserLaunchOptions! options)
System.Threading.Tasks.Task<bool>! static OpenAsync(string! uri)
System.Threading.Tasks.Task<bool>! static OpenAsync(System.Uri! uri, Microsoft.Maui.ApplicationModel.BrowserLaunchMode launchMode)
System.Threading.Tasks.Task<bool>! static OpenAsync(System.Uri! uri, Microsoft.Maui.ApplicationModel.BrowserLaunchOptions! options)
System.Threading.Tasks.Task<bool>! static OpenAsync(System.Uri! uri)
```

## Returns

Completed task when browser is launched, but not necessarily closed. Result indicates if launching was successful or not.

## Parameters

| Parameter | Description |
|---|---|
| `uri` | URI to open. |

## See also

- Declaring type: [[Browser|Browser]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
