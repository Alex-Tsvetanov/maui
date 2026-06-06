---
title: "EmailExtensions.ComposeAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel-Communication
aliases:
  - "Microsoft.Maui.ApplicationModel.Communication.EmailExtensions.ComposeAsync"
declaring_type: "EmailExtensions"
member_kind: method
---

# EmailExtensions.ComposeAsync

> [!abstract] Method of [[EmailExtensions|EmailExtensions]]
> Namespace: `Microsoft.Maui.ApplicationModel.Communication`

Opens the default email client to allow the user to send the message.

## Signatures

```csharp
System.Threading.Tasks.Task! static ComposeAsync(this Microsoft.Maui.ApplicationModel.Communication.IEmail! email, string! subject, string! body, params string![]! to)
System.Threading.Tasks.Task! static ComposeAsync(this Microsoft.Maui.ApplicationModel.Communication.IEmail! email)
```

## Parameters

| Parameter | Description |
|---|---|
| `email` | The object this method is invoked on. |

## Returns

A `Task` object with the current status of the asynchronous operation.

## See also

- Declaring type: [[EmailExtensions|EmailExtensions]]
- [[_Microsoft.Maui.ApplicationModel.Communication|Microsoft.Maui.ApplicationModel.Communication namespace]]
