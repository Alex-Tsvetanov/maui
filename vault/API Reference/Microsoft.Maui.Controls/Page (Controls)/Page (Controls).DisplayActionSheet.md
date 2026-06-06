---
title: "Page (Controls).DisplayActionSheet"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Page.DisplayActionSheet"
declaring_type: "Page (Controls)"
member_kind: method
---

# Page (Controls).DisplayActionSheet

> [!abstract] Method of [[Page (Controls)|Page (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Displays a native action sheet with the specified title and buttons and returns the selected button.

## Signatures

```csharp
System.Threading.Tasks.Task<string> DisplayActionSheet(string title, string cancel, string destruction, Microsoft.Maui.FlowDirection flowDirection, params string[] buttons)
System.Threading.Tasks.Task<string> DisplayActionSheet(string title, string cancel, string destruction, params string[] buttons)
```

## See also

- Declaring type: [[Page (Controls)|Page (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
