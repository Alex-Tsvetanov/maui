---
title: "SourceInfo.Deconstruct"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.SourceInfo.Deconstruct"
declaring_type: "SourceInfo"
member_kind: method
---

# SourceInfo.Deconstruct

> [!abstract] Method of [[SourceInfo|SourceInfo]]
> Namespace: `Microsoft.Maui`

Deconstructs a given `SourceInfo` back to its URI and line numbers.

## Signature

```csharp
void Deconstruct(out System.Uri! sourceUri, out int lineNumber, out int linePosition)
```

## Parameters

| Parameter | Description |
|---|---|
| `sourceUri` | The location of the source file where the object was created. |
| `lineNumber` | The line number of the object. |
| `linePosition` | The line position of the object. |

## See also

- Declaring type: [[SourceInfo|SourceInfo]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
