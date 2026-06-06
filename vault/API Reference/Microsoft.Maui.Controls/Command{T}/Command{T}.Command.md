---
title: "Command<T>.Command"
tags:
  - api
  - member/constructor
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Command<T>.Command"
declaring_type: "Command<T>"
member_kind: constructor
---

# Command<T>.Command

> [!abstract] Constructor of [[Command{T}|Command<T>]]
> Namespace: `Microsoft.Maui.Controls`

Creates a new command with the specified execute action.

## Signatures

```csharp
void Microsoft.Maui.Controls.Command<T>.Command(System.Action<T> execute, System.Func<T, bool> canExecute)
void Microsoft.Maui.Controls.Command<T>.Command(System.Action<T> execute)
```

## Parameters

| Parameter | Description |
|---|---|
| `execute` | The action to execute when the command is invoked. |

## See also

- Declaring type: [[Command{T}|Command<T>]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
