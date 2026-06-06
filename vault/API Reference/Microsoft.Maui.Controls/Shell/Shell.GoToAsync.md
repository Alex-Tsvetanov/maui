---
title: "Shell.GoToAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Shell.GoToAsync"
declaring_type: "Shell"
member_kind: method
---

# Shell.GoToAsync

> [!abstract] Method of [[Shell|Shell]]
> Namespace: `Microsoft.Maui.Controls`

Asynchronously navigates to the specified `state`.

## Signatures

```csharp
System.Threading.Tasks.Task GoToAsync(Microsoft.Maui.Controls.ShellNavigationState state, bool animate, Microsoft.Maui.Controls.ShellNavigationQueryParameters shellNavigationQueryParameters)
System.Threading.Tasks.Task GoToAsync(Microsoft.Maui.Controls.ShellNavigationState state, bool animate, System.Collections.Generic.IDictionary<string, object> parameters)
System.Threading.Tasks.Task GoToAsync(Microsoft.Maui.Controls.ShellNavigationState state, bool animate)
System.Threading.Tasks.Task GoToAsync(Microsoft.Maui.Controls.ShellNavigationState state, Microsoft.Maui.Controls.ShellNavigationQueryParameters shellNavigationQueryParameters)
System.Threading.Tasks.Task GoToAsync(Microsoft.Maui.Controls.ShellNavigationState state, System.Collections.Generic.IDictionary<string, object> parameters)
System.Threading.Tasks.Task GoToAsync(Microsoft.Maui.Controls.ShellNavigationState state)
```

## Parameters

| Parameter | Description |
|---|---|
| `state` | The shell navigation state to navigate to. |

## Returns

A task that represents the asynchronous navigation operation.

## See also

- Declaring type: [[Shell|Shell]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
