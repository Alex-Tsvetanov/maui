---
title: "RadioButton.ContentAsString"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.RadioButton.ContentAsString"
declaring_type: "RadioButton"
member_kind: method
---

# RadioButton.ContentAsString

> [!abstract] Method of [[RadioButton|RadioButton]]
> Namespace: `Microsoft.Maui.Controls`

Converts the `Content` to a string representation.

## Signature

```csharp
string ContentAsString()
```

## Returns

The string representation of the content, or the result of ToString() if content is not a string.

## Remarks

If `Content` is a `View` and no `ControlTemplate` is set, a warning is logged and the ToString() representation is used instead. When a ControlTemplate is applied, View content is supported.

## See also

- Declaring type: [[RadioButton|RadioButton]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
