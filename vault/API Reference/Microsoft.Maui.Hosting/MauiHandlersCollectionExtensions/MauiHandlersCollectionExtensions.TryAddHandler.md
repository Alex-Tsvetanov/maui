---
title: "MauiHandlersCollectionExtensions.TryAddHandler"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Hosting
aliases:
  - "Microsoft.Maui.Hosting.MauiHandlersCollectionExtensions.TryAddHandler"
declaring_type: "MauiHandlersCollectionExtensions"
member_kind: method
---

# MauiHandlersCollectionExtensions.TryAddHandler

> [!abstract] Method of [[MauiHandlersCollectionExtensions|MauiHandlersCollectionExtensions]]
> Namespace: `Microsoft.Maui.Hosting`

Registers a handler with the underlying service container via AddTransient.

## Signature

```csharp
Microsoft.Maui.Hosting.IMauiHandlersCollection! static TryAddHandler(this Microsoft.Maui.Hosting.IMauiHandlersCollection! handlersCollection, System.Type! viewType, System.Type! handlerType)
```

## Parameters

| Parameter | Description |
|---|---|
| `handlersCollection` | The handler collection |
| `viewType` | The type of element to register |
| `handlerType` | The handler type that represents the element |

## Returns

The handler collection

## See also

- Declaring type: [[MauiHandlersCollectionExtensions|MauiHandlersCollectionExtensions]]
- [[_Microsoft.Maui.Hosting|Microsoft.Maui.Hosting namespace]]
