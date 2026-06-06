---
title: "PointerGestureRecognizer.PointerReleased"
tags:
  - api
  - member/event
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.PointerGestureRecognizer.PointerReleased"
declaring_type: "PointerGestureRecognizer"
member_kind: event
---

# PointerGestureRecognizer.PointerReleased

> [!abstract] Event of [[PointerGestureRecognizer|PointerGestureRecognizer]]
> Namespace: `Microsoft.Maui.Controls`

Raised when the pointer that has previous initiated a press is released within the view.

## Signature

```csharp
System.EventHandler<Microsoft.Maui.Controls.PointerEventArgs!>? PointerReleased
```

## Remarks

Secondary / Right Button behavior (iOS & Mac Catalyst): When `Buttons` is set to include `Secondary` on iOS or Mac Catalyst, a secondary pointer press is simulated using an internal ("fake") context menu gesture – the same approach used by `TapGestureRecognizer`. Because UIKit does not expose a stable API for tracking a continuous right-button (secondary) down state, the framework will raise `PointerPressed` followed immediately by `PointerReleased` for a secondary click. There is no intermediate prolonged pressed state for secondary button interactions on these platforms. If you need to distinguish a primary press/release sequence from a secondary one, inspect `Button` in the event handlers. Do not rely on timing (e.g., expecting a noticeable delay between pressed and released) for secondary interactions on iOS/Mac Catalyst as both events may fire in immediate succession.

## See also

- Declaring type: [[PointerGestureRecognizer|PointerGestureRecognizer]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
