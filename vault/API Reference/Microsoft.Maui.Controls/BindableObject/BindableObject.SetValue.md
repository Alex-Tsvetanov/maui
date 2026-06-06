---
title: "BindableObject.SetValue"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.BindableObject.SetValue"
declaring_type: "BindableObject"
member_kind: method
---

# BindableObject.SetValue

> [!abstract] Method of [[BindableObject|BindableObject]]
> Namespace: `Microsoft.Maui.Controls`

Sets the value of the specified bindable property.

## Signatures

```csharp
void SetValue(Microsoft.Maui.Controls.BindableProperty property, object value)
void SetValue(Microsoft.Maui.Controls.BindablePropertyKey propertyKey, object value)
```

## Parameters

| Parameter | Description |
|---|---|
| `property` | The bindable property on which to assign a value. |
| `value` | The value to set. |

## Remarks

If `property` is read-only, nothing will happen.

## See also

- Declaring type: [[BindableObject|BindableObject]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
