---
title: "IFileSystem.OpenAppPackageFileAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Storage
aliases:
  - "Microsoft.Maui.Storage.IFileSystem.OpenAppPackageFileAsync"
declaring_type: "IFileSystem"
member_kind: method
---

# IFileSystem.OpenAppPackageFileAsync

> [!abstract] Method of [[IFileSystem|IFileSystem]]
> Namespace: `Microsoft.Maui.Storage`

Opens a stream to a file contained within the app package.

## Signature

```csharp
System.Threading.Tasks.Task<System.IO.Stream!>! OpenAppPackageFileAsync(string! filename)
```

## Returns

A `Stream` containing the (read-only) file data.

## Parameters

| Parameter | Description |
|---|---|
| `filename` | The name of the file (excluding the path) to load from the app package. |

## See also

- Declaring type: [[IFileSystem|IFileSystem]]
- [[_Microsoft.Maui.Storage|Microsoft.Maui.Storage namespace]]
