---
title: "INativeBindingService.TrySetBinding"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-Xaml-Internals
aliases:
  - "Microsoft.Maui.Controls.Xaml.Internals.INativeBindingService.TrySetBinding"
declaring_type: "INativeBindingService"
member_kind: method
---

# INativeBindingService.TrySetBinding

> [!abstract] Method of [[INativeBindingService|INativeBindingService]]
> Namespace: `Microsoft.Maui.Controls.Xaml.Internals`

Attempts to apply the specified binding to a native target object by bindable property or property name, returning whether it succeeded.

## Signatures

```csharp
bool TrySetBinding(object target, Microsoft.Maui.Controls.BindableProperty property, Microsoft.Maui.Controls.BindingBase binding)
bool TrySetBinding(object target, string propertyName, Microsoft.Maui.Controls.BindingBase binding)
```

## See also

- Declaring type: [[INativeBindingService|INativeBindingService]]
- [[_Microsoft.Maui.Controls.Xaml.Internals|Microsoft.Maui.Controls.Xaml.Internals namespace]]
