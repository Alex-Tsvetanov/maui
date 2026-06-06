---
title: "IShare.RequestAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel-DataTransfer
aliases:
  - "Microsoft.Maui.ApplicationModel.DataTransfer.IShare.RequestAsync"
declaring_type: "IShare"
member_kind: method
---

# IShare.RequestAsync

> [!abstract] Method of [[IShare|IShare]]
> Namespace: `Microsoft.Maui.ApplicationModel.DataTransfer`

Show the operating systems user interface to share text.

## Signatures

```csharp
System.Threading.Tasks.Task! RequestAsync(Microsoft.Maui.ApplicationModel.DataTransfer.ShareFileRequest! request)
System.Threading.Tasks.Task! RequestAsync(Microsoft.Maui.ApplicationModel.DataTransfer.ShareMultipleFilesRequest! request)
System.Threading.Tasks.Task! RequestAsync(Microsoft.Maui.ApplicationModel.DataTransfer.ShareTextRequest! request)
```

## Returns

A `Task` object with the current status of the asynchronous operation.

## Parameters

| Parameter | Description |
|---|---|
| `request` | A `ShareTextRequest` object containing the details of the data to share. |

## See also

- Declaring type: [[IShare|IShare]]
- [[_Microsoft.Maui.ApplicationModel.DataTransfer|Microsoft.Maui.ApplicationModel.DataTransfer namespace]]
