---
title: "VisualDiagnostics.RegisterSourceInfo"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.VisualDiagnostics.RegisterSourceInfo"
declaring_type: "VisualDiagnostics"
member_kind: method
---

# VisualDiagnostics.RegisterSourceInfo

> [!abstract] Method of [[VisualDiagnostics|VisualDiagnostics]]
> Namespace: `Microsoft.Maui`

Registers source file information (URI, line number, and position) for the specified target object when XAML diagnostics are enabled.

## Signature

```csharp
void static RegisterSourceInfo(object! target, System.Uri! uri, int lineNumber, int linePosition)
```

## Parameters

| Parameter | Description |
|---|---|
| `target` | The object to associate with source information. |
| `uri` | The URI of the XAML file where the object was defined. |
| `lineNumber` | The line number in the XAML file. |
| `linePosition` | The position within the line in the XAML file. |

## See also

- Declaring type: [[VisualDiagnostics|VisualDiagnostics]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
