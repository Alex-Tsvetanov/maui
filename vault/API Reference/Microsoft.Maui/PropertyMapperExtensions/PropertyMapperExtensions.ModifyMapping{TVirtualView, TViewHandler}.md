---
title: "PropertyMapperExtensions.ModifyMapping<TVirtualView, TViewHandler>"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.PropertyMapperExtensions.ModifyMapping<TVirtualView, TViewHandler>"
declaring_type: "PropertyMapperExtensions"
member_kind: method
---

# PropertyMapperExtensions.ModifyMapping<TVirtualView, TViewHandler>

> [!abstract] Method of [[PropertyMapperExtensions|PropertyMapperExtensions]]
> Namespace: `Microsoft.Maui`

Modify a property mapping in place.

## Signatures

```csharp
void static ModifyMapping<TVirtualView, TViewHandler>(this Microsoft.Maui.IPropertyMapper<Microsoft.Maui.IElement!, Microsoft.Maui.IElementHandler!>! propertyMapper, string! key, System.Action<TViewHandler, TVirtualView, System.Action<Microsoft.Maui.IElementHandler!, Microsoft.Maui.IElement!>?>! method)
void static ModifyMapping<TVirtualView, TViewHandler>(this Microsoft.Maui.IPropertyMapper<TVirtualView, TViewHandler>! propertyMapper, string! key, System.Action<TViewHandler, TVirtualView, System.Action<Microsoft.Maui.IElementHandler!, Microsoft.Maui.IElement!>?>! method)
```

## Parameters

| Parameter | Description |
|---|---|
| `propertyMapper` | The property mapper in which to change the mapping. |
| `key` | The name of the property. |
| `method` | The modified method to call when the property is updated. |

## See also

- Declaring type: [[PropertyMapperExtensions|PropertyMapperExtensions]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
