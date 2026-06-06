---
title: "CommandMapperExtensions.PrependToMapping<TVirtualView, TViewHandler>"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.CommandMapperExtensions.PrependToMapping<TVirtualView, TViewHandler>"
declaring_type: "CommandMapperExtensions"
member_kind: method
---

# CommandMapperExtensions.PrependToMapping<TVirtualView, TViewHandler>

> [!abstract] Method of [[CommandMapperExtensions|CommandMapperExtensions]]
> Namespace: `Microsoft.Maui`

Specify a method to be run before an existing command mapping.

## Signatures

```csharp
void static PrependToMapping<TVirtualView, TViewHandler>(this Microsoft.Maui.CommandMapper<TVirtualView, TViewHandler>! commandMapper, string! key, System.Action<TViewHandler, TVirtualView, object?>! method)
void static PrependToMapping<TVirtualView, TViewHandler>(this Microsoft.Maui.ICommandMapper<Microsoft.Maui.IElement!, Microsoft.Maui.IElementHandler!>! commandMapper, string! key, System.Action<TViewHandler, TVirtualView, object?>! method)
void static PrependToMapping<TVirtualView, TViewHandler>(this Microsoft.Maui.ICommandMapper<TVirtualView, TViewHandler>! commandMapper, string! key, System.Action<TViewHandler, TVirtualView, object?>! method)
```

## Parameters

| Parameter | Description |
|---|---|
| `commandMapper` | The command mapper in which to change the mapping. |
| `key` | The name of the command. |
| `method` | The method to call before the existing mapping begins. |

## See also

- Declaring type: [[CommandMapperExtensions|CommandMapperExtensions]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
