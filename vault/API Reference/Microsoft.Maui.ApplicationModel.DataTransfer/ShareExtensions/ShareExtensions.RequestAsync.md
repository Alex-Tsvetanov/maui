---
title: "ShareExtensions.RequestAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel-DataTransfer
aliases:
  - "Microsoft.Maui.ApplicationModel.DataTransfer.ShareExtensions.RequestAsync"
declaring_type: "ShareExtensions"
member_kind: method
---

# ShareExtensions.RequestAsync

> [!abstract] Method of [[ShareExtensions|ShareExtensions]]
> Namespace: `Microsoft.Maui.ApplicationModel.DataTransfer`

Show the operating system's user interface to share text.

## Signatures

```csharp
System.Threading.Tasks.Task! static RequestAsync(this Microsoft.Maui.ApplicationModel.DataTransfer.IShare! share, string! text, string! title)
System.Threading.Tasks.Task! static RequestAsync(this Microsoft.Maui.ApplicationModel.DataTransfer.IShare! share, string! text)
```

## Parameters

| Parameter | Description |
|---|---|
| `share` | The object this method is invoked on. |
| `text` | The text to share. |

## Returns

A `Task` object with the current status of the asynchronous operation.

## See also

- Declaring type: [[ShareExtensions|ShareExtensions]]
- [[_Microsoft.Maui.ApplicationModel.DataTransfer|Microsoft.Maui.ApplicationModel.DataTransfer namespace]]
