---
title: "Application (iOSSpecific).SetHandleControlUpdatesOnMainThread"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-PlatformConfiguration-iOSSpecific
aliases:
  - "Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific.Application.SetHandleControlUpdatesOnMainThread"
declaring_type: "Application (iOSSpecific)"
member_kind: method
---

# Application (iOSSpecific).SetHandleControlUpdatesOnMainThread

> [!abstract] Method of [[Application (iOSSpecific)|Application (iOSSpecific)]]
> Namespace: `Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific`

Sets whether control property updates are processed on the main thread on iOS.

## Signatures

```csharp
void static SetHandleControlUpdatesOnMainThread(Microsoft.Maui.Controls.BindableObject element, bool value)
Microsoft.Maui.Controls.IPlatformElementConfiguration<Microsoft.Maui.Controls.PlatformConfiguration.iOS, Microsoft.Maui.Controls.Application> static SetHandleControlUpdatesOnMainThread(this Microsoft.Maui.Controls.IPlatformElementConfiguration<Microsoft.Maui.Controls.PlatformConfiguration.iOS, Microsoft.Maui.Controls.Application> config, bool value)
```

## Parameters

| Parameter | Description |
|---|---|
| `element` | The element to set the value on. |
| `value` | `true` to handle updates on the main thread; otherwise, `false`. |

## See also

- Declaring type: [[Application (iOSSpecific)|Application (iOSSpecific)]]
- [[_Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific|Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific namespace]]
