---
title: "IScreenshotResult.OpenReadAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Media
aliases:
  - "Microsoft.Maui.Media.IScreenshotResult.OpenReadAsync"
declaring_type: "IScreenshotResult"
member_kind: method
---

# IScreenshotResult.OpenReadAsync

> [!abstract] Method of [[IScreenshotResult|IScreenshotResult]]
> Namespace: `Microsoft.Maui.Media`

Opens a `Stream` to the corresponding screenshot file on the filesystem.

## Signature

```csharp
System.Threading.Tasks.Task<System.IO.Stream!>! OpenReadAsync(Microsoft.Maui.Media.ScreenshotFormat format = Microsoft.Maui.Media.ScreenshotFormat.Png, int quality = 100)
```

## Returns

A `Stream` containing the screenshot file data.

## Parameters

| Parameter | Description |
|---|---|
| `format` | The image format used to read this screenshot. |
| `quality` | The quality used to read this screenshot. Quality only applies when `Jpeg` is used. |

## See also

- Declaring type: [[IScreenshotResult|IScreenshotResult]]
- [[_Microsoft.Maui.Media|Microsoft.Maui.Media namespace]]
