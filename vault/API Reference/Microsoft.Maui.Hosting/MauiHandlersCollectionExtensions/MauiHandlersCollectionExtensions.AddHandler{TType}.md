---
title: "MauiHandlersCollectionExtensions.AddHandler<TType>"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Hosting
aliases:
  - "Microsoft.Maui.Hosting.MauiHandlersCollectionExtensions.AddHandler<TType>"
declaring_type: "MauiHandlersCollectionExtensions"
member_kind: method
---

# MauiHandlersCollectionExtensions.AddHandler<TType>

> [!abstract] Method of [[MauiHandlersCollectionExtensions|MauiHandlersCollectionExtensions]]
> Namespace: `Microsoft.Maui.Hosting`

Registers a handler with the underlying service container via AddTransient.

## Signature

```csharp
Microsoft.Maui.Hosting.IMauiHandlersCollection! static AddHandler<TType>(this Microsoft.Maui.Hosting.IMauiHandlersCollection! handlersCollection, System.Func<System.IServiceProvider!, Microsoft.Maui.IElementHandler!>! handlerImplementationFactory)
```

## Returns

The handler collection

## Parameters

| Parameter | Description |
|---|---|
| `handlersCollection` | The element collection |
| `viewType` | The type of view to register |
| `handlerType` | The handler type that represents the element |

## See also

- Declaring type: [[MauiHandlersCollectionExtensions|MauiHandlersCollectionExtensions]]
- [[_Microsoft.Maui.Hosting|Microsoft.Maui.Hosting namespace]]
