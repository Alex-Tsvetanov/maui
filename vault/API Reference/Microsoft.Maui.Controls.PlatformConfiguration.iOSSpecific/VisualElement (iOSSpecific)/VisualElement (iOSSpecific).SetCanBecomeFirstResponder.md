---
title: "VisualElement (iOSSpecific).SetCanBecomeFirstResponder"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-PlatformConfiguration-iOSSpecific
aliases:
  - "Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific.VisualElement.SetCanBecomeFirstResponder"
declaring_type: "VisualElement (iOSSpecific)"
member_kind: method
---

# VisualElement (iOSSpecific).SetCanBecomeFirstResponder

> [!abstract] Method of [[VisualElement (iOSSpecific)|VisualElement (iOSSpecific)]]
> Namespace: `Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific`

Sets whether this element can become the first responder to touch events, rather than the page containing the element.

## Signatures

```csharp
void static SetCanBecomeFirstResponder(Microsoft.Maui.Controls.BindableObject element, bool value)
Microsoft.Maui.Controls.IPlatformElementConfiguration<Microsoft.Maui.Controls.PlatformConfiguration.iOS, Microsoft.Maui.Controls.VisualElement> static SetCanBecomeFirstResponder(this Microsoft.Maui.Controls.IPlatformElementConfiguration<Microsoft.Maui.Controls.PlatformConfiguration.iOS, Microsoft.Maui.Controls.VisualElement> config, bool value)
```

## Parameters

| Parameter | Description |
|---|---|
| `element` | The platform specific element on which to perform the operation. |
| `value` | `true` to set this element as the first responder. Otherwise, `false`. |

## See also

- Declaring type: [[VisualElement (iOSSpecific)|VisualElement (iOSSpecific)]]
- [[_Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific|Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific namespace]]
