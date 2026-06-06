---
title: "Email.ComposeAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel-Communication
aliases:
  - "Microsoft.Maui.ApplicationModel.Communication.Email.ComposeAsync"
declaring_type: "Email"
member_kind: method
---

# Email.ComposeAsync

> [!abstract] Method of [[Email|Email]]
> Namespace: `Microsoft.Maui.ApplicationModel.Communication`

Opens the default email client to allow the user to send the message.

## Signatures

```csharp
System.Threading.Tasks.Task! static ComposeAsync()
System.Threading.Tasks.Task! static ComposeAsync(Microsoft.Maui.ApplicationModel.Communication.EmailMessage! message)
System.Threading.Tasks.Task! static ComposeAsync(string! subject, string! body, params string![]! to)
```

## Returns

A `Task` object with the current status of the asynchronous operation.

## See also

- Declaring type: [[Email|Email]]
- [[_Microsoft.Maui.ApplicationModel.Communication|Microsoft.Maui.ApplicationModel.Communication namespace]]
