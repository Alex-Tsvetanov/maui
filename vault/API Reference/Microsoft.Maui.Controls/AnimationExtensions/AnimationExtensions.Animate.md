---
title: "AnimationExtensions.Animate"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.AnimationExtensions.Animate"
declaring_type: "AnimationExtensions"
member_kind: method
---

# AnimationExtensions.Animate

> [!abstract] Method of [[AnimationExtensions|AnimationExtensions]]
> Namespace: `Microsoft.Maui.Controls`

Runs `animation` on `self`.

## Signatures

```csharp
void static Animate(this Microsoft.Maui.Controls.IAnimatable self, string name, Microsoft.Maui.Controls.Animation animation, uint rate = 16, uint length = 250, Microsoft.Maui.Easing easing = null, System.Action<double, bool> finished = null, System.Func<bool> repeat = null)
void static Animate(this Microsoft.Maui.Controls.IAnimatable self, string name, System.Action<double> callback, double start, double end, uint rate = 16, uint length = 250, Microsoft.Maui.Easing easing = null, System.Action<double, bool> finished = null, System.Func<bool> repeat = null)
void static Animate(this Microsoft.Maui.Controls.IAnimatable self, string name, System.Action<double> callback, uint rate = 16, uint length = 250, Microsoft.Maui.Easing easing = null, System.Action<double, bool> finished = null, System.Func<bool> repeat = null)
```

## See also

- Declaring type: [[AnimationExtensions|AnimationExtensions]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
