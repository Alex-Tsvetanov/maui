---
title: "MauiHandlersCollectionExtensions.AddHandler"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Hosting
aliases:
  - "Microsoft.Maui.Hosting.MauiHandlersCollectionExtensions.AddHandler"
declaring_type: "MauiHandlersCollectionExtensions"
member_kind: method
---

# MauiHandlersCollectionExtensions.AddHandler

> [!abstract] Method of [[MauiHandlersCollectionExtensions|MauiHandlersCollectionExtensions]]
> Namespace: `Microsoft.Maui.Hosting`

Registers a handler with the underlying service container via AddTransient.

## Signature

```csharp
Microsoft.Maui.Hosting.IMauiHandlersCollection! static AddHandler(this Microsoft.Maui.Hosting.IMauiHandlersCollection! handlersCollection, System.Type! viewType, System.Type! handlerType)
```

## Parameters

| Parameter | Description |
|---|---|
| `handlersCollection` | The element collection |
| `viewType` | The type of view to register |
| `handlerType` | The handler type that represents the element |

## Returns

The handler collection

## See also

- Declaring type: [[MauiHandlersCollectionExtensions|MauiHandlersCollectionExtensions]]
- [[_Microsoft.Maui.Hosting|Microsoft.Maui.Hosting namespace]]
