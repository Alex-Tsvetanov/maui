---
title: "KeyboardAutoManagerScroll.Connect"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.KeyboardAutoManagerScroll.Connect"
declaring_type: "KeyboardAutoManagerScroll"
member_kind: method
---

# KeyboardAutoManagerScroll.Connect

> [!abstract] Method of [[KeyboardAutoManagerScroll|KeyboardAutoManagerScroll]]
> Namespace: `Microsoft.Maui.Platform`

Enables automatic scrolling with keyboard interactions on iOS devices.

## Signature

```csharp
void static Connect()
```

## Remarks

This method is being called by default on iOS and will scroll the page when the keyboard comes up. Call the method 'KeyboardAutoManagerScroll.Disconnect()' to remove this scrolling behavior.

## See also

- Declaring type: [[KeyboardAutoManagerScroll|KeyboardAutoManagerScroll]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
