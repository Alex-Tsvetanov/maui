---
title: "ImageSource.FromStream"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ImageSource.FromStream"
declaring_type: "ImageSource"
member_kind: method
---

# ImageSource.FromStream

> [!abstract] Method of [[ImageSource|ImageSource]]
> Namespace: `Microsoft.Maui.Controls`

Creates an `ImageSource` from a stream factory function.

## Signatures

```csharp
Microsoft.Maui.Controls.ImageSource static FromStream(System.Func<System.IO.Stream> stream)
Microsoft.Maui.Controls.ImageSource static FromStream(System.Func<System.Threading.CancellationToken, System.Threading.Tasks.Task<System.IO.Stream>> stream)
```

## Parameters

| Parameter | Description |
|---|---|
| `stream` | A factory function that returns a stream containing the image data. |

## Returns

A `StreamImageSource` for the stream.

## See also

- Declaring type: [[ImageSource|ImageSource]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
