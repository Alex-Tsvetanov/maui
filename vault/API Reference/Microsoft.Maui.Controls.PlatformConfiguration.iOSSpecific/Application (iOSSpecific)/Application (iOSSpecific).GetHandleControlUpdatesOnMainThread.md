---
title: "Application (iOSSpecific).GetHandleControlUpdatesOnMainThread"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-PlatformConfiguration-iOSSpecific
aliases:
  - "Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific.Application.GetHandleControlUpdatesOnMainThread"
declaring_type: "Application (iOSSpecific)"
member_kind: method
---

# Application (iOSSpecific).GetHandleControlUpdatesOnMainThread

> [!abstract] Method of [[Application (iOSSpecific)|Application (iOSSpecific)]]
> Namespace: `Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific`

Gets whether control property updates are processed on the main thread on iOS.

## Signatures

```csharp
bool static GetHandleControlUpdatesOnMainThread(Microsoft.Maui.Controls.BindableObject element)
bool static GetHandleControlUpdatesOnMainThread(this Microsoft.Maui.Controls.IPlatformElementConfiguration<Microsoft.Maui.Controls.PlatformConfiguration.iOS, Microsoft.Maui.Controls.Application> config)
```

## Parameters

| Parameter | Description |
|---|---|
| `element` | The element to get the value from. |

## Returns

`true` if updates are handled on the main thread; otherwise, `false`.

## See also

- Declaring type: [[Application (iOSSpecific)|Application (iOSSpecific)]]
- [[_Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific|Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific namespace]]
