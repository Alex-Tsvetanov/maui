---
title: "ViewExtensions (Microsoft.Maui.Platform).UpdateAutomationId"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.ViewExtensions.UpdateAutomationId"
declaring_type: "ViewExtensions (Microsoft.Maui.Platform)"
member_kind: method
---

# ViewExtensions (Microsoft.Maui.Platform).UpdateAutomationId

> [!abstract] Method of [[ViewExtensions (Microsoft.Maui.Platform)|ViewExtensions (Microsoft.Maui.Platform)]]
> Namespace: `Microsoft.Maui.Platform`

Updates the native platform view to reflect the cross-platform view's AutomationId value.

## Signatures

```csharp
void static UpdateAutomationId(this Android.Views.View! platformView, Microsoft.Maui.IView! view)
void static UpdateAutomationId(this UIKit.UIView! platformView, Microsoft.Maui.IView! view)
void static UpdateAutomationId(this Tizen.NUI.BaseComponents.View! platformView, Microsoft.Maui.IView! view)
void static UpdateAutomationId(this Microsoft.UI.Xaml.FrameworkElement! platformView, Microsoft.Maui.IView! view)
void static UpdateAutomationId(this object! platformView, Microsoft.Maui.IView! view)
```

## See also

- Declaring type: [[ViewExtensions (Microsoft.Maui.Platform)|ViewExtensions (Microsoft.Maui.Platform)]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
