---
title: "Share.RequestAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel-DataTransfer
aliases:
  - "Microsoft.Maui.ApplicationModel.DataTransfer.Share.RequestAsync"
declaring_type: "Share"
member_kind: method
---

# Share.RequestAsync

> [!abstract] Method of [[Share|Share]]
> Namespace: `Microsoft.Maui.ApplicationModel.DataTransfer`

Show the operating system's user interface to share text.

## Signatures

```csharp
System.Threading.Tasks.Task! static RequestAsync(Microsoft.Maui.ApplicationModel.DataTransfer.ShareFileRequest! request)
System.Threading.Tasks.Task! static RequestAsync(Microsoft.Maui.ApplicationModel.DataTransfer.ShareMultipleFilesRequest! request)
System.Threading.Tasks.Task! static RequestAsync(Microsoft.Maui.ApplicationModel.DataTransfer.ShareTextRequest! request)
System.Threading.Tasks.Task! static RequestAsync(string! text, string! title)
System.Threading.Tasks.Task! static RequestAsync(string! text)
```

## Parameters

| Parameter | Description |
|---|---|
| `text` | The text to share. |

## Returns

A `Task` object with the current status of the asynchronous operation.

## See also

- Declaring type: [[Share|Share]]
- [[_Microsoft.Maui.ApplicationModel.DataTransfer|Microsoft.Maui.ApplicationModel.DataTransfer namespace]]
