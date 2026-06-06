---
title: "CommandMapperExtensions.ReplaceMapping<TVirtualView, TViewHandler>"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.CommandMapperExtensions.ReplaceMapping<TVirtualView, TViewHandler>"
declaring_type: "CommandMapperExtensions"
member_kind: method
---

# CommandMapperExtensions.ReplaceMapping<TVirtualView, TViewHandler>

> [!abstract] Method of [[CommandMapperExtensions|CommandMapperExtensions]]
> Namespace: `Microsoft.Maui`

Replace a command mapping in place but call the previous mapping if the types do not match.

## Signature

```csharp
void static ReplaceMapping<TVirtualView, TViewHandler>(this Microsoft.Maui.ICommandMapper<Microsoft.Maui.IElement!, Microsoft.Maui.IElementHandler!>! commandMapper, string! key, System.Action<TViewHandler, TVirtualView, object?>! method)
```

## Parameters

| Parameter | Description |
|---|---|
| `commandMapper` | The command mapper in which to change the mapping. |
| `key` | The name of the command. |
| `method` | The modified method to call when the command is updated. |

## See also

- Declaring type: [[CommandMapperExtensions|CommandMapperExtensions]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
