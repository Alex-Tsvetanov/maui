---
title: "ICommandMapper<TVirtualView, TViewHandler>.Add"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.ICommandMapper<TVirtualView, TViewHandler>.Add"
declaring_type: "ICommandMapper<TVirtualView, TViewHandler>"
member_kind: method
---

# ICommandMapper<TVirtualView, TViewHandler>.Add

> [!abstract] Method of [[ICommandMapper{TVirtualView, TViewHandler}|ICommandMapper<TVirtualView, TViewHandler>]]
> Namespace: `Microsoft.Maui`

Registers a command handler action for the specified key, with an overload that additionally receives the command argument passed to the handler.

## Signatures

```csharp
void Microsoft.Maui.ICommandMapper<TVirtualView, TViewHandler>.Add(string! key, System.Action<TViewHandler, TVirtualView, object?>! action)
void Microsoft.Maui.ICommandMapper<TVirtualView, TViewHandler>.Add(string! key, System.Action<TViewHandler, TVirtualView>! action)
```

## See also

- Declaring type: [[ICommandMapper{TVirtualView, TViewHandler}|ICommandMapper<TVirtualView, TViewHandler>]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
