---
title: "UIViewExtensions (Platform).GetPointsInView"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Platform
aliases:
  - "Microsoft.Maui.Graphics.Platform.UIViewExtensions.GetPointsInView"
declaring_type: "UIViewExtensions (Platform)"
member_kind: method
---

# UIViewExtensions (Platform).GetPointsInView

> [!abstract] Method of [[UIViewExtensions (Platform)|UIViewExtensions (Platform)]]
> Namespace: `Microsoft.Maui.Graphics.Platform`

Returns the touch points relative to the specified iOS view from the given touch set or event.

## Signatures

```csharp
Microsoft.Maui.Graphics.PointF[] static GetPointsInView(this UIKit.UIView target, Foundation.NSSet touchSet)
Microsoft.Maui.Graphics.PointF[] static GetPointsInView(this UIKit.UIView target, UIKit.UIEvent touchEvent)
```

## See also

- Declaring type: [[UIViewExtensions (Platform)|UIViewExtensions (Platform)]]
- [[_Microsoft.Maui.Graphics.Platform|Microsoft.Maui.Graphics.Platform namespace]]
