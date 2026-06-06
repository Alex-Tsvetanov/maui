---
title: "VisualElement (iOSSpecific).CanBecomeFirstResponder"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-PlatformConfiguration-iOSSpecific
aliases:
  - "Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific.VisualElement.CanBecomeFirstResponder"
declaring_type: "VisualElement (iOSSpecific)"
member_kind: method
---

# VisualElement (iOSSpecific).CanBecomeFirstResponder

> [!abstract] Method of [[VisualElement (iOSSpecific)|VisualElement (iOSSpecific)]]
> Namespace: `Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific`

Gets whether this element can become the first responder to touch events, rather than the page containing the element.

## Signature

```csharp
bool static CanBecomeFirstResponder(this Microsoft.Maui.Controls.IPlatformElementConfiguration<Microsoft.Maui.Controls.PlatformConfiguration.iOS, Microsoft.Maui.Controls.VisualElement> config)
```

## Parameters

| Parameter | Description |
|---|---|
| `config` | The platform specific configuration that contains the element on which to perform the operation. |

## Returns

`true` when this element can become first responder. Otherwise, `false`.

## See also

- Declaring type: [[VisualElement (iOSSpecific)|VisualElement (iOSSpecific)]]
- [[_Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific|Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific namespace]]
