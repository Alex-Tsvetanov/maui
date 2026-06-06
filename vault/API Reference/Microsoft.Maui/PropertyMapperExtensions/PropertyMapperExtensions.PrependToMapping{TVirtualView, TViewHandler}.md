---
title: "PropertyMapperExtensions.PrependToMapping<TVirtualView, TViewHandler>"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.PropertyMapperExtensions.PrependToMapping<TVirtualView, TViewHandler>"
declaring_type: "PropertyMapperExtensions"
member_kind: method
---

# PropertyMapperExtensions.PrependToMapping<TVirtualView, TViewHandler>

> [!abstract] Method of [[PropertyMapperExtensions|PropertyMapperExtensions]]
> Namespace: `Microsoft.Maui`

Specify a method to be run before an existing property mapping.

## Signatures

```csharp
void static PrependToMapping<TVirtualView, TViewHandler>(this Microsoft.Maui.IPropertyMapper<Microsoft.Maui.IElement!, Microsoft.Maui.IElementHandler!>! propertyMapper, string! key, System.Action<TViewHandler, TVirtualView>! method)
void static PrependToMapping<TVirtualView, TViewHandler>(this Microsoft.Maui.IPropertyMapper<TVirtualView, TViewHandler>! propertyMapper, string! key, System.Action<TViewHandler, TVirtualView>! method)
```

## Parameters

| Parameter | Description |
|---|---|
| `propertyMapper` | The property mapper in which to change the mapping. |
| `key` | The name of the property. |
| `method` | The method to call before the existing mapping begins. |

## See also

- Declaring type: [[PropertyMapperExtensions|PropertyMapperExtensions]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
