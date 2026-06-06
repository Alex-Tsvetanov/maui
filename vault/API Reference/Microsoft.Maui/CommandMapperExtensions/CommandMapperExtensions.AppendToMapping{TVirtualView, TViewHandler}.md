---
title: "CommandMapperExtensions.AppendToMapping<TVirtualView, TViewHandler>"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.CommandMapperExtensions.AppendToMapping<TVirtualView, TViewHandler>"
declaring_type: "CommandMapperExtensions"
member_kind: method
---

# CommandMapperExtensions.AppendToMapping<TVirtualView, TViewHandler>

> [!abstract] Method of [[CommandMapperExtensions|CommandMapperExtensions]]
> Namespace: `Microsoft.Maui`

Specify a method to be run after an existing command mapping.

## Signatures

```csharp
void static AppendToMapping<TVirtualView, TViewHandler>(this Microsoft.Maui.CommandMapper<TVirtualView, TViewHandler>! commandMapper, string! key, System.Action<TViewHandler, TVirtualView, object?>! method)
void static AppendToMapping<TVirtualView, TViewHandler>(this Microsoft.Maui.ICommandMapper<Microsoft.Maui.IElement!, Microsoft.Maui.IElementHandler!>! commandMapper, string! key, System.Action<TViewHandler, TVirtualView, object?>! method)
void static AppendToMapping<TVirtualView, TViewHandler>(this Microsoft.Maui.ICommandMapper<TVirtualView, TViewHandler>! commandMapper, string! key, System.Action<TViewHandler, TVirtualView, object?>! method)
```

## Parameters

| Parameter | Description |
|---|---|
| `commandMapper` | The command mapper in which to change the mapping. |
| `key` | The name of the command. |
| `method` | The method to call after the existing mapping is finished. |

## See also

- Declaring type: [[CommandMapperExtensions|CommandMapperExtensions]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
