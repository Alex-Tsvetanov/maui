---
title: "PdfPageExtensions.AsStream"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.PdfPageExtensions.AsStream"
declaring_type: "PdfPageExtensions"
member_kind: method
---

# PdfPageExtensions.AsStream

> [!abstract] Method of [[PdfPageExtensions|PdfPageExtensions]]
> Namespace: `Microsoft.Maui.Graphics`

Converts a PDF page to a memory stream.

## Signature

```csharp
System.IO.Stream static AsStream(this Microsoft.Maui.Graphics.IPdfPage target)
```

## Parameters

| Parameter | Description |
|---|---|
| `target` | The PDF page to convert. |

## Returns

A memory stream containing the PDF data, with position set to the beginning, or null if the target is null.

## See also

- Declaring type: [[PdfPageExtensions|PdfPageExtensions]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
