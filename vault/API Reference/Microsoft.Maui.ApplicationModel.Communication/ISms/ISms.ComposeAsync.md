---
title: "ISms.ComposeAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel-Communication
aliases:
  - "Microsoft.Maui.ApplicationModel.Communication.ISms.ComposeAsync"
declaring_type: "ISms"
member_kind: method
---

# ISms.ComposeAsync

> [!abstract] Method of [[ISms|ISms]]
> Namespace: `Microsoft.Maui.ApplicationModel.Communication`

Opens the default SMS client to allow the user to send the message.

## Signature

```csharp
System.Threading.Tasks.Task! ComposeAsync(Microsoft.Maui.ApplicationModel.Communication.SmsMessage? message)
```

## Returns

A `Task` object with the current status of the asynchronous operation.

## Parameters

| Parameter | Description |
|---|---|
| `message` | A `SmsMessage` instance with information about the message to send. |

## See also

- Declaring type: [[ISms|ISms]]
- [[_Microsoft.Maui.ApplicationModel.Communication|Microsoft.Maui.ApplicationModel.Communication namespace]]
