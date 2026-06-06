---
title: "CommandMapperExtensions.ModifyMapping<TVirtualView, TViewHandler>"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.CommandMapperExtensions.ModifyMapping<TVirtualView, TViewHandler>"
declaring_type: "CommandMapperExtensions"
member_kind: method
---

# CommandMapperExtensions.ModifyMapping<TVirtualView, TViewHandler>

> [!abstract] Method of [[CommandMapperExtensions|CommandMapperExtensions]]
> Namespace: `Microsoft.Maui`

Modify a command mapping in place.

## Signatures

```csharp
void static ModifyMapping<TVirtualView, TViewHandler>(this Microsoft.Maui.CommandMapper<TVirtualView, TViewHandler>! commandMapper, string! key, System.Action<TViewHandler, TVirtualView, object?, System.Action<Microsoft.Maui.IElementHandler!, Microsoft.Maui.IElement!, object?>?>! method)
void static ModifyMapping<TVirtualView, TViewHandler>(this Microsoft.Maui.ICommandMapper<Microsoft.Maui.IElement!, Microsoft.Maui.IElementHandler!>! commandMapper, string! key, System.Action<TViewHandler, TVirtualView, object?, System.Action<Microsoft.Maui.IElementHandler!, Microsoft.Maui.IElement!, object?>?>! method)
void static ModifyMapping<TVirtualView, TViewHandler>(this Microsoft.Maui.ICommandMapper<TVirtualView, TViewHandler>! commandMapper, string! key, System.Action<TViewHandler, TVirtualView, object?, System.Action<Microsoft.Maui.IElementHandler!, Microsoft.Maui.IElement!, object?>?>! method)
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
