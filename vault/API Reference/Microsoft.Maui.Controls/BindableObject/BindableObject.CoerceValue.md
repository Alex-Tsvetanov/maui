---
title: "BindableObject.CoerceValue"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.BindableObject.CoerceValue"
declaring_type: "BindableObject"
member_kind: method
---

# BindableObject.CoerceValue

> [!abstract] Method of [[BindableObject|BindableObject]]
> Namespace: `Microsoft.Maui.Controls`

Coerces the value of the specified bindable property. This is done by invoking `CoerceValueDelegate` of the specified bindable property.

## Signatures

```csharp
void CoerceValue(Microsoft.Maui.Controls.BindableProperty property)
void CoerceValue(Microsoft.Maui.Controls.BindablePropertyKey propertyKey)
```

## Parameters

| Parameter | Description |
|---|---|
| `property` | The bindable property to coerce the value of. |

## Remarks

If `CoerceValueDelegate` is not assigned to, nothing will happen.

## See also

- Declaring type: [[BindableObject|BindableObject]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
