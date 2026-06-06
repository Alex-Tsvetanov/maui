---
title: "FileSystem.AppPackageFileExistsAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Storage
aliases:
  - "Microsoft.Maui.Storage.FileSystem.AppPackageFileExistsAsync"
declaring_type: "FileSystem"
member_kind: method
---

# FileSystem.AppPackageFileExistsAsync

> [!abstract] Method of [[FileSystem|FileSystem]]
> Namespace: `Microsoft.Maui.Storage`

Determines whether or not a file exists in the app package.

## Signature

```csharp
System.Threading.Tasks.Task<bool>! static AppPackageFileExistsAsync(string! filename)
```

## Returns

`true` when the specified file exists in the app package, otherwise `false`.

## Parameters

| Parameter | Description |
|---|---|
| `filename` | The path of the file (relative to the app package) to check the existence of. |

## See also

- Declaring type: [[FileSystem|FileSystem]]
- [[_Microsoft.Maui.Storage|Microsoft.Maui.Storage namespace]]
