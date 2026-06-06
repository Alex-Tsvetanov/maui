---
title: "Platform.OpenUrl"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.Platform.OpenUrl"
declaring_type: "Platform"
member_kind: method
---

# Platform.OpenUrl

> [!abstract] Method of [[Platform|Platform]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Opens the specified URI to start a authentication flow.

## Signature

```csharp
bool static OpenUrl(UIKit.UIApplication! app, Foundation.NSUrl! url, Foundation.NSDictionary! options)
```

## Parameters

| Parameter | Description |
|---|---|
| `app` | This parameters is not used. |
| `url` | The URL to open that will start the authentication flow. |
| `options` | This parameters is not used. |

## Returns

`true` when the URI has been opened, otherwise `false`.

## See also

- Declaring type: [[Platform|Platform]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
