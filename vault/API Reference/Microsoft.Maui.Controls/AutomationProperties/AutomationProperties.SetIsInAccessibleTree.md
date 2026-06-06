---
title: "AutomationProperties.SetIsInAccessibleTree"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.AutomationProperties.SetIsInAccessibleTree"
declaring_type: "AutomationProperties"
member_kind: method
---

# AutomationProperties.SetIsInAccessibleTree

> [!abstract] Method of [[AutomationProperties|AutomationProperties]]
> Namespace: `Microsoft.Maui.Controls`

Sets a Boolean value that tells whether the bindable object is available to the accessibility system.

## Signature

```csharp
void static SetIsInAccessibleTree(Microsoft.Maui.Controls.BindableObject bindable, bool? value)
```

## Parameters

| Parameter | Description |
|---|---|
| `bindable` | The object ot add or remove from the accessibility system. |
| `value` | `true` to make `bindable` visible to the accessibility system. `false` to remove it from the system. |

## See also

- Declaring type: [[AutomationProperties|AutomationProperties]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
