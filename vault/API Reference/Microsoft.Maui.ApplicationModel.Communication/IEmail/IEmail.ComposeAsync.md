---
title: "IEmail.ComposeAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel-Communication
aliases:
  - "Microsoft.Maui.ApplicationModel.Communication.IEmail.ComposeAsync"
declaring_type: "IEmail"
member_kind: method
---

# IEmail.ComposeAsync

> [!abstract] Method of [[IEmail|IEmail]]
> Namespace: `Microsoft.Maui.ApplicationModel.Communication`

Opens the default email client to allow the user to send the message.

## Signature

```csharp
System.Threading.Tasks.Task! ComposeAsync(Microsoft.Maui.ApplicationModel.Communication.EmailMessage? message)
```

## Returns

A `Task` object with the current status of the asynchronous operation.

## Parameters

| Parameter | Description |
|---|---|
| `message` | Instance of `EmailMessage` containing details of the email message to compose. |

## See also

- Declaring type: [[IEmail|IEmail]]
- [[_Microsoft.Maui.ApplicationModel.Communication|Microsoft.Maui.ApplicationModel.Communication namespace]]
