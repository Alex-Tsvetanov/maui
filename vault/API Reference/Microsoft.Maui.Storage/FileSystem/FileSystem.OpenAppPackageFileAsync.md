---
title: "FileSystem.OpenAppPackageFileAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Storage
aliases:
  - "Microsoft.Maui.Storage.FileSystem.OpenAppPackageFileAsync"
declaring_type: "FileSystem"
member_kind: method
---

# FileSystem.OpenAppPackageFileAsync

> [!abstract] Method of [[FileSystem|FileSystem]]
> Namespace: `Microsoft.Maui.Storage`

Opens a stream to a file contained within the app package.

## Signature

```csharp
System.Threading.Tasks.Task<System.IO.Stream!>! static OpenAppPackageFileAsync(string! filename)
```

## Returns

A `Stream` containing the (read-only) file data.

## Parameters

| Parameter | Description |
|---|---|
| `filename` | The name of the file (excluding the path) to load from the app package. |

## See also

- Declaring type: [[FileSystem|FileSystem]]
- [[_Microsoft.Maui.Storage|Microsoft.Maui.Storage namespace]]
