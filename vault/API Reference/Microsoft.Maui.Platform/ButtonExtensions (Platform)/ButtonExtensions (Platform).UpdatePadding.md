---
title: "ButtonExtensions (Platform).UpdatePadding"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.ButtonExtensions.UpdatePadding"
declaring_type: "ButtonExtensions (Platform)"
member_kind: method
---

# ButtonExtensions (Platform).UpdatePadding

> [!abstract] Method of [[ButtonExtensions (Platform)|ButtonExtensions (Platform)]]
> Namespace: `Microsoft.Maui.Platform`

Updates the padding of the native button from the cross-platform padding.

## Signatures

```csharp
void static UpdatePadding(this Android.Widget.Button! platformControl, Microsoft.Maui.IPadding! padding, Microsoft.Maui.Thickness? defaultPadding = null)
void static UpdatePadding(this Android.Widget.Button! platformControl, Microsoft.Maui.Thickness padding, Microsoft.Maui.Thickness? defaultPadding = null)
void static UpdatePadding(this UIKit.UIButton! platformButton, Microsoft.Maui.IButton! button, Microsoft.Maui.Thickness? defaultPadding = null)
void static UpdatePadding(this UIKit.UIButton! platformButton, Microsoft.Maui.Thickness padding, Microsoft.Maui.Thickness? defaultPadding = null)
void static UpdatePadding(this Microsoft.UI.Xaml.Controls.Button! platformButton, Microsoft.Maui.IPadding! padding)
```

## See also

- Declaring type: [[ButtonExtensions (Platform)|ButtonExtensions (Platform)]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
