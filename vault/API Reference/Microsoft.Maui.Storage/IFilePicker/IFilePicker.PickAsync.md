---
title: "IFilePicker.PickAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Storage
aliases:
  - "Microsoft.Maui.Storage.IFilePicker.PickAsync"
declaring_type: "IFilePicker"
member_kind: method
---

# IFilePicker.PickAsync

> [!abstract] Method of [[IFilePicker|IFilePicker]]
> Namespace: `Microsoft.Maui.Storage`

Opens the default file picker to allow the user to pick a single file.

## Signature

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.Storage.FileResult?>! PickAsync(Microsoft.Maui.Storage.PickOptions? options = null)
```

## Remarks

File types can be specified in order to limit files that can be selected, using a `PickOptions` object. Note that this method may re-throw platform specific exceptions that occurred during file picking. When calling `PickAsync` again while showing a file picker, the `Task` object that was returned from the first call is cancelled. Be sure to also handle the `TaskCanceledException` in this case.

## Returns

File picking result object, or `null` if picking was cancelled by the user.

## Parameters

| Parameter | Description |
|---|---|
| `options` | File picker options to use; may be `null` for default options. |

## See also

- Declaring type: [[IFilePicker|IFilePicker]]
- [[_Microsoft.Maui.Storage|Microsoft.Maui.Storage namespace]]
