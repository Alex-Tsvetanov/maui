---
title: "ScrollView (iOSSpecific).SetShouldDelayContentTouches"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-PlatformConfiguration-iOSSpecific
aliases:
  - "Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific.ScrollView.SetShouldDelayContentTouches"
declaring_type: "ScrollView (iOSSpecific)"
member_kind: method
---

# ScrollView (iOSSpecific).SetShouldDelayContentTouches

> [!abstract] Method of [[ScrollView (iOSSpecific)|ScrollView (iOSSpecific)]]
> Namespace: `Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific`

Sets whether iOS delays touch events to determine scroll intent.

## Signatures

```csharp
void static SetShouldDelayContentTouches(Microsoft.Maui.Controls.BindableObject element, bool value)
Microsoft.Maui.Controls.IPlatformElementConfiguration<Microsoft.Maui.Controls.PlatformConfiguration.iOS, Microsoft.Maui.Controls.ScrollView> static SetShouldDelayContentTouches(this Microsoft.Maui.Controls.IPlatformElementConfiguration<Microsoft.Maui.Controls.PlatformConfiguration.iOS, Microsoft.Maui.Controls.ScrollView> config, bool value)
```

## Parameters

| Parameter | Description |
|---|---|
| `element` | The element to set the value on. |
| `value` | `true` to delay; `false` for immediate touch response. |

## See also

- Declaring type: [[ScrollView (iOSSpecific)|ScrollView (iOSSpecific)]]
- [[_Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific|Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific namespace]]
