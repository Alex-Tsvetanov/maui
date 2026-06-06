---
title: "VisualDiagnostics.CaptureAsPngAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.VisualDiagnostics.CaptureAsPngAsync"
declaring_type: "VisualDiagnostics"
member_kind: method
---

# VisualDiagnostics.CaptureAsPngAsync

> [!abstract] Method of [[VisualDiagnostics|VisualDiagnostics]]
> Namespace: `Microsoft.Maui`

Captures the given view as a PNG image asynchronously.

## Signatures

```csharp
System.Threading.Tasks.Task<byte[]?>! static CaptureAsPngAsync(Microsoft.Maui.IView! view)
System.Threading.Tasks.Task<byte[]?>! static CaptureAsPngAsync(Microsoft.Maui.IWindow! window)
```

## Returns

A byte array containing the PNG image, or null if capture failed.

## Parameters

| Parameter | Description |
|---|---|
| `view` | The view to capture. |

## See also

- Declaring type: [[VisualDiagnostics|VisualDiagnostics]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
