---
title: "Application (iOSSpecific).GetPanGestureRecognizerShouldRecognizeSimultaneously"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-PlatformConfiguration-iOSSpecific
aliases:
  - "Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific.Application.GetPanGestureRecognizerShouldRecognizeSimultaneously"
declaring_type: "Application (iOSSpecific)"
member_kind: method
---

# Application (iOSSpecific).GetPanGestureRecognizerShouldRecognizeSimultaneously

> [!abstract] Method of [[Application (iOSSpecific)|Application (iOSSpecific)]]
> Namespace: `Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific`

Gets whether pan gesture recognizers can recognize gestures simultaneously with other gesture recognizers.

## Signatures

```csharp
bool static GetPanGestureRecognizerShouldRecognizeSimultaneously(Microsoft.Maui.Controls.BindableObject element)
bool static GetPanGestureRecognizerShouldRecognizeSimultaneously(this Microsoft.Maui.Controls.IPlatformElementConfiguration<Microsoft.Maui.Controls.PlatformConfiguration.iOS, Microsoft.Maui.Controls.Application> config)
```

## Parameters

| Parameter | Description |
|---|---|
| `element` | The element to get the value from. |

## Returns

`true` if simultaneous recognition is enabled; otherwise, `false`.

## See also

- Declaring type: [[Application (iOSSpecific)|Application (iOSSpecific)]]
- [[_Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific|Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific namespace]]
