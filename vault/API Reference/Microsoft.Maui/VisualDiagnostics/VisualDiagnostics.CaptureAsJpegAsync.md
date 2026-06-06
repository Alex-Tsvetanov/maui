---
title: "VisualDiagnostics.CaptureAsJpegAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.VisualDiagnostics.CaptureAsJpegAsync"
declaring_type: "VisualDiagnostics"
member_kind: method
---

# VisualDiagnostics.CaptureAsJpegAsync

> [!abstract] Method of [[VisualDiagnostics|VisualDiagnostics]]
> Namespace: `Microsoft.Maui`

Captures the given view as a JPEG image asynchronously with specified quality.

## Signatures

```csharp
System.Threading.Tasks.Task<byte[]?>! static CaptureAsJpegAsync(Microsoft.Maui.IView! view, int quality = 80)
System.Threading.Tasks.Task<byte[]?>! static CaptureAsJpegAsync(Microsoft.Maui.IWindow! window, int quality = 80)
```

## Returns

A byte array containing the JPEG image, or null if capture failed.

## Parameters

| Parameter | Description |
|---|---|
| `view` | The view to capture. |
| `quality` | The JPEG quality (0-100). |

## See also

- Declaring type: [[VisualDiagnostics|VisualDiagnostics]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
