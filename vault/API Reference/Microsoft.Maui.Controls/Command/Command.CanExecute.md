---
title: "Command.CanExecute"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Command.CanExecute"
declaring_type: "Command"
member_kind: method
---

# Command.CanExecute

> [!abstract] Method of [[Command|Command]]
> Namespace: `Microsoft.Maui.Controls`

Returns a `Boolean` indicating if the Command can be exectued with the given parameter.

## Signature

```csharp
bool CanExecute(object parameter)
```

## Parameters

| Parameter | Description |
|---|---|
| `parameter` | An `Object` used as parameter to determine if the Command can be executed. |

## Returns

`true` if the Command can be executed, `false` otherwise.

## Remarks

If no canExecute parameter was passed to the Command constructor, this method always returns If the Command was created with non-generic execute parameter, the parameter of this method is ignored.

## See also

- Declaring type: [[Command|Command]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
