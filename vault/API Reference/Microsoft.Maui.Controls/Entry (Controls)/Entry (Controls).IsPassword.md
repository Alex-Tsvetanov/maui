---
title: "Entry (Controls).IsPassword"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Entry.IsPassword"
declaring_type: "Entry (Controls)"
member_kind: property
---

# Entry (Controls).IsPassword

> [!abstract] Property of [[Entry (Controls)|Entry (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets a value that indicates if the entry should visually obscure typed text. Value is `true` if the element is a password box; otherwise, `false`. Default value is `false`. This is a bindable property.

## Signature

```csharp
bool IsPassword { get; set; }
```

## Remarks

Toggling this value does not reset the contents of the entry, therefore it is advisable to be careful about setting `IsPassword` to false, as it may contain sensitive information.

## See also

- Declaring type: [[Entry (Controls)|Entry (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
