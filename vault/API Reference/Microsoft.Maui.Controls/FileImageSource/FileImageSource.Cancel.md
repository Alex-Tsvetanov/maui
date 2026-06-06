---
title: "FileImageSource.Cancel"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.FileImageSource.Cancel"
declaring_type: "FileImageSource"
member_kind: method
---

# FileImageSource.Cancel

> [!abstract] Method of [[FileImageSource|FileImageSource]]
> Namespace: `Microsoft.Maui.Controls`

Request a cancel of the ImageSource loading.

## Signature

```csharp
System.Threading.Tasks.Task<bool> override Cancel()
```

## Remarks

overridden for FileImageSource. FileImageSource are not cancellable, so this will always returns a completed Task with `false` as Result.

## Returns

An awaitable Task.

## See also

- Declaring type: [[FileImageSource|FileImageSource]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
