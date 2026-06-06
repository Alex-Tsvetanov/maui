---
title: "Command.Command"
tags:
  - api
  - member/constructor
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Command.Command"
declaring_type: "Command"
member_kind: constructor
---

# Command.Command

> [!abstract] Constructor of [[Command|Command]]
> Namespace: `Microsoft.Maui.Controls`

Creates a new command with the specified execute action.

## Signatures

```csharp
void Command(System.Action execute, System.Func<bool> canExecute)
void Command(System.Action execute)
void Command(System.Action<object> execute, System.Func<object, bool> canExecute)
void Command(System.Action<object> execute)
```

## Parameters

| Parameter | Description |
|---|---|
| `execute` | The action to execute when the command is invoked. |

## See also

- Declaring type: [[Command|Command]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
