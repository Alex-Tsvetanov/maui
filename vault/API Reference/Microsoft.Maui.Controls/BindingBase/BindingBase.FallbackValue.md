---
title: "BindingBase.FallbackValue"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.BindingBase.FallbackValue"
declaring_type: "BindingBase"
member_kind: property
---

# BindingBase.FallbackValue

> [!abstract] Property of [[BindingBase|BindingBase]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the value used when the binding cannot produce a source value (e.g. path not found, conversion failure).

## Signature

```csharp
object FallbackValue { get; set; }
```

## Remarks

FallbackValue is applied when the binding engine fails to obtain a value (e.g., missing source, unresolved path, or type conversion failure within the binding engine itself). It is not used for errors that occur inside value converters; such errors may be handled by the converter or use different fallback mechanisms. If the source resolves to `null`, `TargetNullValue` is applied if set. Together with `TargetNullValue` this allows differentiating between a legitimate `null` value and an unresolved binding.

## See also

- Declaring type: [[BindingBase|BindingBase]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
