---
title: "PropertyMapperExtensions.AppendToMapping<TVirtualView, TViewHandler>"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.PropertyMapperExtensions.AppendToMapping<TVirtualView, TViewHandler>"
declaring_type: "PropertyMapperExtensions"
member_kind: method
---

# PropertyMapperExtensions.AppendToMapping<TVirtualView, TViewHandler>

> [!abstract] Method of [[PropertyMapperExtensions|PropertyMapperExtensions]]
> Namespace: `Microsoft.Maui`

Specify a method to be run after an existing property mapping.

## Signatures

```csharp
void static AppendToMapping<TVirtualView, TViewHandler>(this Microsoft.Maui.IPropertyMapper<Microsoft.Maui.IElement!, Microsoft.Maui.IElementHandler!>! propertyMapper, string! key, System.Action<TViewHandler, TVirtualView>! method)
void static AppendToMapping<TVirtualView, TViewHandler>(this Microsoft.Maui.IPropertyMapper<TVirtualView, TViewHandler>! propertyMapper, string! key, System.Action<TViewHandler, TVirtualView>! method)
```

## Parameters

| Parameter | Description |
|---|---|
| `propertyMapper` | The property mapper in which to change the mapping. |
| `key` | The name of the property. |
| `method` | The method to call after the existing mapping is finished. |

## See also

- Declaring type: [[PropertyMapperExtensions|PropertyMapperExtensions]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
