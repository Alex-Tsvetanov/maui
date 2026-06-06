---
title: "BindingBase.TargetNullValue"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.BindingBase.TargetNullValue"
declaring_type: "BindingBase"
member_kind: property
---

# BindingBase.TargetNullValue

> [!abstract] Property of [[BindingBase|BindingBase]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the value to use when the binding successfully resolves the source path and the resulting source value is `null`.

## Signature

```csharp
object TargetNullValue { get; set; }
```

## Remarks

TargetNullValue acts like a null-coalescing value for the binding source result: if the binding path resolves and the value is `null`, the target receives TargetNullValue instead. It is not used when the binding cannot resolve (e.g. missing property, conversion error) — in those cases `FallbackValue` is considered.

## See also

- Declaring type: [[BindingBase|BindingBase]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
