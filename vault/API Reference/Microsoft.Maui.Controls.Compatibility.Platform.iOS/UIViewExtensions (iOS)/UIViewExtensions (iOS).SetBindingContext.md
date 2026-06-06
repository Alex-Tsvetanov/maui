---
title: "UIViewExtensions (iOS).SetBindingContext"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-Compatibility-Platform-iOS
aliases:
  - "Microsoft.Maui.Controls.Compatibility.Platform.iOS.UIViewExtensions.SetBindingContext"
declaring_type: "UIViewExtensions (iOS)"
member_kind: method
---

# UIViewExtensions (iOS).SetBindingContext

> [!abstract] Method of [[UIViewExtensions (iOS)|UIViewExtensions (iOS)]]
> Namespace: `Microsoft.Maui.Controls.Compatibility.Platform.iOS`

Sets the binding context of the native view, using the supplied delegate to enumerate child views.

## Signature

```csharp
void static SetBindingContext(this UIKit.UIView target, object bindingContext, System.Func<UIKit.UIView, System.Collections.Generic.IEnumerable<UIKit.UIView>> getChildren = null)
```

## See also

- Declaring type: [[UIViewExtensions (iOS)|UIViewExtensions (iOS)]]
- [[_Microsoft.Maui.Controls.Compatibility.Platform.iOS|Microsoft.Maui.Controls.Compatibility.Platform.iOS namespace]]
