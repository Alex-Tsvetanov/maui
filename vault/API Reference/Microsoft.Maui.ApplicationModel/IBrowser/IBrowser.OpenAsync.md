---
title: "IBrowser.OpenAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.IBrowser.OpenAsync"
declaring_type: "IBrowser"
member_kind: method
---

# IBrowser.OpenAsync

> [!abstract] Method of [[IBrowser|IBrowser]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Open the browser to specified URI.

## Signature

```csharp
System.Threading.Tasks.Task<bool>! OpenAsync(System.Uri! uri, Microsoft.Maui.ApplicationModel.BrowserLaunchOptions! options)
```

## Returns

Completed task when browser is launched, but not necessarily closed. Result indicates if launching was successful or not.

## Parameters

| Parameter | Description |
|---|---|
| `uri` | URI to open. |
| `options` | Launch options for the browser. |

## See also

- Declaring type: [[IBrowser|IBrowser]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
