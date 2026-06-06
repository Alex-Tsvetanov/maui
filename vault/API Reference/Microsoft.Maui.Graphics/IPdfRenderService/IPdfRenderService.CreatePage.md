---
title: "IPdfRenderService.CreatePage"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.IPdfRenderService.CreatePage"
declaring_type: "IPdfRenderService"
member_kind: method
---

# IPdfRenderService.CreatePage

> [!abstract] Method of [[IPdfRenderService|IPdfRenderService]]
> Namespace: `Microsoft.Maui.Graphics`

Creates a PDF page from the specified stream.

## Signature

```csharp
Microsoft.Maui.Graphics.IPdfPage CreatePage(System.IO.Stream stream, int pageNumber = -1)
```

## Returns

An `IPdfPage` object representing the specified page.

## Parameters

| Parameter | Description |
|---|---|
| `stream` | The stream containing the PDF document. |
| `pageNumber` | The page number to create (negative values indicate the last page). |

## See also

- Declaring type: [[IPdfRenderService|IPdfRenderService]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
