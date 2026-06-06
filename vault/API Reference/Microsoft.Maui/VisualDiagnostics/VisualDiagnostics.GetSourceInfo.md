---
title: "VisualDiagnostics.GetSourceInfo"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.VisualDiagnostics.GetSourceInfo"
declaring_type: "VisualDiagnostics"
member_kind: method
---

# VisualDiagnostics.GetSourceInfo

> [!abstract] Method of [[VisualDiagnostics|VisualDiagnostics]]
> Namespace: `Microsoft.Maui`

Gets the previously registered source information for a specified object.

## Signature

```csharp
Microsoft.Maui.SourceInfo? static GetSourceInfo(object! obj)
```

## Parameters

| Parameter | Description |
|---|---|
| `obj` | The object whose source information is requested. |

## Returns

A `SourceInfo` instance containing the URI, line number, and position, or null if no information is available.

## See also

- Declaring type: [[VisualDiagnostics|VisualDiagnostics]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
