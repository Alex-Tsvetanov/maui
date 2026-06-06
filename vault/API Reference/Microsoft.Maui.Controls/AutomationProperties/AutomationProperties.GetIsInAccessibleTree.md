---
title: "AutomationProperties.GetIsInAccessibleTree"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.AutomationProperties.GetIsInAccessibleTree"
declaring_type: "AutomationProperties"
member_kind: method
---

# AutomationProperties.GetIsInAccessibleTree

> [!abstract] Method of [[AutomationProperties|AutomationProperties]]
> Namespace: `Microsoft.Maui.Controls`

Gets a nullable Boolean value that tells whether the bindable object is available to the accessibility system.

## Signature

```csharp
bool? static GetIsInAccessibleTree(Microsoft.Maui.Controls.BindableObject bindable)
```

## Parameters

| Parameter | Description |
|---|---|
| `bindable` | The bindable object whose status to check. |

## Returns

`true` if `bindable` is available to the accessibility system. `false` or `null` if it is not.

## See also

- Declaring type: [[AutomationProperties|AutomationProperties]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
