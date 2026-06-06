---
title: "BindableObject.GetValue"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.BindableObject.GetValue"
declaring_type: "BindableObject"
member_kind: method
---

# BindableObject.GetValue

> [!abstract] Method of [[BindableObject|BindableObject]]
> Namespace: `Microsoft.Maui.Controls`

Returns the value that is contained in the given bindable property.

## Signature

```csharp
object GetValue(Microsoft.Maui.Controls.BindableProperty property)
```

## Parameters

| Parameter | Description |
|---|---|
| `property` | The bindable property for which to get the value. |

## Returns

The value that is contained in the `BindableProperty`.

## Remarks

`GetValue` and `SetValue` are used to access the values of properties that are implemented by a `BindableProperty`. That is, application developers typically provide an interface for a bound property by defining a `public` property whose `get` accessor casts the result of `GetValue` to the appropriate type and returns it, and whose `set` accessor uses `SetValue` to set the value on the correct property. Application developers should perform no other steps in the public property that defines the interface of the bound property.

## See also

- Declaring type: [[BindableObject|BindableObject]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
