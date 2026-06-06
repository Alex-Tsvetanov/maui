---
title: "FilePicker.PickMultipleAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Storage
aliases:
  - "Microsoft.Maui.Storage.FilePicker.PickMultipleAsync"
declaring_type: "FilePicker"
member_kind: method
---

# FilePicker.PickMultipleAsync

> [!abstract] Method of [[FilePicker|FilePicker]]
> Namespace: `Microsoft.Maui.Storage`

Opens the default file picker to allow the user to pick one or more files.

## Signature

```csharp
System.Threading.Tasks.Task<System.Collections.Generic.IEnumerable<Microsoft.Maui.Storage.FileResult?>!>! static PickMultipleAsync(Microsoft.Maui.Storage.PickOptions? options = null)
```

## Remarks

File types can be specified in order to limit files that can be selected, using a `PickOptions` object. Note that this method may re-throw platform specific exceptions that occurred during file picking. When calling `PickMultipleAsync` again while showing a file picker, the `Task` object that was returned from the first call is cancelled. Be sure to also handle the `TaskCanceledException` in this case.

## Returns

An IEnumerable of file picking result objects. When the operation was cancelled by the user, this will return an empty collection.

## Parameters

| Parameter | Description |
|---|---|
| `options` | File picker options to use; may be `null` for default options. |

## See also

- Declaring type: [[FilePicker|FilePicker]]
- [[_Microsoft.Maui.Storage|Microsoft.Maui.Storage namespace]]
