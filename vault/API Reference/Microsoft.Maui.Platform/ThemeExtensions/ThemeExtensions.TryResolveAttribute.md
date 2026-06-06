---
title: "ThemeExtensions.TryResolveAttribute"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.ThemeExtensions.TryResolveAttribute"
declaring_type: "ThemeExtensions"
member_kind: method
---

# ThemeExtensions.TryResolveAttribute

> [!abstract] Method of [[ThemeExtensions|ThemeExtensions]]
> Namespace: `Microsoft.Maui.Platform`

Attempts to resolve the specified Android theme attribute by id, returning its value through the out parameter.

## Signatures

```csharp
bool static TryResolveAttribute(this Android.Content.Res.Resources.Theme? theme, int id, out bool? value)
bool static TryResolveAttribute(this Android.Content.Res.Resources.Theme? theme, int id, out float? value)
bool static TryResolveAttribute(this Android.Content.Res.Resources.Theme? theme, int id)
```

## See also

- Declaring type: [[ThemeExtensions|ThemeExtensions]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
