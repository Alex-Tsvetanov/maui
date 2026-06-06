---
title: "MauiScrollView.SafeAreaInsetsDidChange"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.MauiScrollView.SafeAreaInsetsDidChange"
declaring_type: "MauiScrollView"
member_kind: method
---

# MauiScrollView.SafeAreaInsetsDidChange

> [!abstract] Method of [[MauiScrollView|MauiScrollView]]
> Namespace: `Microsoft.Maui.Platform`

Called by iOS when the safe area insets change (e.g., device rotation, notch visibility). This method marks the safe area as invalidated. Note that UIKit automatically calls LayoutSubviews immediately after this method.

## Signature

```csharp
void override SafeAreaInsetsDidChange()
```

## See also

- Declaring type: [[MauiScrollView|MauiScrollView]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
