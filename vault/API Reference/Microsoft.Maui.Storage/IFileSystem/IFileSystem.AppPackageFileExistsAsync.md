---
title: "IFileSystem.AppPackageFileExistsAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Storage
aliases:
  - "Microsoft.Maui.Storage.IFileSystem.AppPackageFileExistsAsync"
declaring_type: "IFileSystem"
member_kind: method
---

# IFileSystem.AppPackageFileExistsAsync

> [!abstract] Method of [[IFileSystem|IFileSystem]]
> Namespace: `Microsoft.Maui.Storage`

Determines whether or not a file exists in the app package.

## Signature

```csharp
System.Threading.Tasks.Task<bool>! AppPackageFileExistsAsync(string! filename)
```

## Returns

`true` when the specified file exists in the app package, otherwise `false`.

## Parameters

| Parameter | Description |
|---|---|
| `filename` | The name of the file (excluding the path) to load from the app package. |

## See also

- Declaring type: [[IFileSystem|IFileSystem]]
- [[_Microsoft.Maui.Storage|Microsoft.Maui.Storage namespace]]
