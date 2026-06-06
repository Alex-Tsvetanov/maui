---
title: "FileSystemImplementation.OpenAppPackageFileAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Storage
aliases:
  - "Microsoft.Maui.Storage.FileSystemImplementation.OpenAppPackageFileAsync"
declaring_type: "FileSystemImplementation"
member_kind: method
---

# FileSystemImplementation.OpenAppPackageFileAsync

> [!abstract] Method of [[FileSystemImplementation|FileSystemImplementation]]
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

- Declaring type: [[FileSystemImplementation|FileSystemImplementation]]
- [[_Microsoft.Maui.Storage|Microsoft.Maui.Storage namespace]]
