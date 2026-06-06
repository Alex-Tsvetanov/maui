---
title: "Page (iOSSpecific).SetUseSafeArea"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-PlatformConfiguration-iOSSpecific
aliases:
  - "Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific.Page.SetUseSafeArea"
declaring_type: "Page (iOSSpecific)"
member_kind: method
---

# Page (iOSSpecific).SetUseSafeArea

> [!abstract] Method of [[Page (iOSSpecific)|Page (iOSSpecific)]]
> Namespace: `Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific`

Sets a value that controls whether padding values are overridden with the safe area insets.

## Signatures

```csharp
void static SetUseSafeArea(Microsoft.Maui.Controls.BindableObject element, bool value)
Microsoft.Maui.Controls.IPlatformElementConfiguration<Microsoft.Maui.Controls.PlatformConfiguration.iOS, Microsoft.Maui.Controls.Page> static SetUseSafeArea(this Microsoft.Maui.Controls.IPlatformElementConfiguration<Microsoft.Maui.Controls.PlatformConfiguration.iOS, Microsoft.Maui.Controls.Page> config, bool value)
```

## Parameters

| Parameter | Description |
|---|---|
| `element` | The element whose safe area use behavior to set. |
| `value` | `true` to use the safe area inset behavior; otherwise, `false`. |

## Remarks

This API is deprecated. Use SafeAreaEdges attached property instead for per-edge safe area control.

## See also

- Declaring type: [[Page (iOSSpecific)|Page (iOSSpecific)]]
- [[_Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific|Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific namespace]]
