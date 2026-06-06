---
title: "MauiHandlersCollectionExtensions.TryAddHandler<TType, TTypeRender>"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Hosting
aliases:
  - "Microsoft.Maui.Hosting.MauiHandlersCollectionExtensions.TryAddHandler<TType, TTypeRender>"
declaring_type: "MauiHandlersCollectionExtensions"
member_kind: method
---

# MauiHandlersCollectionExtensions.TryAddHandler<TType, TTypeRender>

> [!abstract] Method of [[MauiHandlersCollectionExtensions|MauiHandlersCollectionExtensions]]
> Namespace: `Microsoft.Maui.Hosting`

Registers a handler with the underlying service container via AddTransient.

## Signature

```csharp
Microsoft.Maui.Hosting.IMauiHandlersCollection! static TryAddHandler<TType, TTypeRender>(this Microsoft.Maui.Hosting.IMauiHandlersCollection! handlersCollection)
```

## Returns

The handler collection

## Parameters

| Parameter | Description |
|---|---|
| `handlersCollection` | The handler collection |
| `viewType` | The type of element to register |
| `handlerType` | The handler type that represents the element |

## See also

- Declaring type: [[MauiHandlersCollectionExtensions|MauiHandlersCollectionExtensions]]
- [[_Microsoft.Maui.Hosting|Microsoft.Maui.Hosting namespace]]
