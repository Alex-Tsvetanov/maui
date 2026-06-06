---
title: "Platform.Init"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.Platform.Init"
declaring_type: "Platform"
member_kind: method
---

# Platform.Init

> [!abstract] Method of [[Platform|Platform]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Initializes the `WindowStateManager` for the given `UIViewController`.

## Signatures

```csharp
void static Init(Android.App.Activity! activity, Android.OS.Bundle? bundle)
void static Init(Android.App.Application! application)
void static Init(System.Func<UIKit.UIViewController!>? getCurrentUIViewController)
```

## Parameters

| Parameter | Description |
|---|---|
| `getCurrentUIViewController` | The `UIViewController` to use for initialization. |

## See also

- Declaring type: [[Platform|Platform]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
